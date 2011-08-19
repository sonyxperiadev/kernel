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
*  @file    csx_gist.c
*
*  @brief   GIST CSX module.
*
****************************************************************************/

/* ---- Include Files ---------------------------------------------------- */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>

#include <linux/broadcom/gos/gos.h>
#include <linux/broadcom/gist.h>
#include <linux/broadcom/csx_gist.h>
#include <linux/broadcom/csx_framework.h>

/* ---- Public Variables ------------------------------------------------- */
/* ---- Private Constants and Types -------------------------------------- */

/* Arbitrary value to prevent zero value handle.  4 bit value unique among all utilities. */
#define CSX_GIST_UTIL_MAGIC_NUM                 0xA

#define CSX_GIST_HANDLE_INDEX_OFFSET            0
#define CSX_GIST_HANDLE_INDEX_MASK              0xFF
#define CSX_GIST_HANDLE_MODULE_ID_OFFSET        8
#define CSX_GIST_HANDLE_MODULE_ID_MASK          0xF
#define CSX_GIST_HANDLE_SYNC_FLAG_OFFSET        12
#define CSX_GIST_HANDLE_SYNC_FLAG_MASK          0x1
#define CSX_GIST_HANDLE_UNIQUE_ID_OFFSET        20
#define CSX_GIST_HANDLE_UNIQUE_ID_MASK          0xFF
#define CSX_GIST_HANDLE_MAGIC_NUM_OFFSET        28
#define CSX_GIST_HANDLE_MAGIC_NUM_MASK          0xF

#define CSX_GIST_HANDLE_GET_INDEX(x)          ( (x >> CSX_GIST_HANDLE_INDEX_OFFSET) & CSX_GIST_HANDLE_INDEX_MASK )
#define CSX_GIST_HANDLE_GET_MODULE_ID(x)      ( (x >> CSX_GIST_HANDLE_MODULE_ID_OFFSET) & CSX_GIST_HANDLE_MODULE_ID_MASK )
#define CSX_GIST_HANDLE_GET_SYNC_FLAG(x)      ( (x >> CSX_GIST_HANDLE_SYNC_FLAG_OFFSET) & CSX_GIST_HANDLE_SYNC_FLAG_MASK )
#define CSX_GIST_HANDLE_GET_MAGIC_NUM(x)      ( (x >> CSX_GIST_HANDLE_MAGIC_NUM_OFFSET) & CSX_GIST_HANDLE_MAGIC_NUM_MASK )

#define CSX_GIST_SEMAPHORE_TIME_WAIT_MS         10

typedef struct csx_gist_io_point_info
{
   CSX_IO_POINT_INFO csx_io_point_info;
   int capture_gist_handle;
   int inject_gist_handle;
   CSX_IO_HANDLE csx_io_point_handle;
   int io_point_active;

} CSX_GIST_IO_POINT_INFO;

/* ---- Private Function Prototypes -------------------------------------- */

static const char * csx_gist_get_name( void );
static unsigned int csx_gist_get_max_points( void );
static unsigned int csx_gist_size_of_info_struct( void );
static unsigned int csx_gist_size_of_point_struct( void );

static int csx_gist_inject( char *bufp, int bytes, void *csx_priv );
static int csx_gist_capture( char *bufp, int bytes, void *csx_priv );
static int csx_gist_capture_inject( char *bufp, int bytes, void *csx_priv );
static int csx_gist_frame_sync( CSX_IO_MODULE csx_module_id );

static int csx_gist_allocate_gist_handle( CSX_GIST_IO_POINT_INFO *pInfo,
                                              CSX_GIST_INFO *pGistInfo );

static CSX_IO_HANDLE csx_gist_generate_io_point_handle( int index );

static int csx_gist_cleanup_point( int index );

/* ---- Private Variables ------------------------------------------------ */

static char banner[] __initdata = KERN_INFO "CSX GIST: 1.00 (built on "__DATE__" "__TIME__")\n";

