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
    uint8_t dev_id;       // read-only
    uint8_t status;       // read-write
    union reg_u16 v_out;  // read-only
    union reg_u16 kick_a; // write-only
    union reg_u16 kick_b; // write-only
};
extern struct registers_t g_registers;

// status register helpers
bool reg_status_get_charge(uint8_t status);
bool reg_status_get_discharge(uint8_t status);
bool reg_status_get_done(uint8_t status);
bool reg_status_get_fault(uint8_t status);

void reg_status_set_charge(uint8_t* status, bool value);
void reg_status_set_discharge(uint8_t* status, bool value);
void reg_status_set_done(uint8_t* status, bool value);
void reg_status_set_fault(uint8_t* status, bool value);
    
// sets our i2c interrupt handlers
void set_i2c_callbacks();

#ifdef	__cplusplus
}
#endif

#endif	/* PROTOCOL_H */
