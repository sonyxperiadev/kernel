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
*   @file   aadma.h
*
*   @brief  API definitions for the linux AADMA (Asynchronous Audio DMA) interface.
*/
/****************************************************************************/

#if !defined( ASM_ARM_ARCH_BCMHANA_AADMA_H )
#define ASM_ARM_ARCH_BCMHANA_AADMA_H

/* ---- Include Files ---------------------------------------------------- */

#include <linux/kernel.h>
#include <linux/semaphore.h>

#include <mach/profile_timer.h>

#include <chal/chal_caph_dma.h>
#include <chal/chal_caph_cfifo.h>

/****************************************************************************
*
*   The AADMA_Handle_t is the primary object used by callers of the API.
*
*****************************************************************************/

#define AADMA_INVALID_HANDLE           ((AADMA_Handle_t) -1)

#define AADMA_DEVICE_FLAG_IN_USE       0x00000800  /* If set, device is in use on a channel */
#define AADMA_DEVICE_FLAG_SECURE       0x00001000

#define AADMA_CHANNEL_FLAG_IN_USE         0x00000001

typedef int AADMA_Handle_t;

typedef enum
{
   AADMA_DEVICE_AUDIOH_VINR_CFIFO_TO_MEM = 0,
   AADMA_DEVICE_AUDIOH_VINL_CFIFO_TO_MEM,
   AADMA_DEVICE_AUDIOH_NVINR_CFIFO_TO_MEM,
   AADMA_DEVICE_AUDIOH_NVINL_CFIFO_TO_MEM,
   AADMA_DEVICE_AUDIOH_MEM_TO_CFIFO_TO_EARPIECE,
   AADMA_DEVICE_AUDIOH_MEM_TO_CFIFO_TO_HEADSET,
   AADMA_DEVICE_AUDIOH_MEM_TO_CFIFO_TO_HANDSFREE,
   AADMA_DEVICE_AUDIOH_MEM_TO_CFIFO_TO_VIBRA,

   /* Add new entries before this line. */

   AADMA_NUM_DEVICE_ENTRIES,
   AADMA_DEVICE_NONE = 0xff,    /* Special value to indicate that no device is currently assigned. */

} AADMA_Device_t;

typedef struct
{
   int dma_status;

} AADMA_Status_t;

typedef void (*AADMA_DeviceHandler_t)( AADMA_Device_t dev, AADMA_Status_t *dma_status, void *userData );

/* ---- Constants and Types ---------------------------------------------- */

/*
 * If AADMA_DEBUG_TRACK_RESERVATION is set to a non-zero value, then the filename
 * and line number of the reservation request will be recorded in the channel table
 */

#define AADMA_DEBUG_TRACK_RESERVATION   0

/* Number of Asynchronous Audio DMA channels */
#define AADMA_NUM_CHANNELS     16

/****************************************************************************
*
*   The AADMA_DeviceAttribute_t contains information which describes a
*   particular AADMA device (or peripheral).
*
*   It is anticipated that the arrary of AADMA_DeviceAttribute_t's will be
*   statically initialized.
*
*****************************************************************************/

/*
 * Note: Some DMA devices can be used from multiple DMA Controllers. The bitmask is used to
 *       determine which DMA controllers a given device can be used from, and the interface
 *       array determeines the actual interface number to use for a given controller.
 */

typedef struct
{
    uint32_t                     flags;               /* Bitmask of DMA_DEVICE_FLAG_xxx constants */
    const char                   *name;               /* Will show up in the /proc entry */

    uint8_t                      aadma_trans_size;    /* AADMA Transfer burst size */
    CAPH_CFIFO_CHNL_DIRECTION_e  cfifo_dir;           /* CFIFO direction */
    uint16_t                     cfifo_size;          /* CFIFO buffer size */
    uint16_t                     cfifo_thres1;        /* CFIFO threshold 1 trigger */
    uint16_t                     cfifo_thres2;        /* CFIFO threshold 2 trigger */

    void                         *userData;           /* Passed to the devHandler */
    AADMA_DeviceHandler_t        devHandler;          /* Called when DMA operations finish. */
    timer_tick_count_t           transferStartTime;   /* Time the current transfer was started */

    uint32_t                     dev_fifo_addr;       /* Device FIFO address */

    /*
     * The following statistical information will be collected and presented in a proc entry.
     * Note: With a contiuous bandwidth of 1 Gb/sec, it would take 584 years to overflow
     *       a 64 bit counter.
     */

    uint64_t            numTransfers;   /* Number of DMA transfers performed */
    uint64_t            transferTicks;  /* Total time spent doing DMA transfers (measured in timer_tick_count_t's) */
    uint64_t            transferBytes;  /* Total bytes transferred */
    uint32_t            timesBlocked;   /* Number of times a channel was unavailable */
    uint32_t            numBytes;       /* Last transfer size */

} AADMA_DeviceAttribute_t;


