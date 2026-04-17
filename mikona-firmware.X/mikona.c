#include "mikona.h"

#include "mcc_generated_files/mcc.h"
#include "protocol.h"
#include <stdint.h>

#define RECHARGE_THRESHOLD_V   10u
#define MAX_VOLTAGE_SAMPLES    5u
#define CHARGE_TIMEOUT_MS      5000u
#define DISCHARGE_CHECK_MS     1000u
#define DISCHARGE_MIN_DROP_V   5u
#define KICK_CHECK_MS          50u
#define KICK_MIN_DROP_V        10u
#define KICK_PWM_PERIOD_COUNTS 1000u
#define KICK_MAX_DUTY_COUNTS   (KICK_PWM_PERIOD_COUNTS - 1u)
#define KICK_PWM_STEP_US       16u
#define KICK_MAX_PULSE_US      (KICK_MAX_DUTY_COUNTS * KICK_PWM_STEP_US)

typedef enum
{
    CS_IDLE,
    CS_CHARGING,
    CS_CHARGED,
    CS_KICK_INHIBIT,  // charging paused for kick; resumes automatically after KICK_CHECK_MS
} charge_state_t;

// TMR0 ISR fires exactly every 10ms — g_time_ms is incremented there.
// g_time_ms wraps at 65535 — all timeout comparisons use wrapping subtraction.
static volatile uint16_t g_time_ms = 0;

static volatile charge_state_t g_charge_state = CS_IDLE;
static volatile uint16_t g_charge_start_ms = 0;

static volatile uint8_t g_max_voltage = 0;
static volatile uint8_t g_max_voltage_samples[MAX_VOLTAGE_SAMPLES];
static volatile uint8_t g_max_voltage_count = 0;
static volatile uint8_t g_max_voltage_idx = 0;

static volatile uint16_t g_discharge_start_ms = 0;
static volatile uint8_t  g_discharge_start_v  = 0;

static volatile bool     g_kick_pending      = false;
static volatile uint8_t  g_kick_fault_bit    = 0;
static volatile uint16_t g_kick_start_ms     = 0;
static volatile uint8_t  g_kick_start_v      = 0;

#define KICK_MIN_START_V       (KICK_MIN_DROP_V * 2u)  // skip check if caps were below this at kick time

static uint16_t kick_duration_us_to_duty_counts(uint16_t duration_us)
{
    uint16_t duty_counts;

    if (duration_us == 0u)
    {
        return 0u;
    }

    if (duration_us > KICK_MAX_PULSE_US)
    {
        duration_us = KICK_MAX_PULSE_US;
    }

    duty_counts = duration_us / KICK_PWM_STEP_US;
    if ((duration_us % KICK_PWM_STEP_US) != 0u)
    {
        duty_counts++;
    }

    return duty_counts;
}

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

void set_fault(uint8_t fault_bit)
{
    g_registers.fault |= (uint8_t)(1u << fault_bit);
    REG_SET_BIT(REG_STATUS_FAULT_BIT, 1);
    set_led_color(LedColorRed);
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

static void setDischarge(bool enable)
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
    uint16_t duty_counts = kick_duration_us_to_duty_counts(duration);

    if (duty_counts == 0u)
        return;

    RC3_SetDigitalInput();
    PWM4_LoadDutyValue(0);

    RC2_SetDigitalOutput();
    PWM3_LoadDutyValue(duty_counts);

    g_kick_pending   = true;
    g_kick_fault_bit = REG_FAULT_KICK_A_NO_DROP_BIT;
    g_kick_start_ms  = g_time_ms;
    g_kick_start_v   = g_registers.v_out;
    if (g_charge_state != CS_IDLE)
    {
        g_charge_state = CS_KICK_INHIBIT;
        setCharge(false);
    }

    TMR2_Start();
}

