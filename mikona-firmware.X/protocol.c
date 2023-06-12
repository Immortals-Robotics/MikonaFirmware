#include "protocol.h"

#include "mcc_generated_files/mcc.h"

// registers

// read
#define REG_DEV_ID 0x01
#define REG_STATUS 0x02
#define REG_V_OUT  0x03

// write
#define REG_CHARGE    0x11
#define REG_DISCHARGE 0x12
#define REG_KICK_A    0x13
#define REG_KICK_B    0x14

struct registers_t g_registers = 
{
    .dev_id = 0x54
};

static struct
{
    // internal data
    uint8_t reg_id;
    uint8_t counter;

    uint8_t read_address;
} g_internal = {};

static void i2c_read_callback()
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
        
        if (g_internal.reg_id == REG_CHARGE)
        {
            g_registers.charge = read_data;
        }
        else if (g_internal.reg_id == REG_DISCHARGE)
        {
            g_registers.discharge = read_data;
        }
        else if (g_internal.reg_id == REG_KICK_A)
        {
            g_registers.kick_a.u8[array_idx] = read_data;
        }
        else if (g_internal.reg_id == REG_KICK_B)
        {
            g_registers.kick_b.u8[array_idx] = read_data;
        }
    }
    
    g_internal.counter++;
}

static void i2c_write_callback()
{
    uint8_t array_idx = g_internal.counter;
    uint8_t write_data = 0;
        
    if (g_internal.reg_id == REG_DEV_ID)
    {
        write_data = g_registers.dev_id;
    }
    else if (g_internal.reg_id == REG_STATUS)
    {
        write_data = g_registers.status;
    }
    else if (g_internal.reg_id == REG_V_OUT)
    {
        write_data = g_registers.v_out.u8[array_idx];
    }
    
    I2C1_Write(write_data);
    g_internal.counter++;
}

static void i2c_address_callback()
{
    g_internal.read_address = I2C1_Read() >> 1;
    g_internal.counter = 0;
}

static void i2c_write_collision_callback()
{
}

static void i2c_bus_collision_callback()
{
}

void set_i2c_callbacks()
{
    I2C1_SlaveSetReadIntHandler(i2c_read_callback);
    I2C1_SlaveSetWriteIntHandler(i2c_write_callback);
    I2C1_SlaveSetAddrIntHandler(i2c_address_callback);
    I2C1_SlaveSetWrColIntHandler(i2c_write_collision_callback);
    I2C1_SlaveSetBusColIntHandler(i2c_bus_collision_callback);
}