/****************************************************************************
*
*   AADMA_Channel_t, AADMA_Controller_t, and AADMA_State_t are really internal
*   data structures and don't belong in this header file, but are included
*   merely for discussion.
*
*   By the time this is implemented, these structures will be moved out into
*   the appropriate C source file instead.
*
*****************************************************************************/

/****************************************************************************
*
*   The DMA_Channel_t contains state information about each DMA channel. Some
*   of the channels are dedicated. Non-dedicated channels are shared
*   amongst the other devices.
*
*****************************************************************************/

typedef struct
{
    uint32_t            flags;         /* bitmask of DMA_CHANNEL_FLAG_xxx constants */
    AADMA_Device_t      devType;       /* Device this channel is currently reserved for */

#if ( AADMA_DEBUG_TRACK_RESERVATION )
    const char          *fileName;     /* Place where channel reservation took place */
    int                 lineNum;       /* Place where channel reservation took place */
#endif

    int                 ring_buffer;   /* Flag to indicate ring DMA configuration */


    CAPH_DMA_CHANNEL_e  caph_dma_ch;   /* CAPH DMA Channel */
    CAPH_CFIFO_e        caph_cfifo_ch; /* CAPH CFIFO Channel */
    CAPH_SWITCH_CHNL_e  caph_switch_ch;/* CAPH Switch Channel */

} AADMA_Channel_t;


/****************************************************************************
*
*   The AADMA_Global_t contains all of the global state information used by
*   the AADMA code.
*
*   Callers which need to allocate a shared channel will be queued up
*   on the freeChannelQ until a channel becomes available.
*
*****************************************************************************/

typedef struct
{
    struct semaphore    lock;   /* acquired when manipulating table entries */
    wait_queue_head_t   freeChannelQ;

    AADMA_Channel_t     channel[ AADMA_NUM_CHANNELS ];

} AADMA_Global_t;


typedef struct
{
   CHAL_HANDLE         chalCfifoHandle;     /* CFIFO handle */
   CHAL_HANDLE         chalAadmacHandle;    /* AADMAC handle */
   CHAL_HANDLE         chalAhintcHandle;    /* AHINTC handle */
   CHAL_HANDLE         chalSsaswHandle;     /* SSASW handle */

   /* Add here till CCU framework is in place */
   CHAL_HANDLE         chalHubClkHandle;    /* Hub clock handle */

} AADMA_ChalHandle_t;

/* ---- Variable Externs ------------------------------------------------- */

extern  AADMA_DeviceAttribute_t    AADMA_gDeviceAttribute[ AADMA_NUM_DEVICE_ENTRIES ];

/* ---- Function Prototypes ---------------------------------------------- */

#if defined( __KERNEL__ )

/****************************************************************************/
/**
*   Initializes the AADMA module.
*
*   @return
*       0       - Success
*       < 0     - Error
*/
/****************************************************************************/

int aadma_init( void );

/****************************************************************************/
/**
*   Sets up the AADMA module.
*
*   @return
*       0       - Success
*       < 0     - Error
*/
/****************************************************************************/

int aadma_setup( void );

/****************************************************************************/
/**
*   Exits the AADMA module.
*
*   @return
*       0       - Success
*       < 0     - Error
*/
/****************************************************************************/

void aadma_exit( void );

#if ( AADMA_DEBUG_TRACK_RESERVATION )
AADMA_Handle_t aadma_request_channel_dbg( AADMA_Device_t dev, const char *fileName, int lineNum );
#define aadma_request_channel( dev )  aadma_request_channel_dbg( dev, __FILE__, __LINE__ )
#else

/****************************************************************************/
/**
*   Reserves a channel for use with @a dev. If the device is setup to use
*   a shared channel, then this function will block until a free channel
*   becomes available.
*
*   @return
*       >= 0    - A valid AADMA Handle.
*       -EBUSY  - Device is currently being used.
*       -ENODEV - Device handed in is invalid.
*/
/****************************************************************************/

AADMA_Handle_t aadma_request_channel
(
    AADMA_Device_t   dev    /* Device to use with the allocated channel. */
);
#endif

/****************************************************************************/
/**
*   Frees a previously allocated AADMA Handle.
*
*   @return
*        0      - DMA Handle was released successfully.
*       -EINVAL - Invalid DMA handle
*/
/****************************************************************************/

int aadma_free_channel
(
    AADMA_Handle_t    handle            /* AADMA handle. */
);

/****************************************************************************/
/**
*   Enables AADMA operation.
*
*   @return
*        0      - DMA Handle was released successfully.
*       -EINVAL - Invalid DMA handle
*/
/****************************************************************************/
int aadma_enable
(
    AADMA_Handle_t    handle            /* AADMA handle. */
);

