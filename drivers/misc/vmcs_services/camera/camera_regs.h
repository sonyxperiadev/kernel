#ifndef __CAMERA_REGS_H
#define __CAMERA_REGS_H

//access macro
#define CAMERA_REGISTER_RW(base, offset) (*(volatile unsigned long *)(base + offset))
#define CAMERA_REGISTER_RW_BYTE(base, offset) (*(volatile unsigned char *)(base + offset))

#define CAMERA_CONTROL_OFFSET                 0x00
   #define CAMERA_CONTROL_ENABLE_BIT          0x00000001  
   #define CAMERA_CONTROL_CAPTURE_BIT         0x00000010

#define CAMERA_STATUS_OFFSET                  0x04
   #define CAMERA_STATUS_CAPTURE_DONE_BIT    0x00000010

#define CAMERA_IMAGE_BUFFER_ADDRESS_OFFSET    0x20
#define CAMERA_IMAGE_SIZE_OFFSET              0x24

#endif /* __CAMERA_REGS_H */
