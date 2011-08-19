/*****************************************************************************
* Copyright 2004 - 2011 Broadcom Corporation.  All rights reserved.
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2, available at
* http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a
* license other than the GPL, without Broadcom's express prior written
* consent.
*****************************************************************************/


/****************************************************************************/
/**
*   @file   ssasw.h
*
*   @brief  API definitions for the linux SSASW (Semi-Synchronous Audio Switch)
*           interface.
*/
/****************************************************************************/

#if !defined( ASM_ARM_ARCH_BCMHANA_SSASW_H )
#define ASM_ARM_ARCH_BCMHANA_SSASW_H

/* ---- Include Files ---------------------------------------------------- */

#include <linux/kernel.h>
#include <linux/wait.h>
#include <linux/semaphore.h>
//#include <linux/broadcom/timer.h>
#include <linux/broadcom/dump-mem.h>
#include <linux/scatterlist.h>
#include <linux/dma-mapping.h>
#include <linux/mm.h>
#include <linux/vmalloc.h>
#include <linux/pagemap.h>

#include <mach/dma_mmap.h>
//#include <mach/dma.h>

#include <chal/chal_caph.h>

/* ---- Constants and Types ---------------------------------------------- */

/* Number of Semi-synchronous Audio Switch channels */
#define SSASW_NUM_CHANNELS     16

#define SSASW_INVALID_HANDLE  ((SSASW_Handle_t) -1)

typedef int SSASW_Handle_t;

typedef enum
{
   SSASW_AUDIOH_VINR = 0,
   SSASW_AUDIOH_VINL,
   SSASW_AUDIOH_NVINR,
   SSASW_AUDIOH_NVINL,
   SSASW_AUDIOH_EARPIECE,
   SSASW_AUDIOH_HEADSET,
   SSASW_AUDIOH_HANDSFREE,
   SSASW_AUDIOH_VIBRA,
   SSASW_SRC_44100_AUDIOH_EARPIECE,
   SSASW_SRC_44100_AUDIOH_HEADSET,
   SSASW_SRC_44100_AUDIOH_HANDSFREE,

   /* Add new entries before this line. */

   SSASW_NUM_DEVICE_ENTRIES,
   SSASW_DEVICE_NONE = 0xff,    /* Special value to indicate that no device is currently assigned. */

} SSASW_Device_t;

typedef struct
{
   uint32_t                     inUse;               /* Flag to indicate device in use */
   const char                   *name;               /* Will show up in the /proc entry */

   int16_t                      mix_dig_gain;        /* Gain setting for mixer */

   CAPH_SWITCH_TRIGGER_e        dev_sw_trig;         /* Main device switch trigger condition setting */
   CAPH_DATA_FORMAT_e           sw_data_format;      /* Switch data format */

   CAPH_SWITCH_TRIGGER_e        src_sw_trig;         /* SRC switch trigger condition setting */
   CAPH_SRCMixer_FIFO_e         src_in_fifo;         /* SRC input FIFO */
   CAPH_SRCMixer_FIFO_e         src_out_fifo;        /* SRC output FIFO */
   CAPH_SRCMixer_CHNL_e         mix_src_ch;          /* SRC mixer source channel */
   CAPH_SRCMixer_OUTPUT_e       mix_dst_ch;          /* SRC mixer destination channel */
   CAPH_SRCMixer_SRC_e          mix_freq;            /* SRC up/down convert frequency mode */
   CAPH_DATA_FORMAT_e           src_in_data_format;  /* SRC in FIFO mixer data format */
   CAPH_DATA_FORMAT_e           src_out_data_format; /* SRC out FIFO mixer data format */

} SSASW_DeviceAttribute_t;

typedef struct
{
   uint32_t            inUse;            /* Number of channels in use */
   SSASW_Device_t      devType;          /* Device this channel is currently reserved for */

   CAPH_SWITCH_CHNL_e  caph_switch_ch;   /* CAPH Switch Channel */

} SSASW_Channel_t;

typedef struct
{
   CHAL_HANDLE         chalSsaswHandle;      /* SSASW handle */
   CHAL_HANDLE         chalSrcMixerHandle;   /* SRC Mixer handle */

} SSASW_ChalHandle_t;

typedef struct
{
   struct semaphore    lock;   /* acquired when manipulating table entries */
   SSASW_Channel_t     channel[ SSASW_NUM_CHANNELS ];

} SSASW_Global_t;

/* ---- Variable Externs ------------------------------------------------- */
/* ---- Function Prototypes ---------------------------------------------- */

#if defined( __KERNEL__ )

/****************************************************************************/
/**
*   Initializes the SSASW module.
*
*   @return
*       0       - Success
*       < 0     - Error
*/
/****************************************************************************/

int ssasw_init( void );

/****************************************************************************/
/**
*   Exits the SSASW module.
*
*   @return
*       0       - Success
*       < 0     - Error
*/
/****************************************************************************/

void ssasw_exit( void );

/****************************************************************************/
/**
*   Reserves a channel for use with @a dev. If the device is setup to use
*   a shared channel, then this function will block until a free channel
*   becomes available.
*
*   @return
*       >= 0    - A valid SSASW Handle.
*       -EBUSY  - Device is currently being used.
*       -ENODEV - Device handed in is invalid.
*/
/****************************************************************************/

SSASW_Handle_t ssasw_request_channel
(
   SSASW_Device_t dev,
   uint16_t       src_addr,
   uint16_t       dst_addr
);

/****************************************************************************/
/**
*   Frees a previously allocated SSASW Handle.
*
*   @return
*        0      - DMA Handle was released successfully.
*       -EINVAL - Invalid DMA handle
*/
/****************************************************************************/
int ssasw_free_channel
(
    SSASW_Handle_t    handle            /* SSASW handle. */
);

/****************************************************************************/
/**
*   Adds an additional destination to the switch device.
*
*   @return
*       -ENODEV - Invalid handle
*/
/****************************************************************************/
int ssasw_add_dst
(
    SSASW_Handle_t   handle,            /* SSASW handle. */
    uint16_t         dst_addr
);

/****************************************************************************/
/**
*   Adds an additional destination to the switch device.
*
*   @return
*       -ENODEV - Invalid handle
*/
/****************************************************************************/
int ssasw_remove_dst
(
    SSASW_Handle_t   handle,            /* SSASW handle. */
    uint16_t         src_addr
);

/****************************************************************************/
/**
*   Removes all destinations from the switch device.
*
*   @return
*       -ENODEV - Invalid handle
*/
/****************************************************************************/
int ssasw_remove_all_dst
(
    SSASW_Handle_t   handle            /* SSASW handle. */
);

/****************************************************************************/
/**
*   Sets the gain on the mixer switch device.
*
*   @return
*       -ENODEV - Invalid handle
*/
/****************************************************************************/
int ssasw_mixer_gain_set
(
   SSASW_Device_t dev,
   int db
);

/****************************************************************************/
/**
*   Enables/disables the switch
*
*   @return
*       -ENODEV - Invalid handle
*/
/****************************************************************************/
int ssasw_enable
(
   SSASW_Handle_t handle,
   int            enable
);

/****************************************************************************/
/**
*   Enables/disables the switch mixer
*
*   @return
*       -ENODEV - Invalid handle
*/
/****************************************************************************/
int ssasw_mixer_enable
(
   SSASW_Handle_t handle,
   int            enable
);




#endif  /* defined( __KERNEL__ ) */

#endif  /* ASM_ARM_ARCH_BCMHANA_SSASW_H */

