#include "mikona.h"

#include "mcc_generated_files/mcc.h"
#include "protocol.h"
#include <stdint.h>

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

bool is_done(void)
{
    return !Done_GetValue();
}

uint8_t get_v_out(void)
{
    uint16_t adc_raw = ADC_GetConversionResult();
    // V_actual = adc * (4.096 * 101 / 1023)
    // TODO: figure out why a 1.25 factor was needed in the divider
    uint32_t v = ((uint32_t)adc_raw * 404u) / 1250u;
    return (uint8_t)(v > 255u ? 255u : v);
}

void setCharge(bool enable)
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

void setDischarge(bool enable)
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

void setKickA(uint16_t duration)
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

void setKickB(uint16_t duration)
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

static void adc_interrupt_handler(void)
{
    g_registers.v_out = get_v_out();
}

void setup_adc(void)
{
    ADC_SetInterruptHandler(adc_interrupt_handler);
    ADC_SelectChannel(VOut);
}

static void done_ioc_handler(void)
{
    bool done = is_done();
    REG_SET_BIT(REG_STATUS_DONE_BIT, done);
    set_led_color(done ? LedColorRed : LedColorGreen);
}

void setup_done_ioc(void)
{
    IOCAF4_SetInterruptHandler(done_ioc_handler);
}

