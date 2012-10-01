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
*  @file    halaudio_cfg.h
*
*  @brief   Board dependant Hal Audio configurations
*
*****************************************************************************/
#if !defined( HALAUDIO_CFG_H )
#define HALAUDIO_CFG_H

/* ---- Constants and Types ---------------------------------------------- */
#define HALAUDIO_CFG_NAME_LEN    20

typedef struct halaudio_cfg_dev_dir
{
   int chans;                    /* Channels: 0=default, 1=mono, 2=stereo, etc. */
   int chidx;                    /* Channel index for mono channel selection */

   /* Following parameters are not used yet */
#if 0
   int ana_gain;                 /* Default analog gain setting */
   int dig_gain;                 /* Default digital gain setting */
   int equ_coefs_len;            /* Number of equalizer coefficients */
   int equ_coefs[HALAUDIO_EQU_COEFS_MAX_NUM]; /* Equ coefficients */
#endif

} HALAUDIO_CFG_DEV_DIR;

typedef struct halaudio_dev_info
{
   char codec_name[HALAUDIO_CFG_NAME_LEN]; /* codec channel name */
   char mport_name[HALAUDIO_CFG_NAME_LEN]; /* AMXR port name */
   HALAUDIO_HWSEL mic_hwsel;     /* mic mux */
   HALAUDIO_HWSEL spkr_hwsel;    /* speaker switch */

   int mic_ana_gain;             /* Default mic analog gain */
   int mic_dig_gain;             /* Default mic digital gain */
   int spkr_ana_gain;            /* Default speaker analog  gain */
   int spkr_dig_gain;            /* Default speaker digital gain */
   int sidetone_gain;            /* Default sidetone gain */
   int mic_equ_coefs_len;        /* Number of mic equalizer coefficients */
   int mic_equ_coefs[HALAUDIO_EQU_COEFS_MAX_NUM];
   int sprk_equ_coefs_len;       /* Number of speaker equalizer coefficients */
   int sprk_equ_coefs[HALAUDIO_EQU_COEFS_MAX_NUM];

   HALAUDIO_CFG_DEV_DIR mic;     /* Microphone cfg parameters */
   HALAUDIO_CFG_DEV_DIR spkr;    /* Speaker cfg parameters */

} HALAUDIO_DEV_INFO;

typedef struct halaudio_dev_cfg
{
   char name[HALAUDIO_CFG_NAME_LEN]; /* device name */
   int has_aux;                  /* indicate whether device has AUX pair or not */
   HALAUDIO_DEV_INFO info;       /* main device information */
   HALAUDIO_DEV_INFO aux_info;   /* AUX information */

} HALAUDIO_DEV_CFG;

typedef struct halaudio_cfg
{
   /* number of devices */
   int numdev;

   /* pointer to the list of devices */
   HALAUDIO_DEV_CFG *devlist;
} HALAUDIO_CFG;

/* ---- Variable Externs ------------------------------------------------- */

/* ---- Function Prototypes --------------------------------------- */

#endif /* HALAUDIO_CFG_H */
