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
*  @file    csx_halaudio.c
*
*  @brief   HalAudio CSX module.
*
****************************************************************************/

/* ---- Include Files ---------------------------------------------------- */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>

#include <linux/broadcom/halaudio.h>
#include <linux/broadcom/csx.h>
#include <linux/broadcom/csx_framework.h>
#include <linux/broadcom/gos/gos.h>

/* ---- Public Variables ------------------------------------------------- */
/* ---- Private Constants and Types -------------------------------------- */

#define CSX_HALAUDIO_SEMAPHORE_TIME_WAIT_MS        10

/* ---- Private Function Prototypes -------------------------------------- */

static int csx_halaudio_set_point( int csx_device_id,
                                   int csx_point_id,
                                   CSX_IO_POINT_FNCS *csx_io_point_fncs,
                                   void *csx_priv );

static int csx_halaudio_set_frame_sync( CSX_FRAME_SYNC_FP frame_sync_fncp );

/* ---- Private Variables ------------------------------------------------ */

static char banner[] __initdata = KERN_INFO "CSX HAL Audio: 1.00 (built on "__DATE__" "__TIME__")\n";

/* Module function pointers to register */
static CSX_MODULE_FNCS gModuleFncs =
{
   .csx_module_set_frame_sync = csx_halaudio_set_frame_sync,
   .csx_module_set_point = csx_halaudio_set_point,
};

static GOS_SEM csx_halaudio_sem;

/* HAL Audio specific information */
static int gNumCodecs;
static HALAUDIO_HDL gHalHdl;

/* ---- Functions -------------------------------------------------------- */

/***************************************************************************/
/**
*  Set a CSX I/O debug point with given device and point identification.
*
*  @return
*     0           - success
*     -ve         - failure code
*/
static int csx_halaudio_set_point( int csx_device_id,
                                   int csx_point_id,
                                   CSX_IO_POINT_FNCS *csx_io_point_fncs,
                                   void *csx_priv )
{
   int err;

   /*Validate device and point identificators */
   if ( csx_device_id < 0 || csx_device_id >= gNumCodecs )
   {
      return -EINVAL;
   }

   if ( csx_point_id != HALAUDIO_DIR_ADC && csx_point_id != HALAUDIO_DIR_DAC )
   {
      return -EINVAL;
   }

   err = gosSemTimedTake( csx_halaudio_sem, CSX_HALAUDIO_SEMAPHORE_TIME_WAIT_MS );
   if (!err)
   {
      err = halAudioSetCsxIoPoints( csx_device_id, csx_point_id, csx_io_point_fncs, csx_priv );
      gosSemGive( csx_halaudio_sem );
   }

   return err;
}

/***************************************************************************/
/**
*  Set CSX frame sync callback function pointer.
*
*  @return
*     0           - success
*     -ve         - failure code
*/
static int csx_halaudio_set_frame_sync( CSX_FRAME_SYNC_FP frame_sync_fncp )
{
   int err;
   err = halAudioSetCsxFrameSync( frame_sync_fncp, (void *)CSX_IO_MODULE_HALAUDIO );
   return err;
}

/***************************************************************************/
/**
*  Initialize csx_halaudio.  Gathers information from HAL audio module.
*
*  @return
*     0           - success
*     -ve         - failure code
*/
static int __init csx_halaudio_init( void )
{
   int err;
   HALAUDIO_HW_INFO hwinfo;

   printk( banner );

   err = gosSemAlloc( "csx_halaudio_sem", 1, &csx_halaudio_sem );
   if ( err )
   {
      gosSemFree( csx_halaudio_sem );
      return err;
   }

   gHalHdl = halAudioAllocateClient();

   if ( IS_ERR( gHalHdl ))
   {
      printk( KERN_ERR "%s: failed to allocate HAL Audio client err=%li\n", __FUNCTION__, PTR_ERR( gHalHdl ));
      return PTR_ERR( gHalHdl );
   }

   err = halAudioGetHardwareInfo( gHalHdl, &hwinfo );
   if ( err )
   {
      return err;
   }

   gNumCodecs = hwinfo.codecs;

   /* Register this module */
   err = csx_register_module( CSX_IO_MODULE_HALAUDIO, &gModuleFncs );

   return err;
}

/***************************************************************************/
/**
*  Clears all CSX I/O points and free the HAL Audio handle.
*
*  @return
*     0           - success
*     -ve         - failure code
*/
static void __exit csx_halaudio_exit( void )
{
   CSX_IO_POINT_FNCS csx_ops;
   int cid;

   /* Clear all CSX callbacks */
   memset( &csx_ops, 0, sizeof(csx_ops) );

   for ( cid = 0; cid < gNumCodecs; cid++ )
   {
      halAudioSetCsxIoPoints( cid, HALAUDIO_DIR_ADC, &csx_ops, NULL );
      halAudioSetCsxIoPoints( cid, HALAUDIO_DIR_DAC, &csx_ops, NULL );
   }

   /* Clear framesync callback */
   csx_halaudio_set_frame_sync( NULL );

   /* De-register ourselves from CSX framework */
   csx_deregister_module( CSX_IO_MODULE_HALAUDIO );

   /* Free the Hal Audio handle */
   halAudioFreeClient( gHalHdl );

   /* Free the semaphore */
   gosSemFree( csx_halaudio_sem );
}

module_init( csx_halaudio_init );
module_exit( csx_halaudio_exit );

MODULE_AUTHOR( "Broadcom" );
MODULE_DESCRIPTION( "CSX halaudio module" );
MODULE_LICENSE( "GPL v2" );

