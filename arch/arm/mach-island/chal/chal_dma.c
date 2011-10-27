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

/* Currently, the DMA driver enables all DMA interrupts when the DMA driver
*  is initialized.  The ENABLE_INTERRUPT_PER_CHANNEL definition is provided
*  as reference code (requires proper testing) for enabling interrupts per
*  channel when a DMA channel is requested.
*/
#ifndef ENABLE_INTERRUPT_PER_CHANNEL
#define ENABLE_INTERRUPT_PER_CHANNEL      0
#endif

/*============================================================================
 *
 * \file   chal_dma.c
 * \brief  OS independent code of DMA hardware abstraction APIs.
 * \note
 *============================================================================*/
#include <plat/chal/chal_types.h>
#include <plat/chal/chal_common.h>
#include <mach/rdb/brcm_rdb_non_dmac.h>
#include <mach/io_map.h>
#include <chal/chal_dma_reg.h>
//#include <mach/csp/mm_io.h>
#include <chal/chal_dma.h>
#include <chal/chal_dmux.h>
//#include <cfg_global.h>

/*****************************************************************************
 * local macro declarations
 *****************************************************************************/
#define BCM11160   0x11160
#define CFG_GLOBAL_CHIP BCM11160
#define debug_print(a)                      /* printf a*/

#define	DECLARE_INST(inst, enc, len)        { (DMAC_INST_##inst),  (enc), (len) }
#define	DMAC_EVENT_VALID(event)             ( (event)>=CHAL_DMA_EVENT_MIN && (event)<=CHAL_DMA_EVENT_MAX )


typedef enum {
   DMAC_INST_MIN           = 1,
   DMAC_INST_DMAADDH       = 1,
   DMAC_INST_DMAEND        = 2,
   DMAC_INST_DMAFLUSHP     = 3,
   DMAC_INST_DMAGO         = 4,
   DMAC_INST_DMALD         = 5,
   DMAC_INST_DMALDP        = 6,
   DMAC_INST_DMALP         = 7,
   DMAC_INST_DMALPEND      = 8,
   DMAC_INST_DMAPFE        = 9,
   DMAC_INST_DMAKILL       = 10,
   DMAC_INST_DMAMOV        = 11,
   DMAC_INST_DMANOP        = 12,
   DMAC_INST_DMARMB        = 13,
   DMAC_INST_DMASEV        = 14,
   DMAC_INST_DMAST         = 15,
   DMAC_INST_DMASTP        = 16,
   DMAC_INST_DMASTZ        = 17,
   DMAC_INST_DMAWFE        = 18,
   DMAC_INST_DMAWFP        = 19,
   DMAC_INST_DMAWMB        = 20,
   DMAC_INST_DMALPFEEND    = 21,
   DMAC_INST_MAX           = 21
} DMAC_INST_e;


typedef enum {
  DMAC_DESC_TYPE_DATA,
  DMAC_DESC_TYPE_CMD
} DMAC_DESC_TYPE_t;


typedef union
{
   uint32_t control;
   struct
   {
      uint32_t  srcAddrMode     : 1; /* bit 0     */
#define SRC_ADDR_MASK               0x00000001
#define SRC_ADDR_MASK_SHIFT         0
      uint32_t  srcBurstSize    : 3; /* bit 1-3   */
#define SRC_BURST_SIZE_MASK         0x00000007
#define SRC_BURST_SIZE_MASK_SHIFT   1
      uint32_t  srcBurstLen     : 4; /* bit 4-7   */
#define SRC_BURST_LEN_MASK          0x0000000F
#define SRC_BURST_LEN_MASK_SHIFT    4
      uint32_t  srcProtCtrl     : 3; /* bit 8-10  */
#define SRC_PROT_CTRL_MASK          0x00000007
#define SRC_PROT_CTRL_MASK_SHIFT    8
      uint32_t  srcCacheCtrl    : 3; /* bit 11-13 */
#define SRC_CAHCE_CTRL_MASK         0x00000007
#define SRC_CAHCE_CTRL_MASK_SHIFT   11
      uint32_t  dstAddrMode     : 1; /* bit 14    */
#define DST_ADDR_MASK               0x00000001
#define DST_ADDR_MASK_SHIFT         14
      uint32_t  dstBurstSize    : 3; /* bit 15-17 */
#define DST_BURST_SIZE_MASK         0x00000007
#define DST_BURST_SIZE_MASK_SHIFT   15
      uint32_t  dstBurstLen     : 4; /* bit 18-21 */
#define DST_BURST_LEN_MASK          0x0000000F
#define DST_BURST_LEN_MASK_SHIFT    18
      uint32_t  dstProtCtrl     : 3; /* bit 22-24 */
#define DST_PROT_CTRL_MASK          0x00000007
#define DST_PROT_CTRL_MASK_SHIFT    22
      uint32_t  dstCacheCtrl    : 3; /* bit 25-27 */                               
#define DST_CAHCE_CTRL_MASK         0x00000007
#define DST_CAHCE_CTRL_MASK_SHIFT   25
      uint32_t  endianSwapSize  : 4; /* bit 28-31 */
#define END_SWAP_MASK               0x0000000F
#define END_SWAP_MASK_SHIFT         28
   } ChanCtrlBitField;
}DMA_CHAN_CONFIG_t;


typedef struct
{
   uint32_t          src;
   uint32_t          dst;
   uint32_t          size;
   DMAC_DESC_TYPE_t  type;
   DMA_CHAN_CONFIG_t control;
}DMA_DESC_t;

typedef struct 
{
   uint32_t             chanVirtualAddr;
   uint32_t             chanPhysicalAddr;
   uint32_t             codeEndVirtualAddress;
   uint32_t             maxDescCount;
   uint32_t             curDescCount;
   CHAL_DMA_DESC_TYPE_t descType;
   CHAL_DMA_EVENT_t     event_wait;
   CHAL_DMA_EVENT_t     event_send;
   DMA_CHAN_CONFIG_t    config;
   CHAL_HANDLE          dmaHandle;
   CHAL_DMA_CHANNEL_t   channel;
   uint32_t             srcBurstDataLen;
   uint32_t             dstBurstDataLen;
   _Bool                alwaysBurst; 
   _Bool                interruptDisable;
   CHAL_DMA_FLUSH_t     flushMode;
}DMA_CHAN_INFO_t;


typedef struct
{
   _Bool                   initialized;
   CHAL_DMA_CAPABILITIES_t capabilities;
   DMA_CHAN_INFO_t         channelInfo[CHAL_TOTAL_DMA_CHANNELS];
   CHAL_HANDLE             dmuxHandle;
}DMA_CONFIG_t;

typedef struct
{
   uint32_t                 baseAddr;
   CHAL_DMA_SECURE_STATE_t  secState;
   DMA_CONFIG_t*            config;
}DMA_DEV_t;


typedef struct 
{
   DMAC_INST_e  inst;        /* instruction name */
   uint32_t     inst_enc;    /* instruction encoding */
   uint32_t     inst_len;    /* instruction length */
} DMA_INSTR_ENCODE_t;


static DMA_CONFIG_t dmaConfig;
static DMA_DEV_t    secureDev;
static DMA_DEV_t    openDev;

/*******************************************************************************
 * Local Variables
 *******************************************************************************/
static const DMA_INSTR_ENCODE_t dmacInstEncode[ DMAC_INST_MAX + 1 ] = {
   { DMAC_INST_MIN,  0, 0 },	        /* 1st is empty */
   DECLARE_INST(DMAADDH,   0x54,   3),
   DECLARE_INST(DMAEND,    0,      1),
   DECLARE_INST(DMAFLUSHP, 0x35,   2),
   DECLARE_INST(DMAGO,     0xA0,   6),	/* including 32-bit address */
   DECLARE_INST(DMALD,     0x04,   1),
   DECLARE_INST(DMALDP,    0x25,   2),
   DECLARE_INST(DMALP,     0x20,   2),
   DECLARE_INST(DMALPEND,  0x28,   2),
   DECLARE_INST(DMAPFE,    0,      0),
   DECLARE_INST(DMAKILL,   0x1,    0),
   DECLARE_INST(DMAMOV,    0xBC,   6),	/* including 32-bit */
   DECLARE_INST(DMANOP,    0x18,   1),
   DECLARE_INST(DMARMB,    0x12,   1),
   DECLARE_INST(DMASEV,    0x34,   2),
   DECLARE_INST(DMAST,     0x08,   1),
   DECLARE_INST(DMASTP,    0x29,   2),
   DECLARE_INST(DMASTZ,    0x0C,   1),
   DECLARE_INST(DMAWFE,    0x36,   2),
   DECLARE_INST(DMAWFP,    0x30,   2),
   DECLARE_INST(DMAWMB,    0x13,   1),
   DECLARE_INST(DMALPFEEND,0x28,   2)
};

/*******************************************************************************
 * Local Functions
 *******************************************************************************/
static int chal_dma_prepare_circular_transfer 
(
   CHAL_CHANNEL_HANDLE handle
);

static int chal_dma_prepare_list_transfer 
(
   CHAL_CHANNEL_HANDLE handle
);

/*
 * ******************************************************************************
 * 
 *  Function Name:  chal_dma_put32
 * 
 *  Description:  
 *       
 * ******************************************************************************
 */
static inline void chal_dma_put32 ( uint8_t *buf, uint32_t val ) 
{
   buf[0] = val;
   buf[1] = val >> 8;
   buf[2] = val >> 16;
   buf[3] = val >> 24;
}


/*
 * ******************************************************************************
 * 
 *  Function Name:  chal_dma_get_loop_cnt
 * 
 *  Description:  factor loop count, both less than CHAL_DMA_REG_MAX_LOOP_COUNT
 *       
 * ******************************************************************************
 */
static CHAL_DMA_STATUS_t chal_dma_get_loop_cnt 
(
   uint32_t cnt, 
   uint32_t *loop0, 
   uint32_t *loop1,
   uint32_t *rem
)
{
   if ( cnt > CHAL_DMA_REG_MAX_LOOP_COUNT * CHAL_DMA_REG_MAX_LOOP_COUNT )
   {
      *loop0 = 0;
      *loop1 = 0;
      *rem = 0;
      return CHAL_DMA_STATUS_FAILURE;
   }
   else if( cnt <= CHAL_DMA_REG_MAX_LOOP_COUNT ) 
   {
      *loop0 = cnt;
      *loop1 = 1;
      *rem = 0;
      return CHAL_DMA_STATUS_SUCCESS;
   }
   else 
   {
      *loop0 = CHAL_DMA_REG_MAX_LOOP_COUNT;
      *loop1 = ( cnt / CHAL_DMA_REG_MAX_LOOP_COUNT );
      *rem  = cnt - ( ( *loop1 ) * ( *loop0 ) );
   }

   return CHAL_DMA_STATUS_SUCCESS;
}

/*
 * ******************************************************************************
 * 
 *  Function Name:  chal_dma_encode_instruction
 * 
 *  Description:  
 *       
 * ******************************************************************************
 */
