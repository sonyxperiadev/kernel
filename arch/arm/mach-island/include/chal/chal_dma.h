/*****************************************************************************
*  Copyright 2001 - 2009 Broadcom Corporation.  All rights reserved.
*
*  Unless you and Broadcom execute a separate written software license
*  agreement governing use of this software, this software is licensed to you
*  under the terms of the GNU General Public License version 2, available at
*  http://www.gnu.org/licenses/old-license/gpl-2.0.html (the "GPL").
*
*  Notwithstanding the above, under no circumstances may you combine this
*  software in any way with any other Broadcom software provided under a
*  license other than the GPL, without Broadcom's express prior written
*  consent.
*
*****************************************************************************/
/**
*
*  @file   chal_dma.h
*  @brief  DMA cHAL interface
*  @note
*
*****************************************************************************/
#ifndef	_CHAL_DMA_H_
#define	_CHAL_DMA_H_

#include <plat/chal/chal_types.h>
#include <chal/chal_dma_reg.h>
#include <chal/chal_dmux.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void* CHAL_CHANNEL_HANDLE;

/**
* defines
******************************************************************************/

/**
* DMA channel
******************************************************************************/
typedef enum
{
   CHAL_DMA_CHANNEL_0 = 0,
   CHAL_DMA_CHANNEL_1,
   CHAL_DMA_CHANNEL_2,
   CHAL_DMA_CHANNEL_3,
   CHAL_DMA_CHANNEL_4,
   CHAL_DMA_CHANNEL_5,
   CHAL_DMA_CHANNEL_6,
   CHAL_DMA_CHANNEL_7,
   CHAL_DMA_CHANNEL_NULL = 8,
   CHAL_TOTAL_DMA_CHANNELS = 8
} CHAL_DMA_CHANNEL_t;

/**
* DMA secure state
******************************************************************************/
typedef enum {
   CHAL_DMA_STATE_SECURE  = 0x00,            
   CHAL_DMA_STATE_OPEN    = 0x01             
} CHAL_DMA_SECURE_STATE_t;

/**
* DMA events used for channel synchronization 8-31
******************************************************************************/
typedef enum {
    CHAL_DMA_EVENT_MIN      = 8,
    CHAL_DMA_EVENT_8        = 8,
    CHAL_DMA_EVENT_9        = 9,
    CHAL_DMA_EVENT_10       = 10,
    CHAL_DMA_EVENT_11       = 11,
    CHAL_DMA_EVENT_12       = 12,
    CHAL_DMA_EVENT_13       = 13,
    CHAL_DMA_EVENT_14       = 14,
    CHAL_DMA_EVENT_15       = 15,
    CHAL_DMA_EVENT_16       = 16,
    CHAL_DMA_EVENT_17       = 17,
    CHAL_DMA_EVENT_18       = 18,
    CHAL_DMA_EVENT_19       = 19,
    CHAL_DMA_EVENT_20       = 20,
    CHAL_DMA_EVENT_21       = 21,
    CHAL_DMA_EVENT_22       = 22,
    CHAL_DMA_EVENT_23       = 23,
    CHAL_DMA_EVENT_24       = 24,
    CHAL_DMA_EVENT_25       = 25,
    CHAL_DMA_EVENT_26       = 26,
    CHAL_DMA_EVENT_27       = 27,
    CHAL_DMA_EVENT_28       = 28,
    CHAL_DMA_EVENT_29       = 29,
    CHAL_DMA_EVENT_30       = 30,
    CHAL_DMA_EVENT_31       = 31,
    CHAL_DMA_EVENT_MAX      = 31,
    CHAL_DMA_EVENT_INVALID  = 0xFF
} CHAL_DMA_EVENT_t;