static CSX_GIST_IO_POINT_INFO gPointInfoList[CSX_GIST_IO_POINT_NUM_MAX];

/* Track number of points added in the system.  Used to keep a unique csx handle
 * within CSX_GIST.  Must ensure that CSX_GIST_IO_POINT_NUM_MAX does not exceed
 * the maximum value allowable based on bits allocated in handle (CSX_GIST_HANDLE_UNIQUE_ID_OFFSET)
 */
static unsigned int gNumPointsAdded = 0;

/* Track the number of active points in CSX_GIST. */
static unsigned int gNumPointsActive;

static GOS_SEM csx_gist_sem;

static int gSyncEnablePending[CSX_IO_MODULE_NUM_MAX];
static int gSyncEnable[CSX_IO_MODULE_NUM_MAX];

static CSX_UTIL_FNCS gUtilFncs =
{
   .csx_util_frame_sync_event = csx_gist_frame_sync
};

static CSX_UTIL_CTRL_FNCS gUtilCtrlFncs =
{
   .csx_util_get_name = csx_gist_get_name,
   .csx_util_get_max_points = csx_gist_get_max_points,
   .csx_util_size_of_info_struct = csx_gist_size_of_info_struct,
   .csx_util_size_of_point_struct = csx_gist_size_of_point_struct,

   .csx_util_add_point = (CSX_UTIL_ADD_POINT)csx_gist_add_point,
   .csx_util_remove_point = csx_gist_remove_point,
   .csx_util_get_num_active_points = csx_gist_get_num_active_points,
   .csx_util_query_all = (CSX_UTIL_QUERY_ALL)csx_gist_query_all,
   .csx_util_sync_enable = csx_gist_sync_enable,
   .csx_util_sync_disable = csx_gist_sync_disable,
};

/* ---- Functions -------------------------------------------------------- */

/***************************************************************************/
/**
*  Retrieve name of CSX utiltiy
*
*  @return
*     name of utility
*/
static const char * csx_gist_get_name( void )
{
   return "csx_gist";
}

/***************************************************************************/
/**
*  Retrieve maximum number of I/O points supported by CSX utiltiy
*
*  @return
*     maximum number of supported I/O points
*/
static unsigned int csx_gist_get_max_points( void )
{
   return CSX_GIST_IO_POINT_NUM_MAX;
}

/***************************************************************************/
/**
*  Retrieve size of info structure associated with CSX utility
*
*  @return
*     size of info structure
*/
static unsigned int csx_gist_size_of_info_struct( void )
{
   return sizeof(CSX_GIST_INFO);
}

/***************************************************************************/
/**
*  Retrieve size of point structure associated with CSX utility
*
*  @return
*     size of point structure
*/
static unsigned int csx_gist_size_of_point_struct( void )
{
   return sizeof(CSX_GIST_POINT);
}

