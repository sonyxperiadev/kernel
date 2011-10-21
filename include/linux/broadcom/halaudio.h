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
*  @file    halaudio.h
*
*  @brief   HAL Audio API definitions
*
*****************************************************************************/
#if !defined( HALAUDIO_H )
#define HALAUDIO_H

/* ---- Include Files ---------------------------------------------------- */

#if defined( __KERNEL__ )
#include <linux/types.h>                     /* Needed for standard types */
#else
#include <stdint.h>
#endif

#if defined( __KERNEL__ )
#include <linux/broadcom/csx.h>              /* Needed for CSX hooks */
#endif

/* ---- Constants and Types ---------------------------------------------- */

/**
*  Hardware audio codec channel number. Audio channels are numbered starting
*  from 0. The enumeration below is provided as convenience to identify
*  codec channels by enumerated strings as opposed to hard-coded numbers.
*  The total number of supported channels should be queried with
*  the API function halAudioGetHardwareInfo. There is no theoretical limit
*  to the number of supported codec channels.
*/
typedef unsigned int HALAUDIO_CODEC;
#define HALAUDIO_CODEC_INVALID               ((HALAUDIO_CODEC)-1)
enum
{
   HALAUDIO_CODEC0 = 0,
   HALAUDIO_CODEC1,
   HALAUDIO_CODEC2,
   HALAUDIO_CODEC3,
   HALAUDIO_CODEC4,
   HALAUDIO_CODEC5,
   HALAUDIO_CODEC6,
   HALAUDIO_CODEC7,
   HALAUDIO_CODEC8,
   HALAUDIO_CODEC9,
   HALAUDIO_CODEC10,
   HALAUDIO_CODEC11,
   HALAUDIO_CODEC12,
   HALAUDIO_CODEC13,
   HALAUDIO_CODEC14,
   HALAUDIO_CODEC15,
   HALAUDIO_CODEC16,
   HALAUDIO_CODEC17,
   HALAUDIO_CODEC18,
   HALAUDIO_CODEC19,
};

/**
*  Hardware audio interface number. Registered interfaces are numbered
*  starting from 0. Interfaces are numbered starting from 0. The enumeration
*  below is provided as a convenience to idientify the interface by
*  enumerated strings as opposed to hard-coded constants. There is no
*  theoretical limit to the number of supported interfaces.
*/
typedef unsigned int HALAUDIO_IF;
enum
{
   HALAUDIO_IF0 = 0,
   HALAUDIO_IF1,
   HALAUDIO_IF2,
   HALAUDIO_IF3,
   HALAUDIO_IF4,
   HALAUDIO_IF5,
   HALAUDIO_IF6,
   HALAUDIO_IF7,
   HALAUDIO_IF8,
   HALAUDIO_IF9,
};

/**
*  The block ID used to identify the analog gain, digital gain and sidetone
*  blocks uniquely. Each unique block id is formed from the following
*  bit fields:
*
*  codec    - Physical codec ID (HALAUDIO_CODEC)
*  hwsel    - HW mux selection (HALAUDIO_HWSEL)
*  dir      - Direction (HALAUDIO_DIR)
*
*  Digital block ID's are similar to analog block ID's, except that
*  the hwsel field is set to 0 and the digital bit is set.
*
*  Sideonte block ID's consist of a codec id and the sidetone bit.
*
*  Bit field breakdown:
*
*  | reserved (b31-b16) | codec (b15-b8) | hwsel (b7-b4) | digital (b3) | sidetone (b2) | dir (b1-b0) |
*/
typedef unsigned int HALAUDIO_BLOCK;

#define HALAUDIO_BLOCK_CODEC_BMASK              0xFF00
#define HALAUDIO_BLOCK_HWSEL_BMASK              0x00F0
#define HALAUDIO_BLOCK_DIGITAL_BMASK            0x0008
#define HALAUDIO_BLOCK_SIDETONE_BMASK           0x0004
#define HALAUDIO_BLOCK_DIR_BMASK                0x0003

#define HALAUDIO_BLOCK_CODEC_BSHIFT             8
#define HALAUDIO_BLOCK_HWSEL_BSHIFT             4
#define HALAUDIO_BLOCK_DIGITAL_BSHIFT           3
#define HALAUDIO_BLOCK_SIDETONE_BSHIFT          2
#define HALAUDIO_BLOCK_DIR_BSHIFT               0

#define HALAUDIO_CREATE_BLOCK( codec, hwsel, digital, sidetone, dir ) \
   ((( (codec) << HALAUDIO_BLOCK_CODEC_BSHIFT ) & HALAUDIO_BLOCK_CODEC_BMASK ) | \
    (( (hwsel) << HALAUDIO_BLOCK_HWSEL_BSHIFT ) & HALAUDIO_BLOCK_HWSEL_BMASK ) | \
    (( (digital) << HALAUDIO_BLOCK_DIGITAL_BSHIFT ) & HALAUDIO_BLOCK_DIGITAL_BMASK ) | \
    (( (sidetone) << HALAUDIO_BLOCK_SIDETONE_BSHIFT ) & HALAUDIO_BLOCK_SIDETONE_BMASK ) | \
    (( (dir) << HALAUDIO_BLOCK_DIR_BSHIFT ) & HALAUDIO_BLOCK_DIR_BMASK ))

/* Helpers to create or operate on block ID */
#define HALAUDIO_BLOCK_ANA_ID( codec,sel,dir )  HALAUDIO_CREATE_BLOCK( codec, sel, 0, 0, dir )
#define HALAUDIO_BLOCK_DIG_ID( codec, dir )     HALAUDIO_CREATE_BLOCK( codec, 0,   1, 0, dir )
#define HALAUDIO_BLOCK_SIDETONE_ID( codec )     HALAUDIO_CREATE_BLOCK( codec, 0,   0, 1, 0   )
#define HALAUDIO_BLOCK_GET_CODEC( block )       (( block & HALAUDIO_BLOCK_CODEC_BMASK ) >> HALAUDIO_BLOCK_CODEC_BSHIFT )
#define HALAUDIO_BLOCK_GET_HWSEL( block )       (( block & HALAUDIO_BLOCK_HWSEL_BMASK ) >> HALAUDIO_BLOCK_HWSEL_BSHIFT )
#define HALAUDIO_BLOCK_GET_DIR( block )         (( block & HALAUDIO_BLOCK_DIR_BMASK   ) >> HALAUDIO_BLOCK_DIR_BSHIFT   )
#define HALAUDIO_BLOCK_IS_DIGITAL( block )      ( block & HALAUDIO_BLOCK_DIGITAL_BMASK )
#define HALAUDIO_BLOCK_IS_SIDETONE( block )     ( block & HALAUDIO_BLOCK_SIDETONE_BMASK )

/* Useful pre-defined HAL Audio block IDs. Users may create their block IDs
 * using the appropriate helper macros instead of using the following.
 */
#define HALAUDIO_CODEC0A_SPKR                   HALAUDIO_BLOCK_ANA_ID( HALAUDIO_CODEC0, HALAUDIO_HWSEL_A, HALAUDIO_DIR_DAC )
#define HALAUDIO_CODEC0B_SPKR                   HALAUDIO_BLOCK_ANA_ID( HALAUDIO_CODEC0, HALAUDIO_HWSEL_B, HALAUDIO_DIR_DAC )
#define HALAUDIO_CODEC0C_SPKR                   HALAUDIO_BLOCK_ANA_ID( HALAUDIO_CODEC0, HALAUDIO_HWSEL_C, HALAUDIO_DIR_DAC )
#define HALAUDIO_CODEC0A_MIC                    HALAUDIO_BLOCK_ANA_ID( HALAUDIO_CODEC0, HALAUDIO_HWSEL_A, HALAUDIO_DIR_ADC )
#define HALAUDIO_CODEC0B_MIC                    HALAUDIO_BLOCK_ANA_ID( HALAUDIO_CODEC0, HALAUDIO_HWSEL_B, HALAUDIO_DIR_ADC )
#define HALAUDIO_CODEC0C_MIC                    HALAUDIO_BLOCK_ANA_ID( HALAUDIO_CODEC0, HALAUDIO_HWSEL_C, HALAUDIO_DIR_ADC )
#define HALAUDIO_CODEC0_ADC_DGAIN               HALAUDIO_BLOCK_DIG_ID( HALAUDIO_CODEC0, HALAUDIO_DIR_ADC )
#define HALAUDIO_CODEC0_DAC_DGAIN               HALAUDIO_BLOCK_DIG_ID( HALAUDIO_CODEC0, HALAUDIO_DIR_DAC )
#define HALAUDIO_CODEC0_SIDETONE                HALAUDIO_BLOCK_SIDETONE_ID( HALAUDIO_CODEC0 )

