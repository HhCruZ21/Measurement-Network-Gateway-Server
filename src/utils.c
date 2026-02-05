#define _POSIX_C_SOURCE 200112L
#include "../include/utils.h"

static struct timespec current_time;
static uint64_t us_startup_time;
measObj_struct tf_obj_snd, tf_obj_rcv;

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
    us_startup_time = ts.tv_sec * 1000000 + ts.tv_nsec / 1000; // microseconds
}

// ----------------------------
// Get elapsed time since InitTimer
// ----------------------------
uint64_t GetElapsedTime(void)
{
    if (clock_gettime(CLOCK_MONOTONIC, &current_time) != 0)
    {
        perror("clock_gettime failed");
        return 0;
    }
    uint64_t us_current_time = current_time.tv_sec * 1000000 + current_time.tv_nsec / 1000;
    return us_current_time - us_startup_time;
}

// ----------------------------
// TEMPERATURE SENSOR
// ----------------------------

// ----------------------------
// ADC
// ----------------------------
void getADC(int fd, unsigned int *adc_zero, unsigned int *adc_one)
{
    int ccount;
    unsigned int xstatus, adc_lZero, adc1_lOne;

    tf_obj_snd.rnum = 4; // start conversion
    tf_obj_snd.rvalue = 0;
    write(fd, &tf_obj_snd, MEASOBJ_SIZE);
    ccount = 0;
    do
    {
        ccount++;
        read(fd, &tf_obj_rcv, MEASOBJ_SIZE); // Read response from LKM
        xstatus = tf_obj_rcv.rvalue;
    } while ((xstatus == 0) && (ccount < 10000000));
    tf_obj_snd.rnum = REGNUM_ID;
    tf_obj_snd.rvalue = 5; // ADC status register
    write(fd, &tf_obj_snd, MEASOBJ_SIZE);
    read(fd, &tf_obj_rcv, MEASOBJ_SIZE); // Read response from LKM
    adc1_lOne = tf_obj_rcv.rvalue;
    adc_lZero = adc1_lOne & 0x0fff;
    adc1_lOne >>= 16;
    *adc_zero = adc_lZero;
    *adc_one = adc1_lOne;
}

// ----------------------------
// SWITCHES
// ----------------------------
unsigned int readZedSwitches(int fd)
{
    // Read switch register
    tf_obj_snd.rnum = REGNUM_ID;
    tf_obj_snd.rvalue = 0;

    // requesting the LKM for reading the switch from zedboard.
    write(fd, &tf_obj_snd, MEASOBJ_SIZE);

    // getting the read data from the LKM
    read(fd, &tf_obj_rcv, MEASOBJ_SIZE);

    return (tf_obj_rcv.rvalue) & 0xff;
}

// ----------------------------
// PUSH BUTTONS
// ----------------------------
unsigned int readZedPushButtons(int fd)
{
    // Read button register
    tf_obj_snd.rnum = REGNUM_ID;
    tf_obj_snd.rvalue = 0;
    // reqeusting the LKM for reading the data.
    write(fd, &tf_obj_snd, MEASOBJ_SIZE);

    // getting the values from the LKM.
    read(fd, &tf_obj_rcv, MEASOBJ_SIZE);

    return ((tf_obj_rcv.rvalue) >> 8) & 0x1f;
}