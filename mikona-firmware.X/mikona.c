#include "mikona.h"

#include "mcc_generated_files/mcc.h"
#include "protocol.h"

void set_led_color(enum led_color_t color)
{
    if (color == LedColorRed)
    {
        LED_SetHigh();
    }
    else if (color == LedColorGreen)
    {
        LED_SetLow();
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

void kick_a(uint16_t duration)
{
    if (duration == 0)
        return;
    
    duration = duration >> 1;
    
    RC3_SetDigitalInput();
    PWM4_LoadDutyValue(0);
    
    RC2_SetDigitalOutput();
    PWM3_LoadDutyValue(duration+2);
    
    TMR2_Start();
}

void kick_b(uint16_t duration)
{
    if (duration == 0)
        return;
    
    duration = duration >> 1;
    
    RC2_SetDigitalInput();
    PWM3_LoadDutyValue(0);
    
    RC3_SetDigitalOutput();
    PWM4_LoadDutyValue(duration+2);
    
    TMR2_Start();
}

static void adc_interrupt_handler()
{
    uint16_t v_out_raw = get_v_out();
    g_registers.v_out.u16 = v_out_raw;
}

void setup_adc()
{
    ADC_SetInterruptHandler(adc_interrupt_handler);
    ADC_SelectChannel(VOut);
}