/***************************************************************************/
/**
*  Perform an inject operation from GIST.
*
*  @return
*     0           - success
*     -ve         - failure code
*/
static int csx_gist_inject( char *bufp, int bytes, void *csx_priv )
{
   CSX_GIST_IO_POINT_INFO point_info;
   CSX_IO_MODULE module_id = CSX_GIST_HANDLE_GET_MODULE_ID( (CSX_IO_HANDLE)csx_priv );
   int sync_io_point = CSX_GIST_HANDLE_GET_SYNC_FLAG( (CSX_IO_HANDLE)csx_priv );
   int index;
   int inject_gist_handle;
   int allow_inject = 0;
   int rc;
   int valid = 0;

   if ( CSX_GIST_HANDLE_GET_MAGIC_NUM( (CSX_IO_HANDLE)csx_priv ) != CSX_GIST_UTIL_MAGIC_NUM )
   {
      /* Handle does not match magic number of utility */
      return -EBADF;
   }

   /* Validate if inject operation is to be synchronized. */
   if ( sync_io_point )
   {
      if ( gSyncEnable[module_id] )
      {
         allow_inject = 1;
      }
   }
   else
   {
      /* Inject operation is not synched.  Simply allow operation */
      allow_inject = 1;
   }

   if ( allow_inject )
   {
      index = CSX_GIST_HANDLE_GET_INDEX( (CSX_IO_HANDLE)csx_priv );
      if ( index >= 0 &&
           index < CSX_GIST_IO_POINT_NUM_MAX )
      {
         /* Grab instance of point info for atomic operation */
         memcpy( &point_info, &gPointInfoList[index], sizeof(CSX_GIST_IO_POINT_INFO) );
         if ( point_info.io_point_active == CSX_POINT_ENABLE )
         {
            inject_gist_handle = point_info.inject_gist_handle;

            if ( inject_gist_handle != GIST_INVALID_HANDLE )
            {
               valid = 1;
               rc = gist_read( inject_gist_handle, bytes, bufp );
               return rc;
            }
            else
            {
               /* GIST handle is invalid */
               return -EBADF;
            }
         }
      }

      if ( !valid )
      {
         /* csx handle provided is invalid */
         return -EINVAL;
      }
   }
   return 0;
}

/***************************************************************************/
/**
*  Perform a capture operation from I/O GIST.
*
*  @return
*     0           - success
*     -ve         - failure code
*/
static int csx_gist_capture( char *bufp, int bytes, void *csx_priv )
{
   CSX_GIST_IO_POINT_INFO point_info;
   CSX_IO_MODULE module_id = CSX_GIST_HANDLE_GET_MODULE_ID( (CSX_IO_HANDLE)csx_priv );
   int sync_io_point = CSX_GIST_HANDLE_GET_SYNC_FLAG( (CSX_IO_HANDLE)csx_priv );
   int index;
   int capture_gist_handle;
   int allow_capture = 0;
   int rc;
   int valid = 0;

   if ( CSX_GIST_HANDLE_GET_MAGIC_NUM( (CSX_IO_HANDLE)csx_priv ) != CSX_GIST_UTIL_MAGIC_NUM )
   {
      /* Handle does not match magic number of utility */
      return -EBADF;
   }

   /* Validate if capture operation is to be synchronized. */
   if ( sync_io_point )
   {
      if ( gSyncEnable[module_id] )
      {
         allow_capture = 1;
      }
   }
   else
   {
      /* Capture operation is not synched.  Simply allow operation */
      allow_capture = 1;
   }

   if ( allow_capture )
   {
      index = CSX_GIST_HANDLE_GET_INDEX( (CSX_IO_HANDLE)csx_priv );
      if ( index >= 0 &&
           index < CSX_GIST_IO_POINT_NUM_MAX )
      {
         /* Grab instance of point info for atomic operation */
         memcpy( &point_info, &gPointInfoList[index], sizeof(CSX_GIST_IO_POINT_INFO) );
         if ( point_info.io_point_active == CSX_POINT_ENABLE )
         {
            capture_gist_handle = point_info.capture_gist_handle;

            if ( capture_gist_handle != GIST_INVALID_HANDLE )
            {
               valid = 1;
               rc = gist_write( capture_gist_handle, bytes, bufp );
               return rc;
            }
            else
            {
               /* GIST handle is invalid */
               return -EBADF;
            }
         }
      }

      if ( !valid )
      {
         /* csx handle provided is invalid */
         return -EINVAL;
      }
   }

   return 0;
}

