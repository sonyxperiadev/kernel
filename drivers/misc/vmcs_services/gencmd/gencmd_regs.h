#ifndef __GENCMD_REGS_H
#define __GENCMD_REGS_H

#define GENCMD_MAX_LENGTH 1024

/******************************************************************************
  Local typedefs
 *****************************************************************************/

#define GENCMD_CONTROL_OFFSET                0x00
#define GENCMD_CONTROL_GO                    0x01
#define GENCMD_CONTROL_DONE                  0x02

#define GENCMD_REQUEST_LENGTH_OFFSET  0x10
#define GENCMD_REQUEST_DATA_OFFSET    0x20

#define GENCMD_REPLY_LENGTH_OFFSET    0x200
#define GENCMD_REPLY_DATA_OFFSET      0x210

//access macro
#define GENCMD_REGISTER_RW(base, offset) (*(volatile unsigned long *)(base + offset))

#endif /* __GENCMD_REGS_H */
