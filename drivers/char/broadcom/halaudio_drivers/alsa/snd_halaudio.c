/*****************************************************************************
* Copyright 2003 - 2009 Broadcom Corporation.  All rights reserved.
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
/**
*
*  @file    alsaaudio.c
*
*  @brief   This file implements the ALSA PCM and sound card driver
*
*****************************************************************************/

/* ---- Include Files ---------------------------------------------------- */

#include <linux/types.h>                     /* For stdint types: uint8_t, etc. */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/proc_fs.h>                   /* For /proc/audio */
#include <linux/signal.h>
#include <linux/sched.h>                     /* For schedule_timeout */
#include <linux/init.h>
#include <linux/list.h>                      /* Link list definitions */
#include <linux/fs.h>
#include <linux/ioctl.h>
#include <linux/string.h>
#include <linux/vmalloc.h>                   /* For memory alloc */
#include <linux/sysctl.h>
#include <linux/version.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,27)
#include <linux/semaphore.h>                 /* For down_interruptible, up, etc. */
#else
#include <asm/semaphore.h>                   /* For down_interruptible, up, etc. */
#endif

#include <linux/broadcom/knllog.h>           /* For debugging */
#include <linux/broadcom/bcm_major.h>        /* For BCM_HALAUDIO_MAJOR */
#ifdef CONFIG_BCM_BSC
#include <linux/broadcom/bsc.h>              /* Board Specific Configurations */
#endif
#include <linux/broadcom/halaudio.h>         /* HAL Audio API */
#include <linux/broadcom/halaudio_cfg.h>     /* HAL Audio configurations */
#include <linux/broadcom/amxr.h>             /* Audio mixer API */
#include <asm/io.h>
#include <asm/uaccess.h>                     /* User access routines */
#include <asm/atomic.h>                      /* Atomic operations */
#include <sound/core.h>                      /* ALSA core API */
#include <sound/control.h>                   /* ALSA control API */
#include <sound/info.h>
#include <sound/pcm.h>                       /* ALSA PCM API */
#include <sound/pcm_params.h>
#include <sound/initval.h>
#include <sound/tlv.h>

/* ---- Public Variables ------------------------------------------------- */
/* ---- Private Constants and Types -------------------------------------- */

#define SND_HALAUDIO_CARD_NAME               "HALAUDIO"
#define SND_HALAUDIO_MAX_CARDS               1
#define SND_HALAUDIO_MAX_SUBS                1
#define SND_HALAUDIO_MAX_PCMDEVS             8

#define SND_HALAUDIO_PERIOD_BYTES(freq,chans) (((freq) * SND_HALAUDIO_PERIOD_US * SND_HALAUDIO_SAMPLE_BYTES) / 1000000 * (chans))
#define SND_HALAUDIO_SAMPLE_BYTES            2
#define SND_HALAUDIO_PERIOD_US               5000
#define SND_HALAUDIO_MAX_FREQHZ              48000
#define SND_HALAUDIO_MAX_BT_FREQHZ           8000
#define SND_HALAUDIO_MAX_ISR_FRAME_BYTES     SND_HALAUDIO_PERIOD_BYTES( SND_HALAUDIO_MAX_FREQHZ, 2 ) /* stereo 48khz 5ms period */

#define SND_HALAUDIO_DEFAULT_SAMPFREQ        16000
#define SND_HALAUDIO_DEFAULT_CHANS           1
#define SND_HALAUDIO_DEFAULT_BYTES           SND_HALAUDIO_PERIOD_BYTES( SND_HALAUDIO_DEFAULT_SAMPFREQ, SND_HALAUDIO_DEFAULT_CHANS )

#define SND_HALAUDIO_BUFFER_BYTES_MAX        128*1024

#define SND_HALAUDIO_MAX_SW_GAINDB           0     /* Only allow attenuation */
#define SND_HALAUDIO_MIN_SW_GAINDB           -63

/**
*  HAL Audio sound card structure
*/
struct snd_halaudio_card
{
   const char                      *shortname;           /* short name string */
   const char                      *longname;            /* long name string */
   struct list_head                 pcm_list;            /* list of pcm i/f */
   struct snd_card                 *card;                /* pointer to parent ALSA sound card */
   int                              tot_pcms;            /* Number of pcm streams */
};

/**
*  HAL Audio substream (capture or playback) structure
*/
struct snd_halaudio_subs
{
   /* State */
   int                        dir;                 /* playback or capture */
   atomic_t                   enable;              /* 1 for enable, 0 for disable */

   /* HAL Audio and Audio Mixer info */
   HALAUDIO_HDL               halhdl;              /* HAL Audio client handle */
   HALAUDIO_CODEC_INFO        codec_info;          /* Codec channel info */
   HALAUDIO_CODEC_INFO        aux_codec_info;      /* Auxiliary codec channel info */
   AMXR_PORT_ID               halport;             /* Codec AMXR port id */
   AMXR_PORT_ID               aux_halport;         /* Auxiliary codec AMXR port id */
   AMXR_PORT_ID               sndport;             /* ALSA mixer port id */
   AMXR_CONNECT_DESC          desc;                /* Cached descriptor */
   AMXR_CONNECT_DESC          aux_desc;            /* Cached auxiliary descriptor */
   int                        cnx_loss;            /* Connection loss (+ve number) */

   /* ALSA stuff */
   struct snd_pcm_substream  *pcm_substream;       /* PCM substream pointer */
   struct snd_pcm_hw_constraint_list rate_list;    /* constrain sampling rates */

   /* Config */
   int                        format;              /* Data format */
   int                        channels;            /* Number of interleaved channels */
   int                        rate;                /* current sampling rate */
   int                        buffer_bytes;        /* Buffer size in bytes */
   int                        period_bytes;        /* Period size in bytes */

   /* buffer management */
   int8_t                    *scratch_bufp;        /* buffer to handle wrapping in dma_area */
   int                        currbyte;            /* Current byte index */
   int8_t                    *currposp;            /* Current byte pointer into buffer */
   int                        framebytes;          /* Total frame bytes accummulated */
};

/**
*  Device value
*/
struct snd_halaudio_dvalue
{
   int                        notches;             /* unit-less notches */
   int                        mute;                /* 1 for mute, 0 for unmute */
   int                        db;                  /* db for volume */
};

/**
*  HAL Audio device stream structure
*/
struct snd_halaudio_stream
{
   struct list_head           lnode;               /* list node */
   struct snd_halaudio_card  *mycard;              /* pointer to halaudio sound card */
   struct snd_pcm            *pcm;                 /* pointer to parent ALSA pcm instance */
   int                        pcmid;               /* 0-based id */

   /* HAL Audio primary and auxiliary codec channel info */
   struct snd_halaudio_dvalue anaspk;              /* analog speaker gain */
   struct snd_halaudio_dvalue digspk;              /* digital speaker gain */
   struct snd_halaudio_dvalue anamic;              /* analog microphone gain */
   struct snd_halaudio_dvalue digmic;              /* digital microphone gain */
   struct snd_halaudio_dvalue aux_anaspk;          /* aux. analog speaker gain */
   struct snd_halaudio_dvalue aux_digspk;          /* aux. digital speaker gain */
   struct snd_halaudio_dvalue aux_anamic;          /* aux. analog microphone gain */
   struct snd_halaudio_dvalue aux_digmic;          /* aux. digital microphone gain */

   struct snd_halaudio_subs   substream[2];        /* per direction */
   HALAUDIO_DEV_CFG           pcmcfg;              /* PCM configuration */
};

/* Control types */
enum snd_halaudio_ctl_type
{
   SND_HALAUDIO_ANA_CAPTURE,
   SND_HALAUDIO_DIG_CAPTURE,
   SND_HALAUDIO_ANA_PLAYBACK,
   SND_HALAUDIO_DIG_PLAYBACK,
};

/* knllog traces used for debugging */
#define SND_LOG_ENABLED       0
#if SND_LOG_ENABLED
#define SND_LOG               KNLLOG
#else
#define SND_LOG(c...)
#endif

/* ---- Private Variables ------------------------------------------------ */

/* Standard module options for ALSA */
static int     index[SNDRV_CARDS]   = SNDRV_DEFAULT_IDX;
static char   *id[SNDRV_CARDS]      = SNDRV_DEFAULT_STR;
static int     enable[SNDRV_CARDS]  = { 1, 1, [ 2 ... (SNDRV_CARDS-1) ] = 0 };

/* PCM device look-up list */
static const char *pcm_dev_list[] = 
{
   "speakerphone", 
   "handsfree-spkr",
   "handsfree-mic",
   "handset", 
   "handset-spkr", 
   "handset-mic", 
   "headset", 
   "headset-spkr",
   "headset-mic",
   "lineout", 
   "linein", 
   "bt-headset",
   "vibe",
};

module_param_array( index, int, NULL, 0444 );
MODULE_PARM_DESC( index, "Index value for the " SND_HALAUDIO_CARD_NAME " adapter." );
module_param_array( id, charp, NULL, 0444 );
MODULE_PARM_DESC(id, "ID string for the " SND_HALAUDIO_CARD_NAME " adapter." );
module_param_array( enable, bool, NULL, 0444 );
MODULE_PARM_DESC( enable, "Enable " SND_HALAUDIO_CARD_NAME " adapter." );

/* Global AMXR and HAL Audio handles. Global HAL Audio handle is used
 * primarily for the ALSA mixer controls
 */
static AMXR_HDL               gAmixerHdl;
static HALAUDIO_HDL           gHalAudioHdl;

static struct snd_halaudio_card     gCards[SND_HALAUDIO_MAX_CARDS] =
{
   [0] =    /* card 0 contains mapped halaudio devices */
   {
      .shortname     = "halaudio",
      .longname      = "Broadcom HAL Audio devices: ",
   },
};

static struct snd_pcm_hardware snd_halaudio_hardware =
{
   .info             =  SNDRV_PCM_INFO_MMAP |
                        SNDRV_PCM_INFO_MMAP_VALID |
                        SNDRV_PCM_INFO_INTERLEAVED |
                        SNDRV_PCM_INFO_BLOCK_TRANSFER |
                        SNDRV_PCM_INFO_PAUSE,
#if defined( __LITTLE_ENDIAN )
   .formats          =  SNDRV_PCM_FMTBIT_S16_LE,
#else
   .formats          =  SNDRV_PCM_FMTBIT_S16_BE,
#endif
   .channels_min     =  1, /* mono */
   .channels_max     =  2, /* stereo */
   .buffer_bytes_max =  SND_HALAUDIO_BUFFER_BYTES_MAX,
   .period_bytes_min =  SND_HALAUDIO_MAX_ISR_FRAME_BYTES,
   .period_bytes_max =  SND_HALAUDIO_BUFFER_BYTES_MAX,
   .periods_min      =  2,
   .periods_max      =  SND_HALAUDIO_BUFFER_BYTES_MAX/SND_HALAUDIO_MAX_ISR_FRAME_BYTES,
};

