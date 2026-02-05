#ifndef UTILS_H
#define UTILS_H

#include "mainH.h"
#include "measdev.h"

#include <time.h>

static struct timespec Current_time;
static uint64_t US_startup_time;

// ----------------------------
// Initialize timer
// ----------------------------
void InitTimer(void);

// ----------------------------
// Get elapsed time since InitTimer
// ----------------------------
uint64_t GetElapsedTime(void);


// ----------------------------
// TEMPERATURE SENSOR
// ----------------------------
int tempSnsrInit(int fd);
int maxSpiInit(int fd);
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