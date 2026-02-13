/******************************************************************************
 * @file    measdev.h
 * @brief   Measurement device interface definitions.
 *
 * Contains register mappings and communication structures used to interact
 * with the Linux kernel module (/dev/meascdd).
 *
 * Defines:
 *  - Register object structure
 *  - Register identifiers
 *  - Device constants and timer configuration
 *
 * @author  Haizon Helet Cruz
 * @date    2026-02-13
 ******************************************************************************/

#ifndef MEASDEV_H_
#define MEASDEV_H_

// MEAScdd IO data
typedef struct
{
    unsigned int rnum;
    unsigned int rvalue;
} measObj_struct;

#define MEASOBJ_SIZE sizeof(measObj_struct)
#define REGNUM_ID 0x0FF
#define NREGS 8

#define TIMER100ms 0x009896ff
#define MBINT_ENABLE 0x80000000
#define MBINT_ACKN 0x40000000

#endif /* MEASDEV_H_ */
