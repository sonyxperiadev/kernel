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
*   @file   ssasw.c
*
*   @brief  Implements the Semi-Synchronous Audio Switch interface.
*/
/****************************************************************************/

/* ---- Include Files ---------------------------------------------------- */

#include <linux/module.h>
#include <linux/device.h>
//#include <linux/dma-mapping.h>
#include <linux/interrupt.h>
#include <linux/irqreturn.h>
#include <linux/proc_fs.h>
#include <linux/hugetlb.h>
#include <linux/version.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/pfn.h>
#include <linux/clk.h>

#include <linux/broadcom/halaudio.h>

//#include <linux/broadcom/timer.h>
#ifdef CONFIG_BCM_KNLLOG_IRQ
#include <linux/broadcom/knllog.h>
#endif

#include <asm/atomic.h>

#include <mach/ssasw.h>
#include <mach/rdb/brcm_rdb_sysmap.h>

#include <chal/chal_caph.h>
#include <chal/chal_caph_switch.h>
#include <chal/chal_caph_srcmixer.h>

/* ---- Public Variables ------------------------------------------------- */

/**
 * dB to 16-bit Q14 multiplier map. Used in setting software gains.
 * Ranges from -50dB to +18db (16384 = 0dB)
 */
const static short q14GainMap[] =
{
   52, 60, 64, 72, 84, 92, 104, 116, 132, 148,
   164, 184, 208, 232, 260, 292, 328, 368, 412, 460,
   520, 580, 652, 732, 820, 920, 1032, 1160, 1300, 1460,
   1640, 1840, 2064, 2316, 2596, 2912, 3268, 3668, 4116, 4616,
   5180, 5812, 6524, 7320, 8212, 9212, 10336, 11600, 13016, 14604,
   16384, 18384, 20628, 23144, 25968, 29136, 32692,
};

/* ---- Private Constants and Types -------------------------------------- */

#define SSASW_PHYS_BASE_ADDR_START        ( SSASW_BASE_ADDR )
#define SRCMIXER_PHYS_BASE_ADDR_START     ( SRCMIXER_BASE_ADDR )

#define SSASW_REGISTER_LENGTH             0x800
#define SRCMIXER_REGISTER_LENGTH          0xD00    /* Arbitrarily longer than required */

#define MAKE_HANDLE( channel_src_idx, channel_idx )    (( (channel_src_idx) << 4 ) | (channel_idx) )

#define CHANNEL_FROM_HANDLE( handle )       ( (handle) & 0x0f )
#define CHANNEL_SRC_FROM_HANDLE( handle )   ( (handle >> 4) & 0x0f )

#define SRCMIXER_CH_DEF_BITSEL            0x4
#define SRCMIXER_CH_DEF_GAIN              0x1000
#define SRCMIXER_CH_DEF_GAIN_STEP         0x7FFF

/* Q14 gain map min and max ranges */
#define MAX_Q14_GAIN                6
#define MIN_Q14_GAIN                -50

#define DEVICE_SSASW_AUDIOH_VINR \
{ \
   .name = "SSASW VINR", \
   .dev_sw_trig = CAPH_ADC_VOICE_FIFO_THR_MET, \
   .sw_data_format = CAPH_MONO_16BIT, \
}

#define DEVICE_SSASW_AUDIOH_VINL \
{ \
   .name = "SSASW VINL", \
   .dev_sw_trig = CAPH_ADC_VOICE_L_FIFO_THR_MET, \
   .sw_data_format = CAPH_MONO_16BIT, \
}

#define DEVICE_SSASW_AUDIOH_NVINR \
{ \
   .name = "SSASW NVINR", \
   .dev_sw_trig = CAPH_ADC_NOISE_FIFO_THR_MET, \
   .sw_data_format = CAPH_MONO_16BIT, \
}

#define DEVICE_SSASW_AUDIOH_NVINL \
{ \
   .name = "SSASW NVINL", \
   .dev_sw_trig = CAPH_ADC_NOISE_L_FIFO_THR_MET, \
   .sw_data_format = CAPH_MONO_16BIT, \
}

#define DEVICE_SSASW_AUDIOH_EARPIECE \
{ \
   .name = "SSASW Earpiece", \
   .dev_sw_trig = CAPH_EP_THR_MET, \
   .sw_data_format = CAPH_MONO_16BIT, \
}

#define DEVICE_SSASW_AUDIOH_HEADSET \
{ \
   .name = "SSASW Headset", \
   .dev_sw_trig = CAPH_HS_THR_MET, \
   .sw_data_format = CAPH_MONO_16BIT, \
}

#define DEVICE_SSASW_AUDIOH_HANDSFREE \
{ \
   .name = "SSASW Handsfree", \
   .dev_sw_trig = CAPH_IHF_THR_MET, \
   .sw_data_format = CAPH_MONO_16BIT, \
}

#define DEVICE_SSASW_AUDIOH_VIBRA \
{ \
   .name = "SSASW Vibra", \
   .dev_sw_trig = CAPH_VB_THR_MET, \
   .sw_data_format = CAPH_MONO_16BIT, \
}