#define SND_HALAUDIO_PCM_VOL( namestr, privdata ) \
{  .iface         = SNDRV_CTL_ELEM_IFACE_MIXER, \
   .name          = namestr, \
   .access        = SNDRV_CTL_ELEM_ACCESS_READWRITE | \
                    SNDRV_CTL_ELEM_ACCESS_TLV_READ | \
                    SNDRV_CTL_ELEM_ACCESS_TLV_CALLBACK, \
   .private_value = (unsigned long)privdata, \
   .info          = snd_halaudio_control_info, \
   .get           = snd_halaudio_control_get, \
   .put           = snd_halaudio_control_put, \
   .tlv           = { .c = snd_halaudio_vol_tlv }, \
}

#define SND_HALAUDIO_PCM_MUTE( namestr, privdata ) \
{  .iface         = SNDRV_CTL_ELEM_IFACE_MIXER, \
   .name          = namestr, \
   .access        = SNDRV_CTL_ELEM_ACCESS_READWRITE, \
   .private_value = (unsigned long)privdata, \
   .info          = snd_halaudio_control_mute_info, \
   .get           = snd_halaudio_control_mute_get, \
   .put           = snd_halaudio_control_mute_put, \
}

#define SND_HALAUDIO_PCM_SW_GAIN( namestr, privdata ) \
{  .iface         = SNDRV_CTL_ELEM_IFACE_MIXER, \
   .name          = namestr, \
   .access        = SNDRV_CTL_ELEM_ACCESS_READWRITE | \
                    SNDRV_CTL_ELEM_ACCESS_TLV_READ | \
                    SNDRV_CTL_ELEM_ACCESS_TLV_CALLBACK, \
   .private_value = (unsigned long)privdata, \
   .info          = snd_halaudio_control_sw_gain_info, \
   .get           = snd_halaudio_control_sw_gain_get, \
   .put           = snd_halaudio_control_sw_gain_put, \
   .tlv           = { .c = snd_halaudio_sw_gain_vol_tlv }, \
}

static int gDefaultPowerEnable;

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,31)
static int proc_doPowerEnable( ctl_table *table, int write,
      void __user *buffer, size_t *lenp, loff_t *ppos );
#else
static int proc_doPowerEnable( ctl_table *table, int write, struct file *filp,
      void __user *buffer, size_t *lenp, loff_t *ppos );
#endif
static struct ctl_table gSysCtlEntries[] =
{
   {
      .procname      = "default_power",
      .data          = &gDefaultPowerEnable,
      .maxlen        = sizeof(int),
      .mode          = 0644,
      .proc_handler  = &proc_doPowerEnable,
   },
   {},   /* last item, do not change */
};
static struct ctl_table gSysCtl[] =
{
   {
      .procname   = "snd_halaudio",
      .mode       = 0555,
      .child      = gSysCtlEntries
   },
   {}    /* last item, do not change */
};
static struct ctl_table_header  *gSysCtlHeader;

/* ---- Private Function Prototypes -------------------------------------- */
static int snd_halaudio_pcm_open( struct snd_pcm_substream *substream, int dir );
static int snd_halaudio_pcm_close( struct snd_pcm_substream *substream, int dir );
static int snd_halaudio_playback_open( struct snd_pcm_substream *substream );
static int snd_halaudio_playback_close( struct snd_pcm_substream *substream );
static int snd_halaudio_capture_open( struct snd_pcm_substream *substream );
static int snd_halaudio_capture_close( struct snd_pcm_substream *substream );
static int snd_halaudio_hw_params( struct snd_pcm_substream *substream, struct snd_pcm_hw_params *hw_params );
static int snd_halaudio_hw_free( struct snd_pcm_substream *substream );
static int snd_halaudio_pcm_prepare( struct snd_pcm_substream *substream );
static int snd_halaudio_pcm_trigger( struct snd_pcm_substream *substream, int cmd );

static snd_pcm_uframes_t snd_halaudio_pcm_pointer( struct snd_pcm_substream *substream );
static int16_t          *snd_halaudio_getptr_mixcb( int bytes, void *privdata );
static void              snd_halaudio_datadone_mixcb( int bytes, void *privdata );

static int snd_halaudio_control_info( struct snd_kcontrol *kcontrol, struct snd_ctl_elem_info *uinfo );
static int snd_halaudio_control_get( struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol );
static int snd_halaudio_control_put( struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol );
static int snd_halaudio_control_mute_info( struct snd_kcontrol *kcontrol, struct snd_ctl_elem_info *uinfo );
static int snd_halaudio_control_mute_get( struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol );
static int snd_halaudio_control_mute_put( struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol );
static int snd_halaudio_control_sw_gain_info( struct snd_kcontrol *kcontrol, struct snd_ctl_elem_info *uinfo );
static int snd_halaudio_control_sw_gain_get( struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol );
static int snd_halaudio_control_sw_gain_put( struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol );
static int snd_halaudio_vol_tlv( struct snd_kcontrol *kcontrol, int op_flag, unsigned int size, unsigned int __user *_tlv );
static int snd_halaudio_sw_gain_vol_tlv( struct snd_kcontrol *kcontrol, int op_flag, unsigned int size, unsigned int __user *_tlv );

/* ---- Functions -------------------------------------------------------- */

/* ALSA Mixer capture controls for halaudio */
static struct snd_kcontrol_new snd_halaudio_capture_controls[] =
{
   SND_HALAUDIO_PCM_VOL( " Capture Volume", SND_HALAUDIO_ANA_CAPTURE ),
   /* SND_HALAUDIO_PCM_MUTE( " Capture Switch", SND_HALAUDIO_ANA_CAPTURE ), */
};
static struct snd_kcontrol_new snd_halaudio_capture_dig_controls[] =
{
   SND_HALAUDIO_PCM_VOL( "dig Capture Volume", SND_HALAUDIO_DIG_CAPTURE ),
   /* SND_HALAUDIO_PCM_MUTE( "dig Capture Switch", SND_HALAUDIO_DIG_CAPTURE ), */
};
#if 0
static struct snd_kcontrol_new snd_halaudio_capture_sw_gain_controls[] =
{
   SND_HALAUDIO_PCM_SW_GAIN( "_swgain Capture Volume", SNDRV_PCM_STREAM_CAPTURE ),
};
#endif

/* ALSA Mixer playback controls for halaudio */
static struct snd_kcontrol_new snd_halaudio_playback_controls[] =
{
   SND_HALAUDIO_PCM_VOL( " Playback Volume", SND_HALAUDIO_ANA_PLAYBACK ),
   SND_HALAUDIO_PCM_MUTE( " Playback Switch", SND_HALAUDIO_ANA_PLAYBACK ),
};
static struct snd_kcontrol_new snd_halaudio_playback_dig_controls[] =
{
   SND_HALAUDIO_PCM_VOL( "dig Playback Volume", SND_HALAUDIO_DIG_PLAYBACK ),
   SND_HALAUDIO_PCM_MUTE( "dig Playback Switch", SND_HALAUDIO_DIG_PLAYBACK ),
};
static struct snd_kcontrol_new snd_halaudio_playback_sw_gain_controls[] =
{
   SND_HALAUDIO_PCM_SW_GAIN( "_swgain Playback Volume", SNDRV_PCM_STREAM_PLAYBACK ),
};

/* Playback operations */
static struct snd_pcm_ops snd_halaudio_playback_ops =
{
   .open       = snd_halaudio_playback_open,
   .close      = snd_halaudio_playback_close,
   .ioctl      = snd_pcm_lib_ioctl,
   .hw_params  = snd_halaudio_hw_params,
   .hw_free    = snd_halaudio_hw_free,
   .prepare    = snd_halaudio_pcm_prepare,
   .trigger    = snd_halaudio_pcm_trigger,
   .pointer    = snd_halaudio_pcm_pointer,
};

/* Capture operations */
static struct snd_pcm_ops snd_halaudio_capture_ops =
{
   .open       = snd_halaudio_capture_open,
   .close      = snd_halaudio_capture_close,
   .ioctl      = snd_pcm_lib_ioctl,
   .hw_params  = snd_halaudio_hw_params,
   .hw_free    = snd_halaudio_hw_free,
   .prepare    = snd_halaudio_pcm_prepare,
   .trigger    = snd_halaudio_pcm_trigger,
   .pointer    = snd_halaudio_pcm_pointer,
};

/***************************************************************************/
/**
*  Memory free callback for runtime pointer memory
*/
static void snd_halaudio_runtime_free(
   struct snd_pcm_runtime *runtime     /*<< (i) runtime pointer */
)
{
   struct snd_halaudio_subs *subsp;
   subsp = runtime->private_data;
   if ( subsp->sndport )
   {
      amxrRemovePort( subsp->sndport );
      subsp->sndport = NULL;
   }
   if ( subsp->scratch_bufp )
   {
      vfree( subsp->scratch_bufp );
      subsp->scratch_bufp = NULL;
   }
   if ( subsp->halhdl )
   {
      /* Free handle and powers down */
      halAudioFreeClient( subsp->halhdl );
      subsp->halhdl = NULL;
   }
}

