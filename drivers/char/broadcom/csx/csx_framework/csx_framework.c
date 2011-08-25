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
*  @file    csx_framework.c
*
*  @brief   CSX framework source file
*
****************************************************************************/

/* ---- Include Files ---------------------------------------------------- */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/fs.h>

#include <linux/broadcom/csx_framework.h>

/* ---- Public Variables ------------------------------------------------- */
/* ---- Private Constants and Types -------------------------------------- */
/* ---- Private Variables ------------------------------------------------ */

static char banner[] __initdata = KERN_INFO "CSX: 1.00 (built on "__DATE__" "__TIME__")\n";

static CSX_MODULE_FNCS gModuleFncp[CSX_IO_MODULE_NUM_MAX];
static CSX_UTIL_FNCS gUtilFncp[CSX_IO_UTIL_NUM_MAX];

/* ---- Private Function Prototypes -------------------------------------- */

static int csx_frame_sync( void *data );

/* ---- Functions -------------------------------------------------------- */

/***************************************************************************/
/**
*  Frame sync function.  Performs a broadcast to all registered CSX I/O utilities
*  available.
*
*  @return
*     0           - success
*     -ve         - failure code
*/
static int csx_frame_sync( void *data )
{
   int i;

   /* Broadcast out frame sync to CSX utilities */
   for( i = 0; i < CSX_IO_UTIL_NUM_MAX; i++ )
   {
      if ( gUtilFncp[i].csx_util_frame_sync_event != NULL )
      {
         gUtilFncp[i].csx_util_frame_sync_event( (CSX_IO_MODULE)data );
      }
   }

   return 0;
}

/***************************************************************************/
/**
*  Get module function pointers from registered module with
*  specified module identifier
*
*  @return
*     Pointer to CSX module function pointers
*/
CSX_MODULE_FNCS *csx_get_module_fncp( CSX_IO_MODULE csx_module_id )
{
   return &gModuleFncp[csx_module_id];
}

/***************************************************************************/
/**
*  Register CSX I/O module
*
*  @return
*     0           - success
*     -ve         - failure code
*/
int csx_register_module( CSX_IO_MODULE csx_module_id, CSX_MODULE_FNCS *csx_module_fncp )
{
   /* Check if function pointers valid.  Requires at least a set_point function defined */
   if ( csx_module_fncp != NULL &&
        csx_module_fncp->csx_module_set_point != NULL )
   {
      /* Copy over function pointers and setup frame sync function */
      memcpy( &gModuleFncp[csx_module_id], csx_module_fncp, sizeof(CSX_MODULE_FNCS) );

      /* Register callback only if set_frame_sync function is defined */
      if ( csx_module_fncp->csx_module_set_frame_sync != NULL )
      {
         gModuleFncp[csx_module_id].csx_module_set_frame_sync( csx_frame_sync );
      }
   }
   else
   {
      return -EINVAL;
   }

   return 0;
}

/***************************************************************************/
/**
*  Register the CSX I/O Utility
*
*  @return
*     0           - success
*     -ve         - failure code
*/
int csx_register_util( CSX_IO_UTIL csx_util_id,
                       CSX_UTIL_FNCS *csx_util_fncp,
                       CSX_UTIL_CTRL_FNCS *csx_util_ctrl_fncp )
{
   /* Check if function pointers valid */
   if ( csx_util_fncp != NULL &&
        csx_util_fncp->csx_util_frame_sync_event != NULL )
   {
      /* Copy over function pointers */
      memcpy( &gUtilFncp[csx_util_id], csx_util_fncp, sizeof(CSX_UTIL_FNCS) );

      /* Register driver interface for utility */
      if ( csx_util_ctrl_fncp != NULL )
      {
         csx_register_util_drv( csx_util_id, csx_util_ctrl_fncp);
      }
   }

   return 0;
}

/***************************************************************************/
/**
*  De-register the CSX I/O Module
*
*  @return
*     0           - success
*     -ve         - failure code
*/
int csx_deregister_module( CSX_IO_MODULE csx_module_id )
{
   /* Clear out specified module function pointers */
   memset( &gModuleFncp[csx_module_id], 0, sizeof(CSX_MODULE_FNCS));
   return 0;
}

/***************************************************************************/
/**
*  De-register the CSX I/O Utility
*
*  @return
*     0           - success
*     -ve         - failure code
*/
int csx_deregister_util( CSX_IO_UTIL csx_util_id )
{
   /* Clear out I/O utility function pointers */
   csx_deregister_util_drv( csx_util_id);
   memset( &gUtilFncp[csx_util_id], 0, sizeof(CSX_UTIL_FNCS) );
   return 0;
}

/***************************************************************************/
/**
*  Initialize the CSX framework
*
*  @return
*     0           - success
*     -ve         - failure code
*/
static int __init csx_init( void )
{
   printk( banner );

   /* Ensure all function pointers are set to NULL on startup */
   memset( &gModuleFncp, 0, sizeof(CSX_MODULE_FNCS) * CSX_IO_MODULE_NUM_MAX );
   memset( &gUtilFncp, 0, sizeof(CSX_UTIL_FNCS) * CSX_IO_UTIL_NUM_MAX );

   return 0;
}

/***************************************************************************/
/**
*  Exit and cleanup the CSX framework
*
*  @return
*     0           - success
*     -ve         - failure code
*/
static void __exit csx_exit( void )
{
   int i;

   /* Clear out frame sync callbacks */
   for( i = 0; i < CSX_IO_UTIL_NUM_MAX; i++ )
   {
      if ( gModuleFncp[i].csx_module_set_frame_sync != NULL )
      {
         gModuleFncp[i].csx_module_set_frame_sync( NULL );
      }
   }
}

module_init( csx_init );
module_exit( csx_exit );

MODULE_AUTHOR( "Broadcom" );
MODULE_DESCRIPTION( "CSX framework module" );
MODULE_LICENSE( "GPL v2" );

EXPORT_SYMBOL( csx_get_module_fncp );
EXPORT_SYMBOL( csx_register_module );
EXPORT_SYMBOL( csx_register_util );
EXPORT_SYMBOL( csx_deregister_module );
EXPORT_SYMBOL( csx_deregister_util );