static void chal_dma_encode_instruction
(
   uint8_t **ucode, 
   DMAC_INST_e inst, 
   uint32_t para0, 
   uint32_t para1
)
{
   (void) para1;

   switch (inst) 
   {
      case DMAC_INST_DMAWFE: 	/* para0 - event number*/
         (*ucode)[0] = dmacInstEncode[DMAC_INST_DMAWFE].inst_enc;
         (*ucode)[1] = para0 << 3;
         *ucode += dmacInstEncode[DMAC_INST_DMAWFE].inst_len;
         break;
         
      case DMAC_INST_DMAMOV: 	/*para0 - 0 src, 1 dst, 2 ccr*/
                                /*32-bit value*/
         (*ucode)[0] = dmacInstEncode[DMAC_INST_DMAMOV].inst_enc;
         (*ucode)[1] = para0;
         chal_dma_put32 (*ucode+2, para1);
         (*ucode) += dmacInstEncode[DMAC_INST_DMAMOV].inst_len;
         break;
         
      case DMAC_INST_DMALD:
         (*ucode)[0] = dmacInstEncode[DMAC_INST_DMALD].inst_enc;
         (*ucode) += dmacInstEncode[DMAC_INST_DMALD].inst_len;
         break;
         
      case DMAC_INST_DMAST:
         (*ucode)[0] = dmacInstEncode[DMAC_INST_DMAST].inst_enc;
         (*ucode) += dmacInstEncode[DMAC_INST_DMAST].inst_len;
         break;
         
      case DMAC_INST_DMASEV: 	/* para0 -- event #*/
         (*ucode)[0] = dmacInstEncode[DMAC_INST_DMASEV].inst_enc;
         (*ucode)[1] = para0 << 3;
         (*ucode) += dmacInstEncode[DMAC_INST_DMASEV].inst_len;
         break;
         
      case DMAC_INST_DMAEND:
         (*ucode)[0] = dmacInstEncode[DMAC_INST_DMAEND].inst_enc;
         (*ucode) += dmacInstEncode[DMAC_INST_DMAEND].inst_len;
         break;

      case DMAC_INST_DMAWMB:
         (*ucode)[0] = dmacInstEncode[DMAC_INST_DMAWMB].inst_enc;
         (*ucode) += dmacInstEncode[DMAC_INST_DMAWMB].inst_len;
         break;

      case DMAC_INST_DMARMB:
         (*ucode)[0] = dmacInstEncode[DMAC_INST_DMARMB].inst_enc;
         (*ucode) += dmacInstEncode[DMAC_INST_DMARMB].inst_len;
         break;
         
      case DMAC_INST_DMAWFP:	/* para0 -- burst_len*/
                                /*para1 -- peri_num*/
         (*ucode)[0] = (dmacInstEncode[DMAC_INST_DMAWFP].inst_enc & 0xff) | (para0 == CHAL_DMA_BURST_LEN_1? 0:2);
         (*ucode)[1] = (dmacInstEncode[DMAC_INST_DMAWFP].inst_enc>>8) | ( para1<< 3);
         (*ucode) += dmacInstEncode[DMAC_INST_DMAWFP].inst_len;
         break;

      case DMAC_INST_DMALDP:	/* para0 -- burst_len*/
                                /* para1 -- peri_num*/
         (*ucode)[0] = (dmacInstEncode[DMAC_INST_DMALDP].inst_enc & 0xff) | (para0 == CHAL_DMA_BURST_LEN_1? 0:2);
         (*ucode)[1] = (dmacInstEncode[DMAC_INST_DMALDP].inst_enc>>8) | (para1 << 3);
         (*ucode) += dmacInstEncode[DMAC_INST_DMALDP].inst_len;
         break;
         
      case DMAC_INST_DMASTP:	/* para0 -- burst_len*/
                                /* para1 -- peri_num*/
        (*ucode)[0] = (dmacInstEncode[DMAC_INST_DMASTP].inst_enc & 0xff) | (para0 == CHAL_DMA_BURST_LEN_1? 0:2);
         (*ucode)[1] = (dmacInstEncode[DMAC_INST_DMASTP].inst_enc>>8) | (para1 << 3);
         (*ucode) += dmacInstEncode[DMAC_INST_DMASTP].inst_len;
         break;

      case DMAC_INST_DMALP:	/* para0 -- 0, 1, which cnt*/
                                /* para1 -- loops*/
         (*ucode)[0] = dmacInstEncode[DMAC_INST_DMALP].inst_enc | (para0?0x2:0);
         (*ucode)[1] = para1-1;
         (*ucode) += dmacInstEncode[DMAC_INST_DMALP].inst_len;
         break;
         
      case DMAC_INST_DMALPEND:  /* para0 -- 0, 1, which cnt*/
                                /* para1 -- jump back*/
         (*ucode)[0] = dmacInstEncode[DMAC_INST_DMALPEND].inst_enc | 0x10 | (para0 ? 0x04:0 );
         (*ucode)[1] = para1;
         (*ucode) += dmacInstEncode[DMAC_INST_DMALPEND].inst_len;
         break;

      case DMAC_INST_DMANOP:
         (*ucode)[0] = dmacInstEncode[DMAC_INST_DMANOP].inst_enc;
         (*ucode) += dmacInstEncode[DMAC_INST_DMANOP].inst_len;
         break;

      case DMAC_INST_DMALPFEEND: /* para0 -- */
                                 /* para1 -- jump back*/
         (*ucode)[0] = dmacInstEncode[DMAC_INST_DMALPFEEND].inst_enc;
         (*ucode)[1] = para1;
         (*ucode) += dmacInstEncode[DMAC_INST_DMALPFEEND].inst_len;
         break;

      case DMAC_INST_DMAFLUSHP:
	 (*ucode)[0] = dmacInstEncode[DMAC_INST_DMAFLUSHP].inst_enc;
	 (*ucode)[1] = para0 << 3;
	 (*ucode) += dmacInstEncode[DMAC_INST_DMAFLUSHP].inst_len;
	 break;

      default:
         debug_print(("not supported: %d\n", inst));
    }
}


/*
 * ******************************************************************************
 * 
 *  Function Name: chal_dma_init
 * 
 *  Description:   Initialize DMA hardware and software interface
 * 
 * ******************************************************************************
 */
CHAL_HANDLE chal_dma_init 
(
   CHAL_DMA_SECURE_STATE_t  secureState
)
{
   DMA_DEV_t *pDmaDev = NULL;
   uint32_t chan = 0;

   if ( secureState  == CHAL_DMA_STATE_OPEN )
   {
      pDmaDev = &openDev;
      pDmaDev->baseAddr = KONA_DMAC_NS_VA;
   }
   else
   {
      pDmaDev = &secureDev;
      pDmaDev->baseAddr = KONA_DMAC_S_VA;
   }

   pDmaDev->secState = secureState;
   pDmaDev->config = &dmaConfig;

#if ENABLE_INTERRUPT_PER_CHANNEL
   /* Don't blindly enable all interrupts.  Per channel interrupt will be
   *  enabled in chal_dma_config_channel()
   */
#else
   /* Enable DMA interrupts */
   CHAL_REG_WRITE32 ( (pDmaDev->baseAddr + NON_DMAC_INTEN_OFFSET), 0xFF );
#endif

   if ( !pDmaDev->config->initialized )
   {
      for ( chan = 0; chan < CHAL_TOTAL_DMA_CHANNELS; chan++ )
      {
         pDmaDev->config->channelInfo[chan].chanVirtualAddr = 0;
         pDmaDev->config->channelInfo[chan].chanPhysicalAddr = 0;
         pDmaDev->config->channelInfo[chan].codeEndVirtualAddress = 0;
         pDmaDev->config->channelInfo[chan].maxDescCount = 0;
         pDmaDev->config->channelInfo[chan].curDescCount = 0;
      }

      /* populate capability info */
      pDmaDev->config->capabilities.numOfChannel = CHAL_TOTAL_DMA_CHANNELS;
      pDmaDev->config->capabilities.maxBurstSize = 1 << CHAL_DMA_BURST_SIZE_8_BYTES;
      pDmaDev->config->capabilities.maxBurstLen = CHAL_DMA_BURST_LEN_16 + 1;
      /* Get the handle to the DMUX block */
      pDmaDev->config->dmuxHandle = chal_dmux_init ( KONA_DMUX_VA );
      if ( pDmaDev->config->dmuxHandle == NULL )
      {
         return NULL;
      }
      
      pDmaDev->config->initialized = TRUE;
   }
   /*
    * clear any PENDING interrupt
    */
   CHAL_REG_WRITE32 ( pDmaDev->baseAddr + NON_DMAC_INTCLR_OFFSET, NON_DMAC_INTCLR_IRQ_CLR_MASK );
   
   return (CHAL_HANDLE) pDmaDev;
}


/*
 * ******************************************************************************
 * 
 *  Function Name: chal_dma_config_channel
 * 
 *  Description:   channel level configuration
 * 
 * ******************************************************************************
 */
CHAL_CHANNEL_HANDLE chal_dma_config_channel
(
   CHAL_HANDLE              handle,
   CHAL_DMA_CHANNEL_t       channel,
   CHAL_DMA_CHAN_CONFIG_t*  config
)
{
   DMA_DEV_t *pDmaDev = (DMA_DEV_t *)handle;

   DMA_CHAN_INFO_t* pChanInfo = &pDmaDev->config->channelInfo[channel];

   /* Set protection control signals for secure/non-secure channel */
   if ( pDmaDev->secState == CHAL_DMA_STATE_OPEN )
   {
      pChanInfo->config.control = CHAL_DMA_REG_CCR_SRC_OPEN | CHAL_DMA_REG_CCR_DST_OPEN;
   }
   else
   {
      pChanInfo->config.control = 0;
   }

   pChanInfo->config.control      |= ( config->srcEndpoint & SRC_ADDR_MASK ) << SRC_ADDR_MASK_SHIFT;
   pChanInfo->config.control      |= ( config->srcBurstSize & SRC_BURST_SIZE_MASK ) << SRC_BURST_SIZE_MASK_SHIFT;
   pChanInfo->config.control      |= ( config->srcBurstLen & SRC_BURST_LEN_MASK ) << SRC_BURST_LEN_MASK_SHIFT;
   pChanInfo->config.control      |= ( config->dstEndpoint & DST_ADDR_MASK ) << DST_ADDR_MASK_SHIFT;
   pChanInfo->config.control      |= ( config->dstBurstSize & DST_BURST_SIZE_MASK ) << DST_BURST_SIZE_MASK_SHIFT;
   pChanInfo->config.control      |= ( config->dstBurstLen & DST_BURST_LEN_MASK ) << DST_BURST_LEN_MASK_SHIFT;
   pChanInfo->descType             = config->descType;
   pChanInfo->event_wait           = config->event_wait;
   pChanInfo->event_send           = config->event_send;
   pChanInfo->dmaHandle            = handle;  
   pChanInfo->channel              = channel;
   pChanInfo->srcBurstDataLen      = ( config->srcBurstLen + 1) * ( 1 << config->srcBurstSize );
   pChanInfo->dstBurstDataLen      = ( config->dstBurstLen + 1) * ( 1 << config->dstBurstSize );
   pChanInfo->alwaysBurst          = config->alwaysBurst;
   pChanInfo->interruptDisable     = config->interruptDisable;
   pChanInfo->flushMode            = (config->flushMode & CHAL_DMA_FLUSH_NEVER) ? CHAL_DMA_FLUSH_NEVER : config->flushMode;

#if ENABLE_INTERRUPT_PER_CHANNEL
   /* Enable interrupt for specified DMA channel */
   chal_dma_clear_int_status( (CHAL_CHANNEL_HANDLE)pChanInfo );
   chal_dma_interrupt_enable( (CHAL_CHANNEL_HANDLE)pChanInfo );
#endif

   return ( CHAL_CHANNEL_HANDLE ) pChanInfo;
}



/*
 * ******************************************************************************
 * 
 *  Function Name: chal_dma_config_channel_memory
 * 
 *  Description:   channel level memory configuration
 * 
 * ******************************************************************************
 */
void chal_dma_config_channel_memory
(
   CHAL_CHANNEL_HANDLE handle, 
   uint32_t virtualAddr,
   uint32_t physicalAddr,
   uint32_t descCount
)
{
   DMA_CHAN_INFO_t* pChanInfo = (DMA_CHAN_INFO_t*)handle;

   pChanInfo->chanVirtualAddr = virtualAddr;
   pChanInfo->chanPhysicalAddr = physicalAddr;
   pChanInfo->codeEndVirtualAddress = 0;
   pChanInfo->maxDescCount = descCount;
   pChanInfo->curDescCount = 0;   
}


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
void chal_dma_get_capabilities
(
   CHAL_HANDLE handle, 
   CHAL_DMA_CAPABILITIES_t *pCap
)
{
   DMA_DEV_t *pDmaDev = (DMA_DEV_t *)handle;

   pCap->numOfChannel = pDmaDev->config->capabilities.numOfChannel;
   pCap->maxBurstSize = pDmaDev->config->capabilities.maxBurstSize;
   pCap->maxBurstLen = pDmaDev->config->capabilities.maxBurstLen;
}

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
)
{
   DMA_CHAN_INFO_t* pChanInfo = (DMA_CHAN_INFO_t*) handle;
   return pChanInfo->curDescCount;
}