/***************************************************************************/
/**
*  Generic PCM open method to create a playback or capture stream.
*  The substream hardware record is constructed here.
*/
static int snd_halaudio_pcm_open(
   struct snd_pcm_substream *substream,/*<< (io) substream pointer */
   int                       dir       /*<< (i) direction */
)
{
   struct snd_halaudio_stream   *sp;
   struct snd_halaudio_subs     *subsp;
   struct snd_pcm_runtime       *runtime;
#if !defined(SND_HAL_AUDIO_FIXED_FREQ)
   int                           i;
#endif
   int                           err;
   HALAUDIO_CODEC_INFO          *infop;
   char                          portname[32];
   AMXR_PORT_CB                  cb;
   int                           dst_hz, dst_chans;
   int                           src_hz, src_chans;
   int                           cur_freq;
   const char                   *dirname;
   AMXR_PORT_ID                  sndport;
   HALAUDIO_CODEC                cid, aux_cid;

   sp       = snd_pcm_substream_chip( substream );
   subsp    = &sp->substream[dir];
   runtime  = substream->runtime;

   subsp->halhdl = halAudioAllocateClient();
   if ( subsp->halhdl == NULL )
   {
      return -ENOMEM;
   }

   subsp->dir            = dir;
   subsp->pcm_substream  = substream;
   runtime->private_data = subsp;
   runtime->hw           = snd_halaudio_hardware;
   runtime->private_free = snd_halaudio_runtime_free;
   infop                 = &subsp->codec_info;

   memset( &cb, 0, sizeof(cb) );
   if ( dir == SNDRV_PCM_STREAM_PLAYBACK )
   {
      cb.getsrc   = snd_halaudio_getptr_mixcb;
      cb.srcdone  = snd_halaudio_datadone_mixcb;
      dirname     = "playback";
      src_hz      = SND_HALAUDIO_DEFAULT_SAMPFREQ;
      src_chans   = SND_HALAUDIO_DEFAULT_CHANS;
      dst_hz      = 0;
      dst_chans   = 0;
   }
   else
   {
      cb.getdst   = snd_halaudio_getptr_mixcb;
      cb.dstdone  = snd_halaudio_datadone_mixcb;
      dirname     = "capture";
      src_hz      = 0;
      src_chans   = 0;
      dst_hz      = SND_HALAUDIO_DEFAULT_SAMPFREQ;
      dst_chans   = SND_HALAUDIO_DEFAULT_CHANS;
   }

   /* Create ALSA substream mixer port */
   sprintf( portname, "snd.%s.%s", sp->pcmcfg.name, dirname );
   err = amxrCreatePort( portname, &cb, runtime /* privdata */,
         dst_hz, dst_chans, SND_HALAUDIO_PERIOD_BYTES( dst_hz, dst_chans ),
         src_hz, src_chans, SND_HALAUDIO_PERIOD_BYTES( src_hz, src_chans ),
         &sndport );
   if ( err )
   {
      snd_printk( "%s: failed to create mixer port %s\n", __FUNCTION__, portname );
      return err;
   }
   subsp->sndport = sndport;

   /* query HAL Audio mixer port id */
   err = amxrQueryPortByName( gAmixerHdl, sp->pcmcfg.info.mport_name, &subsp->halport );
   if ( err )
   {
      return err;
   }

   /* query codec channel id */
   err = halAudioQueryCodecByName( subsp->halhdl, sp->pcmcfg.info.codec_name, &cid );
   if ( err )
   {
      snd_printk( "%s: failed to find codec channel %s\n", __FUNCTION__, sp->pcmcfg.info.codec_name );
      return err;
   }

   /* query codec channel information and impose constraints */
   err = halAudioGetCodecInfo( subsp->halhdl, cid, infop );
   if ( err )
   {
      return err;
   }

   /* Query auxiliary codec channel information for combo devices */
   if ( sp->pcmcfg.has_aux )
   {
      err = amxrQueryPortByName( gAmixerHdl, sp->pcmcfg.aux_info.mport_name, &subsp->aux_halport );
      if ( err )
      {
         return err;
      }
      err = halAudioQueryCodecByName( subsp->halhdl, sp->pcmcfg.aux_info.codec_name, &aux_cid );
      if ( err )
      {
         snd_printk( "%s: failed to find codec channel %s\n", __FUNCTION__, sp->pcmcfg.aux_info.codec_name );
         return err;
      }
      err = halAudioGetCodecInfo( subsp->halhdl, aux_cid, &subsp->aux_codec_info );
      if ( err )
      {
         return err;
      }

      /* Error check that primary and auxiliary codec channels are mono */
      if ( dir == SNDRV_PCM_STREAM_PLAYBACK )
      {
         if ( subsp->codec_info.channels_rx != 1 && subsp->aux_codec_info.channels_rx != 1 )
         {
            return -EINVAL;
         }
      }
      else
      {
         if ( subsp->codec_info.channels_tx != 1 && subsp->aux_codec_info.channels_tx != 1 )
         {
            return -EINVAL;
         }
      }
   }

#if defined(SND_HAL_AUDIO_FIXED_FREQ)
   /* Fix the channel frequency to the maximum supported, let upper layer
   ** deal with need to resample.
   */
   cur_freq = SND_HALAUDIO_MAX_FREQHZ;
   if ( !strcmp ( sp->pcmcfg.name, "bt-headset" ) )
   {
      cur_freq = SND_HALAUDIO_MAX_BT_FREQHZ;
   }
   runtime->hw.rate_min = cur_freq;
   runtime->hw.rate_max = cur_freq;
   runtime->hw.rates |= snd_pcm_rate_to_rate_bit( cur_freq );
#else
   /* Codec may be locked by other client.  In such case impose constraint on
   ** the available sampling rate to match the one currently assigned.
   */
   if ( infop->locked )
   {
      err = halAudioGetFreq( subsp->halhdl, cid, &cur_freq );
      if ( err )
      {
         return err;
      }

      runtime->hw.rate_min = cur_freq;
      runtime->hw.rate_max = cur_freq;
      runtime->hw.rates |= snd_pcm_rate_to_rate_bit( cur_freq );
   }
   else
   {
      runtime->hw.rate_min = infop->freqs.freq[0];
      runtime->hw.rate_max = infop->freqs.freq[infop->freqs.num - 1];
      runtime->hw.rates    = 0;
      for ( i = 0; i < infop->freqs.num; i++ )
      {
         runtime->hw.rates |= snd_pcm_rate_to_rate_bit( infop->freqs.freq[i] );
      }
   }
#endif

   if ( runtime->hw.rates & SNDRV_PCM_RATE_KNOT )
   {
      subsp->rate_list.count  = infop->freqs.num;
      subsp->rate_list.list   = infop->freqs.freq;
      subsp->rate_list.mask   = 0;
      err = snd_pcm_hw_constraint_list( runtime, 0,
            SNDRV_PCM_HW_PARAM_RATE, &subsp->rate_list );
   }

   return 0;
}

/***************************************************************************/
/**
*  Generic PCM close method
*/
static int snd_halaudio_pcm_close(
   struct snd_pcm_substream *substream,/*<< (io) substream pointer */
   int                       dir       /*<< (i) direction */
)
{
   /* Nothing to do. All resources freed by snd_halaudio_runtime_free */
   return 0;
}

/***************************************************************************/
/**
*  Playback PCM open method
*/
static int snd_halaudio_playback_open(
   struct snd_pcm_substream *substream /*<< (io) substream pointer */
)
{
   return snd_halaudio_pcm_open( substream, SNDRV_PCM_STREAM_PLAYBACK );
}

/***************************************************************************/
/**
*  Playback PCM close method
*/
static int snd_halaudio_playback_close(
   struct snd_pcm_substream *substream /*<< (io) substream pointer */
)
{
   return snd_halaudio_pcm_close( substream, SNDRV_PCM_STREAM_PLAYBACK );
}

/***************************************************************************/
/**
*  Capture PCM open method
*/
static int snd_halaudio_capture_open(
   struct snd_pcm_substream *substream /*<< (io) substream pointer */
)
{
   return snd_halaudio_pcm_open( substream, SNDRV_PCM_STREAM_CAPTURE );
}

/***************************************************************************/
/**
*  Capture PCM close method
*/
static int snd_halaudio_capture_close(
   struct snd_pcm_substream *substream /*<< (io) substream pointer */
)
{
   return snd_halaudio_pcm_close( substream, SNDRV_PCM_STREAM_CAPTURE );
}

/***************************************************************************/
/**
*  Hardware parameter method used to allocate the audio buffer and setup
*  hardware parameters.
*
*  @remarks
*     A vmalloc buffer is used for the buffer. This routine may be
*     called more than once.
*/
static int snd_halaudio_hw_params(
   struct snd_pcm_substream *substream,/*<< (io) substream pointer */
   struct snd_pcm_hw_params *hw_params /*<< (i)  Pointer to hardware params */
)
{
   return snd_pcm_lib_malloc_pages( substream,
         params_buffer_bytes( hw_params ));
}

/***************************************************************************/
/**
*  Free audio buffer and reset hardware parameters.
*/
static int snd_halaudio_hw_free(
   struct snd_pcm_substream *substream /*<< (io) substream pointer */
)
{
   struct snd_halaudio_stream   *sp;
   struct snd_pcm_runtime       *runtime;
   struct snd_halaudio_subs     *subsp;
   AMXR_PORT_ID                  srcport, dstport;
   AMXR_PORT_ID                  aux_srcport, aux_dstport;

   sp       = snd_pcm_substream_chip( substream );
   runtime  = substream->runtime;
   subsp    = runtime->private_data;

   atomic_set( &subsp->enable, 0 );

   if ( subsp->dir == SNDRV_PCM_STREAM_PLAYBACK )
   {
      srcport     = subsp->sndport;
      dstport     = subsp->halport;
      aux_srcport = subsp->sndport;
      aux_dstport = subsp->aux_halport;
   }
   else
   {
      srcport     = subsp->halport;
      dstport     = subsp->sndport;
      aux_srcport = subsp->aux_halport;
      aux_dstport = subsp->sndport;
   }

   amxrDisconnect( gAmixerHdl, srcport, dstport );

   if ( sp->pcmcfg.has_aux )
   {
      amxrDisconnect( gAmixerHdl, aux_srcport, aux_dstport );
   }

   subsp->desc = 0;
   subsp->aux_desc = 0;

   return snd_pcm_lib_free_pages( substream );
}