/**
* DMA burst len
******************************************************************************/
typedef enum {
    CHAL_DMA_BURST_LEN_1  = 0x00,             
    CHAL_DMA_BURST_LEN_2  = 0x01,             
    CHAL_DMA_BURST_LEN_3  = 0x02,             
    CHAL_DMA_BURST_LEN_4  = 0x03,             
    CHAL_DMA_BURST_LEN_5  = 0x04,             
    CHAL_DMA_BURST_LEN_6  = 0x05,             
    CHAL_DMA_BURST_LEN_7  = 0x06,             
    CHAL_DMA_BURST_LEN_8  = 0x07,             
    CHAL_DMA_BURST_LEN_9  = 0x08,             
    CHAL_DMA_BURST_LEN_10 = 0x09,             
    CHAL_DMA_BURST_LEN_11 = 0x0A,             
    CHAL_DMA_BURST_LEN_12 = 0x0B,             
    CHAL_DMA_BURST_LEN_13 = 0x0C,             
    CHAL_DMA_BURST_LEN_14 = 0x0D,             
    CHAL_DMA_BURST_LEN_15 = 0x0E,             
    CHAL_DMA_BURST_LEN_16 = 0x0F              
} CHAL_DMA_BURST_LEN_t;

/**
* DMA burst size
******************************************************************************/
typedef enum {
    CHAL_DMA_BURST_SIZE_1_BYTE    = 0x00,     
    CHAL_DMA_BURST_SIZE_2_BYTES   = 0x01,     
    CHAL_DMA_BURST_SIZE_4_BYTES   = 0x02,     
    CHAL_DMA_BURST_SIZE_8_BYTES   = 0x03      
}CHAL_DMA_BURST_SIZE_t;