/**
*
*  @brief   Get current descriptor index in use for channel
*
*  @param   handle   (in) Handle returned in chal_dma_config_channel()
*  @param   desc_idx (out) Index of descriptor currently in use
*
*  @return  Return 0 if found, negative if error.
*
*  @note
******************************************************************************/
int chal_dma_get_current_channel_descriptor_index
(
   CHAL_CHANNEL_HANDLE handle,
   uint32_t *desc_idx
)
{
   int rc = -1;
   int found = 0;
   uint32_t descCount;
   uint32_t src_addr, dst_addr, buf_len;
   uint32_t cur_src_addr, cur_dst_addr;
   uint32_t src_inc, dst_inc;
   int32_t src_delta, dst_delta;
   DMA_DESC_t *chanDesc;
   DMA_CHAN_INFO_t* pChanInfo = (DMA_CHAN_INFO_t*) handle;
   DMA_DEV_t *pDmaDev = (DMA_DEV_t *) pChanInfo->dmaHandle;
   CHAL_DMA_CHANNEL_t channel = pChanInfo->channel;

   cur_src_addr = CHAL_REG_READ32 ( pDmaDev->baseAddr + NON_DMAC_SA_0_OFFSET + ( channel << 5 ) ) & NON_DMAC_SA_0_SRC_ADDR_MASK;
   cur_dst_addr = CHAL_REG_READ32 ( pDmaDev->baseAddr + NON_DMAC_DA_0_OFFSET + ( channel << 5 ) ) & NON_DMAC_DA_0_DST_ADDR_MASK;

   chanDesc = (DMA_DESC_t *)pChanInfo->chanVirtualAddr;

   for ( descCount = 0 ; descCount < pChanInfo->curDescCount; descCount++ )
   {
      buf_len  = chanDesc[descCount].size;
      src_addr = chanDesc[descCount].src;
      dst_addr = chanDesc[descCount].dst;

      src_inc = pChanInfo->config.ChanCtrlBitField.srcAddrMode;
      dst_inc = pChanInfo->config.ChanCtrlBitField.dstAddrMode;

      src_delta = cur_src_addr - src_addr;
      dst_delta = cur_dst_addr - dst_addr;

      /* If both source and destination are incrementing addresses, only check for destination */
      if ( src_inc && dst_inc )
      {
         if ( dst_delta >= 0 && dst_delta <= buf_len )
         {
            found = 1;
            break;
         }
      }
      else
      {
         if( src_delta >= 0 && src_delta <= buf_len && dst_delta >= 0 && dst_delta <= buf_len )
         {
            found = 1;
            break;
         }
      }
   }

   if ( found )
   {
       *desc_idx = descCount;
       rc  = 0;
   }

   return rc;
}

/*
 * ******************************************************************************
 * 
 * 
 *  Function Name:  chal_dma_clear_int_status
 * 
 *  Description:    clears the interrupt
 * 
 * ******************************************************************************
 */
void chal_dma_clear_int_status 
( 
   CHAL_CHANNEL_HANDLE handle 
)
{
   DMA_CHAN_INFO_t* pChanInfo = (DMA_CHAN_INFO_t*) handle;
   DMA_DEV_t *pDmaDev = (DMA_DEV_t *) pChanInfo->dmaHandle;
   CHAL_DMA_CHANNEL_t channel = pChanInfo->channel;
   
   CHAL_REG_WRITE32 ( pDmaDev->baseAddr + NON_DMAC_INTCLR_OFFSET, ( 0x1 << channel) );
}


/*
 * ******************************************************************************
 * 
 *  Function Name:  chal_dma_shutdown_channel
 * 
 *  Description:    Terminate a DMA transfer gracefully without data lose
 * 
 * ******************************************************************************
 */
void chal_dma_shutdown_channel ( CHAL_CHANNEL_HANDLE handle )
{
   uint32_t val;
   DMA_CHAN_INFO_t* pChanInfo = (DMA_CHAN_INFO_t*) handle;
   DMA_DEV_t *pDmaDev = (DMA_DEV_t *) pChanInfo->dmaHandle;
   CHAL_DMA_CHANNEL_t channel = pChanInfo->channel;

   
   val = dmacInstEncode[DMAC_INST_DMAKILL].inst_enc;
   val <<= 16;
   val |= ( channel << 8 ) | 1;
   CHAL_REG_WRITE32 ( pDmaDev->baseAddr + NON_DMAC_DBGINST0_OFFSET, val );
   
   val = 0;
   CHAL_REG_WRITE32 ( pDmaDev->baseAddr + NON_DMAC_DBGCMD_OFFSET, val );
}


/*
 * ******************************************************************************
 * 
 *  Function Name: chal_dma_get_channel
 * 
 *  Description:   get a DMA channel
 * 
 * ******************************************************************************
 */
CHAL_DMA_STATUS_t chal_dma_get_channel ( CHAL_HANDLE handle, CHAL_DMA_CHANNEL_t *channel )
{
   DMA_DEV_t *pDmaDev = (DMA_DEV_t *)handle;

   CHAL_DMUX_STATUS_t ResultDemux = chal_dmux_alloc_channel ( pDmaDev->config->dmuxHandle, (uint32_t*)channel );

   if (ResultDemux != CHAL_DMUX_STATUS_SUCCESS)
   {
      return (CHAL_DMA_STATUS_FAILURE);
   }
   else
   {
      return (CHAL_DMA_STATUS_SUCCESS);
   }
}


/*
 * ******************************************************************************
 * 
 *  Function Name: chal_dma_release_channel
 * 
 *  Description:   release allocated channel
 * 
 * ******************************************************************************
 */
void chal_dma_release_channel ( CHAL_CHANNEL_HANDLE handle ) 
{

   DMA_CHAN_INFO_t* pChanInfo = (DMA_CHAN_INFO_t*) handle;
   DMA_DEV_t *pDmaDev = (DMA_DEV_t *) pChanInfo->dmaHandle;
   CHAL_DMA_CHANNEL_t channel = pChanInfo->channel;

   chal_dmux_dealloc_channel(pDmaDev->config->dmuxHandle, channel);
   pChanInfo->maxDescCount = 0;
   pChanInfo->curDescCount = 0;

#if ENABLE_INTERRUPT_PER_CHANNEL
   chal_dma_interrupt_disable( handle );
   chal_dma_clear_int_status( handle );
#endif
}


/*
 * ******************************************************************************
 * 
 *  Function Name: chal_dma_connect_peripheral
 * 
 *  Description:   assign peripheral to a DMA channel
 * 
 * ******************************************************************************
 */
CHAL_DMA_STATUS_t chal_dma_connect_peripheral 
( 
   CHAL_CHANNEL_HANDLE     handle,
   CHAL_DMA_PERIPHERAL_t   Peri 
)
{
   CHAL_DMUX_STATUS_t ResultDemux;
   DMA_CHAN_INFO_t* pChanInfo = (DMA_CHAN_INFO_t*) handle;
   DMA_DEV_t *pDmaDev = (DMA_DEV_t *) pChanInfo->dmaHandle;
   CHAL_DMA_CHANNEL_t channel = pChanInfo->channel;
   CHAL_DMA_PERIPHERAL_t PeriB __attribute__ ((unused));
   uint8_t src_id __attribute__ ((unused)); 
   uint8_t dst_id __attribute__ ((unused));
   
   PeriB = CHAL_DMA_PERIPHERAL_INVALID;
   ResultDemux = chal_dmux_alloc_peripheral ( pDmaDev->config->dmuxHandle, channel, (CHAL_DMUX_END_POINT_t)Peri, (CHAL_DMUX_END_POINT_t) PeriB, &src_id, &dst_id);

   if (ResultDemux != CHAL_DMUX_STATUS_SUCCESS)
   {
      return (CHAL_DMA_STATUS_FAILURE);
   }
   else
   {
      return (CHAL_DMA_STATUS_SUCCESS);
   }
}


/*
 * ******************************************************************************
 * 
 *  Function Name: chal_dma_disconnect_peripheral
 * 
 *  Description:   disconnect peripheral from the DMA channel
 * 
 * ******************************************************************************
 */
void chal_dma_disconnect_peripheral 
( 
   CHAL_CHANNEL_HANDLE handle
)
{
   DMA_CHAN_INFO_t* pChanInfo = (DMA_CHAN_INFO_t*) handle;
   DMA_DEV_t *pDmaDev = (DMA_DEV_t *) pChanInfo->dmaHandle;
   CHAL_DMA_CHANNEL_t channel = pChanInfo->channel;

   chal_dmux_dealloc_peripheral(pDmaDev->config->dmuxHandle, channel);
}
            

       

/*
 * ******************************************************************************
 * 
 *  Function Name: chal_dma_add_device_command_to_descriptor
 * 
 *  Description:   Add a descriptor carring a command from memory to a device
 * 
 * ******************************************************************************
 */
CHAL_DMA_STATUS_t chal_dma_add_device_command_to_descriptor
(
   CHAL_CHANNEL_HANDLE     handle,
   _Bool                   firstDescriptor,
   uint32_t                srcPhysicalAddr,
   uint32_t                dstPhysicalAddr
)
{
   DMA_DESC_t *chanDesc;
   DMA_CHAN_INFO_t* pChanInfo = (DMA_CHAN_INFO_t*) handle;
   uint32_t index = 0;


   /* Channel must be connected to a device */
   if ( ( pChanInfo->config.ChanCtrlBitField.srcAddrMode != 0 ) && (pChanInfo->config.ChanCtrlBitField.dstAddrMode != 0 ) )
   {
      return CHAL_DMA_STATUS_FAILURE;
   }
 
   /* Reset current descripror count if this for first descriptor */
   if ( firstDescriptor )
   {
      pChanInfo->curDescCount = 0;
   }

   if ( ( pChanInfo->maxDescCount == 0 ) || 
        ( pChanInfo->curDescCount >= pChanInfo->maxDescCount ) )
   {
      return CHAL_DMA_STATUS_OVERFLOW;
   }

   chanDesc = (DMA_DESC_t *)pChanInfo->chanVirtualAddr;
      
   if( firstDescriptor )
   {
      index = 0;
      pChanInfo->curDescCount = 1;
   }
   else
   {
      index = pChanInfo->curDescCount;
      pChanInfo->curDescCount++;
   }
   /* Configure descriptor */
   chanDesc[index].src  = srcPhysicalAddr;
   chanDesc[index].dst  = dstPhysicalAddr;
   chanDesc[index].size = sizeof(uint32_t);
   /* Command descriptor */
   chanDesc[index].type = DMAC_DESC_TYPE_CMD;
   /* Get channel control parameters */
   chanDesc[index].control.control = pChanInfo->config.control;
   /* Re-configure the endpoint type 
      src endpoint type: memory
      dst endpoint type: memory
    */
   chanDesc[index].control.control = ( chanDesc[index].control.control & ~(SRC_ADDR_MASK << SRC_ADDR_MASK_SHIFT))
                                      | (CHAL_DMA_ENDPOINT_MEMORY << SRC_ADDR_MASK_SHIFT);
   chanDesc[index].control.control = ( chanDesc[index].control.control & ~(DST_ADDR_MASK << DST_ADDR_MASK_SHIFT))
                                      | (CHAL_DMA_ENDPOINT_MEMORY << DST_ADDR_MASK_SHIFT);
   /* Re-configure burst length to CHAL_DMA_BURST_LEN_1 for src and dst */
   chanDesc[index].control.control = ( chanDesc[index].control.control & ~(SRC_BURST_LEN_MASK << SRC_BURST_LEN_MASK_SHIFT))
                                      | (CHAL_DMA_BURST_LEN_1 << SRC_BURST_LEN_MASK_SHIFT);
   chanDesc[index].control.control = ( chanDesc[index].control.control & ~(DST_BURST_LEN_MASK << DST_BURST_LEN_MASK_SHIFT)) 
                                      | (CHAL_DMA_BURST_LEN_1 << DST_BURST_LEN_MASK_SHIFT);

   /* Re-configure burst size to CHAL_DMA_BURST_SIZE_4_BYTES for src and dst */
   chanDesc[index].control.control = ( chanDesc[index].control.control & ~(SRC_BURST_SIZE_MASK << SRC_BURST_SIZE_MASK_SHIFT))
                                      | (CHAL_DMA_BURST_SIZE_4_BYTES << SRC_BURST_SIZE_MASK_SHIFT);
   chanDesc[index].control.control = ( chanDesc[index].control.control & ~(DST_BURST_SIZE_MASK << DST_BURST_SIZE_MASK_SHIFT)) 
                                      | (CHAL_DMA_BURST_SIZE_4_BYTES << DST_BURST_SIZE_MASK_SHIFT);

   return CHAL_DMA_STATUS_SUCCESS;
}


       
/*
 * ******************************************************************************
 * 
 *  Function Name: chal_dma_add_descriptor
 * 
 *  Description:   Add a descriptor to a DMA channel
 * 
 * ******************************************************************************
 */
