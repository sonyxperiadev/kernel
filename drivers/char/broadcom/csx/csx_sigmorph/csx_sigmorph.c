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
 *  @file    csx_sigmorph.c
 *
 *  @brief   CSX SIGMORPH utility 
 *
 ****************************************************************************/

/* ---- Include Files ---------------------------------------------------- */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

#include <linux/broadcom/gist.h>
#include <linux/broadcom/csx_framework.h>
#include <linux/broadcom/gos/gos.h>              /* semaphore */

#include <linux/broadcom/csx_sigmorph.h>
#include <linux/broadcom/sigmorph.h>

/* ---- Private Debug Definitions  -------------------------------------- */

#ifdef CSX_SIGMORPH_DEBUG_ON
#undef CSX_SIGMORPH_DEBUG_ON        /* Turn OFF debugging */
#endif

#ifdef CSX_SIGMORPH_DEBUG_ON
#include <linux/broadcom/knllog.h>

#define CSX_SIGMORPH_DEBUG( msg, args... )\
            KNLLOG("%d " msg,__LINE__,##args)
#define CSX_SIGMORPH_DEBUG_DUMP_MEM(addr,mem,nbytes)\
            KNLLOG_DUMP_MEM(addr,mem,nbytes)
#else
#define CSX_SIGMORPH_DEBUG( msg, args... ) do {} while ( 0 )
#define CSX_SIGMORPH_DEBUG_DUMP_MEM( addr,mem,nbytes ) do {} while ( 0 )
#endif

/* ---- Private Constants and Types -------------------------------------- */

#define CSX_SIGMORPH_SEMAPHORE_TIME_WAIT_MS        (10)
#define CSX_SIGMORPH_BUF_SIZE                      (1 << 8)

/* Unique bit fields for each CSX_SIGMORPH operation */
#define CSX_SIGMORPH_CAPTURE_RAW                   (0x1)
#define CSX_SIGMORPH_CAPTURE_PROCD                 (0x2)
#define CSX_SIGMORPH_AMPSTRM                       (0x4)
#define CSX_SIGMORPH_AMPINJ                        (0x8)
#define CSX_SIGMORPH_MIX                           (0x10)
#define CSX_SIGMORPH_INJECT                        (0x20)

/* 
 * Arbitrary value to prevent zero value handle.  
 * 4 bit value unique among all utilities. 
 */
#define CSX_SIGMORPH_MAGIC_NUM                     (0xB)

#define CSX_SIGMORPH_HNDL_INDEX_OFFSET             (0)
#define CSX_SIGMORPH_HNDL_INDEX_MASK               (0xFF)
#define CSX_SIGMORPH_HNDL_MODULE_ID_OFFSET         (8)
#define CSX_SIGMORPH_HNDL_MODULE_ID_MASK           (0xF)
#define CSX_SIGMORPH_HNDL_SYNC_FLAG_OFFSET         (12)
#define CSX_SIGMORPH_HNDL_SYNC_FLAG_MASK           (0x1)
#define CSX_SIGMORPH_HNDL_UNIQUE_ID_OFFSET         (20)
#define CSX_SIGMORPH_HNDL_UNIQUE_ID_MASK           (0xFF)
#define CSX_SIGMORPH_HNDL_MAGIC_NUM_OFFSET         (28)
#define CSX_SIGMORPH_HNDL_MAGIC_NUM_MASK           (0xF)

#define CSX_SIGMORPH_HNDL_GET_INDEX(x)\
   ( (x >> CSX_SIGMORPH_HNDL_INDEX_OFFSET) & CSX_SIGMORPH_HNDL_INDEX_MASK )
#define CSX_SIGMORPH_HNDL_GET_MODULE_ID(x)\
   ( (x >> CSX_SIGMORPH_HNDL_MODULE_ID_OFFSET) & CSX_SIGMORPH_HNDL_MODULE_ID_MASK )
#define CSX_SIGMORPH_HNDL_GET_SYNC_FLAG(x)\
   ( (x >> CSX_SIGMORPH_HNDL_SYNC_FLAG_OFFSET) & CSX_SIGMORPH_HNDL_SYNC_FLAG_MASK )
#define CSX_SIGMORPH_HNDL_GET_MAGIC_NUM(x)\
   ( (x >> CSX_SIGMORPH_HNDL_MAGIC_NUM_OFFSET) & CSX_SIGMORPH_HNDL_MAGIC_NUM_MASK )
#define CSX_SIGMORPH_ISVALID_INDEX(x)\
   ( (x >= 0) && (x < CSX_SIGMORPH_IO_POINT_NUM_MAX) )

typedef struct csx_sigmorph_io_point_info
{
   CSX_IO_POINT_INFO csx_io_point_info;
   CSX_IO_HANDLE csx_io_point_handle;
   
   /* File I/O handles */
   int capture_raw_hndl;
   int capture_proc_hndl;
   int inject_hndl;
   
   /* Attenuation gains - Linear Q12 values */
   int16_t stream_gain;
   int16_t inject_gain;
   
   /* Tracking flags */
   int io_point_active;
   int options_priv;          /* Embeds processing operations */

} CSX_SIGMORPH_IO_POINT_INFO;

/* ---- Private Function Prototypes -------------------------------------- */

static CSX_IO_HANDLE csx_sigmorph_generate_io_point_handle( int index );
static int csx_sigmorph_get_available_index( CSX_IO_POINT_INFO *csx_info );
static const char * csx_sigmorph_get_name( void );
static unsigned int csx_sigmorph_get_max_points( void );
static unsigned int csx_sigmorph_size_of_info_struct( void );
static unsigned int csx_sigmorph_size_of_point_struct( void );

static int csx_sigmorph_callback( char *bufp, int bytes, void *csx_priv );
static int csx_sigmorph_cleanup_point( int index );
static int csx_sigmorph_frame_sync( CSX_IO_MODULE csx_module_id );

/* ---- Private Variables ------------------------------------------------ */

static CSX_SIGMORPH_IO_POINT_INFO point_registry [CSX_SIGMORPH_IO_POINT_NUM_MAX];
static int csx_sigmorph_syncregistry_pending [CSX_IO_MODULE_NUM_MAX];
static int csx_sigmorph_syncregistry_enable  [CSX_IO_MODULE_NUM_MAX];
static unsigned int point_registry_active;
static unsigned int point_registry_count = 0;
static GOS_SEM csx_sigmorph_sem;

/* ---- Functions -------------------------------------------------------- */

static CSX_IO_HANDLE csx_sigmorph_generate_io_point_handle( int index )
{
   CSX_IO_HANDLE handle = 0;
   CSX_SIGMORPH_IO_POINT_INFO *pinfo = &point_registry[index];

   /* Add util magic num (prevent possibility of 0 handle value) */
   handle |= ((CSX_SIGMORPH_MAGIC_NUM & CSX_SIGMORPH_HNDL_MAGIC_NUM_MASK) 
         << CSX_SIGMORPH_HNDL_MAGIC_NUM_OFFSET);

   /* Add CSX GIST unique ID based on number of new added points in system */
   handle |= ((point_registry_count & CSX_SIGMORPH_HNDL_UNIQUE_ID_MASK ) 
         << CSX_SIGMORPH_HNDL_UNIQUE_ID_OFFSET);

   /* Add sync bit */
   handle |= ((pinfo->csx_io_point_info.csx_io_sync & CSX_SIGMORPH_HNDL_SYNC_FLAG_MASK) 
         << CSX_SIGMORPH_HNDL_SYNC_FLAG_OFFSET);

   /* Place CSX module ID */
   handle |= ((pinfo->csx_io_point_info.csx_module_id & CSX_SIGMORPH_HNDL_MODULE_ID_MASK) 
         << CSX_SIGMORPH_HNDL_MODULE_ID_OFFSET);

   /* Place index in list */
   handle |= ((index & CSX_SIGMORPH_HNDL_INDEX_MASK ) 
         << CSX_SIGMORPH_HNDL_INDEX_OFFSET);

   return handle;
}

/**
 * Get next available index in the point registry 
 * 
 * Input Arguments
 *    csx_info    - pointer to I/O point information 
 *    
 * Output Arguments
 *    None
 *    
 * @return
 *    index       - next available index in registry
 *    -ve         - error value
 */
static int csx_sigmorph_get_available_index( CSX_IO_POINT_INFO *csx_info )
{
   int i, index = -1;
   CSX_IO_POINT_INFO *pinfo;
   
   for( i = 0; i < CSX_SIGMORPH_IO_POINT_NUM_MAX; i++ )
   {
      if ( point_registry[i].io_point_active == CSX_POINT_ENABLE )
      {
         pinfo = &point_registry[i].csx_io_point_info;

         if( pinfo->csx_module_id == csx_info->csx_module_id &&
             pinfo->csx_device_id == csx_info->csx_device_id &&
             pinfo->csx_point_id == csx_info->csx_point_id )
         {
            printk( KERN_ERR "I/O point exists, must remove prior to adding\n");
            return -EEXIST;
         }
      }
   }

   /* Search for inactive position */
   for ( i = 0; i < CSX_SIGMORPH_IO_POINT_NUM_MAX; i++ )
   {
      if ( point_registry[i].io_point_active == CSX_POINT_DISABLE )
      {
         index = i; break;
      }
   }
   CSX_SIGMORPH_DEBUG("Available index  (%d)", index);
   return index;
}

/**
 *  Retrieve maximum number of I/O points supported by CSX utiltiy
 *
 *  @return
 *     maximum number of supported I/O points
 */
static unsigned int csx_sigmorph_get_max_points( void )
{
   return CSX_SIGMORPH_IO_POINT_NUM_MAX;
}

/**
 * Retrieve name of CSX utiltiy
 *
 * @return
 *    name of utility
 */
static const char * csx_sigmorph_get_name( void )
{
   return "csx_sigmorph";
}

/**
 * Retrieve size of info structure associated with CSX utility
 *
 * @return
 *    size of info structure
 */
static unsigned int csx_sigmorph_size_of_info_struct( void )
{
   return sizeof(CSX_SIGMORPH_INFO);
}

/**
 * Retrieve size of point structure associated with CSX utility
 *
 * @return
 *   size of point structure
 */
static unsigned int csx_sigmorph_size_of_point_struct( void )
{
   return sizeof(CSX_SIGMORPH_POINT);
}

/**
 * Perform a attenuate/amplify/mix or capture/inject operation per
 * specified options
 * 
 * Input arguments:
 *    bufp        - module buffer pointer
 *    bytes       - number of bytes of data in the buffer
 *    *csx_priv   - unique handle assigned to kernel module
 *    
 * Output arguments: 
 *    None
 * 
 * @return
 *    0     - success
 *    -ve   - failure code
 */
static int csx_sigmorph_callback( char *bufp, int bytes, void *csx_priv )
{
   int err = 0;
   int options = 0;
   int inj_bytes = 0;
   CSX_SIGMORPH_HDL handle    = (CSX_SIGMORPH_HDL)csx_priv;
   int sync_point             = CSX_SIGMORPH_HNDL_GET_SYNC_FLAG( handle );
   int index                  = CSX_SIGMORPH_HNDL_GET_INDEX( handle );
   CSX_IO_MODULE module_id    = CSX_SIGMORPH_HNDL_GET_MODULE_ID( handle );
   int16_t *stream = (int16_t *) bufp;
   int16_t *result = stream;
   int16_t inj_buf[CSX_SIGMORPH_BUF_SIZE];
   int16_t mix_buf[CSX_SIGMORPH_BUF_SIZE];
   CSX_SIGMORPH_IO_POINT_INFO *rcrd;

   
   if ( CSX_SIGMORPH_HNDL_GET_MAGIC_NUM(handle) != CSX_SIGMORPH_MAGIC_NUM )
   {
      return -EBADF;
   }
   if ( sync_point == CSX_IO_SYNC_ON )
   {
      if ( csx_sigmorph_syncregistry_enable [module_id] == CSX_IO_SYNC_OFF )
      {
         return -EBUSY; 
      }
   }
   if ( !CSX_SIGMORPH_ISVALID_INDEX(index) )
   {
      return -EFAULT;
   }
   rcrd = &point_registry[index];
   
   if ( rcrd->io_point_active != CSX_POINT_ENABLE )
   {
      return -EBUSY; 
   }
   options = rcrd->options_priv;

   memset ( inj_buf, 0, CSX_SIGMORPH_BUF_SIZE );
   memset ( mix_buf, 0, CSX_SIGMORPH_BUF_SIZE );

   /*
    *       BEGIN PROCESSING
    */
   if ( options & CSX_SIGMORPH_CAPTURE_RAW )
   {
      (void) gist_write( rcrd->capture_raw_hndl, bytes, stream );
   }

   if ( options & CSX_SIGMORPH_AMPSTRM )
   {
      (void) sigmorph_amplify ( stream, bytes, rcrd->stream_gain );
   }

   if ( options & CSX_SIGMORPH_INJECT )
   {
      inj_bytes = gist_read( rcrd->inject_hndl, bytes, inj_buf );
      result = inj_buf;
   }
   if( options & CSX_SIGMORPH_AMPINJ )
   {
      (void) sigmorph_amplify ( inj_buf, bytes, rcrd->inject_gain );
      result = inj_buf;
   }

   if ( options & CSX_SIGMORPH_MIX )
   {
      if ( inj_bytes == bytes )
      {
         (void) sigmorph_mix ( mix_buf, stream, inj_buf, bytes );
         result = mix_buf;
      }
      else
      {
         result = stream;
      }
   }

   if ( options & CSX_SIGMORPH_CAPTURE_PROCD )
   {
      (void) gist_write( rcrd->capture_proc_hndl, bytes, result);
   }

   /* update stream */ 
   if ( result != stream )
   {
      (void) memcpy ( stream, result, bytes );
   }

   return err;
} /* csx_sigmorph_callback */

/**
 * Set synchronized enable/disable
 * 
 * @return
 *    0     - success
 *    -ve   - failure code
 */
static int csx_sigmorph_frame_sync( CSX_IO_MODULE csx_module_id )
{
   int err = 0;
   csx_sigmorph_syncregistry_enable [csx_module_id] 
                             = csx_sigmorph_syncregistry_pending [csx_module_id];
   return err;
}

static int csx_sigmorph_cleanup_point( int index )
{
   int err = -1;
   CSX_IO_HANDLE csx_io_handle;
   CSX_MODULE_FNCS *csx_mod_ops;
   CSX_IO_POINT_INFO *csx_info;
   CSX_IO_POINT_FNCS csx_ops;
   CSX_SIGMORPH_IO_POINT_INFO *rcrd = &point_registry[index];
   
   (void) memset( &csx_ops, 0, sizeof(CSX_IO_POINT_FNCS) );

   err = gosSemTimedTake( csx_sigmorph_sem, CSX_SIGMORPH_SEMAPHORE_TIME_WAIT_MS );
   if ( err )
   {
      return err;
   }

   /* BEGIN critical section */
   do {
         
      csx_io_handle  = rcrd->csx_io_point_handle;
      csx_info       = &rcrd->csx_io_point_info;
      csx_mod_ops    = csx_get_module_fncp(csx_info->csx_module_id);
      
      if (rcrd->io_point_active == CSX_POINT_ENABLE)
      {
         /* remove callbacks from kernel module */
         if ( csx_mod_ops->csx_module_set_point )
         {
            (void) csx_mod_ops->csx_module_set_point(
                           csx_info->csx_device_id, csx_info->csx_point_id,
                           &csx_ops, (void *)csx_io_handle);
         }
         /* Release GIST workers */
         if ( rcrd->capture_raw_hndl != GIST_INVALID_HANDLE )
         {
            CSX_SIGMORPH_DEBUG("Deleting GIST writer (0x%08X)",rcrd->capture_raw_hndl);
            (void) gist_free_writer( rcrd->capture_raw_hndl );
         }
         if ( rcrd->capture_proc_hndl != GIST_INVALID_HANDLE )
         {
            CSX_SIGMORPH_DEBUG("Deleting GIST writer (0x%08X)", rcrd->capture_proc_hndl);
            (void) gist_free_writer( rcrd->capture_proc_hndl );
         }
         if ( rcrd->inject_hndl != GIST_INVALID_HANDLE )
         {
            CSX_SIGMORPH_DEBUG("Deleting GIST reader (0x%08X)",rcrd->inject_hndl);
            (void) gist_free_reader( rcrd->inject_hndl );
         }
         
         /* retain rcrd slot */
         CSX_SIGMORPH_DEBUG("Removing/ disabling I/O point (%d)", index );
         (void) memset (rcrd, 0, sizeof (CSX_SIGMORPH_IO_POINT_INFO));
         rcrd->io_point_active = CSX_POINT_DISABLE;
         point_registry_active--;
      }
      else
      {
         err = -EINVAL;
      }
   /* END Critical section */
   } while (0);
   
   gosSemGive(csx_sigmorph_sem);
   return err;
}

/**
 * 
 * Input Arguments
 *    csx_info             -
 *    csx_simorph_info     -
 * 
 * Output Arguments
 *    csx_handle           - unique ID of I/O point to add
 * 
 * @return
 *    0        -  success
 *    -(ve)    -  Error code
 */
int csx_sigmorph_add_point(   CSX_IO_POINT_INFO *csx_info ,
                              CSX_SIGMORPH_INFO *csx_sigmorph_info ,
                              CSX_IO_HANDLE *csx_handle  )
{
   int err = -1;
   int available_index = -1;
   int gist_handle = GIST_INVALID_HANDLE;
   CSX_IO_HANDLE handle;
   CSX_IO_POINT_FNCS csx_ops;
   CSX_MODULE_FNCS *csx_mod_ops;
   CSX_SIGMORPH_IO_POINT_INFO *slot;

   err = gosSemTimedTake( csx_sigmorph_sem, CSX_SIGMORPH_SEMAPHORE_TIME_WAIT_MS );
   if ( err )
   {
      return err;
   }
   
   available_index = csx_sigmorph_get_available_index( csx_info );

   if ( !CSX_SIGMORPH_ISVALID_INDEX(available_index) )
   {
      printk( KERN_ERR "%s: No available ports. Remove unused points\n", __FUNCTION__);
      *csx_handle = CSX_HANDLE_INVALID;
      (void) gosSemGive( csx_sigmorph_sem );
      return -ENOSPC;
   }

   do 
   {
      slot = &point_registry[available_index];

      (void) memset ( slot, 0, sizeof (CSX_SIGMORPH_IO_POINT_INFO) );
      (void) memcpy ( &slot->csx_io_point_info, csx_info, 
            sizeof(CSX_IO_POINT_INFO) );

      point_registry_active++;
      slot->io_point_active = CSX_POINT_ENABLE;

      if ( csx_sigmorph_info->capture_raw.filename[0] )
      {
         gist_handle = gist_alloc_writer( & (csx_sigmorph_info->capture_raw) );
         
         if ( gist_handle == GIST_INVALID_HANDLE )
         {
            err = -EINVAL; break;
         }
         slot->capture_raw_hndl = gist_handle;
         slot->options_priv |= CSX_SIGMORPH_CAPTURE_RAW;
      }
      
      if ( csx_sigmorph_info->capture_procd.filename[0] )
      {
         gist_handle = gist_alloc_writer( & (csx_sigmorph_info->capture_procd) );
         
         if ( gist_handle == GIST_INVALID_HANDLE )
         {
            err = -EINVAL; break;
         }
         
         slot->capture_proc_hndl = gist_handle;
         slot->options_priv |= CSX_SIGMORPH_CAPTURE_PROCD;
      }
   
      if ( csx_sigmorph_info->inject.filename[0] )
      {
         gist_handle = gist_alloc_reader( & (csx_sigmorph_info->inject) );
         
         if ( gist_handle == GIST_INVALID_HANDLE )
         {
            err = -EINVAL; break;
         }
         slot->inject_hndl = gist_handle;
         slot->options_priv |= CSX_SIGMORPH_INJECT;
      }

      if ( csx_sigmorph_info->stream_gain )
      {
         slot->stream_gain = csx_sigmorph_info->stream_gain;
         slot->options_priv |= CSX_SIGMORPH_AMPSTRM;
      }

      if ( csx_sigmorph_info->inject_gain )
      {
         slot->inject_gain = csx_sigmorph_info->inject_gain;
         slot->options_priv |= CSX_SIGMORPH_AMPINJ;
      }

      if ( csx_sigmorph_info->mix == CSX_SIGMORPH_MIX_ENABLE )
      {
         CSX_SIGMORPH_DEBUG("enable MIXING operation");
         slot->options_priv |= CSX_SIGMORPH_MIX;
      }
      
      handle = csx_sigmorph_generate_io_point_handle( available_index );
      slot->csx_io_point_handle = handle;
      
      /* Register function with HAL module */
      csx_ops.csxCallback = &csx_sigmorph_callback;
      csx_mod_ops = csx_get_module_fncp( csx_info->csx_module_id );
      
      if ( csx_mod_ops->csx_module_set_point )
      {
         CSX_SIGMORPH_DEBUG("Set module device point");
         err = csx_mod_ops->csx_module_set_point
               ( 
                     csx_info->csx_device_id, csx_info->csx_point_id, 
                     &csx_ops, (void *) handle 
               );
         if ( err )
         {
            printk( KERN_ERR "Attempt to set HAL module point failed\n");
            break;
         }
      }
      else
      {
         printk( KERN_ERR "CSX module selected for SIGMORPH not registered\n");
         err = -EINVAL;
         break;
      }
      *csx_handle = handle;
      point_registry_count++;

   } while ( 0 ); /* END critical section */

   if ( err )
   {
      (void) csx_sigmorph_cleanup_point( available_index );
      *csx_handle = GIST_INVALID_HANDLE;
   }
   (void) gosSemGive( csx_sigmorph_sem );
   return err;
}

int csx_sigmorph_remove_point( CSX_IO_HANDLE csx_handle )
{
   int err = -EINVAL;
   int index = CSX_SIGMORPH_HNDL_GET_INDEX( csx_handle );

   if ( CSX_SIGMORPH_ISVALID_INDEX(index) )
   {
      err = csx_sigmorph_cleanup_point( index );
   }
 
   return err;
}

unsigned int csx_sigmorph_get_num_active_points( void )
{
   return point_registry_active;
}

int csx_sigmorph_query_all(   CSX_SIGMORPH_POINT *block , 
                              unsigned int *num_points )
{
   int err = 0;
   int i, count = 0;
   
   for (i = 0 ; i < CSX_SIGMORPH_IO_POINT_NUM_MAX; i++)
   {
      if (point_registry[i].io_point_active == CSX_POINT_ENABLE)
      {
         (void)memset ( &block[i], 0, sizeof (CSX_SIGMORPH_POINT) );
         (void)memcpy ( &block[i].csx_io_point_info, 
                        &point_registry[i].csx_io_point_info,
                        sizeof ( CSX_IO_POINT_INFO ));
         (void)memcpy ( &block[i].csx_io_point_handle,
                        &point_registry[i].csx_io_point_handle,
                        sizeof (CSX_IO_HANDLE));
         
         if ( point_registry[i].capture_proc_hndl != GIST_INVALID_HANDLE )
         {
            (void)gist_get_writer_params( point_registry[i].capture_proc_hndl, 
                                    & block[i].csx_sigmorph_info.capture_procd);
         }
         
         if (point_registry[i].capture_raw_hndl != GIST_INVALID_HANDLE )
         {
            (void)gist_get_writer_params( point_registry[i].capture_raw_hndl, 
                                    & block[i].csx_sigmorph_info.capture_raw);
         }
         
         if ( point_registry[i].inject_hndl != GIST_INVALID_HANDLE )
         {
            (void)gist_get_reader_params( point_registry[i].inject_hndl, 
                                    & block[i].csx_sigmorph_info.inject);
         }
         
         block[i].csx_sigmorph_info.inject_gain = point_registry[i].inject_gain;
         block[i].csx_sigmorph_info.stream_gain = point_registry[i].stream_gain;
         block[i].csx_sigmorph_info.mix = 
               (point_registry[i].options_priv & CSX_SIGMORPH_MIX) 
               ? CSX_SIGMORPH_MIX_ENABLE : 0; 
         count++;
      }
   }
   *num_points = count;
   return err;
}

int csx_sigmorph_sync_enable( CSX_IO_MODULE csx_io_module )
{
   int err = 0;
   csx_sigmorph_syncregistry_pending[csx_io_module] = CSX_IO_SYNC_ON;
   CSX_SIGMORPH_DEBUG( "I/O Module (%d) SYNC ON", csx_io_module);
   return err;
}

int csx_sigmorph_sync_disable( CSX_IO_MODULE csx_io_module )
{
   int err = 0;
   csx_sigmorph_syncregistry_pending [csx_io_module] = CSX_IO_SYNC_OFF;
   CSX_SIGMORPH_DEBUG( "I/O Module (%d) SYNC OFF", csx_io_module);
   return err;
}

static int __init csx_sigmorph_init( void )
{
   int i, err = 0;

   CSX_UTIL_FNCS util_funcs =
   {
      .csx_util_frame_sync_event       = &csx_sigmorph_frame_sync
   };

   CSX_UTIL_CTRL_FNCS util_ctrl_funcs =
   {
      .csx_util_get_name               = &csx_sigmorph_get_name,
      .csx_util_get_max_points         = &csx_sigmorph_get_max_points,
      .csx_util_size_of_info_struct    = &csx_sigmorph_size_of_info_struct,
      .csx_util_size_of_point_struct   = &csx_sigmorph_size_of_point_struct,
      .csx_util_add_point              = (CSX_UTIL_ADD_POINT)&csx_sigmorph_add_point,
      .csx_util_remove_point           = &csx_sigmorph_remove_point,
      .csx_util_get_num_active_points  = &csx_sigmorph_get_num_active_points,
      .csx_util_query_all              = (CSX_UTIL_QUERY_ALL)&csx_sigmorph_query_all,
      .csx_util_sync_enable            = &csx_sigmorph_sync_enable,
      .csx_util_sync_disable           = &csx_sigmorph_sync_disable,
   };

   /* setup mutual exclusion construct */
   err = gosSemAlloc( "csx_sigmorph_sem", 1, &csx_sigmorph_sem );
   if ( err )
   {
      return err;
   }

   /* clear ALL allocated data structures */
   memset ( (void *)&point_registry[0], 0, CSX_SIGMORPH_IO_POINT_NUM_MAX
            * sizeof (CSX_SIGMORPH_IO_POINT_INFO) );
   
   for (i = 0; i < CSX_IO_MODULE_NUM_MAX; i++)
   {
      csx_sigmorph_syncregistry_enable[i]    = CSX_IO_SYNC_OFF;
      csx_sigmorph_syncregistry_pending[i]   = CSX_IO_SYNC_OFF;
   }
   point_registry_active = 0;
   
   err = csx_register_util( CSX_IO_UTIL_SIGMORPH, &util_funcs, &util_ctrl_funcs );
   
   return err;
}

static void __exit csx_sigmorph_exit( void )
{
   int i, err = 0;

   for (i = 0; i < CSX_SIGMORPH_IO_POINT_NUM_MAX; i++)
   {
      if (point_registry[i].io_point_active == CSX_POINT_ENABLE )
      {
         err = csx_sigmorph_cleanup_point(i);
      }
   }

   err = csx_deregister_util( CSX_IO_UTIL_SIGMORPH ); /* unhook from CSX framework */
   err = gosSemFree( csx_sigmorph_sem );
   (void) err;
}

module_init( csx_sigmorph_init );
module_exit( csx_sigmorph_exit );

MODULE_AUTHOR( "Broadcom" );
MODULE_DESCRIPTION( "CSX sigmorph module" );
MODULE_LICENSE( "GPL" );

EXPORT_SYMBOL( csx_sigmorph_add_point );
EXPORT_SYMBOL( csx_sigmorph_remove_point );
EXPORT_SYMBOL( csx_sigmorph_get_num_active_points );
EXPORT_SYMBOL( csx_sigmorph_query_all );
EXPORT_SYMBOL( csx_sigmorph_sync_enable );
EXPORT_SYMBOL( csx_sigmorph_sync_disable );