#define HALAUDIO_CODEC1A_SPKR                   HALAUDIO_BLOCK_ANA_ID( HALAUDIO_CODEC1, HALAUDIO_HWSEL_A, HALAUDIO_DIR_DAC )
#define HALAUDIO_CODEC1B_SPKR                   HALAUDIO_BLOCK_ANA_ID( HALAUDIO_CODEC1, HALAUDIO_HWSEL_B, HALAUDIO_DIR_DAC )
#define HALAUDIO_CODEC1C_SPKR                   HALAUDIO_BLOCK_ANA_ID( HALAUDIO_CODEC1, HALAUDIO_HWSEL_C, HALAUDIO_DIR_DAC )
#define HALAUDIO_CODEC1A_MIC                    HALAUDIO_BLOCK_ANA_ID( HALAUDIO_CODEC1, HALAUDIO_HWSEL_A, HALAUDIO_DIR_ADC )
#define HALAUDIO_CODEC1B_MIC                    HALAUDIO_BLOCK_ANA_ID( HALAUDIO_CODEC1, HALAUDIO_HWSEL_B, HALAUDIO_DIR_ADC )
#define HALAUDIO_CODEC1C_MIC                    HALAUDIO_BLOCK_ANA_ID( HALAUDIO_CODEC1, HALAUDIO_HWSEL_C, HALAUDIO_DIR_ADC )
#define HALAUDIO_CODEC1_ADC_DGAIN               HALAUDIO_BLOCK_DIG_ID( HALAUDIO_CODEC1, HALAUDIO_DIR_ADC )
#define HALAUDIO_CODEC1_DAC_DGAIN               HALAUDIO_BLOCK_DIG_ID( HALAUDIO_CODEC1, HALAUDIO_DIR_DAC )
#define HALAUDIO_CODEC1_SIDETONE                HALAUDIO_BLOCK_SIDETONE_ID( HALAUDIO_CODEC1 )

#define HALAUDIO_CODEC2A_SPKR                   HALAUDIO_BLOCK_ANA_ID( HALAUDIO_CODEC2, HALAUDIO_HWSEL_A, HALAUDIO_DIR_DAC )
#define HALAUDIO_CODEC2B_SPKR                   HALAUDIO_BLOCK_ANA_ID( HALAUDIO_CODEC2, HALAUDIO_HWSEL_B, HALAUDIO_DIR_DAC )
#define HALAUDIO_CODEC2C_SPKR                   HALAUDIO_BLOCK_ANA_ID( HALAUDIO_CODEC2, HALAUDIO_HWSEL_C, HALAUDIO_DIR_DAC )
#define HALAUDIO_CODEC2A_MIC                    HALAUDIO_BLOCK_ANA_ID( HALAUDIO_CODEC2, HALAUDIO_HWSEL_A, HALAUDIO_DIR_ADC )
#define HALAUDIO_CODEC2B_MIC                    HALAUDIO_BLOCK_ANA_ID( HALAUDIO_CODEC2, HALAUDIO_HWSEL_B, HALAUDIO_DIR_ADC )
#define HALAUDIO_CODEC2C_MIC                    HALAUDIO_BLOCK_ANA_ID( HALAUDIO_CODEC2, HALAUDIO_HWSEL_C, HALAUDIO_DIR_ADC )
#define HALAUDIO_CODEC2_ADC_DGAIN               HALAUDIO_BLOCK_DIG_ID( HALAUDIO_CODEC2, HALAUDIO_DIR_ADC )
#define HALAUDIO_CODEC2_DAC_DGAIN               HALAUDIO_BLOCK_DIG_ID( HALAUDIO_CODEC2, HALAUDIO_DIR_DAC )
#define HALAUDIO_CODEC2_SIDETONE                HALAUDIO_BLOCK_SIDETONE_ID( HALAUDIO_CODEC2 )

#define HALAUDIO_CODEC3A_SPKR                   HALAUDIO_BLOCK_ANA_ID( HALAUDIO_CODEC3, HALAUDIO_HWSEL_A, HALAUDIO_DIR_DAC )
#define HALAUDIO_CODEC3B_SPKR                   HALAUDIO_BLOCK_ANA_ID( HALAUDIO_CODEC3, HALAUDIO_HWSEL_B, HALAUDIO_DIR_DAC )
#define HALAUDIO_CODEC3C_SPKR                   HALAUDIO_BLOCK_ANA_ID( HALAUDIO_CODEC3, HALAUDIO_HWSEL_C, HALAUDIO_DIR_DAC )
#define HALAUDIO_CODEC3A_MIC                    HALAUDIO_BLOCK_ANA_ID( HALAUDIO_CODEC3, HALAUDIO_HWSEL_A, HALAUDIO_DIR_ADC )
#define HALAUDIO_CODEC3B_MIC                    HALAUDIO_BLOCK_ANA_ID( HALAUDIO_CODEC3, HALAUDIO_HWSEL_B, HALAUDIO_DIR_ADC )
#define HALAUDIO_CODEC3C_MIC                    HALAUDIO_BLOCK_ANA_ID( HALAUDIO_CODEC3, HALAUDIO_HWSEL_C, HALAUDIO_DIR_ADC )
#define HALAUDIO_CODEC3_ADC_DGAIN               HALAUDIO_BLOCK_DIG_ID( HALAUDIO_CODEC3, HALAUDIO_DIR_ADC )
#define HALAUDIO_CODEC3_DAC_DGAIN               HALAUDIO_BLOCK_DIG_ID( HALAUDIO_CODEC3, HALAUDIO_DIR_DAC )
#define HALAUDIO_CODEC3_SIDETONE                HALAUDIO_BLOCK_SIDETONE_ID( HALAUDIO_CODEC3 )

#define HALAUDIO_CODEC4A_SPKR                   HALAUDIO_BLOCK_ANA_ID( HALAUDIO_CODEC4, HALAUDIO_HWSEL_A, HALAUDIO_DIR_DAC )
#define HALAUDIO_CODEC4B_SPKR                   HALAUDIO_BLOCK_ANA_ID( HALAUDIO_CODEC4, HALAUDIO_HWSEL_B, HALAUDIO_DIR_DAC )
#define HALAUDIO_CODEC4C_SPKR                   HALAUDIO_BLOCK_ANA_ID( HALAUDIO_CODEC4, HALAUDIO_HWSEL_C, HALAUDIO_DIR_DAC )
#define HALAUDIO_CODEC4A_MIC                    HALAUDIO_BLOCK_ANA_ID( HALAUDIO_CODEC4, HALAUDIO_HWSEL_A, HALAUDIO_DIR_ADC )
#define HALAUDIO_CODEC4B_MIC                    HALAUDIO_BLOCK_ANA_ID( HALAUDIO_CODEC4, HALAUDIO_HWSEL_B, HALAUDIO_DIR_ADC )
#define HALAUDIO_CODEC4C_MIC                    HALAUDIO_BLOCK_ANA_ID( HALAUDIO_CODEC4, HALAUDIO_HWSEL_C, HALAUDIO_DIR_ADC )
#define HALAUDIO_CODEC4_ADC_DGAIN               HALAUDIO_BLOCK_DIG_ID( HALAUDIO_CODEC4, HALAUDIO_DIR_ADC )
#define HALAUDIO_CODEC4_DAC_DGAIN               HALAUDIO_BLOCK_DIG_ID( HALAUDIO_CODEC4, HALAUDIO_DIR_DAC )
#define HALAUDIO_CODEC4_SIDETONE                HALAUDIO_BLOCK_SIDETONE_ID( HALAUDIO_CODEC4 )

#define HALAUDIO_CODEC5A_SPKR                   HALAUDIO_BLOCK_ANA_ID( HALAUDIO_CODEC5, HALAUDIO_HWSEL_A, HALAUDIO_DIR_DAC )
#define HALAUDIO_CODEC5B_SPKR                   HALAUDIO_BLOCK_ANA_ID( HALAUDIO_CODEC5, HALAUDIO_HWSEL_B, HALAUDIO_DIR_DAC )
#define HALAUDIO_CODEC5C_SPKR                   HALAUDIO_BLOCK_ANA_ID( HALAUDIO_CODEC5, HALAUDIO_HWSEL_C, HALAUDIO_DIR_DAC )
#define HALAUDIO_CODEC5A_MIC                    HALAUDIO_BLOCK_ANA_ID( HALAUDIO_CODEC5, HALAUDIO_HWSEL_A, HALAUDIO_DIR_ADC )
#define HALAUDIO_CODEC5B_MIC                    HALAUDIO_BLOCK_ANA_ID( HALAUDIO_CODEC5, HALAUDIO_HWSEL_B, HALAUDIO_DIR_ADC )
#define HALAUDIO_CODEC5C_MIC                    HALAUDIO_BLOCK_ANA_ID( HALAUDIO_CODEC5, HALAUDIO_HWSEL_C, HALAUDIO_DIR_ADC )
#define HALAUDIO_CODEC5_ADC_DGAIN               HALAUDIO_BLOCK_DIG_ID( HALAUDIO_CODEC5, HALAUDIO_DIR_ADC )
#define HALAUDIO_CODEC5_DAC_DGAIN               HALAUDIO_BLOCK_DIG_ID( HALAUDIO_CODEC5, HALAUDIO_DIR_DAC )
#define HALAUDIO_CODEC5_SIDETONE                HALAUDIO_BLOCK_SIDETONE_ID( HALAUDIO_CODEC5 )

