#define _POSIX_C_SOURCE 200112L
#include "../include/utils.h"

static struct timespec Current_time;
static uint64_t US_startup_time;

// ----------------------------
// Initialize timer
// ----------------------------
void InitTimer(void)
{
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0)
    {
        perror("clock_gettime failed");
        exit(EXIT_FAILURE);
    }
    US_startup_time = ts.tv_sec * 1000000 + ts.tv_nsec / 1000; // microseconds
}

// ----------------------------
// Get elapsed time since InitTimer
// ----------------------------
uint64_t GetElapsedTime(void)
{
    if (clock_gettime(CLOCK_MONOTONIC, &Current_time) != 0)
    {
        perror("clock_gettime failed");
        return 0;
    }
    uint64_t us_current_time = Current_time.tv_sec * 1000000 + Current_time.tv_nsec / 1000;
    return us_current_time - US_startup_time;
}