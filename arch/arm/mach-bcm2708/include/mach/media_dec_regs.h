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

//implemention of peterson's algorithm for shared memory semaphores
#define MEDIA_DEC_FLAG0_OFFSET                     0x200
#define MEDIA_DEC_FLAG1_OFFSET                     0x204
#define MEDIA_DEC_TURN_OFFSET                      0x208

#define MEDIA_DEC_OUT_WRITE_PTR_OFFSET             0x230
#define MEDIA_DEC_OUT_READ_PTR_OFFSET              0x234
#define MEDIA_DEC_OUT_FIFO_SIZE_OFFSET             0x238
#define MEDIA_DEC_OUT_FIFO_ENTRY_OFFSET            0x23C
#define MEDIA_DEC_OUT_FIFO_START_OFFSET            0x240

//8 entries here of 4 words each = 0x80 gap from 0x50
#define MEDIA_DEC_OUT_FIFO_OFFSET                  0x250

#define MEDIA_DEC_IN_WRITE_PTR_OFFSET              0x2D0
#define MEDIA_DEC_IN_READ_PTR_OFFSET               0x2D4
#define MEDIA_DEC_IN_FIFO_SIZE_OFFSET              0x2D8
#define MEDIA_DEC_IN_FIFO_ENTRY_OFFSET             0x2DC
#define MEDIA_DEC_IN_FIFO_START_OFFSET             0x2E0

//8 entries here of 4 words each = 0x80 gap from 0xF0
#define MEDIA_DEC_IN_FIFO_OFFSET                   0x2F0

//video types

typedef enum
{
    MEDIA_DEC_VIDEO_CodingUnused,     /**< Value when coding is N/A */
    MEDIA_DEC_VIDEO_CodingAutoDetect, /**< Autodetection of coding type */
    MEDIA_DEC_VIDEO_CodingMPEG2,      /**< AKA: H.262 */
    MEDIA_DEC_VIDEO_CodingH263,       /**< H.263 */
    MEDIA_DEC_VIDEO_CodingMPEG4,      /**< MPEG-4 */
    MEDIA_DEC_VIDEO_CodingWMV,        /**< all versions of Windows Media Video */
    MEDIA_DEC_VIDEO_CodingRV,         /**< all versions of Real Video */
    MEDIA_DEC_VIDEO_CodingAVC,        /**< H.264/AVC */
    MEDIA_DEC_VIDEO_CodingMJPEG,      /**< Motion JPEG */
    MEDIA_DEC_VIDEO_CodingVP6,        /**< On2 VP6 */
    MEDIA_DEC_VIDEO_CodingVP7,        /**< On2 VP7 */
    MEDIA_DEC_VIDEO_CodingVP8,        /**< On2 VP8 */
    MEDIA_DEC_VIDEO_CodingSorenson,   /**< Sorenson */
    MEDIA_DEC_VIDEO_CodingTheora      /**< Theora */
} MEDIA_DEC_VIDEO_T;

typedef enum
{
    MEDIA_DEC_AUDIO_CodingUnused = 0,  /**< Placeholder value when coding is N/A  */
    MEDIA_DEC_AUDIO_CodingAutoDetect,  /**< auto detection of audio format */
    MEDIA_DEC_AUDIO_CodingPCM,         /**< Any variant of PCM coding */
    MEDIA_DEC_AUDIO_CodingADPCM,       /**< Any variant of ADPCM encoded data */
    MEDIA_DEC_AUDIO_CodingAMR,         /**< Any variant of AMR encoded data */
    MEDIA_DEC_AUDIO_CodingGSMFR,       /**< Any variant of GSM fullrate (i.e. GSM610) */
    MEDIA_DEC_AUDIO_CodingGSMEFR,      /**< Any variant of GSM Enhanced Fullrate encoded data*/
    MEDIA_DEC_AUDIO_CodingGSMHR,       /**< Any variant of GSM Halfrate encoded data */
    MEDIA_DEC_AUDIO_CodingPDCFR,       /**< Any variant of PDC Fullrate encoded data */
    MEDIA_DEC_AUDIO_CodingPDCEFR,      /**< Any variant of PDC Enhanced Fullrate encoded data */
    MEDIA_DEC_AUDIO_CodingPDCHR,       /**< Any variant of PDC Halfrate encoded data */
    MEDIA_DEC_AUDIO_CodingTDMAFR,      /**< Any variant of TDMA Fullrate encoded data (TIA/EIA-136-420) */
    MEDIA_DEC_AUDIO_CodingTDMAEFR,     /**< Any variant of TDMA Enhanced Fullrate encoded data (TIA/EIA-136-410) */
    MEDIA_DEC_AUDIO_CodingQCELP8,      /**< Any variant of QCELP 8kbps encoded data */
    MEDIA_DEC_AUDIO_CodingQCELP13,     /**< Any variant of QCELP 13kbps encoded data */
    MEDIA_DEC_AUDIO_CodingEVRC,        /**< Any variant of EVRC encoded data */
    MEDIA_DEC_AUDIO_CodingSMV,         /**< Any variant of SMV encoded data */
    MEDIA_DEC_AUDIO_CodingG711,        /**< Any variant of G.711 encoded data */
    MEDIA_DEC_AUDIO_CodingG723,        /**< Any variant of G.723 dot 1 encoded data */
    MEDIA_DEC_AUDIO_CodingG726,        /**< Any variant of G.726 encoded data */
    MEDIA_DEC_AUDIO_CodingG729,        /**< Any variant of G.729 encoded data */
    MEDIA_DEC_AUDIO_CodingAAC,         /**< Any variant of AAC encoded data */
    MEDIA_DEC_AUDIO_CodingMP3,         /**< Any variant of MP3 encoded data */
    MEDIA_DEC_AUDIO_CodingSBC,         /**< Any variant of SBC encoded data */
    MEDIA_DEC_AUDIO_CodingVORBIS,      /**< Any variant of VORBIS encoded data */
    MEDIA_DEC_AUDIO_CodingWMA,         /**< Any variant of WMA encoded data */
    MEDIA_DEC_AUDIO_CodingRA,          /**< Any variant of RA encoded data */
    MEDIA_DEC_AUDIO_CodingMIDI,        /**< Any variant of MIDI encoded data */

    MEDIA_DEC_AUDIO_CodingFLAC,        /**< Any variant of FLAC */
    MEDIA_DEC_AUDIO_CodingDDP,         /**< Any variant of Dolby Digital Plus */
    MEDIA_DEC_AUDIO_CodingDTS,         /**< Any variant of DTS */
    MEDIA_DEC_AUDIO_CodingWMAPRO,      /**< Any variant of WMA Professional */
    MEDIA_DEC_AUDIO_CodingATRAC3,      /**< Sony ATRAC-3 variants */
    MEDIA_DEC_AUDIO_CodingATRACX,      /**< Sony ATRAC-X variants */
    MEDIA_DEC_AUDIO_CodingATRACAAL     /**< Sony ATRAC advanced-lossless variants  */

} MEDIA_DEC_AUDIO_T;

//this struct is tightly packet - its size is 32bytes
typedef struct
{
   uint32_t buffer_id;
   uint32_t buffer_size;
   uint32_t buffer_filled_size;
   uint32_t buffer_ptr;
   uint32_t timestamp;
   uint32_t sequence_number;
   uint32_t spare;

} MEDIA_DEC_FIFO_ENTRY_T;


#endif // MEDIA_DEC_REGS_H_
