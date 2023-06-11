#include "i2c-callbacks.h"

#include "mcc_generated_files/mcc.h"

// registers:

// read
#define REG_DEV_ID 0x01
#define REG_STATUS 0x02
#define REG_V_OUT  0x03

// write
#define REG_CHARGE    0x11
#define REG_DISCHARGE 0x12
#define REG_KICK_A    0x13
#define REG_KICK_B    0x14

// read:


// write:
bool reg_charge = 0;
bool reg_discharge = 0;
uint8_t reg_kick_a = 0;
uint8_t reg_kick_b = 0;

// internal data
uint8_t reg_id = 0;
uint8_t counter = 0;

uint8_t read_address = 0;

static void I2CReadCallback() {
    uint8_t read_data = I2C1_Read();
    
    if (counter == 0) {
        // first byte is register address
        reg_id = read_data;
    }
    else {
        uint8_t array_idx = counter - 1;
        
        if (reg_id == REG_CHARGE) {
            reg_charge = read_data;
        }
        else if (reg_id == REG_DISCHARGE) {
            reg_discharge = read_data;
        }
        else if (reg_id == REG_KICK_A) {
            reg_kick_a = read_data;
        }
        else if (reg_id == REG_KICK_B) {
            reg_kick_b = read_data;
        }
    }
    
    counter++;
}

static void I2CWriteCallback() {
    I2C1_Write(counter++);
}

static void I2CAddressCallback() {
    read_address = I2C1_Read() >> 1;
    counter = 0;
}

static void I2CWriteCollisionCallback() {
}

static void I2CBusCollisionCallback() {
}

void I2CSetCallbacks() {
    I2C1_SlaveSetReadIntHandler(I2CReadCallback);
    I2C1_SlaveSetWriteIntHandler(I2CWriteCallback);
    I2C1_SlaveSetAddrIntHandler(I2CAddressCallback);
    I2C1_SlaveSetWrColIntHandler(I2CWriteCollisionCallback);
    I2C1_SlaveSetBusColIntHandler(I2CBusCollisionCallback);
}