#define DEVICE_SSASW_SRC_44100_AUDIOH_EARPIECE \
{ \
   .name = "SSASW 44100 Earpiece",           \
   .dev_sw_trig = CAPH_EP_THR_MET,           \
   .sw_data_format = CAPH_MONO_16BIT,        \
   .src_sw_trig = CAPH_VOID,                 \
   .src_in_fifo = CAPH_CH_INFIFO_NONE,       \
   .src_out_fifo = CAPH_MIXER2_OUTFIFO1,     \
   .mix_src_ch = CAPH_SRCM_CH5_R,            \
   .mix_dst_ch = CAPH_M1_Right,              \
   .mix_dig_gain = SRCMIXER_CH_DEF_GAIN,     \
   .src_out_data_format = CAPH_MONO_16BIT,   \
}

#define DEVICE_SSASW_SRC_44100_AUDIOH_HEADSET      \
{ \
   .name = "SSASW 44100 Headset",                  \
   .dev_sw_trig = CAPH_HS_THR_MET,                 \
   .sw_data_format = CAPH_MONO_16BIT,              \
   .src_sw_trig = CAPH_TAPSUP_CH5_NORM_INT,        \
   .src_in_fifo = CAPH_CH5_INFIFO,                 \
   .src_out_fifo = CAPH_MIXER1_OUTFIFO,            \
   .src_in_data_format = CAPH_STEREO_16BIT,        \
   .src_out_data_format = CAPH_STEREO_16BIT,       \
   .mix_freq = CAPH_44_1KHz_48KHz,                 \
   .mix_src_ch = CAPH_SRCM_CH5,                    \
   .mix_dst_ch = (CAPH_M0_Left | CAPH_M0_Right),   \
   .mix_dig_gain = SRCMIXER_CH_DEF_GAIN,           \
}

#define DEVICE_SSASW_SRC_44100_AUDIOH_HANDSFREE \
{ \
   .name = "SSASW 44100 Handsfree",          \
   .dev_sw_trig = CAPH_IHF_THR_MET,          \
   .sw_data_format = CAPH_MONO_16BIT,        \
   .src_sw_trig = CAPH_VOID,                 \
   .src_in_fifo = CAPH_CH_INFIFO_NONE,       \
   .src_out_fifo = CAPH_MIXER2_OUTFIFO2,     \
   .mix_src_ch = CAPH_SRCM_CH5_L,            \
   .mix_dst_ch = CAPH_M1_Left,               \
   .mix_dig_gain = SRCMIXER_CH_DEF_GAIN,     \
   .src_out_data_format = CAPH_MONO_16BIT,   \
}

/* ---- Private Variables ------------------------------------------------ */
static SSASW_Global_t            gSSASW;
static SSASW_ChalHandle_t        gChalHandle;

static spinlock_t                gHwSsaswLock;

static struct clk                *gSRCMixer_clk;


SSASW_DeviceAttribute_t SSASW_gDeviceAttribute[ SSASW_NUM_DEVICE_ENTRIES ] =
{
   [SSASW_AUDIOH_VINR]                 = DEVICE_SSASW_AUDIOH_VINR,
   [SSASW_AUDIOH_VINL]                 = DEVICE_SSASW_AUDIOH_VINL,
   [SSASW_AUDIOH_NVINR]                = DEVICE_SSASW_AUDIOH_NVINR,
   [SSASW_AUDIOH_NVINL]                = DEVICE_SSASW_AUDIOH_NVINL,
   [SSASW_AUDIOH_EARPIECE]             = DEVICE_SSASW_AUDIOH_EARPIECE,
   [SSASW_AUDIOH_HEADSET]              = DEVICE_SSASW_AUDIOH_HEADSET,
   [SSASW_AUDIOH_HANDSFREE]            = DEVICE_SSASW_AUDIOH_HANDSFREE,
   [SSASW_AUDIOH_VIBRA]                = DEVICE_SSASW_AUDIOH_VIBRA,
   [SSASW_SRC_44100_AUDIOH_EARPIECE]   = DEVICE_SSASW_SRC_44100_AUDIOH_EARPIECE,
   [SSASW_SRC_44100_AUDIOH_HEADSET]    = DEVICE_SSASW_SRC_44100_AUDIOH_HEADSET,
   [SSASW_SRC_44100_AUDIOH_HANDSFREE]  = DEVICE_SSASW_SRC_44100_AUDIOH_HANDSFREE,
};

EXPORT_SYMBOL( SSASW_gDeviceAttribute );

/* ---- Private Function Prototypes -------------------------------------- */

static inline SSASW_Channel_t *HandleToChannel( SSASW_Handle_t handle )
{
    int channel_idx = CHANNEL_FROM_HANDLE( handle );
    if ( channel_idx > SSASW_NUM_CHANNELS )
    {
        return NULL;
    }
    return &gSSASW.channel[ channel_idx ];
}

static inline SSASW_Channel_t *HandleToChannelSrc( SSASW_Handle_t handle )
{
    int channel_idx = CHANNEL_SRC_FROM_HANDLE( handle );
    if ( channel_idx > SSASW_NUM_CHANNELS )
    {
        return NULL;
    }
    return &gSSASW.channel[ channel_idx ];
}

