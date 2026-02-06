#define _POSIX_C_SOURCE 200112L
#include "../include/utils.h"

static struct timespec current_time;
static uint64_t us_startup_time;
measObj_struct tf_obj_snd, tf_obj_rcv;

// ----------------------------
// Initialize timer
// ----------------------------
void initTimer(void)
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
uint64_t getElapsedTime(void)
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
// the maxread function, for reading commands into temperature sensor.
unsigned int readMAXSpiInterface(int fd, unsigned int mreg)
{
    unsigned int xdata;
    int k;

    mreg <<= 8;
    tf_obj_snd.rnum = 1;
    tf_obj_snd.rvalue = mreg;
    write(fd, &tf_obj_snd, MEASOBJ_SIZE);
    k = 0;
    xdata = 0xc0000000;
    do
    {
        tf_obj_snd.rnum = REGNUM_ID;
        tf_obj_snd.rvalue = 1;
        write(fd, &tf_obj_snd, MEASOBJ_SIZE);
        k++;
        read(fd, &tf_obj_rcv, MEASOBJ_SIZE); // Read response from LKM
        xdata = tf_obj_rcv.rvalue;
    } while (((xdata & 0x01) == 0) && (k < 1000));
    if (k >= 1000)
    {
        printf(" *** MaxRead timeout.");
    }
    else
    {
        tf_obj_snd.rnum = REGNUM_ID;
        tf_obj_snd.rvalue = 2;
        write(fd, &tf_obj_snd, MEASOBJ_SIZE);
        read(fd, &tf_obj_rcv, MEASOBJ_SIZE); // Read response from LKM
        xdata = tf_obj_rcv.rvalue;
        xdata &= 0x0ff;
        // printf("loop count: %d |  data: %x\n\r", k, xdata);
    }
    return xdata;
}

// the function for writting the values from the temperature sensor.
void writeMAXSpiInterface(int fd, unsigned int mreg, unsigned int msend)
{
    unsigned int transm_data, xdata;
    int k;

    mreg |= 0x080;
    transm_data = (mreg << 8) | (msend & 0x0ff);
    tf_obj_snd.rnum = 1;
    tf_obj_snd.rvalue = transm_data;
    write(fd, &tf_obj_snd, MEASOBJ_SIZE);
    k = 0;
    xdata = 0xc0000000;
    do
    {
        tf_obj_snd.rnum = REGNUM_ID;
        tf_obj_snd.rvalue = 1;
        write(fd, &tf_obj_snd, MEASOBJ_SIZE);
        k++;
        read(fd, &tf_obj_rcv, MEASOBJ_SIZE); // Read response from LKM
        xdata = tf_obj_rcv.rvalue;
    } while (((xdata & 0x01) == 0) && (k < 1000));
    if (k >= 1000)
    {
        printf(" *** MaxWrite timeout.");
    }
}

int tempSnsrInit(int fd)
{
    tf_obj_snd.rnum   = 0;
    tf_obj_snd.rvalue = 0x022;
    if (write(fd, &tf_obj_snd, MEASOBJ_SIZE) < 0)
        return -1;

    // Enable VBIAS + continuous conversion (50Hz)
    writeMAXSpiInterface(fd, MAX31865_REG_CONFIG, MAX31865_CFG_CONT_50HZ);

    // Allow bias + first conversion to settle
    usleep(10000);   // 10 ms

    printf("MAX31865 initialized in continuous mode...\n");
    return 0;
}

unsigned int readTempSnsrVal(int fd, unsigned int *regVal)
{
    uint16_t msb, lsb, val;

    if (!regVal)
        return -1;

    /*
     * Continuous mode:
     *  - DO NOT start conversion
     *  - DO NOT poll status
     *  - Just read registers
     */

    msb = readMAXSpiInterface(fd, MAX31865_REG_RTD_MSB);
    lsb = readMAXSpiInterface(fd, MAX31865_REG_RTD_LSB);

    val = ((msb << 8) | lsb) >> 1;  // strip fault bit

    *regVal = val;
    return 0;

}

void Temp_Shutdown(int fd)
{
    writeMAXSpiInterface(fd, 0x00, MAX31865_CFG_SHUTDOWN);
}

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