typedef enum {
    CHAL_DMA_PERIPHERAL_UART0_R      = CHAL_DMUX_EPT_UARTB0_A,
    CHAL_DMA_PERIPHERAL_UART0_W      = CHAL_DMUX_EPT_UARTB0_B,
    CHAL_DMA_PERIPHERAL_UART1_R      = CHAL_DMUX_EPT_UARTB1_A,
    CHAL_DMA_PERIPHERAL_UART1_W      = CHAL_DMUX_EPT_UARTB1_B,
    CHAL_DMA_PERIPHERAL_UART2_R      = CHAL_DMUX_EPT_UARTB2_A,
    CHAL_DMA_PERIPHERAL_UART2_W      = CHAL_DMUX_EPT_UARTB2_B,
    CHAL_DMA_PERIPHERAL_UART3_R      = CHAL_DMUX_EPT_UARTB3_A,
    CHAL_DMA_PERIPHERAL_UART3_W      = CHAL_DMUX_EPT_UARTB3_B,
    CHAL_DMA_PERIPHERAL_UART4_R      = CHAL_DMUX_EPT_UARTB4_A,
    CHAL_DMA_PERIPHERAL_UART4_W      = CHAL_DMUX_EPT_UARTB4_B,
    CHAL_DMA_PERIPHERAL_UART5_R      = CHAL_DMUX_EPT_UARTB5_A,
    CHAL_DMA_PERIPHERAL_UART5_W      = CHAL_DMUX_EPT_UARTB5_B,
    CHAL_DMA_PERIPHERAL_PWM_0        = CHAL_DMUX_EPT_PWM_0,
    CHAL_DMA_PERIPHERAL_PWM_1        = CHAL_DMUX_EPT_PWM_1,
    CHAL_DMA_PERIPHERAL_PWM_2        = CHAL_DMUX_EPT_PWM_2,
    CHAL_DMA_PERIPHERAL_PWM_3        = CHAL_DMUX_EPT_PWM_3,
    CHAL_DMA_PERIPHERAL_PWM_4        = CHAL_DMUX_EPT_PWM_4,
    CHAL_DMA_PERIPHERAL_PWM_5        = CHAL_DMUX_EPT_PWM_5,
    CHAL_DMA_PERIPHERAL_SSP_0A_RX0   = CHAL_DMUX_EPT_SSP_0A_RX0,
    CHAL_DMA_PERIPHERAL_SSP_0B_TX0   = CHAL_DMUX_EPT_SSP_0B_TX0,
    CHAL_DMA_PERIPHERAL_SSP_0C_RX1   = CHAL_DMUX_EPT_SSP_0C_RX1,
    CHAL_DMA_PERIPHERAL_SSP_0D_TX1   = CHAL_DMUX_EPT_SSP_0D_TX1,
    CHAL_DMA_PERIPHERAL_SSP_1A_RX0   = CHAL_DMUX_EPT_SSP_1A_RX0,
    CHAL_DMA_PERIPHERAL_SSP_1B_TX0   = CHAL_DMUX_EPT_SSP_1B_TX0,
    CHAL_DMA_PERIPHERAL_SSP_1C_RX1   = CHAL_DMUX_EPT_SSP_1C_RX1,
    CHAL_DMA_PERIPHERAL_SSP_1D_TX1   = CHAL_DMUX_EPT_SSP_1D_TX1,
    CHAL_DMA_PERIPHERAL_HSI_R0       = CHAL_DMUX_EPT_HSIA,
    CHAL_DMA_PERIPHERAL_HSI_W0       = CHAL_DMUX_EPT_HSIC,
    CHAL_DMA_PERIPHERAL_HSI_R1       = CHAL_DMUX_EPT_HSIB,
    CHAL_DMA_PERIPHERAL_HSI_W1       = CHAL_DMUX_EPT_HSID,
    CHAL_DMA_PERIPHERAL_EANC         = CHAL_DMUX_EPT_EANC,
    CHAL_DMA_PERIPHERAL_STEREO       = CHAL_DMUX_EPT_STEREO,
    CHAL_DMA_PERIPHERAL_NVIN         = CHAL_DMUX_EPT_NVIN,
    CHAL_DMA_PERIPHERAL_VIN          = CHAL_DMUX_EPT_VIN,
    CHAL_DMA_PERIPHERAL_VIBRA        = CHAL_DMUX_EPT_VIBRA,
    CHAL_DMA_PERIPHERAL_IHF_0        = CHAL_DMUX_EPT_IHF_0,
    CHAL_DMA_PERIPHERAL_VOUT         = CHAL_DMUX_EPT_VOUT,
    CHAL_DMA_PERIPHERAL_SLIM_R       = CHAL_DMUX_EPT_SLIMA,
    CHAL_DMA_PERIPHERAL_SLIM_W       = CHAL_DMUX_EPT_SLIMB,
    CHAL_DMA_PERIPHERAL_SLIMC        = CHAL_DMUX_EPT_SLIMC,
    CHAL_DMA_PERIPHERAL_SLIMD        = CHAL_DMUX_EPT_SLIMD,
    CHAL_DMA_PERIPHERAL_SIM_R        = CHAL_DMUX_EPT_SIM_A,
    CHAL_DMA_PERIPHERAL_SIM_W        = CHAL_DMUX_EPT_SIM_B,
    CHAL_DMA_PERIPHERAL_SIM2_R       = CHAL_DMUX_EPT_SIM2_A,
    CHAL_DMA_PERIPHERAL_SIM2_W       = CHAL_DMUX_EPT_SIM2_B,
    CHAL_DMA_PERIPHERAL_IHF_1        = CHAL_DMUX_EPT_IHF_1,
    CHAL_DMA_PERIPHERAL_SSP_2A_RX0   = CHAL_DMUX_EPT_SSP_2A_RX0,
    CHAL_DMA_PERIPHERAL_SSP_2B_TX0   = CHAL_DMUX_EPT_SSP_2B_TX0,
    CHAL_DMA_PERIPHERAL_SSP_2C_RX1   = CHAL_DMUX_EPT_SSP_2C_RX1,
    CHAL_DMA_PERIPHERAL_SSP_2D_TX1   = CHAL_DMUX_EPT_SSP_2D_TX1,
    CHAL_DMA_PERIPHERAL_SSP_3A_RX0   = CHAL_DMUX_EPT_SSP_3A_RX0,
    CHAL_DMA_PERIPHERAL_SSP_3B_TX0   = CHAL_DMUX_EPT_SSP_3B_TX0,
    CHAL_DMA_PERIPHERAL_SSP_3C_RX1   = CHAL_DMUX_EPT_SSP_3C_RX1,
    CHAL_DMA_PERIPHERAL_SSP_3D_TX1   = CHAL_DMUX_EPT_SSP_3D_TX1,
    CHAL_DMA_PERIPHERAL_SSP_4A_RX0   = CHAL_DMUX_EPT_SSP_4A_RX0,
    CHAL_DMA_PERIPHERAL_SSP_4B_TX0   = CHAL_DMUX_EPT_SSP_4B_TX0,
    CHAL_DMA_PERIPHERAL_SSP_4C_RX1   = CHAL_DMUX_EPT_SSP_4C_RX1,
    CHAL_DMA_PERIPHERAL_SSP_4D_TX1   = CHAL_DMUX_EPT_SSP_4D_TX1,
    CHAL_DMA_PERIPHERAL_SSP_5A_RX0   = CHAL_DMUX_EPT_SSP_5A_RX0,
    CHAL_DMA_PERIPHERAL_SSP_5B_TX0   = CHAL_DMUX_EPT_SSP_5B_TX0,
    CHAL_DMA_PERIPHERAL_SSP_5C_RX1   = CHAL_DMUX_EPT_SSP_5C_RX1,
    CHAL_DMA_PERIPHERAL_SSP_5D_TX1   = CHAL_DMUX_EPT_SSP_5D_TX1,
    CHAL_DMA_PERIPHERAL_SSP_6A_RX0   = CHAL_DMUX_EPT_SSP_6A_RX0,
    CHAL_DMA_PERIPHERAL_SSP_6B_TX0   = CHAL_DMUX_EPT_SSP_6B_TX0,
    CHAL_DMA_PERIPHERAL_SSP_6C_RX1   = CHAL_DMUX_EPT_SSP_6C_RX1,
    CHAL_DMA_PERIPHERAL_SSP_6D_TX1   = CHAL_DMUX_EPT_SSP_6D_TX1,
    CHAL_DMA_PERIPHERAL_SPUM_SecureW = CHAL_DMUX_EPT_SPUM_SecureA,
    CHAL_DMA_PERIPHERAL_SPUM_SecureR = CHAL_DMUX_EPT_SPUM_SecureB,
    CHAL_DMA_PERIPHERAL_SPUM_OpenW   = CHAL_DMUX_EPT_SPUM_OpenA,
    CHAL_DMA_PERIPHERAL_SPUM_OpenR   = CHAL_DMUX_EPT_SPUM_OpenB,
    CHAL_DMA_PERIPHERAL_MPHI         = CHAL_DMUX_EPT_MPHI,
    CHAL_DMA_PERIPHERAL_CIR_RX       = CHAL_DMUX_EPT_CIR_RX,
    CHAL_DMA_PERIPHERAL_CIR_TX       = CHAL_DMUX_EPT_CIR_TX,
    CHAL_DMA_PERIPHERAL_I2S_A        = CHAL_DMUX_EPT_I2SA,
    CHAL_DMA_PERIPHERAL_I2S_B        = CHAL_DMUX_EPT_I2SB,
    CHAL_DMA_PERIPHERAL_INVALID      = CHAL_DMUX_EPT_INVALID
} CHAL_DMA_PERIPHERAL_t;


