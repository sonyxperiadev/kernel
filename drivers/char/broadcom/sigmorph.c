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
 *
 *  @file    sigmorph.c
 *
 *  @brief   SIGMORPH module
 *
 ****************************************************************************/

/* ---- Include Files ---------------------------------------------------- */

#include <linux/module.h>
#include <linux/types.h>

#include <linux/broadcom/sigmorph.h>
#include <linux/broadcom/halaudio.h>

/* ---- Private Constants and Types -------------------------------------- */

/* Fixed point Q value, and corresponding constant value */
#define SIGMORPH_FP_Q_VAL           (12)
#define SIGMORPH_FP_K_VAL           (1 << (SIGMORPH_FP_Q_VAL - 1))

/* Q12 gain map min and max ranges */
#define SIGMORPH_MAX_Q12_GAIN       (32536)
#define SIGMORPH_MIN_Q12_GAIN       (13)

/**
 * dB to 16-bit Q12 multiplier map. Used in setting software gains.
 * Ranges from -50dB to +18db (4096 = 0dB)
 */
const static int16_t gain_map_q12[] =
{
   13, 15, 16, 18, 21, 23, 26, 29, 33, 37,
   41, 46, 52, 58, 65, 73, 82, 92, 103, 115,
   130, 145, 163, 183, 205, 230, 258, 290, 325, 365,
   410, 460, 516, 579, 649, 728, 817, 917, 1029, 1154,
   1295, 1453, 1631, 1830, 2053, 2303, 2584, 2900, 3254, 3651,
   4096, 4596, 5157, 5786, 6492, 7284, 8173, 9170, 10289, 11544,
   12953, 14533, 16306, 18296, 20529, 23034, 25844, 28997, 32536
};

static char banner[] __initdata = KERN_INFO "BROADCOM Generic WB Signals module\n";

/* ---- Functions -------------------------------------------------------- */

inline static int16_t db2linear_q12( int16_t *db_gain /* (io) */)
{
   int16_t dbval = *db_gain;
   int16_t q12val;

   if( dbval > SIGMORPH_MAX_DB_GAIN )
   {
      *db_gain = SIGMORPH_MAX_DB_GAIN;
      q12val = SIGMORPH_MAX_Q12_GAIN;        /* maximum gain */
   }
   else if( dbval < SIGMORPH_MIN_DB_GAIN )
   {
      *db_gain = HALAUDIO_GAIN_MUTE;
      q12val = SIGMORPH_MIN_Q12_GAIN;        /* mute */
   }
   else
   {   
      /* Map db to mapped linear value */
      q12val = gain_map_q12[dbval - SIGMORPH_MIN_DB_GAIN];
   }
   return q12val;
}

/**
 * Input Arguments
 *    buf      - pointer to buffer to amplify
 *    bsize    - size of buffer
 *    gain     - SIGMORPH_FP_Q_VAL gain value
 *    
 * Output Arguments
 *    buf      - buffer to store result of amplification
 *    
 * @return 
 *    none
 */
int sigmorph_amplify( int16_t *buf, int bsize, int16_t db_gain )
{
   int i;
   int32_t temp = 0;
   int32_t length = bsize >> 1;
   int16_t q12_gain = db2linear_q12( &db_gain );
   
   for (i = 0; i < length; i++, buf++)
   {
      temp =  (int32_t)( *buf * q12_gain);   /* multiply with double precision */
//      temp += SIGMORPH_FP_K_VAL;           /* round up */
      temp = temp >> SIGMORPH_FP_Q_VAL;      /* correct by dividing by base */
      /* saturate result */
      if (temp > SIGMORPH_SATURATION_MAX)
      {
         *buf = SIGMORPH_SATURATION_MAX;
      }
      else if (temp < SIGMORPH_SATURATION_MIN)
      {
         *buf = SIGMORPH_SATURATION_MIN;
      } 
      else
      {
         *buf = temp;
      }
   }
   return 0;
}
EXPORT_SYMBOL ( sigmorph_amplify );

/**
 * Input Arguments
 *    output   - destination buffer
 *    op1      - operand 1 buffer
 *    op2      - operand 2 buffer
 *    size     - byte-length of buffers 
 * Output Arguments
 *    None
 *    
 * @return
 *    number of mixed bytes
 */
int sigmorph_mix ( int16_t *output, int16_t *op1,  int16_t *op2, int size )
{
   int i;
   int32_t temp = 0;

   for (i = 0; i < size; i++, output++, op1++, op2++)
   {
      temp = *op1 + *op2; 
      /* saturate result */ 
      if (temp > SIGMORPH_SATURATION_MAX)
      {
         *output = SIGMORPH_SATURATION_MAX;
      }
      else if (temp < SIGMORPH_SATURATION_MIN)
      {
         *output = SIGMORPH_SATURATION_MIN;
      } 
      else
      {
         *output = temp;
      }
   }
   return 0;
}
EXPORT_SYMBOL ( sigmorph_mix );

static int __init sigmorph_init( void )
{
   printk (banner);
   return 0;
}

static void __exit sigmorph_exit( void )
{
   /* exit */
}

module_init( sigmorph_init );
module_exit( sigmorph_exit );

MODULE_AUTHOR( "Broadcom" );
MODULE_DESCRIPTION( "CSX sigmorph module" );
MODULE_LICENSE( "GPL" );