/***************************************************************************/
/**
*  Prepare the audio channel before playback or capture.
*/
static int snd_halaudio_pcm_prepare(
   struct snd_pcm_substream *substream /*<< (io) substream pointer */
)
{
   struct snd_halaudio_subs     *subsp;
   struct snd_halaudio_stream   *sp;
   struct snd_pcm_runtime       *runtime;
   int                           err;
   AMXR_PORT_ID                  srcport, dstport;
   AMXR_PORT_ID                  aux_srcport, aux_dstport;
   AMXR_CONNECT_DESC             desc, aux_desc = 0;
   AMXR_CONNECT_TYPE             cnx_type;
   int (*setfreq)( AMXR_PORT_ID, int, int );
   int (*setchans)( AMXR_PORT_ID, int, int );
   const char                   *srcportname, *dstportname;
   int                           buffer_bytes, hal_chans;
   AMXR_PORT_INFO                portinfo;
   HALAUDIO_CODEC                cid, aux_cid;

   sp                   = snd_pcm_substream_chip( substream );
   runtime              = substream->runtime;
   subsp                = runtime->private_data;
   subsp->currposp      = runtime->dma_area;
   subsp->currbyte      = 0;
   subsp->framebytes    = 0;
   subsp->buffer_bytes  = snd_pcm_lib_buffer_bytes( substream );
   subsp->period_bytes  = snd_pcm_lib_period_bytes( substream );
   subsp->format        = runtime->format;
   subsp->channels      = runtime->channels;
   subsp->rate          = runtime->rate;

   /* Allocate scratch used by AMXR to handle buffer wrap */
   if ( subsp->scratch_bufp == NULL )
   {
      subsp->scratch_bufp = vmalloc( SND_HALAUDIO_MAX_ISR_FRAME_BYTES );
      if ( subsp->scratch_bufp == NULL )
      {
         return -ENOMEM;
      }
   }

   /* query codec channel id */
   err = halAudioQueryCodecByName( subsp->halhdl, sp->pcmcfg.info.codec_name, &cid );
   if ( err )
   {
      snd_printk( "%s: failed to find codec channel %s\n", __FUNCTION__, sp->pcmcfg.info.codec_name );
      return err;
   }

   /* Setup default microphone gain and mic mux */
   if ( subsp->dir == SNDRV_PCM_STREAM_CAPTURE )
   {
      HALAUDIO_BLOCK block;

      block = HALAUDIO_BLOCK_ANA_ID( cid, sp->pcmcfg.info.mic_hwsel, HALAUDIO_DIR_ADC );
      halAudioSetGain( subsp->halhdl, block, sp->anamic.db );

      if ( sp->pcmcfg.has_aux )
      {
         block = HALAUDIO_BLOCK_ANA_ID( cid, sp->pcmcfg.aux_info.mic_hwsel, HALAUDIO_DIR_ADC );
         halAudioSetGain( subsp->halhdl, block, sp->aux_anamic.db );
      }
   }

   err = halAudioSetFreq( subsp->halhdl, cid, runtime->rate );
   if ( err )
   {
      /* Codec may be locked by other client. In such case, revert the decision
      ** of failing the call to 'snd_halaudio_pcm_prepare' at this time.
      **
      ** Note that if the frequency asked for (runtime->rate) is not compatible
      ** with our current supported capabilities, the 'amxrConnect' call done
      ** after will return an error and will reject the 'snd_halaudio_pcm_prepare'
      ** anyways.
      */
      if ( err == -EBUSY )
      {
         err = 0;
      }

      if ( err )
      {
         snd_printk( "%s: failed to set to rate=%i for codec %i [%s]\n",
               __FUNCTION__, runtime->rate, cid, subsp->codec_info.name );
         return err;
      }
   }

   err = amxrGetPortInfo( gAmixerHdl, subsp->halport, &portinfo );
   if ( err )
   {
      snd_printk( "%s: failed to query hw port info err=%i\n", __FUNCTION__, err );
      return -EINVAL;
   }

   if ( subsp->dir == SNDRV_PCM_STREAM_PLAYBACK )
   {
      setfreq     = amxrSetPortSrcFreq;
      setchans    = amxrSetPortSrcChannels;
      srcport     = subsp->sndport;
      dstport     = subsp->halport;
      aux_srcport = subsp->sndport;
      aux_dstport = subsp->aux_halport;

      buffer_bytes = portinfo.dst_bytes;
      hal_chans   = portinfo.dst_chans;
   }
   else
   {
      setfreq     = amxrSetPortDstFreq;
      setchans    = amxrSetPortDstChannels;
      srcport     = subsp->halport;
      dstport     = subsp->sndport;
      aux_srcport = subsp->aux_halport;
      aux_dstport = subsp->sndport;

      buffer_bytes = portinfo.src_bytes;
      hal_chans   = portinfo.src_chans;
   }

   if ( subsp->channels >= hal_chans )
   {
      buffer_bytes *= subsp->channels / hal_chans;
   }
   else
   {
      buffer_bytes /= hal_chans / subsp->channels;
   }

   /* Sanity check buffer size */
   if ( subsp->buffer_bytes < buffer_bytes )
   {
      snd_printk( "%s: buffer size of %i bytes is less than minimum expected at %i bytes\n",
            __FUNCTION__, subsp->buffer_bytes, buffer_bytes );
      return -EINVAL;
   }

   err = setfreq( subsp->sndport, runtime->rate, buffer_bytes );
   if ( err )
   {
      snd_printk( "%s: failed to set %s amxr port sample rate=%i\n",
            __FUNCTION__,
            subsp->dir == SNDRV_PCM_STREAM_PLAYBACK ? "playback" : "capture",
            runtime->rate );
      return err;
   }

   err = setchans( subsp->sndport, runtime->channels, buffer_bytes );
   if ( err )
   {
      snd_printk( "%s: failed to set %s amxr port channels=%i\n",
            __FUNCTION__,
            subsp->dir == SNDRV_PCM_STREAM_PLAYBACK ? "playback" : "capture",
            runtime->channels );
      return err;
   }

   if ( sp->pcmcfg.has_aux )
   {
      /* Combo devices */

      /* query codec channel id */
      err = halAudioQueryCodecByName( subsp->halhdl, sp->pcmcfg.aux_info.codec_name, &aux_cid );
      if ( err )
      {
         snd_printk( "%s: failed to find codec channel %s\n", __FUNCTION__, sp->pcmcfg.aux_info.codec_name );
         return err;
      }

      /* Configure auxiliary codec channel sampling frequency */
      err = halAudioSetFreq( subsp->halhdl, aux_cid, runtime->rate );
      if ( err )
      {
         /* Codec may be locked by other client. In such case, revert the decision
         ** of failing the call to 'snd_halaudio_pcm_prepare' at this time.
         **
         ** Note that if the frequency asked for (runtime->rate) is not compatible
         ** with our current supported capabilities, the 'amxrConnect' call done
         ** after will return an error and will reject the 'snd_halaudio_pcm_prepare'
         ** anyways.
         */
         if ( err == -EBUSY )
         {
            err = 0;
         }

         if ( err )
         {
            snd_printk( "%s: failed to set to rate=%i for codec %i [%s]\n",
                  __FUNCTION__, runtime->rate, aux_cid, subsp->aux_codec_info.name );
            return err;
         }
      }

      /* Create AMXR connections */
      if ( runtime->channels == 1 )
      {
         desc     = AMXR_CONNECT_MONO2MONO;
         aux_desc = AMXR_CONNECT_MONO2MONO;
      }
      else if ( runtime->channels == 2 )
      {
         desc     = AMXR_CONNECT_STEREOSPLITL;
         aux_desc = AMXR_CONNECT_STEREOSPLITR;
      }
      else
      {
         snd_printk( "%s: unsupported config for combo device channels=%i\n",
               __FUNCTION__, runtime->channels );
         return -EINVAL;
      }
   }
   else
   {
      int src_chans, dst_chans;
      HALAUDIO_CFG_DEV_DIR *devdirp;
      int src_chidx = 0, dst_chidx = 0, codec_chans;

      /* Monolithic devices */

      if ( subsp->dir == SNDRV_PCM_STREAM_PLAYBACK )
      {
         src_chans = runtime->channels;
         dst_chans = subsp->codec_info.channels_rx;
         devdirp = &sp->pcmcfg.info.spkr;
         codec_chans = dst_chans;
         dst_chidx = devdirp->chidx;
      }
      else
      {
         src_chans = subsp->codec_info.channels_tx;
         dst_chans = runtime->channels;
         devdirp = &sp->pcmcfg.info.mic;
         codec_chans = src_chans;
         src_chidx = devdirp->chidx;
      }

      /* Determine AMXR connection descriptor */
      if ( devdirp->chans != 0 && devdirp->chans != codec_chans )
      {
         /* TODO: This is a special case of supporting a single mono channel
          * extraction. To be more generic, this code needs to loop to create 
          * multiple extractions. For example, to support duplicating 
          * a left channel of a stereo source into a stereo stream.
          */
         cnx_type = AMXR_CONNECT_EXTRACT_AND_DEPOSIT;
      }
      else if ( src_chans == dst_chans )
      {
         cnx_type = AMXR_CONNECT_STRAIGHT_THROUGH;
      }
      else if ( src_chans == 1 )
      {
         cnx_type = AMXR_CONNECT_MONO_TO_MULT_DUPLICATE;
      }
      else if ( dst_chans == 1 )
      {
         cnx_type = AMXR_CONNECT_MULTI_TO_MONO_CONVERT;
      }
      else
      {
         snd_printk( "%s: unsupported config src_chans=%i dst_chans=%i\n",
               __FUNCTION__, src_chans, dst_chans );
         return -EINVAL;
      }

      desc = AMXR_CREATE_DESC( cnx_type, src_chans, src_chidx, dst_chans, dst_chidx );
   }

   subsp->desc = desc;
   subsp->aux_desc = aux_desc;

   err = amxrConnect( gAmixerHdl, srcport, dstport, desc );
   if ( err )
   {
      amxrQueryNameById( srcport, &srcportname );
      amxrQueryNameById( dstport, &dstportname );
      snd_printk( "%s: failed to connect %s -> %s\n",
            __FUNCTION__, srcportname, dstportname );
      return err;
   }
   amxrSetCnxLoss( gAmixerHdl, srcport, dstport, desc, subsp->cnx_loss );

   if ( sp->pcmcfg.has_aux )
   {
      err = amxrConnect( gAmixerHdl, aux_srcport, aux_dstport, aux_desc );
      if ( err )
      {
         amxrQueryNameById( aux_srcport, &srcportname );
         amxrQueryNameById( aux_dstport, &dstportname );
         snd_printk( "%s: failed to connect %s -> %s\n",
               __FUNCTION__, srcportname, dstportname );
         return err;
      }
      amxrSetCnxLoss( gAmixerHdl, aux_srcport, aux_dstport, aux_desc, subsp->cnx_loss );
   }

   snd_pcm_format_set_silence( runtime->format, runtime->dma_area,
         bytes_to_samples( runtime, runtime->dma_bytes ));

   err = halAudioSetPower( subsp->halhdl, HALAUDIO_POWER_FULL_POWER );
   if ( err )
   {
      snd_printk( "%s: failed to enable full audio power err=%i\n",
            __FUNCTION__, err );
      return err;
   }

   return 0;
}

/***************************************************************************/
/**
*  PCM trigger method
*
*  @remarks
*     This routine run in an atomic context.
*/
static int snd_halaudio_pcm_trigger(
   struct snd_pcm_substream *substream,/*<< (io) substream pointer */
   int                       cmd       /*<< (i)  trigger command */
)
{
   struct snd_halaudio_subs     *subsp;

   subsp = substream->runtime->private_data;

   SND_LOG( "cmd=%i period_size=%u buffer_size=%u ", cmd,
         (unsigned)substream->runtime->period_size,
         (unsigned)substream->runtime->buffer_size );

   switch ( cmd )
   {
      case SNDRV_PCM_TRIGGER_START:
      case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
         atomic_set( &subsp->enable, 1 );
         return 0;

      case SNDRV_PCM_TRIGGER_STOP:
      case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
         atomic_set( &subsp->enable, 0 );
         return 0;

      default:
         return -EINVAL;
   }

   return -EINVAL;
}

/***************************************************************************/
/**
*  Return the current completed pcm pointer position.
*  Called in atomic context with interrupts disabled
*/
static snd_pcm_uframes_t snd_halaudio_pcm_pointer(
   struct snd_pcm_substream *substream /*<< (io) substream pointer */
)
{
   struct snd_halaudio_subs  *subsp;
   struct snd_pcm_runtime    *runtime;
   snd_pcm_uframes_t          frames;

   runtime  = substream->runtime;
   subsp    = runtime->private_data;

   frames = bytes_to_frames( runtime, subsp->currbyte );

   SND_LOG( "start_threshold=%i hw_ptr_interrupt=%u appl_ptr=%u\n",
         (unsigned)runtime->start_threshold, (unsigned)runtime->hw_ptr_interrupt,
         runtime->control->appl_ptr );

   return frames;
}