/***************************************************************************/
/**
*  Translate db to Q14 linear gain value. The db value will also be
*  range limited
*
*  @return
*     +ve number  - linear gain value
*/
inline static short dbToLinearQ14(
   int     *db                      /**< (io) Ptr to gain in db */
)
{
   int dbval = *db;

   if( dbval > MAX_Q14_GAIN )
   {
      *db = MAX_Q14_GAIN;
      return 0x7fff;        /* maximum gain */
   }

   if( dbval < MIN_Q14_GAIN )
   {
      *db = HALAUDIO_GAIN_MUTE;
      return 0;             /* mute */
   }

   /* Map db to mapped linear value */
   return q14GainMap[dbval - MIN_Q14_GAIN];
}

static int ssasw_ioremap_init( SSASW_ChalHandle_t *chal_handle )
{
   void __iomem *ssasw_virt_addr = 0;
   void __iomem *srcmixer_virt_addr = 0;

   struct resource *ssasw_ioarea;
   struct resource *srcmixer_ioarea;

   ssasw_ioarea = request_mem_region( SSASW_PHYS_BASE_ADDR_START, SSASW_REGISTER_LENGTH, "SSASW Component Mem Region" );
   srcmixer_ioarea = request_mem_region( SRCMIXER_PHYS_BASE_ADDR_START, SRCMIXER_REGISTER_LENGTH, "SRC Mixer Component Mem Region" );

   if( !ssasw_ioarea || !srcmixer_ioarea )
   {
      return -EBUSY;
   }

   ssasw_virt_addr = ioremap( SSASW_PHYS_BASE_ADDR_START, SSASW_REGISTER_LENGTH );
   srcmixer_virt_addr = ioremap( SRCMIXER_PHYS_BASE_ADDR_START, SRCMIXER_REGISTER_LENGTH );

   if( !ssasw_virt_addr || !srcmixer_virt_addr )
   {
      return -ENOMEM;
   }

   chal_handle->chalSsaswHandle = chal_caph_switch_init((uint32_t)ssasw_virt_addr);
   chal_handle->chalSrcMixerHandle = chal_caph_srcmixer_init((uint32_t)srcmixer_virt_addr);

   chal_caph_srcmixer_clr_all_tapbuffers( chal_handle->chalSrcMixerHandle );

   chal_caph_srcmixer_set_spkrgain_bitsel( chal_handle->chalSrcMixerHandle, CAPH_M0_Left, SRCMIXER_CH_DEF_BITSEL);
   chal_caph_srcmixer_set_spkrgain_bitsel( chal_handle->chalSrcMixerHandle, CAPH_M0_Right, SRCMIXER_CH_DEF_BITSEL );
   chal_caph_srcmixer_set_spkrgain_bitsel( chal_handle->chalSrcMixerHandle, CAPH_M1_Left, SRCMIXER_CH_DEF_BITSEL );
   chal_caph_srcmixer_set_spkrgain_bitsel( chal_handle->chalSrcMixerHandle, CAPH_M1_Right, SRCMIXER_CH_DEF_BITSEL );

   return 0;
}

static int ssasw_ioremap_exit( SSASW_ChalHandle_t *chal_handle )
{
   chal_caph_srcmixer_set_spkrgain_bitsel( chal_handle->chalSrcMixerHandle, CAPH_M0_Left, 0x0000);
   chal_caph_srcmixer_set_spkrgain_bitsel( chal_handle->chalSrcMixerHandle, CAPH_M0_Right, 0x0000);
   chal_caph_srcmixer_set_spkrgain_bitsel( chal_handle->chalSrcMixerHandle, CAPH_M1_Left, 0x0000);
   chal_caph_srcmixer_set_spkrgain_bitsel( chal_handle->chalSrcMixerHandle, CAPH_M1_Right, 0x0000);

   chal_caph_switch_deinit( chal_handle->chalSsaswHandle );
   chal_caph_srcmixer_deinit( chal_handle->chalSrcMixerHandle );
   release_mem_region( SSASW_PHYS_BASE_ADDR_START, SSASW_REGISTER_LENGTH );
   release_mem_region( SRCMIXER_PHYS_BASE_ADDR_START, SRCMIXER_REGISTER_LENGTH );

   return 0;
}

static int ssasw_set_clock( SSASW_ChalHandle_t *chal_handle, int enable )
{
   int err = 0;

   if( enable )
   {
      gSRCMixer_clk = clk_get( NULL, "caph_srcmixer_clk" );
      err = clk_set_rate( gSRCMixer_clk, 26000000 );
      if ( err )
      {
         printk( KERN_ERR "%s: failed to set rate on CAPH SRC Mixer clock %d!\n", __FUNCTION__, err );
         return err;
      }

      err = clk_enable( gSRCMixer_clk );
      if ( err )
      {
         printk( KERN_ERR "%s: failed to enable CAPH SRC Mixer clock %d!\n", __FUNCTION__, err );
         return err;
      }

      chal_caph_switch_enable_clock_bypass( chal_handle->chalSsaswHandle );
      chal_caph_switch_enable_clock( chal_handle->chalSsaswHandle );
   }
   else
   {
      chal_caph_switch_disable_clock_bypass( chal_handle->chalSsaswHandle );
      chal_caph_switch_disable_clock( chal_handle->chalSsaswHandle );

      clk_disable( gSRCMixer_clk );
      clk_put( gSRCMixer_clk );
   }
   return 0;
}