/***************************************************************************/
/**
*  Perform a capture and inject operation from I/O GIST.
*
*  @return
*     0           - success
*     -ve         - failure code
*/
static int csx_gist_capture_inject( char *bufp, int bytes, void *csx_priv )
{
   CSX_GIST_IO_POINT_INFO point_info;
   CSX_IO_MODULE module_id = CSX_GIST_HANDLE_GET_MODULE_ID( (CSX_IO_HANDLE)csx_priv );
   int sync_io_point = CSX_GIST_HANDLE_GET_SYNC_FLAG( (CSX_IO_HANDLE)csx_priv );
   int index;
   int capture_gist_handle;
   int inject_gist_handle;
   int allow_capture_inject = 0;
   int rc;
   int valid = 0;

   if ( CSX_GIST_HANDLE_GET_MAGIC_NUM( (CSX_IO_HANDLE)csx_priv ) != CSX_GIST_UTIL_MAGIC_NUM )
   {
      /* Handle does not match magic number of utility */
      return -EBADF;
   }

   /* Validate if capture operation is to be synchronized. */
   if ( sync_io_point )
   {
      if ( gSyncEnable[module_id] )
      {
         allow_capture_inject = 1;
      }
   }
   else
   {
      /* Capture operation is not synched.  Simply allow operation */
      allow_capture_inject = 1;
   }

   if ( allow_capture_inject )
   {
      index = CSX_GIST_HANDLE_GET_INDEX( (CSX_IO_HANDLE)csx_priv );
      if ( index >= 0 &&
           index < CSX_GIST_IO_POINT_NUM_MAX )
      {
         /* Grab instance of point info for atomic operation */
         memcpy( &point_info, &gPointInfoList[index], sizeof(CSX_GIST_IO_POINT_INFO) );
         if ( point_info.io_point_active == CSX_POINT_ENABLE )
         {
            capture_gist_handle = point_info.capture_gist_handle;
            inject_gist_handle = point_info.capture_gist_handle;

            if ( capture_gist_handle != GIST_INVALID_HANDLE )
            {
               rc = gist_write( capture_gist_handle, bytes, bufp );
            }
            else
            {
               /* GIST handle is invalid */
               return -EBADF;
            }

            if ( inject_gist_handle != GIST_INVALID_HANDLE )
            {
               valid = 1;
               rc = gist_read( inject_gist_handle, bytes, bufp );
               return rc;
            }
            else
            {
               /* GIST handle is invalid */
               return -EBADF;
            }
         }
      }

      if ( !valid )
      {
         /* csx handle provided is invalid */
         return -EINVAL;
      }
   }
   return 0;
}

/***************************************************************************/
/**
*  Set synchronized enable/disable operation based on pending flag.
*  Function gets called every module frame sync.
*
*  @return
*     0           - success
*     -ve         - failure code
*/
static int csx_gist_frame_sync( CSX_IO_MODULE csx_module_id )
{
   if ( gSyncEnable[csx_module_id] != gSyncEnablePending[csx_module_id] )
   {
      gSyncEnable[csx_module_id] = gSyncEnablePending[csx_module_id];
   }
   return 0;
}