/***************************************************************************/
/**
*  Additional cleanup for PCM device memory
*/
static void snd_halaudio_pcm_free(
   struct snd_pcm *pcm                 /*<< (i) Pointer to SND PCM device */
)
{
   struct snd_halaudio_stream *streamp;
   streamp = snd_pcm_chip( pcm );
   if ( streamp )
   {
      vfree( streamp );
   }
}

/***************************************************************************/
/**
*  Audio Mixer callback to get data pointer
*
*  @return
*     NULL     - non-matching buffer size or non-existent buffer
*     ptr      - pointer to data buffer
*/
static int16_t *snd_halaudio_getptr_mixcb(
   int   bytes,               /**< (i) buffer size in bytes */
   void *privdata             /**< (i) mixer port index  */
)
{
   struct snd_pcm_runtime    *runtime;
   struct snd_halaudio_subs  *subsp;
   int                        bufferbytes;
   void                      *ptr;

   runtime     = privdata;
   subsp       = runtime->private_data;
   bufferbytes = subsp->buffer_bytes;

   if ( subsp->currbyte + bytes > bufferbytes )
   {
      int len, remainlen;

      /* use scratch buffer to handle buffer wrap */
      len         = bufferbytes - subsp->currbyte;
      remainlen   = bytes - len;
      memcpy( subsp->scratch_bufp, subsp->currposp, len );
      memcpy( &subsp->scratch_bufp[len], runtime->dma_area, remainlen );

      ptr = subsp->scratch_bufp;
   }
   else
   {
      ptr = subsp->currposp;
   }

   if ( atomic_read( &subsp->enable ) == 0 )
   {
      /* If paused or stopped, return zeroed data */
      ptr = subsp->scratch_bufp;
      memset( ptr, 0, bytes );
   }

   SND_LOG( "bytes=%i currbyte=%i bufferbytes=%i enabled=%i",
         bytes, subsp->currbyte, bufferbytes, atomic_read( &subsp->enable ));

   return ptr;
}

/***************************************************************************/
/**
*  Audio Mixer callback used to indicate when mixer operation has
*  completed.
*/
static void snd_halaudio_datadone_mixcb(
   int   bytes,               /**< (i) buffer size in bytes */
   void *privdata             /**< (i) mixer port index  */
)
{
   struct snd_pcm_runtime    *runtime;
   struct snd_halaudio_subs  *subsp;
   int                        buffer_bytes, period_bytes;

   runtime        = privdata;

   SND_LOG( "bytes=%i avail=%i hw_ptr=%i appl_ptr=%i", 
         bytes, snd_pcm_playback_avail( runtime ), 
         runtime->status->hw_ptr, runtime->control->appl_ptr );

   subsp          = runtime->private_data;
   buffer_bytes   = subsp->buffer_bytes;
   period_bytes   = subsp->period_bytes;

   if ( atomic_read( &subsp->enable ))
   {
      subsp->currbyte += bytes;
      subsp->currposp += bytes;
      subsp->framebytes += bytes;

      if ( subsp->currbyte >= buffer_bytes )
      {
         subsp->currbyte -= buffer_bytes;
         subsp->currposp -= buffer_bytes;
      }

      if ( subsp->framebytes >= period_bytes )
      {
         subsp->framebytes %= period_bytes;
         snd_pcm_period_elapsed( subsp->pcm_substream );
      }
   }

   SND_LOG( "currbyte=%i period_bytes=%i framebytes=%i",
         subsp->currbyte, period_bytes, subsp->framebytes );
}

/***************************************************************************/
/**
*  Helper routine to parse the control type and return block and value ptr.
*
*  @return
*     0 for success, otherwise a negative error code
*/
static int parse_ctl_type(
   enum snd_halaudio_ctl_type    ctl_type,   /*<< (i) mixer ctl type */
   struct snd_halaudio_stream   *sp,         /*<< (i) ptr to pcm stream */
   HALAUDIO_BLOCK               *blockp,     /*<< (o) ptr to block id, may be NULL */
   struct snd_halaudio_dvalue  **vpp,        /*<< (o) ptr to value ptr, may be NULL */
   HALAUDIO_BLOCK               *auxblockp,  /*<< (o) ptr to block id, may be NULL */
   struct snd_halaudio_dvalue  **auxvpp      /*<< (o) ptr to value ptr, may be NULL */
)
{
   int                 err;
   HALAUDIO_CODEC      cid, aux_cid;
   HALAUDIO_DEV_CFG   *cfg;
   HALAUDIO_DEV_INFO  *info, *aux_info;

   cfg = &sp->pcmcfg;
   
   info = &cfg->info;
   err = halAudioQueryCodecByName( gHalAudioHdl, info->codec_name, &cid );
   if ( err )
   {
      snd_printk( "%s: failed to find codec channel %s\n", __FUNCTION__, info->codec_name );
      return err;
   }

   if ( cfg->has_aux )
   {
      aux_info = &cfg->aux_info;
      err = halAudioQueryCodecByName( gHalAudioHdl, aux_info->codec_name, &aux_cid );
      if ( err )
      {
         snd_printk( "%s: failed to find codec channel %s\n", __FUNCTION__, aux_info->codec_name );
         return err;
      }
   }

   switch ( ctl_type )
   {
      case SND_HALAUDIO_ANA_CAPTURE:
         if ( blockp )
         {
            *blockp = HALAUDIO_BLOCK_ANA_ID( cid, info->mic_hwsel, HALAUDIO_DIR_ADC );
         }
         if ( auxblockp )
         {
            *auxblockp = HALAUDIO_BLOCK_ANA_ID( aux_cid, info->mic_hwsel, HALAUDIO_DIR_ADC );
         }
         if ( vpp )
         {
            *vpp = &sp->anamic;
         }
         if ( auxvpp )
         {
            *auxvpp = &sp->aux_anamic;
         }
         break;

      case SND_HALAUDIO_DIG_CAPTURE:
         if ( blockp )
         {
            *blockp = HALAUDIO_BLOCK_DIG_ID( cid, HALAUDIO_DIR_ADC );
         }
         if ( auxblockp )
         {
            *auxblockp = HALAUDIO_BLOCK_DIG_ID( aux_cid, HALAUDIO_DIR_ADC );
         }

         if ( vpp )
         {
            *vpp = &sp->digmic;
         }
         if ( auxvpp )
         {
            *auxvpp = &sp->aux_digmic;
         }
         break;

      case SND_HALAUDIO_ANA_PLAYBACK:
         if ( blockp )
         {
            *blockp = HALAUDIO_BLOCK_ANA_ID( cid, info->spkr_hwsel, HALAUDIO_DIR_DAC );
         }
         if ( auxblockp )
         {
            *auxblockp = HALAUDIO_BLOCK_ANA_ID( aux_cid, info->spkr_hwsel, HALAUDIO_DIR_DAC );
         }

         if ( vpp )
         {
            *vpp = &sp->anaspk;
         }
         if ( auxvpp )
         {
            *auxvpp = &sp->aux_anaspk;
         }
         break;

      case SND_HALAUDIO_DIG_PLAYBACK:
         if ( blockp )
         {
            *blockp = HALAUDIO_BLOCK_DIG_ID( cid, HALAUDIO_DIR_DAC );
         }
         if ( auxblockp )
         {
            *auxblockp = HALAUDIO_BLOCK_DIG_ID( aux_cid, HALAUDIO_DIR_DAC );
         }
         if ( vpp )
         {
            *vpp = &sp->digspk;
         }
         if ( auxvpp )
         {
            *auxvpp = &sp->aux_digspk;
         }
         break;

      default:
         return -EINVAL;
   }

   return 0;
}

/***************************************************************************/
/**
*  Mixer control info callback
*
*  @return
*     0 for success, otherwise a negative error code
*/
static int snd_halaudio_control_info(
   struct snd_kcontrol       *kcontrol,   /*<< (i) Ptr to control structure */
   struct snd_ctl_elem_info  *uinfo       /*<< (o) Ptr to output value */
)
{
   struct snd_halaudio_stream   *sp;
   int                           ctl_type;
   HALAUDIO_BLOCK                block;
   HALAUDIO_GAIN_INFO            info;
   int                           err;
   int                           max_step;

   sp       = snd_kcontrol_chip( kcontrol );
   ctl_type = kcontrol->private_value;

   err = parse_ctl_type( ctl_type, sp, &block, NULL, NULL, NULL );
   if ( err )
   {
      return err;
   }

   err = halAudioGetGainInfo( gHalAudioHdl, block, &info );
   if ( err )
   {
      return err;
   }

   /* Convert gain info into notches */
   if ( info.range_type == HALAUDIO_RANGETYPE_FIXED_STEPSIZE )
   {
      max_step = (info.maxdb - info.mindb) / info.range.fixed_step;
   }
   else
   {
      max_step = info.range.list.num - 1;
   }

   /* Add 1 step to accommodate mute setting */
   max_step++;

   /* Assume that combo devices consist of component devices
    * with similar gain configuration
    */

   uinfo->type                = SNDRV_CTL_ELEM_TYPE_INTEGER;
   uinfo->count               = sp->pcmcfg.has_aux ? 2 : 1;
   uinfo->value.integer.min   = 0;
   uinfo->value.integer.max   = max_step;

   return 0;
}

/***************************************************************************/
/**
*  Mixer mute control info callback
*
*  @return
*     0 for success, otherwise a negative error code
*/
static int snd_halaudio_control_mute_info(
   struct snd_kcontrol       *kcontrol,   /*<< (i) Ptr to control structure */
   struct snd_ctl_elem_info  *uinfo       /*<< (o) Ptr to output value */
)
{
   struct snd_halaudio_stream *sp = snd_kcontrol_chip( kcontrol );
   uinfo->type                = SNDRV_CTL_ELEM_TYPE_BOOLEAN;
   uinfo->count               = sp->pcmcfg.has_aux ? 2 : 1;
   uinfo->value.integer.min   = 0;
   uinfo->value.integer.max   = 1;
   return 0;
}

/***************************************************************************/
/**
*  Helper routine to map dB value into linear notches
*
*  @return  notches
*
*  @remarks Notch 0 is considered to be mute for all volume controls
*/
static int map_db_to_notches(
   HALAUDIO_GAIN_INFO   *infop      /*<< (io) Gain info structure pointer */
)
{
   int notches;

   if ( infop->currentdb <= HALAUDIO_GAIN_MUTE )
   {
      return 0;   /* mute */
   }

   if ( infop->range_type == HALAUDIO_RANGETYPE_FIXED_STEPSIZE )
   {
      notches = (infop->currentdb - infop->mindb) / infop->range.fixed_step;
   }
   else
   {
      for ( notches = 0; notches < infop->range.list.num; notches++ )
      {
         if ( infop->currentdb == infop->range.list.db[notches] )
         {
            break;
         }
      }
      if ( notches >= infop->range.list.num )
      {
         /* Limit notches in case the current db does not match list */
         notches = infop->range.list.num - 1;
      }
   }

   return notches + 1;  /* account for mute */
}