/**
* DMA status
******************************************************************************/
typedef enum {
   CHAL_DMA_STATUS_SUCCESS            = CHAL_DMA_REG_CHAN_STATUS_STOPPED,              
   CHAL_DMA_STATUS_FAULT              = CHAL_DMA_REG_CHAN_STATUS_FAULT,                
   CHAL_DMA_STATUS_RUNNING            = CHAL_DMA_REG_CHAN_STATUS_EXECUTING,            
   CHAL_DMA_STATUS_BUSY               = CHAL_DMA_REG_CHAN_STATUS_UPDATE_PC,            
   CHAL_DMA_STATUS_CACHE_MISS         = CHAL_DMA_REG_CHAN_STATUS_CACHE_MISS,           
   CHAL_DMA_STATUS_WAITING            = CHAL_DMA_REG_CHAN_STATUS_WAIT_FOR_EVENT,       
   CHAL_DMA_STATUS_WAITING_PERI       = CHAL_DMA_REG_CHAN_STATUS_WAIT_FOR_PERIPHERAL,  
   CHAL_DMA_STATUS_QUEUE_BUSY         = CHAL_DMA_REG_CHAN_STATUS_Q_BUSY,               
   CHAL_DMA_STATUS_INVALID_PARAMETER  = 0xFFFFFFF0,                                    
   CHAL_DMA_STATUS_INVALID_STATE      = 0xFFFFFFF1,                                    
   CHAL_DMA_STATUS_OVERFLOW           = 0xFFFFFFF2,                                    
   CHAL_DMA_STATUS_FAILURE            = 0xFFFFFFF3,
   CHAL_DMA_STATUS_TIMEOUT            = 0xFFFFFFF4
} CHAL_DMA_STATUS_t;