#define HALAUDIO_CODEC6A_SPKR                   HALAUDIO_BLOCK_ANA_ID( HALAUDIO_CODEC6, HALAUDIO_HWSEL_A, HALAUDIO_DIR_DAC )
#define HALAUDIO_CODEC6B_SPKR                   HALAUDIO_BLOCK_ANA_ID( HALAUDIO_CODEC6, HALAUDIO_HWSEL_B, HALAUDIO_DIR_DAC )
#define HALAUDIO_CODEC6C_SPKR                   HALAUDIO_BLOCK_ANA_ID( HALAUDIO_CODEC6, HALAUDIO_HWSEL_C, HALAUDIO_DIR_DAC )
#define HALAUDIO_CODEC6A_MIC                    HALAUDIO_BLOCK_ANA_ID( HALAUDIO_CODEC6, HALAUDIO_HWSEL_A, HALAUDIO_DIR_ADC )
#define HALAUDIO_CODEC6B_MIC                    HALAUDIO_BLOCK_ANA_ID( HALAUDIO_CODEC6, HALAUDIO_HWSEL_B, HALAUDIO_DIR_ADC )
#define HALAUDIO_CODEC6C_MIC                    HALAUDIO_BLOCK_ANA_ID( HALAUDIO_CODEC6, HALAUDIO_HWSEL_C, HALAUDIO_DIR_ADC )
#define HALAUDIO_CODEC6_ADC_DGAIN               HALAUDIO_BLOCK_DIG_ID( HALAUDIO_CODEC6, HALAUDIO_DIR_ADC )
#define HALAUDIO_CODEC6_DAC_DGAIN               HALAUDIO_BLOCK_DIG_ID( HALAUDIO_CODEC6, HALAUDIO_DIR_DAC )
#define HALAUDIO_CODEC6_SIDETONE                HALAUDIO_BLOCK_SIDETONE_ID( HALAUDIO_CODEC6 )

#define HALAUDIO_CODEC7A_SPKR                   HALAUDIO_BLOCK_ANA_ID( HALAUDIO_CODEC7, HALAUDIO_HWSEL_A, HALAUDIO_DIR_DAC )
#define HALAUDIO_CODEC7B_SPKR                   HALAUDIO_BLOCK_ANA_ID( HALAUDIO_CODEC7, HALAUDIO_HWSEL_B, HALAUDIO_DIR_DAC )
#define HALAUDIO_CODEC7C_SPKR                   HALAUDIO_BLOCK_ANA_ID( HALAUDIO_CODEC7, HALAUDIO_HWSEL_C, HALAUDIO_DIR_DAC )
#define HALAUDIO_CODEC7A_MIC                    HALAUDIO_BLOCK_ANA_ID( HALAUDIO_CODEC7, HALAUDIO_HWSEL_A, HALAUDIO_DIR_ADC )
#define HALAUDIO_CODEC7B_MIC                    HALAUDIO_BLOCK_ANA_ID( HALAUDIO_CODEC7, HALAUDIO_HWSEL_B, HALAUDIO_DIR_ADC )
#define HALAUDIO_CODEC7C_MIC                    HALAUDIO_BLOCK_ANA_ID( HALAUDIO_CODEC7, HALAUDIO_HWSEL_C, HALAUDIO_DIR_ADC )
#define HALAUDIO_CODEC7_ADC_DGAIN               HALAUDIO_BLOCK_DIG_ID( HALAUDIO_CODEC7, HALAUDIO_DIR_ADC )
#define HALAUDIO_CODEC7_DAC_DGAIN               HALAUDIO_BLOCK_DIG_ID( HALAUDIO_CODEC7, HALAUDIO_DIR_DAC )
#define HALAUDIO_CODEC7_SIDETONE                HALAUDIO_BLOCK_SIDETONE_ID( HALAUDIO_CODEC7 )

/**
*  There are three power levels: deep sleep, digital only, and full power.
*  Digital only powers the digital blocks and enables the interrupts.
*/
typedef enum
{
   HALAUDIO_POWER_DEEP_SLEEP,       /* Deep sleep */
   HALAUDIO_POWER_DIGITAL_ONLY,     /* Digital blocks enabled only */
   HALAUDIO_POWER_FULL_POWER,       /* Full analog and digital power */
}
HALAUDIO_POWER_LEVEL;

/**
*  Data flow direction
*/
typedef enum
{
   HALAUDIO_DIR_ADC = 0,            /* microphone to ADC direction */
   HALAUDIO_DIR_DAC,                /* DAC to speaker direction */
   HALAUDIO_DIR_BOTH,               /* both directions */
}
HALAUDIO_DIR;

/**
*  Hardware multiplex select positions.
*/
typedef enum
{
   HALAUDIO_HWSEL_NONE = -1,        /* no path */
   HALAUDIO_HWSEL_ALL  = 0,         /* all paths */
   HALAUDIO_HWSEL_A,
   HALAUDIO_HWSEL_B,
   HALAUDIO_HWSEL_C,
   HALAUDIO_HWSEL_D,
   HALAUDIO_HWSEL_MAX_NUM
}
HALAUDIO_HWSEL;

/**
*  Data format used with read and write operations
*/
typedef enum
{
   HALAUDIO_FMT_S16_BE,             /* signed 16-bit big-endian */
   HALAUDIO_FMT_S16_LE,             /* signed 16-bit little-endian */
   HALAUDIO_FMT_ULAW,               /* 8-bit G.711 u-law */
   HALAUDIO_FMT_ALAW,               /* 8-bit G.711 a-law */
   HALAUDIO_FMT_MAX                 /* last entry */
}
HALAUDIO_FMT;

/**
*  HAL Audio specific CSX I/O points used for capture and inject.
*  Enumerated as an index.
*/
typedef enum
{
   HALAUDIO_CSX_POINT_ADC = 0,
   HALAUDIO_CSX_POINT_DAC,

   HALAUDIO_NUM_CSX_POINTS
}
HALAUDIO_CSX_POINT_ID;

/**
*  Special gain mapping for mute. This mapping is used in conjunction
*  with setting analog and digital gains. The value of -1000 is
*  arbitrarily choosen, as this value is essentially equal to -infinity in dB.
*/
#define  HALAUDIO_GAIN_MUTE           -1000

/**
*  Types of gain ranges. Some gain ranges are linear with a fixed step size
*  between gain values, where others ranges are specific values.
*/
typedef enum
{
   HALAUDIO_RANGETYPE_FIXED_STEPSIZE,  /* Fixed step size */
   HALAUDIO_RANGETYPE_LIST             /* Specific values with variable step sizes */
}
HALAUDIO_RANGETYPE;

/**
*  Gain range structure used to hold the valid gain range for a
*  hardware analog or digital block.
*/
#define HALAUDIO_DB_SETTINGS_MAX_NUM   128
typedef union
{
   int    fixed_step;                  /* Fixed step size */
   struct                              /* List of gains for variable step sizes */
   {
      int num;
      int db[HALAUDIO_DB_SETTINGS_MAX_NUM];
   } list;
}
HALAUDIO_RANGE;

/**
*  Sampling frequency list used to describe the frequencies
*  supported in hardware.
*/
#define HALAUDIO_MAX_NUM_FREQ_SETTINGS 10
typedef struct halaudio_freqs
{
   unsigned int num;                /* Number of supported frequencies */
   /* Frequency list sorted from low to high rates */
   unsigned int freq[HALAUDIO_MAX_NUM_FREQ_SETTINGS];
}
HALAUDIO_FREQS;

/**
*  Equalizer parameters structure
*/
#define HALAUDIO_EQU_COEFS_MAX_NUM       256
typedef struct halaudio_equ
{
   int      len;                    /* Number of filter coefficents, 0 is to disable */
   int32_t  coeffs[HALAUDIO_EQU_COEFS_MAX_NUM]; /* 32-bit coefficients */
   void    *extra;                  /* Platform specific extra data */
}
HALAUDIO_EQU;

/**
*  High-level hardware information used to describe the number of audio
*  interfaces installed, the number of audio codecs and other information
*/
typedef struct halaudio_hw_info
{
   int interfaces;                  /* Number of register audio interfaces */
   int codecs;                      /* Total number of audio codecs across all interfaces */
   int frame_period_us;             /* Default period between interrupts in usec */
}
HALAUDIO_HW_INFO;

/**
*  Audio interface information used to describe the number of audio
*  codecs it has and other information.
*/
typedef struct halaudio_if_info
{
   char  name[32];                  /* Interface name */
   int   codecs;                    /* Number of audio codecs in this interface */
   int   frame_period_us;           /* Period between interrupts in usec for interface */
   int   enabled;                   /* 1 if enabled, otherwise 0 */
   int   sync;                      /* 1 if synchronized with others */
}
HALAUDIO_IF_INFO;