/***************************************************************************/
/**
*  Helper routine to map linear steps into dB
*
*  @return  step
*/
static int map_notches_to_db(
   int                  notches,    /*<< (i) unit-less notches */
   HALAUDIO_GAIN_INFO  *infop       /*<< (i) HAL Audio gain info block */
)
{
   int db = infop->mindb;

   if ( notches == 0 )
   {
      return HALAUDIO_GAIN_MUTE;
   }

   notches--;  /* skip mute notch */

   if ( infop->range_type == HALAUDIO_RANGETYPE_FIXED_STEPSIZE )
   {
      db += notches * infop->range.fixed_step;
   }
   else
   {
      db = infop->range.list.db[notches];
   }

   return db;
}

/***************************************************************************/
/**
*  Mixer control get callback
*
*  @return
*     0 for success, otherwise a negative error code
*/
static int snd_halaudio_control_get(
   struct snd_kcontrol       *kcontrol,   /*<< (i) Ptr to control structure */
   struct snd_ctl_elem_value *ucontrol    /*<< (o) Ptr to output value */
)
{
   struct snd_halaudio_stream   *sp;
   struct snd_halaudio_dvalue   *vp, *auxvp;
   HALAUDIO_BLOCK                block, auxblock;
   HALAUDIO_GAIN_INFO            info;
   int                           ctl_type;
   int                           notches;
   int                           err;

   sp       = snd_kcontrol_chip( kcontrol );
   ctl_type = kcontrol->private_value;

   err = parse_ctl_type( ctl_type, sp, &block, &vp, &auxblock, &auxvp );
   if ( err )
   {
      return err;
   }

   if ( vp->mute )
   {
      notches = vp->notches;
   }
   else
   {
      err = halAudioGetGainInfo( gHalAudioHdl, block, &info );
      if ( err )
      {
         return err;
      }
      notches     = map_db_to_notches( &info );
      vp->notches = notches;
      vp->db      = info.currentdb;
   }

   ucontrol->value.integer.value[0] = notches;

   /*snd_printk( "%s: name=%s ctl_type=%i value=%i\n", __FUNCTION__, sp->pcmcfg.name, ctl_type, notches ); */

   if ( sp->pcmcfg.has_aux )
   {
      if ( auxvp->mute )
      {
         notches = auxvp->notches;
      }
      else
      {
         err = halAudioGetGainInfo( gHalAudioHdl, auxblock, &info );
         if ( err )
         {
            return err;
         }
         notches        = map_db_to_notches( &info );
         auxvp->notches = notches;
         auxvp->db      = info.currentdb;
      }

      ucontrol->value.integer.value[1] = notches;

      /*snd_printk( "%s: name=%s ctl_type=%i aux_value=%i\n", __FUNCTION__, sp->pcmcfg.name, ctl_type, notches ); */
   }

   return 0;
}

/***************************************************************************/
/**
*  Mixer mute control get callback
*
*  @return
*     0 for success, otherwise a negative error code
*/
static int snd_halaudio_control_mute_get(
   struct snd_kcontrol       *kcontrol,   /*<< (i) Ptr to control structure */
   struct snd_ctl_elem_value *ucontrol    /*<< (o) Ptr to output value */
)
{
   struct snd_halaudio_stream   *sp;
   int                           ctl_type;
   int                           err;
   struct snd_halaudio_dvalue   *vp, *auxvp;
   HALAUDIO_BLOCK                block, auxblock;

   sp       = snd_kcontrol_chip( kcontrol );
   ctl_type = kcontrol->private_value;

   err = parse_ctl_type( ctl_type, sp, &block, &vp, &auxblock, &auxvp );
   if ( err )
   {
      return err;
   }

   ucontrol->value.integer.value[0] = !vp->mute; /* Return negated logic */
   /*snd_printk( "%s: name=%s ctl_type=%i mute=%i\n", __FUNCTION__, sp->pcmcfg.name, ctl_type, vp->mute ); */

   if ( sp->pcmcfg.has_aux )
   {
      ucontrol->value.integer.value[1] = !auxvp->mute;
      /*snd_printk( "%s: name=%s ctl_type=%i aux_value=%i\n", __FUNCTION__, sp->pcmcfg.name, ctl_type, auxvp->mute ); */
   }

   return 0;
}

/***************************************************************************/
/**
*  Mixer control put callback
*
*  @return
*     1 if parameter changed successfully, otherwise 0.
*/
static int snd_halaudio_control_put(
   struct snd_kcontrol       *kcontrol,   /*<< (i) Ptr to control structure */
   struct snd_ctl_elem_value *ucontrol    /*<< (i) Ptr to input value */
)
{
   struct snd_halaudio_stream   *sp;
   struct snd_halaudio_dvalue   *vp, *auxvp;
   HALAUDIO_BLOCK                block, auxblock;
   HALAUDIO_GAIN_INFO            info;
   int                           ctl_type;
   int                           err, changed;
   int                           delta;

   sp       = snd_kcontrol_chip( kcontrol );
   ctl_type = kcontrol->private_value;

   err = parse_ctl_type( ctl_type, sp, &block, &vp, &auxblock, &auxvp );
   if ( err )
   {
      return err;
   }

   changed = 0;
   if ( vp->notches != ucontrol->value.integer.value[0] )
   {
      if ( !vp->mute )
      {
         delta = ucontrol->value.integer.value[0] - vp->notches;
         err = halAudioAlterGain( gHalAudioHdl, block, delta );
         if ( err < 0 )
         {
            return err;
         }
      }

      halAudioGetGainInfo( gHalAudioHdl, block, &info );
      vp->notches = ucontrol->value.integer.value[0];
      vp->db      = map_notches_to_db( vp->notches, &info );
      changed     = 1;
      /*snd_printk( "%s: name=%s ctl_type=%i notches=%i db=%i mute=%i\n", __FUNCTION__, sp->pcmcfg.name, ctl_type, vp->notches, vp->db, vp->mute ); */
   }

   if ( sp->pcmcfg.has_aux )
   {
      if ( auxvp->notches != ucontrol->value.integer.value[1] )
      {
         if ( !auxvp->mute )
         {
            delta = ucontrol->value.integer.value[1] - auxvp->notches;
            err = halAudioAlterGain( gHalAudioHdl, auxblock, delta );
            if ( err < 0 )
            {
               return err;
            }
         }

         halAudioGetGainInfo( gHalAudioHdl, auxblock, &info );
         auxvp->notches = ucontrol->value.integer.value[1];
         auxvp->db      = map_notches_to_db( auxvp->notches, &info );
         changed        = 1;
         /*snd_printk( "%s: (aux) name=%s ctl_type=%i notches=%i db=%i mute=%i\n", __FUNCTION__, sp->pcmcfg.name, ctl_type, auxvp->notches, auxvp->db, auxvp->mute ); */
      }
   }

   return changed;
}

/***************************************************************************/
/**
*  Mixer mute control put callback
*
*  @return
*     1 if parameter changed successfully, otherwise 0.
*/
static int snd_halaudio_control_mute_put(
   struct snd_kcontrol       *kcontrol,   /*<< (i) Ptr to control structure */
   struct snd_ctl_elem_value *ucontrol    /*<< (i) Ptr to input value */
)
{
   struct snd_halaudio_stream   *sp;
   int                           ctl_type;
   int                           err, changed;
   struct snd_halaudio_dvalue   *vp, *auxvp;
   HALAUDIO_BLOCK                block, auxblock;
   int                           value;

   sp       = snd_kcontrol_chip( kcontrol );
   ctl_type = kcontrol->private_value;

   err = parse_ctl_type( ctl_type, sp, &block, &vp, &auxblock, &auxvp );
   if ( err )
   {
      return err;
   }

   changed = 0;
   value = !ucontrol->value.integer.value[0]; /* negated logic */
   if ( vp->mute != value )
   {
      vp->mute = value;
      err = halAudioSetGain( gHalAudioHdl, block, vp->mute ? HALAUDIO_GAIN_MUTE : vp->db );
      if ( err )
      {
         return err;
      }
      /*snd_printk( "%s: name=%s ctl_type=%i mute=%i\n", __FUNCTION__, sp->pcmcfg.name, ctl_type, vp->mute ); */
      changed = 1;
   }

   if ( sp->pcmcfg.has_aux )
   {
      value = !ucontrol->value.integer.value[1]; /* negated logic */
      if ( auxvp->mute != value )
      {
         auxvp->mute = value;
         err = halAudioSetGain( gHalAudioHdl, auxblock, auxvp->mute ? HALAUDIO_GAIN_MUTE : auxvp->db );
         if ( err )
         {
            return err;
         }
         /*snd_printk( "%s: (aux) name=%s ctl_type=%i mute=%i\n", __FUNCTION__, sp->pcmcfg.name, ctl_type, auxvp->mute ); */
         changed = 1;
      }
   }

   return changed;
}

/***************************************************************************/
/**
*  Mixer software gain control info callback
*
*  @return
*     0 for success, otherwise a negative error code
*/
static int snd_halaudio_control_sw_gain_info(
   struct snd_kcontrol       *kcontrol,   /*<< (i) Ptr to control structure */
   struct snd_ctl_elem_info  *uinfo       /*<< (o) Ptr to output value */
)
{
   struct snd_halaudio_stream *sp = snd_kcontrol_chip( kcontrol );
   uinfo->type                = SNDRV_CTL_ELEM_TYPE_INTEGER;
   uinfo->count               = sp->pcmcfg.has_aux ? 2 : 1;
   uinfo->value.integer.min   = 0;
   uinfo->value.integer.max   = SND_HALAUDIO_MAX_SW_GAINDB - SND_HALAUDIO_MIN_SW_GAINDB + 1;
   return 0;
}

