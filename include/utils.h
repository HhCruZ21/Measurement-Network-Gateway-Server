#include "mainH.h"

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