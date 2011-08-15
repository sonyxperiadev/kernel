/*****************************************************************************
* Copyright 2008 - 2010 Broadcom Corporation.  All rights reserved.
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
*  halaudio_audioh.c
*
*  PURPOSE:
*
*     This file contains the Audio Hub driver routines.
*
*  NOTES:
*
*****************************************************************************/


/* ---- Include Files ---------------------------------------- */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/errno.h>
#include <linux/proc_fs.h>                   /* For /proc/audio */
#include <linux/sysctl.h>
#include <linux/device.h>
#include <linux/dmapool.h>                   /* Linux DMA API */
#include <linux/dma-mapping.h>
#include <linux/vmalloc.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/io.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/semaphore.h>
#include <linux/sched.h>
#include <linux/gpio.h>

#include <mach/halaudio_audioh_platform.h>

#include <linux/broadcom/knllog.h>           /* for debugging */
#include <linux/broadcom/halaudio.h>
#include <linux/broadcom/halaudio_lib.h>
#include <linux/broadcom/amxr.h>

#include <chal/chal_audio.h>

#include <mach/rdb/brcm_rdb_sysmap.h>
#include <mach/aadma.h>
#include <mach/ssasw.h>

#include "dma_priv.h"

/* ---- Private Constants and Types -------------------------- */

/* Compile time settings */
/* At minimum one ingress path must be enabled */
#define AUDIOH_VIN_RIGHT_ENABLE  1
#define AUDIOH_VIN_LEFT_ENABLE   1
#define AUDIOH_NVIN_LEFT_ENABLE  1
#define AUDIOH_NVIN_RIGHT_ENABLE 1
#define AUDIOH_EARPIECE_ENABLE   1
#define AUDIOH_HEADSET_ENABLE    1
#define AUDIOH_HANDSFREE_ENABLE  1
#define AUDIOH_VIBRA_ENABLE      1

#define AUDIOH_SIDETONE_SUPPORT  1

#define AUDIOH_STEREO_VIN        0
#define AUDIOH_STEREO_NVIN       0

#define AUDIOH_REGISTER_LENGTH            0x800
#define SDT_REGISTER_LENGTH               0x100  /* Arbitrarily longer than actual */
#define ACI_REGISTER_LENGTH               0x500  /* Arbitrarily longer than actual */

#define AUDIOH_PHYS_BASE_ADDR_START       ( AUDIOH_BASE_ADDR )
#define SDT_PHYS_BASE_ADDR_START          ( SDT_BASE_ADDR )
#define ACI_PHYS_BASE_ADDR_START          ( ACI_BASE_ADDR )
#define HUB_CLK_PHYS_BASE_ADDR_START      ( HUB_CLK_BASE_ADDR )

#define AUDIOH_PROCDIR_NAME                  "audioh"

#define AUDIOH_LOG_ENABLED                   0
#if AUDIOH_LOG_ENABLED
#define AUDIOH_LOG                           KNLLOG
#else
#define AUDIOH_LOG(c,args...)
#endif

#define CAPH_DMA_NUM_DEVICE_ENTRIES          AADMA_NUM_DEVICE_ENTRIES

/* DMA constants */
#define AUDIOH_MAX_MONO_DMA_BUFFER_SIZE_BYTES    (480*2)  /* 5ms 48kHz mono frames double buffer */
#define AUDIOH_MAX_STEREO_DMA_BUFFER_SIZE_BYTES  (960*2)  /* 5ms 48kHz stereo frames double buffer */
#define AUDIOH_DMA_ALIGN_IN_BYTES                8        /* 64-bit alignment */
#define AUDIOH_DMA_ALLOC_CROSS_RESTRICT          4096     /* Buffers cannot cross 1kb boundary */

#define AUDIOH_DEFAULT_SAMP_FREQ                 48000    /* In Hz */
#define AUDIOH_DEFAULT_FRAME_PERIOD              5000     /* In usec */
#define AUDIOH_DEFAULT_FRAME_SIZE                480      /* In bytes (mono frame) */
#define AUDIOH_CALC_PRIME_SIZE(frame)            ( (frame * 5) / 2 )
#define AUDIOH_DEFAULT_PRIME_SIZE_BYTES          AUDIOH_CALC_PRIME_SIZE(AUDIOH_DEFAULT_FRAME_SIZE)
#define AUDIOH_SAMP_WIDTH                        2

#define AUDIOH_MAX_DMA_BUF_SIZE_BYTES     (AUDIOH_MAX_STEREO_DMA_BUFFER_SIZE_BYTES * 2)

/* Priming */
#define AUDIOH_MAX_PRIME_SIZE_BYTES       (AUDIOH_MAX_STEREO_DMA_BUFFER_SIZE_BYTES * 2)  /* Worst case possible size */

/* Q12 gain map min and max ranges */
#define MAX_Q12_GAIN                6
#define MIN_Q12_GAIN                -50

#define AUDIOH_MAX_SINT16      0x7FFF
#define AUDIOH_MIN_SINT16      (-AUDIOH_MAX_SINT16-1)

#define AUDIOH_DEFAULT_THRES        4
#define AUDIOH_DEFAULT_THRES2       2

/* Frequency settings */
#define AUDIOH_MAX_NUM_SUPPORTED_FREQ        3

/* Power up ramp delay time */
#define AUDIOH_DACPWRUP_SETTLE_TIME              (400)         /* Milli Seconds */
#define AUDIOH_SLOWRAMP_PWRDN_PULSE_TIME         (150)         /* Micro Seconds */
#define AUDIOH_SLOWRAMP_RAMP1UP_TIME             (40)          /* Milli Seconds */
#define AUDIOH_SLOWRAMP_RAMP2UP_TIME             (5)           /* Milli Seconds */
#define AUDIOH_SLOWRAMP_RAMP2DOWN_TIME           (35)          /* Milli Seconds */
#define AUDIOH_SLOWRAMP_RAMP1DOWN_TIME           (10)          /* Milli Seconds */
#define AUDIOH_PATHENDIS_SETTLING_TIME           (1)           /* Milli Seconds */

#define AUDIOH_DISABLE_CAPH_TIME                 5           /* Milli Seconds */

/* With this slope a taget gain change of 2000, willl take 256 samples */
#define AUDIOH_SIDETONE_DEF_LINEAR_SOF_SLOPE          0x4000
#define AUDIOH_DEF_SIDETONE_CTRL_TAPS                 63

#define CAPH_DEF_BURST_SIZE      0x4
#define CAPH_CFIFO_MAX_CFIFOSIZE 0x2000
#define CAPH_MAX_CHANNELS        16

#define CAPH_CFIFO_EG_SIZE       32
#define CAPH_CFIFO_EG_THRES      16
#define CAPH_CFIFO_IG_SIZE       32
#define CAPH_CFIFO_IG_THRES      16

/* Synchronization check. If the sampling frequency is a multiple of 8
 * then it can be synchronized with HAL Audio
 */
#define AUDIOH_SYNC_FREQ( freq )             (((freq) % 8 ) == 0)

struct gain_map
{
   int db_val;
   int reg_val;
};

const static struct gain_map audiohAnaGainMap[] =
{
   { 0, 0x0 },       /* 0 dB */
   { 3, 0x1 },       /* 2.96 dB */
   { 6, 0x2 },       /* 6.06 dB */
   { 9, 0x3 },       /* 8.98 dB */
   { 12, 0x4 },      /* 12.08 dB */
   { 15, 0x5 },      /* 15 dB */
   { 18, 0x20 },     /* 17.71 dB */
   { 19, 0x21 },     /* 18.50 dB */
   { 20, 0x23 },     /* 20.34 dB */
   { 21, 0x24 },     /* 21.05 dB */
   { 22, 0x25 },     /* 21.83 dB */
   { 23, 0x26 },     /* 22.68 dB */
   { 24, 0x27 },     /* 23.62 dB */
   { 25, 0x29 },     /* 25.26 dB */
   { 26, 0x2A },     /* 25.88 dB */
   { 27, 0x2C },     /* 27.28 dB */
   { 28, 0x2D },     /* 28.07 dB */
   { 29, 0x2E },     /* 28.95 dB */
   { 30, 0x2F },     /* 29.92 dB */
   { 31, 0x30 },     /* 31.01 dB */
   { 32, 0x32 },     /* 32.26 dB */
   { 33, 0x33 },     /* 32.96 dB */
   { 34, 0x34 },     /* 33.75 dB */
   { 35, 0x35 },     /* 34.56 dB */
   { 36, 0x36 },     /* 35.49 dB */
   { 37, 0x38 },     /* 37.29 dB */
   { 38, 0x39 },     /* 38.13 dB */
   { 39, 0x3A },     /* 39.06 dB */
   { 40, 0x3B },     /* 40.10 dB */
   { 41, 0x3C },     /* 40.97 dB */
   { 42, 0x3D },     /* 41.94 dB */
   { 43, 0x3E },     /* 43.03 dB */
   { 44, 0x3F },     /* 44.02 dB */
};

/**
 * dB to 16-bit Q12 multiplier map. Used in setting software gains.
 * Ranges from -50dB to +18db (4096 = 0dB)
 */
const static short q12GainMap[] =
{
   13, 15, 16, 18, 21, 23, 26, 29, 33, 37,
   41, 46, 52, 58, 65, 73, 82, 92, 103, 115,
   130, 145, 163, 183, 205, 230, 258, 290, 325, 365,
   410, 460, 516, 579, 649, 728, 817, 917, 1029, 1154,
   1295, 1453, 1631, 1830, 2053, 2303, 2584, 2900, 3254, 3651,
   4096, 4596, 5157, 5786, 6492, 7284, 8173, 9170, 10289, 11544,
   12953, 14533, 16306, 18296, 20529, 23034, 25844, 28997, 32536
};

const static uint32_t sidetoneFirCoeff[] = {
   0x44713DB5, 0xC4148C74, 0xC4B183BC, 0xC52FCD37,
   0xC594E166, 0xC5E177A4, 0xC61CA31D, 0xC64A7445,
   0xC6750D3D, 0xC68B156D, 0xC69380EB, 0xC6908530,
   0xC67EFD7E, 0xC63DD965, 0xC5BBB314, 0x44E6D563,
   0x4629DBC5, 0x469E5C7E, 0x46E2D947, 0x470CC6F4,
   0x471CCE45, 0x471DA349, 0x470C8152, 0x46D0AFB7,
   0x4648BA9F, 0xC594E25B, 0xC6C0AF75, 0xC72F540D,
   0xC7767D34, 0xC7961F47, 0xC7A3CA86, 0xC7A09E3A,
   0xC78A6820, 0xC741C08B, 0xC697E4CE, 0x46881C13,
   0x4761628F, 0x47BDDD9E, 0x48002F7F, 0x48177620,
   0x482095EF, 0x481863B1, 0x47FA6AE1, 0x479E8D7E,
   0x4687B77C, 0xC764B1EA, 0xC808960C, 0xC8551F68,
   0xC88AD2BE, 0xC8A06647, 0xC8A6EE90, 0xC89AD23F,
   0xC8734364, 0xC804D4E7, 0x46A12838, 0x48529DB1,
   0x48D714D7, 0x4926DEB9, 0x4963490D, 0x498E92BE,
   0x49A867A2, 0x49BD7593, 0x49CC569C, 0x49D40A1C,
   0x49D40A1C, 0x49CC569C, 0x49BD7593, 0x49A867A2,
   0x498E92BE, 0x4963490D, 0x4926DEB9, 0x48D714D7,
   0x48529DB1, 0x46A12838, 0xC804D4E7, 0xC8734364,
   0xC89AD23F, 0xC8A6EE90, 0xC8A06647, 0xC88AD2BE,
   0xC8551F68, 0xC808960C, 0xC764B1EA, 0x4687B77C,
   0x479E8D7E, 0x47FA6AE1, 0x481863B1, 0x482095EF,
   0x48177620, 0x48002F7F, 0x47BDDD9E, 0x4761628F,
   0x46881C13, 0xC697E4CE, 0xC741C08B, 0xC78A6820,
   0xC7A09E3A, 0xC7A3CA86, 0xC7961F47, 0xC7767D34,
   0xC72F540D, 0xC6C0AF75, 0xC594E25B, 0x4648BA9F,
   0x46D0AFB7, 0x470C8152, 0x471DA349, 0x471CCE45,
   0x470CC6F4, 0x46E2D947, 0x469E5C7E, 0x4629DBC5,
   0x44E6D563, 0xC5BBB314, 0xC63DD965, 0xC67EFD7E,
   0xC6908530, 0xC69380EB, 0xC68B156D, 0xC6750D3D,
   0xC64A7445, 0xC61CA31D, 0xC5E177A4, 0xC594E166,
   0xC52FCD37, 0xC4B183BC, 0xC4148C74, 0x44713DB5,
};


/* Audio Hub only supports stereo FIFO channels */
#define AUDIOH_MAX_FIFO_CHANNELS    2
#define AUDIOH_MAX_MUX_POSITIONS    3

enum audioh_vin_right_fifo_ch
{
   AUDIOH_VIN_RIGHT_FIFO_CH_PRIM_HEAD_HAND_MIC = 0,

   AUDIOH_MAX_VIN_RIGHT_FIFO_CHANNELS
};

enum audioh_vin_left_fifo_ch
{
   AUDIOH_VIN_LEFT_FIFO_CH_SECONDARY_MIC,

   AUDIOH_MAX_VIN_LEFT_FIFO_CHANNELS
};

enum audioh_vin_right_mux_pos
{
   AUDIOH_VIN_RIGHT_MUX_POS_PRIMARY_MIC = 0,
   AUDIOH_VIN_RIGHT_MUX_POS_HEADSET_MIC,
   AUDIOH_VIN_RIGHT_MUX_POS_HANDSET_MIC,

   AUDIOH_MAX_VIN_RIGHT_MUX_POSITIONS
};

enum audioh_vin_left_mux
{
   AUDIOH_VIN_LEFT_MUX_POS_SECONDARY_MIC = 0,

   AUDIOH_MAX_VIN_LEFT_MUX_POSITIONS
};

enum audioh_nvin_right_fifo_ch
{
   AUDIOH_NVIN_RIGHT_FIFO_CH_ERR_FBACK_MIC = 0,

   AUDIOH_MAX_NVIN_RIGHT_FIFO_CHANNELS
};

enum audioh_nvin_left_fifo_ch
{
   AUDIOH_NVIN_LEFT_FIFO_CH_NOISE_REF_MIC,

   AUDIOH_MAX_NVIN_LEFT_FIFO_CHANNELS
};

enum audioh_nvin_right_mux_pos
{
   AUDIOH_NVIN_RIGHT_MUX_POS_ERR_FBACK_MIC = 0,

   AUDIOH_MAX_NVIN_RIGHT_MUX_POSITIONS
};

enum audioh_nvin_left_mux_pos
{
   AUDIOH_NVIN_LEFT_MUX_POS_NOISE_REF_MIC = 0,

   AUDIOH_MAX_NVIN_LEFT_MUX_POSITIONS
};

enum audioh_earpath_fifo_ch
{
   AUDIOH_EARPATH_MONO_FIFO_CH_EARPIECE_SPK = 0,

   AUDIOH_MAX_EARPATH_FIFO_CHANNELS
};

enum audioh_earpath_mux_pos
{
   AUDIOH_EARPATH_MUX_POS_EARPIECE_SPK = 0,

   AUDIOH_MAX_EARPATH_MUX_POSITIONS
};

enum audioh_hspath_fifo_ch
{
   AUDIOH_HSPATH_LEFT_FIFO_CH_HEADSET_LEFT_SPK = 0,
   AUDIOH_HSPATH_RIGHT_FIFO_CH_HEADSET_RIGHT_SPK,

   AUDIOH_MAX_HSPATH_FIFO_CHANNELS
};

enum audioh_hspath_left_mux_pos
{
   AUDIOH_HSPATH_LEFT_MUX_POS_HEADSET_LEFT_SPK = 0,

   AUDIOH_MAX_HSPATH_LEFT_MUX_POSITIONS
};

enum audioh_hspath_right_mux_pos
{
   AUDIOH_HSPATH_RIGHT_MUX_POS_HEADSET_RIGHT_SPK = 0,

   AUDIOH_MAX_HSPATH_RIGHT_MUX_POSITIONS
};

enum audioh_ihfpath_fifo_ch
{
   AUDIOH_IHFPATH_MONO_FIFO_CH_HANDSFREE_SPK = 0,

   AUDIOH_MAX_IHFPATH_FIFO_CHANNELS
};

enum audioh_ihfpath_mux_pos
{
   AUDIOH_IHFPATH_MUX_POS_HANDSFREE_SPK = 0,

   AUDIOH_MAX_IHFPATH_MUX_POSITIONS
};

enum audioh_vibra_fifo_ch
{
   AUDIOH_VIBRA_MONO_FIFO_CH_MOTOR = 0,

   AUDIOH_MAX_VIBRA_FIFO_CHANNELS
};

enum audioh_vibra_mux_pos
{
   AUDIOH_VIBRA_MUX_POS_MOTOR,

   AUDIOH_MAX_VIBRA_MUX_POSITIONS
};

/* Codec channel enumeration */
enum audioh_chan
{
#if AUDIOH_VIN_RIGHT_ENABLE
   AUDIOH_CHAN_VIN_RIGHT = 0,
#endif
#if AUDIOH_VIN_LEFT_ENABLE
   AUDIOH_CHAN_VIN_LEFT,
#endif
#if AUDIOH_NVIN_RIGHT_ENABLE
   AUDIOH_CHAN_NVIN_RIGHT,
#endif
#if AUDIOH_NVIN_LEFT_ENABLE
   AUDIOH_CHAN_NVIN_LEFT,
#endif
#if AUDIOH_EARPIECE_ENABLE
   AUDIOH_CHAN_EARPIECE,
#endif
#if AUDIOH_HANDSFREE_ENABLE
   AUDIOH_CHAN_HANDSFREE,
#endif
#if AUDIOH_HEADSET_ENABLE
   AUDIOH_CHAN_HEADSET,
#endif
#if AUDIOH_VIBRA_ENABLE
   AUDIOH_CHAN_VIBRA,
#endif

   AUDIOH_MAX_NUM_CHANS
};

enum audioh_ch_dir
{
   AUDIOH_CH_DIR_INGRESS = 0,
   AUDIOH_CH_DIR_EGRESS
};

/* Audioh FIFO Status Function Pointer */
typedef uint32_t (*audiohFifoStatusFp)(CHAL_HANDLE handle);

typedef void* CAPH_DMA_Handle_t;

/* CAPH DMA Device attribute */
struct caph_dma_device_attr
{
   int                           allocated;        /* Flag to indicate allocated */
   AADMA_Handle_t                aadma_handle;     /* AADMA driver handle */
   SSASW_Handle_t                ssasw_handle;     /* SSASW driver handle */
};

/* Clocks struct */
struct audioh_clks
{
   struct clk           *audioh_156m;
   struct clk           *audioh_26m;
   struct clk           *audioh_2p4m;
   struct clk           *audioh_apb;
};

/* CSX Data stucture */
struct audioh_csx_data
{
   CSX_IO_POINT_FNCS    csx_ops;
   void                *priv;
};

/* Audio Hub fifo error statistics */
struct audioh_ch_errs
{
   unsigned int            dma_err;          /* DMA path errors */
   unsigned int            dma_wrong_dir;    /* Unsupported DMA direction requested */
   unsigned int            dma_fifo_err;     /* DMA FIFO errors */
   unsigned int            dma_fifo_thrs_err;/* DMA FIFO threshold errors */
   unsigned int            fifo_underflow;   /* FIFO underflow */
   unsigned int            fifo_overflow;    /* FIFO overflow */
};

struct audioh_fifo_mask
{
   audiohFifoStatusFp      getFifoStatus;    /* Fifo status function pointer */
   unsigned int            isr_fifo_err_mask;/* ISR fifo error mask */
   unsigned int            isr_fifo_ths_mask;/* ISR fifo threshold mask */
};

/* Analog gain state */
struct audioh_ana_gain
{
   const int               ana_gain_sup;     /* Support for analog gain */
   int                     ana_gain_value;   /* Analog gain in dB */
};

/* Audio Hub gain state */
struct audioh_gain
{
   const int                num_mux_pos;                         /* Number of mux positons per fifo channel */
   struct audioh_ana_gain   ana_gain[AUDIOH_MAX_MUX_POSITIONS];  /* Analog gain per mux position */
};

/* Audio Hub sidetone state */
struct audioh_sidetone
{
   int db;               /* Gain in dB */
   int enable;           /* Enable flag */
};

/* Audio Hub channel configuration and parameters */
struct audioh_ch_cfg
{
   /* Channel configuration */
   const unsigned int            ch_no;         /* Codec channel number */
   const enum audioh_ch_dir      ch_dir;        /* Indicate the channel direction.  Uni-directional codec channels */
   unsigned int                  num_fifo_ch;   /* Number of fifo channels to interleave. (1=mono, 2=stereo) */
   struct dma_cfg                dma_config;    /* DMA config */
   struct dma_data_buf           dma_buf[2];    /* DMA double buffer */
   SSASW_Device_t                switch_dev;    /* CAPH Switch trigger condition setting */
   unsigned int                  samp_freq;     /* Sampling frequency in Hz */
   unsigned int                  frame_period;  /* Frame period in usec */
   unsigned int                  frame_size;    /* Frame size in bytes */
   unsigned int                  prime_egr;     /* Egress priming in bytes */
   int                           anadb;         /* cached analog DB */
   HALAUDIO_HWSEL                anahwsel;      /* cached HW selection */
   HALAUDIO_EQU                  equ;           /* Equalizer parameters */

   atomic_t                      ind_active_idx;/* Per channel active index to active buffer when running independant */

   /* Hardware block states */
   struct audioh_gain            audio_gain[AUDIOH_MAX_FIFO_CHANNELS];  /* Active microphone audio settings per fifo channel*/
   struct audioh_sidetone        sidetone;      /* Active sidetone settings */
   int16_t                       dig_gain;      /* Digital gain in dB per codec channel */