/* ---- Functions  ------------------------------------------------------- */

/****************************************************************************/
/**
*   Initializes the SSASW module.
*
*   @return
*       0       - Success
*       < 0     - Error
*/
/****************************************************************************/
int ssasw_init( void )
{
   int rc = 0;
   int channel_idx;
   SSASW_Channel_t *channel;

   printk( "%s: initializing SSASW\n", __func__ );

   if( (rc = ssasw_ioremap_init( &gChalHandle )) != 0 )
   {
      printk( KERN_ERR "Failed to perform i/o remap\n" );
      return rc;
   }

   ssasw_set_clock( &gChalHandle, 1 );

   sema_init( &gSSASW.lock, 1 );

   spin_lock_init( &gHwSsaswLock );

   for ( channel_idx = 0; channel_idx < SSASW_NUM_CHANNELS; channel_idx++ )
   {
      channel = &gSSASW.channel[ channel_idx ];

      channel->inUse = 0;
      channel->devType = SSASW_DEVICE_NONE;

      channel->caph_switch_ch = CAPH_SWITCH_CH_VOID;
   }

   return rc;
}

EXPORT_SYMBOL(ssasw_init);

/****************************************************************************/
/**
*   Exits the SSASW module.
*
*   @return
*       0       - Success
*       < 0     - Error
*/
/****************************************************************************/
void ssasw_exit( void )
{
   ssasw_set_clock( &gChalHandle, 0 );
   ssasw_ioremap_exit( &gChalHandle );
}

EXPORT_SYMBOL(ssasw_exit);