/***************************************************************************/
/**
*  Allocate GIST stream and store GIST handle
*
*  @return
*     0           - success
*     -ve         - failure code
*/
static int csx_gist_allocate_gist_handle( CSX_GIST_IO_POINT_INFO *pInfo,
                                              CSX_GIST_INFO *pGistInfo )
{
   int rc = 0;
   int capture_gist_handle;
   int inject_gist_handle;

   /* Allocate based on I/O mode */
   switch( pInfo->csx_io_point_info.csx_io_mode )
   {
      case CSX_IO_MODE_NONE:
      {
         pInfo->capture_gist_handle = GIST_INVALID_HANDLE;
         pInfo->inject_gist_handle = GIST_INVALID_HANDLE;
         break;
      }
      case CSX_IO_MODE_CAPTURE:
      {
         capture_gist_handle = gist_alloc_writer( &pGistInfo->capture );

         if ( capture_gist_handle != GIST_INVALID_HANDLE )
         {
            pInfo->capture_gist_handle = capture_gist_handle;
         }
         else
         {
            rc = -EINVAL;
         }
         break;
      }
      case CSX_IO_MODE_INJECT:
      {
         inject_gist_handle = gist_alloc_reader( &pGistInfo->inject );

         if ( inject_gist_handle != GIST_INVALID_HANDLE )
         {
            pInfo->inject_gist_handle = inject_gist_handle;
         }
         else
         {
            rc = -EINVAL;
         }
         break;
      }
      case CSX_IO_MODE_CAPTURE_INJECT:
      {
         capture_gist_handle = gist_alloc_writer( &pGistInfo->capture );
         inject_gist_handle = gist_alloc_reader( &pGistInfo->inject );

         /* Expect both capture and inject */
         if ( capture_gist_handle != GIST_INVALID_HANDLE && inject_gist_handle != GIST_INVALID_HANDLE )
         {
            pInfo->capture_gist_handle = capture_gist_handle;
            pInfo->inject_gist_handle = inject_gist_handle;
         }
         else
         {
            /* Free a successful allocation if there exists one
             */
            if ( capture_gist_handle != GIST_INVALID_HANDLE )
            {
               gist_free_writer( capture_gist_handle );
            }
            else if ( inject_gist_handle != GIST_INVALID_HANDLE )
            {
               gist_free_reader( inject_gist_handle );
            }

            rc = -EINVAL;
         }
         break;
      }
      default:
      {
         rc = -EINVAL;
         break;
      }
   }

   return rc;
}

/***************************************************************************/
/**
*  Generate CSX GIST type I/O point handle
*
*  @return
*     CSX handle
*/
static CSX_IO_HANDLE csx_gist_generate_io_point_handle( int index )
{
   CSX_IO_HANDLE handle = 0;
   CSX_GIST_IO_POINT_INFO *pInfo = &gPointInfoList[index];

   /* Add util magic num (prevent possibility of 0 handle value) */
   handle |= ((CSX_GIST_UTIL_MAGIC_NUM & CSX_GIST_HANDLE_MAGIC_NUM_MASK) << CSX_GIST_HANDLE_MAGIC_NUM_OFFSET);

   /* Add CSX GIST unique ID based on number of new added points in system */
   handle |= ((gNumPointsAdded & CSX_GIST_HANDLE_UNIQUE_ID_MASK ) << CSX_GIST_HANDLE_UNIQUE_ID_OFFSET);

   /* Add sync bit */
   handle |= ((pInfo->csx_io_point_info.csx_io_sync & CSX_GIST_HANDLE_SYNC_FLAG_MASK) << CSX_GIST_HANDLE_SYNC_FLAG_OFFSET);

   /* Place CSX module ID */
   handle |= ((pInfo->csx_io_point_info.csx_module_id & CSX_GIST_HANDLE_MODULE_ID_MASK) << CSX_GIST_HANDLE_MODULE_ID_OFFSET);

   /* Place index in list */
   handle |= ((index & CSX_GIST_HANDLE_INDEX_MASK ) << CSX_GIST_HANDLE_INDEX_OFFSET);

   return handle;
}

