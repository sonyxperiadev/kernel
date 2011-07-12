/*****************************************************************************
* Copyright 2004 - 2010 Broadcom Corporation.  All rights reserved.
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
*   @file   sdma.h
*
*   @brief  API definitions for the linux SDMA interface.
*/
/****************************************************************************/

#if !defined( ASM_ARM_ARCH_BCMHANA_SDMA_H )
#define ASM_ARM_ARCH_BCMHANA_SDMA_H

/* ---- Include Files ---------------------------------------------------- */

#include <linux/kernel.h>
#include <linux/wait.h>
#include <linux/semaphore.h>
#include <chal/chal_dma.h>
//#include <linux/broadcom/timer.h>
#include <linux/broadcom/dump-mem.h>
#include <linux/scatterlist.h>
#include <linux/dma-mapping.h>
#include <linux/mm.h>
#include <linux/vmalloc.h>
#include <linux/pagemap.h>
#include <mach/profile_timer.h>
#include <mach/dma_mmap.h>

typedef enum
{
    DMA_DEVICE_MEM_TO_MEM,              /* For memory to memory transfers (64 bit transaction) */
    DMA_DEVICE_MEM_TO_MEM_32,           /* For memory to memory transfers (32 bit transaction) */
    DMA_DEVICE_I2S0_DEV_TO_MEM,
    DMA_DEVICE_I2S0_MEM_TO_DEV,
    DMA_DEVICE_I2S1_DEV_TO_MEM,
    DMA_DEVICE_I2S1_MEM_TO_DEV,
    DMA_DEVICE_APM_CODEC_A_DEV_TO_MEM,
    DMA_DEVICE_APM_CODEC_A_MEM_TO_DEV,
    DMA_DEVICE_APM_CODEC_B_DEV_TO_MEM,
    DMA_DEVICE_APM_CODEC_B_MEM_TO_DEV,
    DMA_DEVICE_APM_CODEC_C_DEV_TO_MEM,  /* Additional mic input for beam-forming */
    DMA_DEVICE_APM_PCM0_DEV_TO_MEM,
    DMA_DEVICE_APM_PCM0_MEM_TO_DEV,
    DMA_DEVICE_APM_PCM1_DEV_TO_MEM,
    DMA_DEVICE_APM_PCM1_MEM_TO_DEV,
    DMA_DEVICE_SPUM_DEV_TO_MEM,
    DMA_DEVICE_SPUM_MEM_TO_DEV,
    DMA_DEVICE_SPIH_DEV_TO_MEM,
    DMA_DEVICE_SPIH_MEM_TO_DEV,
    DMA_DEVICE_UART_A_DEV_TO_MEM,
    DMA_DEVICE_UART_A_MEM_TO_DEV,
    DMA_DEVICE_UART_B_DEV_TO_MEM,
    DMA_DEVICE_UART_B_MEM_TO_DEV,
    DMA_DEVICE_PIF_MEM_TO_DEV,
    DMA_DEVICE_PIF_DEV_TO_MEM,
    DMA_DEVICE_ESW_DEV_TO_MEM,
    DMA_DEVICE_ESW_MEM_TO_DEV,
    DMA_DEVICE_VPM_MEM_TO_MEM,
    DMA_DEVICE_CLCD_MEM_TO_MEM,
    DMA_DEVICE_NAND_MEM_TO_MEM,
    DMA_DEVICE_MEM_TO_VRAM,
    DMA_DEVICE_VRAM_TO_MEM,
#define DMA_DEVICE_MEM_TO_MEM_0           DMA_DEVICE_MEM_TO_MEM
    DMA_DEVICE_MEM_TO_MEM_1,           /* Additional mem-to-mem devices */
    DMA_DEVICE_MEM_TO_MEM_2,
    DMA_DEVICE_MEM_TO_MEM_3,
    DMA_DEVICE_MEM_TO_MEM_4,
    DMA_DEVICE_MEM_TO_MEM_5,
    DMA_DEVICE_MEM_TO_MEM_6,
    DMA_DEVICE_MEM_TO_MEM_7,
#define DMA_DEVICE_MEM_TO_MEM_32_0        DMA_DEVICE_MEM_TO_MEM_32
    DMA_DEVICE_MEM_TO_MEM_32_1,
    DMA_DEVICE_MEM_TO_MEM_32_2,
    DMA_DEVICE_MEM_TO_MEM_32_3,
    DMA_DEVICE_MEM_TO_MEM_32_4,
    DMA_DEVICE_MEM_TO_MEM_32_5,
    DMA_DEVICE_MEM_TO_MEM_32_6,
    DMA_DEVICE_MEM_TO_MEM_32_7,
    DMA_DEVICE_MPHI_MEM_TO_DEV,
    DMA_DEVICE_MPHI_DEV_TO_MEM,
    DMA_DEVICE_SSP0A_RX0,
    DMA_DEVICE_SSP0B_TX0,
    DMA_DEVICE_SSP0C_RX1,
    DMA_DEVICE_SSP0D_TX1,
    DMA_DEVICE_SSP1A_RX0,
    DMA_DEVICE_SSP1B_TX0,
    DMA_DEVICE_SSP1C_RX1,
    DMA_DEVICE_SSP1D_TX1,
    DMA_DEVICE_SSP2A_RX0,
    DMA_DEVICE_SSP2B_TX0,
    DMA_DEVICE_SSP2C_RX1,
    DMA_DEVICE_SSP2D_TX1,
    DMA_DEVICE_SSP3A_RX0,
    DMA_DEVICE_SSP3B_TX0,
    DMA_DEVICE_SSP3C_RX1,
    DMA_DEVICE_SSP3D_TX1,
    DMA_DEVICE_SSP4A_RX0,
    DMA_DEVICE_SSP4B_TX0,
    DMA_DEVICE_SSP4C_RX1,
    DMA_DEVICE_SSP4D_TX1,
    DMA_DEVICE_AUDIOH_VIN_TO_MEM,
    DMA_DEVICE_AUDIOH_NVIN_TO_MEM,
    DMA_DEVICE_AUDIOH_MEM_TO_EARPIECE,
    DMA_DEVICE_AUDIOH_MEM_TO_HEADSET,
    DMA_DEVICE_AUDIOH_MEM_TO_HANDSFREE,
    DMA_DEVICE_AUDIOH_MEM_TO_VIBRA,

    /* Add new entries before this line. */

    DMA_NUM_DEVICE_ENTRIES,
    DMA_DEVICE_NONE = 0xff,    /* Special value to indicate that no device is currently assigned. */

} DMA_Device_t;