CHAL_DMA_STATUS_t chal_dma_add_descriptor
(
   CHAL_CHANNEL_HANDLE     handle,
   _Bool                   firstDescriptor,
   uint32_t                srcPhysicalAddr,
   uint32_t                dstPhysicalAddr,
   uint32_t                size
)
{
   DMA_DESC_t *chanDesc;
   DMA_CHAN_INFO_t* pChanInfo = (DMA_CHAN_INFO_t*) handle;
   uint32_t index = 0;

#if ( CFG_GLOBAL_CHIP == BCM11160 ) 
   /* Destination memory address must be 8 byte aligned.
      This is a the bug in the DMA controller.
    */
   if ( pChanInfo->config.ChanCtrlBitField.dstAddrMode && ( dstPhysicalAddr & 0x00000007 ) )
   {
      return CHAL_DMA_STATUS_INVALID_PARAMETER;
   }
#endif
   /* Reset current descripror count if this for first descriptor */
   if ( firstDescriptor )
   {
      pChanInfo->curDescCount = 0;
   }

   if ( size )
   {
      if ( ( pChanInfo->maxDescCount == 0 ) || 
           ( pChanInfo->curDescCount >= pChanInfo->maxDescCount ) )
      {
         return CHAL_DMA_STATUS_OVERFLOW;
      }

      chanDesc = (DMA_DESC_t *)pChanInfo->chanVirtualAddr;
      
      if( firstDescriptor )
      {
         index = 0;
         pChanInfo->curDescCount = 1;
      }
      else
      {
         index = pChanInfo->curDescCount;
         pChanInfo->curDescCount++;
      }
      /* Configure descriptor */
      chanDesc[index].src  = srcPhysicalAddr;
      chanDesc[index].dst  = dstPhysicalAddr;
      chanDesc[index].size = size;
      /* Data descriptor */
      chanDesc[index].type = DMAC_DESC_TYPE_DATA;
      /* Configure channel control parameters */
      chanDesc[index].control.control = pChanInfo->config.control;
      /* Do single  or modified burst transfer if data is not multiple of burst length */
      if ( size % chal_dma_get_burst_length ( handle ) )
      {
         uint32_t mask;
         uint32_t burstLen;
         mask = ~ ( ( 0xFFFFFFFF >> chanDesc[index].control.ChanCtrlBitField.srcBurstSize ) << chanDesc[index].control.ChanCtrlBitField.srcBurstSize );
         
         /* Data size must be multiple of srcBurstSize bytes */
         if ( size & mask )
         {
            pChanInfo->curDescCount --;
            return CHAL_DMA_STATUS_INVALID_PARAMETER;
         }

         if ( pChanInfo->alwaysBurst )
         {
            burstLen = ( size / ( 1 << chanDesc[index].control.ChanCtrlBitField.srcBurstSize ) ) - 1;
            /* Re-configure burst length to new burstLen */
            chanDesc[index].control.control = ( chanDesc[index].control.control & ~(SRC_BURST_LEN_MASK << SRC_BURST_LEN_MASK_SHIFT))
                                              | (burstLen << SRC_BURST_LEN_MASK_SHIFT);
            chanDesc[index].control.control = ( chanDesc[index].control.control & ~(DST_BURST_LEN_MASK << DST_BURST_LEN_MASK_SHIFT)) 
                                              | (burstLen << DST_BURST_LEN_MASK_SHIFT);
         }
         else
         {
            /* Re-configure burst length to 1 */
            chanDesc[index].control.control = ( chanDesc[index].control.control & ~(SRC_BURST_LEN_MASK << SRC_BURST_LEN_MASK_SHIFT))
                                              | (CHAL_DMA_BURST_LEN_1 << SRC_BURST_LEN_MASK_SHIFT);
            chanDesc[index].control.control = ( chanDesc[index].control.control & ~(DST_BURST_LEN_MASK << DST_BURST_LEN_MASK_SHIFT)) 
                                              | (CHAL_DMA_BURST_LEN_1 << DST_BURST_LEN_MASK_SHIFT);
         }
      }
   }
   
   return CHAL_DMA_STATUS_SUCCESS;
}


/*
 * ******************************************************************************
 * 
 *  Function Name:  chal_dma_transfer_complete
 * 
 *  Description:    Wait for a channel to complete the transfer 
 * 
 * ******************************************************************************
 */
CHAL_DMA_STATUS_t chal_dma_transfer_complete
( 
   CHAL_CHANNEL_HANDLE handle
)
{
   DMA_CHAN_INFO_t* pChanInfo = (DMA_CHAN_INFO_t*) handle;
   DMA_DEV_t *pDmaDev = (DMA_DEV_t *) pChanInfo->dmaHandle;
   CHAL_DMA_CHANNEL_t channel = pChanInfo->channel;


   return CHAL_REG_READ32 ( pDmaDev->baseAddr + NON_DMAC_CS0_OFFSET + ( channel << 3 ) ) & NON_DMAC_CS0_CHANNEL_STATUS_MASK;
}

/*
 * ******************************************************************************
 * 
 *  Function Name: chal_dma_get_burst_length
 * 
 *  Description:   returns the burst size in bytes of the channel
 * 
 * ******************************************************************************
 */
uint32_t chal_dma_get_burst_length
(
   CHAL_CHANNEL_HANDLE handle
)
{
   DMA_CHAN_INFO_t* pChanInfo = (DMA_CHAN_INFO_t*)handle;

   return ( pChanInfo->srcBurstDataLen > pChanInfo->dstBurstDataLen ) ? pChanInfo->srcBurstDataLen : pChanInfo->dstBurstDataLen;
}

/*
 * ******************************************************************************
 * 
 *  Function Name:  chal_dma_calculate_max_data_per_descriptor
 * 
 *  Description:    calculate maximum number of bytes one descriptor can transfer
 *       
 * ******************************************************************************
 */
uint32_t  chal_dma_calculate_max_data_per_descriptor
(
   CHAL_CHANNEL_HANDLE handle
)
{
   return  chal_dma_get_burst_length ( handle ) * CHAL_DMA_REG_MAX_LOOP_COUNT * CHAL_DMA_REG_MAX_LOOP_COUNT;
} 


/*
 * ******************************************************************************
 * 
 *  Function Name:  chal_dma_calculate_channel_memory 
 * 
 *  Description:  calculate amount of memory (in bytes) needed for a channel
 *       
 * ******************************************************************************
 */
uint32_t chal_dma_calculate_channel_memory 
(
   CHAL_CHANNEL_HANDLE handle,
   uint32_t            descCount
)
{
   uint32_t length = 0;
   uint32_t src_inc = 0;
   uint32_t dst_inc = 0;
   uint32_t src_bytes = 0;
   uint32_t dst_bytes = 0;
   uint32_t ld_cnt = 0;
   uint32_t st_cnt = 0;
   CHAL_DMA_EVENT_t is_wait = CHAL_DMA_EVENT_INVALID; 
   CHAL_DMA_EVENT_t is_send = CHAL_DMA_EVENT_INVALID;
   DMA_CHAN_INFO_t* pChanInfo = (DMA_CHAN_INFO_t*) handle;

   is_wait = DMAC_EVENT_VALID ( pChanInfo->event_wait ) ? 1 : 0;
   is_send = DMAC_EVENT_VALID ( pChanInfo->event_send ) ? 1 : 0;
   
   src_inc = pChanInfo->config.ChanCtrlBitField.srcAddrMode;
   dst_inc = pChanInfo->config.ChanCtrlBitField.dstAddrMode;
   
   src_bytes = pChanInfo->srcBurstDataLen;
   dst_bytes = pChanInfo->dstBurstDataLen;
   
   if ( (src_bytes % dst_bytes) && ( dst_bytes % src_bytes) ) 
   {
      return CHAL_DMA_STATUS_INVALID_PARAMETER;
   }

   if ( src_bytes < dst_bytes ) 
   {
      ld_cnt = dst_bytes / src_bytes;
      st_cnt = 1;
   }
   else 
   {
      ld_cnt = 1;
      st_cnt = src_bytes / dst_bytes;
   }

   length = ( is_wait ? dmacInstEncode[DMAC_INST_DMAWFE].inst_len : 0 );

   if ( pChanInfo->descType == CHAL_DMA_DESC_RING )
   {
      if((src_inc) && (dst_inc))
      {
        /* memory to memory */
        length += descCount * ( 3 * dmacInstEncode[DMAC_INST_DMAMOV].inst_len
                                + 2 * ld_cnt * dmacInstEncode[DMAC_INST_DMALD].inst_len
                                + 2 * ld_cnt * dmacInstEncode[DMAC_INST_DMARMB].inst_len
                                + 2 * st_cnt * dmacInstEncode[DMAC_INST_DMAST].inst_len
                                + 2 * st_cnt * dmacInstEncode[DMAC_INST_DMAWMB].inst_len
                                + 3 * ( dmacInstEncode[DMAC_INST_DMALP].inst_len + dmacInstEncode[DMAC_INST_DMALPEND].inst_len )
                                + dmacInstEncode[DMAC_INST_DMAWMB].inst_len
                                + ( is_send ? dmacInstEncode[DMAC_INST_DMASEV].inst_len : 0 )
                                + dmacInstEncode[DMAC_INST_DMASEV].inst_len
                                )
                  + dmacInstEncode[DMAC_INST_DMALPFEEND].inst_len;
      }
      else if((src_inc) && (! dst_inc ))
      {
         /* memory to peripheral */
         length += descCount * ( dmacInstEncode[DMAC_INST_DMAFLUSHP].inst_len
                                 + 3 * dmacInstEncode[DMAC_INST_DMAMOV].inst_len
                                 + 2 * ld_cnt * dmacInstEncode[DMAC_INST_DMALD].inst_len
                                 + 2 * st_cnt * (dmacInstEncode[DMAC_INST_DMAWFP].inst_len + dmacInstEncode[DMAC_INST_DMASTP].inst_len )
                                 + 3 * (dmacInstEncode[DMAC_INST_DMALP].inst_len + dmacInstEncode[DMAC_INST_DMALPEND].inst_len )
                                 + dmacInstEncode[DMAC_INST_DMAWMB].inst_len
                                 + ( is_send ? dmacInstEncode[DMAC_INST_DMASEV].inst_len:0 )
                                 + dmacInstEncode[DMAC_INST_DMASEV].inst_len
                                 )
                    + dmacInstEncode[DMAC_INST_DMALPFEEND].inst_len;
      }
      else if(( !src_inc ) && (dst_inc))
      {
         /* peripheral to memory */
         length += descCount * ( dmacInstEncode[DMAC_INST_DMAFLUSHP].inst_len
                                 + 3 * dmacInstEncode[DMAC_INST_DMAMOV].inst_len
                                 + 2 * ld_cnt * (dmacInstEncode[DMAC_INST_DMAWFP].inst_len + dmacInstEncode[DMAC_INST_DMALDP].inst_len )
                                 + 2 * st_cnt * dmacInstEncode[DMAC_INST_DMAST].inst_len
                                 + 3 * (dmacInstEncode[DMAC_INST_DMALP].inst_len + dmacInstEncode[DMAC_INST_DMALPEND].inst_len )
                                 + dmacInstEncode[DMAC_INST_DMAWMB].inst_len
                                 + ( is_send ? dmacInstEncode[DMAC_INST_DMASEV].inst_len : 0 )
                                 + dmacInstEncode[DMAC_INST_DMASEV].inst_len
                                 )
                    + dmacInstEncode[DMAC_INST_DMALPFEEND].inst_len;
      }
      else  
      {
         /* peripheral to peripheral */
         return -1;
      }
   }
   else /* CHAL_DMA_DESC_LIST */
   {
       if ( (src_inc) && ( dst_inc) )
       {
          /* memory to memory */
          length += descCount * ( 3 * dmacInstEncode[DMAC_INST_DMAMOV].inst_len
                                  + 2 * ld_cnt * dmacInstEncode[DMAC_INST_DMALD].inst_len
                                  + 2 * ld_cnt * dmacInstEncode[DMAC_INST_DMARMB].inst_len
                                  + 2 * st_cnt * dmacInstEncode[DMAC_INST_DMAST].inst_len
                                  + 2 * st_cnt * dmacInstEncode[DMAC_INST_DMAWMB].inst_len
                                  + 3 * (dmacInstEncode[DMAC_INST_DMALP].inst_len + dmacInstEncode[DMAC_INST_DMALPEND].inst_len)
                                  )
                   + dmacInstEncode[DMAC_INST_DMAWMB].inst_len
                   + ( is_send ? dmacInstEncode[DMAC_INST_DMASEV].inst_len:0 )
                   + dmacInstEncode[DMAC_INST_DMASEV].inst_len
                   + dmacInstEncode[DMAC_INST_DMAEND].inst_len;
       }
       else if ((src_inc) && (!dst_inc) )
       {
          /* memory to peripheral */
          length +=  descCount * ( dmacInstEncode[DMAC_INST_DMAFLUSHP].inst_len 
				   + 3 * dmacInstEncode[DMAC_INST_DMAMOV].inst_len
                                   + 2 * ld_cnt * dmacInstEncode[DMAC_INST_DMALD].inst_len
                                   + 2 * st_cnt * ( dmacInstEncode[DMAC_INST_DMAWFP].inst_len + dmacInstEncode[DMAC_INST_DMASTP].inst_len)
                                   + 3 * (dmacInstEncode[DMAC_INST_DMALP].inst_len + dmacInstEncode[DMAC_INST_DMALPEND].inst_len)
				   + dmacInstEncode[DMAC_INST_DMAWMB].inst_len
                                   )
                   + ( is_send ? dmacInstEncode[DMAC_INST_DMASEV].inst_len:0 )
                   + dmacInstEncode[DMAC_INST_DMAFLUSHP].inst_len
                   + dmacInstEncode[DMAC_INST_DMASEV].inst_len
                   + dmacInstEncode[DMAC_INST_DMAEND].inst_len;
       }
       else if ((!src_inc) && (dst_inc) )
       {
          /* peripheral to memory */
          length +=  descCount * ( dmacInstEncode[DMAC_INST_DMAFLUSHP].inst_len
				 + 3 * dmacInstEncode[DMAC_INST_DMAMOV].inst_len
                                 + 2 * ld_cnt * (dmacInstEncode[DMAC_INST_DMAWFP].inst_len + dmacInstEncode[DMAC_INST_DMALDP].inst_len)
                                 + 2 * st_cnt * dmacInstEncode[DMAC_INST_DMAST].inst_len
                                 + 3 * (dmacInstEncode[DMAC_INST_DMALP].inst_len + dmacInstEncode[DMAC_INST_DMALPEND].inst_len)
				 + dmacInstEncode[DMAC_INST_DMAWMB].inst_len
				 )
                   + ( is_send ? dmacInstEncode[DMAC_INST_DMASEV].inst_len:0 )
                   + dmacInstEncode[DMAC_INST_DMAFLUSHP].inst_len
                   + dmacInstEncode[DMAC_INST_DMASEV].inst_len
                   + dmacInstEncode[DMAC_INST_DMAEND].inst_len;
       }
       else  
       {
          /* peripheral to peripheral */
          return -1;
       }
   }

   /* Return number of bytes needed for descriptors and micro code */
   return  ( sizeof (DMA_DESC_t) * descCount ) + length;
}