/***************************************************************************/
/**
*  Helper routine to map AMXR software gain/loss to notches
*
*  @return
*     Notch value
*/
static int sw_gain_to_notches( int gain )
{
   int notches;
   if ( gain < SND_HALAUDIO_MIN_SW_GAINDB )
   {
      notches = 0;
   }
   else
   {
      if ( gain > SND_HALAUDIO_MAX_SW_GAINDB )
      {
         gain = SND_HALAUDIO_MAX_SW_GAINDB;
      }
      notches = gain - SND_HALAUDIO_MIN_SW_GAINDB + 1;
   }
   return notches;
}
/***************************************************************************/
/**
*  Mixer software gain control get callback
*
*  @return
*     0 for success, otherwise a negative error code
*/
static int snd_halaudio_control_sw_gain_get(
   struct snd_kcontrol       *kcontrol,   /*<< (i) Ptr to control structure */
   struct snd_ctl_elem_value *ucontrol    /*<< (o) Ptr to output value */
)
{
   struct snd_halaudio_stream   *sp;
   struct snd_halaudio_subs     *subsp;
   int                           dir;

   sp = snd_kcontrol_chip( kcontrol );
   dir = kcontrol->private_value;
   subsp = &sp->substream[dir];
   ucontrol->value.integer.value[0] = sw_gain_to_notches( -subsp->cnx_loss );

   if ( sp->pcmcfg.has_aux )
   {
      ucontrol->value.integer.value[1] = ucontrol->value.integer.value[0];
   }
#if 0
   snd_printk( "%s: name=%s dir=%s value=%li has_aux=%i\n", __FUNCTION__, sp->pcmcfg.name,
         dir == SNDRV_PCM_STREAM_PLAYBACK ? "playback" : "capture", 
         ucontrol->value.integer.value[0], sp->pcmcfg.has_aux );
#endif
   return 0;
}
/***************************************************************************/
/**
*  Software gain mixer control put callback
*
*  @return
*     1 if parameter changed successfully, otherwise 0.
*/
static int snd_halaudio_control_sw_gain_put(
   struct snd_kcontrol       *kcontrol,   /*<< (i) Ptr to control structure */
   struct snd_ctl_elem_value *ucontrol    /*<< (i) Ptr to input value */
)
{
   struct snd_halaudio_stream   *sp;
   struct snd_halaudio_subs     *subsp;
   int                           dir;
   int                           changed;
   int                           notches;
   AMXR_PORT_ID                  srcport, dstport;
   AMXR_PORT_ID                  aux_srcport, aux_dstport;

   sp = snd_kcontrol_chip( kcontrol );
   dir = kcontrol->private_value;
   subsp = &sp->substream[dir];
   notches = sw_gain_to_notches( -subsp->cnx_loss );

   changed = 0;
   if ( notches != ucontrol->value.integer.value[0] )
   {
      changed     = 1;

      /* Convert notches to dB gain value */
      if ( ucontrol->value.integer.value[0] == 0 )
      {
         subsp->cnx_loss = 1000; /* absolute loss */
      }
      else
      {
         subsp->cnx_loss = -(ucontrol->value.integer.value[0] + SND_HALAUDIO_MIN_SW_GAINDB - 1);
      }
         
      if ( subsp->desc )
      {
         /* Only set connection gain if connection is alive */

         if ( dir == SNDRV_PCM_STREAM_PLAYBACK )
         {
            srcport     = subsp->sndport;
            dstport     = subsp->halport;
            aux_srcport = subsp->sndport;
            aux_dstport = subsp->aux_halport;
         }
         else
         {
            srcport     = subsp->halport;
            dstport     = subsp->sndport;
            aux_srcport = subsp->aux_halport;
            aux_dstport = subsp->sndport;
         }

         amxrSetCnxLoss( gAmixerHdl, srcport, dstport, subsp->desc, subsp->cnx_loss );
         if ( sp->pcmcfg.has_aux )
         {
            amxrSetCnxLoss( gAmixerHdl, aux_srcport, aux_dstport, subsp->aux_desc, subsp->cnx_loss );
         }
      }
   }
#if 0
   snd_printk( "%s: changed=%i name=%s dir=%s old_value=%i new_value=%li\n", __FUNCTION__, 
         changed, sp->pcmcfg.name,
         dir == SNDRV_PCM_STREAM_PLAYBACK ? "playback" : "capture", 
         notches, ucontrol->value.integer.value[0] );
#endif
   return changed;
}

/***************************************************************************/
/**
*  TLV callback for mapping software gain volume setting to dB
*
*  @return
*     0              Success
*     -ve            Error code
*
*  @remarks    ALSA TLV values are expressed in 1/100 dB
*/
static int snd_halaudio_sw_gain_vol_tlv(
   struct snd_kcontrol *kcontrol,
   int                  op_flag,
   unsigned int         size,
   unsigned int __user *_tlv
)
{
   struct snd_halaudio_stream   *sp;
   struct snd_halaudio_subs     *subsp;
   int                           err, dir, notches;

   sp = snd_kcontrol_chip( kcontrol );
   dir = kcontrol->private_value;
   subsp = &sp->substream[dir];
   notches = sw_gain_to_notches( -subsp->cnx_loss );

   {
      int mindb = (SND_HALAUDIO_MIN_SW_GAINDB-1) * 100; /* in 1/100 db */
      unsigned int dbscale[] = { TLV_DB_SCALE_ITEM( mindb, 1 * 100, notches == 0 ) };

      if ( size < sizeof(dbscale) )
      {
         return -ENOMEM;
      }

      err = copy_to_user( _tlv, dbscale, sizeof(dbscale) );
      if ( err )
      {
         return err;
      }
   }

   return 0;
}

/***************************************************************************/
/**
*  Helper routine to print substream information
*/
static void snd_halaudio_subs_readproc(
   struct snd_halaudio_subs *subsp,    /*<< (i) Pointer to halaudio substream */
   struct snd_info_buffer   *bufp      /*<< (o) Pointer to output buffer */
)
{

   if ( atomic_read( &subsp->enable ))
   {
      const char *portname;

      amxrQueryNameById( subsp->sndport, &portname );

      snd_iprintf( bufp, "   %s: [%s] currbyte=%i fmt=%i ch=%i rate=%i period_bytes=%i buffer_bytes=%i\n",
            portname, subsp->codec_info.name, subsp->currbyte,
            subsp->format, subsp->channels, subsp->rate,
            subsp->period_bytes, subsp->buffer_bytes );
   }
}

/***************************************************************************/
/**
*  Sound card proc read callback for debugging
*/
static void snd_halaudio_card_readproc(
   struct snd_info_entry  *entryp,     /*<< (i) Pointer to proc entry */
   struct snd_info_buffer *bufp        /*<< (o) Pointer to output buffer */
)
{
   struct snd_halaudio_card     *mycardp;
   struct snd_card              *cardp;
   struct snd_halaudio_stream   *sp;

   mycardp  = entryp->private_data;
   cardp    = mycardp->card;

   snd_iprintf( bufp, "Mixer: %s\n", cardp->mixername );
   snd_iprintf( bufp, "Comps: %s\n", cardp->components );

   snd_iprintf( bufp, "PCM Streams:\n" );
   list_for_each_entry( sp, &mycardp->pcm_list, lnode )
   {
      snd_iprintf( bufp, " %i: [%s] anaspk=%i digspk=%i anamic=%i digmic=%i\n",
            sp->pcmid, sp->pcmcfg.name, sp->anaspk.notches, sp->digspk.notches,
            sp->anamic.notches, sp->digmic.notches );

      snd_halaudio_subs_readproc( &sp->substream[SNDRV_PCM_STREAM_CAPTURE], bufp );
      snd_halaudio_subs_readproc( &sp->substream[SNDRV_PCM_STREAM_PLAYBACK], bufp );
   }
}

/***************************************************************************/
/**
*  TLV callback for mapping volume setting to dB
*
*  @return
*     0              Success
*     -ve            Error code
*
*  @remarks    ALSA TLV values are expressed in 1/100 dB
*/
static int snd_halaudio_vol_tlv(
   struct snd_kcontrol *kcontrol,
   int                  op_flag,
   unsigned int         size,
   unsigned int __user *_tlv
)
{
   struct snd_halaudio_stream   *sp;
   int                           ctl_type;
   HALAUDIO_BLOCK                block;
   struct snd_halaudio_dvalue   *vp;
   int                           err;
   HALAUDIO_GAIN_INFO            info;
   int                           mindb, step;

   sp       = snd_kcontrol_chip( kcontrol );
   ctl_type = kcontrol->private_value;

   /* Assume primary and auxiliary gains are identical for combo devices */
   err = parse_ctl_type( ctl_type, sp, &block, &vp, NULL, NULL );
   if ( err )
   {
      return err;
   }

   err = halAudioGetGainInfo( gHalAudioHdl, block, &info );
   if ( err )
   {
      return err;
   }

   if ( info.range_type == HALAUDIO_RANGETYPE_FIXED_STEPSIZE )
   {
      /* in 1/100 db and adjust for mute setting */
      mindb = (info.mindb - info.range.fixed_step) * 100;
      step  = info.range.fixed_step * 100; /* in 1/100 db */
   }
   else
   {
      mindb = info.mindb * 100;
      step  = 0;
      if ( vp->notches )
      {
         step = (vp->db - info.mindb) * 100 / vp->notches;
      }
   }

   /*snd_printk( "%s: name=%s ctl_type=%i notches=%i mindb=%i step=%i\n", __FUNCTION__, sp->pcmcfg.name, ctl_type, vp->notches, mindb, step ); */

   {
      unsigned int dbscale[] = { TLV_DB_SCALE_ITEM( mindb, step, vp->notches == 0 ) };

      if ( size < sizeof(dbscale) )
      {
         return -ENOMEM;
      }

      err = copy_to_user( _tlv, dbscale, sizeof(dbscale) );
      if ( err )
      {
         return err;
      }
   }

   return 0;
}

/***************************************************************************/
/**
*  Helper routine to add sound controls to a sound card
*
*  @return
*     0              Success
*     -ve            Error code
*/
static int add_controls(
   struct snd_card            *card,
   struct snd_halaudio_stream *streamp,
   struct snd_kcontrol_new    *controlsp,
   int                         len
)
{
   int ctl, err;

   /* Add ALSA mixer controls for PCM device */
   for ( ctl = 0; ctl < len; ctl++ )
   {
      struct snd_kcontrol_new kctl_new;
      char                    namestr[44];

      memcpy( &kctl_new, &controlsp[ctl], sizeof( kctl_new ));
      sprintf( namestr, "%s", streamp->pcmcfg.name );
      strlcat( namestr, controlsp[ctl].name, sizeof(namestr) );
      kctl_new.name  = namestr;

      err = snd_ctl_add( card, snd_ctl_new1( &kctl_new, streamp ));
      if ( err )
      {
         return err;
      }
   }

   return 0;
}

/***************************************************************************/
/**
*  Sysctl callback to handle default power level setting
*
*  @return  0 success, otherwise failure
*/
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,31)
static int proc_doPowerEnable( ctl_table *table, int write,
      void __user *buffer, size_t *lenp, loff_t *ppos )
#else
static int proc_doPowerEnable( ctl_table *table, int write, struct file *filp,
      void __user *buffer, size_t *lenp, loff_t *ppos )
#endif
{
   int rc;

   /* Process integer operation */
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,31)
   rc = proc_dointvec( table, write, buffer, lenp, ppos );
#else
   rc = proc_dointvec( table, write, filp, buffer, lenp, ppos );
#endif

   if ( write )
   {
      halAudioSetPower( gHalAudioHdl, gDefaultPowerEnable ?
            HALAUDIO_POWER_FULL_POWER : HALAUDIO_POWER_DEEP_SLEEP );
   }

   return rc;
}

