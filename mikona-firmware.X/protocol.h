#ifndef PROTOCOL_H
#define	PROTOCOL_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

union reg_u16
{
    uint8_t u8[2];
    uint16_t u16;
};
    
struct registers_t
{
    // read
    uint8_t dev_id;
    uint8_t status;
    union reg_u16 v_out;

    // write
    bool    charge;
    bool    discharge;
    union reg_u16 kick_a;
    union reg_u16 kick_b;
};

extern struct registers_t g_registers;
    
void set_i2c_callbacks();

#ifdef	__cplusplus
}
#endif

#endif	/* PROTOCOL_H */
