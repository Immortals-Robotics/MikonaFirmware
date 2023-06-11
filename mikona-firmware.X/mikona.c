#include "mikona.h"

#include "mcc_generated_files/mcc.h"
#include "i2c-callbacks.h"

void set_led_color(enum led_color_t color)
{
    if (color == LedColorRed)
    {
        PWM3_LoadDutyValue(32);
    }
    else if (color == LedColorGreen)
    {
        PWM3_LoadDutyValue(0);
    }
    else if (color == LedColorOrange)
    {
        PWM3_LoadDutyValue(8);
    }
}

bool is_done()
{
    return !Done_GetValue();
}

uint16_t get_v_out()
{
    // TODO: scale this to mV
    return  ADC_GetConversionResult();
}

void charge(bool enable)
{
    if (enable)
    {
        Charge_SetHigh();
    }
    else
    {
        Charge_SetLow();
    }
}

void discharge(bool enable)
{
    if (enable)
    {
        Discharge_SetHigh();
    }
    else
    {
        Discharge_SetLow();
    }
}

void kick_a(uint8_t duration)
{
    // TODO: improve accuracy
    KickA_SetHigh();
            
    for(;duration > 0; --duration)
        __delay_us(100);

    KickA_SetLow();
}

void kick_b(uint8_t duration)
{
    // TODO: improve accuracy
   KickB_SetHigh();
            
    for(;duration > 0; --duration)
        __delay_us(100);

    KickB_SetLow(); 
}

static void adc_interrupt_handler()
{
    uint16_t v_out_raw = get_v_out();
    g_registers.v_out[0] = v_out_raw & 0xff;
    g_registers.v_out[1] = v_out_raw >> 8;
}

void setup_adc()
{
    ADC_SetInterruptHandler(adc_interrupt_handler);
    ADC_SelectChannel(VOut);
}