/**
* DMA flush operation
******************************************************************************/
typedef enum {
   CHAL_DMA_FLUSH_ALWAYS      = 0x00000000,          // Always flush before processing any descriptor
   CHAL_DMA_FLUSH_FIRST       = 0x00000001,          // Flush at the begining of DMA oepration
   CHAL_DMA_FLUSH_LAST        = 0x00000002,          // Flush at the end of DMA oepration
   CHAL_DMA_FLUSH_CMD         = 0x00000004,          // Flush before processing data descriptor
   CHAL_DMA_FLUSH_NEVER       = 0x80000000           // Never Flush 
} CHAL_DMA_FLUSH_t;


#define CHAL_DMA_STATUS_TRANSFER_FAIL(x)  (((x) == CHAL_DMA_REG_CHAN_STATUS_FAULT_COMPLETING) || \
                                           ((x) == CHAL_DMA_REG_CHAN_STATUS_FAULT))

/**
* DMA capabilities
******************************************************************************/
typedef struct
{
   uint32_t             numOfChannel;
   uint32_t             maxBurstSize;
   uint32_t             maxBurstLen;
} CHAL_DMA_CAPABILITIES_t;


typedef enum
{
   CHAL_DMA_ENDPOINT_PERIPHERAL = 0,
   CHAL_DMA_ENDPOINT_MEMORY = 1
}CHAL_DMA_ENDPOINT_t;


typedef enum
{
  CHAL_DMA_DESC_LIST = 0,
  CHAL_DMA_DESC_RING
}CHAL_DMA_DESC_TYPE_t;


/**
* DMA channel control
******************************************************************************/
typedef struct
{
   CHAL_DMA_BURST_LEN_t    dstBurstLen;
   CHAL_DMA_BURST_SIZE_t   dstBurstSize;
   CHAL_DMA_ENDPOINT_t     dstEndpoint;
   CHAL_DMA_BURST_LEN_t    srcBurstLen;
   CHAL_DMA_BURST_SIZE_t   srcBurstSize;
   CHAL_DMA_ENDPOINT_t     srcEndpoint;
   CHAL_DMA_EVENT_t        event_wait;
   CHAL_DMA_EVENT_t        event_send;
   CHAL_DMA_DESC_TYPE_t    descType;
   _Bool                   interruptDisable;
   _Bool                   alwaysBurst;
   CHAL_DMA_FLUSH_t        flushMode;
} CHAL_DMA_CHAN_CONFIG_t;


/**
*
*  @brief   Initialize DMA hardware and software interface.
*
*  @param   baseAddress (in) mapped address of DMA controller
*  @param   secureState (in) DMA secure state
*
*  @return  Handle of this DMA instance
*
*  @note
*
******************************************************************************/
CHAL_HANDLE chal_dma_init
(
   CHAL_DMA_SECURE_STATE_t  secureState
);


/**
*
* @brief         Channel level configuration
*
*
******************************************************************************/
CHAL_CHANNEL_HANDLE chal_dma_config_channel
(
   CHAL_HANDLE              handle,
   CHAL_DMA_CHANNEL_t       channel,
   CHAL_DMA_CHAN_CONFIG_t*  config
);



/**
*
*  @brief   Channel level memory configuration
*
*  @param   handle   (in) Handle returned in chal_dma_config_channel()
*  @param   pDevNode (in) Allocated physical memory information for DMA node
*
*  @return  DMA status
*
*  @note    DMA mode memory and instruction memory are allocated by driver and
*           passed to cHAL in This function.
******************************************************************************/
void chal_dma_config_channel_memory
(
   CHAL_CHANNEL_HANDLE handle,
   uint32_t virtualAddr,
   uint32_t physicalAddr,
   uint32_t descCount
);


/**
*
*  @brief
*
*  @param
*  @param
*
*  @return
*
*  @note
*
******************************************************************************/
void chal_dma_clear_int_status
(
   CHAL_CHANNEL_HANDLE handle
);

