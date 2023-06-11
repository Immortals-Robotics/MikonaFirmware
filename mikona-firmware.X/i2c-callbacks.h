#ifndef I2C_CALLBACKS_H
#define	I2C_CALLBACKS_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>
    
extern bool reg_charge;
extern bool reg_discharge;
extern uint8_t reg_kick_a;
extern uint8_t reg_kick_b;
    
void I2CSetCallbacks();

#ifdef	__cplusplus
}
#endif

#endif	/* I2C_CALLBACKS_H */