/*
 * ******************************************************************************
 * 
 *  Function Name:  chal_dma_prepare_transfer
 * 
 *  Description:  Prepare instruction encoding for transfer
 *       
 * ******************************************************************************
 */
static int chal_dma_prepare_list_transfer
(
   CHAL_CHANNEL_HANDLE handle
)
{
   uint32_t src_burst_len, src_burst_size, dst_burst_len, dst_burst_size;
   uint32_t src_addr, dst_addr, buf_len;
   uint32_t src_inc = 0;
   uint32_t dst_inc = 0;
   uint32_t j, cnt, descCount, src_bytes, dst_bytes;
   uint32_t loop0, loop1, ld_cnt, st_cnt, rem;
   uint8_t *inst = NULL;
   uint8_t *inst_loop0, *inst_loop1, is_wait, is_send;
   DMA_DESC_t *chanDesc;
   uint32_t control;
   DMA_CHAN_INFO_t* pChanInfo = (DMA_CHAN_INFO_t*) handle;
   CHAL_DMA_CHANNEL_t channel = pChanInfo->channel;
   uint32_t peripheralId = 2 * channel;
   int dataBytes = 0;

   /* Locate instruction memory */
   inst = (uint8_t *)(pChanInfo->chanVirtualAddr + pChanInfo->maxDescCount * sizeof (DMA_DESC_t));
   /* Locate channel descriptor */
   chanDesc = (DMA_DESC_t *) pChanInfo->chanVirtualAddr;
   /* Identify events */
   is_wait = DMAC_EVENT_VALID ( pChanInfo->event_wait ) ? 1 : 0;
   is_send = DMAC_EVENT_VALID ( pChanInfo->event_send ) ? 1 : 0;

   /* Start encoding */
   is_wait ? chal_dma_encode_instruction ( &inst, DMAC_INST_DMAWFE, pChanInfo->event_wait, 0 ) : 0;

   /* Flush peripheral at the begining of the transfer */
   if ( pChanInfo->flushMode & CHAL_DMA_FLUSH_FIRST )
   {
      /* Channel must be connected to a device */
      if ( ( pChanInfo->config.ChanCtrlBitField.srcAddrMode == 0 ) || (pChanInfo->config.ChanCtrlBitField.dstAddrMode == 0 ) )
      {
         /* DMAFLUSHP */
         chal_dma_encode_instruction ( &inst, DMAC_INST_DMAFLUSHP, peripheralId, 0);
      }
   }

   /* Program for each descriptor */
   for ( descCount = 0 ; descCount < pChanInfo->curDescCount; descCount++ )
   {
      buf_len  = chanDesc[descCount].size;
      src_addr = chanDesc[descCount].src;
      dst_addr = chanDesc[descCount].dst;
      control  = chanDesc[descCount].control.control;
      dataBytes += buf_len;
      /* Address mode */
      src_inc = chanDesc[descCount].control.ChanCtrlBitField.srcAddrMode;
      dst_inc = chanDesc[descCount].control.ChanCtrlBitField.dstAddrMode;

      /* Obtain channel properties */
      src_burst_len = chanDesc[descCount].control.ChanCtrlBitField.srcBurstLen;
      src_burst_size = chanDesc[descCount].control.ChanCtrlBitField.srcBurstSize;
      dst_burst_len = chanDesc[descCount].control.ChanCtrlBitField.dstBurstLen;
      dst_burst_size = chanDesc[descCount].control.ChanCtrlBitField.dstBurstSize;
      /* Obtain burst data length */
      src_bytes = ( (src_burst_len + 1) * (1 << src_burst_size));
      dst_bytes = ( (dst_burst_len + 1) * (1 << dst_burst_size));
      /* Find number of load/store instructions needed */
      if ( src_bytes < dst_bytes ) 
      {
	 ld_cnt = dst_bytes / src_bytes;
	 st_cnt = 1;
      }
      else 
      {
	 ld_cnt = 1;
	 st_cnt = src_bytes / dst_bytes;
      }

      /* Count total number of burst */
      cnt = buf_len / ( src_bytes * ld_cnt );
      /* Factor the burst count */
      if ( chal_dma_get_loop_cnt (cnt, &loop0, &loop1, &rem) )
      {
         debug_print(( "chal_dma_prepare_list_transfer: invalid loop cnt\n"));
         return CHAL_DMA_STATUS_INVALID_PARAMETER;
      }
      /* Clean up the peripheral if needed*/
      if ( ( src_inc == 0 ) || ( dst_inc == 0) )
      {
         /* Always flush peripheral */
         if ( pChanInfo->flushMode == CHAL_DMA_FLUSH_ALWAYS )
         {
            /* DMAFLUSHP */
            chal_dma_encode_instruction ( &inst, DMAC_INST_DMAFLUSHP, peripheralId, 0);
         }
      }
      /* mov CCR */
      chal_dma_encode_instruction ( &inst, DMAC_INST_DMAMOV, 1, control );
      /* mov SRC */
      chal_dma_encode_instruction ( &inst, DMAC_INST_DMAMOV, 0, src_addr );
      /* mov DST */
      chal_dma_encode_instruction ( &inst, DMAC_INST_DMAMOV, 2, dst_addr );
      /* loop 1 */
      chal_dma_encode_instruction ( &inst, DMAC_INST_DMALP, 1, loop1 );
      inst_loop1 = inst;
      /* loop 0 */
      chal_dma_encode_instruction ( &inst, DMAC_INST_DMALP, 0, loop0 );
      inst_loop0 = inst;
      /* Encode load instructions */
      for ( j = 0; j < ld_cnt; j ++ ) 
      {
         if ( src_inc )
         {
            /* DMALD */
            chal_dma_encode_instruction ( &inst, DMAC_INST_DMALD, 0, 0 );
#if ( CFG_GLOBAL_CHIP == BCM11160 ) 
	    /* Wait until all read completed */
	    chal_dma_encode_instruction ( &inst, DMAC_INST_DMARMB, 0, 0 );
#endif
         }
         else if ( dst_inc )
         {
            /* DMAWFP */
            chal_dma_encode_instruction ( &inst, DMAC_INST_DMAWFP, src_burst_len, peripheralId );
            /* DMALDP */
            chal_dma_encode_instruction ( &inst, DMAC_INST_DMALDP, src_burst_len, peripheralId );
         }
         else
         {
            /* peripheral to peripheral */
            return CHAL_DMA_STATUS_FAILURE;
         }
      }
      /* Encode store instructions */  
      if ( ( pChanInfo->flushMode & CHAL_DMA_FLUSH_CMD ) && (chanDesc[descCount].type == DMAC_DESC_TYPE_CMD) )
      {
         /* DMAWFP (Wait for a burst request signal ) */
         chal_dma_encode_instruction ( &inst, DMAC_INST_DMAWFP, CHAL_DMA_BURST_LEN_2, peripheralId );
         /* DMAST */
         chal_dma_encode_instruction ( &inst, DMAC_INST_DMAST, 0, 0 );
         /* Wait until write completed */
         chal_dma_encode_instruction ( &inst, DMAC_INST_DMAWMB, 0, 0 );
         /* DMAFLUSHP */
         chal_dma_encode_instruction ( &inst, DMAC_INST_DMAFLUSHP, peripheralId, 0);
      }
      else
      {
         for ( j = 0; j < st_cnt; j ++ ) 
         {	
            if ( dst_inc )
            {

               /* DMAST */
               chal_dma_encode_instruction ( &inst, DMAC_INST_DMAST, 0, 0 );
#if ( CFG_GLOBAL_CHIP == BCM11160 ) 
               /* Wait until all write completed */
               chal_dma_encode_instruction ( &inst, DMAC_INST_DMAWMB, 0, 0 );
#endif
            }
            else if ( src_inc )
            {
               /* DMAWFP */
               chal_dma_encode_instruction ( &inst, DMAC_INST_DMAWFP, dst_burst_len, peripheralId );
               /* DMASTP */
               chal_dma_encode_instruction ( &inst, DMAC_INST_DMASTP, dst_burst_len, peripheralId );
            }
            else
            {
               /* peripheral to peripheral */
               return CHAL_DMA_STATUS_FAILURE;
            }
         }
      }
      /* loopend 0 */
      chal_dma_encode_instruction ( &inst, DMAC_INST_DMALPEND, 0, inst - inst_loop0 );
      /* loopend 1 */
      chal_dma_encode_instruction ( &inst, DMAC_INST_DMALPEND, 1, inst - inst_loop1 );
      /* Encode remaining loop */
      if (rem) 
      {
         /* rem */
         chal_dma_encode_instruction ( &inst, DMAC_INST_DMALP, 0, rem );
         /* remember the location to jump */
         inst_loop0 = inst;
         /* Encode load instructions */
         for ( j = 0; j < ld_cnt; j ++ ) 
         {
            if ( src_inc )
            {
               /* DMALD */
               chal_dma_encode_instruction ( &inst, DMAC_INST_DMALD, 0, 0 );
#if ( CFG_GLOBAL_CHIP == BCM11160 ) 
               /* Wait until all read completed */
	       chal_dma_encode_instruction ( &inst, DMAC_INST_DMARMB, 0, 0 );
#endif
            }
            else 
            {
               /* DMAWFP */
               chal_dma_encode_instruction ( &inst, DMAC_INST_DMAWFP, src_burst_len, peripheralId );
               /* DMALDP */
               chal_dma_encode_instruction ( &inst, DMAC_INST_DMALDP, src_burst_len, peripheralId );
            }
         }
         /* Encode store instructions */      
         for ( j = 0; j < st_cnt; j ++ ) 
         {	
            if ( dst_inc )
            {
               /* DMAST */
               chal_dma_encode_instruction ( &inst, DMAC_INST_DMAST, 0, 0 );
#if ( CFG_GLOBAL_CHIP == BCM11160 ) 
               /* Wait until all write completed */
	       chal_dma_encode_instruction ( &inst, DMAC_INST_DMAWMB, 0, 0 );
#endif
            }
            else
            {
               /* DMAWFP */
               chal_dma_encode_instruction ( &inst, DMAC_INST_DMAWFP, dst_burst_len, peripheralId );
               /* DMASTP */
               chal_dma_encode_instruction ( &inst, DMAC_INST_DMASTP, dst_burst_len, peripheralId );
            }
         }
         /* DMALPEND */
         /* Jump to the begining of the loop */
         chal_dma_encode_instruction ( &inst, DMAC_INST_DMALPEND, 0, inst - inst_loop0);
      }
      /* DMA WMB */
      chal_dma_encode_instruction ( &inst, DMAC_INST_DMAWMB, pChanInfo->event_send, 0 );
   }
   /* DMA SEV */
   is_send ? chal_dma_encode_instruction ( &inst, DMAC_INST_DMASEV, pChanInfo->event_send, 0 ) : 0;
   /* DMA SEV (interrupt) */
   if (! pChanInfo->interruptDisable )
   {
      chal_dma_encode_instruction ( &inst, DMAC_INST_DMASEV, channel, 0 );
   }
   /* Flush peripheral at the end of the transfer */
   if ( pChanInfo->flushMode & CHAL_DMA_FLUSH_LAST )
   {
      /* Flush peripheral */
      if ( (src_inc == 0 ) || ( dst_inc ==0 ) ) 
      {
         /* DMAFLUSHP */
         chal_dma_encode_instruction ( &inst, DMAC_INST_DMAFLUSHP, peripheralId, 0);
      }
   }
   /* DMA END */
   chal_dma_encode_instruction ( &inst, DMAC_INST_DMAEND, 0, 0 );
   /* Mark the last micro code address */
   pChanInfo->codeEndVirtualAddress = (uint32_t) inst;

   return dataBytes;
}