typedef enum
{
   DMA_UPDATE_MODE_NO_INC = 0,
   DMA_UPDATE_MODE_INC

} DMA_UpdateMode_t;

typedef void (*DMA_DeviceHandler_t)( DMA_Device_t dev, int reason, void *userData );

#define DMA_DEVICE_FLAG_ON_DMA0             0x00000001
#define DMA_DEVICE_FLAG_ON_DMA1             0x00000002
#define DMA_DEVICE_FLAG_PORT_PER_DMAC       0x00000004  /* If set, it means that the port used on DMAC0 is different from the port used on DMAC1 */
#define DMA_DEVICE_FLAG_ALLOC_DMA1_FIRST    0x00000008  /* If set, allocate from DMA1 before allocating from DMA0 */
#define DMA_DEVICE_FLAG_IS_DEDICATED        0x00000100
#define DMA_DEVICE_FLAG_NO_ISR              0x00000200
#define DMA_DEVICE_FLAG_ALLOW_LARGE_FIFO    0x00000400
#define DMA_DEVICE_FLAG_IN_USE              0x00000800  /* If set, device is in use on a channel */
#define DMA_DEVICE_FLAG_SECURE              0x00001000

#define DMA_CHANNEL_FLAG_IN_USE         0x00000001
#define DMA_CHANNEL_FLAG_IS_DEDICATED   0x00000002
#define DMA_CHANNEL_FLAG_NO_ISR         0x00000004
#define DMA_CHANNEL_FLAG_LARGE_FIFO     0x00000008

