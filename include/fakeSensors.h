#ifndef FAKESNSR_H
#define FAKESNSR_H

#include "mainH.h"

#define M_PI 3.14159265358979323846

unsigned int backend_read_temp();
void backend_read_adc(unsigned int *a0, unsigned int *a1);
unsigned int backend_read_switches();
unsigned int backend_read_buttons();

#endif // FAKESNSR_H
