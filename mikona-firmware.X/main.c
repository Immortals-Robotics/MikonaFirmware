#include "mcc_generated_files/mcc.h"

#include "protocol.h"
#include "mikona.h"

int main(void)
{
    SYSTEM_Initialize();
    INTERRUPT_GlobalInterruptEnable();
    INTERRUPT_PeripheralInterruptEnable();

    setup_adc();
    setup_timer();
    setup_i2c();
    setup_done_ioc();
    set_led_color(LedColorGreen);

    // Never SLEEP here: this part has no idle mode, so sleep stops FOSC and
    // freezes TMR0/TMR2 — including the PWM one-shot mid-kick-pulse.
    while (1)
    {
        CLRWDT();
    }

    shutdown_i2c();
    
    return 0;
}
