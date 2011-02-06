#ifndef __CAMERA_REGS_H
#define __CAMERA_REGS_H

#if defined( __cplusplus )
extern "C"
{
#endif

//access macro
#define CAMERA_REGISTER_RW(base, offset) (*(volatile unsigned long *)(base + offset))
#define CAMERA_REGISTER_RW_BYTE(base, offset) (*(volatile unsigned char *)(base + offset))

#define CAMERA_CONTROL_OFFSET                 0x00
#define CAMERA_CONTROL_ENABLE_BIT             0x00000001

#define CAMERA_STATUS_OFFSET                  0x04

#if defined( __cplusplus )
}
#endif
#endif /* __CAMERA_REGS_H */