   /* Write state */
   HALAUDIO_WRITE                write;         /* Write state */

   /* ISR status */
   atomic_t                      queued_pkts_egr;  /* Num of egress packets awaiting to be DMA'd. Should not exceed 2. When 0, means DMA is idle */

   struct audioh_ch_errs         errs;          /* Channel errors */

   struct audioh_fifo_mask       masks;         /* Channel masks */

   /* Debug facilities */
   int                           debug;         /* Flag to indicate in debug mode */
   int                           ramp;          /* Sysctl: ramp generation */
   unsigned short                rampseed;      /* Ramp seed */
   HALAUDIO_SINECTL              sinectl;

   /* Mixer facilities */
   AMXR_PORT_ID                  mixer_port;    /* Mixer port handle for channel */

   /* CSX data */
   struct audioh_csx_data        csx_data[HALAUDIO_NUM_CSX_POINTS]; /* Array of CSX data structures */

   /* Debug facilities */
   unsigned int                  audioh_isrcount;
};

/* Audio Hub channel information structure */
struct audioh_info
{
   int                           initialized;
   atomic_t                      prepared;      /* Flag indicating channels are prepared to be enabled */
   atomic_t                      running;       /* Flag indicating channels are active */
   int                           thresh_intr_en;/* Flag indicating to enable threshold interrupts */
   unsigned int                  audioh_isr_cnt;/* Counter for AudioH ISRs */
   struct dma_data_buf           zero;          /* DMA scratch buffer used for priming */
   struct audioh_ch_cfg          ch[AUDIOH_MAX_NUM_CHANS];
};

struct freq_map
{
   unsigned int                  freq;          /* Frequency entry allowable */
   unsigned int                  num_ch;        /* Number of channels associated with frequency */
   unsigned int                  frame_period;  /* Frame period setting for frequency */
   SSASW_Device_t                switch_dev;    /* Switch device associated with frequency */
};

struct freq_map_array
{
   struct freq_map freq_cfg_array[AUDIOH_MAX_NUM_SUPPORTED_FREQ];
};

/* ---- Private Variables ------------------------------------ */

/* HAL Audio interface handle */
static HALAUDIO_IF_HDL     gInterfHandle;

/* Static allocation of Audio Hub control block */
static ChalAudioCtrlBlk_t  gAudioHubCtrlBlk;

/* CHAL layer Audio Hub Handle */
static CHAL_HANDLE         gChalAudioHandle;

/* DMA buffer pool */
static struct dma_pool    *gDmaPool;

/* Installed callback. Called when all ingress processing has completed */
static HALAUDIO_IF_FRAME_ELAPSED gAudiohElapsedCb;

/* Private user data to pass in with assigned callback function */
static void               *gAudiohUserData;

/* Reference counter used to determine when all ingress interrupt processing
 * has completed.  Used as a timing mechanism.
 */
static atomic_t            gAudiohRefIgrCount;

static atomic_t            gActive_ig_idx;       /* Index to active buffer in ingress double buffers */
static atomic_t            gActive_eg_idx;       /* Index to active buffer in egress double buffers */

static atomic_t            gPwrDacLevel;
static atomic_t            gPwrTransducerLevel;

/* Ingress channel counter */
static int                 gNumIngressChannels = 0;

/* Platform information */
static HALAUDIO_AUDIOH_PLATFORM_INFO gAudiohPlatformInfo;

static struct audioh_clks  gAudiohClk;

/* CAPH DMA Device attribute array */
static struct caph_dma_device_attr gCaphDmaDevAttr[CAPH_DMA_NUM_DEVICE_ENTRIES];

struct semaphore gProcBufLock;   /* acquired when accessing buffer in proc entry */
struct semaphore gPwrLock;       /* acquired when powering up/down transducers and dacs */

/* Proc entry buffer */
int16_t gProcbuf_active[AUDIOH_MAX_DMA_BUF_SIZE_BYTES];

static short gHalFiltHist[HALAUDIO_EQU_COEFS_MAX_NUM];

const static struct freq_map_array gAudiohFreqMap[AUDIOH_MAX_NUM_CHANS] =
{
#if AUDIOH_VIN_RIGHT_ENABLE
   /* Primary Speech mic, Headset mic, Handset mic */
   [AUDIOH_CHAN_VIN_RIGHT] =
   {
      .freq_cfg_array =
      {
         [0] =
         {
            .freq          = 48000,
            .num_ch        = 1,
            .frame_period  = AUDIOH_DEFAULT_FRAME_PERIOD,
            .switch_dev    = SSASW_AUDIOH_VINR,
         },
      }
   },
#endif
#if AUDIOH_VIN_LEFT_ENABLE
   /* Secondary Speech mic */
   [AUDIOH_CHAN_VIN_LEFT] =
   {
      .freq_cfg_array =
      {
         [0] =
         {
            .freq          = 48000,
            .num_ch        = 1,
            .frame_period  = AUDIOH_DEFAULT_FRAME_PERIOD,
            .switch_dev    = SSASW_AUDIOH_VINL,
         },
      }
   },
#endif
#if AUDIOH_NVIN_RIGHT_ENABLE
   /* Error Feedback mic */
   [AUDIOH_CHAN_NVIN_RIGHT] =
   {
      .freq_cfg_array =
      {
         [0] =
         {
            .freq          = 48000,
            .num_ch        = 1,
            .frame_period  = AUDIOH_DEFAULT_FRAME_PERIOD,
            .switch_dev    = SSASW_AUDIOH_NVINR,
         },
      }
   },
#endif
#if AUDIOH_NVIN_LEFT_ENABLE
   /* Noise Reference mic */
   [AUDIOH_CHAN_NVIN_LEFT] =
   {
      .freq_cfg_array =
      {
         [0] =
         {
            .freq          = 48000,
            .num_ch        = 1,
            .frame_period  = AUDIOH_DEFAULT_FRAME_PERIOD,
            .switch_dev    = SSASW_AUDIOH_NVINL,
         },
      }
   },
#endif
#if AUDIOH_EARPIECE_ENABLE
   /* Earpiece speaker */
   [AUDIOH_CHAN_EARPIECE] =
   {
      .freq_cfg_array =
      {
         [0] =
         {
            .freq          = 48000,
            .num_ch        = 1,
            .frame_period  = AUDIOH_DEFAULT_FRAME_PERIOD,
            .switch_dev    = SSASW_AUDIOH_EARPIECE,
         },
         [1] =
         {
            .freq          = 44100,
            .num_ch        = 0,     /* Special case where we source from headset channel */
            .frame_period  = (AUDIOH_DEFAULT_FRAME_PERIOD * 2),
            .switch_dev    = SSASW_SRC_44100_AUDIOH_EARPIECE,
         },
      }
   },
#endif
#if AUDIOH_HEADSET_ENABLE
   /* Headset speakers */
   [AUDIOH_CHAN_HEADSET] =
   {
      .freq_cfg_array =
      {
         [0] =
         {
            .freq          = 48000,
            .num_ch        = 2,
            .frame_period  = AUDIOH_DEFAULT_FRAME_PERIOD,
            .switch_dev    = SSASW_AUDIOH_HEADSET,
         },
         [1] =
         {
            .freq          = 44100,
            .num_ch        = 2,
            .frame_period  = (AUDIOH_DEFAULT_FRAME_PERIOD * 2),
            .switch_dev    = SSASW_SRC_44100_AUDIOH_HEADSET,
         }
      }
   },
#endif
#if AUDIOH_HANDSFREE_ENABLE
   /* Handsfree speaker */
   [AUDIOH_CHAN_HANDSFREE] =
   {
      .freq_cfg_array =
      {
         [0] =
         {
            .freq          = 48000,
            .num_ch        = 1,
            .frame_period  = AUDIOH_DEFAULT_FRAME_PERIOD,
            .switch_dev    = SSASW_AUDIOH_HANDSFREE,
         },
         [1] =
         {
            .freq          = 44100,
            .num_ch        = 0,  /* Special case where we source from headset channel */
            .frame_period  = (AUDIOH_DEFAULT_FRAME_PERIOD * 2),
            .switch_dev    = SSASW_SRC_44100_AUDIOH_HANDSFREE,
         },
      }
   },
#endif
#if AUDIOH_VIBRA_ENABLE
   /* Vibra motor */
   [AUDIOH_CHAN_VIBRA] =
   {
      .freq_cfg_array =
      {
         [0] =
         {
            .freq          = 48000,
            .num_ch        = 1,
            .frame_period  = AUDIOH_DEFAULT_FRAME_PERIOD,
            .switch_dev    = SSASW_AUDIOH_VIBRA,
         },
      }
   },
#endif
};

static struct audioh_info gAudioh =
{
   .ch =
   {
#if AUDIOH_VIN_RIGHT_ENABLE
   /* Primary Speech mic, Headset mic, Handset mic */
      [AUDIOH_CHAN_VIN_RIGHT] =
      {
         .ch_no            = AUDIOH_CHAN_VIN_RIGHT,
         .ch_dir           = AUDIOH_CH_DIR_INGRESS,
         .num_fifo_ch      = AUDIOH_MAX_VIN_RIGHT_FIFO_CHANNELS,
         .switch_dev       = SSASW_AUDIOH_VINR,
         .samp_freq        = AUDIOH_DEFAULT_SAMP_FREQ,
         .frame_period     = AUDIOH_DEFAULT_FRAME_PERIOD,
         .frame_size       = (AUDIOH_DEFAULT_FRAME_SIZE * AUDIOH_MAX_VIN_RIGHT_FIFO_CHANNELS ),
         .dma_config =
         {
            .device = AADMA_DEVICE_AUDIOH_VINR_CFIFO_TO_MEM,
            .fifo_addr = (AUDIOH_PHYS_BASE_ADDR_START + CHAL_AUDIO_VINR_FIFO_OFFSET),
         },
         .audio_gain =
         {
            [AUDIOH_VIN_RIGHT_FIFO_CH_PRIM_HEAD_HAND_MIC] =
            {
               .num_mux_pos = AUDIOH_MAX_VIN_RIGHT_MUX_POSITIONS,
               .ana_gain =
               {
                  [AUDIOH_VIN_RIGHT_MUX_POS_PRIMARY_MIC] =
                  {
                     .ana_gain_sup = 0,
                  },
                  [AUDIOH_VIN_RIGHT_MUX_POS_HEADSET_MIC] =
                  {
                     .ana_gain_sup = 1,
                  },
                  [AUDIOH_VIN_RIGHT_MUX_POS_HANDSET_MIC] =
                  {
                     .ana_gain_sup = 1,
                  },
               },
            },
         },
         .masks =
         {
            .getFifoStatus = chal_audio_vinpath_read_fifo_status,
            .isr_fifo_err_mask = CHAL_AUDIO_VIN_FIFO_ERR_MASK,
            .isr_fifo_ths_mask = CHAL_AUDIO_VIN_INT_MASK,
         },
      },
#endif
#if AUDIOH_VIN_LEFT_ENABLE
      /* Secondary Speech mic */
      [AUDIOH_CHAN_VIN_LEFT] =
      {
         .ch_no            = AUDIOH_CHAN_VIN_LEFT,
         .ch_dir           = AUDIOH_CH_DIR_INGRESS,
         .num_fifo_ch      = AUDIOH_MAX_VIN_LEFT_FIFO_CHANNELS,
         .switch_dev       = SSASW_AUDIOH_VINL,
         .samp_freq        = AUDIOH_DEFAULT_SAMP_FREQ,
         .frame_period     = AUDIOH_DEFAULT_FRAME_PERIOD,
         .frame_size       = (AUDIOH_DEFAULT_FRAME_SIZE * AUDIOH_MAX_VIN_LEFT_FIFO_CHANNELS ),
         .dma_config =
         {
            .device = AADMA_DEVICE_AUDIOH_VINL_CFIFO_TO_MEM,
            .fifo_addr = (AUDIOH_PHYS_BASE_ADDR_START + CHAL_AUDIO_VINL_FIFO_OFFSET),
         },
         .audio_gain =
         {
            [AUDIOH_VIN_LEFT_FIFO_CH_SECONDARY_MIC] =
            {
               .num_mux_pos = AUDIOH_MAX_VIN_LEFT_MUX_POSITIONS,
               .ana_gain =
               {
                  [AUDIOH_VIN_LEFT_MUX_POS_SECONDARY_MIC] =
                  {
                     .ana_gain_sup = 0,
                  }
               },
            },
         },
         .masks =
         {
            .getFifoStatus = chal_audio_vinpath_left_read_fifo_status,
            .isr_fifo_err_mask = CHAL_AUDIO_VIN_LEFT_FIFO_ERR_MASK,
            .isr_fifo_ths_mask = CHAL_AUDIO_VIN_LEFT_INT_MASK,
         },
      },
#endif
#if AUDIOH_NVIN_RIGHT_ENABLE
      /* Error Feedback mic */
      [AUDIOH_CHAN_NVIN_RIGHT] =
      {
         .ch_no            = AUDIOH_CHAN_NVIN_RIGHT,
         .ch_dir           = AUDIOH_CH_DIR_INGRESS,
         .num_fifo_ch      = AUDIOH_MAX_NVIN_RIGHT_FIFO_CHANNELS,
         .switch_dev       = SSASW_AUDIOH_NVINR,
         .samp_freq        = AUDIOH_DEFAULT_SAMP_FREQ,
         .frame_period     = AUDIOH_DEFAULT_FRAME_PERIOD,
         .frame_size       = (AUDIOH_DEFAULT_FRAME_SIZE * AUDIOH_MAX_NVIN_RIGHT_FIFO_CHANNELS),
         .dma_config =
         {
            .device = AADMA_DEVICE_AUDIOH_NVINR_CFIFO_TO_MEM,
            .fifo_addr = (AUDIOH_PHYS_BASE_ADDR_START + CHAL_AUDIO_NVINR_FIFO_OFFSET),
         },
         .audio_gain =
         {
            [AUDIOH_NVIN_RIGHT_FIFO_CH_ERR_FBACK_MIC] =
            {
               .num_mux_pos = AUDIOH_MAX_NVIN_RIGHT_MUX_POSITIONS,
               .ana_gain =
               {
                  [AUDIOH_NVIN_RIGHT_MUX_POS_ERR_FBACK_MIC] =
                  {
                     .ana_gain_sup = 0,
                  },
               },
            },
         },
         .masks =
         {
            .getFifoStatus = chal_audio_nvinpath_read_fifo_status,
            .isr_fifo_err_mask = CHAL_AUDIO_NVIN_FIFO_ERR_MASK,
            .isr_fifo_ths_mask = CHAL_AUDIO_NVIN_INT_MASK,
         },
      },
#endif
#if AUDIOH_NVIN_LEFT_ENABLE
      /* Noise Reference mic */
      [AUDIOH_CHAN_NVIN_LEFT] =
      {
         .ch_no            = AUDIOH_CHAN_NVIN_LEFT,
         .ch_dir           = AUDIOH_CH_DIR_INGRESS,
         .num_fifo_ch      = AUDIOH_MAX_NVIN_LEFT_FIFO_CHANNELS,
         .switch_dev       = SSASW_AUDIOH_NVINL,
         .samp_freq        = AUDIOH_DEFAULT_SAMP_FREQ,
         .frame_period     = AUDIOH_DEFAULT_FRAME_PERIOD,
         .frame_size       = (AUDIOH_DEFAULT_FRAME_SIZE * AUDIOH_MAX_NVIN_LEFT_FIFO_CHANNELS),
         .dma_config =
         {
            .device = AADMA_DEVICE_AUDIOH_NVINL_CFIFO_TO_MEM,
            .fifo_addr = (AUDIOH_PHYS_BASE_ADDR_START + CHAL_AUDIO_NVINL_FIFO_OFFSET),
         },
         .audio_gain =
         {
            [AUDIOH_NVIN_LEFT_FIFO_CH_NOISE_REF_MIC] =
            {
               .num_mux_pos = AUDIOH_MAX_NVIN_LEFT_MUX_POSITIONS,
               .ana_gain =
               {
                  [AUDIOH_NVIN_LEFT_MUX_POS_NOISE_REF_MIC] =
                  {
                     .ana_gain_sup = 0,
                  },
               },
            },
         },
         .masks =
         {
            .getFifoStatus = chal_audio_nvinpath_left_read_fifo_status,
            .isr_fifo_err_mask = CHAL_AUDIO_NVIN_LEFT_FIFO_ERR_MASK,
            .isr_fifo_ths_mask = CHAL_AUDIO_NVIN_LEFT_INT_MASK,
         },
      },
#endif
#if AUDIOH_EARPIECE_ENABLE
      /* Earpiece speaker */
      [AUDIOH_CHAN_EARPIECE] =
      {
         .ch_no            = AUDIOH_CHAN_EARPIECE,
         .ch_dir           = AUDIOH_CH_DIR_EGRESS,
         .num_fifo_ch      = AUDIOH_MAX_EARPATH_FIFO_CHANNELS,
         .switch_dev       = SSASW_AUDIOH_EARPIECE,
         .samp_freq        = AUDIOH_DEFAULT_SAMP_FREQ,
         .frame_period     = AUDIOH_DEFAULT_FRAME_PERIOD,
         .frame_size       = AUDIOH_DEFAULT_FRAME_SIZE,
         .prime_egr        = AUDIOH_DEFAULT_PRIME_SIZE_BYTES,
         .dma_config =
         {
            .device = AADMA_DEVICE_AUDIOH_MEM_TO_CFIFO_TO_EARPIECE,
            .fifo_addr = (AUDIOH_PHYS_BASE_ADDR_START + CHAL_AUDIO_EARPIECE_FIFO_OFFSET),
         },
         .audio_gain =
         {
            [AUDIOH_EARPATH_MONO_FIFO_CH_EARPIECE_SPK] =
            {
               .num_mux_pos  = AUDIOH_MAX_EARPATH_MUX_POSITIONS,
               .ana_gain =
               {
                  [AUDIOH_EARPATH_MUX_POS_EARPIECE_SPK] =
                  {
                     .ana_gain_sup = 0,
                  },
               },
            },
         },
         .masks =
         {
            .getFifoStatus = chal_audio_earpath_read_fifo_status,
            .isr_fifo_err_mask = CHAL_AUDIO_VOUT_FIFO_ERR_MASK,
            .isr_fifo_ths_mask = CHAL_AUDIO_VOUT_INT_MASK,
         },
      },
#endif
#if AUDIOH_HEADSET_ENABLE
      /* Headset speakers */
      [AUDIOH_CHAN_HEADSET] =
      {
         .ch_no            = AUDIOH_CHAN_HEADSET,
         .ch_dir           = AUDIOH_CH_DIR_EGRESS,
         .num_fifo_ch      = AUDIOH_MAX_HSPATH_FIFO_CHANNELS,
         .switch_dev       = SSASW_AUDIOH_HEADSET,
         .samp_freq        = AUDIOH_DEFAULT_SAMP_FREQ,
         .frame_period     = AUDIOH_DEFAULT_FRAME_PERIOD,
         .frame_size       = (AUDIOH_DEFAULT_FRAME_SIZE * AUDIOH_MAX_HSPATH_FIFO_CHANNELS),
         .prime_egr        = (AUDIOH_DEFAULT_PRIME_SIZE_BYTES * AUDIOH_MAX_HSPATH_FIFO_CHANNELS),
         .dma_config =
         {
            .device = AADMA_DEVICE_AUDIOH_MEM_TO_CFIFO_TO_HEADSET,
            .fifo_addr = (AUDIOH_PHYS_BASE_ADDR_START + CHAL_AUDIO_HEADSET_FIFO_OFFSET),
         },
         .audio_gain =
         {
            [AUDIOH_HSPATH_LEFT_FIFO_CH_HEADSET_LEFT_SPK] =
            {
               .num_mux_pos = AUDIOH_MAX_HSPATH_LEFT_MUX_POSITIONS,
               .ana_gain =
               {
                  [AUDIOH_HSPATH_LEFT_MUX_POS_HEADSET_LEFT_SPK] =
                  {
                     .ana_gain_sup = 0,
                  },
               },
            },
            [AUDIOH_HSPATH_RIGHT_FIFO_CH_HEADSET_RIGHT_SPK] =
            {
               .num_mux_pos = AUDIOH_MAX_HSPATH_RIGHT_MUX_POSITIONS,
               .ana_gain =
               {
                  [AUDIOH_HSPATH_RIGHT_MUX_POS_HEADSET_RIGHT_SPK] =
                  {
                     .ana_gain_sup = 0,
                  },
               },
            },
         },
         .masks =
         {
            .getFifoStatus = chal_audio_hspath_read_fifo_status,
            .isr_fifo_err_mask = CHAL_AUDIO_HS_FIFO_ERR_MASK,
            .isr_fifo_ths_mask = CHAL_AUDIO_HS_INT_MASK,
         },
      },
#endif
#if AUDIOH_HANDSFREE_ENABLE
      /* Handsfree speaker */
      [AUDIOH_CHAN_HANDSFREE] =
      {
         .ch_no            = AUDIOH_CHAN_HANDSFREE,
         .ch_dir           = AUDIOH_CH_DIR_EGRESS,
         .num_fifo_ch      = AUDIOH_MAX_IHFPATH_FIFO_CHANNELS,
         .switch_dev       = SSASW_AUDIOH_HANDSFREE,
         .samp_freq        = AUDIOH_DEFAULT_SAMP_FREQ,
         .frame_period     = AUDIOH_DEFAULT_FRAME_PERIOD,
         .frame_size       = AUDIOH_DEFAULT_FRAME_SIZE,
         .prime_egr        = AUDIOH_DEFAULT_PRIME_SIZE_BYTES,
         .dma_config =
         {
            .device = AADMA_DEVICE_AUDIOH_MEM_TO_CFIFO_TO_HANDSFREE,
            .fifo_addr = (AUDIOH_PHYS_BASE_ADDR_START + CHAL_AUDIO_IHF_FIFO_OFFSET),
         },
         .audio_gain =
         {
            [AUDIOH_IHFPATH_MONO_FIFO_CH_HANDSFREE_SPK] =
            {
               .num_mux_pos = AUDIOH_MAX_IHFPATH_MUX_POSITIONS,
               .ana_gain =
               {
                  [AUDIOH_IHFPATH_MUX_POS_HANDSFREE_SPK] =
                  {
                     .ana_gain_sup = 0,
                  },
               },
            },
         },
         .masks =
         {
            .getFifoStatus = chal_audio_ihfpath_read_fifo_status,
            .isr_fifo_err_mask = CHAL_AUDIO_IHF_FIFO_ERR_MASK,
            .isr_fifo_ths_mask = CHAL_AUDIO_IHF_INT_MASK,
         },
      },
#endif
#if AUDIOH_VIBRA_ENABLE
      /* Vibra motor */
      [AUDIOH_CHAN_VIBRA] =
      {
         .ch_no            = AUDIOH_CHAN_VIBRA,
         .ch_dir           = AUDIOH_CH_DIR_EGRESS,
         .num_fifo_ch      = AUDIOH_MAX_VIBRA_FIFO_CHANNELS,
         .switch_dev       = SSASW_AUDIOH_VIBRA,
         .samp_freq        = AUDIOH_DEFAULT_SAMP_FREQ,
         .frame_period     = AUDIOH_DEFAULT_FRAME_PERIOD,
         .frame_size       = AUDIOH_DEFAULT_FRAME_SIZE,
         .prime_egr        = AUDIOH_DEFAULT_PRIME_SIZE_BYTES,
         .dma_config =
         {
            .device = AADMA_DEVICE_AUDIOH_MEM_TO_CFIFO_TO_VIBRA,
            .fifo_addr = (AUDIOH_PHYS_BASE_ADDR_START + CHAL_AUDIO_VIBRA_FIFO_OFFSET),
         },
         .audio_gain =
         {
            [AUDIOH_VIBRA_MONO_FIFO_CH_MOTOR] =
            {
               .num_mux_pos = AUDIOH_MAX_VIBRA_MUX_POSITIONS,
               .ana_gain =
               {
                  [AUDIOH_VIBRA_MUX_POS_MOTOR] =
                  {
                     .ana_gain_sup = 0,
                  },
               },
            },
         },
         .masks =
         {
            .getFifoStatus = chal_audio_vibra_read_fifo_status,
            .isr_fifo_err_mask = CHAL_AUDIO_VIBRA_FIFO_ERR_MASK,
            .isr_fifo_ths_mask = CHAL_AUDIO_VIBRA_INT_MASK,
         },
      },
#endif
   },
};