/***************************************************************************/
/**
*  Cleanup I/O point and remove.
*  Disables I/O point in module, delete GIST stream, and clear out
*  I/O point information in list.
*
*  @return
*     0           - success
*     -ve         - failure code
*/
static int csx_gist_cleanup_point( int index )
{
   CSX_IO_POINT_INFO *csx_info;
   CSX_MODULE_FNCS *csx_mod_ops;
   CSX_IO_POINT_FNCS csx_ops;
   CSX_IO_HANDLE csx_io_handle;
   int err;

   memset( &csx_ops, 0, sizeof(CSX_IO_POINT_FNCS) );

   err = gosSemTimedTake( csx_gist_sem, CSX_GIST_SEMAPHORE_TIME_WAIT_MS );
   if ( err )
   {
      return err;
   }

   csx_io_handle = gPointInfoList[index].csx_io_point_handle;
   csx_info = &gPointInfoList[index].csx_io_point_info;
   csx_mod_ops = csx_get_module_fncp( csx_info->csx_module_id );

   if ( gPointInfoList[index].io_point_active == CSX_POINT_ENABLE )
   {
      /* Clear out callbacks in module */
      if ( csx_mod_ops->csx_module_set_point != NULL )
      {
         err = csx_mod_ops->csx_module_set_point( csx_info->csx_device_id, csx_info->csx_point_id, &csx_ops, (void*)csx_io_handle );
         if ( err )
         {
            gosSemGive( csx_gist_sem );
            return err;
         }
      }

      if ( gPointInfoList[index].capture_gist_handle )
      {
         gist_free_writer( gPointInfoList[index].capture_gist_handle );
      }

      if ( gPointInfoList[index].inject_gist_handle )
      {
         gist_free_reader( gPointInfoList[index].inject_gist_handle );
      }

      /* Clear out I/O point information */
      memset( &gPointInfoList[index], 0, sizeof(CSX_GIST_IO_POINT_INFO) );

      gNumPointsActive--;
   }
   else
   {
      gosSemGive( csx_gist_sem );
      /* Handle passed in selects non-active I/O point */
      return -EINVAL;
   }
   gosSemGive( csx_gist_sem );
   return 0;
}

/***************************************************************************/
/**
*  Adds active I/O point to module that streams to and from GIST
*
*  @return
*     CSX handle
*/
int csx_gist_add_point( CSX_IO_POINT_INFO *csx_info,
                        CSX_GIST_INFO *csx_gist_info,
                        CSX_IO_HANDLE *csx_handle )
{
   CSX_IO_POINT_FNCS csx_ops;
   CSX_MODULE_FNCS *csx_mod_ops;
   CSX_IO_HANDLE handle;
   int first_inactive_index = -1;
   int err;
   int i;

   err = gosSemTimedTake( csx_gist_sem, CSX_GIST_SEMAPHORE_TIME_WAIT_MS );
   if ( err )
   {
      return err;
   }

   /* Verify point not already added */
   for( i = 0; i < CSX_GIST_IO_POINT_NUM_MAX; i++ )
   {
      if ( gPointInfoList[i].io_point_active == CSX_POINT_ENABLE )
      {
         CSX_IO_POINT_INFO *pInfo = &gPointInfoList[i].csx_io_point_info;

         /* Check for match */
         if( pInfo->csx_module_id == csx_info->csx_module_id &&
             pInfo->csx_device_id == csx_info->csx_device_id &&
             pInfo->csx_point_id == csx_info->csx_point_id )
         {
            /* Point exists in list.  Must remove point prior to add */
            gosSemGive( csx_gist_sem );
            printk( KERN_ERR "%s: I/O point specified already exists, must remove prior to adding\n", __FUNCTION__);
            *csx_handle = CSX_HANDLE_INVALID;
            return -EEXIST;
         }
      }
   }

   /* Point not found, search for inactive position */
   for ( i = 0; i < CSX_GIST_IO_POINT_NUM_MAX; i++ )
   {
      if ( gPointInfoList[i].io_point_active == CSX_POINT_DISABLE )
      {
         first_inactive_index = i;
         break;
      }
   }

   if ( first_inactive_index < 0 )
   {
      /* Exceeded the maximum number of I/O points allowable */
      gosSemGive( csx_gist_sem );
      printk( KERN_ERR "%s: Cannot exceed maximum number of I/O points (%d)\n", __FUNCTION__, CSX_GIST_IO_POINT_NUM_MAX);
      *csx_handle = CSX_HANDLE_INVALID;
      return -ENOSPC;
   }

   /* Add new I/O point info to the list */
   memcpy( &gPointInfoList[first_inactive_index].csx_io_point_info, csx_info, sizeof(CSX_IO_POINT_INFO) );

   /* Generate GIST handle and store */
   err = csx_gist_allocate_gist_handle( &gPointInfoList[first_inactive_index], csx_gist_info );

   if ( err )
   {
      gosSemGive( csx_gist_sem );
      *csx_handle = CSX_HANDLE_INVALID;
      return err;
   }

   /* Generate I/O point handle */
   handle = csx_gist_generate_io_point_handle( first_inactive_index );

   /* Setup function pointers based on I/O mode */
   switch( csx_info->csx_io_mode )
   {
      case CSX_IO_MODE_NONE:
      {
         csx_ops.csxCallback = NULL;
         break;
      }
      case CSX_IO_MODE_CAPTURE:
      {
         csx_ops.csxCallback = csx_gist_capture;
         break;
      }
      case CSX_IO_MODE_INJECT:
      {
         csx_ops.csxCallback = csx_gist_inject;
         break;
      }
      case CSX_IO_MODE_CAPTURE_INJECT:
      {
         csx_ops.csxCallback = csx_gist_capture_inject;
         break;
      }
      default:
      {
         printk( KERN_ERR "%s: CSX IO mode selected is invalid\n", __FUNCTION__ );
         *csx_handle = CSX_HANDLE_INVALID;
         return -EINVAL;
      }
   }

   csx_mod_ops = csx_get_module_fncp( csx_info->csx_module_id );

   if ( csx_mod_ops->csx_module_set_point != NULL )
   {
      err = csx_mod_ops->csx_module_set_point( csx_info->csx_device_id, csx_info->csx_point_id, &csx_ops, (void *)handle );
   }
   else
   {
      gosSemGive( csx_gist_sem );
      printk( KERN_ERR "%s: CSX Module selected for I/O point not registered\n", __FUNCTION__ );
      return CSX_HANDLE_INVALID;
   }

   if ( !err )
   {
      /* Store handle for query purposes */
      gPointInfoList[first_inactive_index].csx_io_point_handle = handle;

      /* Activate I/O point on the list */
      gPointInfoList[first_inactive_index].io_point_active = CSX_POINT_ENABLE;

      gNumPointsAdded++;
      gNumPointsActive++;
   }
   else
   {
      /* Clear information as I/O point could not be added */
      memset( &gPointInfoList[first_inactive_index], 0, sizeof(CSX_GIST_IO_POINT_INFO) );
      gosSemGive( csx_gist_sem );
      *csx_handle = CSX_HANDLE_INVALID;
      return err;
   }

   gosSemGive( csx_gist_sem );
   *csx_handle = handle;
   return 0;
}