/**
*  Audio codec info structure used to describe the audio codec's
*  capabilities and other information.
*/
typedef struct halaudio_codec_info
{
   char           name[32];         /* Codec name */
   HALAUDIO_FREQS freqs;            /* Supported sampling frequencies */
   int            channels_tx;      /* # of channels for tx (mic), 1=mono, 2=stereo, etc. */
   int            channels_rx;      /* # of channels for rx (spk), 1=mono, 2=stereo, etc. */
   int            equlen_tx;        /* max # of tx equalizer coeffs, 0 = equ unsupported */
   int            equlen_rx;        /* max # of rx equalizer coeffs, 0 = equ unsupported */
   int            sample_width;     /* sample size width in bytes */
   int            mics;             /* # of mic paths. 0 means no ADC */
   int            spkrs;            /* # of speaker paths. 0 means no DAC */
   int            bulk_delay;       /* Echo bulk delay in samples, -1 = not calibrated */
   unsigned long  locked;           /* non-zero if in locked state, otherwise 0 */
   HALAUDIO_IF    parent_id;        /* Parent interface id */
   HALAUDIO_FMT   read_format;      /* Read format */
   HALAUDIO_FMT   write_format;     /* Write format */
}
HALAUDIO_CODEC_INFO;

/**
*  Audio gain block info structure used to describe each block's (either
*  analog or digital) gain capabilities and other properties.
*/
typedef struct halaudio_gain_info
{
   int                  currentdb;  /* Current gain value in db */
   int                  mindb;      /* Minimum gain value in db */
   int                  maxdb;      /* Maximum gain value in db */
   HALAUDIO_RANGETYPE   range_type; /* Type of gain range */
   HALAUDIO_RANGE       range;      /* Gain rain */
}
HALAUDIO_GAIN_INFO;

/* HAL Audio handle for client. */
#if defined( __KERNEL__ )
typedef void * HALAUDIO_HDL;
#else
typedef int    HALAUDIO_HDL;
#endif

#if defined( __KERNEL__ )
/* MakeDefs: Off */

/* ---- Kernel Constants, Types, and Definitions ------------------------- */

/***************************************************************************/
/**
*  HAL Audio Codec operations
*
*  HAL Audio Codecs are audio channels that support a DAC, an ADC or both.
*  They belong to a HAL Audio Interface.
*
*  HAL Audio Codecs are named two ways: (1) internally within an audio
*  interface, and (2) externally for users of HAL Audio. Internally, relative
*  to other audio channels within the same audio interface, they are numbered
*  starting from 0. For example, within the APM interface, there may be three
*  audio channels that are labeled as channels 0, 1, and 2. Externally,
*  regardless of which audio interface they belong to, all audio codecs are
*  numbered uniquely starting from 0 (i.e. HALAUDIO_CODEC). The numbering is
*  assigned based on order of initialization. For example, suppose the APM
*  and I2S interfaces are installed in that order. The APM support 3 audio
*  channels, whereas the I2S interface supports 2 audio channels. The 5
*  audio channels for users of HAL Audio will refer to them as HALAUDIO_CODEC0
*  through to HALAUDIO_CODEC4, with the first three labels reserved for
*  APM channels and the latter two labels reserved for I2S channels.
*/

/***************************************************************************/
/**
*  Read/write IO callback used by low-level driver to indicate that the
*  operation has completed.
*/
typedef void (*HALAUDIO_CODEC_IORW_CB)(
   int bytes,                       /**< (i) Number of bytes transacted */
   void *data                       /**< (i) User data */
);

/***************************************************************************/
/**
*  Select sampling frequency for a codec channel.
*
*  @return
*     0        Success
*     -ve      Error code
*/
typedef int (*HALAUDIO_CODEC_SET_FREQ_OP)(
   int chno,                        /**< (i) Codec channel number */
   int freqhz                       /**< (i) Sampling frequency in Hz */
);

/***************************************************************************/
/**
*  Retrieve sampling frequency information for a codec channel.
*
*  @return
*     0        Success
*     -ve      Error code
*/
typedef int (*HALAUDIO_CODEC_GET_FREQ_OP)(
   int   chno,                      /**< (i) Codec channel number */
   int  *freqhz                     /**< (o) Ptr to sampling frequency in Hz */
);

/***************************************************************************/
/**
*  Set analog gains for a codec channel.
*
*  @return
*     0        Success
*     -ve      Error code
*
*  @remarks
*     Mute may be set with db=HALAUDIO_GAIN_MUTE.
*
*     Microphone selection behaves differently from speaker selection
*     because microphone inputs are mutually exclusive. Selecting one
*     microphone position is equivalent to deselecting the other
*     microphone positions. In contrasts, selecting a speaker position
*     does not preclude selecting another speaker position, which
*     means that multiple speaker paths may be active simultaneously.
*/
typedef int (*HALAUDIO_CODEC_SET_ANA_OP)(
   int            chno,             /**< (i) Codec channel number */
   int            db,               /**< (i) Gain in db */
   HALAUDIO_DIR   dir,              /**< (i) Direction path */
   HALAUDIO_HWSEL hwsel             /**< (i) Hardware mux selection */
);

/***************************************************************************/
/**
*  Retrieve analog gains for a codec channel. Valid gain range excludes
*  mute.
*
*  @return
*     0        Success
*     -ve      Error code
*/
typedef int (*HALAUDIO_CODEC_GET_ANA_OP)(
   int                  chno,       /**< (i) codec channel number */
   HALAUDIO_GAIN_INFO  *info,       /**< (o) Ptr to gain info structure */
   HALAUDIO_DIR         dir,        /**< (i) Direction path */
   HALAUDIO_HWSEL       hwsel       /**< (i) Hardware mux selection */
);

/***************************************************************************/
/**
*  Set digital gains for a codec channel. Mute may be set with
*  db=HALAUDIO_GAIN_MUTE.
*
*  @return
*     0        Success
*     -ve      Error code
*/
typedef int (*HALAUDIO_CODEC_SET_DIG_OP)(
   int            chno,             /**< (i) Codec channel number */
   int            db,               /**< (i) Gain in db */
   HALAUDIO_DIR   dir               /**< (i) Direction path */
);

/***************************************************************************/
/**
*  Retrieve digital gains for a codec channel. Valid gain range excludes
*  mute.
*
*  @return
*     0        Success
*     -ve      Error code
*/
typedef int (*HALAUDIO_CODEC_GET_DIG_OP)(
   int                  chno,       /**< (i) codec channel number */
   HALAUDIO_GAIN_INFO  *info,       /**< (o) Ptr to gain info structure */
   HALAUDIO_DIR         dir         /**< (i) Direction path */
);

/***************************************************************************/
/**
*  Set sidetone gain. Mute may be set with db=HALAUDIO_GAIN_MUTE.
*
*  @return
*     0        Success
*     -ve      Error code
*/
typedef int (*HALAUDIO_CODEC_SET_SIDETONE_OP)(
   int chno,                        /**< (i) Codec channel number */
   int db                           /**< (i) Gain in db */
);

/***************************************************************************/
/**
*  Query sidetone gain info. Current gain and other gain information
*  are returned.
*
*  @return
*     0        Success
*     -ve      Error code
*/
typedef int (*HALAUDIO_CODEC_GET_SIDETONE_OP)(
   int                  chno,       /**< (i) codec channel number */
   HALAUDIO_GAIN_INFO  *info        /**< (o) Ptr to gain info structure */
);

/***************************************************************************/
/**
*  Set equalizer parameters such as filter coefficients, filter length,
*  and other parameters.
*
*  @return
*     0        Success
*     -ve      Error code
*/
typedef int (*HALAUDIO_CODEC_SET_EQU_OP)(
   int                  chno,       /**< (i) Codec channel number */
   HALAUDIO_DIR         dir,        /**< (i) Direction path */
   const HALAUDIO_EQU  *equ         /**< (i) Ptr to equalizer parameters */
);

/***************************************************************************/
/**
*  Query equalizer parameters such as filter coefficients, filter length,
*  and other parameters.
*
*  @return
*     0        Success
*     -ve      Error code
*/
typedef int (*HALAUDIO_CODEC_GET_EQU_OP)(
   int                  chno,       /**< (i) Codec channel number */
   HALAUDIO_DIR         dir,        /**< (i) Direction path */
   HALAUDIO_EQU        *equ         /**< (0) Ptr to equalizer parameters */
);

/***************************************************************************/
/**
*  Read raw audio buffer. This is a non-blocking call. The registered
*  callback will be triggered when more samples are available to read.
*
*  @return
*     0        Success
*     -ve      Error code
*/
typedef int (*HALAUDIO_CODEC_READ_OP)(
   int                     chno,    /**< (i) Codec channel number */
   int                     bytes,   /**< (i) Number of bytes to read */
   char                   *audiobuf,/**< (o) Ptr to buffer to store samples */
   HALAUDIO_CODEC_IORW_CB  cb,      /**< (i) Callback to call when more samples are avail */
   void                   *data     /**< (i) User data to pass to callback */
);

/***************************************************************************/
/**
*  Write to raw audio buffer. This is a non-blocking call. The registered
*  callback will be triggered when there is more room to write.
*
*  @return
*     0        Success
*     -ve      Error code
*/
typedef int (*HALAUDIO_CODEC_WRITE_OP)(
   int                     chno,    /**< (i) Codec channel number */
   int                     bytes,   /**< (i) Number of bytes to read */
   const char             *audiobuf,/**< (o) Ptr to buffer to store samples */
   HALAUDIO_CODEC_IORW_CB  cb,      /**< (i) Callback to call when more samples are avail */
   void                   *data     /**< (i) User data to pass to callback */
);