/**
*
*  @brief   Get individual capability of the device
*
*  @param   handle   (in) Handle returned in chal_dma_init()
*  @param   pCap     (out) Pointer to capabilities
*
*  @return  DMA status
*
*  @note
******************************************************************************/
void chal_dma_get_capabilities
(
   CHAL_HANDLE              handle,
   CHAL_DMA_CAPABILITIES_t  *pCap
);

/**
*
*  @brief   Get total number of descriptors added towards specified channel
*
*  @param   handle   (in) Handle returned in chal_dma_config_channel()
*
*  @return  Number of descriptors allocated towards channel
*
*  @note
******************************************************************************/
uint32_t chal_dma_get_channel_descriptor_count
(
   CHAL_CHANNEL_HANDLE handle
);

/**
*
*  @brief   Get current descriptor index in use for channel
*
*  @param   handle   (in) Handle returned in chal_dma_config_channel()
*
*  @return  Index of current descriptor in use
*
*  @note
******************************************************************************/
int chal_dma_get_current_channel_descriptor_index
(
   CHAL_CHANNEL_HANDLE handle,
   uint32_t *desc_idx
);


/**
*
*  @brief   Terminate a DMA transfer gracefully without data lose
*
*  @param   handle   (in) Handle returned in chal_dma_config_channel()
*  @param   channel  (in) DMA channel
*
*  @return  DMA status
*
*  @note
******************************************************************************/
void chal_dma_shutdown_channel
(
   CHAL_CHANNEL_HANDLE handle
);



/**
*
*  @brief
*
*  @param
*  @param
*
*  @return
*
*  @note
*
******************************************************************************/
CHAL_DMA_STATUS_t chal_dma_get_channel
(
    CHAL_HANDLE        handle,
    CHAL_DMA_CHANNEL_t *channel
);


/**
*
*  @brief
*
*  @param
*  @param
*
*  @return
*
*  @note
*
******************************************************************************/
CHAL_DMA_STATUS_t chal_dma_connect_peripheral
(
   CHAL_CHANNEL_HANDLE    handle,
   CHAL_DMA_PERIPHERAL_t  peripheral
);



/**
*
*  @brief
*
*  @param
*  @param
*
*  @return
*
*  @note
*
******************************************************************************/
void chal_dma_disconnect_peripheral
(
   CHAL_CHANNEL_HANDLE handle
);


/**
*
*  @brief   Free the channel previously allocated
*
*  @param   handle   (in) Handle returned in chal_dma_config_channel()
*  @param   channel  (in) DMA channel to be freed
*
*  @return  DMA status
*
*  @note
******************************************************************************/
void chal_dma_release_channel
(
   CHAL_CHANNEL_HANDLE handle
);


/**
*
*  @brief
*
*  @param
*  @param
*
*  @return
*
*  @note
*
******************************************************************************/
uint32_t chal_dma_get_burst_length
(
   CHAL_CHANNEL_HANDLE handle
);


/**
*
*  @brief
*
*  @param
*  @param
*
*  @return
*
*  @note
*
******************************************************************************/
uint32_t chal_dma_calculate_max_data_per_descriptor
(
   CHAL_CHANNEL_HANDLE handle
);



/**
*
*  @brief
*
*  @param
*  @param
*
*  @return
*
*  @note
*
******************************************************************************/
uint32_t  chal_dma_calculate_channel_memory
(
   CHAL_CHANNEL_HANDLE handle,
   uint32_t             descCount
);






/**
*
*  @brief
*
*  @param
*  @param
*
*  @return
*
*  @note
*
******************************************************************************/
int chal_dma_prepare_transfer
(
   CHAL_CHANNEL_HANDLE handle
);


/**
*
*  @brief
*
*  @param
*  @param
*
*  @return
*
*  @note
*
******************************************************************************/
CHAL_DMA_STATUS_t chal_dma_start_transfer
(
   CHAL_CHANNEL_HANDLE handle
);