/***************************************************************************/
/**
*  Add PCM device control
*
*  @return
*     0              Success
*     -ve            Error code
*/
static int snd_halaudio_pcm_control_add(
   struct snd_card            *card,
   struct snd_halaudio_stream *streamp,
   HALAUDIO_DEV_CFG           *pcmcfg
)
{
   HALAUDIO_GAIN_INFO info;
   HALAUDIO_CODEC cid;
   int err;

   err = halAudioQueryCodecByName( gHalAudioHdl, pcmcfg->info.codec_name, &cid );
   if ( err )
   {
      return err;
   }

   if ( pcmcfg->info.mic_hwsel != HALAUDIO_HWSEL_NONE )
   {
      err = halAudioGetGainInfo( gHalAudioHdl, 
            HALAUDIO_BLOCK_ANA_ID( cid, pcmcfg->info.mic_hwsel, HALAUDIO_DIR_ADC ), &info );
      if ( err )
      {
         return err;
      }
      if ( info.mindb != info.maxdb )
      {
         err = add_controls( card, streamp, snd_halaudio_capture_controls, ARRAY_SIZE( snd_halaudio_capture_controls ));
         if ( err )
         {
            return err;
         }
      }

      err = halAudioGetGainInfo( gHalAudioHdl, 
            HALAUDIO_BLOCK_DIG_ID( cid, HALAUDIO_DIR_ADC ), &info );
      if ( err )
      {
         return err;
      }
      if ( info.mindb != info.maxdb )
      {
         err = add_controls( card, streamp, snd_halaudio_capture_dig_controls, ARRAY_SIZE( snd_halaudio_capture_dig_controls ));
         if ( err )
         {
            return err;
         }
      }
   }

   if ( pcmcfg->info.spkr_hwsel != HALAUDIO_HWSEL_NONE )
   {
      err = halAudioGetGainInfo( gHalAudioHdl, 
            HALAUDIO_BLOCK_ANA_ID( cid, pcmcfg->info.spkr_hwsel, HALAUDIO_DIR_DAC ), &info );
      if ( err )
      {
         return err;
      }
      if ( info.mindb != info.maxdb )
      {
         err = add_controls( card, streamp, snd_halaudio_playback_controls, ARRAY_SIZE( snd_halaudio_playback_controls ));
         if ( err )
         {
            return err;
         }
      }

      err = halAudioGetGainInfo( gHalAudioHdl, 
            HALAUDIO_BLOCK_DIG_ID( cid, HALAUDIO_DIR_DAC ), &info );
      if ( err )
      {
         return err;
      }
      if ( info.mindb != info.maxdb )
      {
         err = add_controls( card, streamp, snd_halaudio_playback_dig_controls, ARRAY_SIZE( snd_halaudio_playback_dig_controls ));
         if ( err )
         {
            return err;
         }
      }

      err = add_controls( card, streamp, snd_halaudio_playback_sw_gain_controls, ARRAY_SIZE( snd_halaudio_playback_sw_gain_controls ));
      if ( err )
      {
         return err;
      }
   }

   return 0;
}

/***************************************************************************/
/**
*  Create ALSA PCM device
*
*  @return
*     0              Success
*     -ve            Error code
*/
static int snd_halaudio_pcm_device_create(
   int                       numdevs,
   struct snd_card          *card,
   struct snd_halaudio_card *mycardp
)
{
   int  i, pcmid;
   int err;
#ifdef CONFIG_BCM_BSC
   int rc;
#endif
   int found = 0;
   HALAUDIO_DEV_CFG *pcmcfgp;

   err = 0;
   pcmid = 0;

   pcmcfgp = vmalloc( sizeof(*pcmcfgp) );
   if ( pcmcfgp == NULL )
   {
      err = -ENOMEM;
      goto cleanup_and_exit;
   }

   for ( i = 0; i < numdevs && pcmid < SND_HALAUDIO_MAX_PCMDEVS; i++ )
   {
      struct snd_pcm             *pcm;
      struct snd_halaudio_stream *streamp;
      int playback_count, capture_count;

#ifdef CONFIG_BCM_BSC
      if (( rc = bsc_query( pcm_dev_list[i], pcmcfgp, sizeof(*pcmcfgp) )))
      {
         /* Device not found */
         /* printk( "%s: '%s' not found, rc=%i\n", __FUNCTION__, pcm_dev_list[i], rc ); */
         continue;
      }
#else
      err = -EPERM;
      goto cleanup_and_exit;
#endif

      found = 1;
      streamp = vmalloc( sizeof(*streamp) );
      if ( streamp == NULL )
      {
         err = -ENOMEM;
         goto cleanup_and_exit;
      }
      memset( streamp, 0, sizeof(*streamp) );

      playback_count = ( pcmcfgp->info.spkr_hwsel == HALAUDIO_HWSEL_NONE )
         ? 0 : SND_HALAUDIO_MAX_SUBS;
      capture_count = ( pcmcfgp->info.mic_hwsel == HALAUDIO_HWSEL_NONE ) 
         ? 0 : SND_HALAUDIO_MAX_SUBS;

      err = snd_pcm_new( card, "HAL Audio", pcmid,
            playback_count, capture_count, &pcm );
      if ( err )
      {
         goto cleanup_and_exit;
      }

      streamp->pcm         = pcm;
      streamp->mycard      = mycardp;
      streamp->pcmid       = pcmid;
      memcpy( &streamp->pcmcfg, pcmcfgp, sizeof(streamp->pcmcfg) );

      pcm->private_data    = streamp;
      pcm->private_free    = snd_halaudio_pcm_free;
      pcm->info_flags      = 0;
      sprintf( pcm->name, "%s", pcmcfgp->name );

      strlcat( card->longname, pcm->name, sizeof(card->longname) );
      strlcat( card->longname, " ", sizeof(card->longname) );

      list_add_tail( &streamp->lnode, &mycardp->pcm_list );
      mycardp->tot_pcms++;

      if ( pcmcfgp->info.mic_hwsel != HALAUDIO_HWSEL_NONE )
      {
         snd_pcm_set_ops( pcm, SNDRV_PCM_STREAM_CAPTURE, &snd_halaudio_capture_ops );
      }
      if ( pcmcfgp->info.spkr_hwsel != HALAUDIO_HWSEL_NONE )
      {
         snd_pcm_set_ops( pcm, SNDRV_PCM_STREAM_PLAYBACK, &snd_halaudio_playback_ops );
      }

      err = snd_pcm_lib_preallocate_pages_for_all( pcm, SNDRV_DMA_TYPE_CONTINUOUS,
            snd_dma_continuous_data( GFP_KERNEL ),
            SND_HALAUDIO_BUFFER_BYTES_MAX/2, SND_HALAUDIO_BUFFER_BYTES_MAX );
      if ( err )
      {
         goto cleanup_and_exit;
      }

      /* add the control */
      snd_halaudio_pcm_control_add( card, streamp, pcmcfgp );

      pcmid++;
   }

   /* free resource */
   vfree( pcmcfgp );

   if ( !found )
   {
      /* No devices supported */
      err = -EINVAL;
      printk( "%s: no PCM devices found\n", __FUNCTION__ );
   }

cleanup_and_exit:
   return err;
}


/***************************************************************************/
/**
*  Sound card probe/initialization routine.
*
*  @return
*     0              Success
*     -ve            Error code
*/
static int __init snd_halaudio_card_probe(
   int cardidx                         /*<< (i) Card index */
)
{
   struct snd_halaudio_card *mycardp;
   struct snd_card          *card;
   int                       err;
   struct snd_info_entry    *entryp;
   int                       numdevs;

   mycardp = &gCards[cardidx];
   INIT_LIST_HEAD( &mycardp->pcm_list );

   /* create new sound card */
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,29)
   err = snd_card_create(index[cardidx], id[cardidx], THIS_MODULE, 0, &card);
   if (err < 0)
      return -ENOMEM;
#else
   card = snd_card_new( index[cardidx], id[cardidx], THIS_MODULE, 0 );
#endif
   if ( card == NULL )
   {
      return -ENOMEM;
   }

   strcpy( card->driver, "snd-halaudio"  );
   strcpy( card->shortname, mycardp->shortname );
   strcpy( card->longname, mycardp->longname );

   mycardp->card = card;

   numdevs = ARRAY_SIZE(pcm_dev_list);

   /* create pcm devices */
   err = snd_halaudio_pcm_device_create( numdevs, card, mycardp );
   if ( err )
   {
      snd_printk( "%s: failed to create PCM devices , err=%i\n", __FUNCTION__, err );
      goto cleanup_and_exit;
   }

   /* Add proc entries for debuging */
   err = snd_card_proc_new( card, mycardp->shortname, &entryp );
   if ( err )
   {
      snd_printk( "%s: failed to create proc entries, err=%i\n", __FUNCTION__, err );
      goto cleanup_and_exit;
   }
   snd_info_set_text_ops( entryp, mycardp, snd_halaudio_card_readproc );

   err = snd_card_register( card );
   if ( err )
   {
      snd_printk( "%s: failed to register sound card, err=%i\n", __FUNCTION__, err );
      goto cleanup_and_exit;
   }

   return 0;

cleanup_and_exit:

   snd_card_free( card );
   return err;
}

/***************************************************************************/
/**
*  Driver initialization called when module loaded by kernel
*
*  @return
*     0              Success
*     -ve            Error code
*/
static int __init snd_halaudio_init( void )
{
   int i, err = 0, cards;

   printk( KERN_INFO "snd_halaudio driver: 1.0 built %s %s\n", __DATE__, __TIME__ );

   gAmixerHdl = amxrAllocateClient();
   if ( gAmixerHdl < 0 )
   {
      return -ENOMEM;
   }

   gHalAudioHdl = halAudioAllocateClient();
   if ( gHalAudioHdl == NULL )
   {
      amxrFreeClient( gAmixerHdl );
      return -ENOMEM;
   }

   cards = 0;
   for ( i = 0; i < ARRAY_SIZE( enable ) && enable[i] && i < SND_HALAUDIO_MAX_CARDS; i++ )
   {
      err = snd_halaudio_card_probe( i );
      if ( err )
      {
         printk( "%s: Failed probing HAL Audio sound card, err=%i\n", __FUNCTION__, err );
         break;
      }
      cards++;
   }

   if ( cards == 0 )
   {
      err = -ENODEV;
      goto backout;
   }

   /* Install sysctl files */
   gSysCtlHeader = register_sysctl_table( gSysCtl );

   /* FIXME: Add platform driver for power management support */

   return 0;

backout:
   halAudioFreeClient( gHalAudioHdl );
   return err;
}

/***************************************************************************/
/**
*  Driver destructor routine. Frees all resources
*/
static void __exit snd_halaudio_exit( void )
{
   int i;

   if ( gSysCtlHeader )
   {
      unregister_sysctl_table( gSysCtlHeader );
   }

   for ( i = 0; i < SND_HALAUDIO_MAX_CARDS; i++ )
   {
      if ( gCards[i].card )
      {
         snd_card_free( gCards[i].card );
      }
   }

   amxrFreeClient( gAmixerHdl );
   halAudioFreeClient( gHalAudioHdl );
}

module_init( snd_halaudio_init );
module_exit( snd_halaudio_exit );
MODULE_AUTHOR( "Broadcom" );
MODULE_DESCRIPTION( "ALSA-HALAUDIO Driver" );
MODULE_LICENSE( "GPL v2" );