typedef enum
{
   dmacHw_INTERRUPT_STATUS_NONE  = 0x0,                                 /* No DMA interrupt */
   dmacHw_INTERRUPT_STATUS_TRANS = 0x1,                                 /* End of DMA transfer interrupt */
   dmacHw_INTERRUPT_STATUS_BLOCK = 0x2,                                 /* End of block transfer interrupt */
   dmacHw_INTERRUPT_STATUS_ERROR = 0x4                                  /* Error interrupt */
} dmacHw_INTERRUPT_STATUS_e;

#define DMA_HANDLER_REASON_BLOCK_COMPLETE       dmacHw_INTERRUPT_STATUS_BLOCK
#define DMA_HANDLER_REASON_TRANSFER_COMPLETE    dmacHw_INTERRUPT_STATUS_TRANS
#define DMA_HANDLER_REASON_ERROR                dmacHw_INTERRUPT_STATUS_ERROR

/****************************************************************************
*
*   The SDMA_Handle_t is the primary object used by callers of the API.
*
*****************************************************************************/

#define SDMA_INVALID_HANDLE  ((SDMA_Handle_t) -1)
typedef     int SDMA_Handle_t;
#define DMA_INVALID_HANDLE  ((DMA_Handle_t) -1)
typedef     int DMA_Handle_t;

/* ---- Constants and Types ---------------------------------------------- */

/*
 * If SDMA_DEBUG_TRACK_RESERVATION is set to a non-zero value, then the filename 
 * and line number of the reservation request will be recorded in the channel table 
 */

#define SDMA_DEBUG_TRACK_RESERVATION   0

/* Number of System DMA channels */
#define SDMA_NUM_CHANNELS     CHAL_TOTAL_DMA_CHANNELS   

/****************************************************************************
*
*   The SDMA_DescriptorRing_t contains a ring of descriptors which is used
*   to point to regions of memory.
*
*****************************************************************************/

typedef struct
{
    void       *virtAddr;               /* Virtual Address of the descriptor ring */
    dma_addr_t  physAddr;               /* Physical address of the descriptor ring */
    int         descriptorsAllocated;   /* Number of descriptors allocated in the descriptor ring */
    size_t      bytesAllocated;         /* Number of bytes allocated in the descriptor ring */

} SDMA_DescriptorRing_t;


/****************************************************************************
*
*   The DMA_DeviceAttribute_t contains information which describes a 
*   particular DMA device (or peripheral).
*
*   It is anticipated that the arrary of DMA_DeviceAttribute_t's will be 
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
    uint32_t               flags;              /* Bitmask of DMA_DEVICE_FLAG_xxx constants */
    const char             *name;              /* Will show up in the /proc entry */

    CHAL_DMA_CHAN_CONFIG_t config;             /* Configuration to use when DMA'ing using this device */
    CHAL_DMA_PERIPHERAL_t  peripheralId;       /* Peripheral ID */

    void                   *userData;          /* Passed to the devHandler */
    DMA_DeviceHandler_t    devHandler;         /* Called when DMA operations finish. */
    timer_tick_count_t     transferStartTime;  /* Time the current transfer was started */

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

    /*
     * It's not possible to free memory which is allocated for the descriptors from within 
     * the ISR. So make the presumption that a given device will tend to use the 
     * same sized buffers over and over again, and we keep them around. 
     */

    SDMA_DescriptorRing_t   ring;       /* Ring of descriptors allocated for this device */

    /*
     * We stash away some of the information from the previous transfer. If back-to-back 
     * transfers are performed from the same buffer, then we don't have to keep re-initializing 
     * the descriptor buffers. 
     */

    uint32_t            prevNumBytes;
    dma_addr_t          prevSrcData;
    dma_addr_t          prevDstData;

    int                 initDesc;
    int                 descUse; 

} SDMA_DeviceAttribute_t;


/****************************************************************************
*
*   DMA_Channel_t, DMA_Controller_t, and DMA_State_t are really internal
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
    uint32_t        flags;      /* bitmask of DMA_CHANNEL_FLAG_xxx constants */
    DMA_Device_t    devType;    /* Device this channel is currently reserved for */
    DMA_Device_t    lastDevType;/* Device type that used this previously */
    char            name[ 20 ]; /* Name passed onto request_irq */