/**
*
*  @brief   Add descriptor to the channel
*
*  @param   handle   (in) Handle returned in chal_dma_config_channel()
*  @param   channel  (in) DMA channel
*
*  @return  DMA status
*
*  @note
******************************************************************************/
CHAL_DMA_STATUS_t chal_dma_add_descriptor
(
   CHAL_CHANNEL_HANDLE     handle,
   _Bool                 firstDescriptor,
   uint32_t                 srcPhysicalAddr,
   uint32_t                 dstPhysicalAddr,
   uint32_t                 size
);


/**
*
*  @brief   Add command to a descriptor for a device
*
*  @param   handle   (in) Handle returned in chal_dma_config_channel()
*
*  @return  DMA status
*
*  @note
******************************************************************************/
CHAL_DMA_STATUS_t chal_dma_add_device_command_to_descriptor
(
   CHAL_CHANNEL_HANDLE     handle,
   _Bool                 firstDescriptor,
   uint32_t                 srcPhysicalAddr,
   uint32_t                 dstPhysicalAddr
);


/**
*
*  @brief
*
*  @param
*  @param
*
*  @return
*
*  @note
******************************************************************************/
CHAL_DMA_STATUS_t chal_dma_transfer_complete
(
   CHAL_CHANNEL_HANDLE handle
);



/**
*
*  @brief   Returns the DMA controller's interrupt status
*
*  @param   handle   (in) Handle returned in chal_dma_init()
*
*  @return  Bit fields of DMA interrupt.
*
*  @note
******************************************************************************/
uint32_t chal_dma_get_int_status
(
   CHAL_HANDLE handle
);



/**
*
*  @brief   chal_dma_handle_to_channel
*
*  @param   handle   (in) Channel Handle returned in chal_dma_config_channel()
*
*  @return  Returns channel from channel handle
*
******************************************************************************/
CHAL_DMA_CHANNEL_t chal_dma_handle_to_channel ( CHAL_CHANNEL_HANDLE handle );


/**
*
*  @brief   chal_dma_channel_to_handle
*
*  @param   handle   (in) DMA handle
*  @param   channel  (in) DMA channel
*
*  @return  Returns channel handle from channel
*
******************************************************************************/
CHAL_CHANNEL_HANDLE chal_dma_channel_to_handle
(
   CHAL_HANDLE handle, 
   CHAL_DMA_CHANNEL_t channel
);


/**
*
*  @brief   Returns the channel that triggers interrupt
*
*  @param   handle   (in) Handle returned in chal_dma_init()
*
*  @return  DMA channel that triggers interrupt.
*
*  @note
******************************************************************************/
CHAL_DMA_CHANNEL_t chal_dma_get_int_source
(
   CHAL_HANDLE handle
);



/**
*
*  @brief   Returns the state of the DMA channel
*
*  @param   handle   (in) Handle returned in chal_dma_config_channel()
*
*  @return  state of the channel - secure / non-secure.
*
*  @note
******************************************************************************/
CHAL_DMA_SECURE_STATE_t chal_dma_get_channel_state
(
   CHAL_CHANNEL_HANDLE handle
);



/**
*
*  @brief   Returns the DMA transfer status
*
*  @param   handle   (in) Handle returned in chal_dma_init()
*
*  @return  status of the transfer.
*
*  @note
******************************************************************************/
CHAL_DMA_STATUS_t chal_dma_get_channel_status
(
   CHAL_HANDLE handle
);

/**
*
*  @brief   Disable specified DMA interrupt
*
*  @param   handle   (in) DMA channel handle
*
*  @return  None
*
*  @note
******************************************************************************/
void chal_dma_interrupt_disable
(
   CHAL_CHANNEL_HANDLE handle 
);

/**
*
*  @brief   Enable specified DMA interrupt
*
*  @param   handle   (in) DMA channel handle
*
*  @return  None
*
*  @note
******************************************************************************/
void chal_dma_interrupt_enable
(
   CHAL_CHANNEL_HANDLE handle 
);

/******************************************************************************
*
* Function Name:  chal_dma_dump_register
*
* Description:    Function to dump DMA registers
*
*******************************************************************************/
void chal_dma_dump_register
(
   CHAL_CHANNEL_HANDLE handle ,       /* [ IN ] DMA channel handle */
   int (*fpPrint) (const char *, ...) /* [ IN ] Print callback function */
);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* _CHAL_DMA_H_ */
