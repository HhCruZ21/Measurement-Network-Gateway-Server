#include "../include/fakeSensors.h"
#include <stdlib.h>
#include <time.h>
#include <math.h>

/* -------------------------------------------------
 * Monotonic simulation time (microseconds)
 * ------------------------------------------------- */

 #define ADC_FREQ_HZ 20.0  /* Range: 1.0 to 100.0 */

static inline uint64_t sim_time_us(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000ULL +
           (uint64_t)ts.tv_nsec / 1000ULL;
}

/* -------------------------------------------------
 * Temperature Sensor (slow thermal drift)
 * ------------------------------------------------- */
unsigned int backend_read_temp(void)
{
    uint64_t t = sim_time_us();

    /* Very slow drift: 0.05 Hz (~20 s period) */
    double phase = 2.0 * M_PI *
                   ((t % 20000000ULL) / 20000000.0);

    /* Raw RTD-ish range */
    return (unsigned int)(800 + 5 * sin(phase));
}

/* -------------------------------------------------
 * ADC Channels (phase-locked sine / cosine)
 * ------------------------------------------------- */
/* -------------------------------------------------
 * Configuration
 * ------------------------------------------------- */


/* -------------------------------------------------
 * ADC Channels (Variable frequency sine / cosine)
 * ------------------------------------------------- */
void backend_read_adc(unsigned int *a0, unsigned int *a1)
{
    uint64_t t = sim_time_us();

    /* * Calculate period in microseconds: T = 1,000,000 / frequency
     * For 50Hz, period_us = 20,000
     */
    double period_us = 1000000.0 / ADC_FREQ_HZ;

    /* Use fmod for floating point modulo to handle fractional periods cleanly */
    double phase = 2.0 * M_PI * (fmod((double)t, period_us) / period_us);

    *a0 = (unsigned int)(2048 + 2047 * sin(phase));
    *a1 = (unsigned int)(2048 + 2047 * cos(phase));
}

/* -------------------------------------------------
 * Switches (human-speed toggles)
 * ------------------------------------------------- */
unsigned int backend_read_switches(void)
{
    uint64_t t = sim_time_us();

    /* Toggle pattern every 500 ms */
    return (t / 500000ULL) & 0xFF;
}

/* -------------------------------------------------
 * Push Buttons (momentary pulses)
 * ------------------------------------------------- */
unsigned int backend_read_buttons(void)
{
    uint64_t t = sim_time_us();

    /* 20 ms press every 2 seconds */
    return ((t % 2000000ULL) < 20000ULL) ? 0x01 : 0x00;
}
