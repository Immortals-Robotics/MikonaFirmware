#include "mcc_generated_files/mcc.h"

#include "protocol.h"
#include "mikona.h"

int main(void)
{
    SYSTEM_Initialize();
    INTERRUPT_GlobalInterruptEnable();
    INTERRUPT_PeripheralInterruptEnable();

    setup_adc();
    setup_i2c();
    setup_done_ioc();
    set_led_color(LedColorGreen);

    while (1)
    {
        SLEEP();
    }

    shutdown_i2c();
    
    return 0;
}
