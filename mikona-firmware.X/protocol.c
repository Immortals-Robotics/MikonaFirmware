#include "protocol.h"

#include "mcc_generated_files/mcc.h"
#include "mikona.h"

volatile struct registers_t g_registers = {};

static volatile struct
{
    uint8_t reg_id;
    uint8_t counter;
    uint8_t read_address;
} g_internal = {};

static void i2c_read_callback(void)
{
    uint8_t read_data = I2C1_Read();
    
    if (g_internal.counter == 0)
    {
        // first byte is register address
        // for both read and write registers
        g_internal.reg_id = read_data;
    }
    else
    {
        uint8_t array_idx = g_internal.counter - 1;
        
        if (g_internal.reg_id == REG_ADDR_STATUS)
        {
            g_registers.status =
                    (~REG_STATUS_WRITE_MASK & g_registers.status) |
                    (REG_STATUS_WRITE_MASK & read_data);

            bool charge = REG_GET_BIT(REG_STATUS_CHARGE_BIT);
            bool discharge = REG_GET_BIT(REG_STATUS_DISCHARGE_BIT);

            if (charge && discharge)
            {
                // Charging and discharging simultaneously is invalid: reject both.
                REG_SET_BIT(REG_STATUS_CHARGE_BIT, 0);
                REG_SET_BIT(REG_STATUS_DISCHARGE_BIT, 0);
                set_fault(REG_FAULT_INVALID_CMD_BIT);
                on_charge_requested(false);
                on_discharge_requested(false);
            }
            else
            {
                on_charge_requested(charge);
                on_discharge_requested(discharge);
            }
        }
        else if (g_internal.reg_id == REG_ADDR_KICK_A)
        {
            g_registers.kick_a.u8[array_idx] = read_data;

            if (array_idx == 1)
            {
                if (REG_GET_BIT(REG_STATUS_DISCHARGE_BIT))
                {
                    set_fault(REG_FAULT_INVALID_CMD_BIT);
                }
                else
                {
                    setKickA(g_registers.kick_a.u16);
                }
                g_registers.kick_a.u16 = 0;
            }
        }
        else if (g_internal.reg_id == REG_ADDR_KICK_B)
        {
            g_registers.kick_b.u8[array_idx] = read_data;

            if (array_idx == 1)
            {
                if (REG_GET_BIT(REG_STATUS_DISCHARGE_BIT))
                {
                    set_fault(REG_FAULT_INVALID_CMD_BIT);
                }
                else
                {
                    setKickB(g_registers.kick_b.u16);
                }
                g_registers.kick_b.u16 = 0;
            }
        }
        else if (g_internal.reg_id == REG_ADDR_FAULT)
        {
            // Master writes 0 to clear all fault bits and the status FAULT flag.
            if (read_data == 0)
            {
                g_registers.fault = 0;
                REG_SET_BIT(REG_STATUS_FAULT_BIT, 0);
                set_led_color(LedColorGreen);
            }
        }
    }

    g_internal.counter++;
}

static void i2c_write_callback(void)
{
    uint8_t array_idx = g_internal.counter;
    uint8_t write_data = 0;
        
    if (g_internal.reg_id == REG_ADDR_DEV_ID)
    {
        write_data = DEV_ID;
    }
    else if (g_internal.reg_id == REG_ADDR_STATUS)
    {
        write_data = g_registers.status & REG_STATUS_READ_MASK;
    }
    else if (g_internal.reg_id == REG_ADDR_V_OUT)
    {
        write_data = g_registers.v_out;
    }
    else if (g_internal.reg_id == REG_ADDR_FAULT)
    {
        write_data = g_registers.fault;
    }
    
    I2C1_Write(write_data);
    g_internal.counter++;
}

static void i2c_address_callback(void)
{
    g_internal.read_address = I2C1_Read() >> 1;
    g_internal.counter = 0;
}

void setup_i2c(void)
{
    I2C1_Open();
    I2C1_SlaveSetReadIntHandler(i2c_read_callback);
    I2C1_SlaveSetWriteIntHandler(i2c_write_callback);
    I2C1_SlaveSetAddrIntHandler(i2c_address_callback);
}

void shutdown_i2c(void)
{
    I2C1_Close();
}