/***************************************************************************/
/**
*  Remove active I/O point
*
*  @return
*     0           - success
*     -ve         - failure code
*/
int csx_gist_remove_point( CSX_IO_HANDLE csx_handle )
{
   int rc;
   int index = CSX_GIST_HANDLE_GET_INDEX(csx_handle);
   if ( index >= 0 && index < CSX_GIST_IO_POINT_NUM_MAX )
   {
      rc = csx_gist_cleanup_point( index );
   }
   else
   {
      return -EINVAL;
   }
   return rc;
}

/***************************************************************************/
/**
*  Returns number of active I/O points in CSX GIST
*
*  @return
*     0           - success
*     -ve         - failure code
*/
unsigned int csx_gist_get_num_active_points( void )
{
   return gNumPointsActive;
}


/***************************************************************************/
/**
*  Query for information on active I/O point
*
*  @return
*     0           - success
*     -ve         - failure code
*/
int csx_gist_query_all( CSX_GIST_POINT *csx_gist_point, unsigned int *num_points )
{
   int i;
   int j = 0;
   for( i = 0; i < CSX_GIST_IO_POINT_NUM_MAX; i++ )
   {
      if ( gPointInfoList[i].io_point_active == CSX_POINT_ENABLE )
      {
         memcpy( &csx_gist_point[j].csx_io_point_info, &gPointInfoList[i].csx_io_point_info, sizeof(CSX_IO_POINT_INFO) );
         memcpy( &csx_gist_point[j].csx_io_point_handle, &gPointInfoList[i].csx_io_point_handle, sizeof(CSX_IO_HANDLE) );

         if ( gPointInfoList[i].capture_gist_handle != GIST_INVALID_HANDLE )
         {
            gist_get_writer_params( gPointInfoList[i].capture_gist_handle, &csx_gist_point[j].csx_gist_info.capture );
         }
         else
         {
            memset( &csx_gist_point[j].csx_gist_info.capture, 0, sizeof(GIST_WRITER_PARAMS) );
         }

         if ( gPointInfoList[i].inject_gist_handle != GIST_INVALID_HANDLE )
         {
            gist_get_reader_params( gPointInfoList[i].inject_gist_handle, &csx_gist_point[j].csx_gist_info.inject );
         }
         else
         {
            memset( &csx_gist_point[j].csx_gist_info.inject, 0, sizeof(GIST_READER_PARAMS) );
         }

         j++;

         if ( j == *num_points )
         {
            break;
         }
      }
   }

   /* Update with actual number of active points */
   *num_points = j;
   return 0;
}