/****************************************************************************/
/**
*   Reserves a channel for use with a device.
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
)
{
   SSASW_Handle_t             ch_handle;
   SSASW_ChalHandle_t         *chal_handle;
   SSASW_Channel_t            *channel;
   SSASW_Channel_t            *channel_sec = NULL;
   SSASW_DeviceAttribute_t    *devAttr;
   int channel_idx = 0;
   int channel_src_idx = 0;
   uint16_t srcmixer_addr;

   /* Get device attribute */
   devAttr = &SSASW_gDeviceAttribute[ dev ];

   if (( dev < 0 ) || ( dev >= SSASW_NUM_DEVICE_ENTRIES ) || devAttr->name == NULL )
   {
      printk( KERN_ERR "%s Invalid device selected\n", __FUNCTION__ );
      return SSASW_INVALID_HANDLE;
   }

   down( &gSSASW.lock );

   if ( devAttr->inUse != 0 )
   {
      /* This device has already been requested and not been freed */
      up( &gSSASW.lock );
      printk( KERN_ERR "%s: device %s is already requested\n", __func__, devAttr->name );
      return SSASW_INVALID_HANDLE;
   }

   for ( channel_idx = 0; channel_idx < SSASW_NUM_CHANNELS; channel_idx++ )
   {
      channel = &gSSASW.channel[ channel_idx ];
      if( !channel->inUse )
      {
         break;
      }
   }

   if( channel_idx == SSASW_NUM_CHANNELS )
   {
      /* No available channels */
      up( &gSSASW.lock );
      printk( KERN_ERR "%s No available channels\n", __FUNCTION__ );
      return SSASW_INVALID_HANDLE;
   }

   chal_handle = &gChalHandle;

   channel->caph_switch_ch = chal_caph_switch_alloc_given_channel( chal_handle->chalSsaswHandle, CAPH_SWITCH_CH_VOID );
   if( channel->caph_switch_ch == CAPH_SWITCH_CH_VOID )
   {
      up( &gSSASW.lock );
      printk( KERN_ERR "%s Allocation of main SWITCH channel failed\n", __FUNCTION__ );
      return SSASW_INVALID_HANDLE;
   }

   channel->inUse = 1;
   devAttr->inUse = 1;
   channel->devType = dev;

   /* Analyze device to determine amount of switch channels required */
   if ( devAttr->src_in_fifo != CAPH_CH_INFIFO_NONE && devAttr->src_out_fifo != CAPH_CH_INFIFO_NONE )
   {
      /* We require a second switch channel */
      for ( channel_src_idx = 0; channel_src_idx < SSASW_NUM_CHANNELS; channel_src_idx++ )
      {
         channel_sec = &gSSASW.channel[ channel_src_idx ];
         if( !channel_sec->inUse )
         {
            break;
         }
      }

      if( channel_src_idx == SSASW_NUM_CHANNELS )
      {
         /* No available channels.  Cleanup main channel */
         chal_caph_switch_free_channel( gChalHandle.chalSsaswHandle, channel->caph_switch_ch );

         channel->caph_switch_ch = CAPH_SWITCH_CH_VOID;
         channel->inUse = 0;
         devAttr->inUse = 0;

         up( &gSSASW.lock );
         printk( KERN_ERR "%s No available channels\n", __FUNCTION__ );
         return SSASW_INVALID_HANDLE;
      }

      channel_sec->caph_switch_ch = chal_caph_switch_alloc_given_channel( chal_handle->chalSsaswHandle, CAPH_SWITCH_CH_VOID );
      if( channel_sec->caph_switch_ch == CAPH_SWITCH_CH_VOID )
      {
         /* Failed to allocate second channel. Cleanup main channel */
         chal_caph_switch_free_channel( gChalHandle.chalSsaswHandle, channel->caph_switch_ch );

         channel->caph_switch_ch = CAPH_SWITCH_CH_VOID;
         channel->inUse = 0;
         devAttr->inUse = 0;

         up( &gSSASW.lock );
         printk( KERN_ERR "%s Allocation of main SWITCH channel failed\n", __FUNCTION__ );
         return SSASW_INVALID_HANDLE;
      }

      channel_sec->inUse = 1;
      channel_sec->devType = dev;
   }

   if( devAttr->src_in_fifo != CAPH_CH_INFIFO_NONE )
   {
      chal_caph_srcmixer_set_SRC( chal_handle->chalSrcMixerHandle, devAttr->src_in_fifo, devAttr->mix_freq );
      chal_caph_srcmixer_set_filter_type( chal_handle->chalSrcMixerHandle, devAttr->src_in_fifo, CAPH_SRCM_LINEAR_PHASE );
      chal_caph_srcmixer_set_fifo_datafmt( chal_handle->chalSrcMixerHandle, devAttr->src_in_fifo, devAttr->src_in_data_format );
      chal_caph_srcmixer_set_fifo_thres( chal_handle->chalSrcMixerHandle, devAttr->src_in_fifo, 5, 2 );
      chal_caph_srcmixer_clr_fifo( chal_handle->chalSrcMixerHandle, devAttr->src_in_fifo );
   }

   if( devAttr->src_out_fifo != CAPH_CH_INFIFO_NONE )
   {
      chal_caph_srcmixer_set_fifo_datafmt( chal_handle->chalSrcMixerHandle, devAttr->src_out_fifo, devAttr->src_out_data_format );
      chal_caph_srcmixer_set_fifo_thres( chal_handle->chalSrcMixerHandle, devAttr->src_out_fifo, 3, 6 );
      chal_caph_srcmixer_clr_fifo( chal_handle->chalSrcMixerHandle, devAttr->src_out_fifo );

      if ( devAttr->mix_src_ch != CAPH_SRCM_CH_NONE && devAttr->mix_dst_ch != CAPH_M_NONE )
      {
         if ( devAttr->mix_dst_ch == (CAPH_M0_Left | CAPH_M0_Right) )
         {
            chal_caph_srcmixer_set_mixingainstep( chal_handle->chalSrcMixerHandle, devAttr->mix_src_ch, CAPH_M0_Left, SRCMIXER_CH_DEF_GAIN_STEP);
            chal_caph_srcmixer_set_mixingainstep( chal_handle->chalSrcMixerHandle, devAttr->mix_src_ch, CAPH_M0_Right, SRCMIXER_CH_DEF_GAIN_STEP);
            chal_caph_srcmixer_set_mixingain ( chal_handle->chalSrcMixerHandle, devAttr->mix_src_ch, CAPH_M0_Left, devAttr->mix_dig_gain );
            chal_caph_srcmixer_set_mixingain ( chal_handle->chalSrcMixerHandle, devAttr->mix_src_ch, CAPH_M0_Right, devAttr->mix_dig_gain );
            chal_caph_srcmixer_set_spkrgain_bitsel ( chal_handle->chalSrcMixerHandle, devAttr->mix_src_ch, SRCMIXER_CH_DEF_BITSEL );
         }
         else
         {
            chal_caph_srcmixer_set_mixingainstep( chal_handle->chalSrcMixerHandle, devAttr->mix_src_ch, devAttr->mix_dst_ch, SRCMIXER_CH_DEF_GAIN_STEP);
            chal_caph_srcmixer_set_mixingain ( chal_handle->chalSrcMixerHandle, devAttr->mix_src_ch, devAttr->mix_dst_ch, devAttr->mix_dig_gain );
            chal_caph_srcmixer_set_spkrgain_bitsel ( chal_handle->chalSrcMixerHandle, devAttr->mix_src_ch, SRCMIXER_CH_DEF_BITSEL );
         }
      }
   }

   ch_handle = MAKE_HANDLE( channel_src_idx, channel_idx );

   /* Configure main channel */

   /* Set switch trigger */
   chal_caph_switch_select_trigger( chal_handle->chalSsaswHandle, channel->caph_switch_ch, devAttr->dev_sw_trig );

   /* Set data format */
   chal_caph_switch_set_datafmt( chal_handle->chalSsaswHandle, channel->caph_switch_ch, devAttr->sw_data_format );

   if ( devAttr->src_out_fifo != CAPH_CH_INFIFO_NONE )
   {
      /* Override source address to SRC Mixer out FIFO specified by device */
      srcmixer_addr = (uint16_t)(SRCMIXER_BASE_ADDR + chal_caph_srcmixer_get_fifo_addr_offset( chal_handle->chalSrcMixerHandle, devAttr->src_out_fifo ));

      /* Set src mixer switch source */
      chal_caph_switch_select_src( chal_handle->chalSsaswHandle, channel->caph_switch_ch, (uint16_t)srcmixer_addr );
   }
   else
   {
      /* Use provided switch source */
      chal_caph_switch_select_src( chal_handle->chalSsaswHandle, channel->caph_switch_ch, (uint16_t)src_addr );
   }

   if ( devAttr->src_in_fifo != CAPH_CH_INFIFO_NONE && devAttr->src_out_fifo == CAPH_CH_INFIFO_NONE )
   {
      /* Override destination address to SRC Mixer in FIFO specified by device */
      srcmixer_addr = (uint16_t)(SRCMIXER_BASE_ADDR + chal_caph_srcmixer_get_fifo_addr_offset( chal_handle->chalSrcMixerHandle, devAttr->src_in_fifo ));

      /* Select src mixer switch destination*/
      chal_caph_switch_add_dst( chal_handle->chalSsaswHandle, channel->caph_switch_ch, (uint16_t)srcmixer_addr );
   }
   else
   {
      /* Use provided switch destination*/
      chal_caph_switch_add_dst( chal_handle->chalSsaswHandle, channel->caph_switch_ch, (uint16_t)dst_addr );
   }

   /* Configure secondary channel */
   if ( devAttr->src_in_fifo != CAPH_CH_INFIFO_NONE && devAttr->src_out_fifo != CAPH_CH_INFIFO_NONE )
   {
      /* Set switch trigger */
      chal_caph_switch_select_trigger( chal_handle->chalSsaswHandle, channel_sec->caph_switch_ch, devAttr->src_sw_trig );

      /* Set data format */
      chal_caph_switch_set_datafmt( chal_handle->chalSsaswHandle, channel_sec->caph_switch_ch, devAttr->sw_data_format );

      /* Select switch source */
      chal_caph_switch_select_src( chal_handle->chalSsaswHandle, channel_sec->caph_switch_ch, (uint16_t)src_addr );

      srcmixer_addr = (uint16_t)(SRCMIXER_BASE_ADDR + chal_caph_srcmixer_get_fifo_addr_offset( chal_handle->chalSrcMixerHandle, devAttr->src_in_fifo ));

      /* Select src mixer switch destination*/
      chal_caph_switch_add_dst( chal_handle->chalSsaswHandle, channel_sec->caph_switch_ch, (uint16_t)srcmixer_addr );
   }

   up( &gSSASW.lock );

   return ch_handle;
}

