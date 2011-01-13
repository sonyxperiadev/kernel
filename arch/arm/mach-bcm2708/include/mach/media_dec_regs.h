/*=============================================================================
Copyright (c) 2007 Broadcom Europe Limited.
All rights reserved.

Project  :  vcfw
Module   :  ipc driver
File     :  $RCSfile: adc.h,v $
Revision :  $Revision: #1 $

FILE DESCRIPTION

=============================================================================*/

#ifndef MEDIA_DEC_REGS_H_
#define MEDIA_DEC_REGS_H_

#define MEDIA_DEC_CONTROL_OFFSET                    0x00
   #define MEDIA_DEC_CONTROL_ENABLE_BIT             0x00000001  
   #define MEDIA_DEC_CONTROL_PLAY_BIT               0x00000002
   
   // The following are independent bits of the above control bits;
   // set them to indicate you want the following requests
   #define MEDIA_DEC_CONTROL_SET_FLAGS_BIT          0x00000004
   #define MEDIA_DEC_CONTROL_SET_SRC_REGION_BIT     0x00000008
   #define MEDIA_DEC_CONTROL_SET_TARGET_REGION_BIT  0x00000010
   #define MEDIA_DEC_CONTROL_SET_VOLUME_BIT         0x00000020
   #define MEDIA_DEC_CONTROL_GET_VOLUME_BIT         0x00000040
   #define MEDIA_DEC_CONTROL_GET_VIDEO_LEVEL_BIT    0x00000080
   #define MEDIA_DEC_CONTROL_GET_AUDIO_LEVEL_BIT    0x00000100
   #define MEDIA_DEC_CONTROL_SET_MUTED_BIT          0x00000200
   #define MEDIA_DEC_CONTROL_SET_TRANSPARENCY_BIT   0x00000400
   
   #define MEDIA_DEC_CONTROL_LOCAL_DATAMODE_BIT     0x20000000
   

   #define MEDIA_DEC_CONTROL_LOCAL_FILEMODE_BIT     0x40000000
   #define MEDIA_DEC_CONTROL_ERROR_BIT              0x80000000
#define MEDIA_DEC_STATUS_OFFSET                     0x04

#define MEDIA_DEC_SOURCE_X_OFFSET                   0x20
#define MEDIA_DEC_SOURCE_Y_OFFSET                   0x24
#define MEDIA_DEC_SOURCE_WIDTH_OFFSET               0x20
#define MEDIA_DEC_SOURCE_HEIGHT_OFFSET              0x24

#define MEDIA_DEC_TARGET_DISPLAY_OFFSET             0x40
#define MEDIA_DEC_TARGET_LAYER_OFFSET               0x44
#define MEDIA_DEC_TARGET_X_OFFSET                   0x48
#define MEDIA_DEC_TARGET_Y_OFFSET                   0x4C
#define MEDIA_DEC_TARGET_WIDTH_OFFSET               0x50
#define MEDIA_DEC_TARGET_HEIGHT_OFFSET              0x54
#define MEDIA_DEC_TARGET_ASPECT_MODE                0x58

#define MEDIA_DEC_VID_TYPE                          0x60
#define MEDIA_DEC_AUD_TYPE                          0x64

#define MEDIA_DEC_MAX_BUFFER_SIZE                   0x70
#define MEDIA_DEC_PLAYBACK_TIME                     0x74

#define MEDIA_DEC_DEBUG_MASK                        0x80

#define MEDIA_DEC_DEBUG_FILENAME                    0x90
#define MEDIA_DEC_DEBUG_FILENAME_LENGTH             256

#define MEDIA_DEC_DATA_ADDRESS_OFFSET               0xA0
#define MEDIA_DEC_DATA_LENGTH_OFFSET                0xA4

//implemention of peterson's algorithm for shared memory semaphores
#define MEDIA_DEC_FLAG0_OFFSET                     0x200
#define MEDIA_DEC_FLAG1_OFFSET                     0x204
#define MEDIA_DEC_TURN_OFFSET                      0x208


