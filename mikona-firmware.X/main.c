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

    while (1)
    {
        bool is_done_value = is_done();
        REG_SET_BIT(REG_STATUS_DONE_BIT, is_done_value);
        if (is_done_value)
        {
            set_led_color(LedColorRed);
        }
        else
        {
            set_led_color(LedColorGreen);
        }


    }

    I2C1_Close();

    return 0;
}