EXPORT_SYMBOL(ssasw_request_channel);

/****************************************************************************/
/**
*   Frees a previously allocated SSASW Handle.
*
*   @return
*        0      - SSASW Handle was released successfully.
*       -EINVAL - Invalid SSASW handle
*/
/****************************************************************************/

int ssasw_free_channel
(
    SSASW_Handle_t    handle            /* SSASW handle. */
)
{
   SSASW_Channel_t            *channel = NULL;
   SSASW_Channel_t            *channel_src = NULL;
   SSASW_DeviceAttribute_t    *devAttr;
   SSASW_ChalHandle_t         *chal_handle;
   unsigned long              flags;

   if (( channel = HandleToChannel( handle )) == NULL )
   {
      return -ENODEV;
   }

   chal_handle = &gChalHandle;
   devAttr = &SSASW_gDeviceAttribute[ channel->devType ];

   if ( devAttr->src_in_fifo != CAPH_CH_INFIFO_NONE && devAttr->src_out_fifo != CAPH_CH_INFIFO_NONE )
   {
      /* Obtain SRC switch channel */
      channel_src = HandleToChannelSrc( handle );
   }

   down( &gSSASW.lock );
   spin_lock_irqsave( &gHwSsaswLock, flags );

   chal_caph_switch_free_channel( chal_handle->chalSsaswHandle, channel->caph_switch_ch );

   channel->caph_switch_ch = CAPH_SWITCH_CH_VOID;
   channel->inUse = 0;

   if ( channel_src )
   {
      chal_caph_switch_free_channel( chal_handle->chalSsaswHandle, channel_src->caph_switch_ch );

      channel_src->caph_switch_ch = CAPH_SWITCH_CH_VOID;
      channel_src->inUse = 0;
   }

   devAttr->inUse = 0;

   spin_unlock_irqrestore( &gHwSsaswLock, flags );
   up( &gSSASW.lock );

   return 0;
}

EXPORT_SYMBOL(ssasw_free_channel);

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
)
{
   SSASW_Channel_t            *channel = NULL;
   SSASW_Channel_t            *channel_src = NULL;
   SSASW_DeviceAttribute_t    *devAttr;
   SSASW_ChalHandle_t         *chal_handle;
   unsigned long              flags;

   if (( channel = HandleToChannel( handle )) == NULL )
   {
      return -ENODEV;
   }

   chal_handle = &gChalHandle;
   devAttr = &SSASW_gDeviceAttribute[ channel->devType ];

   if ( devAttr->src_in_fifo != CAPH_CH_INFIFO_NONE && devAttr->src_out_fifo != CAPH_CH_INFIFO_NONE )
   {
      /* Obtain SRC switch channel */
      channel_src = HandleToChannelSrc( handle );
   }

   spin_lock_irqsave( &gHwSsaswLock, flags );

   /* Only add destinations to SRC switch channel if available */
   if ( channel_src )
   {
      chal_caph_switch_add_dst( chal_handle->chalSsaswHandle, channel_src->caph_switch_ch, (uint16_t)dst_addr );
   }
   else
   {
      chal_caph_switch_add_dst( chal_handle->chalSsaswHandle, channel->caph_switch_ch, (uint16_t)dst_addr );
   }

   spin_unlock_irqrestore( &gHwSsaswLock, flags );

   return 0;
}