/***************************************************************************/
/**
*  Query detailed codec information. Refer to codec information structure
*  for more details.
*
*  @return
*     0        Success
*     -ve      Error code
*/
typedef int (*HALAUDIO_CODEC_GET_INFO_OP)(
   int                  chno,       /**< (i) Codec channel number */
   HALAUDIO_CODEC_INFO *codec_info  /**< (o) Ptr to codec info structure */
);


/***************************************************************************/
/**
*  Set up CSX points for capture and injection. Refer to CSX documentation
*  for more informatin.
*
*  @return
*     0        Success
*     -ve      Error code
*/
typedef int (*HALAUDIO_CODEC_SET_CSX_OP)(
   int                        chno, /**< (i) Codec channel number */
   HALAUDIO_CSX_POINT_ID      point,   /**< (i) Point ID to install the CSX point */
   const CSX_IO_POINT_FNCS   *fncp, /**< (i) Ptr to CSX callbacks */
   void                      *data  /**< (i) User data to pass back to callbacks */
);

/**
* HAL Audio Codec structure of operations. Not all operations are
* mandatory. However, the prepare, enable, and disable methods are
* mandatory.
*/
typedef struct halaudio_codec_ops
{
   HALAUDIO_CODEC_SET_FREQ_OP       setfreq;       /* Set sampling frequency */
   HALAUDIO_CODEC_GET_FREQ_OP       getfreq;       /* Get sampling frequency */
   HALAUDIO_CODEC_SET_ANA_OP        setana;        /* Set analog (PGA) gains */
   HALAUDIO_CODEC_GET_ANA_OP        getana;        /* Get analog (PGA) gains */
   HALAUDIO_CODEC_SET_DIG_OP        setdig;        /* Set digital gains */
   HALAUDIO_CODEC_GET_DIG_OP        getdig;        /* Get digital gains */
   HALAUDIO_CODEC_SET_SIDETONE_OP   setsidetone;   /* Set sidetone gain */
   HALAUDIO_CODEC_GET_SIDETONE_OP   getsidetone;   /* Get sidetone gain */
   HALAUDIO_CODEC_SET_EQU_OP        setequ;        /* Set equalizer parameters */
   HALAUDIO_CODEC_GET_EQU_OP        getequ;        /* Get equalizer parameters */
   HALAUDIO_CODEC_READ_OP           read;          /* Read digital samples from ADC */
   HALAUDIO_CODEC_WRITE_OP          write;         /* Write digital samples to DAC */
   HALAUDIO_CODEC_GET_INFO_OP       info;          /* Retrieve codec information */
   HALAUDIO_CODEC_SET_CSX_OP        setcsx;        /* Set CSX inject and capture points */
}
HALAUDIO_CODEC_OPS;

/***************************************************************************/
/**
*  HAL Audio Interface operations.
*
*  HAL Audio Interfaces model hardware interfaces, such as PCM or I2S, that
*  provide one or more audio codec channels. The HAL Audio Interface
*  abstraction is flexible and allows physical hardware interfaces to be
*  broken into multiple HAL Audio Interfaces. For example, the hardware may
*  support two hardware PCM channels, each supporting up to 4 time slots.
*  Each time slot is an audio codec channel. In this example, the hardware
*  PCM channel maps well into an HAL Audio Interface. Another example is
*  I2S. Suppose there are two hardware I2S channels, each supporting
*  stereo audio codec channels. Each hardware I2S channel may
*  themselves be an HAL Audio Interface, or both I2S channels may be
*  represented by a single HAL Audio Interface, where each stereo
*  channel is an HAL Audio codec channel. The point is that an HAL Audio
*  Interface is a flexible abstraction for hardware interfaces. The mapping
*  should be based on how well the underlying hardware suits the
*  abstraction model.
*/

/**
*  This callback is used to synchronize audio interfaces. It is
*  registered with each audio interface and is expected to be called
*  once per frame tick (by each interface regardless of the number
*  of codec channels the interface has).
*/
typedef void (*HALAUDIO_IF_FRAME_ELAPSED)( void *data );

/***************************************************************************/
/**
*  Audio interface constructor
*/
typedef int  (*HALAUDIO_IF_INIT_OP)(
   HALAUDIO_IF_FRAME_ELAPSED isrcb, /**< (i) Callback to indicate when a frame tick has elapsed */
   void                     *data   /**< (i) User data */
);

/***************************************************************************/
/**
*  Audio interface destructor
*/
typedef int (*HALAUDIO_IF_EXIT_OP)( void );

/***************************************************************************/
/**
*  Enable an interface. Interrupts are enabled and expected to fire
*  after calling this method.
*
*  This callback runs in an ATOMIC context and must NEVER block.
*
*  @return
*     0        Success
*     -ve      Error code
*/
typedef int (*HALAUDIO_IF_ENABLE_OP)( void );

/***************************************************************************/
/**
*  Prepare an interface before enabling. This operation is allowed
*  to block and may configure and allocated such things as hardware 
*  parameters, and DMA resources.
*
*  @return
*     0        Success
*     -ve      Error code
*/
typedef int (*HALAUDIO_IF_PREPARE_OP)( void );

/***************************************************************************/
/**
*  Disable an interface. Interrupts and resources associated with the 
*  interface will be stopped and released.
*
*  @return
*     0        Success
*     -ve      Error code
*/
typedef int (*HALAUDIO_IF_DISABLE_OP)( void );

/***************************************************************************/
/**
*  Power down analog hardware for the interface
*
*  @return
*     0        Success
*     -ve      Error code
*/
typedef int (*HALAUDIO_IF_ANA_POWERDN_OP)(
   int powerdn                      /**< (i) 1 to power down, 0 to power up */
);

/***************************************************************************/
/**
*  Shutdown interface for power management.
*
*  @return
*     0        Success
*     -ve      Error code
*
*  @remarks
*     The driver may use this method to shutdown hardware to
*     achieve the lowest possible power state. For example, hardware clocks
*     may be taken offline by this method. The HAL Audio framework will
*     not make use of the driver while in shutdown state.
*/
typedef int (*HALAUDIO_IF_PM_SHUTDOWN)( void );

/***************************************************************************/
/**
*  Resume interface for power management.
*
*  @return
*     0        Success
*     -ve      Error code
*
*  @remarks
*     The driver may use this method to restore hardware after a full
*     shutdown.
*/
typedef int (*HALAUDIO_IF_PM_RESUME)( void );

/**
* HAL Audio Interface structure of operations
*/
typedef struct halaudio_if_ops
{
   HALAUDIO_IF_INIT_OP              init;          /* Initialize entire interface */
   HALAUDIO_IF_EXIT_OP              exit;          /* Terminate entire interface */
   HALAUDIO_IF_PREPARE_OP           prepare;       /* Prepare interface before enabling */
   HALAUDIO_IF_ENABLE_OP            enable;        /* Enable interface. WARNING: Runs in atomic context. Must not block! */
   HALAUDIO_IF_DISABLE_OP           disable;       /* Disable interface */
   HALAUDIO_IF_ANA_POWERDN_OP       ana_powerdn;   /* Power down analog for interface */
   HALAUDIO_IF_PM_SHUTDOWN          pm_shutdown;   /* Shutdown interface for power management */
   HALAUDIO_IF_PM_RESUME            pm_resume;     /* Resume interface after shutdown for power management */

   HALAUDIO_CODEC_OPS               codec_ops;     /* Default codec ops */
}
HALAUDIO_IF_OPS;

/**
*  Structure of platform extension callbacks. These operations allow platform
*  specific operations to be hooked into the upper layer of the core HAL Audio
*  driver. For example, these callbacks are useful for hooking in GPIO
*  functionality to control external op-amps or LEDs.
*/
typedef struct halaudio_extensions_ops
{
   int (*setgain)( HALAUDIO_BLOCK block, int db);  /* Set analog (PGA) gains */
}
HALAUDIO_EXTENSIONS_OPS;

/**
*  Structure of Linux power management specific callbacks. These operations
*  give the access to several HAL Audio functions related to power management
*  including power up/down, etc.
*/
typedef struct halaudio_pm_ops
{
   int (*suspend)( void );  /* achieve lowest power state by setting power level to deep sleep and disabling clocks */
   int (*resume)( void );   /* restore power level and re-enable clocks */
}
HALAUDIO_PM_OPS;

/* HAL Audio Interface handle type */
typedef void *HALAUDIO_IF_HDL;