/*
 * ******************************************************************************
 * 
 *  Function Name:  chal_dma_prepare_circular_transfer
 * 
 *  Description:  Prepare instruction encoding for circular transfer
 *       
 * ******************************************************************************
 */
static int chal_dma_prepare_circular_transfer
(
   CHAL_CHANNEL_HANDLE handle
)
{
   uint32_t src_addr, dst_addr, buf_len;
   uint32_t src_inc;
   uint32_t dst_inc;
   uint32_t j, cnt, descCount, src_bytes, dst_bytes;
   uint32_t loop0, loop1, ld_cnt, st_cnt, rem;
   uint8_t *inst = 0;
   uint8_t *inst_loop0, *inst_loop1, is_wait, is_send, *inst_lpfe;
   DMA_DESC_t *chanDesc;
   uint32_t control;
   DMA_CHAN_INFO_t* pChanInfo = (DMA_CHAN_INFO_t*) handle;
   CHAL_DMA_CHANNEL_t channel = pChanInfo->channel;
   uint32_t peripheralId = 2 * channel;
   int dataBytes = 0;

   /* Locate instruction (ucode) memory */
   inst = (uint8_t *) (pChanInfo->chanVirtualAddr + pChanInfo->maxDescCount * sizeof (DMA_DESC_t));
   /* Locate channel descriptors */
   chanDesc = (DMA_DESC_t *) pChanInfo->chanVirtualAddr;

   is_wait = DMAC_EVENT_VALID ( pChanInfo->event_wait ) ? 1 : 0;
   is_send = DMAC_EVENT_VALID ( pChanInfo->event_send ) ? 1 : 0;
  
   
   src_bytes = pChanInfo->srcBurstDataLen;
   dst_bytes = pChanInfo->dstBurstDataLen;
  
   if ( (src_bytes % dst_bytes) && (dst_bytes % src_bytes) ) 
   {
      debug_print(( "chal_dma_prepare_circular_transfer: invalid parameter\n"));
      return CHAL_DMA_STATUS_INVALID_PARAMETER;
   }
  
   if ( src_bytes < dst_bytes ) 
   { 
      ld_cnt = dst_bytes / src_bytes;
      st_cnt = 1;
   }
   else 
   {
      ld_cnt = 1;
      st_cnt = src_bytes / dst_bytes;
   }
   /* wait for event */
   is_wait ?  chal_dma_encode_instruction ( &inst, DMAC_INST_DMAWFE, pChanInfo->event_wait, 0 ) : 0;
   /* Flush peripheral at the begining of the transfer */
   if ( pChanInfo->flushMode & CHAL_DMA_FLUSH_FIRST )
   {
      /* Channel must be connected to a device */
      if ( ( pChanInfo->config.ChanCtrlBitField.srcAddrMode == 0 ) || (pChanInfo->config.ChanCtrlBitField.dstAddrMode == 0 ) )
      {
         /* DMAFLUSHP */
         chal_dma_encode_instruction ( &inst, DMAC_INST_DMAFLUSHP, peripheralId, 0);
      }
   }

   inst_lpfe = inst;
   /* Program for each descriptor */
   for ( descCount = 0; descCount < pChanInfo->curDescCount; descCount++ )
   {
      /* Address mode */
      src_inc = chanDesc[descCount].control.ChanCtrlBitField.srcAddrMode;
      dst_inc = chanDesc[descCount].control.ChanCtrlBitField.dstAddrMode;

      /* Flush peripheral always */
      if ( pChanInfo->flushMode == CHAL_DMA_FLUSH_ALWAYS )
      {
         if (src_inc == 0 || dst_inc ==0) 
         {
            /* DMAFLUSHP */
            chal_dma_encode_instruction ( &inst, DMAC_INST_DMAFLUSHP, peripheralId, 0);
         }   
      }
      buf_len  = chanDesc[descCount].size;
      src_addr = chanDesc[descCount].src;
      dst_addr = chanDesc[descCount].dst;
      control  = chanDesc[descCount].control.control;
      dataBytes += buf_len;
      /* Count total number of burst */
      cnt = buf_len / (src_bytes * ld_cnt);
      /* Factor the burst count */
      if ( chal_dma_get_loop_cnt ( cnt, &loop0, &loop1, &rem ) ) 
      {
	 debug_print(( "chal_dma_prepare_circular_transfer: invalid loop cnt\n"));
	 return CHAL_DMA_STATUS_INVALID_PARAMETER;
      }
      /* mov CCR */
      chal_dma_encode_instruction ( &inst, DMAC_INST_DMAMOV, 1, control );
      /* mov SRC */
      chal_dma_encode_instruction ( &inst, DMAC_INST_DMAMOV, 0, src_addr );
      /* mov DST */
      chal_dma_encode_instruction ( &inst, DMAC_INST_DMAMOV, 2, dst_addr );
      /* loop 1 */
      chal_dma_encode_instruction ( &inst, DMAC_INST_DMALP, 1, loop1 );
      inst_loop1 = inst;
      /* loop 0 */
      chal_dma_encode_instruction ( &inst, DMAC_INST_DMALP, 0, loop0 );
      inst_loop0 = inst;
      /* Encode load instructions */
      for ( j = 0; j < ld_cnt; j ++) 
      {
	 if ( src_inc )
	 {
            /* DMALD */
	    chal_dma_encode_instruction ( &inst, DMAC_INST_DMALD, 0, 0 );
#if ( CFG_GLOBAL_CHIP == BCM11160 ) 
	    /* Wait until all read completed */
            chal_dma_encode_instruction ( &inst, DMAC_INST_DMARMB, 0, 0 );
#endif

	 }
	 else if ( dst_inc )
	 {
            /* DMAWFP */
	    chal_dma_encode_instruction ( &inst, DMAC_INST_DMAWFP, pChanInfo->config.ChanCtrlBitField.srcBurstLen, peripheralId );
	    /* DMALDP */
	    chal_dma_encode_instruction ( &inst, DMAC_INST_DMALDP, pChanInfo->config.ChanCtrlBitField.srcBurstLen, peripheralId );
	 }
	 else
	 {
            /* peripheral to peripheral */
            return CHAL_DMA_STATUS_FAILURE;
	 }
      }
      /* Encode store instructions */      
      for ( j = 0; j < st_cnt; j ++ ) 
      {	
         if ( dst_inc )
	 {
            /* DMAST */
            chal_dma_encode_instruction ( &inst, DMAC_INST_DMAST, 0, 0 );
#if ( CFG_GLOBAL_CHIP == BCM11160 ) 
	    /* Wait until all write completed */
            chal_dma_encode_instruction ( &inst, DMAC_INST_DMAWMB, 0, 0 );
#endif
	 }
	 else if ( src_inc )
	 {
            /* DMAWFP */
            chal_dma_encode_instruction ( &inst, DMAC_INST_DMAWFP, pChanInfo->config.ChanCtrlBitField.dstBurstLen, peripheralId );
            /* DMASTP */
	    chal_dma_encode_instruction ( &inst, DMAC_INST_DMASTP, pChanInfo->config.ChanCtrlBitField.dstBurstLen, peripheralId );
	 }
	 else
	 {
            /* peripheral to peripheral */
            return CHAL_DMA_STATUS_FAILURE;
	 }
      }
      /* loopend 0 */
      chal_dma_encode_instruction ( &inst, DMAC_INST_DMALPEND, 0, inst - inst_loop0 );
      /* loopend 1 */
      chal_dma_encode_instruction ( &inst, DMAC_INST_DMALPEND, 1, inst - inst_loop1 );
      /* Encode remaining loop */
      if (rem) 
      {
         /* rem */
         chal_dma_encode_instruction ( &inst, DMAC_INST_DMALP, 0, rem );
         /* remember the location to jump */
         inst_loop0 = inst;
         /* Encode load instructions */
         for ( j = 0; j < ld_cnt; j ++ ) 
         {
            if ( src_inc )
            {
               /* DMALD */
               chal_dma_encode_instruction ( &inst, DMAC_INST_DMALD, 0, 0 );
#if ( CFG_GLOBAL_CHIP == BCM11160 ) 
               /* Wait until all read completed */
               chal_dma_encode_instruction ( &inst, DMAC_INST_DMARMB, 0, 0 );
#endif

            }
            else 
            {
               /* DMAWFP */
               chal_dma_encode_instruction ( &inst, DMAC_INST_DMAWFP, pChanInfo->config.ChanCtrlBitField.srcBurstLen, peripheralId );
               /* DMALDP */
               chal_dma_encode_instruction ( &inst, DMAC_INST_DMALDP, pChanInfo->config.ChanCtrlBitField.srcBurstLen, peripheralId );
            }
         }
         /* Encode store instructions */      
         for ( j = 0; j < st_cnt; j ++ ) 
         {	
            if ( dst_inc )
            {
               /* DMAST */
               chal_dma_encode_instruction ( &inst, DMAC_INST_DMAST, 0, 0 );
#if ( CFG_GLOBAL_CHIP == BCM11160 ) 
               /* Wait until all write completed */
               chal_dma_encode_instruction ( &inst, DMAC_INST_DMAWMB, 0, 0 );
#endif
            }
            else
            {
               /* DMAWFP */
               chal_dma_encode_instruction ( &inst, DMAC_INST_DMAWFP, pChanInfo->config.ChanCtrlBitField.dstBurstLen, peripheralId );
               /* DMASTP */
               chal_dma_encode_instruction ( &inst, DMAC_INST_DMASTP, pChanInfo->config.ChanCtrlBitField.dstBurstLen, peripheralId );
            }
         }
         /* DMALPEND */
         /* Jump to the begining of the loop */
         chal_dma_encode_instruction ( &inst, DMAC_INST_DMALPEND, 0, inst - inst_loop0 );
      }
      /* DMA WMB */
      chal_dma_encode_instruction ( &inst, DMAC_INST_DMAWMB, pChanInfo->event_send, 0 );
      /* DMA SEV */
      is_send ? chal_dma_encode_instruction ( &inst, DMAC_INST_DMASEV, pChanInfo->event_send, 0 ): 0;
      /* DMA SEV ( interrupt ) */
      if (! pChanInfo->interruptDisable )
      {
	 chal_dma_encode_instruction ( &inst, DMAC_INST_DMASEV, channel, 0 );
      }
   }
   /* DMALPFEEND */
   /* Continue from the begining */
   chal_dma_encode_instruction ( &inst, DMAC_INST_DMALPFEEND, 0, inst - inst_lpfe );
   /* Mark the last micro code address */
   pChanInfo->codeEndVirtualAddress = (uint32_t) inst;

   return dataBytes;
}