EXPORT_SYMBOL(ssasw_add_dst);

/****************************************************************************/
/**
*   Removes specific destination from the switch device.
*
*   @return
*       -ENODEV - Invalid handle
*/
/****************************************************************************/
int ssasw_remove_dst
(
    SSASW_Handle_t   handle,            /* SSASW handle. */
    uint16_t         src_addr
)
{
   SSASW_Channel_t            *channel;
   SSASW_Channel_t            *channel_src = NULL;
   SSASW_DeviceAttribute_t    *devAttr;
   SSASW_ChalHandle_t         *chal_handle;
   unsigned long              flags;

   if (( channel = HandleToChannel( handle )) == NULL )
   {
      return -ENODEV;
   }

   chal_handle = &gChalHandle;
   devAttr = &SSASW_gDeviceAttribute[ channel->devType ];

   if ( devAttr->src_in_fifo != CAPH_CH_INFIFO_NONE && devAttr->src_out_fifo != CAPH_CH_INFIFO_NONE )
   {
      /* Obtain SRC switch channel */
      channel_src = HandleToChannelSrc( handle );
   }

   spin_lock_irqsave( &gHwSsaswLock, flags );

   /* Only remove destinations to SRC switch channel if available */
   if ( channel_src )
   {
      chal_caph_switch_remove_dst( chal_handle->chalSsaswHandle, channel_src->caph_switch_ch, src_addr );
   }
   else
   {
      chal_caph_switch_remove_dst( chal_handle->chalSsaswHandle, channel->caph_switch_ch, src_addr );
   }

   spin_unlock_irqrestore( &gHwSsaswLock, flags );

   return 0;
}

EXPORT_SYMBOL(ssasw_remove_dst);

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
)
{
   SSASW_Channel_t            *channel;
   SSASW_Channel_t            *channel_src = NULL;
   SSASW_DeviceAttribute_t    *devAttr;
   SSASW_ChalHandle_t         *chal_handle;
   unsigned long              flags;

   if (( channel = HandleToChannel( handle )) == NULL )
   {
      return -ENODEV;
   }

   chal_handle = &gChalHandle;
   devAttr = &SSASW_gDeviceAttribute[ channel->devType ];

   if ( devAttr->src_in_fifo != CAPH_CH_INFIFO_NONE && devAttr->src_out_fifo != CAPH_CH_INFIFO_NONE )
   {
      /* Obtain SRC switch channel */
      channel_src = HandleToChannelSrc( handle );
   }

   spin_lock_irqsave( &gHwSsaswLock, flags );

   /* Only remove destinations to SRC switch channel if available */
   if ( channel_src )
   {
      chal_caph_switch_clear_all_dst( chal_handle->chalSsaswHandle, channel_src->caph_switch_ch );
   }
   else
   {
      chal_caph_switch_clear_all_dst( chal_handle->chalSsaswHandle, channel->caph_switch_ch );
   }

   spin_unlock_irqrestore( &gHwSsaswLock, flags );

   return 0;
}

EXPORT_SYMBOL(ssasw_remove_all_dst);

/****************************************************************************/
/**
*   Sets the linear gain on the mixer switch device.
*
*   @return
*       -ENODEV - Invalid handle
*/
/****************************************************************************/
int ssasw_mixer_gain_set
(
   SSASW_Device_t dev,
   int db
)
{
   SSASW_DeviceAttribute_t    *devAttr;
   SSASW_ChalHandle_t         *chal_handle;
   int lin_gain;

   chal_handle = &gChalHandle;

   /* Get device attribute */
   devAttr = &SSASW_gDeviceAttribute[ dev ];

   if(   devAttr->src_out_fifo != CAPH_CH_INFIFO_NONE &&
         devAttr->mix_src_ch != CAPH_SRCM_CH_NONE     &&
         devAttr->mix_dst_ch != CAPH_M_NONE )
   {
      down( &gSSASW.lock );

      lin_gain = dbToLinearQ14(&db);

      devAttr->mix_dig_gain = lin_gain;

      if ( devAttr->inUse )
      {
         if ( devAttr->mix_dst_ch == (CAPH_M0_Left | CAPH_M0_Right) )
         {
            chal_caph_srcmixer_set_mixingain ( chal_handle->chalSrcMixerHandle, devAttr->mix_src_ch, CAPH_M0_Left, lin_gain );
            chal_caph_srcmixer_set_mixingain ( chal_handle->chalSrcMixerHandle, devAttr->mix_src_ch, CAPH_M0_Right, lin_gain );
         }
         else
         {
            chal_caph_srcmixer_set_mixingain ( chal_handle->chalSrcMixerHandle, devAttr->mix_src_ch, devAttr->mix_dst_ch, lin_gain );
         }
      }

      up( &gSSASW.lock );
   }
   else
   {
      return -ENODEV;
   }

   return 0;
}