/**
*  Structure containing function pointer types for the HAL Audio API. This
*  structure is used to install a proprietary implementation of the HAL 
*  Audio framework.
*
*  The function pointer type must match the parameters of the actual API.
*  Refer to the API function prototypes for the explanation of the 
*  parameters and description of the API.
*/
typedef struct halaudio_api_funcs
{
   /* Common kernel and user API */
   HALAUDIO_HDL (*allocateClient)( void );
   int (*freeClient)( HALAUDIO_HDL );
   int (*queryInterfaceByName)( HALAUDIO_HDL, const char *, HALAUDIO_IF * );
   int (*queryCodecByName)( HALAUDIO_HDL, const char *, HALAUDIO_CODEC * );
   int (*lockCodec)( HALAUDIO_HDL, HALAUDIO_CODEC );
   int (*unlockCodec)( HALAUDIO_HDL, HALAUDIO_CODEC );
   int (*getGainInfo)( HALAUDIO_HDL, HALAUDIO_BLOCK, HALAUDIO_GAIN_INFO * );
   int (*setGain)( HALAUDIO_HDL, HALAUDIO_BLOCK, int );
   int (*setPower)( HALAUDIO_HDL, HALAUDIO_POWER_LEVEL );
   int (*getPower)( HALAUDIO_HDL, HALAUDIO_POWER_LEVEL * );
   int (*write)( HALAUDIO_HDL, HALAUDIO_CODEC, HALAUDIO_FMT, const uint8_t *, int );
   int (*read)( HALAUDIO_HDL, HALAUDIO_CODEC, HALAUDIO_FMT, uint8_t *, int );
   int (*setSuperUser)( HALAUDIO_HDL, int );
   int (*setFreq)( HALAUDIO_HDL, HALAUDIO_CODEC, int );
   int (*getFreq)( HALAUDIO_HDL, HALAUDIO_CODEC, int * );
   int (*setEquParms)( HALAUDIO_HDL, HALAUDIO_CODEC, HALAUDIO_DIR, const HALAUDIO_EQU * );
   int (*getEquParms)( HALAUDIO_HDL, HALAUDIO_CODEC, HALAUDIO_DIR, HALAUDIO_EQU * );
   int (*getHardwareInfo)( HALAUDIO_HDL, HALAUDIO_HW_INFO * );
   int (*getInterfaceInfo)( HALAUDIO_HDL, HALAUDIO_IF, HALAUDIO_IF_INFO * );
   int (*getCodecInfo)( HALAUDIO_HDL, HALAUDIO_CODEC, HALAUDIO_CODEC_INFO * );
   int (*enableInterface)( HALAUDIO_HDL, HALAUDIO_IF, int );

   /* Kernel specific API */
   int (*addInterface)( HALAUDIO_IF_OPS *, unsigned int, const char *, int, int, HALAUDIO_IF_HDL * );
   int (*delInterface)( HALAUDIO_IF_HDL );
   int (*setSyncFlag)( HALAUDIO_IF_HDL, int );
   int (*setCsxIoPoints)( HALAUDIO_CODEC, HALAUDIO_CSX_POINT_ID, const CSX_IO_POINT_FNCS *, void * );
   int (*setCsxFrameSync)( CSX_FRAME_SYNC_FP, void * );
}
HALAUDIO_API_FUNCS;

/* MakeDefs: On */

#endif   /* __KERNEL__ */

/* ---- Variable Externs ------------------------------------------ */

#if defined( __KERNEL__ )

/* Controls the amount of delay in msec before going to deep sleep. */
extern int gHalAudioSleepDelayMsec;

#endif   /* __KERNEL__ */

/* ---- Function Prototypes --------------------------------------- */