/*
 * ******************************************************************************
 * 
 *  Function Name:  chal_dma_prepare_transfer
 * 
 *  Description:    Prepare DMA transfer
 * 
 * ******************************************************************************
 */
int chal_dma_prepare_transfer
(
   CHAL_CHANNEL_HANDLE handle
)
{
   DMA_CHAN_INFO_t* pChanInfo = (DMA_CHAN_INFO_t*) handle;
   DMA_DEV_t *pDmaDev = (DMA_DEV_t *) pChanInfo->dmaHandle;
   int rc = 0;

   debug_print(( "chal_dma_prepare_transfer\n"));

   if ( pChanInfo->descType == CHAL_DMA_DESC_RING )
   {
      rc = chal_dma_prepare_circular_transfer ( handle );
   }
   else
   {
      rc = chal_dma_prepare_list_transfer ( handle );
   }
   
   if ( rc > 0 )
   {
      /* Wait until DMA is ready to accept command */
      while ( CHAL_REG_READ32 ( pDmaDev->baseAddr + NON_DMAC_DBGSTATUS_OFFSET ) & 0x1 ) ;
   }

   /* Return status */
   return rc;
}

/*
 * ******************************************************************************
 * 
 *  Function Name:  chal_dma_start_transfer
 * 
 *  Description:    Start a DMA transfer
 * 
 * ******************************************************************************
 */
CHAL_DMA_STATUS_t chal_dma_start_transfer
(
   CHAL_CHANNEL_HANDLE handle
)
{
   uint32_t val, time_out;    
   DMA_CHAN_INFO_t* pChanInfo = (DMA_CHAN_INFO_t*) handle;
   DMA_DEV_t *pDmaDev = (DMA_DEV_t *) pChanInfo->dmaHandle;
   CHAL_DMA_CHANNEL_t channel = pChanInfo->channel;
   CHAL_DMA_STATUS_t rc = CHAL_DMA_STATUS_SUCCESS;

   debug_print(( "chal_dma_start_transfer\n"));

   if ( ( rc = chal_dma_get_channel_status ( handle ) ) != CHAL_DMA_STATUS_SUCCESS )
   {
      return rc;
   }

   /* Try to aquire DMA lock */
   if ( chal_dmux_protect ( pDmaDev->config->dmuxHandle ) == CHAL_DMUX_STATUS_SUCCESS )
   {
      /* Encode DMAGO */
      if( pDmaDev->secState == CHAL_DMA_STATE_SECURE )
      {
	 val = dmacInstEncode[DMAC_INST_DMAGO].inst_enc | CHAL_DMA_DMAGO_SECURE;
      }
      else
      {
	 val = dmacInstEncode[DMAC_INST_DMAGO].inst_enc | CHAL_DMA_DMAGO_OPEN;
      }
      /* Configure debug register 0 */
      /* Sepecify channel number in the DMAGO instruction */
      val = val | channel << CHAL_DMA_DMAGO_CHANNEL_NUM_SHIFT;
      val = val << NON_DMAC_DBGINST0_INSTR0_SHIFT;
      /* Set channel number */
      val = val | channel << NON_DMAC_DBGINST0_CHANNEL_NUM_SHIFT;
      /* Set debug register 0 */
      CHAL_REG_WRITE32 ( pDmaDev->baseAddr + NON_DMAC_DBGINST0_OFFSET, val );
   
      /* Locate physical address of the micro-code */
      val = pChanInfo->chanPhysicalAddr + pChanInfo->maxDescCount * sizeof (DMA_DESC_t);
      /* Specify the location of the ucode */
      CHAL_REG_WRITE32 ( pDmaDev->baseAddr + NON_DMAC_DBGINST1_OFFSET, val );
   
      /* Configure interrupt */
      if ( pChanInfo->interruptDisable )
      {
	 val = CHAL_REG_READ32 ( pDmaDev->baseAddr + NON_DMAC_INTEN_OFFSET ) & (~( 1 << channel));
      }
      else
      {
	 val = CHAL_REG_READ32 ( pDmaDev->baseAddr + NON_DMAC_INTEN_OFFSET ) | ( 1 << channel);	 
      }
      CHAL_REG_WRITE32 ( pDmaDev->baseAddr + NON_DMAC_INTEN_OFFSET, val );
      /* Execute instructions written in Debug register 0 and 1 */
      val = 0;
      CHAL_REG_WRITE32 ( pDmaDev->baseAddr + NON_DMAC_DBGCMD_OFFSET, val );
      
      time_out = 256;
      while ( CHAL_REG_READ32 ( pDmaDev->baseAddr + NON_DMAC_DBGSTATUS_OFFSET ) & 0x1 && time_out ) time_out--;

      chal_dmux_unprotect ( pDmaDev->config->dmuxHandle );

      if ( !time_out ) 
      {
	 debug_print(( "chal_dma_start_transfer: time out\n"));
	 return CHAL_DMA_STATUS_TIMEOUT;
      }
   }
   else
   {
      return CHAL_DMA_STATUS_FAILURE;
   }

   return CHAL_DMA_STATUS_SUCCESS;   
}


/*
 * ******************************************************************************
 * 
 *  Function Name:  chal_dma_get_int_status
 * 
 *  Description:    Returns the DMA controller's interrupt status
 * 
 * ******************************************************************************
 */
uint32_t chal_dma_get_int_status ( CHAL_HANDLE handle )
{
   DMA_DEV_t *pDmaDev = (DMA_DEV_t *)handle;
    
   return CHAL_REG_READ32 ( pDmaDev->baseAddr + NON_DMAC_INTSTATUS_OFFSET );
}


/*
 * ******************************************************************************
 * 
 *  Function Name:  chal_dma_handle_to_channel
 * 
 *  Description:    Returns channel from channel handle
 * 
 * ******************************************************************************
 */
CHAL_DMA_CHANNEL_t chal_dma_handle_to_channel ( CHAL_CHANNEL_HANDLE handle )
{
   DMA_CHAN_INFO_t* pChanInfo = (DMA_CHAN_INFO_t*) handle;

   return pChanInfo->channel;
}


/*
 * ******************************************************************************
 * 
 *  Function Name:  chal_dma_channel_to_handle
 * 
 *  Description:    Returns channel handle from channel
 * 
 * ******************************************************************************
 */
CHAL_CHANNEL_HANDLE chal_dma_channel_to_handle ( CHAL_HANDLE handle, CHAL_DMA_CHANNEL_t channel )
{
   DMA_DEV_t *pDmaDev = (DMA_DEV_t *)handle;
   DMA_CHAN_INFO_t* pChanInfo = &pDmaDev->config->channelInfo[channel];

   return ( CHAL_CHANNEL_HANDLE ) pChanInfo;
}


/*
 * ******************************************************************************
 * 
 *  Function Name:  chal_dma_get_int_source
 * 
 *  Description:    Returns the channel that triggers interrupt
 * 
 * ******************************************************************************
 */
CHAL_DMA_CHANNEL_t chal_dma_get_int_source ( CHAL_HANDLE handle )
{

   uint32_t status = chal_dma_get_int_status ( handle );
   CHAL_DMA_CHANNEL_t i = CHAL_DMA_CHANNEL_0;
   
   for ( i = CHAL_DMA_CHANNEL_0; i < CHAL_TOTAL_DMA_CHANNELS; i++ ) 
   {
      if ( status & ( 0x1 << i ) ) 
      {
         return i;
      }
   }
   return CHAL_DMA_CHANNEL_NULL;
}


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
CHAL_DMA_SECURE_STATE_t chal_dma_get_channel_state ( CHAL_CHANNEL_HANDLE handle )
{
   DMA_CHAN_INFO_t* pChanInfo = (DMA_CHAN_INFO_t*) handle;
   DMA_DEV_t *pDmaDev = (DMA_DEV_t *) pChanInfo->dmaHandle;

   return pDmaDev->secState;
}

/*
 * ******************************************************************************
 * 
 *  Function Name:  chal_dma_get_channel_status
 * 
 *  Description:    Returns the DMA channel status after transfer
 * 
 * ******************************************************************************
 */
CHAL_DMA_STATUS_t chal_dma_get_channel_status ( CHAL_CHANNEL_HANDLE handle )
{
   uint32_t status;
   DMA_CHAN_INFO_t* pChanInfo = (DMA_CHAN_INFO_t*) handle;
   DMA_DEV_t *pDmaDev = (DMA_DEV_t *) pChanInfo->dmaHandle;
   CHAL_DMA_CHANNEL_t channel = pChanInfo->channel;
   
   status = CHAL_REG_READ32 ( pDmaDev->baseAddr + NON_DMAC_CS0_OFFSET + ( channel << 3 ) );

   return (CHAL_DMA_STATUS_t)( status & NON_DMAC_CS0_CHANNEL_STATUS_MASK);
}

/*
 * ******************************************************************************
 * 
 *  Function Name:  chal_dma_interrupt_enable
 * 
 *  Description:    Enable DMA interrupt
 * 
 * ******************************************************************************
 */
void chal_dma_interrupt_enable
(
   CHAL_CHANNEL_HANDLE handle 
)
{
   DMA_CHAN_INFO_t* pChanInfo = (DMA_CHAN_INFO_t*) handle;
   DMA_DEV_t *pDmaDev = (DMA_DEV_t *) pChanInfo->dmaHandle;
   CHAL_DMA_CHANNEL_t channel = pChanInfo->channel;
   volatile uint32_t reg;

   reg = CHAL_REG_READ32 ( pDmaDev->baseAddr + NON_DMAC_INTEN_OFFSET );
   reg |= (0x1 << channel);
   CHAL_REG_WRITE32 ( pDmaDev->baseAddr + NON_DMAC_INTEN_OFFSET, reg );
}

/*
 * ******************************************************************************
 * 
 *  Function Name:  chal_dma_interrupt_disable
 * 
 *  Description:    Disable DMA interrupt
 * 
 * ******************************************************************************
 */
void chal_dma_interrupt_disable
(
   CHAL_CHANNEL_HANDLE handle 
)
{
   DMA_CHAN_INFO_t* pChanInfo = (DMA_CHAN_INFO_t*) handle;
   DMA_DEV_t *pDmaDev = (DMA_DEV_t *) pChanInfo->dmaHandle;
   CHAL_DMA_CHANNEL_t channel = pChanInfo->channel;
   volatile uint32_t reg;

   reg = CHAL_REG_READ32 ( pDmaDev->baseAddr + NON_DMAC_INTEN_OFFSET );
   reg &= ~(0x1 << channel);
   CHAL_REG_WRITE32 ( pDmaDev->baseAddr + NON_DMAC_INTEN_OFFSET, reg );
}


/*
 * ******************************************************************************
 * 
 *  Function Name:  chal_dma_dump_register
 * 
 *  Description:    Function to dump DMA registers
 * 
 * ******************************************************************************
 */
