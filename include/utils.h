/******************************************************************************
 * @file    utils.h
 * @brief   Hardware utility and time abstraction layer.
 *
 * Provides:
 *  - High-resolution monotonic timing functions
 *  - Temperature sensor (MAX31865) interface
 *  - ADC acquisition functions
 *  - Switch and push-button access functions
 *
 * Abstracts hardware interaction from application logic.
 *
 * @author  Haizon Helet Cruz
 * @date    2026-02-13
 ******************************************************************************/

#ifndef UTILS_H
#define UTILS_H

#include "mainH.h"
#include "measdev.h"

#include <time.h>

#define MAX31865_REG_CONFIG 0x00
#define MAX31865_REG_RTD_MSB 0x01
#define MAX31865_REG_RTD_LSB 0x02

#define MAX31865_CFG_50HZ 0x81 // VBIAS + AUTO + 50Hz
#define MAX31865_CNV_START 0x0a1 // VBIAS + 1SHOT + 3WIRE
#define MAX31865_CFG_SHUTDOWN 0x001

// ----------------------------
// Initialize timer
// ----------------------------
void initTimer(void);

// ----------------------------
// Get elapsed time since InitTimer
// ----------------------------
uint64_t getElapsedTime(void);

// ----------------------------
// TEMPERATURE SENSOR
// ----------------------------
int tempSnsrInit(int fd);
unsigned int readMAXSpiInterface(int fd, unsigned int mreg);
void writeMAXSpiInterface(int fd, unsigned int mreg, unsigned int msend);
unsigned int readTempSnsrVal(int fd, unsigned int *val);
void tempSnsrPwrDwn(int fd);

// ----------------------------
// ADC
// ----------------------------
void getADC(int fd, unsigned int *adc_zero, unsigned int *adc_one);

// ----------------------------
// SWITCHES
// ----------------------------
unsigned int readZedSwitches(int fd);

// ----------------------------
// PUSH BUTTONS
// ----------------------------
unsigned int readZedPushButtons(int fd);
#endif /**  UTILS_H    **/