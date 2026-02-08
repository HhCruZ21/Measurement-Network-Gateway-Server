#include "../include/fakeSensors.h"
#include "utils.h"
#include <stdlib.h>
#include <time.h>
#include "math.h"

static unsigned int sim_counter = 0;

unsigned int backend_read_temp()
{
    // SIM: fake RTD raw value (~25°C)
    return 800 + (rand() % 10);
}

void backend_read_adc(unsigned int *a0, unsigned int *a1)
{
    float angle = (2.0f * M_PI * sim_counter) / SINE_PERIOD;

    *a0 = (unsigned int)(2048 + 2047 * sinf(angle));

    *a1 = (unsigned int)(2048 + 2047 * cosf(angle)); // Cosine = sin(angle + π/2)

    sim_counter++;
    if (sim_counter >= SINE_PERIOD)
    {
        sim_counter = 0; // Wrap around for continuous wave
    }
}

unsigned int backend_read_switches()
{
    // SIM: toggle bits
    return (sim_counter >> 4) & 0xFF;
}

unsigned int backend_read_buttons()
{
    // SIM: momentary presses
    return (sim_counter % 1000 == 0) ? 0x01 : 0x00;
}
