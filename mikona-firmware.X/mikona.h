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

void charge(bool charge);
void discharge(bool enable);

void kick_a(uint16_t duration);
void kick_b(uint16_t duration);

void setup_adc(void);

#ifdef	__cplusplus
}
#endif

#endif	/* MIKONA_H */