#if ( SDMA_DEBUG_TRACK_RESERVATION )
    const char     *fileName;   /* Place where channel reservation took place */
    int             lineNum;    /* Place where channel reservation took place */
#endif
    CHAL_CHANNEL_HANDLE sdmacHandle; /* low level channel handle. */

} SDMA_Channel_t;


/****************************************************************************
*
*   The DMA_Global_t contains all of the global state information used by
*   the DMA code.
*
*   Callers which need to allocate a shared channel will be queued up
*   on the freeChannelQ until a channel becomes available. 
*
*****************************************************************************/

typedef struct
{
    struct semaphore    lock;   /* acquired when manipulating table entries */
    wait_queue_head_t   freeChannelQ;

    SDMA_Channel_t      channel[ SDMA_NUM_CHANNELS ];

} SDMA_Global_t;

/* ---- Variable Externs ------------------------------------------------- */

extern  SDMA_DeviceAttribute_t    SDMA_gDeviceAttribute[ DMA_NUM_DEVICE_ENTRIES ];

/* ---- Function Prototypes ---------------------------------------------- */

#if defined( __KERNEL__ )

/****************************************************************************/
/**
*   Initializes the DMA module.
*
*   @return
*       0       - Success
*       < 0     - Error
*/
/****************************************************************************/

int sdma_init( void );

#if ( SDMA_DEBUG_TRACK_RESERVATION )
SDMA_Handle_t sdma_request_channel_dbg( DMA_Device_t dev, const char *fileName, int lineNum );
#define sdma_request_channel( dev )  sdma_request_channel_dbg( dev, __FILE__, __LINE__ )
#else

/****************************************************************************/
/**
*   Reserves a channel for use with @a dev. If the device is setup to use
*   a shared channel, then this function will block until a free channel
*   becomes available.
*
*   @return
*       >= 0    - A valid DMA Handle.
*       -EBUSY  - Device is currently being used.
*       -ENODEV - Device handed in is invalid.
*/
/****************************************************************************/

SDMA_Handle_t sdma_request_channel
(
    DMA_Device_t dev    /* Device to use with the allocated channel. */
);
#endif

/****************************************************************************/
/**
*   Frees a previously allocated DMA Handle.
*
*   @return
*        0      - DMA Handle was released successfully.
*       -EINVAL - Invalid DMA handle
*/
/****************************************************************************/

int sdma_free_channel
(
    SDMA_Handle_t    handle            /* DMA handle. */
);

/****************************************************************************/
/**
*   Allocates buffers for the descriptors. This is normally done automatically
*   but needs to be done explicitly when initiating a dma from interrupt
*   context.
*
*   @return
*       0       Descriptors were allocated successfully
*       -EINVAL Invalid device type for this kind of transfer
*               (i.e. the device is _MEM_TO_DEV and not _DEV_TO_MEM)
*       -ENOMEM Memory exhausted
*/
/****************************************************************************/