EXPORT_SYMBOL(ssasw_mixer_gain_set);

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
)
{
   SSASW_Channel_t            *channel;
   SSASW_Channel_t            *channel_src = NULL;
   SSASW_DeviceAttribute_t    *devAttr;
   SSASW_ChalHandle_t         *chal_handle;
   unsigned long              flags;

   if (( channel = HandleToChannel( handle )) == NULL )
   {
      return -ENODEV;
   }

   chal_handle = &gChalHandle;
   devAttr = &SSASW_gDeviceAttribute[ channel->devType ];

   if ( devAttr->src_in_fifo != CAPH_CH_INFIFO_NONE && devAttr->src_out_fifo != CAPH_CH_INFIFO_NONE )
   {
      /* Obtain SRC switch channel */
      channel_src = HandleToChannelSrc( handle );
   }

   spin_lock_irqsave( &gHwSsaswLock, flags );

   if ( enable )
   {
      chal_caph_switch_enable( chal_handle->chalSsaswHandle, channel->caph_switch_ch );
      if ( channel_src )
      {
         chal_caph_switch_enable( chal_handle->chalSsaswHandle, channel_src->caph_switch_ch );
      }
   }
   else
   {
      if ( channel_src )
      {
         chal_caph_switch_disable( chal_handle->chalSsaswHandle, channel_src->caph_switch_ch );
      }
      chal_caph_switch_disable( chal_handle->chalSsaswHandle, channel->caph_switch_ch );
   }

   spin_unlock_irqrestore( &gHwSsaswLock, flags );

   return 0;
}

EXPORT_SYMBOL(ssasw_enable);

/****************************************************************************/
/**
*   Enables/disables the mixer
*
*   @return
*       -ENODEV - Invalid handle
*/
/****************************************************************************/
int ssasw_mixer_enable
(
   SSASW_Handle_t handle,
   int            enable
)
{
   SSASW_Channel_t            *channel;
   SSASW_Channel_t            *channel_src = NULL;
   SSASW_DeviceAttribute_t    *devAttr;
   SSASW_ChalHandle_t         *chal_handle;
   unsigned long              flags;

   if (( channel = HandleToChannel( handle )) == NULL )
   {
      return -ENODEV;
   }

   chal_handle = &gChalHandle;
   devAttr = &SSASW_gDeviceAttribute[ channel->devType ];

   if ( devAttr->src_in_fifo != CAPH_CH_INFIFO_NONE && devAttr->src_out_fifo != CAPH_CH_INFIFO_NONE )
   {
      /* Obtain SRC switch channel */
      channel_src = HandleToChannelSrc( handle );
   }

   spin_lock_irqsave( &gHwSsaswLock, flags );
   if ( enable )
   {
      if ( devAttr->src_out_fifo != CAPH_CH_INFIFO_NONE )
      {
         if ( devAttr->mix_src_ch != CAPH_SRCM_CH_NONE && devAttr->mix_dst_ch != CAPH_M_NONE )
         {
            if ( devAttr->mix_dst_ch == (CAPH_M0_Left | CAPH_M0_Right) )
            {
               chal_caph_srcmixer_enable_mixing ( chal_handle->chalSrcMixerHandle, devAttr->mix_src_ch, CAPH_M0_Left );
               chal_caph_srcmixer_enable_mixing ( chal_handle->chalSrcMixerHandle, devAttr->mix_src_ch, CAPH_M0_Right );
            }
            else
            {
               chal_caph_srcmixer_enable_mixing ( chal_handle->chalSrcMixerHandle, devAttr->mix_src_ch, devAttr->mix_dst_ch );
            }
         }
      }

      if ( devAttr->src_in_fifo != CAPH_CH_INFIFO_NONE )
      {
         chal_caph_srcmixer_enable_chnl ( chal_handle->chalSrcMixerHandle, devAttr->src_in_fifo );
      }
   }
   else
   {
      if ( devAttr->src_in_fifo != CAPH_CH_INFIFO_NONE )
      {
         chal_caph_srcmixer_disable_chnl ( chal_handle->chalSrcMixerHandle, devAttr->src_in_fifo );
      }

      if ( devAttr->src_out_fifo != CAPH_CH_INFIFO_NONE )
      {
         if ( devAttr->mix_src_ch != CAPH_SRCM_CH_NONE && devAttr->mix_dst_ch != CAPH_M_NONE )
         {
            if ( devAttr->mix_dst_ch == (CAPH_M0_Left | CAPH_M0_Right) )
            {
               chal_caph_srcmixer_disable_mixing ( chal_handle->chalSrcMixerHandle, devAttr->mix_src_ch, CAPH_M0_Left );
               chal_caph_srcmixer_disable_mixing ( chal_handle->chalSrcMixerHandle, devAttr->mix_src_ch, CAPH_M0_Right );
            }
            else
            {
               chal_caph_srcmixer_disable_mixing ( chal_handle->chalSrcMixerHandle, devAttr->mix_src_ch, devAttr->mix_dst_ch );
            }
         }
      }
   }
   spin_unlock_irqrestore( &gHwSsaswLock, flags );


   return 0;
}

EXPORT_SYMBOL(ssasw_mixer_enable);