/****************************************************************************/
/**
*   Sets the physical address to the CFIFO channel FIFO
*/
/****************************************************************************/
int aadma_set_device_fifo_address
(
   AADMA_Device_t    dev,
   uint32_t          dev_fifo_addr
);

/****************************************************************************/
/**
*   Sets the size to the CFIFO channel FIFO
*/
/****************************************************************************/
int aadma_set_device_fifo_size
(
   AADMA_Device_t    dev,
   uint16_t          fifo_size
);

/****************************************************************************/
/**
*   Sets the threshold values to the CFIFO channel FIFO
*/
/****************************************************************************/
int aadma_set_device_thres
(
   AADMA_Device_t    dev,
   uint16_t          thres1,
   uint16_t          thres2
);

/****************************************************************************/
/**
*   Allocates buffers for the descriptors. This is normally done automatically
*   but needs to be done explicitly when initiating an aadma from interrupt
*   context.
*
*   @return
*       0       Descriptors were allocated successfully
*       -EINVAL Invalid device type for this kind of transfer
*               (i.e. the device is _MEM_TO_DEV and not _DEV_TO_MEM)
*       -ENOMEM Memory exhausted
*/
/****************************************************************************/

int aadma_alloc_descriptors
(
    AADMA_Handle_t   handle,           /* AADMA Handle */
    dma_addr_t       data_addr,        /* Place to read/write data to/from device */
    size_t           numBytes          /* Number of bytes to transfer to the device */
);

/****************************************************************************/
/**
*   Allocates and sets up descriptors for a double buffered circular buffer
*   for destination data.
*
*   This is primarily intended to be used for things like the ingress samples
*   from a microphone.
*
*   @return
*       > 0     Number of descriptors actually allocated.
*       -EINVAL Invalid device type for this kind of transfer
*               (i.e. the device is _MEM_TO_DEV and not _DEV_TO_MEM)
*       -ENOMEM Memory exhausted
*/
/****************************************************************************/

int aadma_alloc_ring_descriptors
(
    AADMA_Handle_t   handle,           /* AADMA Handle */
    dma_addr_t       data_addr,        /* Place to read/write data to/from device */
    size_t           numBytes          /* Number of bytes in each destination buffer */
);

/****************************************************************************/
/**
*   Initiates a transfer when the descriptors have already been setup.
*
*   This is a special case, and normally, the aadma_transfer_xxx functions
*   should be used.
*
*   @return
*       0       Transfer was started successfully
*       -EINVAL Invalid handle
*/
/****************************************************************************/

int aadma_start_transfer( AADMA_Handle_t handle );

/****************************************************************************/
/**
*   Continues a ring transfer when the descriptors have already been setup.
*
*   This is a special case, and normally, the aadma_transfer_xxx functions
*   should be used.
*
*   @return
*       0       Transfer was started successfully
*       -EINVAL Invalid handle
*/
/****************************************************************************/

int aadma_continue_transfer( AADMA_Handle_t handle, int dma_status );


/****************************************************************************/
/**
*   Stops a previously started DMA transfer.
*
*   @return
*       0       Transfer was stopped successfully
*       -ENODEV Invalid handle
*/
/****************************************************************************/

int aadma_stop_transfer( AADMA_Handle_t handle );

/****************************************************************************/
/**
*   Initiates a DMA transfer
*
*   @return
*       0       Transfer was started successfully
*       -EINVAL Invalid device type for this kind of transfer
*               (i.e. the device is _MEM_TO_DEV and not _DEV_TO_MEM)
*/
/****************************************************************************/

int aadma_transfer
(
    AADMA_Handle_t          handle,         /* AADMA Handle */
    dma_addr_t              data_addr,      /* Place to read/write data to/from device */
    size_t                  numBytes        /* Number of bytes to transfer to the device */
);

/****************************************************************************/
/**
*   Set the callback function which will be called when a transfer completes.
*   If a NULL callback function is set, then no callback will occur.
*
*   @note   @a devHandler will be called from IRQ context.
*
*   @return
*       0       - Success
*       -ENODEV - Device handed in is invalid.
*/
/****************************************************************************/

int aadma_set_device_handler
(
    AADMA_Device_t         dev,           /* Device to set the callback for. */
    AADMA_DeviceHandler_t  devHandler,    /* Function to call when the DMA completes */
    void                   *userData      /* Pointer which will be passed to devHandler. */
);

/****************************************************************************/
/**
*   Obtains the physical address to the CFIFO channel FIFO
*/
/****************************************************************************/
uint32_t aadma_get_cfifo_address
(
   AADMA_Handle_t handle    /* DMA handle. */
);

#endif  /* defined( __KERNEL__ ) */

#endif  /* ASM_ARM_ARCH_BCMHANA_AADMA_H */

