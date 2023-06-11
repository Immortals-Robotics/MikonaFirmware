#ifndef I2C_CALLBACKS_H
#define	I2C_CALLBACKS_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

struct registers_t
{
    // read
    uint8_t dev_id;
    uint8_t status;
    uint8_t v_out[2];

    // write
    bool    charge;
    bool    discharge;
    uint8_t kick_a;
    uint8_t kick_b;
};

extern struct registers_t g_registers;
    
void set_i2c_callbacks();

#ifdef	__cplusplus
}
#endif

#endif	/* I2C_CALLBACKS_H */