#ifdef __cplusplus
extern "C"
{
#endif

#if !defined( SWIG ) && !defined( MAKEDEFS )
/***************************************************************************/
/**
*  Allocate a client handle to make use of HAL Audio
*
*  @return for user applications
*     >= 0     Client handle
*     -1       Failed to open driver. 
*
*  @return for kernel applications
*     != NULL  Successfully opened driver handle
*     NULL     Failed to open driver handle
*/
HALAUDIO_HDL halAudioAllocateClient( void );

/***************************************************************************/
/**
*  Free client handle
*
*  @return
*     0        Success
*     -ve      Failure code
*/
int halAudioFreeClient(
   HALAUDIO_HDL client_hdl          /**< (i) Client handle */
);

/***************************************************************************/
/**
*  Obtain audio interface ID by name string
*
*  @return
*     0        Success
*     -ve      Failure code
*/
int halAudioQueryInterfaceByName(
   HALAUDIO_HDL client_hdl,         /**< (i) Client handle */
   const char  *name,               /**< (i) Name of interface */
   HALAUDIO_IF *id                  /**< (i) Ptr to store found interface ID */
);

/***************************************************************************/
/**
*  Obtain audio codec channel ID by name string
*
*  @return
*     0        Success
*     -ve      Failure code
*/
int halAudioQueryCodecByName(
   HALAUDIO_HDL    client_hdl,      /**< (i) Client handle */
   const char     *name,            /**< (i) Name of codec channel */
   HALAUDIO_CODEC *id               /**< (i) Ptr to store found codec ID */
);

/***************************************************************************/
/**
*  Acquire lock for an audio codec channel. Locking a codec will prevent
*  other clients from modifying any of its hardware parameters, such as
*  gains, equalizer parameters, and sampling frequency. Locking a channel
*  is not mandatory to make use of it. A user can make use of a channel
*  that is in an unlocked state; however, other clients are equally free
*  to modify the codec's hardware properties.
*
*  @return
*     0        Success
*     -EBUSY   Codec is already locked by another client, try again later.
*     -ENODEV  Non-existent codec or client
*
*  @remarks
*     Client can safely lock the same codec multiple times.
*/
int halAudioLockCodec(
   HALAUDIO_HDL    client_hdl,      /**< (i) Client handle */
   HALAUDIO_CODEC  cid              /**< (i) Codec to lock */
);

/***************************************************************************/
/**
*  Acquire lock for an audio codec channel by name. Refer to 
*  halAudioLockCodec for more information.
*
*  @return
*     0        Success
*     -EBUSY   Codec is already locked by another client, try again later.
*     -ENODEV  Non-existent codec or client
*/
static inline int halAudioLockCodecByName(
   HALAUDIO_HDL    client_hdl,      /**< (i) Client handle */
   const char     *name             /**< (i) Name of codec to lock */
)
{
   HALAUDIO_CODEC cid;
   int            err;
   err = halAudioQueryCodecByName( client_hdl, name, &cid );
   if ( err )
   {
      return err;
   }
   return halAudioLockCodec( client_hdl, cid );
}

/***************************************************************************/
/**
*  Release lock for an audio codec channel.
*
*  @return
*     0        Success
*     -EINVAL  Non-existent codec, client does not hold the lock, or codec
*              is already unlocked.
*     -ENODEV  Non-existing codec
*/
int halAudioUnlockCodec(
   HALAUDIO_HDL    client_hdl,      /**< (i) Client handle */
   HALAUDIO_CODEC  cid              /**< (i) Codec ID to unlock */
);

/***************************************************************************/
/**
*  Release lock for an audio codec channel by name.
*
*  @return
*     0        Success
*     -EINVAL  Non-existent codec, client does not hold the lock, or codec
*              is already unlocked.
*     -ENODEV  Non-existing codec
*/
static inline int halAudioUnlockCodecByName(
   HALAUDIO_HDL    client_hdl,      /**< (i) Client handle */
   const char     *name             /**< (i) Name of code to unlock */
)
{
   HALAUDIO_CODEC cid;
   int            err;
   err = halAudioQueryCodecByName( client_hdl, name, &cid );
   if ( err )
   {
      return err;
   }
   return halAudioUnlockCodec( client_hdl, cid );
}

/***************************************************************************/
/**
*  To query gain information for a particular analog or digital gain block.
*
*  @return
*     0        Success
*     -EINVAL  Invalid parameters
*     -EPERM   Parent interface is disabled
*     -ve      Failure code
*/
int halAudioGetGainInfo(
   HALAUDIO_HDL        client_hdl,  /**< (i) Client handle */
   HALAUDIO_BLOCK      block,       /**< (i) Block id for digital or analog gain */
   HALAUDIO_GAIN_INFO *info         /**< (o) Pointer to returned gain information */
);

/***************************************************************************/
/**
*  To set gain for a particular analog or digital gain block.
*
*  @return
*     0        Success
*     -EBUSY   Codec is locked by another user
*     -ENODEV  Non-existing codec
*     -EPERM   Parent interface is disabled
*     -ve      Failure code
*/
int halAudioSetGain(
   HALAUDIO_HDL    client_hdl,      /**< (i) Client handle */
   HALAUDIO_BLOCK  block,           /**< (i) Block id for digital or analog gain */
   int             db               /**< (i) Gain in dB to set */
);

/***************************************************************************/
/**
*  To adjust gain for a particular analog or digital gain block for any
*  number of unitless notches. The routine abstracts the knowledge of the
*  precise gain settings and ranges for the user.
*
*  @return
*     1        Gain level either reach the max or min setting
*     0        Success
*     -EINVAL  Invalid gain block
*     -ENODEV  Non-existing codec
*     -EPERM   Parent interface is disabled
*     -EBUSY   Codec is locked by another user
*/
int halAudioAlterGain(
   HALAUDIO_HDL    client_hdl,      /**< (i) Client handle */
   HALAUDIO_BLOCK  block,           /**< (i) Block id for digital or analog gain */
   int             delta            /**< (i) Number of notches to adjust gain by in unitless notches */
);

/***************************************************************************/
/**
*  Set power level. The actual power level takes into consideration requests
*  from all clients with a bias for power to be enabled. For example, deep
*  sleep only occurs if there is consensus with all clients. In contrast,
*  active power level is engaged even if only one client needs power.
*
*  @return
*     0        Success
*     -EINVAL  Invalid power level
*/
int halAudioSetPower(
   HALAUDIO_HDL         client_hdl, /**< (i) Client handle */
   HALAUDIO_POWER_LEVEL level       /**< (i) Power level request */
);

/***************************************************************************/
/**
*  Query actual power level.
*
*  @return
*     0        Success
*     -ve      Error code
*/
int halAudioGetPower(
   HALAUDIO_HDL         client_hdl, /**< (i) Client handle */
   HALAUDIO_POWER_LEVEL *level      /**< (o) Pointer to store power level */
);

/***************************************************************************/
/**
*  Write audio samples directly to an audio channel's hardware buffers.
*
*  @return
*     >= 0     Success, returns the number of bytes written
*     -EBUSY   Codec is already locked by other client, try again later.
*     -EPERM   Parent interface is disabled
*     -ve      Error code
*/
int halAudioWrite(
   HALAUDIO_HDL   client_hdl,       /**< (i) Client handle */
   HALAUDIO_CODEC cid,              /**< (i) Codec channel to write to */
   HALAUDIO_FMT   format,           /**< (i) Format of samples */
   const uint8_t *audio,            /**< (i) Ptr to data buffer to playback */
   int            bytes             /**< (i) Number of bytes contained in buffer */
);

/***************************************************************************/
/**
*  Write audio samples directly to an audio channel's hardware buffers
*  by codec's name.
*
*  @return
*     >= 0     Success, returns the number of bytes written
*     -EBUSY   Codec is already locked by other client, try again later.
*     -EPERM   Parent interface is disabled
*     -ve      Error code
*/
static inline int halAudioWriteByName(
   HALAUDIO_HDL   client_hdl,       /**< (i) Client handle */
   const char    *name,             /**< (i) Name of codec channel to write to */
   HALAUDIO_FMT   format,           /**< (i) Format of samples */
   const uint8_t *audio,            /**< (i) Ptr to data buffer to playback */
   int            bytes             /**< (i) Number of bytes contained in buffer */
)
{
   HALAUDIO_CODEC cid;
   int            err;
   err = halAudioQueryCodecByName( client_hdl, name, &cid );
   if ( err )
   {
      return err;
   }
   return halAudioWrite( client_hdl, cid, format, audio, bytes );
}

/***************************************************************************/
/**
*  Read audio samples directly from an audio channel's hardware buffers.
*
*  @return
*     >= 0     Number of bytes read
*     -EBUSY   Codec is already locked by other client, try again later.
*     -EPERM   Parent interface is disabled
*     -ve      Error code
*/
int halAudioRead(
   HALAUDIO_HDL   client_hdl,       /**< (i) Client handle */
   HALAUDIO_CODEC cid,              /**< (i) Codec channel to write to */
   HALAUDIO_FMT   format,           /**< (i) Format of samples */
   uint8_t       *audio,            /**< (o) Ptr to data buffer to record to */
   int            bytes             /**< (i) Max number of bytes to read */
);

/***************************************************************************/
/**
*  Read audio samples directly from an audio channel's hardware buffers
*  by codec's name.
*
*  @return
*     >= 0     Number of bytes read
*     -EBUSY   Codec is already locked by other client, try again later.
*     -EPERM   Parent interface is disabled
*     -ve      Error code
*/
static inline int halAudioReadByName(
   HALAUDIO_HDL   client_hdl,       /**< (i) Client handle */
   const char    *name,             /**< (i) Name of codec channel */
   HALAUDIO_FMT   format,           /**< (i) Format of samples */
   uint8_t       *audio,            /**< (o) Ptr to data buffer to record to */
   int            bytes             /**< (i) Max number of bytes to read */
)
{
   HALAUDIO_CODEC cid;
   int            err;
   err = halAudioQueryCodecByName( client_hdl, name, &cid );
   if ( err )
   {
      return err;
   }
   return halAudioRead( client_hdl, cid, format, audio, bytes );
}

/***************************************************************************/
/**
*  Enable and disable super user privileges. A super user is permitted
*  to modify any codec channel property regardless if the codec is in
*  a locked or unlocked state.
*
*  @return
*     0        Success
*     -1       Super user privileges not permitted
*/
int halAudioSetSuperUser(
   HALAUDIO_HDL   client_hdl,       /**< (i) Client handle */
   int            enable            /**< (i) 1 to enable super user, 0 to disable */
);

/***************************************************************************/
/**
*  Set sampling frequency for an audio codec channel
*
*  @return
*     0        Success
*     -EBUSY   Codec is already locked by other client, try again later.
*     -EINVAL  Invalid sampling frequency to set, or invalid codec
*     -EPERM   Parent interface is disabled
*
*  @remarks
*     Changing the sampling frequency while a codec is running may produce
*     an audible glitch. To minimize audible glitches, it is recommended
*     that the the analog gains be lowered prior to switching the sampling
*     frequency.
*/
int halAudioSetFreq(
   HALAUDIO_HDL   client_hdl,       /**< (i) Client handle */
   HALAUDIO_CODEC cid,              /**< (i) Codec channel */
   int            freqhz            /**< (i) Sampling frequency in Hz to set */
);

/***************************************************************************/
/**
*  Set sampling frequency for an audio codec channel by name. Refer to
*  halAudioSetFreq for more information.
*
*  @return
*     0        Success
*     -EBUSY   Codec is already locked by other client, try again later.
*     -EINVAL  Invalid sampling frequency to set, or invalid codec
*     -EPERM   Parent interface is disabled
*/
static inline int halAudioSetFreqByName(
   HALAUDIO_HDL   client_hdl,       /**< (i) Client handle */
   const char    *name,             /**< (i) Name of codec channel */
   int            freqhz            /**< (i) Sampling frequency in Hz to set */
)
{
   HALAUDIO_CODEC cid;
   int            err;
   err = halAudioQueryCodecByName( client_hdl, name, &cid );
   if ( err )
   {
      return err;
   }
   return halAudioSetFreq( client_hdl, cid, freqhz );
}

/***************************************************************************/
/**
*  Query sampling frequency for an audio codec channel
*
*  @return
*     0        Success
*     -EINVAL  Invalid sampling frequency to set, or invalid codec
*     -EPERM   Parent interface is disabled
*/
int halAudioGetFreq(
   HALAUDIO_HDL   client_hdl,       /**< (i) Client handle */
   HALAUDIO_CODEC cid,              /**< (i) Codec channel */
   int           *freqhz            /**< (o) Ptr to store sampling freq */
);

/***************************************************************************/
/**
*  Query sampling frequency for an audio codec channel by name
*
*  @return
*     0        Success
*     -EINVAL  Invalid sampling frequency to set, or invalid codec
*     -EPERM   Parent interface is disabled
*/
static inline int halAudioGetFreqByName(
   HALAUDIO_HDL   client_hdl,       /**< (i) Client handle */
   const char    *name,             /**< (i) Name of codec channel */
   int           *freqhz            /**< (o) Ptr to store sampling freq */
)
{
   HALAUDIO_CODEC cid;
   int            err;
   err = halAudioQueryCodecByName( client_hdl, name, &cid );
   if ( err )
   {
      return err;
   }
   return halAudioGetFreq( client_hdl, cid, freqhz );
}

/***************************************************************************/
/**
*  Set equalizer parameters for a particular audio codec channel
*
*  @return
*     0        Success
*     -EBUSY   Codec is already locked by other client, try again later.
*     -EINVAL  Invalid parameters
*     -EPERM   Parent interface is disabled
*/
int halAudioSetEquParms(
   HALAUDIO_HDL         client_hdl, /**< (i) Client handle */
   HALAUDIO_CODEC       cid,        /**< (i) Codec channel */
   HALAUDIO_DIR         dir,        /**< (i) Select equalizer direction */
   const HALAUDIO_EQU  *equ         /**< (i) Pointer to equalizer parameters */
);

/***************************************************************************/
/**
*  Set equalizer parameters for a particular audio codec channel by name.
*
*  @return
*     0        Success
*     -EBUSY   Codec is already locked by other client, try again later.
*     -EINVAL  Invalid parameters
*     -EPERM   Parent interface is disabled
*/
static inline int halAudioSetEquParmsByName(
   HALAUDIO_HDL         client_hdl, /**< (i) Client handle */
   const char          *name,       /**< (i) Name of codec */
   HALAUDIO_DIR         dir,        /**< (i) Select equalizer direction */
   const HALAUDIO_EQU  *equ         /**< (i) Pointer to equalizer parameters */
)
{
   HALAUDIO_CODEC cid;
   int            err;
   err = halAudioQueryCodecByName( client_hdl, name, &cid );
   if ( err )
   {
      return err;
   }
   return halAudioSetEquParms( client_hdl, cid, dir, equ );
}

/***************************************************************************/
/**
*  Query equalizer parameters for a particular audio codec channel
*
*  @return
*     0        Success
*     -EINVAL  Invalid parameters
*     -EPERM   Parent interface is disabled
*/
int halAudioGetEquParms(
   HALAUDIO_HDL         client_hdl, /**< (i) Client handle */
   HALAUDIO_CODEC       cid,        /**< (i) Codec channel */
   HALAUDIO_DIR         dir,        /**< (i) Select equalizer direction */
   HALAUDIO_EQU        *equ         /**< (o) Ptr to store equalizer parameters */
);

/***************************************************************************/
/**
*  Query equalizer parameters for a particular audio codec channel by name.
*
*  @return
*     0        Success
*     -EINVAL  Invalid parameters
*     -EPERM   Parent interface is disabled
*/
static inline int halAudioGetEquParmsByName(
   HALAUDIO_HDL         client_hdl, /**< (i) Client handle */
   const char          *name,       /**< (i) Name of codec channel */
   HALAUDIO_DIR         dir,        /**< (i) Select equalizer direction */
   HALAUDIO_EQU        *equ         /**< (o) Ptr to store equalizer parameters */
)
{
   HALAUDIO_CODEC cid;
   int            err;
   err = halAudioQueryCodecByName( client_hdl, name, &cid );
   if ( err )
   {
      return err;
   }
   return halAudioGetEquParms( client_hdl, cid, dir, equ );
}

/***************************************************************************/
/**
*  Query high-level hardware information. To get detailed sub-component
*  information call halAudioGetInterfaceInfo, halAudioGetCodecInfo and
*  halAudioGetGainInfo.
*
*  @return
*     0        Success
*     -EINVAL  Invalid parameters
*/
int halAudioGetHardwareInfo(
   HALAUDIO_HDL       client_hdl,   /**< (i) Client handle */
   HALAUDIO_HW_INFO  *info          /**< (o) Ptr to store hardware info */
);

/***************************************************************************/
/**
*  Query audio interface information.
*
*  @return
*     0        Success
*     -EINVAL  Invalid parameters
*/
int halAudioGetInterfaceInfo(
   HALAUDIO_HDL         client_hdl, /**< (i) Client handle */
   HALAUDIO_IF          id,         /**< (i) Interface id */
   HALAUDIO_IF_INFO    *info        /**< (o) Ptr to store interface info */
);

/***************************************************************************/
/**
*  Query audio interface information by name
*
*  @return
*     0        Success
*     -EINVAL  Invalid parameters
*/
static inline int halAudioGetInterfaceInfoByName(
   HALAUDIO_HDL         client_hdl, /**< (i) Client handle */
   const char          *name,       /**< (i) Interface name */
   HALAUDIO_IF_INFO    *info        /**< (o) Ptr to store interface info */
)
{
   HALAUDIO_IF id;
   int         err;
   err = halAudioQueryInterfaceByName( client_hdl, name, &id );
   if ( err )
   {
      return err;
   }
   return halAudioGetInterfaceInfo( client_hdl, id, info );
}

/***************************************************************************/
/**
*  Query audio codec information.
*
*  @return
*     0        Success
*     -EINVAL  Invalid parameters
*     -EPERM   Parent interface is disabled
*/
int halAudioGetCodecInfo(
   HALAUDIO_HDL         client_hdl, /**< (i) Client handle */
   HALAUDIO_CODEC       cid,        /**< (i) Codec channel */
   HALAUDIO_CODEC_INFO *info        /**< (o) Ptr to store codec info */
);

/***************************************************************************/
/**
*  Query audio codec information by name
*
*  @return
*     0        Success
*     -EINVAL  Invalid parameters
*     -EPERM   Parent interface is disabled
*/
static inline int halAudioGetCodecInfoByName(
   HALAUDIO_HDL         client_hdl, /**< (i) Client handle */
   const char          *name,       /**< (i) Name of codec channel */
   HALAUDIO_CODEC_INFO *info        /**< (o) Ptr to store codec info */
)
{
   HALAUDIO_CODEC cid;
   int            err;
   err = halAudioQueryCodecByName( client_hdl, name, &cid );
   if ( err )
   {
      return err;
   }
   return halAudioGetCodecInfo( client_hdl, cid, info );
}

/***************************************************************************/
/**
*  Enable an audio interface. By default, audio interfaces are all
*  enabled at start up. However, in some cases, it is necessary or
*  desirable to disable an interface and re-enable at a later time.
*  For example, if interface is slaved to an external clock and that
*  clock needs to power down, it is necessary to disable the interface
*  first to ensure that synchronization between interfaces does not
*  get out of phase.
*
*  @return
*     0        Success
*     -EINVAL  Invalid parameters
*/
int halAudioEnableInterface(
   HALAUDIO_HDL         client_hdl, /**< (i) Client handle */
   HALAUDIO_IF          id,         /**< (i) Interface number */
   int                  enable      /**< (i) 1 to enable, 0 to disable */
);

/***************************************************************************/
/**
*  Enable an audio interface by name. Refer to halAudioEnableInterface
*  for more information.
*
*  @return
*     0        Success
*     -EINVAL  Invalid parameters
*/
static inline int halAudioEnableInterfaceByName(
   HALAUDIO_HDL         client_hdl, /**< (i) Client handle */
   const char          *name,       /**< (i) Interface name */
   int                  enable      /**< (i) 1 to enable, 0 to disable */
)
{
   HALAUDIO_IF id;
   int         err;
   err = halAudioQueryInterfaceByName( client_hdl, name, &id );
   if ( err )
   {
      return err;
   }
   return halAudioEnableInterface( client_hdl, id, enable );
}

#ifdef __cplusplus
}
#endif



