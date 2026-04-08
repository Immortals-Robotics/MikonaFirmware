#ifndef MIKONA_H
#define	MIKONA_H

#ifdef	__cplusplus
extern "C" {
#endif
    
#include <stdbool.h>
#include <stdint.h>

enum led_color_t
{
    LedColorRed = 0,
    LedColorGreen = 1
};

void set_led_color(enum led_color_t color);

bool is_done(void);
// Returns voltage in volts, scaled to fit in uint8_t (0-255 = 0-4.096V)
uint8_t get_v_out(void);

void on_charge_requested(bool enable);
void setDischarge(bool enable);

void setKickA(uint16_t duration);
void setKickB(uint16_t duration);

void setup_adc(void);
void setup_done_ioc(void);

#ifdef	__cplusplus
}
#endif

#endif	/* MIKONA_H */