int sdma_alloc_descriptors
(
    SDMA_Handle_t    handle,           /* DMA Handle */
    dma_addr_t       srcData,          /* Place to get data to write to device */
    dma_addr_t       dstData,          /* Pointer to device data address */
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

int sdma_alloc_double_dst_descriptors
(
    SDMA_Handle_t    handle,           /* DMA Handle */
    dma_addr_t       srcData,          /* Physical address of source data */
    dma_addr_t       dstData1,         /* Physical address of first destination buffer */
    dma_addr_t       dstData2,         /* Physical address of second destination buffer */
    size_t           numBytes          /* Number of bytes in each destination buffer */
);

/****************************************************************************/
/**
*   Allocates and sets up descriptors for a double buffered circular buffer
*   for source data.
*
*   This is primarily intended to be used for things like the egress samples
*   to a speaker.
*
*   @return
*       > 0     Number of descriptors actually allocated.
*       -EINVAL Invalid device type for this kind of transfer
*               (i.e. the device is _MEM_TO_DEV and not _DEV_TO_MEM)
*       -ENOMEM Memory exhausted
*/
/****************************************************************************/

int sdma_alloc_double_src_descriptors
(
    SDMA_Handle_t    handle,           /* DMA Handle */
    dma_addr_t       srcData1,         /* Physical address of first source buffer */
    dma_addr_t       srcData2,         /* Physical address of second source buffer */
    dma_addr_t       dstData,          /* Physical address of destination data */
    size_t           numBytes          /* Number of bytes in each source buffer */
);

/****************************************************************************/
/**
*   Creates a descriptor ring from a memory mapping.
*
*   @return 0 on sucess, error code otherwise.
*/
/****************************************************************************/

int sdma_map_create_descriptor_ring
(
    SDMA_Handle_t    handle,           /* DMA Handle */ /* FIXME: should be device DMA_Device_t*/
    DMA_MMAP_CFG_T   *memMap,          /* Memory map that will be used */
    dma_addr_t       devPhysAddr,      /* Physical address of device */
    DMA_UpdateMode_t updateMode        /* Address update mode for device */
);

/****************************************************************************/
/**
*   Creates a descriptor ring from a memory mapping with a command to a device.
*
*   @return 0 on sucess, error code otherwise.
*/
/****************************************************************************/

int sdma_map_create_descriptor_ring_with_command
(
   SDMA_Handle_t           handle,
   DMA_MMAP_CFG_T         *memMap,              /* Memory map that will be used */
   dma_addr_t              commandSrcPhysAddr,  /* Physical address of the command source */
   dma_addr_t              commandRegPhysAddr,  /* Physical address of the command register */
   dma_addr_t              devPhysAddr,         /* Physical address of device */
   DMA_UpdateMode_t        updateMode
);


/****************************************************************************/
/**
*   Initiates a transfer when the descriptors have already been setup.
*
*   This is a special case, and normally, the sdma_transfer_xxx functions 
*   should be used.
*
*   @return
*       0       Transfer was started successfully
*       -EINVAL Invalid handle
*/
/****************************************************************************/

int sdma_start_transfer( SDMA_Handle_t handle );

/****************************************************************************/
/**
*   Stops a previously started DMA transfer.
*
*   @return
*       0       Transfer was stopped successfully
*       -ENODEV Invalid handle
*/
/****************************************************************************/

int sdma_stop_transfer( SDMA_Handle_t handle );

/****************************************************************************/
/**
*   Waits for a DMA to complete by polling. This function is only intended
*   to be used for testing. Interrupts should be used for most DMA operations.
*/
/****************************************************************************/

int sdma_wait_transfer_done( SDMA_Handle_t handle );

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

int sdma_transfer
(
    SDMA_Handle_t           handle,         /* DMA Handle */
    dma_addr_t              srcData,        /* Place to get data to write to device */
    dma_addr_t              dstData,        /* Pointer to device data address */
    size_t                  numBytes        /* Number of bytes to transfer to the device */
);

/****************************************************************************/
/**
*   Initiates a memory to memory transfer.
*
*   @return
*       0       Transfer was started successfully
*       -EINVAL Invalid device type for this kind of transfer
*               (i.e. the device wasn't DMA_DEVICE_MEM_TO_MEM)
*/
/****************************************************************************/

static inline int sdma_transfer_mem_to_mem
(
    SDMA_Handle_t           handle,         /* DMA Handle */
    dma_addr_t              srcData,        /* Place to get data to write to device */
    dma_addr_t              dstData,        /* Pointer to device data address */
    size_t                  numBytes        /* Number of bytes to transfer to the device */
)
{
   return sdma_transfer( handle, srcData, dstData, numBytes );
}

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

int sdma_set_device_handler
(
    DMA_Device_t         dev,           /* Device to set the callback for. */
    DMA_DeviceHandler_t  devHandler,    /* Function to call when the DMA completes */
    void                 *userData      /* Pointer which will be passed to devHandler. */
);

/****************************************************************************/
/**
*   Dump debug information for a DMA channel (debug only, console dump)
*
*   @return
*       0       Success
*       -ENODEV - Device handed in is invalid.
*/
/****************************************************************************/

int sdma_dump_debug_info( SDMA_Handle_t handle );

#endif  /* defined( __KERNEL__ ) */

#endif  /* ASM_ARM_ARCH_BCMHANA_SDMA_H */