#if defined( __KERNEL__ )

/***************************************************************************/
/**
*  Add an HAL Audio Interface
*
*  @return
*     0        success
*     -ve      Error code such as -EFAULT, -ENOMEM, -EINVAL
*
*  @remarks
*     Interfaces by default will be synchronized by HAL Audio based on
*     their interrupt periodicity. However, some interfaces may choose
*     not to synchronize with other interfaces because of timing
*     limitations or other reasons. To opt out of synchronization, set
*     the frame period to 0.
*/
int halAudioAddInterface(
   HALAUDIO_IF_OPS  *ops,           /**< (i) Interface operations */
   unsigned int      codecs,        /**< (i) Total number of audio codec channels */
   const char       *name,          /**< (i) Name string */
   int               frame_usec,    /**< (i) Interrupt period in usec. 0 means not synchronized */
   int               sync,          /*<< (i) Requests to synchronize with other interfaces */
   HALAUDIO_IF_HDL  *hdlp           /**< (o) Ptr to store handle */
);

/***************************************************************************/
/**
*  Delete HAL Audio Interface. All associated audio codec channels are
*  cleaned up as part of deleting the interface.
*
*  @return
*     0        Success
*     -1       Failed to delete interface, may have orphaned audio codec
*              channel resources
*     -ve      Other error codes
*/
int halAudioDelInterface(
   HALAUDIO_IF_HDL handle           /**< (i) interface to delete */
);

/***************************************************************************/
/**
*  Changes an interface's synchronization behavior. Synchronization refers
*  to timing with other interfaces. All synchronized interfaces are expected
*  to start at the same time and execute at similar sampling frequencies.
*  Unsynchronized interfaces are free to execute at any sampling frequencies
*  however the their clocks do not contribute to the HAL Audio wall clock.
*
*  @return
*     0        on success
*     -1       Failed, may have orphan codec channels
*     -ve      Other error codes
*
*  @remarks
*/
int halAudioSetSyncFlag(
   HALAUDIO_IF_HDL   handle,        /*<< (i) interface to delete */
   int               sync           /*<< (i) 1 requests to sync with other interfaces */
);

/***************************************************************************/
/**
*  Set CSX IO callbacks for debug infrastructure to support audio inject and
*  capture points.
*
*  @return
*     0        Success
*     -EAGAIN  Failed to acquire resource
*     -ve      Other error codes
*/
int halAudioSetCsxIoPoints(
   HALAUDIO_CODEC           cid,    /**< (i) Codec to install CSX callbacks */
   HALAUDIO_CSX_POINT_ID    point,  /**< (i) Point ID selected for CSX callbacks */
   const CSX_IO_POINT_FNCS *fncp,   /**< (i) List of callbacks */
   void                    *data    /**< (i) User data */
);

/***************************************************************************/
/**
*  Set CSX frame sync callback for debug infrastructure to support alignment
*  of audio inject and capture operations.
*
*  @return
*     0        Success
*     -ve      Other error codes
*/
int halAudioSetCsxFrameSync(
   CSX_FRAME_SYNC_FP fncp,    /**< (i) Frame sync callback */
   void              *data    /**< (i) User data */
);

/***************************************************************************/
/**
*  Set platform extension operations. This routine is called to setup platform 
*  specific extensions.
*
*  @return Nothing
*/
void halAudioSetExtensionsOps(
   const HALAUDIO_EXTENSIONS_OPS *extensions_ops  /**< (in) Extensions ops */
);

/***************************************************************************/
/**
*  Register power management operations.
*
*  @return
*     0     - Success
*     -ve   - error code
*/
int halAudioSetPowerManageOps(
   HALAUDIO_PM_OPS *pm_ops          /**< (i) Power management callbacks 
                                             If NULL, clears registration */
);

/***************************************************************************/
/**
*  Setup proprietary HAL Audio framework API functions
*
*  @return    none
*/
void halAudioSetApiFuncs( 
   const HALAUDIO_API_FUNCS *funcs  /**< (i) Ptr to API functions. 
                                             If NULL, clears registration */
);

#endif   /* __KERNEL__ */
#endif   /* SWIG */

#endif /* HALAUDIO_H */