void setKickB(uint16_t duration)
{
    uint16_t duty_counts = kick_duration_us_to_duty_counts(duration);

    if (duty_counts == 0u)
        return;

    RC2_SetDigitalInput();
    PWM3_LoadDutyValue(0);

    RC3_SetDigitalOutput();
    PWM4_LoadDutyValue(duty_counts);

    g_kick_pending   = true;
    g_kick_fault_bit = REG_FAULT_KICK_B_NO_DROP_BIT;
    g_kick_start_ms  = g_time_ms;
    g_kick_start_v   = g_registers.v_out;
    if (g_charge_state != CS_IDLE)
    {
        g_charge_state = CS_KICK_INHIBIT;
        setCharge(false);
    }

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

static void tmr0_interrupt_handler(void)
{
    g_time_ms += 10u;
}

static void adc_interrupt_handler(void)
{
    g_registers.v_out = get_v_out();

    // Charge timeout fault: IC never pulled DONE low within the allowed window.
    if (g_charge_state == CS_CHARGING &&
        (uint16_t)(g_time_ms - g_charge_start_ms) >= CHARGE_TIMEOUT_MS)
    {
        setCharge(false);
        g_charge_state = CS_IDLE;
        REG_SET_BIT(REG_STATUS_CHARGE_BIT, 0);
        set_fault(REG_FAULT_CHARGE_TIMEOUT_BIT);
    }

    // Discharge stuck fault: voltage hasn't dropped enough after check window.
    if (REG_GET_BIT(REG_STATUS_DISCHARGE_BIT) &&
        (uint16_t)(g_time_ms - g_discharge_start_ms) >= DISCHARGE_CHECK_MS)
    {
        if (g_registers.v_out > g_discharge_start_v ||
            (g_discharge_start_v - g_registers.v_out) < DISCHARGE_MIN_DROP_V)
        {
            setDischarge(false);
            REG_SET_BIT(REG_STATUS_DISCHARGE_BIT, 0);
            set_fault(REG_FAULT_DISCHARGE_STUCK_BIT);
        }
        else
        {
            // Still dropping fine — slide the window forward.
            g_discharge_start_ms = g_time_ms;
            g_discharge_start_v  = g_registers.v_out;
        }
    }

    // Kick no-drop fault: voltage didn't fall after a kick (stuck FET or open circuit).
    if (g_kick_pending &&
        (uint16_t)(g_time_ms - g_kick_start_ms) >= KICK_CHECK_MS)
    {
        g_kick_pending = false;
        if (g_kick_start_v >= KICK_MIN_START_V &&
            (g_registers.v_out > g_kick_start_v ||
             (g_kick_start_v - g_registers.v_out) < KICK_MIN_DROP_V))
        {
            set_fault(g_kick_fault_bit);
        }
        if (g_charge_state == CS_KICK_INHIBIT)
        {
            g_charge_start_ms = g_time_ms;
            g_charge_state = CS_CHARGING;
            setCharge(true);
        }
    }

    if (g_max_voltage > 0)
    {
        uint8_t threshold = g_max_voltage > RECHARGE_THRESHOLD_V
                          ? g_max_voltage - RECHARGE_THRESHOLD_V
                          : 0;
        bool done = g_registers.v_out >= threshold;

        REG_SET_BIT(REG_STATUS_DONE_BIT, done);

        if (!done && REG_GET_BIT(REG_STATUS_CHARGE_BIT) && g_charge_state == CS_CHARGED)
        {
            // IC is already in reset (done_ioc pulled CHARGE low when it finished).
            // Start a new charge cycle directly.
            g_charge_start_ms = g_time_ms;
            g_charge_state = CS_CHARGING;
            setCharge(true);
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
        setCharge(false);  // reset IC immediately; recharge will re-arm directly when needed
    }
    // DONE going high is just the IC acknowledging the reset — nothing to do.
}

void on_charge_requested(bool enable)
{
    if (enable)
    {
        if (g_charge_state == CS_IDLE)
        {
            g_charge_start_ms = g_time_ms;
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

void on_discharge_requested(bool enable)
{
    setDischarge(enable);
    if (enable)
    {
        g_discharge_start_ms = g_time_ms;
        g_discharge_start_v  = g_registers.v_out;
    }
}

void setup_adc(void)
{
    ADC_SetInterruptHandler(adc_interrupt_handler);
    ADC_SelectChannel(VOut);
}

void setup_timer(void)
{
    TMR0_SetInterruptHandler(tmr0_interrupt_handler);
}

void setup_done_ioc(void)
{
    IOCAF4_SetInterruptHandler(done_ioc_handler);
}