void chal_dma_dump_register ( CHAL_CHANNEL_HANDLE handle,  int (*fpPrint) (const char *, ...) )
{

   DMA_CHAN_INFO_t* pChanInfo = (DMA_CHAN_INFO_t*) handle;
   DMA_DEV_t *pDmaDev = (DMA_DEV_t *) pChanInfo->dmaHandle;
   uint32_t baseAddr = pDmaDev->baseAddr;
   uint8_t * startCode, * endCode, * phyAddr;
   uint32_t count;

   (*fpPrint) ( "\nDebugging for channel # %d\n",    pChanInfo->channel );

   /* Locate physical address of the micro-code */
   (*fpPrint) ( "\nChannel Micro code:\n" );

   startCode = (uint8_t *) ( pChanInfo->chanVirtualAddr + pChanInfo->maxDescCount * sizeof (DMA_DESC_t) );
   phyAddr   = (uint8_t *) ( pChanInfo->chanPhysicalAddr + pChanInfo->maxDescCount * sizeof (DMA_DESC_t) );
   endCode   = (uint8_t *) pChanInfo->codeEndVirtualAddress;

   count = 0;
   while ( startCode <= endCode )
   {
      if ( ( count % 16 ) == 0)
      {
         (*fpPrint) ( "\n0x%08X : ", phyAddr + count);
      }
      (*fpPrint) ( "0x%02X ", *startCode );
      count++;
      startCode++;
   }

   (*fpPrint) ( "\nDMA Status 0x%08X\n",           *(volatile uint32_t*)(baseAddr + 0x0) );
   (*fpPrint) ( "DMA Prog Counter 0x%08X\n",       *(volatile uint32_t*)(baseAddr + 0x4) );
   (*fpPrint) ( "Interrupt enable 0x%08X\n",       *(volatile uint32_t*)(baseAddr + 0x20) );
   (*fpPrint) ( "Event status 0x%08X\n",           *(volatile uint32_t*)(baseAddr + 0x24) );
   (*fpPrint) ( "Interrupt status 0x%08X\n",       *(volatile uint32_t*)(baseAddr + 0x28) );
   (*fpPrint) ( "Interrupt clear 0x%08X\n",        *(volatile uint32_t*)(baseAddr + 0x2c) );
   (*fpPrint) ( "Fault status (manager) 0x%08X\n", *(volatile uint32_t*)(baseAddr + 0x30) );
   (*fpPrint) ( "Fault status (channel) 0x%08X\n", *(volatile uint32_t*)(baseAddr + 0x34) );
   (*fpPrint) ( "Fault type (manager) 0x%08X\n",   *(volatile uint32_t*)(baseAddr + 0x38) );
   (*fpPrint) ( "Fault type (channel 0) 0x%08X\n", *(volatile uint32_t*)(baseAddr + 0x40) );
   (*fpPrint) ( "Fault type (channel 1) 0x%08X\n", *(volatile uint32_t*)(baseAddr + 0x44) );
   (*fpPrint) ( "Fault type (channel 2) 0x%08X\n", *(volatile uint32_t*)(baseAddr + 0x48) );
   (*fpPrint) ( "Fault type (channel 3) 0x%08X\n", *(volatile uint32_t*)(baseAddr + 0x4c) );
   (*fpPrint) ( "Fault type (channel 4) 0x%08X\n", *(volatile uint32_t*)(baseAddr + 0x50) );
   (*fpPrint) ( "Fault type (channel 5) 0x%08X\n", *(volatile uint32_t*)(baseAddr + 0x54) );
   (*fpPrint) ( "Fault type (channel 6) 0x%08X\n", *(volatile uint32_t*)(baseAddr + 0x58) );
   (*fpPrint) ( "Fault type (channel 7) 0x%08X\n", *(volatile uint32_t*)(baseAddr + 0x5c) );
   (*fpPrint) ( "Channel 0 status 0x%08X\n",       *(volatile uint32_t*)(baseAddr + 0x100) );
   (*fpPrint) ( "Channel 1 status 0x%08X\n",       *(volatile uint32_t*)(baseAddr + 0x108) );
   (*fpPrint) ( "Channel 2 status 0x%08X\n",       *(volatile uint32_t*)(baseAddr + 0x110) );
   (*fpPrint) ( "Channel 3 status 0x%08X\n",       *(volatile uint32_t*)(baseAddr + 0x118) );
   (*fpPrint) ( "Channel 4 status 0x%08X\n",       *(volatile uint32_t*)(baseAddr + 0x120) );
   (*fpPrint) ( "Channel 5 status 0x%08X\n",       *(volatile uint32_t*)(baseAddr + 0x128) );
   (*fpPrint) ( "Channel 6 status 0x%08X\n",       *(volatile uint32_t*)(baseAddr + 0x130) );
   (*fpPrint) ( "Channel 7 status 0x%08X\n",       *(volatile uint32_t*)(baseAddr + 0x138) );
   (*fpPrint) ( "Channel 0 PC 0x%08X\n",           *(volatile uint32_t*)(baseAddr + 0x104) );
   (*fpPrint) ( "Channel 1 PC 0x%08X\n",           *(volatile uint32_t*)(baseAddr + 0x10c) );
   (*fpPrint) ( "Channel 2 PC 0x%08X\n",           *(volatile uint32_t*)(baseAddr + 0x114) );
   (*fpPrint) ( "Channel 3 PC 0x%08X\n",           *(volatile uint32_t*)(baseAddr + 0x11c) );
   (*fpPrint) ( "Channel 4 PC 0x%08X\n",           *(volatile uint32_t*)(baseAddr + 0x124) );
   (*fpPrint) ( "Channel 5 PC 0x%08X\n",           *(volatile uint32_t*)(baseAddr + 0x12c) );
   (*fpPrint) ( "Channel 6 PC 0x%08X\n",           *(volatile uint32_t*)(baseAddr + 0x134) );
   (*fpPrint) ( "Channel 7 PC 0x%08X\n",           *(volatile uint32_t*)(baseAddr + 0x13c) );
   (*fpPrint) ( "Channel 0 SAR 0x%08X\n",          *(volatile uint32_t*)(baseAddr + 0x400) );
   (*fpPrint) ( "Channel 1 SAR 0x%08X\n",          *(volatile uint32_t*)(baseAddr + 0x420) );
   (*fpPrint) ( "Channel 2 SAR 0x%08X\n",          *(volatile uint32_t*)(baseAddr + 0x440) );
   (*fpPrint) ( "Channel 3 SAR 0x%08X\n",          *(volatile uint32_t*)(baseAddr + 0x460) );
   (*fpPrint) ( "Channel 4 SAR 0x%08X\n",          *(volatile uint32_t*)(baseAddr + 0x480) );
   (*fpPrint) ( "Channel 5 SAR 0x%08X\n",          *(volatile uint32_t*)(baseAddr + 0x4A0) );
   (*fpPrint) ( "Channel 6 SAR 0x%08X\n",          *(volatile uint32_t*)(baseAddr + 0x4c0) );
   (*fpPrint) ( "Channel 7 SAR 0x%08X\n",          *(volatile uint32_t*)(baseAddr + 0x4e0) );
   (*fpPrint) ( "Channel 0 DAR 0x%08X\n",          *(volatile uint32_t*)(baseAddr + 0x404) );
   (*fpPrint) ( "Channel 1 DAR 0x%08X\n",          *(volatile uint32_t*)(baseAddr + 0x424) );
   (*fpPrint) ( "Channel 2 DAR 0x%08X\n",          *(volatile uint32_t*)(baseAddr + 0x444) );
   (*fpPrint) ( "Channel 3 DAR 0x%08X\n",          *(volatile uint32_t*)(baseAddr + 0x464) );
   (*fpPrint) ( "Channel 4 DAR 0x%08X\n",          *(volatile uint32_t*)(baseAddr + 0x484) );
   (*fpPrint) ( "Channel 5 DAR 0x%08X\n",          *(volatile uint32_t*)(baseAddr + 0x4a4) );
   (*fpPrint) ( "Channel 6 DAR 0x%08X\n",          *(volatile uint32_t*)(baseAddr + 0x4c4) );
   (*fpPrint) ( "Channel 7 DAR 0x%08X\n",          *(volatile uint32_t*)(baseAddr + 0x4e4) );
   (*fpPrint) ( "Channel 0 control 0x%08X\n",      *(volatile uint32_t*)(baseAddr + 0x408) );
   (*fpPrint) ( "Channel 1 control 0x%08X\n",      *(volatile uint32_t*)(baseAddr + 0x428) );
   (*fpPrint) ( "Channel 2 control 0x%08X\n",      *(volatile uint32_t*)(baseAddr + 0x448) );
   (*fpPrint) ( "Channel 3 control 0x%08X\n",      *(volatile uint32_t*)(baseAddr + 0x468) );
   (*fpPrint) ( "Channel 4 control 0x%08X\n",      *(volatile uint32_t*)(baseAddr + 0x488) );
   (*fpPrint) ( "Channel 5 control 0x%08X\n",      *(volatile uint32_t*)(baseAddr + 0x4a8) );
   (*fpPrint) ( "Channel 6 control 0x%08X\n",      *(volatile uint32_t*)(baseAddr + 0x4c8) );
   (*fpPrint) ( "Channel 7 control 0x%08X\n",      *(volatile uint32_t*)(baseAddr + 0x4e8) );
   (*fpPrint) ( "Channel 0 LC0 0x%08X\n",          *(volatile uint32_t*)(baseAddr + 0x40c) );
   (*fpPrint) ( "Channel 1 LC0 0x%08X\n",          *(volatile uint32_t*)(baseAddr + 0x42c) );
   (*fpPrint) ( "Channel 2 LC0 0x%08X\n",          *(volatile uint32_t*)(baseAddr + 0x44c) );
   (*fpPrint) ( "Channel 3 LC0 0x%08X\n",          *(volatile uint32_t*)(baseAddr + 0x46c) );
   (*fpPrint) ( "Channel 4 LC0 0x%08X\n",          *(volatile uint32_t*)(baseAddr + 0x48c) );
   (*fpPrint) ( "Channel 5 LC0 0x%08X\n",          *(volatile uint32_t*)(baseAddr + 0x4ac) );
   (*fpPrint) ( "Channel 6 LC0 0x%08X\n",          *(volatile uint32_t*)(baseAddr + 0x4cc) );
   (*fpPrint) ( "Channel 7 LC0 0x%08X\n",          *(volatile uint32_t*)(baseAddr + 0x4ec) );
   (*fpPrint) ( "Channel 0 LC1 0x%08X\n",          *(volatile uint32_t*)(baseAddr + 0x410) );
   (*fpPrint) ( "Channel 1 LC1 0x%08X\n",          *(volatile uint32_t*)(baseAddr + 0x430) );
   (*fpPrint) ( "Channel 2 LC1 0x%08X\n",          *(volatile uint32_t*)(baseAddr + 0x450) );
   (*fpPrint) ( "Channel 3 LC1 0x%08X\n",          *(volatile uint32_t*)(baseAddr + 0x470) );
   (*fpPrint) ( "Channel 4 LC1 0x%08X\n",          *(volatile uint32_t*)(baseAddr + 0x490) );
   (*fpPrint) ( "Channel 5 LC1 0x%08X\n",          *(volatile uint32_t*)(baseAddr + 0x4b0) );
   (*fpPrint) ( "Channel 6 LC1 0x%08X\n",          *(volatile uint32_t*)(baseAddr + 0x4d0) );
   (*fpPrint) ( "Channel 7 LC1 0x%08X\n",          *(volatile uint32_t*)(baseAddr + 0x4f0) );
   (*fpPrint) ( "Debug status 0x%08X\n",           *(volatile uint32_t*)(baseAddr + 0xd00) );
   (*fpPrint) ( "Debug command 0x%08X\n",          *(volatile uint32_t*)(baseAddr + 0xd04) );
   (*fpPrint) ( "Debug inst0 0x%08X\n",            *(volatile uint32_t*)(baseAddr + 0xd08) );
   (*fpPrint) ( "Debug inst1 0x%08X\n",            *(volatile uint32_t*)(baseAddr + 0xd0c) );

}
