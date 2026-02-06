#ifndef UTILS_H
#define UTILS_H

#include "mainH.h"
#include "measdev.h"

#include <time.h>

#define MAX31865_REG_CONFIG     0x00
#define MAX31865_REG_RTD_MSB    0x01
#define MAX31865_REG_RTD_LSB    0x02

#define MAX31865_CFG_CONT_50HZ  0xC2  // VBIAS + AUTO + 50Hz
#define MAX31865_CFG_CONT_60HZ  0xC0  // VBIAS + AUTO + 60Hz
#define MAX31865_CFG_SHUTDOWN   0x00

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
unsigned int readTempSnsrVal(int fd, int *val);
int tempSnsrPwrDwn(int fd);

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