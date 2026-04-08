#include "mikona.h"

#include "mcc_generated_files/mcc.h"
#include "protocol.h"
#include <stdint.h>

#define RECHARGE_THRESHOLD_V  10u
#define MAX_VOLTAGE_SAMPLES   5u

typedef enum
{
    CS_IDLE,
    CS_CHARGING,
    CS_CHARGED,
} charge_state_t;

static volatile charge_state_t g_charge_state = CS_IDLE;
static volatile uint8_t g_max_voltage = 0;
static volatile uint8_t g_max_voltage_samples[MAX_VOLTAGE_SAMPLES];
static volatile uint8_t g_max_voltage_count = 0;
static volatile uint8_t g_max_voltage_idx = 0;

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

static void setCharge(bool enable)
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

static void update_max_voltage(uint8_t v)
{
    g_max_voltage_samples[g_max_voltage_idx] = v;
    g_max_voltage_idx = (g_max_voltage_idx + 1) % MAX_VOLTAGE_SAMPLES;
    if (g_max_voltage_count < MAX_VOLTAGE_SAMPLES)
        g_max_voltage_count++;

    // Copy filled portion and insertion-sort to find median
    uint8_t sorted[MAX_VOLTAGE_SAMPLES];
    for (uint8_t i = 0; i < g_max_voltage_count; i++)
        sorted[i] = g_max_voltage_samples[i];
    for (uint8_t i = 1; i < g_max_voltage_count; i++)
    {
        uint8_t key = sorted[i];
        uint8_t j = i;
        while (j > 0 && sorted[j - 1] > key)
        {
            sorted[j] = sorted[j - 1];
            j--;
        }
        sorted[j] = key;
    }
    g_max_voltage = sorted[g_max_voltage_count / 2];
}

static void adc_interrupt_handler(void)
{
    g_registers.v_out = get_v_out();

    if (g_max_voltage > 0)
    {
        uint8_t threshold = g_max_voltage > RECHARGE_THRESHOLD_V
                          ? g_max_voltage - RECHARGE_THRESHOLD_V
                          : 0;
        bool done = g_registers.v_out >= threshold;

        REG_SET_BIT(REG_STATUS_DONE_BIT, done);
        set_led_color(done ? LedColorRed : LedColorGreen);

        if (!done && REG_GET_BIT(REG_STATUS_CHARGE_BIT) && g_charge_state == CS_CHARGED)
        {
            // Voltage dropped below threshold: pull CHARGE low to reset IC.
            // The IC will respond by pulling DONE high, triggering done_ioc_handler,
            // which will re-arm CHARGE high to start the next cycle.
            g_charge_state = CS_CHARGING;
            setCharge(false);
        }
    }
}

static void done_ioc_handler(void)
{
    if (is_done())
    {
        // DONE pin went low: IC finished a charge cycle.
        update_max_voltage(g_registers.v_out);
        g_charge_state = CS_CHARGED;
    }
    else
    {
        // DONE pin went high: IC was reset (we pulled CHARGE low).
        // Re-arm immediately if user still wants charging.
        if (REG_GET_BIT(REG_STATUS_CHARGE_BIT))
        {
            setCharge(true);
        }
    }
}

void on_charge_requested(bool enable)
{
    if (enable)
    {
        if (g_charge_state == CS_IDLE)
        {
            setCharge(true);
            g_charge_state = CS_CHARGING;
        }
    }
    else
    {
        setCharge(false);
        g_charge_state = CS_IDLE;
    }
}

void setup_adc(void)
{
    ADC_SetInterruptHandler(adc_interrupt_handler);
    ADC_SelectChannel(VOut);
}

void setup_done_ioc(void)
{
    IOCAF4_SetInterruptHandler(done_ioc_handler);
}