#define MEDIA_DEC_VIDEO_OUT_WRITE_PTR_OFFSET             0x230
#define MEDIA_DEC_VIDEO_OUT_READ_PTR_OFFSET              0x234
#define MEDIA_DEC_VIDEO_OUT_FIFO_SIZE_OFFSET             0x238
#define MEDIA_DEC_VIDEO_OUT_FIFO_ENTRY_OFFSET            0x23C
#define MEDIA_DEC_VIDEO_OUT_FIFO_START_OFFSET            0x240

//0x14 entries here of 0x1c words each = 0x230 gap from 0x50
#define MEDIA_DEC_VIDEO_OUT_FIFO_OFFSET                  0x250

#define MEDIA_DEC_AUDIO_OUT_WRITE_PTR_OFFSET             0x480
#define MEDIA_DEC_AUDIO_OUT_READ_PTR_OFFSET              0x484
#define MEDIA_DEC_AUDIO_OUT_FIFO_SIZE_OFFSET             0x488
#define MEDIA_DEC_AUDIO_OUT_FIFO_ENTRY_OFFSET            0x48C
#define MEDIA_DEC_AUDIO_OUT_FIFO_START_OFFSET            0x490

//0x14 entries here of 0x1c words each = 0x230 gap from 0x50
#define MEDIA_DEC_AUDIO_OUT_FIFO_OFFSET                  0x4A0


#define MEDIA_DEC_VIDEO_IN_WRITE_PTR_OFFSET              0x6D0
#define MEDIA_DEC_VIDEO_IN_READ_PTR_OFFSET               0x6D4
#define MEDIA_DEC_VIDEO_IN_FIFO_SIZE_OFFSET              0x6D8
#define MEDIA_DEC_VIDEO_IN_FIFO_ENTRY_OFFSET             0x6DC
#define MEDIA_DEC_VIDEO_IN_FIFO_START_OFFSET             0x6E0

//8 entries here of 4 words each = 0x80 gap from 0xF0
#define MEDIA_DEC_VIDEO_IN_FIFO_OFFSET                   0x6F0


#define MEDIA_DEC_AUDIO_IN_WRITE_PTR_OFFSET              0x7A0
#define MEDIA_DEC_AUDIO_IN_READ_PTR_OFFSET               0x7A4
#define MEDIA_DEC_AUDIO_IN_FIFO_SIZE_OFFSET              0x7A8
#define MEDIA_DEC_AUDIO_IN_FIFO_ENTRY_OFFSET             0x7AC
#define MEDIA_DEC_AUDIO_IN_FIFO_START_OFFSET             0x7B0

//8 entries here of 4 words each = 0x80 gap from 0xF0
#define MEDIA_DEC_AUDIO_IN_FIFO_OFFSET                   0x7C0

#define MEDIA_DEC_FLAGS_OFFSET                           0x840
#define MEDIA_DEC_VOLUME_OFFSET                          0x844

// Flags for the FLAGS field above
#define MEDIA_DEC_FLAGS_PAUSED      0x01
#define MEDIA_DEC_FLAGS_MUTED       0x02

#define MEDIA_DEC_TARGET_FULLSCREEN_OFFSET               0x848
#define MEDIA_DEC_TARGET_TRANSFORM_OFFSET                0x84C

#define MEDIA_DEC_TRANSPARENCY_OFFSET                    0x850
#define MEDIA_DEC_VIDEO_LEVEL_OFFSET                     0x854
#define MEDIA_DEC_AUDIO_LEVEL_OFFSET                     0x858


//this struct is tightly packet - its size is 32bytes
typedef struct
{
   uint32_t buffer_id;
   uint32_t buffer_size;
   uint32_t buffer_filled_size;
   uint32_t buffer_ptr;
   uint32_t timestamp;
   uint32_t sequence_number;
   uint32_t flags;
   uint32_t spare;

} MEDIA_DEC_FIFO_ENTRY_T;


#endif // MEDIA_DEC_REGS_H_