/**
* Static sysctl data structures
*/
#define CTL_TABLE_INT(varStr,var) \
      procname: varStr,\
      data: var,\
      maxlen: sizeof(int),\
      mode: 0644,\
      proc_handler: &proc_dointvec,

static struct ctl_table_header *gSysCtlHeader;

static int proc_doSineGen( ctl_table *table, int write,
      void __user *buffer, size_t *lenp, loff_t *ppos );


#define AUDIOH_CHAN_SYS_CTL( CH ) \
   { \
      .procname      = "sine_freq_"#CH, \
      .data          = &gAudioh.ch[CH].sinectl.freq, \
      .maxlen        = sizeof(int), \
      .mode          = 0644, \
      .proc_handler  = &proc_doSineGen, \
      .extra1        = &gAudioh.ch[CH], \
   }, \
   { CTL_TABLE_INT( "ramp_"#CH,    &gAudioh.ch[CH].ramp ) }

static struct ctl_table gSysCtlAudioh[] =
{
#if AUDIOH_VIN_RIGHT_ENABLE
   AUDIOH_CHAN_SYS_CTL(AUDIOH_CHAN_VIN_RIGHT),
#endif
#if AUDIOH_VIN_LEFT_ENABLE
   AUDIOH_CHAN_SYS_CTL(AUDIOH_CHAN_VIN_LEFT),
#endif
#if AUDIOH_NVIN_RIGHT_ENABLE
   AUDIOH_CHAN_SYS_CTL(AUDIOH_CHAN_NVIN_RIGHT),
#endif
#if AUDIOH_NVIN_LEFT_ENABLE
   AUDIOH_CHAN_SYS_CTL(AUDIOH_CHAN_NVIN_LEFT),
#endif
#if AUDIOH_EARPIECE_ENABLE
   AUDIOH_CHAN_SYS_CTL(AUDIOH_CHAN_EARPIECE),
#endif
#if AUDIOH_HEADSET_ENABLE
   AUDIOH_CHAN_SYS_CTL(AUDIOH_CHAN_HEADSET),
#endif
#if AUDIOH_HANDSFREE_ENABLE
   AUDIOH_CHAN_SYS_CTL(AUDIOH_CHAN_HANDSFREE),
#endif
#if AUDIOH_VIBRA_ENABLE
   AUDIOH_CHAN_SYS_CTL(AUDIOH_CHAN_VIBRA),
#endif
   { CTL_TABLE_INT( "thresh_intr_en", &gAudioh.thresh_intr_en ) },
   {}
};

static struct ctl_table gSysCtl[] =
{
   {
      .procname   = "audioh",
      .mode       = 0555,
      .child      = gSysCtlAudioh
   },
   {}
};
static struct ctl_table_header  *gSysCtlHeader;

/* Static procfs dir entry */
static struct proc_dir_entry    *gProcDir;

/* ---- Private Function Prototypes -------------------------- */
static irqreturn_t audioh_isr( int irq, void *dev_id );
static int audiohDmaInit( void );
static int audiohDmaTerm( void );
static int audiohIoRemap( void );
static void audiohDmaEgressDoTransfer( struct audioh_ch_cfg *ch );
static int16_t *audiohMixerCb_BufGetIgPtr( int numBytes, void *privdata );
static int16_t *audiohMixerCb_BufGetEgPtr( int numBytes, void *privdata );
static void audiohMixerCb_EgressDone( int   numBytes, void *privdata );
static void audiohMixerCb_EgressFlush( void *privdata );
static void audiohDmaIngressHandler( AADMA_Device_t dev, int dma_status, void *userData );
static void audiohDmaSyncEgressHandler( AADMA_Device_t dev, int dma_status, void *userData );
static void audiohDmaEgressHandler( AADMA_Device_t dev, int dma_status, void *userData );
static int audiohMixerPortsRegister( void );
static int audiohMixerPortsDeregister( void );
static int audiohHwSelToMux( unsigned int ch_no, unsigned int *fifo_ch, unsigned int *mux_pos, HALAUDIO_HWSEL hwsel );
static int audiohResetGainBlocks( void );
static int audiohSoftDigGain( int16_t *samp, int num_samp, int db );
static int audiohMuxSelect( int ch_no, int fifo_ch, int mux_pos );
static int audiohChConfigAll( void );
#if AUDIOH_EARPIECE_ENABLE
static int audiohEarpathPowerRamp( int enable );
#endif
#if AUDIOH_HEADSET_ENABLE
static int audiohHeadsetpathEnable( int enable );
#endif
#if AUDIOH_HANDSFREE_ENABLE
static int audiohHandsfreepathEnable( int enable );
#endif
static int audiohChPowerTransducer( int powerup );
static int audiohChPowerDac( int powerup );
static int audiohFifoChGainSet( unsigned int chno, unsigned int fifo_ch, unsigned int mux_pos, int db );

static int audioh_platform_init( HALAUDIO_AUDIOH_PLATFORM_INFO *info );
static void audioh_platform_exit( HALAUDIO_AUDIOH_PLATFORM_INFO *info );

/* HAL Audio Interface Operation Functions */
static int audiohInit( HALAUDIO_IF_FRAME_ELAPSED isrcb, void *data );
static int audiohExit( void );
static int audiohPrepare( void );
static int audiohEnable( void );
static int audiohDisable( void );
static int audiohAnaPowerDown( int powerdn );
static int audiohPmShutdown( void );
static int audiohPmResume( void );
static int audiohSetFreq( int chno, int freqHz );
static int audiohGetFreq( int chno, int *freqHz );
static int audiohEquParmSet( int chno, HALAUDIO_DIR dir, const HALAUDIO_EQU *equ );
static int audiohEquParmGet( int chno, HALAUDIO_DIR dir, HALAUDIO_EQU *equ );
static int audiohAnaGainSet( int chno, int db, HALAUDIO_DIR dir, HALAUDIO_HWSEL hwsel );
static int audiohAnaGainGet( int chno, HALAUDIO_GAIN_INFO *info, HALAUDIO_DIR dir, HALAUDIO_HWSEL hwsel );
static int audiohDigGainSet( int chno, int db, HALAUDIO_DIR dir );
static int audiohDigGainGet( int chno, HALAUDIO_GAIN_INFO *info, HALAUDIO_DIR dir );
static int audiohSidetoneGainSet( int chno, int db );
static int audiohSidetoneGainGet( int chno, HALAUDIO_GAIN_INFO *info );
static int audiohWrite( int chno, int bytes, const char *audiobuf, HALAUDIO_CODEC_IORW_CB usercb, void *userdata );
static int audiohCodecInfo( int chno, HALAUDIO_CODEC_INFO *codec_info );
static int audiohCsxSet( int chno, HALAUDIO_CSX_POINT_ID point, const CSX_IO_POINT_FNCS *fncp, void *data );

/* Proc entries */
static void audiohProcInit( void );
static void audiohProcTerm( void );
static int  audiohReadProc( char *buf, char **start, off_t offset, int count, int *eof, void *data );

/* CAPH DMA Functions */
static int caph_dma_set_device_handler( AADMA_Device_t dev, AADMA_DeviceHandler_t dma_handler, void *data );
static CAPH_DMA_Handle_t caph_dma_request_channel( AADMA_Device_t aadma_dev, SSASW_Device_t switch_dev, enum audioh_ch_dir ch_dir, uint16_t audioh_fifo_addr );
static int caph_dma_transfer( CAPH_DMA_Handle_t handle, dma_addr_t data_addr, size_t numBytes );
static int caph_dma_alloc_ring_descriptors( CAPH_DMA_Handle_t, dma_addr_t data_addr, size_t numBytes );
static int caph_dma_start_transfer( CAPH_DMA_Handle_t handle );
static int caph_dma_continue_transfer( CAPH_DMA_Handle_t handle, int dma_status );
static int caph_dma_enable( CAPH_DMA_Handle_t handle );
static int caph_dma_sw_enable( CAPH_DMA_Handle_t handle );
static int caph_dma_mixer_enable( CAPH_DMA_Handle_t handle );
static int caph_dma_stop_transfer( CAPH_DMA_Handle_t handle );
static int caph_dma_free_channel( CAPH_DMA_Handle_t handle );

/* HAL Audio Audio Hub interface operations
 */
static HALAUDIO_IF_OPS halaudio_audioh_ops =
{
   .init                = audiohInit,
   .exit                = audiohExit,
   .prepare             = audiohPrepare,
   .enable              = audiohEnable,
   .disable             = audiohDisable,
   .ana_powerdn         = audiohAnaPowerDown,
   .pm_shutdown         = audiohPmShutdown,
   .pm_resume           = audiohPmResume,
   .codec_ops           =
   {
      .setfreq          = audiohSetFreq,
      .getfreq          = audiohGetFreq,
      .setana           = audiohAnaGainSet,
      .getana           = audiohAnaGainGet,
      .setdig           = audiohDigGainSet,
      .getdig           = audiohDigGainGet,
      .setequ           = audiohEquParmSet,
      .getequ           = audiohEquParmGet,
      .setsidetone      = audiohSidetoneGainSet,
      .getsidetone      = audiohSidetoneGainGet,
      .write            = audiohWrite,
      .info             = audiohCodecInfo,
      .setcsx           = audiohCsxSet,
   },
};

/* ---- Functions -------------------------------------------- */

/***************************************************************************/
/**
*  AUDIOH ISR for debug use only to detect DMA under and over runs
*
*  @return  irqreturn_t
*/
static irqreturn_t audioh_isr(
   int irq,             /**< IRQ number */
   void *dev_id         /**< user data */
)
{
   struct audioh_ch_cfg *ch = gAudioh.ch /*(struct audioh_ch_cfg *)dev_id*/;
   uint32_t intr_status;
   uint32_t fifo_status;
   int i;

   intr_status = chal_audio_read_int_status(gChalAudioHandle);
   chal_audio_int_clear(gChalAudioHandle, intr_status);

   AUDIOH_LOG( "Interrupt status: 0x%x", intr_status );
   gAudioh.audioh_isr_cnt++;

   for ( i = 0; i < AUDIOH_MAX_NUM_CHANS; i++, ch++ )
   {
      if( intr_status & ch->masks.isr_fifo_err_mask )
      {
         ch->errs.dma_fifo_err++;
      }
      if( intr_status & ch->masks.isr_fifo_ths_mask )
      {
         ch->errs.dma_fifo_thrs_err++;
      }
      if( ch->masks.getFifoStatus != NULL )
      {
         fifo_status = ch->masks.getFifoStatus(gChalAudioHandle);

         if(fifo_status & CHAL_AUDIO_FIFO_STATUS_UDF)
         {
            ch->errs.fifo_underflow++;
         }
         if(fifo_status & CHAL_AUDIO_FIFO_STATUS_OVF)
         {
            ch->errs.fifo_overflow++;
         }
      }
   }

   return IRQ_HANDLED;
}

/***************************************************************************/
/**
*  Helper routine to initialize DMA and enable DMA handshaking on FIFOs
*
*  @return  0 if successful, else error code
*
*  @remark
*
*/
static int audiohDmaInit( void )
{
   struct audioh_ch_cfg *ch;
   int i;
   int rc = 0;

   gDmaPool = dma_pool_create("Audio Hub DMA memory pool", NULL,
                              AUDIOH_MAX_DMA_BUF_SIZE_BYTES, AUDIOH_DMA_ALIGN_IN_BYTES,
                              AUDIOH_DMA_ALLOC_CROSS_RESTRICT );

   if( gDmaPool == NULL )
   {
      printk( KERN_ERR "AUDIOH: failed to allocate DMA buffer pool\n" );
      return -ENOMEM;
   }
   (void)rc;
   ch = gAudioh.ch;

   for ( i = 0; i < AUDIOH_MAX_NUM_CHANS; i++, ch++ )
   {
      ch->dma_buf[0].virt = dma_pool_alloc( gDmaPool, GFP_KERNEL, &ch->dma_buf[0].phys );
      ch->dma_buf[1].phys = ch->dma_buf[0].phys + ch->frame_size;
      ch->dma_buf[1].virt = ch->dma_buf[0].virt + ch->frame_size;

      if( !ch->dma_buf[0].virt || !ch->dma_buf[1].virt )
      {
         printk( KERN_ERR "AUDIOH: [CH %u] failed to allocate DMA audio buffers\n", ch->ch_no );
         rc = -ENOMEM;
         goto cleanup_exit;
      }
   }

   gAudioh.zero.virt = dma_alloc_writecombine( NULL, AUDIOH_MAX_PRIME_SIZE_BYTES,
            &gAudioh.zero.phys, GFP_KERNEL );

   if( gAudioh.zero.virt == NULL )
   {
      rc = -ENOMEM;
      goto cleanup_exit;
   }

   memset( gAudioh.zero.virt, 0, AUDIOH_MAX_PRIME_SIZE_BYTES );

#if AUDIOH_VIN_RIGHT_ENABLE
   /* Primary Speech mic, Headset mic, Handset mic */
   chal_audio_vinpath_dma_enable(gChalAudioHandle, CHAL_AUDIO_ENABLE);
   chal_audio_vinpath_set_fifo_thres( gChalAudioHandle, AUDIOH_DEFAULT_THRES, AUDIOH_DEFAULT_THRES2 );
#endif

#if AUDIOH_VIN_LEFT_ENABLE
   /* Secondary Speech mic */
   chal_audio_vinpath_left_dma_enable(gChalAudioHandle, CHAL_AUDIO_ENABLE);
   chal_audio_vinpath_set_fifo_thres( gChalAudioHandle, AUDIOH_DEFAULT_THRES, AUDIOH_DEFAULT_THRES2 );
#endif

#if AUDIOH_NVIN_RIGHT_ENABLE
   /* Noise reference mic */
   chal_audio_nvinpath_dma_enable(gChalAudioHandle, CHAL_AUDIO_ENABLE);
   chal_audio_nvinpath_set_fifo_thres( gChalAudioHandle, AUDIOH_DEFAULT_THRES, AUDIOH_DEFAULT_THRES2 );
#endif

#if AUDIOH_NVIN_LEFT_ENABLE
   /* Error Feedback mic*/
   chal_audio_nvinpath_left_dma_enable(gChalAudioHandle, CHAL_AUDIO_ENABLE);
   chal_audio_nvinpath_set_fifo_thres( gChalAudioHandle, AUDIOH_DEFAULT_THRES, AUDIOH_DEFAULT_THRES2 );
#endif

#if AUDIOH_EARPIECE_ENABLE
   /* Earpiece speaker */
   chal_audio_earpath_dma_enable(gChalAudioHandle, CHAL_AUDIO_ENABLE);
   chal_audio_earpath_set_fifo_thres( gChalAudioHandle, AUDIOH_DEFAULT_THRES, AUDIOH_DEFAULT_THRES2 );
#endif

#if AUDIOH_HEADSET_ENABLE
   /* Headset speakers */
   chal_audio_hspath_dma_enable(gChalAudioHandle, CHAL_AUDIO_ENABLE);
   chal_audio_hspath_set_fifo_thres( gChalAudioHandle, AUDIOH_DEFAULT_THRES, AUDIOH_DEFAULT_THRES2 );
#endif

#if AUDIOH_HANDSFREE_ENABLE
   /* Handsfree speaker */
   chal_audio_ihfpath_dma_enable(gChalAudioHandle, CHAL_AUDIO_ENABLE);
   chal_audio_ihfpath_set_fifo_thres( gChalAudioHandle, AUDIOH_DEFAULT_THRES, AUDIOH_DEFAULT_THRES2 );
#endif

#if AUDIOH_VIBRA_ENABLE
   /* Vibra motor */
   chal_audio_vibra_dma_enable(gChalAudioHandle, CHAL_AUDIO_ENABLE);
   chal_audio_vibra_set_fifo_thres( gChalAudioHandle, AUDIOH_DEFAULT_THRES, AUDIOH_DEFAULT_THRES2 );
#endif

#if AUDIOH_SIDETONE_SUPPORT
   chal_audio_stpath_set_fifo_thres( gChalAudioHandle, AUDIOH_DEFAULT_THRES, AUDIOH_DEFAULT_THRES2 );
#endif

   return 0;

cleanup_exit:
   audiohDmaTerm();
   return rc;
}

/***************************************************************************/
/**
*  Helper routine to free DMA memory allocation and disable DMA handshaking on FIFOs
*
*  @return  0 if successful, else error code
*
*  @remark
*
*/
static int audiohDmaTerm( void )
{
   int                i;
   struct audioh_ch_cfg *ch;

   ch = gAudioh.ch;

   for ( i = 0; i < AUDIOH_MAX_NUM_CHANS; i++, ch++ )
   {
      if( ch->dma_buf[0].virt )
      {
         dma_pool_free( gDmaPool, ch->dma_buf[0].virt, ch->dma_buf[0].phys );
         ch->dma_buf[0].virt = NULL;
         ch->dma_buf[1].virt = NULL;
      }
   }

   /* Disable all DMA paths regardless of supported */

   /* Primary Speech mic, Headset mic, Handset mic */
   chal_audio_vinpath_dma_enable(gChalAudioHandle, CHAL_AUDIO_DISABLE);

   /* Secondary Speech mic */
   chal_audio_vinpath_left_dma_enable(gChalAudioHandle, CHAL_AUDIO_DISABLE);

   /* Error Feedback mic */
   chal_audio_nvinpath_dma_enable(gChalAudioHandle, CHAL_AUDIO_DISABLE);

   /* Noise Reference mic*/
   chal_audio_nvinpath_left_dma_enable(gChalAudioHandle, CHAL_AUDIO_DISABLE);

   /* Earpiece speaker */
   chal_audio_earpath_dma_enable(gChalAudioHandle, CHAL_AUDIO_DISABLE);

   /* Headset speakers */
   chal_audio_hspath_dma_enable(gChalAudioHandle, CHAL_AUDIO_DISABLE);

   /* Handsfree speaker */
   chal_audio_ihfpath_dma_enable(gChalAudioHandle, CHAL_AUDIO_DISABLE);

   /* Vibra motor */
   chal_audio_vibra_dma_enable(gChalAudioHandle, CHAL_AUDIO_DISABLE);

   dma_pool_destroy( gDmaPool );

   if( gAudioh.zero.virt )
   {
      dma_free_writecombine( NULL, AUDIOH_MAX_PRIME_SIZE_BYTES,
            gAudioh.zero.virt, gAudioh.zero.phys );
      gAudioh.zero.virt = NULL;
   }

   return 0;
}

/***************************************************************************/
/**
*  Helper routine to memory region request and remap of memory to virtual addressing
*
*  @return  0 if successful, else error code
*
*  @remark
*
*/
static int audiohIoRemap( void )
{
   void __iomem *audioh_virt_addr = 0;
   void __iomem *sdt_virt_addr = 0;
   void __iomem *aci_virt_addr = 0;
   void __iomem *auxmic_virt_addr = 0;

   struct resource *audioh_ioarea;
   struct resource *sdt_ioarea;
   struct resource *aci_ioarea;

   audioh_ioarea = request_mem_region( AUDIOH_PHYS_BASE_ADDR_START, AUDIOH_REGISTER_LENGTH, "Audio Hub Mem Region" );
   sdt_ioarea = request_mem_region( SDT_PHYS_BASE_ADDR_START, SDT_REGISTER_LENGTH, "Sidetone Mem Region" );
   aci_ioarea = request_mem_region( ACI_PHYS_BASE_ADDR_START, ACI_REGISTER_LENGTH, "Accessory Component Mem Region" );

   if( !audioh_ioarea   ||
       !sdt_ioarea      ||
       !aci_ioarea )
   {
      return -EBUSY;
   }

   audioh_virt_addr = ioremap( AUDIOH_PHYS_BASE_ADDR_START, AUDIOH_REGISTER_LENGTH );
   sdt_virt_addr = ioremap( SDT_PHYS_BASE_ADDR_START, SDT_REGISTER_LENGTH );
   aci_virt_addr = ioremap( ACI_PHYS_BASE_ADDR_START, ACI_REGISTER_LENGTH );

   if( !audioh_virt_addr || !sdt_virt_addr || !aci_virt_addr )
   {
      return -ENOMEM;
   }

   /* ACI and AUXMIC share same address */
   auxmic_virt_addr = aci_virt_addr;

   gChalAudioHandle = (CHAL_HANDLE)(&gAudioHubCtrlBlk);

   chal_audio_init(gChalAudioHandle, (uint32_t)audioh_virt_addr, (uint32_t)sdt_virt_addr, (uint32_t)aci_virt_addr, (uint32_t)auxmic_virt_addr );

   return 0;
}

/***************************************************************************/
/**
*  Helper routine to do the egress DMA transfer for an AUDIOH channel
*
*  @return  None
*
*  @remark
*     It is expected that egress DMA transfers only occur after ingress
*     DMA transfers have completed.
*
*/
static void audiohDmaEgressDoTransfer(
   struct audioh_ch_cfg *ch         /** (io) Ptr to AUDIOH channel */
)
{
   void *egressp;
   int   rc;
   int   frame_size;
   int   active_idx;
   int   debug_mode = 0;

   if( ch->ch_dir == AUDIOH_CH_DIR_EGRESS )
   {
      active_idx = atomic_read(&gActive_eg_idx);

      egressp     = ch->dma_buf[active_idx].virt;
      frame_size  = ch->frame_size;

      /* Service write requests */
      halAudioWriteService( &ch->write, egressp, frame_size );

      /* Apply software digital gain based on channel settings */
      if( AUDIOH_SYNC_FREQ(ch->samp_freq) && ch->dig_gain != 0 )
      {
         audiohSoftDigGain( (int16_t *)egressp, (frame_size/AUDIOH_SAMP_WIDTH), ch->dig_gain );
      }

      if( ch->equ.len )
      {
         halAudioEquProcess( (int16_t *)egressp, ch->equ.coeffs, gHalFiltHist,
            ch->equ.len, (ch->frame_size/AUDIOH_SAMP_WIDTH) );
      }

      /* Service test and debug facilities */
      if( ch->ramp )
      {
         debug_mode = 1;
         halAudioGenerateRamp( (uint16_t *)egressp, &ch->rampseed, (frame_size/AUDIOH_SAMP_WIDTH), ch->num_fifo_ch );
      }
      else if( ch->sinectl.freq )
      {
         debug_mode = 1;
         halAudioSine( (uint16_t *)egressp, &ch->sinectl, (frame_size/AUDIOH_SAMP_WIDTH), ch->num_fifo_ch );
      }

      if( ch->csx_data[HALAUDIO_CSX_POINT_DAC].csx_ops.csxCallback )
      {
         debug_mode = 1;
         ch->csx_data[HALAUDIO_CSX_POINT_DAC].csx_ops.csxCallback( egressp, frame_size, ch->csx_data[HALAUDIO_CSX_POINT_DAC].priv );
      }

      if ( debug_mode )
      {
         ch->debug = 1;
      }
      else if ( ch->debug )
      {
         /* Clear all samples */
         memset( ch->dma_buf[0].virt, 0, ch->frame_size );
         memset( ch->dma_buf[1].virt, 0, ch->frame_size );

         ch->debug = 0;
      }

      AUDIOH_LOG( "active_idx=%d, ch=%d", active_idx, ch->ch_no );

      /* DMA egress samples */
      rc = caph_dma_transfer( ch->dma_config.caph_handle, ch->dma_buf[active_idx].phys, frame_size );
      if( rc )
      {
         ch->errs.dma_err++;
      }
   }
   else
   {
      ch->errs.dma_wrong_dir++;
   }
}


/***************************************************************************/
/**
*  AUDIOH mixer callback for outgoing data (i.e. ingress)
*
*  @return
*     NULL     - non-matching frame size or non-existent buffer
*     ptr      - pointer to egress buffer
*/
static int16_t *audiohMixerCb_BufGetIgPtr(
   int   numBytes,            /**< (i) frame size in bytes */
   void *privdata             /**< (i) private data */
)
{
   struct audioh_ch_cfg *ch;
   int16_t *ptr;
   int active_idx;

   ch  = (struct audioh_ch_cfg *)privdata;
   ptr = NULL;

   active_idx = atomic_read(&gActive_ig_idx);

   if( numBytes == ch->frame_size )
   {
      ptr = ch->dma_buf[active_idx].virt;
   }

   AUDIOH_LOG( "audiohMixerCb_BufGetIgPtr active_idx=%d, ch=%d", active_idx, ch->ch_no );

   return ptr;
}

/***************************************************************************/
/**
*  AUDIOH mixer callback for incoming data (i.e. egress)
*
*  @return
*     NULL     - non-matching frame size or non-existent buffer
*     ptr      - pointer to egress buffer
*/
static int16_t *audiohMixerCb_BufGetEgPtr(
   int   numBytes,            /**< (i) frame size in bytes */
   void *privdata             /**< (i) private data */
)
{
   struct audioh_ch_cfg *ch;
   int16_t *ptr;
   int active_idx;

   ch  = (struct audioh_ch_cfg *)privdata;
   ptr = NULL;

   if ( AUDIOH_SYNC_FREQ( ch->samp_freq ) )
   {
      active_idx = atomic_read(&gActive_eg_idx);
   }
   else
   {
      active_idx = atomic_read(&ch->ind_active_idx);
   }

   if( numBytes == ch->frame_size )
   {
      ptr = ch->dma_buf[active_idx].virt;
   }

   AUDIOH_LOG( "audiohMixerCb_BufGetEgPtr active_idx=%d, ch=%d", active_idx, ch->ch_no );

   return ptr;
}

/***************************************************************************/
/**
*  AUDIOH mixer callback to indicate that the egress data has been deposited.
*
*  @return     None
*
*  @remark
*     This callback is used as a trigger to DMA more data to the DAC, if
*     appropriate.
*/
static void audiohMixerCb_EgressDone(
   int   numBytes,            /**< (i) frame size in bytes */
   void *privdata             /**< (i) private data */
)
{
   struct audioh_ch_cfg *ch;

   ch  = (struct audioh_ch_cfg *)privdata;

   if ( AUDIOH_SYNC_FREQ( ch->samp_freq ) )
   {
      /* new packet arrived */
      atomic_inc( &ch->queued_pkts_egr );

      AUDIOH_LOG( "pkts=%d ch=%d", atomic_read( &ch->queued_pkts_egr ), ch->ch_no);

      /* Exactly 1 packet awaits, thus DMA was idle. Start a new transfer right away. */
      if( atomic_read( &ch->queued_pkts_egr ) == 1 )
      {
         audiohDmaEgressDoTransfer( ch );
      }
   }
   else
   {
      AUDIOH_LOG( "ch=%d", ch->ch_no);
   }
}

/***************************************************************************/
/**
*  AUDIOH mixer callback to flush the egress buffers when the last destination
*  connection is removed.
*
*  @return     None
*/
static void audiohMixerCb_EgressFlush(
   void *privdata             /*<< (i) private data */
)
{
   struct audioh_ch_cfg *ch;
   unsigned long flags;

   ch  = (struct audioh_ch_cfg *)privdata;

   /* Clear double buffers */
   local_irq_save( flags );
   memset( ch->dma_buf[0].virt, 0, AUDIOH_MAX_DMA_BUF_SIZE_BYTES );
   local_irq_restore( flags );
}

/***************************************************************************/
/**
*  AUDIOH ingress DMA interrupt handler that services ingress DMA's for all the
*  AUDIOH channels.
*
*  @return  None
*/
static void audiohDmaIngressHandler(
   AADMA_Device_t dev,
   int dma_status,
   void *userData
)
{
   struct audioh_ch_cfg *ch;
   unsigned short       *ingressp;
   int active_ig_idx = atomic_read(&gActive_ig_idx);
   int active_eg_idx = atomic_read(&gActive_eg_idx);
   int debug_mode = 0;
   int rc = 0;

   ch = userData;

   AUDIOH_LOG( "isr=%u ch=%i jiffies=%lu", ch->audioh_isrcount, ch->ch_no, jiffies );
   ch->audioh_isrcount++;

   if( ch->ch_dir == AUDIOH_CH_DIR_INGRESS )
   {
      if( atomic_read( &gAudioh.running ) )
      {
         /* Continue DMA ring operation */
         rc = caph_dma_continue_transfer( ch->dma_config.caph_handle, dma_status );

         if( rc )
         {
            ch->errs.dma_err++;
         }

         /* Point to buffer index with actual samples */
         active_ig_idx = (active_ig_idx + 1) & 1;
         ingressp = ch->dma_buf[active_ig_idx].virt;

         /* Apply software digital gain based on channel settings */
         if( ch->dig_gain != 0 )
         {
            audiohSoftDigGain( (int16_t *)ingressp, (ch->frame_size/AUDIOH_SAMP_WIDTH), ch->dig_gain );
         }

         if( ch->equ.len )
         {
            halAudioEquProcess( (int16_t *)ingressp, ch->equ.coeffs, gHalFiltHist,
                  ch->equ.len, (ch->frame_size/AUDIOH_SAMP_WIDTH) );
         }

         if( ch->ramp )
         {
            debug_mode = 1;
            halAudioGenerateRamp( (uint16_t *)ingressp, &ch->rampseed, ch->frame_size/AUDIOH_SAMP_WIDTH, ch->num_fifo_ch );
         }
         else if( ch->sinectl.freq )
         {
            debug_mode = 1;
            halAudioSine( (uint16_t *)ingressp, &ch->sinectl, (ch->frame_size/AUDIOH_SAMP_WIDTH), ch->num_fifo_ch );
         }

         if( ch->csx_data[HALAUDIO_CSX_POINT_ADC].csx_ops.csxCallback )
         {
            debug_mode = 1;
            ch->csx_data[HALAUDIO_CSX_POINT_ADC].csx_ops.csxCallback( (char *)ingressp, ch->frame_size, ch->csx_data[HALAUDIO_CSX_POINT_ADC].priv );
         }

         if ( debug_mode )
         {
            ch->debug = 1;
         }
         else if ( ch->debug )
         {
            /* Clear samples */
            memset( ch->dma_buf[0].virt, 0, ch->frame_size );
            memset( ch->dma_buf[1].virt, 0, ch->frame_size );

            ch->debug = 0;
         }

         /* Add to global ingress operation counter */
         atomic_inc( &gAudiohRefIgrCount );

         /* Check if all ingress operations complete */
         if( atomic_read( &gAudiohRefIgrCount ) >= gNumIngressChannels )
         {
            /* Increment ingress and egress active indices when all ingress channels accounted towards */
            active_eg_idx = (active_eg_idx + 1) & 1;
            atomic_set( &gActive_eg_idx, active_eg_idx );
            atomic_set( &gActive_ig_idx, active_ig_idx );

            if( gAudiohElapsedCb )
            {
               (*gAudiohElapsedCb)( gAudiohUserData );
            }
            atomic_set( &gAudiohRefIgrCount, 0 );
         }
      }
   }
   else
   {
      ch->errs.dma_wrong_dir++;
   }

   AUDIOH_LOG( "end ch=%d", ch->ch_no );
}

/***************************************************************************/
/**
*  AUDIOH egress sync DMA interrupt handler
*
*  @return  None
*/
static void audiohDmaSyncEgressHandler(
   AADMA_Device_t dev,
   int dma_status,
   void *userData
)
{
   struct audioh_ch_cfg *ch;
   ch = userData;

   AUDIOH_LOG( "isr=%u pkts=%i ch=%d", ch->audioh_isrcount, atomic_read( &ch->queued_pkts_egr ), ch->ch_no);
   ch->audioh_isrcount++;

   if( ch->ch_dir == AUDIOH_CH_DIR_EGRESS )
   {
      if( atomic_read( &gAudioh.running ) )
      {
         atomic_dec( &ch->queued_pkts_egr );

         if( atomic_read( &ch->queued_pkts_egr ) > 0 )
         {
            /* More egress packets awaiting to be DMA'd. Start another transfer */
            audiohDmaEgressDoTransfer( ch );
         }
      }
   }
   else
   {
      ch->errs.dma_wrong_dir++;
   }

   AUDIOH_LOG( "end pkts=%i ch=%d", atomic_read( &ch->queued_pkts_egr ), ch->ch_no);
}

/***************************************************************************/
/**
*  AUDIOH egress DMA interrupt handler
*
*  @return  None
*/
static void audiohDmaEgressHandler(
   AADMA_Device_t dev,
   int dma_status,
   void *userData
)
{
   struct audioh_ch_cfg *ch;
   unsigned short       *egressp;
   int debug_mode = 0;
   int rc = 0;

   ch = userData;

   AUDIOH_LOG( "isr=%u ch=%i jiffies=%lu", ch->audioh_isrcount, ch->ch_no, jiffies );
   ch->audioh_isrcount++;

   if( ch->ch_dir == AUDIOH_CH_DIR_EGRESS )
   {
      if( atomic_read( &gAudioh.running ) )
      {
         if ( dma_status & CAPH_READY_LOW )
         {
            /* DMA finished on low buffer, so we set the active index to be 0*/
            atomic_set(&ch->ind_active_idx, 0);
         }
         else
         {
            /* DMA finished on high buffer, so we set the active index to be 1*/
            atomic_set(&ch->ind_active_idx, 1);
         }

         /* Continue DMA ring operation */
         rc = caph_dma_continue_transfer( ch->dma_config.caph_handle, dma_status );

         if( rc )
         {
            ch->errs.dma_err++;
         }

         amxrServiceUnsyncPort( ch->mixer_port );

         /* Point to buffer index with actual samples */
         egressp = ch->dma_buf[atomic_read( &ch->ind_active_idx )].virt;

         /* Service write requests */
         halAudioWriteService( &ch->write, egressp, ch->frame_size );

         if( ch->equ.len )
         {
            halAudioEquProcess( (int16_t *)egressp, ch->equ.coeffs, gHalFiltHist,
                  ch->equ.len, (ch->frame_size/AUDIOH_SAMP_WIDTH) );
         }

         if( ch->ramp )
         {
            debug_mode = 1;
            halAudioGenerateRamp( (uint16_t *)egressp, &ch->rampseed, ch->frame_size/AUDIOH_SAMP_WIDTH, ch->num_fifo_ch );
         }
         else if( ch->sinectl.freq )
         {
            debug_mode = 1;
            halAudioSine( (uint16_t *)egressp, &ch->sinectl, (ch->frame_size/AUDIOH_SAMP_WIDTH), ch->num_fifo_ch );
         }

         if( ch->csx_data[HALAUDIO_CSX_POINT_DAC].csx_ops.csxCallback )
         {
            debug_mode = 1;
            ch->csx_data[HALAUDIO_CSX_POINT_DAC].csx_ops.csxCallback( (char *)egressp, ch->frame_size, ch->csx_data[HALAUDIO_CSX_POINT_DAC].priv );
         }

         if ( debug_mode )
         {
            ch->debug = 1;
         }
         else if ( ch->debug )
         {
            /* Clear samples */
            memset( ch->dma_buf[0].virt, 0, ch->frame_size );
            memset( ch->dma_buf[1].virt, 0, ch->frame_size );

            ch->debug = 0;
         }
      }
   }
   else
   {
      ch->errs.dma_wrong_dir++;
   }

   AUDIOH_LOG( "end idx=%i ch=%d", atomic_read( &ch->ind_active_idx ), ch->ch_no);
}


/***************************************************************************/
/**
*  Helper routine to register ports to the AMXR
*
*  @return  0 if successful, else error code
*
*  @remark
*
*/
static int audiohMixerPortsRegister( void )
{
   struct audioh_ch_cfg *ch;
   int                  i, err;
   AMXR_PORT_CB         cb;
   AMXR_PORT_ID         portid;
   char                 name[32];

   ch = gAudioh.ch;

   for(i = 0; i < AUDIOH_MAX_NUM_CHANS; i++, ch++)
   {

      memset(&cb, 0, sizeof(cb));

      if( ch->ch_dir == AUDIOH_CH_DIR_EGRESS )
      {
         cb.dstdone  = audiohMixerCb_EgressDone;
         cb.dstcnxsremoved = audiohMixerCb_EgressFlush;
         cb.getdst = audiohMixerCb_BufGetEgPtr;
      }
      else
      {
         cb.getsrc = audiohMixerCb_BufGetIgPtr;
      }

      sprintf( name, "halaudio.audioh%i", ch->ch_no );
      err = amxrCreatePort( name, &cb, ch /* privdata */,
            ((ch->ch_dir == AUDIOH_CH_DIR_EGRESS) ? ch->samp_freq : 0),
            ((ch->ch_dir == AUDIOH_CH_DIR_EGRESS) ? ch->num_fifo_ch : 0),
            ((ch->ch_dir == AUDIOH_CH_DIR_EGRESS) ? ch->frame_size : 0),
            ((ch->ch_dir == AUDIOH_CH_DIR_EGRESS) ? 0 : ch->samp_freq),
            ((ch->ch_dir == AUDIOH_CH_DIR_EGRESS) ? 0 : ch->num_fifo_ch),
            ((ch->ch_dir == AUDIOH_CH_DIR_EGRESS) ? 0 : ch->frame_size),
            &portid );

      if( err )
      {
         printk( KERN_ERR "%s: failed to create mixer port %i err=%i\n", __FUNCTION__, i, err );
         return err;
      }

      ch->mixer_port = portid;
   }
   return 0;
}

/***************************************************************************/
/**
*  Helper routine to deregister ports on the AMXR
*
*  @return  0 if successful, else error code
*
*  @remark
*
*/
static int audiohMixerPortsDeregister( void )
{
   struct audioh_ch_cfg  *ch;
   int i, err, rc;

   rc = 0;
   ch = gAudioh.ch;

   for ( i = 0; i < AUDIOH_MAX_NUM_CHANS; i++, ch++ )
   {
      err = amxrRemovePort( ch->mixer_port );
      if( err )
      {
         printk( KERN_ERR "AUDIOH [ch=%d]: failed to deregister mixer port err=%i\n", ch->ch_no, err );
         rc = err;
      }
      ch->mixer_port = NULL;  /* invalidate handle */
   }
   return rc;
}

/***************************************************************************/
/**
*  Helper routine to convert from HALAUDIO_HWSEL to FIFO channel and mux position
*  values
*
*  @return  0 if successful, else error code
*
*  @remark
*
*/
static int audiohHwSelToMux( unsigned int ch_no, unsigned int *fifo_ch, unsigned int *mux_pos, HALAUDIO_HWSEL hwsel )
{
   if( ch_no > AUDIOH_MAX_NUM_CHANS || hwsel > HALAUDIO_HWSEL_MAX_NUM )
   {
      return -1;
   }

   switch( ch_no )
   {
#if AUDIOH_VIN_RIGHT_ENABLE
      case AUDIOH_CHAN_VIN_RIGHT:
         switch( hwsel )
         {
            case HALAUDIO_HWSEL_A:
               *fifo_ch = AUDIOH_VIN_RIGHT_FIFO_CH_PRIM_HEAD_HAND_MIC;
               *mux_pos = AUDIOH_VIN_RIGHT_MUX_POS_PRIMARY_MIC;
               break;
            case HALAUDIO_HWSEL_B:
               *fifo_ch = AUDIOH_VIN_RIGHT_FIFO_CH_PRIM_HEAD_HAND_MIC;
               *mux_pos = AUDIOH_VIN_RIGHT_MUX_POS_HANDSET_MIC;
               break;
            case HALAUDIO_HWSEL_C:
               *fifo_ch = AUDIOH_VIN_RIGHT_FIFO_CH_PRIM_HEAD_HAND_MIC;
               *mux_pos = AUDIOH_VIN_RIGHT_MUX_POS_HEADSET_MIC;
               break;
            default:
               return -1;
         }
         break;
#endif
#if AUDIOH_VIN_LEFT_ENABLE
      case AUDIOH_CHAN_VIN_LEFT:
         switch( hwsel )
         {
            case HALAUDIO_HWSEL_D:
               *fifo_ch = AUDIOH_VIN_LEFT_FIFO_CH_SECONDARY_MIC;
               *mux_pos = AUDIOH_VIN_LEFT_MUX_POS_SECONDARY_MIC;
               break;
            default:
               return -1;
         }
         break;
#endif
#if AUDIOH_HEADSET_ENABLE
      case AUDIOH_CHAN_HEADSET:
         switch( hwsel )
         {
            case HALAUDIO_HWSEL_A:
               *fifo_ch = AUDIOH_HSPATH_LEFT_FIFO_CH_HEADSET_LEFT_SPK;
               *mux_pos = AUDIOH_HSPATH_LEFT_MUX_POS_HEADSET_LEFT_SPK;
               break;
            case HALAUDIO_HWSEL_B:
               *fifo_ch = AUDIOH_HSPATH_RIGHT_FIFO_CH_HEADSET_RIGHT_SPK;
               *mux_pos = AUDIOH_HSPATH_RIGHT_MUX_POS_HEADSET_RIGHT_SPK;
               break;
            default:
               return -1;
         }
         break;
#endif
#if AUDIOH_NVIN_RIGHT_ENABLE
      case AUDIOH_CHAN_NVIN_RIGHT:
         switch( hwsel )
         {
            case HALAUDIO_HWSEL_A:
               *fifo_ch = AUDIOH_NVIN_RIGHT_FIFO_CH_ERR_FBACK_MIC;
               *mux_pos = AUDIOH_NVIN_RIGHT_MUX_POS_ERR_FBACK_MIC;
               break;
            default:
               return -1;
         }
         break;
#endif
#if AUDIOH_NVIN_LEFT_ENABLE
      case AUDIOH_CHAN_NVIN_LEFT:
         switch( hwsel )
         {
            case HALAUDIO_HWSEL_A:
               *fifo_ch = AUDIOH_NVIN_LEFT_FIFO_CH_NOISE_REF_MIC;
               *mux_pos = AUDIOH_NVIN_LEFT_MUX_POS_NOISE_REF_MIC;
               break;
            default:
               return -1;
         }
         break;
#endif
#if AUDIOH_EARPIECE_ENABLE
      case AUDIOH_CHAN_EARPIECE:
         switch( hwsel )
         {
            case HALAUDIO_HWSEL_A:
               *fifo_ch = AUDIOH_EARPATH_MONO_FIFO_CH_EARPIECE_SPK;
               *mux_pos = AUDIOH_EARPATH_MUX_POS_EARPIECE_SPK;
               break;
            default:
               return -1;
         }
         break;
#endif
#if AUDIOH_HANDSFREE_ENABLE
      case AUDIOH_CHAN_HANDSFREE:
         switch( hwsel )
         {
            case HALAUDIO_HWSEL_A:
               *fifo_ch = AUDIOH_IHFPATH_MONO_FIFO_CH_HANDSFREE_SPK;
               *mux_pos = AUDIOH_IHFPATH_MUX_POS_HANDSFREE_SPK;
               break;
            default:
               return -1;
         }
         break;
#endif
#if AUDIOH_VIBRA_ENABLE
      case AUDIOH_CHAN_VIBRA:
         switch( hwsel )
         {
            case HALAUDIO_HWSEL_A:
               *fifo_ch = AUDIOH_VIBRA_MONO_FIFO_CH_MOTOR;
               *mux_pos = AUDIOH_VIBRA_MUX_POS_MOTOR;
               break;
            default:
               return -1;
         }
         break;
#endif
      default:
         return -1;
   }
   return 0;
}

/***************************************************************************/
/**
*  Helper routine to reset all gain blocks to default state in Audio Hub
*  values
*
*  @return  0 if successful, else error code
*
*  @remark
*
*/
static int audiohResetGainBlocks( void )
{
   int i, j, k;
   struct audioh_ch_cfg   *ch;

   ch = gAudioh.ch;
   /* Reset gain for each codec channel */
   for ( i = 0; i < AUDIOH_MAX_NUM_CHANS; i++, ch++ )
   {
#if AUDIOH_SIDETONE_SUPPORT
      /* Mute sidetone */
      ch->sidetone.db = HALAUDIO_GAIN_MUTE;
      audiohSidetoneGainSet( ch->ch_no, ch->sidetone.db );
#endif

      /*Unity digital gain for each channel */
      ch->dig_gain = 0; /* Default unity gain */

      /* Mute analog gains for each fifo channel */
      for( j = 0; j < ch->num_fifo_ch; j++ )
      {
         for( k = 0; k < ch->audio_gain[j].num_mux_pos; k++ )
         {
            if( ch->audio_gain[j].ana_gain[k].ana_gain_sup )
            {
               ch->audio_gain[j].ana_gain[k].ana_gain_value = HALAUDIO_GAIN_MUTE;

               audiohFifoChGainSet( ch->ch_no, j, k, ch->audio_gain[j].ana_gain[k].ana_gain_value );
            }
         }
      }
   }
   return 0;
}

/***************************************************************************/
/**
*  Translate db to Q12 linear gain value. The db value will also be
*  range limited
*
*  @return
*     +ve number  - linear gain value
*/
inline static short dbToLinearQ12(
   int     *db                      /**< (io) Ptr to gain in db */
)
{
   int dbval = *db;

   if( dbval > MAX_Q12_GAIN )
   {
      *db = MAX_Q12_GAIN;
      return 0x7fff;        /* maximum gain */
   }

   if( dbval < MIN_Q12_GAIN )
   {
      *db = HALAUDIO_GAIN_MUTE;
      return 0;             /* mute */
   }

   /* Map db to mapped linear value */
   return q12GainMap[dbval - MIN_Q12_GAIN];
}

/***************************************************************************/
/**
*  Helper routine to perform software digital gain operation on q12 db
*  values
*
*  @return  0 if successful, else error code
*
*  @remark
*
*/
static int audiohSoftDigGain( int16_t *samp, int num_samp, int db )
{
   int32_t reg;
   int16_t result;
   int i;
   short lin_gain;

   lin_gain = dbToLinearQ12( &db );

   for (i = 0; i < num_samp; i++)
   {
      reg = (int32_t)(*samp) * lin_gain;

      /*
       * Round result
       */
      reg += (1 << 11);

      /*
       * Scale and save result
       */

      reg = (reg >> 12);

      result = (int16_t)reg;

      /*
       * Saturate result
       */

      if( reg > (int32_t)AUDIOH_MAX_SINT16 )
      {
         result = AUDIOH_MAX_SINT16;
      }
      else if( reg < (int32_t)AUDIOH_MIN_SINT16 )
      {
         result = AUDIOH_MIN_SINT16;
      }

      *samp = result;
      samp++;
   }
   return 0;
}

/***************************************************************************/
/**
*  Helper routine to perform mux operation within Audio Hub
*  values
*
*  @return  0 if successful, else error code
*
*  @remark
*
*/
static int audiohMuxSelect( int ch_no, int fifo_ch, int mux_pos )
{
   switch( ch_no )
   {
#if AUDIOH_VIN_RIGHT_ENABLE
      case AUDIOH_CHAN_VIN_RIGHT:
         switch( mux_pos )
         {
            case AUDIOH_VIN_RIGHT_MUX_POS_PRIMARY_MIC:
#if AUDIOH_VIN_LEFT_ENABLE
               chal_audio_vinpath_digi_mic_enable( gChalAudioHandle, (CHAL_AUDIO_CHANNEL_LEFT | CHAL_AUDIO_CHANNEL_RIGHT) );
#else
               chal_audio_vinpath_digi_mic_enable( gChalAudioHandle, (CHAL_AUDIO_CHANNEL_RIGHT) );
#endif
               chal_audio_vinpath_select_primary_mic( gChalAudioHandle, 0 /* disable analog mic path */ );
               break;
            case AUDIOH_VIN_RIGHT_MUX_POS_HEADSET_MIC:
#if AUDIOH_VIN_LEFT_ENABLE
               chal_audio_vinpath_digi_mic_enable( gChalAudioHandle, (CHAL_AUDIO_CHANNEL_LEFT) );
#endif
               chal_audio_vinpath_select_primary_mic( gChalAudioHandle, 1 /* switch to analog mics */ );
               chal_audio_headset_mic_select( gChalAudioHandle );
               break;
            case AUDIOH_VIN_RIGHT_MUX_POS_HANDSET_MIC:
#if AUDIOH_VIN_LEFT_ENABLE
               chal_audio_vinpath_digi_mic_enable( gChalAudioHandle, (CHAL_AUDIO_CHANNEL_LEFT) );
#endif
               chal_audio_vinpath_select_primary_mic( gChalAudioHandle, 1 /* switch to analog mics */ );
               chal_audio_handset_mic_select( gChalAudioHandle );
               break;
            default:
               return -1;
         }
         break;
#endif
#if AUDIOH_VIN_LEFT_ENABLE
      case AUDIOH_CHAN_VIN_LEFT:
#endif
#if AUDIOH_NVIN_RIGHT_ENABLE
      case AUDIOH_CHAN_NVIN_RIGHT:
#endif
#if AUDIOH_NVIN_LEFT_ENABLE
      case AUDIOH_CHAN_NVIN_LEFT:
#endif
#if AUDIOH_EARPIECE_ENABLE
      case AUDIOH_CHAN_EARPIECE:
#endif
#if AUDIOH_HEADSET_ENABLE
      case AUDIOH_CHAN_HEADSET:
#endif
#if AUDIOH_HANDSFREE_ENABLE
      case AUDIOH_CHAN_HANDSFREE:
#endif
#if AUDIOH_VIBRA_ENABLE
      case AUDIOH_CHAN_VIBRA:
         /* Nothing to mux */
         break;
#endif
      default:
         return -1;
   }
   return 0;
}

static int audiohChConfigAll( void )
{
   chal_audio_vinpath_set_mono_stereo( gChalAudioHandle, 1 );        /* Mono mode */
#if ( AUDIOH_VIN_RIGHT_ENABLE && AUDIOH_VIN_LEFT_ENABLE && AUDIOH_STEREO_VIN )
   chal_audio_vinpath_set_mono_stereo( gChalAudioHandle, 0 );        /* Stereo mode */
#endif
   chal_audio_vinpath_set_pack( gChalAudioHandle, 0 );               /* Unpacked */
   chal_audio_vinpath_set_bits_per_sample( gChalAudioHandle, 16 );   /* 16-bit samples */

   chal_audio_nvinpath_set_mono_stereo( gChalAudioHandle, 1 );       /* Mono mode */
#if ( AUDIOH_NVIN_RIGHT_ENABLE && AUDIOH_NVIN_LEFT_ENABLE && AUDIOH_STEREO_NVIN )
   chal_audio_nvinpath_set_mono_stereo( gChalAudioHandle, 0 );       /* Stereo mode */
#endif
   chal_audio_nvinpath_set_pack( gChalAudioHandle, 0 );              /* Unpacked */
   chal_audio_nvinpath_set_bits_per_sample( gChalAudioHandle, 16 );  /* 16-bit samples */

   chal_audio_earpath_set_mono_stereo( gChalAudioHandle, 1 );        /* Mono mode */
   chal_audio_earpath_set_pack( gChalAudioHandle, 0 );               /* Unpacked */
   chal_audio_earpath_set_bits_per_sample( gChalAudioHandle, 16 );   /* 16-bit samples */

   chal_audio_hspath_set_mono_stereo( gChalAudioHandle, 0 );         /* Stereo mode */
   chal_audio_hspath_set_pack( gChalAudioHandle, 0 );                /* Unpacked */
   chal_audio_hspath_set_bits_per_sample( gChalAudioHandle, 16 );    /* 16-bit samples */

   chal_audio_ihfpath_set_mono_stereo( gChalAudioHandle, 1 );        /* Mono mode */
   chal_audio_ihfpath_set_pack( gChalAudioHandle, 0 );               /* Unpacked */
   chal_audio_ihfpath_set_bits_per_sample( gChalAudioHandle, 16 );   /* 16-bit samples */

   chal_audio_vibra_set_mono_stereo( gChalAudioHandle, 1 );          /* Mono mode */
   chal_audio_vibra_set_pack( gChalAudioHandle, 0 );                 /* Unpacked */
   chal_audio_vibra_set_bits_per_sample( gChalAudioHandle, 16 );     /* 16-bit samples */

   return 0;
}

#if AUDIOH_EARPIECE_ENABLE
static int audiohEarpathPowerRamp( int enable )
{
   if( enable )
   {
      /* External Pop-Click sequence */
      chal_audio_earpath_set_slowramp_ctrl( gChalAudioHandle, CHAL_AUDIO_AUDIOTX_SR_EXT_POPCLICK);

      /* Isolate Input = 0 */
      chal_audio_earpath_clear_isolation_ctrl( gChalAudioHandle, CHAL_AUDIO_AUDIOTX_ISO_IN);

      /* Wait for 400 msec */
      schedule_timeout_uninterruptible( HZ * AUDIOH_DACPWRUP_SETTLE_TIME / 1000 );

      /* Cause a raising edge on SR_PUP_ED_DRV_TRIG and END_PWRDOWN to 1*/
      chal_audio_earpath_set_slowramp_ctrl( gChalAudioHandle, CHAL_AUDIO_AUDIOTX_SR_PUP_ED_DRV_TRIG );
      chal_audio_earpath_set_slowramp_ctrl( gChalAudioHandle, CHAL_AUDIO_AUDIOTX_SR_END_PWRDOWN );

      /* Wait for 150usec */
      schedule_timeout_uninterruptible( HZ * AUDIOH_SLOWRAMP_PWRDN_PULSE_TIME / 1000000 );

      /* END_PWRDOWN to 0. This will create a 150usec pulse for this signal */
      chal_audio_earpath_clear_slowramp_ctrl( gChalAudioHandle, CHAL_AUDIO_AUDIOTX_SR_END_PWRDOWN );
      chal_audio_earpath_set_slowramp_ctrl( gChalAudioHandle, CHAL_AUDIO_AUDIOTX_SR_EN_RAMP1_45M );
      chal_audio_earpath_set_slowramp_ctrl( gChalAudioHandle, CHAL_AUDIO_AUDIOTX_SR_PU_ENABLE );

      /* Powerup the Ear Piece Driver */
      chal_audio_earpath_set_drv_pwr( gChalAudioHandle, CHAL_AUDIO_ENABLE );

      /* Wait for 400 msec */
      schedule_timeout_uninterruptible( HZ * AUDIOH_SLOWRAMP_RAMP1UP_TIME / 1000 );

      /* End RAMP1_45M and Start RAMP2_5M */
      chal_audio_earpath_clear_slowramp_ctrl( gChalAudioHandle ,CHAL_AUDIO_AUDIOTX_SR_EN_RAMP1_45M );
      chal_audio_earpath_set_slowramp_ctrl( gChalAudioHandle, CHAL_AUDIO_AUDIOTX_SR_EN_RAMP2_5M );

      /* Wait for 5msec */
      schedule_timeout_uninterruptible( HZ * AUDIOH_SLOWRAMP_RAMP2UP_TIME / 1000 );

      /* End RAMP2_5M */
      chal_audio_earpath_clear_slowramp_ctrl( gChalAudioHandle, CHAL_AUDIO_AUDIOTX_SR_EN_RAMP2_5M );
      chal_audio_earpath_clear_slowramp_ctrl( gChalAudioHandle, CHAL_AUDIO_AUDIOTX_SR_PU_ENABLE );
      chal_audio_earpath_set_slowramp_ctrl( gChalAudioHandle, CHAL_AUDIO_AUDIOTX_SR_END_PWRUP );
   }
   else
   {
      chal_audio_earpath_clear_slowramp_ctrl(gChalAudioHandle,CHAL_AUDIO_AUDIOTX_SR_PUP_ED_DRV_TRIG);
      chal_audio_earpath_clear_slowramp_ctrl(gChalAudioHandle,CHAL_AUDIO_AUDIOTX_SR_END_PWRUP);
      chal_audio_earpath_set_slowramp_ctrl(gChalAudioHandle,CHAL_AUDIO_AUDIOTX_SR_EN_RAMP2_5M);
      chal_audio_earpath_set_slowramp_ctrl(gChalAudioHandle,CHAL_AUDIO_AUDIOTX_SR_PD_ENABLE);

      /* Wait for 35msec */
      schedule_timeout_uninterruptible( HZ * AUDIOH_SLOWRAMP_RAMP2DOWN_TIME / 1000 );

      chal_audio_earpath_clear_slowramp_ctrl(gChalAudioHandle,CHAL_AUDIO_AUDIOTX_SR_EN_RAMP2_5M);
      chal_audio_earpath_set_slowramp_ctrl(gChalAudioHandle,CHAL_AUDIO_AUDIOTX_SR_EN_RAMP1_45M);

      /* Wait for 10msec */
      schedule_timeout_uninterruptible( HZ * AUDIOH_SLOWRAMP_RAMP1DOWN_TIME / 1000 );

      chal_audio_earpath_clear_slowramp_ctrl(gChalAudioHandle,CHAL_AUDIO_AUDIOTX_SR_EN_RAMP1_45M);
      chal_audio_earpath_set_slowramp_ctrl(gChalAudioHandle,CHAL_AUDIO_AUDIOTX_SR_END_PWRDOWN);
      chal_audio_earpath_clear_slowramp_ctrl(gChalAudioHandle,CHAL_AUDIO_AUDIOTX_SR_PD_ENABLE);

      /* Wait for 150usec */
      schedule_timeout_uninterruptible( HZ * AUDIOH_SLOWRAMP_PWRDN_PULSE_TIME / 1000000 );

      chal_audio_earpath_clear_slowramp_ctrl(gChalAudioHandle,CHAL_AUDIO_AUDIOTX_SR_END_PWRDOWN);

      /* power down the earpiece Driver  */
      chal_audio_earpath_set_drv_pwr(gChalAudioHandle, CHAL_AUDIO_DISABLE);

      /* Isolate Input = 1 */
      chal_audio_earpath_set_isolation_ctrl(gChalAudioHandle, CHAL_AUDIO_AUDIOTX_ISO_IN);

      /* Wait for 1 msec */
      schedule_timeout_uninterruptible( HZ * AUDIOH_PATHENDIS_SETTLING_TIME / 1000 );
   }

   return 0;
}
#endif

#if AUDIOH_HEADSET_ENABLE
static int audiohHeadsetpathEnable( int enable )
{
   HALAUDIO_AUDIOH_PLATFORM_INFO *info = &gAudiohPlatformInfo;

   if( enable )
   {
      if( info->spk_en_gpio.headset_en >= 0)
      {
         gpio_set_value( info->spk_en_gpio.headset_en, 0 );
      }
   }
   else
   {
      if( info->spk_en_gpio.headset_en >= 0)
      {
         gpio_set_value( info->spk_en_gpio.headset_en, 1 );
      }
   }

   return 0;
}
#endif

#if AUDIOH_HANDSFREE_ENABLE
static int audiohHandsfreepathEnable( int enable )
{
   HALAUDIO_AUDIOH_PLATFORM_INFO *info = &gAudiohPlatformInfo;

   if( enable )
   {
      if( info->spk_en_gpio.handsfree_left_en >= 0)
      {
         gpio_set_value( info->spk_en_gpio.handsfree_left_en, 0 );
      }
      if( info->spk_en_gpio.handsfree_right_en >= 0)
      {
         gpio_set_value( info->spk_en_gpio.handsfree_right_en, 0 );
      }
   }
   else
   {
      if( info->spk_en_gpio.handsfree_left_en >= 0)
      {
         gpio_set_value( info->spk_en_gpio.handsfree_left_en, 1 );
      }
      if( info->spk_en_gpio.handsfree_right_en >= 0)
      {
         gpio_set_value( info->spk_en_gpio.handsfree_right_en, 1 );
      }
   }

   return 0;
}
#endif

static int audiohChPowerTransducer( int powerup )
{
   if ( powerup && !atomic_read(&gPwrTransducerLevel) )
   {
      /* Power up analog microphones */
      chal_audio_enable_aci_auxmic(gChalAudioHandle, 1 );
      chal_audio_mic_pwrctrl(gChalAudioHandle, 1);

#if AUDIOH_EARPIECE_ENABLE
      audiohEarpathPowerRamp( 1 );
#endif

#if AUDIOH_HEADSET_ENABLE
      audiohHeadsetpathEnable( 1 );
#endif

#if AUDIOH_HANDSFREE_ENABLE
      audiohHandsfreepathEnable( 1 );
#endif

      atomic_set( &gPwrTransducerLevel, 1 );
   }
   else if ( !powerup && atomic_read(&gPwrTransducerLevel) )
   {
      /* Power down analog microphones */
      chal_audio_mic_pwrctrl(gChalAudioHandle, 0);
      chal_audio_enable_aci_auxmic(gChalAudioHandle, 0 );

#if AUDIOH_EARPIECE_ENABLE
      audiohEarpathPowerRamp( 0 );
#endif

#if AUDIOH_HEADSET_ENABLE
      audiohHeadsetpathEnable( 0 );
#endif

#if AUDIOH_HANDSFREE_ENABLE
      audiohHandsfreepathEnable( 0 );
#endif
      atomic_set( &gPwrTransducerLevel, 0 );
   }

   return 0;
}

static int audiohChPowerDac( int powerup )
{
   if ( powerup && !atomic_read(&gPwrDacLevel) )
   {

#if AUDIOH_EARPIECE_ENABLE
      chal_audio_earpath_set_dac_pwr( gChalAudioHandle, CHAL_AUDIO_ENABLE );
#endif

#if AUDIOH_HEADSET_ENABLE
      chal_audio_hspath_set_dac_pwr( gChalAudioHandle, (CHAL_AUDIO_CHANNEL_LEFT | CHAL_AUDIO_CHANNEL_RIGHT) );
#endif

#if AUDIOH_HANDSFREE_ENABLE
      chal_audio_ihfpath_set_dac_pwr( gChalAudioHandle, (CHAL_AUDIO_CHANNEL_LEFT | CHAL_AUDIO_CHANNEL_RIGHT) );
#endif

#if AUDIOH_VIBRA_ENABLE
      chal_audio_vibra_set_dac_pwr( gChalAudioHandle, CHAL_AUDIO_ENABLE );
#endif
      atomic_set( &gPwrDacLevel, 1 );
   }
   else if ( !powerup && atomic_read(&gPwrDacLevel) )
   {

#if AUDIOH_EARPIECE_ENABLE
      chal_audio_earpath_set_dac_pwr( gChalAudioHandle, CHAL_AUDIO_DISABLE );
#endif

#if AUDIOH_HEADSET_ENABLE
      chal_audio_hspath_set_dac_pwr( gChalAudioHandle, CHAL_AUDIO_DISABLE );
#endif

#if AUDIOH_HANDSFREE_ENABLE
      chal_audio_ihfpath_set_dac_pwr( gChalAudioHandle, CHAL_AUDIO_DISABLE );
#endif

#if AUDIOH_VIBRA_ENABLE
      chal_audio_vibra_set_dac_pwr( gChalAudioHandle, CHAL_AUDIO_DISABLE );
#endif
      atomic_set( &gPwrDacLevel, 0 );
   }

   return 0;
}

static int audiohFifoChGainSet( unsigned int chno, unsigned int fifo_ch, unsigned int mux_pos, int db )
{
   struct audioh_ch_cfg *ch;
   int i;
   int found = 0;
   int db_reg_val = 0;

   ch = &gAudioh.ch[chno];

   /* Check if channel with hw selection supports analog gain */
   if( !ch->audio_gain[fifo_ch].ana_gain[mux_pos].ana_gain_sup )
   {
      return -EINVAL;
   }

   /* Check if db gain value is supported */
   for( i = 0; i < sizeof(audiohAnaGainMap)/sizeof(audiohAnaGainMap[0]); i++ )
   {
      if ( db == audiohAnaGainMap[i].db_val )
      {
         found = 1;
         break;
      }
   }

   if( !found )
   {
      /* Could not find supported analog gain */
      return -EINVAL;
   }
   else
   {
      /* Grab register value for specified db gain in map */
      db_reg_val = audiohAnaGainMap[i].reg_val;

      ch->audio_gain[fifo_ch].ana_gain[mux_pos].ana_gain_value = db;
      chal_audio_mic_pga_set_gain( gChalAudioHandle, db_reg_val );
   }

   return 0;
}

/***************************************************************************/
/**
*  Initialize Audio Hub
*
*  @return  0 if successful, else error code
*
*  @remark
*
*/
static int audiohInit(
   HALAUDIO_IF_FRAME_ELAPSED  isrcb,   /*<< (i) Callback to call when one frame elapses */
   void                      *data     /*<< (i) User data to pass to callback */
)
{
   int rc;

   /* Initialize state flags */
   atomic_set( &gAudioh.running, 0 );
   atomic_set( &gAudioh.prepared, 0 );

   /* Initialize lock */
   sema_init( &gProcBufLock , 1 );
   sema_init( &gPwrLock, 1 );

   /* Set threshold interrupt default to be off */
   gAudioh.thresh_intr_en = 0;

   /* Set the AudioH ISR count */
   gAudioh.audioh_isr_cnt = 0;

   /* Initialize the Audio Hub */
   if( (rc = audiohIoRemap()) )
   {
      printk(KERN_ERR "AUDIOH: Failed on I/O remapping rc=%i\n", rc);
      return rc;
   }

   rc = aadma_init();
   if( rc )
   {
      printk(KERN_ERR "AUDIOH: Failed to initialize AADMA rc=%i\n", rc );
      return rc;
   }

   rc = ssasw_init();
   if( rc )
   {
      printk(KERN_ERR "AUDIOH: Failed to initialize SSASW rc=%i\n", rc );
      return rc;
   }

   /* Setup and enable DMA on all channels */
   rc = audiohDmaInit();
   if( rc )
   {
      printk(KERN_ERR "AUDIOH: Failed to initialize DMA resources rc=%i\n", rc);
      return rc;
   }

   audiohProcInit();

   /* Setup the mixer */
   rc = audiohMixerPortsRegister();
   if( rc )
   {
      printk(KERN_ERR "AUDIOH: Failed to create mixer ports rc=%i\n", rc);
      return rc;
   }

   rc = aadma_setup();
   if( rc )
   {
      printk( KERN_ERR "AUDIOH: Failed to setup AADMA rc=%i\n", rc );
      return rc;
   }

   /* Setup interrupt */
   gAudiohElapsedCb  = isrcb;
   gAudiohUserData   = data;

   rc = request_irq( BCM_INT_ID_AUDIO, audioh_isr, IRQF_DISABLED, "audioh", gAudioh.ch);
   if( rc )
   {
      printk( KERN_ERR "AUDIOH: failed to initialize AUDIOH ISR rc=%i\n", rc );
      return rc;
   }

   audiohResetGainBlocks();

   gAudioh.initialized = 1;

   return 0;
}

/***************************************************************************/
/**
*  De-initialize Audio Hub
*
*  @return  0 if successful, else error code
*
*  @remark
*
*/
static int audiohExit( void )
{
   int rc;

   rc = audiohMixerPortsDeregister();

   if( rc )
   {
      printk( KERN_ERR "%s: failed to cleanup mixer ports\n", __FUNCTION__ );
   }

   audiohProcTerm();

   audiohDmaTerm();

   chal_audio_deinit(gChalAudioHandle);

   ssasw_exit();

   aadma_exit();

   clk_disable( gAudiohClk.audioh_156m );
   clk_disable( gAudiohClk.audioh_26m );
   clk_disable( gAudiohClk.audioh_2p4m );
   clk_disable( gAudiohClk.audioh_apb );
   clk_put( gAudiohClk.audioh_156m );
   clk_put( gAudiohClk.audioh_26m );
   clk_put( gAudiohClk.audioh_2p4m );
   clk_put( gAudiohClk.audioh_apb );

   free_irq( BCM_INT_ID_AUDIO, gAudioh.ch );

   gAudioh.initialized = 0;

   release_mem_region( AUDIOH_PHYS_BASE_ADDR_START, AUDIOH_REGISTER_LENGTH );
   release_mem_region( SDT_PHYS_BASE_ADDR_START, SDT_REGISTER_LENGTH );
   release_mem_region( ACI_PHYS_BASE_ADDR_START, ACI_REGISTER_LENGTH );

   return 0;
}

/***************************************************************************/
/**
*  Prepare and configure Audio Hub for operation
*
*  @return  0 if successful, else error code
*
*  @remark
*
*/
static int audiohPrepare( void )
{
   int rc = 0;
   int i;
   struct audioh_ch_cfg   *ch;

   if( atomic_read( &gAudioh.running ) || atomic_read( &gAudioh.prepared ) )
   {
      return -EBUSY; /* Already running or prepared */
   }

   /* Reset the AudioH ISR count */
   gAudioh.audioh_isr_cnt = 0;

   /* Reset synchronized ingress channel counter */
   gNumIngressChannels = 0;

   /* Determine number of synchronized ingress channels */
   for( i = 0; i < AUDIOH_MAX_NUM_CHANS; i++ )
   {
      if( gAudioh.ch[i].ch_dir == AUDIOH_CH_DIR_INGRESS )
      {
         if( AUDIOH_SYNC_FREQ( gAudioh.ch[i].samp_freq ) )
         {
            gNumIngressChannels++;
         }
      }
   }

   /* Configure settings for each channel */
   audiohChConfigAll();

   /* Clear all CAPH DMA attributes */
   memset( &gCaphDmaDevAttr, 0, sizeof(gCaphDmaDevAttr) );

   /* Clear the FIFOs */
   chal_audio_vinpath_clr_fifo( gChalAudioHandle );
   chal_audio_nvinpath_clr_fifo( gChalAudioHandle );
   chal_audio_earpath_clr_fifo( gChalAudioHandle );
   chal_audio_hspath_clr_fifo( gChalAudioHandle );
   chal_audio_ihfpath_clr_fifo( gChalAudioHandle );
   chal_audio_vibra_clr_fifo( gChalAudioHandle );
   chal_audio_stpath_clr_fifo( gChalAudioHandle );

   /* Clear priming buffer */
   memset( gAudioh.zero.virt, 0, AUDIOH_MAX_PRIME_SIZE_BYTES );

   /* Setup DMA for each channel */
   ch = gAudioh.ch;
   for ( i = 0; i < AUDIOH_MAX_NUM_CHANS; i++, ch++ )
   {
      /* Index is pre-incremented before use. Set to second buffer to start */
      atomic_set(&gActive_ig_idx, 1);
      atomic_set(&gActive_eg_idx, 1);

      ch->audioh_isrcount      = 0;
      memset( &ch->errs, 0, sizeof(ch->errs) );

      if( ch->ch_dir == AUDIOH_CH_DIR_EGRESS )
      {
         /* Clear egress samples */
         memset( ch->dma_buf[0].virt, 0, ch->frame_size );
         memset( ch->dma_buf[1].virt, 0, ch->frame_size );

         if ( AUDIOH_SYNC_FREQ( ch->samp_freq ) )
         {
            rc = caph_dma_set_device_handler( ch->dma_config.device, audiohDmaSyncEgressHandler, ch /* userData */ );

            if( rc != 0 )
            {
               printk( KERN_ERR "AUDIOH: [CH %u] Setting device handler failed\n", ch->ch_no);
               goto cleanup_dma_channels;
            }

            ch->dma_config.caph_handle = caph_dma_request_channel( ch->dma_config.device, ch->switch_dev, ch->ch_dir, ch->dma_config.fifo_addr );

            if( ch->dma_config.caph_handle == NULL )
            {
               printk( KERN_ERR "AUDIOH: [CH %u] failed to get MTP DMA channel\n", ch->ch_no );
               rc = -EBUSY;
               goto cleanup_dma_channels;
            }

            if ( ch->frame_size != 0 )
            {
               /* Prime egress */
               atomic_set( &ch->queued_pkts_egr, 1 );

               AUDIOH_LOG( "Priming ch %d", ch->ch_no );
               rc = caph_dma_transfer( ch->dma_config.caph_handle, gAudioh.zero.phys, ( ch->prime_egr ) );

               if( rc != 0 )
               {
                  printk( KERN_ERR "AUDIOH: [CH %u] egress prime DMA failed\n", ch->ch_no);
                  goto cleanup_dma_channels;
               }
            }
         }
         else
         {
            /* Clear ingress samples */
            memset( ch->dma_buf[0].virt, 0, ch->frame_size );
            memset( ch->dma_buf[1].virt, 0, ch->frame_size );

            /* Check if channel can accept non sync samples */
            rc = caph_dma_set_device_handler( ch->dma_config.device, audiohDmaEgressHandler, ch /* userData */ );

            if( rc != 0 )
            {
               printk( KERN_ERR "AUDIOH: [CH %u] Setting device handler failed\n", ch->ch_no);
               goto cleanup_dma_channels;
            }

            ch->dma_config.caph_handle = caph_dma_request_channel( ch->dma_config.device, ch->switch_dev, ch->ch_dir, ch->dma_config.fifo_addr );

            if( ch->dma_config.caph_handle == NULL )
            {
               printk( KERN_ERR "AUDIOH: [CH %u] failed to get MTP DMA channel\n", ch->ch_no );
               rc = -EBUSY;
               goto cleanup_dma_channels;
            }

            /* Only transfer if frame size is non-zero */
            if ( ch->frame_size != 0 )
            {
               /* Allocate egress double buffer DMA descriptors */
               rc = caph_dma_alloc_ring_descriptors( ch->dma_config.caph_handle, ch->dma_buf[0].phys, (ch->frame_size *2) );

               if( rc != 0 )
               {
                  printk( KERN_ERR "AUDIOH: [CH %u] ring descriptors are not allocated\n", ch->ch_no );
                  goto cleanup_dma_channels;
               }

               /* Start egress DMA channel. DMA does not actually start until AUDIOH channel is enabled. */
               rc = caph_dma_start_transfer( ch->dma_config.caph_handle );

               if( rc != 0 )
               {
                  printk( KERN_ERR "AUDIOH: [CH %u] Failed to start egress DMA\n", ch->ch_no );
                  goto cleanup_dma_channels;
               }
            }
         }
      }
      else
      {
         caph_dma_set_device_handler( ch->dma_config.device, audiohDmaIngressHandler, ch /* userData */ );

         /* Request ingress DMA channel */
         ch->dma_config.caph_handle = caph_dma_request_channel( ch->dma_config.device, ch->switch_dev, ch->ch_dir, ch->dma_config.fifo_addr );

         if( ch->dma_config.caph_handle == NULL )
         {
            printk( KERN_ERR "AUDIOH: [CH %u] failed to get PTM DMA channel\n", ch->ch_no );
            rc = -EBUSY;
            goto cleanup_dma_channels;
         }

         /* Only transfer if frame size is non-zero */
         if ( ch->frame_size != 0 )
         {
            /* Allocate ingress double buffer DMA descriptors */
            rc = caph_dma_alloc_ring_descriptors( ch->dma_config.caph_handle, ch->dma_buf[0].phys, (ch->frame_size *2) );

            if( rc != 0 )
            {
               printk( KERN_ERR "AUDIOH: [CH %u] ring descriptors are not allocated\n", ch->ch_no );
               goto cleanup_dma_channels;
            }

            /* Start ingress DMA channel. DMA does not actually start until AUDIOH channel is enabled. */
            rc = caph_dma_start_transfer( ch->dma_config.caph_handle );

            if( rc != 0 )
            {
               printk( KERN_ERR "AUDIOH: [CH %u] Failed to start ingress DMA\n", ch->ch_no );
               goto cleanup_dma_channels;
            }
         }
      }
   }

   atomic_set( &gAudiohRefIgrCount, 0 );

   /* Setup DMA for each channel */
   ch = gAudioh.ch;
   for ( i = 0; i < AUDIOH_MAX_NUM_CHANS; i++, ch++ )
   {
      caph_dma_sw_enable( ch->dma_config.caph_handle );

      if ( ch->frame_size != 0 )
      {
         caph_dma_enable( ch->dma_config.caph_handle );
      }
   }

   AUDIOH_LOG( "DMA enabled" );

   /* Forced power up */
   audiohAnaPowerDown(0);

   /* Engage microphone analog gains and muxes after power is enabled */
   ch = gAudioh.ch;
   for ( i = 0; i < AUDIOH_MAX_NUM_CHANS; i++, ch++ )
   {
      if( ch->ch_dir == AUDIOH_CH_DIR_INGRESS )
      {
         audiohDigGainSet( i, ch->dig_gain, HALAUDIO_DIR_ADC );
         audiohAnaGainSet( i, ch->anadb, HALAUDIO_DIR_ADC, ch->anahwsel );
      }
      else
      {
         audiohDigGainSet( i, ch->dig_gain, HALAUDIO_DIR_DAC );
      }
   }

   atomic_set( &gAudioh.prepared, 1 );

   return 0;

cleanup_dma_channels:

   /* Free DMA channels that may have been allocated */
   ch = gAudioh.ch;
   for ( i = 0; i < AUDIOH_MAX_NUM_CHANS; i++, ch++ )
   {
      caph_dma_set_device_handler( ch->dma_config.device, NULL, NULL );

      if( ch->dma_config.caph_handle != NULL )
      {
         caph_dma_free_channel( ch->dma_config.caph_handle );
         ch->dma_config.caph_handle = NULL;
      }
   }

   return rc;
}

/***************************************************************************/
/**
*  Enable Audio Hub operation
*
*  @return  0 if successful, else error code
*
*  @remark
*
*/
static int audiohEnable( void )
{
   int rc = 0;
   int ths_en = gAudioh.thresh_intr_en;
   struct audioh_ch_cfg *ch;
   int i;

   if( !atomic_read( &gAudioh.prepared ))
   {
      return -EPERM;
   }

   if( atomic_read( &gAudioh.running ))
   {
      return -EBUSY; /* Already enabled */
   }

   /* Enable path for each channel */
#if (AUDIOH_VIN_RIGHT_ENABLE && AUDIOH_VIN_LEFT_ENABLE)
   chal_audio_vinpath_digi_mic_enable( gChalAudioHandle, (CHAL_AUDIO_CHANNEL_LEFT | CHAL_AUDIO_CHANNEL_RIGHT) );
#elif AUDIOH_VIN_RIGHT_ENABLE
   chal_audio_vinpath_digi_mic_enable( gChalAudioHandle, (CHAL_AUDIO_CHANNEL_RIGHT) );
#elif AUDIOH_VIN_LEFT_ENABLE
   chal_audio_vinpath_digi_mic_enable( gChalAudioHandle, (CHAL_AUDIO_CHANNEL_LEFT) );
#endif

#if ( AUDIOH_NVIN_RIGHT_ENABLE && AUDIOH_NVIN_LEFT_ENABLE )
   chal_audio_nvinpath_digi_mic_enable( gChalAudioHandle, (CHAL_AUDIO_CHANNEL_LEFT | CHAL_AUDIO_CHANNEL_RIGHT) );
#elif AUDIOH_NVIN_RIGHT_ENABLE
   chal_audio_nvinpath_digi_mic_enable( gChalAudioHandle, CHAL_AUDIO_CHANNEL_RIGHT );
#elif AUDIOH_NVIN_LEFT_ENABLE
   chal_audio_nvinpath_digi_mic_enable( gChalAudioHandle, CHAL_AUDIO_CHANNEL_LEFT );
#endif

#if AUDIOH_HEADSET_ENABLE
   chal_audio_hspath_enable( gChalAudioHandle, (CHAL_AUDIO_CHANNEL_LEFT | CHAL_AUDIO_CHANNEL_RIGHT) );
#endif

#if AUDIOH_HANDSFREE_ENABLE
   chal_audio_ihfpath_enable( gChalAudioHandle, (CHAL_AUDIO_CHANNEL_LEFT | CHAL_AUDIO_CHANNEL_RIGHT) );
#endif

#if AUDIOH_EARPIECE_ENABLE
   chal_audio_earpath_enable( gChalAudioHandle, 1 );
#endif

#if AUDIOH_VIBRA_ENABLE
   chal_audio_vibra_enable( gChalAudioHandle, 1 );
#endif

   ch  = gAudioh.ch;

   for ( i = 0; i < AUDIOH_MAX_NUM_CHANS; i++, ch++ )
   {
      caph_dma_mixer_enable( ch->dma_config.caph_handle );
   }

   /* Enable interrupts for each channel */
#if AUDIOH_VIN_RIGHT_ENABLE
   chal_audio_vinpath_int_enable( gChalAudioHandle, ths_en, 1 );
#endif

#if AUDIOH_VIN_LEFT_ENABLE
   chal_audio_vinpath_left_int_enable( gChalAudioHandle, ths_en, 1 );
#endif

#if AUDIOH_NVIN_RIGHT_ENABLE
   chal_audio_nvinpath_int_enable( gChalAudioHandle, ths_en, 1 );
#endif

#if AUDIOH_NVIN_LEFT_ENABLE
   chal_audio_nvinpath_left_int_enable( gChalAudioHandle, ths_en, 1 );
#endif

#if AUDIOH_HEADSET_ENABLE
   chal_audio_hspath_int_enable( gChalAudioHandle, ths_en, 1 );
#endif

#if AUDIOH_HANDSFREE_ENABLE
   chal_audio_ihfpath_int_enable( gChalAudioHandle, ths_en, 1 );
#endif

#if AUDIOH_EARPIECE_ENABLE
   chal_audio_earpath_int_enable( gChalAudioHandle, ths_en, 1 );
#endif

#if AUDIOH_VIBRA_ENABLE
   chal_audio_vibra_int_enable( gChalAudioHandle, ths_en, 1 );
#endif

   ch  = gAudioh.ch;

   /* Sidetone must be set after enabling the path */
   for ( i = 0; i < AUDIOH_MAX_NUM_CHANS; i++, ch++ )
   {
      audiohSidetoneGainSet( i, ch->sidetone.db );
   }

   if( rc == 0 )
   {
      atomic_set( &gAudioh.running, 1 );
   }

   AUDIOH_LOG( "Audioh paths enabled" );

   return rc;
}

/***************************************************************************/
/**
*  Disable Audio Hub operation
*
*  @return  0 if successful, else error code
*
*  @remark
*
*/
static int audiohDisable( void )
{
   int rc;
   int i, err;
   struct audioh_ch_cfg *ch;
   unsigned long flags;

   AUDIOH_LOG( "Disabling" );

   if( !atomic_read( &gAudioh.prepared ) )
   {
      return 0; /* Nothing to disable, has not been prepared */
   }

   /* Ensure DAC is powered off */
   audiohAnaPowerDown( 1 );

   /* Clear running state */
   atomic_set( &gAudioh.running, 0 );

   /* Disable sidetone */
   chal_audio_stpath_enable( gChalAudioHandle, CHAL_AUDIO_DISABLE );
   chal_audio_hspath_sidetone_in( gChalAudioHandle, CHAL_AUDIO_DISABLE );
   chal_audio_ihfpath_sidetone_in( gChalAudioHandle, CHAL_AUDIO_DISABLE );
   chal_audio_earpath_sidetone_in( gChalAudioHandle, CHAL_AUDIO_DISABLE );

   chal_audio_nvinpath_select_sidetone( gChalAudioHandle, CHAL_AUDIO_DISABLE);
   chal_audio_vinpath_select_sidetone( gChalAudioHandle, CHAL_AUDIO_DISABLE);

   local_irq_save( flags );

   /* Mask AUDIOH interrupts to avoid false detecting errors during disable */
   chal_audio_vinpath_int_enable( gChalAudioHandle, 0, 0 );
   chal_audio_vinpath_left_int_enable( gChalAudioHandle, 0, 0 );
   chal_audio_nvinpath_int_enable( gChalAudioHandle, 0, 0 );
   chal_audio_nvinpath_left_int_enable( gChalAudioHandle, 0, 0 );
   chal_audio_hspath_int_enable( gChalAudioHandle, 0, 0 );
   chal_audio_ihfpath_int_enable( gChalAudioHandle, 0, 0 );
   chal_audio_earpath_int_enable( gChalAudioHandle, 0, 0 );
   chal_audio_stpath_int_enable( gChalAudioHandle, 0, 0 );
   chal_audio_vibra_int_enable( gChalAudioHandle, 0, 0 );

   /* Disable all paths */
   chal_audio_vinpath_digi_mic_enable( gChalAudioHandle, 0 );
   chal_audio_nvinpath_digi_mic_enable( gChalAudioHandle, 0 );
   chal_audio_vinpath_select_primary_mic( gChalAudioHandle, 0 );     /* Disable analog mic path */
   chal_audio_hspath_enable( gChalAudioHandle, 0 );
   chal_audio_ihfpath_enable( gChalAudioHandle, 0 );
   chal_audio_earpath_enable( gChalAudioHandle, 0 );
   chal_audio_vibra_enable( gChalAudioHandle, 0 );

   local_irq_restore( flags );

   /* stop egress DMA, AUDIOH block, and then ingress DMA */
   err = 0;
   ch  = gAudioh.ch;

   for ( i = 0; i < AUDIOH_MAX_NUM_CHANS; i++, ch++ )
   {
      if( ch->ch_dir == AUDIOH_CH_DIR_EGRESS )
      {
         caph_dma_set_device_handler( ch->dma_config.device, NULL, NULL );

         rc = caph_dma_stop_transfer( ch->dma_config.caph_handle );

         if( rc != 0 )
         {
            printk( KERN_ERR "AUDIOH: [CH %u] failed to stop egress DMA\n", ch->ch_no );
            err = rc;
         }
         atomic_set( &ch->queued_pkts_egr, 0 );
      }
   }

   ch = gAudioh.ch;

   for ( i = 0; i < AUDIOH_MAX_NUM_CHANS; i++, ch++ )
   {
      if( ch->ch_dir == AUDIOH_CH_DIR_INGRESS )
      {
         caph_dma_set_device_handler( ch->dma_config.device, NULL, NULL );

         rc = caph_dma_stop_transfer( ch->dma_config.caph_handle );

         if( rc != 0 )
         {
            printk( KERN_ERR "AUDIOH: [CH %u] failed to stop egress DMA\n", ch->ch_no );
            err = rc;
         }
      }
   }

   /* Free DMA channels */
   ch = gAudioh.ch;
   for ( i = 0; i < AUDIOH_MAX_NUM_CHANS; i++, ch++ )
   {
      caph_dma_free_channel( ch->dma_config.caph_handle );
      ch->dma_config.caph_handle = NULL;
   }

   /* Clear the FIFOs */
   chal_audio_vinpath_clr_fifo( gChalAudioHandle );
   chal_audio_nvinpath_clr_fifo( gChalAudioHandle );
   chal_audio_earpath_clr_fifo( gChalAudioHandle );
   chal_audio_hspath_clr_fifo( gChalAudioHandle );
   chal_audio_ihfpath_clr_fifo( gChalAudioHandle );
   chal_audio_vibra_clr_fifo( gChalAudioHandle );
   chal_audio_stpath_clr_fifo( gChalAudioHandle );

   /* Flush write buffers */
   ch = gAudioh.ch;
   for ( i = 0; i < AUDIOH_MAX_NUM_CHANS; i++, ch++ )
   {
      halAudioWriteFlush( &ch->write );
   }

   atomic_set( &gAudioh.prepared, 0 );

   return err;
}

/***************************************************************************/
/**
*  Power down analog hardware for codec channel(s)
*
*  @return
*     0        Success
*     -ve      Error code
*
*  @remark
*     This method can choose to operate on one or all of the channels
*     in the interface at a time. To operate on all of the channels,
*     use the HALAuDIO_IF_ALL_CODECS.
*/
static int audiohAnaPowerDown(
   int powerdn                      /*<< (i) 1 to power down, 0 to power up */
)
{
   down( &gPwrLock );
   if ( powerdn )
   {
      audiohChPowerTransducer( 0 );
      audiohChPowerDac( 0 );
      AUDIOH_LOG( "Power down" );
   }
   else
   {
      audiohChPowerDac( 1 );
      audiohChPowerTransducer( 1 );
      AUDIOH_LOG( "Power up" );
   }
   up( &gPwrLock );

   return 0;
};

/***************************************************************************/
/**
*  Disable clocks to enable lower-power state.  Shutdown is only permitted
*  if the interface is already disabled.
*
*  @return
*     0        Success
*     -ve      Error code
*
*  @remark
*/
static int audiohPmShutdown( void )
{
   AUDIOH_LOG( "+" );
   return 0;
}

/***************************************************************************/
/**
*  Re-enable clocks to resume after shutdown.
*
*  @return
*     0        Success
*     -ve      Error code
*
*  @remark
*/
static int audiohPmResume( void )
{
   AUDIOH_LOG( "+" );
   return 0;
}

static int audiohSetFreq(
   int chno,                        /**< (i) channel number 0-2 */
   int freqHz                       /**< (i) sampling frequency in Hz */
)
{
   struct audioh_ch_cfg *ch;
   SSASW_Device_t switch_dev;
   int i, validhz;
   unsigned int frame_size, frame_period, prime_egr, num_ch;
   int rc;

   if( !gAudioh.initialized )
   {
      return -ENODEV;
   }

   if( atomic_read( &gAudioh.running ) || atomic_read( &gAudioh.prepared ) )
   {
      /* Cannot change sampling frequency if currently running or already prepared */
      printk( KERN_ERR "%s Port currently busy\n", __FUNCTION__ );
      return -EBUSY;
   }

   validhz = 0;
   for ( i = 0; i < AUDIOH_MAX_NUM_SUPPORTED_FREQ; i++ )
   {
      if ( freqHz == gAudiohFreqMap[chno].freq_cfg_array[i].freq )
      {
         num_ch = gAudiohFreqMap[chno].freq_cfg_array[i].num_ch;
         switch_dev = gAudiohFreqMap[chno].freq_cfg_array[i].switch_dev;
         frame_period = gAudiohFreqMap[chno].freq_cfg_array[i].frame_period;
         validhz = 1;
         break;
      }
   }

   if ( !validhz )
   {
      printk( KERN_ERR "%s Invalid port frequency %d\n", __FUNCTION__, freqHz );
      return -EINVAL;
   }

   /* Calculate frame size and period based on selected sampling frequency */
   frame_size = (freqHz * num_ch * frame_period * AUDIOH_SAMP_WIDTH) / 1000000; /* in bytes */
   frame_size &= ~0x3;  /* 4 byte alignment */

   if ( (frame_size * 2) > AUDIOH_MAX_DMA_BUF_SIZE_BYTES )
   {
      printk( KERN_ERR "%s Frame size %d greater than max buffer bytes %d\n", __FUNCTION__, frame_size, AUDIOH_MAX_DMA_BUF_SIZE_BYTES );
      return -EINVAL;
   }

   /* Recalculate the adjusted frame period as it may differ from setting due to alignment */
   if ( num_ch )
   {
      frame_period = (frame_size * (1000000/AUDIOH_SAMP_WIDTH)) / ( freqHz * num_ch );               /* in usec */
   }
   else
   {
      frame_period = 0;
   }

   prime_egr = AUDIOH_CALC_PRIME_SIZE(frame_size);
   prime_egr &= ~0x3;   /* 4 byte alignment */

   ch = &gAudioh.ch[chno];

   if ( ch->ch_dir == AUDIOH_CH_DIR_EGRESS )
   {
      rc = amxrSetPortFreq( ch->mixer_port, freqHz, frame_size, 0, 0 );
   }
   else
   {
      rc = amxrSetPortFreq( ch->mixer_port, 0, 0, freqHz, frame_size );
   }

   if ( rc )
   {
      printk( KERN_ERR "%s Unable to set port frequency to %d\n", __FUNCTION__, freqHz );
      return rc;
   }

   if ( ch->ch_dir == AUDIOH_CH_DIR_EGRESS )
   {
      rc = amxrSetPortDstChannels( ch->mixer_port, num_ch, frame_size );
   }
   else
   {
      rc = amxrSetPortSrcChannels( ch->mixer_port, num_ch, frame_size );
   }

   if ( rc )
   {
      /* Revert back the frequency settings */
      if ( ch->ch_dir == AUDIOH_CH_DIR_EGRESS )
      {
         rc = amxrSetPortFreq( ch->mixer_port, ch->samp_freq, ch->frame_size, 0, 0 );
      }
      else
      {
         rc = amxrSetPortFreq( ch->mixer_port, 0, 0, ch->samp_freq, ch->frame_size );
      }
      printk( KERN_ERR "%s Unable to set channels to %d\n", __FUNCTION__, num_ch );
      return rc;
   }

   /* Update channel configuration settings */
   ch->samp_freq = freqHz;
   ch->frame_size = frame_size;
   ch->frame_period = frame_period;
   ch->prime_egr = prime_egr;
   ch->num_fifo_ch = num_ch;
   ch->switch_dev = switch_dev;

   halAudioSineConfig( &ch->sinectl, ch->sinectl.freq, ch->samp_freq );

   ch->dma_buf[1].phys = ch->dma_buf[0].phys + (ch->frame_size );
   ch->dma_buf[1].virt = ch->dma_buf[0].virt + (ch->frame_size );

   AUDIOH_LOG( "samp_freq=%i frame_size=%i frame_period=%i prime_egr=%i num_fifo_ch=%i",
         ch->samp_freq, ch->frame_size, ch->frame_period, ch->prime_egr, ch->num_fifo_ch );

   return 0;
}

static int audiohAnaGainSet( int chno, int db, HALAUDIO_DIR dir, HALAUDIO_HWSEL hwsel )
{
   struct audioh_ch_cfg *ch;
   unsigned int fifo_ch = 0;
   unsigned int mux_pos = 0;
   int rc = 0;

   if( chno >= AUDIOH_MAX_NUM_CHANS )
   {
      return -EINVAL;
   }

   ch = &gAudioh.ch[chno];

   if( ch->ch_dir == AUDIOH_CH_DIR_EGRESS && dir != HALAUDIO_DIR_DAC)
   {
      return -EINVAL;
   }

   if( ch->ch_dir == AUDIOH_CH_DIR_INGRESS && dir != HALAUDIO_DIR_ADC )
   {
      return -EINVAL;
   }

   if( audiohHwSelToMux( chno, &fifo_ch, &mux_pos, hwsel ) )
   {
      return -EINVAL;
   }

   /* Only set hardware parameters when interface has been prepared/powered */
   if ( atomic_read( &gAudioh.prepared ))
   {
      /* Do not error return as we need to provide a method to mux switch.  Change in future for API to mux switch */
      rc = audiohFifoChGainSet( chno, fifo_ch, mux_pos, db );

      /* Set the possible mux selection here */
      rc = audiohMuxSelect( chno, fifo_ch, mux_pos );
   }
   ch->anahwsel = hwsel;   /* Save mux position */
   ch->anadb = db;

   return rc;
}

static int audiohAnaGainGet( int chno, HALAUDIO_GAIN_INFO *info, HALAUDIO_DIR dir, HALAUDIO_HWSEL hwsel )
{
   struct audioh_ch_cfg *ch;
   unsigned int fifo_ch = 0;
   unsigned int mux_pos = 0;
   int current_gain_reg_val = 0;
   int current_gain_db_val = 0;
   int i;

   if( chno >= AUDIOH_MAX_NUM_CHANS )
   {
      return -EINVAL;
   }

   ch = &gAudioh.ch[chno];

   if( ch->ch_dir == AUDIOH_CH_DIR_EGRESS && dir != HALAUDIO_DIR_DAC )
   {
      return -EINVAL;
   }

   if( !ch->ch_dir == AUDIOH_CH_DIR_EGRESS && dir != HALAUDIO_DIR_ADC )
   {
      return -EINVAL;
   }

   if( audiohHwSelToMux( chno, &fifo_ch, &mux_pos, hwsel ) )
   {
      return -EINVAL;
   }

   /* Check if channel with hw selection supports analog gain */
   if( !ch->audio_gain[fifo_ch].ana_gain[mux_pos].ana_gain_sup )
   {
      /* Return 0db as the only gain supported to use with HW selection */
      memset( info, 0, sizeof(*info) );
      info->range_type = HALAUDIO_RANGETYPE_LIST;
      info->range.list.num = 1;
      info->range.list.db[0] = 0;
      return 0;
   }

   chal_audio_mic_pga_get_gain( gChalAudioHandle, &current_gain_reg_val );
   for( i = 0; i < sizeof(audiohAnaGainMap)/sizeof(audiohAnaGainMap[0]); i++ )
   {
      if ( current_gain_reg_val == audiohAnaGainMap[i].reg_val )
      {
         current_gain_db_val = audiohAnaGainMap[i].db_val;
         break;
      }
   }

   info->currentdb = current_gain_db_val;
   i = sizeof(audiohAnaGainMap)/sizeof(audiohAnaGainMap[0]) - 1;
   info->maxdb = audiohAnaGainMap[i].db_val;
   info->mindb = 0;
   info->range_type = HALAUDIO_RANGETYPE_LIST;
   info->range.list.num = sizeof(audiohAnaGainMap)/sizeof(audiohAnaGainMap[0]);
   for( i = 0; i < sizeof(audiohAnaGainMap)/sizeof(audiohAnaGainMap[0]); i++ )
   {
      info->range.list.db[i] = audiohAnaGainMap[i].db_val;
   }
   return 0;
}

static int audiohGetFreq(
   int chno,                        /**< (i) channel number 0-2 */
   int *freqHz                      /**< (i) sampling frequency in Hz */
)
{
   struct audioh_ch_cfg *ch;

   ch       = &gAudioh.ch[chno];
   *freqHz  = ch->samp_freq;

   return 0;
}

static int audiohEquParmSet(
   int                  chno, /*<< (i) Codec channel number */
   HALAUDIO_DIR         dir,  /*<< (i) Direction path */
   const HALAUDIO_EQU   *equ  /*<< (i) Ptr to equalizer parameters */
)
{
   HALAUDIO_EQU         *saved_equ;
   struct audioh_ch_cfg   *ch;

   ch = &gAudioh.ch[chno];

   if( ch->ch_dir == AUDIOH_CH_DIR_INGRESS && dir != HALAUDIO_DIR_ADC )
   {
      return -EINVAL;
   }

   if( ch->ch_dir == AUDIOH_CH_DIR_EGRESS && dir != HALAUDIO_DIR_DAC )
   {
      return -EINVAL;
   }

   saved_equ = &ch->equ;

   memcpy( saved_equ, equ, sizeof(*saved_equ) );

   return 0;
}

static int audiohEquParmGet(
   int                  chno, /*<< (i) Codec channel number */
   HALAUDIO_DIR         dir,  /*<< (i) Direction path */
   HALAUDIO_EQU         *equ  /*<< (i) Ptr to equalizer parameters */
)
{
   HALAUDIO_EQU         *saved_equ;
   struct audioh_ch_cfg   *ch;

   ch = &gAudioh.ch[chno];

   if( ch->ch_dir == AUDIOH_CH_DIR_INGRESS && dir != HALAUDIO_DIR_ADC )
   {
      return -EINVAL;
   }

   if( ch->ch_dir == AUDIOH_CH_DIR_EGRESS && dir != HALAUDIO_DIR_DAC )
   {
      return -EINVAL;
   }

   saved_equ = &ch->equ;

   memcpy( equ, saved_equ, sizeof(*equ) );
   return 0;
}

static int audiohDigGainSet( int chno, int db, HALAUDIO_DIR dir )
{
   struct audioh_ch_cfg *ch;

   if( chno >= AUDIOH_MAX_NUM_CHANS )
   {
      return -EINVAL;
   }

   ch = &gAudioh.ch[chno];

   if( ch->ch_dir == AUDIOH_CH_DIR_INGRESS && dir != HALAUDIO_DIR_ADC )
   {
      return -EINVAL;
   }

   if( ch->ch_dir == AUDIOH_CH_DIR_EGRESS && dir != HALAUDIO_DIR_DAC )
   {
      return -EINVAL;
   }

   if( db > MAX_Q12_GAIN )
   {
      return -EINVAL;
   }

   if( db < MIN_Q12_GAIN )
   {
      db = HALAUDIO_GAIN_MUTE;
   }

   if ( !AUDIOH_SYNC_FREQ( ch->samp_freq ) )
   {
      /* Call to switch to set SRC mixer gain settings */
      ssasw_mixer_gain_set( ch->switch_dev, db );
   }

   ch->dig_gain = db;

   return 0;
}

static int  audiohDigGainGet( int chno, HALAUDIO_GAIN_INFO *info, HALAUDIO_DIR dir )
{
   struct audioh_ch_cfg *ch;

   if( chno >= AUDIOH_MAX_NUM_CHANS )
   {
      return -EINVAL;
   }

   ch = &gAudioh.ch[chno];

   if( ch->ch_dir == AUDIOH_CH_DIR_INGRESS && dir != HALAUDIO_DIR_ADC )
   {
      return -EINVAL;
   }

   if( ch->ch_dir == AUDIOH_CH_DIR_EGRESS && dir != HALAUDIO_DIR_DAC )
   {
      return -EINVAL;
   }

   info->currentdb = ch->dig_gain;
   info->maxdb = MAX_Q12_GAIN;
   info->mindb = MIN_Q12_GAIN;
   info->range_type = HALAUDIO_RANGETYPE_FIXED_STEPSIZE;
   info->range.fixed_step = 1;
   return 0;
}

static int audiohSidetoneGainSet( int chno, int db )
{
#if AUDIOH_SIDETONE_SUPPORT
   struct audioh_ch_cfg *ch;
   short lin_gain;

   ch = &gAudioh.ch[chno];

   /* Only allow sidetone on egress channels */
   if( !ch->ch_dir == AUDIOH_CH_DIR_EGRESS )
   {
      return -EINVAL;
   }

   lin_gain = dbToLinearQ12( &db );

   /* Update filter coefficients, use the same control taps for both upper and lower */
   chal_audio_stpath_load_filter( gChalAudioHandle, (uint32_t *)sidetoneFirCoeff, 0/*unused*/ );
   chal_audio_stpath_set_filter_taps( gChalAudioHandle, AUDIOH_DEF_SIDETONE_CTRL_TAPS, AUDIOH_DEF_SIDETONE_CTRL_TAPS );

   /* Enable sidetone on specific channel path here */
   switch(ch->ch_no)
   {
#if AUDIOH_EARPIECE_ENABLE
      case AUDIOH_CHAN_EARPIECE:
         /* TODO: Default to vin but must allow for selection of vin or nvin */
         chal_audio_vinpath_select_sidetone( gChalAudioHandle, (lin_gain ? CHAL_AUDIO_ENABLE : CHAL_AUDIO_DISABLE) );
         chal_audio_nvinpath_select_sidetone( gChalAudioHandle, CHAL_AUDIO_DISABLE);
         chal_audio_earpath_sidetone_in( gChalAudioHandle, (lin_gain ? CHAL_AUDIO_ENABLE : CHAL_AUDIO_DISABLE) );
         break;
#endif
#if AUDIOH_HEADSET_ENABLE
      case AUDIOH_CHAN_HEADSET:
         /* TODO: Default to vin but must allow for selection of vin or nvin */
         chal_audio_vinpath_select_sidetone( gChalAudioHandle, (lin_gain ? CHAL_AUDIO_ENABLE : CHAL_AUDIO_DISABLE) );
         chal_audio_nvinpath_select_sidetone( gChalAudioHandle, CHAL_AUDIO_DISABLE);
         chal_audio_hspath_sidetone_in( gChalAudioHandle, (lin_gain ? CHAL_AUDIO_ENABLE : CHAL_AUDIO_DISABLE) );
         break;
#endif
#if AUDIOH_HANDSFREE_ENABLE
      case AUDIOH_CHAN_HANDSFREE:
         /* TODO: Default to vin but must allow for selection of vin or nvin */
         chal_audio_vinpath_select_sidetone( gChalAudioHandle, (lin_gain ? CHAL_AUDIO_ENABLE : CHAL_AUDIO_DISABLE) );
         chal_audio_nvinpath_select_sidetone( gChalAudioHandle, CHAL_AUDIO_DISABLE);
         chal_audio_ihfpath_sidetone_in( gChalAudioHandle, (lin_gain ? CHAL_AUDIO_ENABLE : CHAL_AUDIO_DISABLE) );
         break;
#endif
      default:
         return -EINVAL;
   }

   /* Enable soft slope, set linear gain (linear gain step/0x100 will be the target gain step incremental) */
   chal_audio_stpath_set_sofslope( gChalAudioHandle, 0, CHAL_AUDIO_ENABLE, CHAL_AUDIO_ENABLE );

   chal_audio_stpath_set_gain( gChalAudioHandle, lin_gain );

   /* disable clipping, enable filtering, do not bypass gain control */
   chal_audio_stpath_config_misc( gChalAudioHandle, CHAL_AUDIO_DISABLE, CHAL_AUDIO_DISABLE, CHAL_AUDIO_DISABLE );

   /* Enable sidetone path */
   chal_audio_stpath_enable( gChalAudioHandle, (lin_gain ? CHAL_AUDIO_ENABLE : CHAL_AUDIO_DISABLE) );

   chal_audio_stpath_int_enable( gChalAudioHandle, 0, (lin_gain ? CHAL_AUDIO_ENABLE : CHAL_AUDIO_DISABLE) );

   ch->sidetone.db = db;

   return 0;

#else
   return -EPERM;
#endif
}

static int  audiohSidetoneGainGet( int chno, HALAUDIO_GAIN_INFO *info )
{
#if AUDIOH_SIDETONE_SUPPORT
   struct audioh_ch_cfg *ch;

   ch = &gAudioh.ch[chno];

   /* Should get sidetone value from chal layer but no API to support that yet */
   info->currentdb         = ch->sidetone.db;
   info->mindb             = MIN_Q12_GAIN;
   info->maxdb             = 0;
   info->range_type        = HALAUDIO_RANGETYPE_FIXED_STEPSIZE;
   info->range.fixed_step  = 1;  /* 1dB increments */

   return 0;
#else
   return -EPERM;
#endif
}

static int audiohWrite(
   int                     chno,    /**< (i) Codec channel number */
   int                     bytes,   /**< (i) Number of bytes to read */
   const char             *audiobuf,/**< (o) Ptr to buffer to store samples */
   HALAUDIO_CODEC_IORW_CB  cb,      /**< (i) Callback to call when more samples are avail */
   void                   *data     /**< (i) User data to pass to callback */
)
{
   struct audioh_ch_cfg *ch;

   ch = &gAudioh.ch[chno];
   if( !ch->ch_dir == AUDIOH_CH_DIR_EGRESS )
   {
      return -EPERM;
   }

   return halAudioWriteRequest( &ch->write, bytes, audiobuf, cb, data );
}

static int audiohCodecInfo(
   int                  chno,       /**< (i) Codec channel number */
   HALAUDIO_CODEC_INFO *codec_info  /**< (o) Ptr to codec info structure */
)
{
   struct audioh_ch_cfg *ch;
   int i;

   if( chno >= AUDIOH_MAX_NUM_CHANS )
   {
      return -EINVAL;
   }

   ch = &gAudioh.ch[chno];

   memset( codec_info, 0, sizeof(*codec_info) );

   codec_info->freqs.num      = 1;
   codec_info->freqs.freq[0]  = ch->samp_freq;
   codec_info->sample_width   = AUDIOH_SAMP_WIDTH;
   codec_info->bulk_delay     = -1;
   codec_info->read_format    = HALAUDIO_FMT_S16_LE;
   codec_info->write_format   = HALAUDIO_FMT_S16_LE;

   if( ch->ch_dir == AUDIOH_CH_DIR_EGRESS )
   {
      codec_info->channels_rx = ch->num_fifo_ch;
      for( i = 0; i < ch->num_fifo_ch; i++ )
      {
         codec_info->spkrs += ch->audio_gain[i].num_mux_pos;
      }
   }
   else
   {
      codec_info->channels_tx = ch->num_fifo_ch;
      for( i = 0; i < ch->num_fifo_ch; i++ )
      {
         codec_info->mics += ch->audio_gain[i].num_mux_pos;
      }
   }

   sprintf( codec_info->name, "AUDIOH CH%i", ch->ch_no );

   return 0;
}

static int audiohCsxSet(
   int                        chno, /**< (i) Codec channel number */
   HALAUDIO_CSX_POINT_ID      point,   /**< (i) Point ID to install the CSX point */
   const CSX_IO_POINT_FNCS   *fncp, /**< (i) Ptr to CSX callbacks */
   void                      *data  /**< (i) User data to pass back to callbacks */
)
{
   struct audioh_ch_cfg *ch;
   unsigned long      flags;
   int err = 0;

   if( chno >= AUDIOH_MAX_NUM_CHANS )
   {
      return -EINVAL;
   }

   ch = &gAudioh.ch[chno];

   local_irq_save( flags );
   switch( point )
   {
      case HALAUDIO_CSX_POINT_ADC:
      {
         if( !ch->ch_dir == AUDIOH_CH_DIR_EGRESS )
         {
            memcpy( &ch->csx_data[HALAUDIO_CSX_POINT_ADC].csx_ops,
                  fncp,
                  sizeof(ch->csx_data[HALAUDIO_CSX_POINT_ADC].csx_ops) );

            ch->csx_data[HALAUDIO_CSX_POINT_ADC].priv = data;
         }
         else
         {
            err = -EINVAL;
         }
         break;
      }
      case HALAUDIO_CSX_POINT_DAC:
      {
         if( ch->ch_dir == AUDIOH_CH_DIR_EGRESS )
         {
            memcpy( &ch->csx_data[HALAUDIO_CSX_POINT_DAC].csx_ops,
                  fncp,
                  sizeof(ch->csx_data[HALAUDIO_CSX_POINT_DAC].csx_ops) );

            ch->csx_data[HALAUDIO_CSX_POINT_DAC].priv = data;
         }
         else
         {
            err = -EINVAL;
         }
         break;
      }
      default:
      {
         err = -EINVAL;
      }
   }
   local_irq_restore( flags );

   return err;
}

static void audiohProcInit( void )
{
   int i;
   char procName[128];
   gProcDir = proc_mkdir( AUDIOH_PROCDIR_NAME, NULL );

   for( i = 0; i < AUDIOH_MAX_NUM_CHANS; i++ )
   {
      sprintf(procName, "audioh%d", i );
      create_proc_read_entry( procName, 0, gProcDir, audiohReadProc, &gAudioh.ch[i] );
   }

   gSysCtlHeader = register_sysctl_table( gSysCtl );
}

static void audiohProcTerm( void )
{
   int i;
   char procName[128];

   for( i = 0; i < AUDIOH_MAX_NUM_CHANS; i++ )
   {
      sprintf(procName, "audioh%d", i );
      remove_proc_entry( procName, gProcDir );
   }

   remove_proc_entry( AUDIOH_PROCDIR_NAME, NULL );

   if ( gSysCtlHeader )
   {
      unregister_sysctl_table( gSysCtlHeader );
   }
}

static int audiohReadProc( char *buf, char **start, off_t offset, int count, int *eof, void *data )
{
   int len = 0;
   struct audioh_ch_cfg *ch;
   unsigned long flags;
   int active_idx;
   unsigned int frame_size;
   int buffer_cut = 0;
   (void) start; (void) offset; (void) count;      /* avoid compiler warning */

   ch = data;

   if( ch->ch_dir == AUDIOH_CH_DIR_INGRESS )
   {
      active_idx = atomic_read(&gActive_ig_idx);
   }
   else
   {
      active_idx = atomic_read(&gActive_eg_idx);
   }

   len += sprintf( buf+len, "      AUDIOH CH%i @ %i Hz, %i bytes, %i us\n",
         ch->ch_no, ch->samp_freq, ch->frame_size, ch->frame_period );

   down( &gProcBufLock );

   memset( gProcbuf_active, 0, sizeof(gProcbuf_active) );

   /* Cache data in critical section */
   local_irq_save( flags );
   memcpy( gProcbuf_active, ch->dma_buf[active_idx].virt, ch->frame_size );
   local_irq_restore( flags );

   if ( ch->frame_size > 1000 )
   {
      frame_size = 1000;
      buffer_cut = 1;
   }
   else
   {
      frame_size = ch->frame_size;
   }

   if( ch->ch_dir == AUDIOH_CH_DIR_INGRESS )
   {
      len += sprintf( buf+len, "Active Igr:\n" );
      halAudioPrintMemory( buf, &len, gProcbuf_active, frame_size/sizeof(short), 1 /* in hex */, 10 /* line length */,
            2 /* word width */, 0 /* print_addr */, 0 /* addr */ );
   }
   else
   {
      len += sprintf( buf+len, "Active Egr:\n" );
      halAudioPrintMemory( buf, &len, gProcbuf_active, frame_size/sizeof(short), 1 /* in hex */, 10 /* line length */,
         2 /* word width */, 0 /* print_addr */, 0 /* addr */ );
   }

   if ( buffer_cut )
   {
      len += sprintf( buf+len, "Unable to display entire buffer due to large size.  Displaying %d samples\n", frame_size );
   }

   up( &gProcBufLock );

   /* Error report and other information */
   if ( ch->ch_dir == AUDIOH_CH_DIR_INGRESS )
   {
      len += sprintf( buf+len, "DMA Irqs:       ingress=%u\n", ch->audioh_isrcount );
      len += sprintf( buf+len, "DMA errors:     ingress=%i\n", ch->errs.dma_err );
      len += sprintf( buf+len, "FIFO Irqs:      ingress=%i\n", gAudioh.audioh_isr_cnt );
      len += sprintf( buf+len, "FIFO errors:    ingress=%i\n", ch->errs.dma_fifo_err );
      len += sprintf( buf+len, "FIFO threshold: ingress=%i\n", ch->errs.dma_fifo_thrs_err );
      len += sprintf( buf+len, "FIFO underflow: ingress=%i\n", ch->errs.fifo_underflow );
      len += sprintf( buf+len, "FIFO overflow:  ingress=%i\n", ch->errs.fifo_overflow );
   }
   else
   {
      len += sprintf( buf+len, "Irqs:           egress=%u\n", ch->audioh_isrcount );
      len += sprintf( buf+len, "DMA errors:     egress=%i\n", ch->errs.dma_err );
      len += sprintf( buf+len, "FIFO Irqs:      egress=%i\n", gAudioh.audioh_isr_cnt );
      len += sprintf( buf+len, "FIFO errors:    egress=%i\n", ch->errs.dma_fifo_err );
      len += sprintf( buf+len, "FIFO threshold: egress=%i\n", ch->errs.dma_fifo_thrs_err );
      len += sprintf( buf+len, "FIFO underflow: egress=%i\n", ch->errs.fifo_underflow );
      len += sprintf( buf+len, "FIFO overflow:  egress=%i\n", ch->errs.fifo_overflow );
   }

   /* Channel parameters */
   if ( ch->ch_dir == AUDIOH_CH_DIR_EGRESS )
   {
      len += sprintf( buf+len, "Priming:     samples=%i\n", ch->prime_egr/sizeof(short) );
   }

   if ( ch->ch_dir == AUDIOH_CH_DIR_INGRESS )
   {
      len += sprintf( buf+len, "Ingress DMA: device=0x%x handle=0x%x\n",
            ch->dma_config.device, (unsigned int)ch->dma_config.caph_handle );
   }
   else
   {
      len += sprintf( buf+len, "Egress DMA:  device=0x%x handle=0x%x\n",
            ch->dma_config.device, (unsigned int)ch->dma_config.caph_handle );
   }

   if ( ch->ch_dir == AUDIOH_CH_DIR_INGRESS )
   {
      len += sprintf( buf+len, "Buffer Igr:  [0] 0x%.8lx (0x%.8x phy)\n"
            "             [1] 0x%.8lx (0x%.8x phy)\n",
            (unsigned long)ch->dma_buf[0].virt, ch->dma_buf[0].phys, (unsigned long)ch->dma_buf[1].virt, ch->dma_buf[1].phys );
   }
   else
   {
      len += sprintf( buf+len, "Buffer Egr:  [0] 0x%.8lx (0x%.8x phy)\n"
                               "             [1] 0x%.8lx (0x%.8x phy)\n",
            (unsigned long)ch->dma_buf[0].virt, ch->dma_buf[0].phys, (unsigned long)ch->dma_buf[1].virt, ch->dma_buf[1].phys );

      len += sprintf( buf+len, "Prime Egr:   [0] 0x%.8lx (0x%.8x phy)\n",
            (unsigned long)gAudioh.zero.virt, gAudioh.zero.phys );
   }
   len += sprintf( buf+len, "Debug:       running=%i prepared=%i\n",
         atomic_read( &gAudioh.running ), atomic_read( &gAudioh.prepared ) );

   *eof = 1;
   return len+1;
}

/***************************************************************************/
/**
*  Sysctl callback to handle sine generation test
*
*  @return  0 success, otherwise failure
*/
static int proc_doSineGen( ctl_table *table, int write,
      void __user *buffer, size_t *lenp, loff_t *ppos )
{
   struct audioh_ch_cfg *ch;
   int                        rc;

   /* Process integer operation */
   rc = proc_dointvec( table, write, buffer, lenp, ppos );

   if ( write )
   {
      ch = table->extra1;
      halAudioSineConfig( &ch->sinectl, ch->sinectl.freq, ch->samp_freq );
   }
   return rc;
}

static int caph_dma_set_device_handler( AADMA_Device_t dev, AADMA_DeviceHandler_t dma_handler, void *data )
{
   int rc;

   if( dev < 0 || dev > AADMA_NUM_DEVICE_ENTRIES )
   {
      return -ENODEV;
   }

   rc = aadma_set_device_handler( dev, dma_handler, data );

   return rc;
}

static CAPH_DMA_Handle_t caph_dma_request_channel( AADMA_Device_t aadma_dev, SSASW_Device_t switch_dev, enum audioh_ch_dir ch_dir, uint16_t audioh_fifo_addr )
{
   struct caph_dma_device_attr   *devAttr;
   uint32_t cfifo_addr, src_addr, dst_addr;

   if( aadma_dev < 0 || aadma_dev > AADMA_NUM_DEVICE_ENTRIES )
   {
      printk( KERN_ERR "%s AADMA Device ID invalid\n", __FUNCTION__ );
      return NULL;
   }

   if( switch_dev < 0 || switch_dev > SSASW_NUM_DEVICE_ENTRIES )
   {
      printk( KERN_ERR "%s SSASW Device ID invalid\n", __FUNCTION__ );
      return NULL;
   }

   /* We use the AADMA device as the CAPH DMA channel index */
   devAttr = &gCaphDmaDevAttr[aadma_dev];

   if( devAttr->allocated )
   {
      printk( KERN_ERR "%s Channel already allocated\n", __FUNCTION__ );
      return NULL;
   }

   devAttr->aadma_handle = aadma_request_channel( aadma_dev );
   if( devAttr->aadma_handle == AADMA_INVALID_HANDLE )
   {
      printk( KERN_ERR "%s Allocation of AADMA channel failed\n", __FUNCTION__ );
      return NULL;
   }

   cfifo_addr = aadma_get_cfifo_address( devAttr->aadma_handle );
   if( !cfifo_addr )
   {
      printk( KERN_ERR "%s Invalid CFIFO address\n", __FUNCTION__ );
      return NULL;
   }

   /* Setup source and destination address */
   if( ch_dir == AUDIOH_CH_DIR_INGRESS  )
   {
      src_addr = audioh_fifo_addr;
      dst_addr = cfifo_addr;
   }
   else
   {
      src_addr = cfifo_addr;
      dst_addr = audioh_fifo_addr;
   }

   devAttr->ssasw_handle = ssasw_request_channel( switch_dev, (uint16_t)src_addr, (uint16_t)dst_addr );
   if( devAttr->ssasw_handle == SSASW_INVALID_HANDLE )
   {
      aadma_free_channel( devAttr->aadma_handle );
      printk( KERN_ERR "%s Allocation of SSASW channel failed\n", __FUNCTION__ );
      return NULL;
   }

   devAttr->allocated = 1;

   return (CAPH_DMA_Handle_t)(devAttr);
}

static int caph_dma_transfer( CAPH_DMA_Handle_t handle, dma_addr_t data_addr, size_t numBytes )
{
   struct caph_dma_device_attr *devAttr;
   int rc;

   devAttr = (struct caph_dma_device_attr*)handle;

   if(( rc = aadma_alloc_descriptors( devAttr->aadma_handle, data_addr, numBytes )) != 0 )
   {
      return rc;
   }

   return aadma_start_transfer( devAttr->aadma_handle );
}

static int caph_dma_alloc_ring_descriptors( CAPH_DMA_Handle_t handle, dma_addr_t data_addr, size_t numBytes )
{
   struct caph_dma_device_attr *devAttr;
   int rc;

   devAttr = (struct caph_dma_device_attr*)handle;

   rc = aadma_alloc_ring_descriptors( devAttr->aadma_handle, data_addr, numBytes );

   return rc;
}

static int caph_dma_start_transfer( CAPH_DMA_Handle_t handle )
{
   struct caph_dma_device_attr *devAttr;
   int rc;

   devAttr = (struct caph_dma_device_attr*)handle;

   rc = aadma_start_transfer( devAttr->aadma_handle );

   return rc;
}

static int caph_dma_continue_transfer( CAPH_DMA_Handle_t handle, int dma_status )
{
   struct caph_dma_device_attr *devAttr;
   int rc;

   devAttr = (struct caph_dma_device_attr*)handle;

   rc = aadma_continue_transfer( devAttr->aadma_handle, dma_status );

   return rc;
}

static int caph_dma_enable( CAPH_DMA_Handle_t handle )
{
   struct caph_dma_device_attr *devAttr;

   devAttr = (struct caph_dma_device_attr*)handle;

   aadma_enable( devAttr->aadma_handle );

   return 0;
}

static int caph_dma_sw_enable( CAPH_DMA_Handle_t handle )
{
   struct caph_dma_device_attr *devAttr;

   devAttr = (struct caph_dma_device_attr*)handle;

   ssasw_enable( devAttr->ssasw_handle, 1 );

   return 0;
}

static int caph_dma_mixer_enable( CAPH_DMA_Handle_t handle )
{
   struct caph_dma_device_attr *devAttr;

   devAttr = (struct caph_dma_device_attr*)handle;

   ssasw_mixer_enable( devAttr->ssasw_handle, 1 );

   return 0;
}

static int caph_dma_stop_transfer( CAPH_DMA_Handle_t handle )
{
   struct caph_dma_device_attr *devAttr;

   devAttr = (struct caph_dma_device_attr*)handle;

   aadma_stop_transfer( devAttr->aadma_handle );
   ssasw_remove_all_dst( devAttr->ssasw_handle );
   ssasw_enable( devAttr->ssasw_handle, 0 );
   ssasw_mixer_enable( devAttr->ssasw_handle, 0 );

   return 0;
}

static int caph_dma_free_channel( CAPH_DMA_Handle_t handle )
{
   struct caph_dma_device_attr *devAttr;

   devAttr = (struct caph_dma_device_attr*)handle;

   ssasw_free_channel( devAttr->ssasw_handle );
   aadma_free_channel( devAttr->aadma_handle );

   memset( devAttr, 0, sizeof(struct caph_dma_device_attr) );

   return 0;
}

/***************************************************************************/
/**
*  Platform support constructor
*/
static int audioh_platform_init( HALAUDIO_AUDIOH_PLATFORM_INFO *info )
{
   int            err;

   if ( info->spk_en_gpio.handsfree_left_en >= 0 )
   {
      err = gpio_request( info->spk_en_gpio.handsfree_left_en, "SPKRAMP_L_EN" );
      if( err )
      {
         printk( KERN_ERR "%s: failed to request SPKRAMP_EN gpio pin rc=%u\n", __FUNCTION__, err );
         err = -EBUSY;
         goto cleanup_and_exit;
      }
      gpio_direction_output( info->spk_en_gpio.handsfree_left_en, 0 );
   }
   if ( info->spk_en_gpio.handsfree_right_en >= 0 )
   {
      err = gpio_request( info->spk_en_gpio.handsfree_right_en, "SPKRAMP_R_EN" );
      if( err )
      {
         printk( KERN_ERR "%s: failed to request SPKRAMP_EN gpio pin rc=%u\n", __FUNCTION__, err );
         err = -EBUSY;
         goto cleanup_and_exit;
      }
      gpio_direction_output( info->spk_en_gpio.handsfree_right_en, 0 );
   }
   if ( info->spk_en_gpio.headset_en >= 0 )
   {
      err = gpio_request( info->spk_en_gpio.headset_en, "HEADAMP_EN" );
      if( err )
      {
         printk( KERN_ERR "%s: failed to request HEADAMP_EN gpio pin rc=%u\n", __FUNCTION__, err );
         err = -EBUSY;
         goto cleanup_and_exit;
      }
      gpio_direction_output( info->spk_en_gpio.headset_en, 0 );
   }

   return 0;

cleanup_and_exit:
   audioh_platform_exit( info );
   return err;
}

static void audioh_platform_exit( HALAUDIO_AUDIOH_PLATFORM_INFO *info )
{
   if( info->spk_en_gpio.handsfree_left_en >= 0 )
   {
      gpio_free( info->spk_en_gpio.handsfree_left_en );
   }

   if( info->spk_en_gpio.handsfree_right_en >= 0 )
   {
      gpio_free( info->spk_en_gpio.handsfree_right_en );
   }

   if( info->spk_en_gpio.headset_en >= 0 )
   {
      gpio_free( info->spk_en_gpio.headset_en );
   }
}

static int audioh_probe( struct platform_device *pdev )
{
   int err;
   HALAUDIO_AUDIOH_PLATFORM_INFO *info;

   gAudiohClk.audioh_156m = clk_get( &pdev->dev, "audioh_156m_clk" );
   gAudiohClk.audioh_26m = clk_get( &pdev->dev, "audioh_26m_clk" );
   gAudiohClk.audioh_2p4m = clk_get( &pdev->dev, "audioh_2p4m_clk" );
   gAudiohClk.audioh_apb = clk_get( &pdev->dev, "audioh_apb_clk" );

   err = clk_enable( gAudiohClk.audioh_156m );
   if( err )
   {
      printk( KERN_ERR "%s: failed to enable 156 MHz audioh clock %d!\n", __FUNCTION__, err );
      return err;
   }
   err = clk_enable( gAudiohClk.audioh_26m );
   if( err )
   {
      printk( KERN_ERR "%s: failed to enable 26 MHz audioh clock %d!\n", __FUNCTION__, err );
      return err;
   }
   err = clk_enable( gAudiohClk.audioh_2p4m );
   if( err )
   {
      printk( KERN_ERR "%s: failed to enable 2.4 MHz audioh clock %d!\n", __FUNCTION__, err );
      return err;
   }
   err = clk_enable( gAudiohClk.audioh_apb );
   if( err )
   {
      printk( KERN_ERR "%s: failed to enable audioh apb clock %d!\n", __FUNCTION__, err );
      return err;
   }

   /* Grab platform configuration */
   if ( pdev->dev.platform_data == NULL )
   {
      printk( KERN_ERR "%s: missing platform_data\n",  __FUNCTION__ );
      return -ENODEV;
   }
   memcpy( &gAudiohPlatformInfo, pdev->dev.platform_data, sizeof( gAudiohPlatformInfo ));
   info = &gAudiohPlatformInfo;

   printk( KERN_INFO "HAL Audio Audio Hub Driver: 1.0. Built %s %s\n", __DATE__, __TIME__ );

   err = audioh_platform_init( info );
   if( err )
   {
      printk( KERN_ERR "%s: failed to initialize the AUDIOH platform %d!\n", __FUNCTION__, err );
      return err;
   }

   err = halAudioAddInterface( &halaudio_audioh_ops, AUDIOH_MAX_NUM_CHANS, "AUDIOH",
         AUDIOH_DEFAULT_FRAME_PERIOD, 1 /* synchronize */, &gInterfHandle );

   if( err )
   {
      printk( KERN_ERR "Audioh: failed to install the audio interface %d!\n", err );
      goto err_platform_exit;
   }

   return 0;

err_platform_exit:
   audioh_platform_exit( info );
   return err;
}

static int __exit audioh_remove( struct platform_device *pdev )
{
   HALAUDIO_AUDIOH_PLATFORM_INFO *info = &gAudiohPlatformInfo;

   (void)pdev;
   halAudioDelInterface( gInterfHandle );
   audioh_platform_exit( info );
   clk_disable( gAudiohClk.audioh_26m );
   clk_put( gAudiohClk.audioh_26m );

   return 0;
}

static struct platform_driver audioh_driver =
{
   .driver =
   {
      .name = "bcm-halaudio-audioh",
      .owner = THIS_MODULE,
   },
   .probe = audioh_probe,
   .remove = audioh_remove,
};


/***************************************************************************/
/**
*  Driver initialization called when module loaded by kernel
*
*  @return
*     0              Success
*     -ve            Error code
*/
static int __init audioh_init( void )
{
   return platform_driver_register( &audioh_driver );
}

/***************************************************************************/
/**
*  Driver destructor routine.
*/
static void __exit audioh_exit( void )
{
   platform_driver_unregister( &audioh_driver );
}

module_init( audioh_init );
module_exit( audioh_exit );

MODULE_AUTHOR( "Broadcom" );
MODULE_DESCRIPTION( "HAL Audio Audio Hub Driver" );
MODULE_LICENSE( "GPL" );
