/******************************************************************************
 * @file    fakeSensor.h
 * @brief   Simulation backend interface for virtual sensor data generation.
 *
 * Provides function prototypes for simulated temperature, ADC, switch,
 * and push-button inputs when the system operates in MODE_SIM.
 *
 * Used when real hardware device (/dev/meascdd) is unavailable.
 *
 * @author  Haizon Helet Cruz
 * @date    2026-02-13
 ******************************************************************************/

#ifndef FAKESNSR_H
#define FAKESNSR_H

#include "mainH.h"

#define M_PI 3.14159265358979323846

unsigned int backend_read_temp();
void backend_read_adc(unsigned int *a0, unsigned int *a1);
unsigned int backend_read_switches();
unsigned int backend_read_buttons();

#endif // FAKESNSR_H
