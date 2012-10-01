/*****************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
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
 *  sigmorph.h
 *
 *  PURPOSE:
 *     This file contains the interface to the Generic DSP utility
 *
 *  NOTES:
 *
 *****************************************************************************/

#if !defined( SIGMORPH_H )
#define SIGMORPH_H

/* ---- Include Files ---------------------------------------- */
/* ---- Constants and Types ---------------------------------- */

/* Sigmorph arithmetic ranges */
#define SIGMORPH_SATURATION_MAX     (0x7FFF)
#define SIGMORPH_SATURATION_MIN     (-SIGMORPH_SATURATION_MAX-1)

/* Q12 gain min and max ranges */
#define SIGMORPH_MAX_DB_GAIN        (18)
#define SIGMORPH_MIN_DB_GAIN        (-50)

#if defined( __KERNEL__ )
/* ---- Variable Externs ------------------------------------- */
/* ---- Function Prototypes ---------------------------------- */
extern int sigmorph_amplify( int16_t *buf_ptr, int buf_size, int16_t db_gain );
extern int sigmorph_mix ( int16_t *output, int16_t *op1,  int16_t *op2, int size );

#endif /* __KERNEL__ */
#endif /* SIGMORPH_H */