/***************************************************************************/
/**
*  Set pending a synchronized enable
*
*  @return
*     0           - success
*     -ve         - failure code
*/
int csx_gist_sync_enable( CSX_IO_MODULE csx_io_module )
{
   gSyncEnablePending[csx_io_module] = 1;
   return 0;
}

/***************************************************************************/
/**
*  Set pending a synchronized disable
*
*  @return
*     0           - success
*     -ve         - failure code
*/
int csx_gist_sync_disable( CSX_IO_MODULE csx_io_module )
{
   gSyncEnablePending[csx_io_module] = 0;
   return 0;
}

/***************************************************************************/
/**
*  CSX GIST module initilization
*
*  @return
*     0           - success
*     -ve         - failure code
*/
static int __init csx_gist_init( void )
{
   int err;

   printk( banner );

   err = gosSemAlloc( "csx_gist_sem", 1, &csx_gist_sem );
   if ( err )
   {
      gosSemFree( csx_gist_sem );
      return err;
   }

   memset( &gPointInfoList, 0, (sizeof(CSX_GIST_IO_POINT_INFO) * CSX_GIST_IO_POINT_NUM_MAX) );

   /* Default disabled capture/inject sync */
   memset( gSyncEnable, 0, (sizeof(int) * CSX_IO_MODULE_NUM_MAX) );
   memset( gSyncEnablePending, 0, (sizeof(int) * CSX_IO_MODULE_NUM_MAX) );

   gNumPointsActive = 0;

   err = csx_register_util( CSX_IO_UTIL_GIST, &gUtilFncs, &gUtilCtrlFncs );

   return err;
}

/***************************************************************************/
/**
*  CSX GIST module exit
*
*  @return
*     None
*/
static void __exit csx_gist_exit( void )
{
   int i;

   /* Clear out all associated active I/O points */
   for ( i = 0; i < CSX_GIST_IO_POINT_NUM_MAX; i++ )
   {
      if ( gPointInfoList[i].io_point_active == CSX_POINT_ENABLE )
      {
         csx_gist_cleanup_point(i);
      }
   }

   /* De-register ourselves from CSX framework */
   csx_deregister_util( CSX_IO_UTIL_GIST );

   gosSemFree( csx_gist_sem );
}

module_init( csx_gist_init );
module_exit( csx_gist_exit );

MODULE_AUTHOR( "Broadcom" );
MODULE_DESCRIPTION( "CSX gist module" );
MODULE_LICENSE( "GPL" );

EXPORT_SYMBOL( csx_gist_add_point );
EXPORT_SYMBOL( csx_gist_remove_point );
EXPORT_SYMBOL( csx_gist_get_num_active_points );
EXPORT_SYMBOL( csx_gist_query_all );
EXPORT_SYMBOL( csx_gist_sync_enable );
EXPORT_SYMBOL( csx_gist_sync_disable );

