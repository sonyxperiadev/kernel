/*****************************************************************************
* Copyright 2011 Broadcom Corporation.  All rights reserved.
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

// ---- Include Files --------------------------------------------------------

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/cdev.h>
#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/pagemap.h>
#include <linux/slab.h>
#include <linux/ioctl.h>
#include <linux/semaphore.h>
#include <linux/proc_fs.h>
#include <linux/dma-mapping.h>
#include <linux/pfn.h>
#include <linux/hugetlb.h>
#include <linux/delay.h>

#include <vc_sm_defs.h>
#include <vc_sm_knl.h>
#include "interface/vcos/vcos.h"
#include "vc_vchi_wifihdmi.h"

// ---- Private Constants and Types ------------------------------------------

// Logging macros (for remapping to other logging mechanisms, i.e., vcos_log)
#define LOG_DBG( exp, fmt, arg... )  if ( exp ) printk( KERN_INFO "[D] " fmt "\n", ##arg )
#define LOG_INFO( fmt, arg... )      printk( KERN_INFO "[I] " fmt "\n", ##arg )
#define LOG_ERR( fmt, arg... )       printk( KERN_ERR  "[E] " fmt "\n", ##arg )

#define LOG_DBG_LEVEL_MIN          1
#define LOG_DBG_LEVEL_INTER_1      2
#define LOG_DBG_LEVEL_MAX          3

#define DEVICE_NAME              "wht"
#define DEVICE_MINOR             0

#define PROC_DIR_ROOT_NAME       "wht"
#define PROC_DEBUG               "debug"
#define PROC_CONTROL             "ctrl"
#define PROC_MODE                "mode"
#define PROC_WRITE_BUF_SIZE      128

#define SMT_ACTION_INIT          "init"
#define SMT_ACTION_DEINIT        "deinit"
#define SMT_ACTION_START         "start"
#define SMT_ACTION_STOP          "stop"
#define SMT_ACTION_STATS         "stats"
#define SMT_ACTION_SKTIN         "sktin"
#define SMT_ACTION_SKTDATA       "sktdata"

/* Global state information.
*/
typedef struct
{
   VC_VCHI_WIFIHDMI_HANDLE_T   wifihdmi_handle; // Handle for videocore service.

   struct proc_dir_entry *dir_root;             // Proc entries root.
   struct proc_dir_entry *control;              // Proc entry control.
   struct proc_dir_entry *debug;                // Proc entry debug.
   struct proc_dir_entry *mode;                 // Proc entry mode.

   char                  smt_cmd[PROC_WRITE_BUF_SIZE+1];
   int                   smt_mode;

   struct cdev           smt_cdev;              // Device.
   dev_t                 smt_devid;             // Device identifier.
   struct class          *smt_class;            // Class.
   struct device         *smt_dev;              // Device.

   VCOS_THREAD_T         smt_thread;
   uint32_t              smt_event_mask;
   uint32_t              smt_skt_hdl;
   uint32_t              smt_port_hdl;
   VCOS_MUTEX_T          smt_lock;
   VCOS_EVENT_T          smt_event;

} SMT_STATE_T;

#define START_EVENT_MASK     0x1
#define STOP_EVENT_MASK      0x2
#define INIT_EVENT_MASK      0x4
#define DEINIT_EVENT_MASK    0x8
#define STATS_EVENT_MASK     0x10
#define SKTIN_EVENT_MASK     0x20
#define SKTDATA_EVENT_MASK   0x40


// ---- Private Variables ----------------------------------------------------

static SMT_STATE_T *smt_state;
static unsigned int smt_debug_log = 0;

// ---- Private Function Prototypes ------------------------------------------
extern int whdmi_incoming_socket( int km_socket_handle, int socket_port );
extern int whdmi_data_on_socket( int km_socket_handle, int canned_data );

// ---- Private Functions ----------------------------------------------------

static void *vc_smt_ops_waiter( void *arg )
{
   uint32_t event_mask;
   VCOS_STATUS_T status;
   VC_WIFIHDMI_RESULT_T result;
   uint32_t trans_id;
   VC_WIFIHDMI_MODE_T mode;
   VC_WIFIHDMI_STATS_T stats;

   while ( 1 )
   {
      event_mask = 0;

      status = vcos_event_wait( &smt_state->smt_event );
      if ( status == VCOS_SUCCESS )
      {
         if ( vcos_mutex_lock ( &smt_state->smt_lock ) == VCOS_SUCCESS )
         {
            event_mask = smt_state->smt_event_mask;
            smt_state->smt_event_mask = 0;
            vcos_mutex_unlock ( &smt_state->smt_lock );
         }
         else
         {
            LOG_ERR( "[%s]: failed on smt-lock",
                     __func__ );
         }

         if ( event_mask & STOP_EVENT_MASK )
         {
            memset ( &mode, 0, sizeof(mode) );
            memset ( &result, 0, sizeof(result) );

            if ( smt_state->smt_mode == 1 )
            {
               mode.wifihdmi = 1;
            }
            else
            {
               mode.loopback = 1;
            }

            vc_vchi_wifihdmi_stop( smt_state->wifihdmi_handle,
                                   &mode, 
                                   &result,
                                   &trans_id );

            LOG_INFO( "[%s]: stopinng test returns %d",
                      __func__, result.success );
         }

         if ( event_mask & DEINIT_EVENT_MASK )
         {
            LOG_INFO( "[%s]: terminating test harness...",
                      __func__ );
         }

         if ( event_mask & STATS_EVENT_MASK )
         {
            memset ( &mode, 0, sizeof(mode) );
            memset ( &stats, 0, sizeof(stats) );

            if ( smt_state->smt_mode == 1 )
            {
               mode.wifihdmi = 1;
            }
            else
            {
               mode.loopback = 1;
            }

            vc_vchi_wifihdmi_stats( smt_state->wifihdmi_handle,
                                    &mode, 
                                    &stats,
                                    &trans_id );

            LOG_INFO( "[%s]: stats tx:%d, tx-missed:%d, tx-busy:%d, tx-rec:%d",
                      __func__,
                      stats.tx_cnt,
                      stats.tx_miss_cnt,
                      stats.tx_busy_cnt,
                      stats.tx_rec_cnt );
         }

         if ( event_mask & START_EVENT_MASK )
         {
            memset ( &mode, 0, sizeof(mode) );
            memset ( &result, 0, sizeof(result) );

            if ( smt_state->smt_mode == 1 )
            {
               mode.wifihdmi = 1;
            }
            else
            {
               mode.loopback = 1;
            }

            vc_vchi_wifihdmi_start( smt_state->wifihdmi_handle,
                                    &mode, 
                                    &result,
                                    &trans_id );

            LOG_INFO( "[%s]: starting test returns %d",
                      __func__, result.success );
         }

         if ( event_mask & INIT_EVENT_MASK )
         {
            LOG_INFO( "[%s]: init test data...",
                      __func__ );

            vc_vchi_wifihdmi_tx_pool( smt_state->wifihdmi_handle,
                                      40,
                                      2048 );
         }

         if ( event_mask & SKTIN_EVENT_MASK )
         {
            LOG_INFO( "[%s]: faking in accepted socket on %u, port %u",
                      __func__, smt_state->smt_skt_hdl, smt_state->smt_port_hdl );

            whdmi_incoming_socket( smt_state->smt_skt_hdl,
                                   smt_state->smt_port_hdl );
         }

         if ( event_mask & SKTDATA_EVENT_MASK )
         {
            LOG_INFO( "[%s]: faking in canned data on %u, canned-id %u",
                      __func__, smt_state->smt_skt_hdl, smt_state->smt_port_hdl );

            whdmi_data_on_socket( smt_state->smt_skt_hdl,
                                  smt_state->smt_port_hdl );
         }
      }
   }

   return NULL;
}

/* Read callback for the control proc entry.
*/
static int vc_smt_ctl_proc_read( char *buffer,
                                 char **start,
                                 off_t off,
                                 int count,
                                 int *eof,
                                 void *data )
{
   int len = 0;

   if ( (smt_state == NULL) || (off > 0) )
   {
      return 0;
   }

   len += sprintf( buffer + len,
                   "%s\n",
                   smt_state->smt_cmd );

   return len;
}

/* Read callback for the mode proc entry.
*/
static int vc_smt_mode_proc_read( char *buffer,
                                  char **start,
                                  off_t off,
                                  int count,
                                  int *eof,
                                  void *data )
{
   int len = 0;

   if ( (smt_state == NULL) || (off > 0) )
   {
      return 0;
   }

   len += sprintf( buffer + len,
                   "%s\n",
                   (smt_state->smt_mode == 1) ? "wifihdmi" : "loopback" );

   return len;
}

/* Read callback for the debug proc entry.
*/
static int vc_smt_debug_proc_read( char *buffer,
                                   char **start,
                                   off_t off,
                                   int count,
                                   int *eof,
                                   void *data )
{
   int len = 0;

   len += sprintf( buffer + len,
                   "- wifi-hdmi vc service handle 0x%x\n\n",
                   (unsigned int)smt_state->wifihdmi_handle );

   len += sprintf( buffer + len,
                   "- debug log level set to %u\n",
                   (unsigned int) smt_debug_log );
   len += sprintf( buffer + len,
                   "-- level is one increment in [0 (disabled), %u (highest)]\n",
                   LOG_DBG_LEVEL_MAX );

   return len;
}

/* Write callback for the control proc entry.
*/
static int vc_smt_ctl_proc_write( struct file *file,
                                  const char __user *buffer,
                                  unsigned long count,
                                  void *data )
{
   int ret;
   unsigned char kbuf[PROC_WRITE_BUF_SIZE+1];

   memset ( kbuf, 0, PROC_WRITE_BUF_SIZE+1 );
   if ( count >= PROC_WRITE_BUF_SIZE )
   {
      count = PROC_WRITE_BUF_SIZE;
   }

   if ( copy_from_user( &kbuf,
                        buffer,
                        count ) != 0 )
   {
      LOG_ERR( "[%s]: failed to copy-from-user",
               __func__ );

      ret = -EFAULT;
      goto out;
   }
   kbuf[ count - 1 ] = 0;

   /* Return read value no matter what from there on.
   */
   ret = count;

   if ( (strcmp ( kbuf, SMT_ACTION_START ) == 0) ||
        (strcmp ( kbuf, SMT_ACTION_STOP ) == 0)  ||
        (strcmp ( kbuf, SMT_ACTION_INIT ) == 0)  ||
        (strcmp ( kbuf, SMT_ACTION_DEINIT ) == 0) ||
        (strcmp ( kbuf, SMT_ACTION_STATS ) == 0) ||
        (strncmp ( kbuf, SMT_ACTION_SKTIN, strlen(SMT_ACTION_SKTIN) ) == 0) ||
        (strncmp ( kbuf, SMT_ACTION_SKTDATA, strlen(SMT_ACTION_SKTDATA) ) == 0) )
   {
      memcpy ( smt_state->smt_cmd,
               kbuf,
               sizeof ( kbuf ));

      if ( vcos_mutex_lock ( &smt_state->smt_lock ) == VCOS_SUCCESS )
      {
         if ( !strcmp ( smt_state->smt_cmd, SMT_ACTION_START ) )
         {
            smt_state->smt_event_mask |= START_EVENT_MASK;
         }
         else if ( !strcmp ( smt_state->smt_cmd, SMT_ACTION_STOP ) )
         {
            smt_state->smt_event_mask |= STOP_EVENT_MASK;
         }
         else if ( !strcmp ( smt_state->smt_cmd, SMT_ACTION_INIT ) )
         {
            smt_state->smt_event_mask |= INIT_EVENT_MASK;
         }
         else if ( !strcmp ( smt_state->smt_cmd, SMT_ACTION_DEINIT ) )
         {
            smt_state->smt_event_mask |= DEINIT_EVENT_MASK;
         }
         else if ( !strcmp ( smt_state->smt_cmd, SMT_ACTION_STATS ) )
         {
            smt_state->smt_event_mask |= STATS_EVENT_MASK;
         }
         else if ( !strncmp ( smt_state->smt_cmd, SMT_ACTION_SKTIN, strlen(SMT_ACTION_SKTIN) ) )
         {
            unsigned int skt_val, port_val;
            if ( sscanf( kbuf, "%s %u %u", smt_state->smt_cmd, &skt_val, &port_val ) != 3 )
            {
               LOG_ERR( "[%s]: sccanf parsing failed on \'%s\', expected \'sktin <skt_hdl> <port>\'",
                        __func__,
                        kbuf );
            }
            else
            {
               smt_state->smt_event_mask |= SKTIN_EVENT_MASK;
               smt_state->smt_skt_hdl    = skt_val;
               smt_state->smt_port_hdl   = port_val;
            }
         }
         else if ( !strncmp ( smt_state->smt_cmd, SMT_ACTION_SKTDATA, strlen(SMT_ACTION_SKTDATA) ) )
         {
            unsigned int skt_val, port_val;
            if ( sscanf( kbuf, "%s %u %u", smt_state->smt_cmd, &skt_val, &port_val ) != 3 )
            {
               LOG_ERR( "[%s]: sccanf parsing failed on \'%s\', expected \'sktdata <skt_hdl> <canned-id>\'",
                        __func__,
                        kbuf );
            }
            else
            {
               smt_state->smt_event_mask |= SKTDATA_EVENT_MASK;
               smt_state->smt_skt_hdl    = skt_val;
               smt_state->smt_port_hdl   = port_val;
            }
         }
         vcos_mutex_unlock ( &smt_state->smt_lock );
         vcos_event_signal( &smt_state->smt_event );
      }
   }
   else
   {
      LOG_ERR( "[%s]: unsupported action \'%s\'",
               __func__,
               kbuf );

      goto out;
   }

   /* Done.
   */
   goto out;

out:
   return ret;
}

/* Write callback for the debug proc entry.
*/
static int vc_smt_debug_proc_write( struct file *file,
                                    const char __user *buffer,
                                    unsigned long count,
                                    void *data )
{
   int ret;
   uint32_t debug_value;
   unsigned char kbuf[PROC_WRITE_BUF_SIZE+1];

   memset ( kbuf, 0, PROC_WRITE_BUF_SIZE+1 );
   if ( count >= PROC_WRITE_BUF_SIZE )
   {
      count = PROC_WRITE_BUF_SIZE;
   }

   if ( copy_from_user( kbuf,
                        buffer,
                        count ) != 0 )
   {
      LOG_ERR( "[%s]: failed to copy-from-user",
               __func__ );

      ret = -EFAULT;
      goto out;
   }
   kbuf[ count - 1 ] = 0;

   /* Return read value no matter what from there on.
   */
   ret = count;

   if( sscanf( kbuf, "%u", &debug_value ) != 1 )
   {
      LOG_ERR( "[%s]: echo <value> > /proc/%s/%s",
               __func__,
               PROC_DIR_ROOT_NAME,
               PROC_DEBUG );

      /* Failed to assign the proper value.
      */
      goto out;
   }

   if ( debug_value > LOG_DBG_LEVEL_MAX )
   {
      LOG_ERR( "[%s]: echo [0,%u] > /proc/%s/%s",
               __func__,
               LOG_DBG_LEVEL_MAX,
               PROC_DIR_ROOT_NAME,
               PROC_DEBUG );

      /* Failed to assign the proper value.
      */
      goto out;
   }

   LOG_INFO( "[%s]: debug log change from level %u to level %u",
            __func__,
            smt_debug_log,
            debug_value );
   smt_debug_log = debug_value;

   /* Done.
   */
   goto out;

out:
   return ret;
}

/* Write callback for the mode proc entry.
*/
static int vc_smt_mode_proc_write( struct file *file,
                                   const char __user *buffer,
                                   unsigned long count,
                                   void *data )
{
   int ret;
   uint32_t mode_value;
   unsigned char kbuf[PROC_WRITE_BUF_SIZE+1];

   memset ( kbuf, 0, PROC_WRITE_BUF_SIZE+1 );
   if ( count >= PROC_WRITE_BUF_SIZE )
   {
      count = PROC_WRITE_BUF_SIZE;
   }

   if ( copy_from_user( kbuf,
                        buffer,
                        count ) != 0 )
   {
      LOG_ERR( "[%s]: failed to copy-from-user",
               __func__ );

      ret = -EFAULT;
      goto out;
   }
   kbuf[ count - 1 ] = 0;

   /* Return read value no matter what from there on.
   */
   ret = count;

   if( sscanf( kbuf, "%u", &mode_value ) != 1 )
   {
      LOG_ERR( "[%s]: echo <value> > /proc/%s/%s",
               __func__,
               PROC_DIR_ROOT_NAME,
               PROC_MODE );

      /* Failed to assign the proper value.
      */
      goto out;
   }

   if ( mode_value > 1 )
   {
      LOG_ERR( "[%s]: echo (0: loopback | 1: wifihdmi) > /proc/%s/%s",
               __func__,
               PROC_DIR_ROOT_NAME,
               PROC_MODE );

      /* Failed to assign the proper value.
      */
      goto out;
   }

   LOG_INFO( "[%s]: mode change from %s to %s",
            __func__,
            (smt_state->smt_mode == 1) ? "wifihdmi" : "loopback",
            (mode_value == 1) ? "wifihdmi" : "loopback" );
   smt_state->smt_mode = mode_value;

   /* Done.
   */
   goto out;

out:
   return ret;
}

/* Open the device.  Creates a private state to help track all allocation
** associated with this device.
*/
static int vc_smt_open( struct inode *inode, struct file *file )
{
   int ret                   = 0;

   /* Make sure the device was started properly.
   */
   if ( smt_state == NULL )
   {
      LOG_ERR( "[%s]: invalid device",
               __func__ );
      
      ret = -EPERM;
      goto out;
   }

out:
   return ret;
}

/* Close the device.  Free up all resources still associated with this device
** at the time.
*/
static int vc_smt_release( struct inode *inode, struct file *file )
{
   int ret                         = 0;

   /* Make sure the device was started properly.
   */
   if ( smt_state == NULL )
   {
      LOG_ERR( "[%s]: invalid device",
               __func__ );
      
      ret = -EPERM;
      goto out;
   }

out:
   return ret;
}

/* Device operations that we managed in this driver.
*/
static struct file_operations vmcs_smt_ops =
{
   .owner           = THIS_MODULE,
   .open            = vc_smt_open,
   .release         = vc_smt_release,
};

/* Creation of device.
*/
static int vc_smt_create_test( void )
{
   int ret;

   if ( smt_state == NULL )
   {
      ret = -ENOMEM;
      goto out;
   }

   /* Create a device class for creating dev nodes.
   */
   smt_state->smt_class = class_create( THIS_MODULE, "vc-smt" );
   if ( IS_ERR( smt_state->smt_class ) )
   {
      LOG_ERR( "[%s]: unable to create device class",
               __func__ );

      ret = PTR_ERR( smt_state->smt_class );
      goto out;
   }

   /* Create a character driver.
   */
   if ( (ret = alloc_chrdev_region( &smt_state->smt_devid,
                                    DEVICE_MINOR,
                                    1,
                                    DEVICE_NAME)) != 0 )
   {
      LOG_ERR( "[%s]: unable to allocate device number",
               __func__ );
      goto out_dev_class_destroy;
   }

	cdev_init( &smt_state->smt_cdev, &vmcs_smt_ops );
	if ( (ret = cdev_add( &smt_state->smt_cdev,
	                      smt_state->smt_devid,
	                      1 )) != 0 )
	{
      LOG_ERR( "[%s]: unable to register device",
               __func__ );
		goto out_chrdev_unreg;
	}

   /* Create a device node.
   */
   smt_state->smt_dev = device_create( smt_state->smt_class,
                                       NULL,
                                       MKDEV( MAJOR( smt_state->smt_devid ), DEVICE_MINOR ),
                                       NULL,
                                       DEVICE_NAME );
   if ( IS_ERR( smt_state->smt_dev ) )
   {
      LOG_ERR( "[%s]: unable to create device node",
               __func__ );
      ret = PTR_ERR( smt_state->smt_dev );
      goto out_chrdev_del;
   }

   goto out;


out_chrdev_del:
   cdev_del( &smt_state->smt_cdev );
out_chrdev_unreg:
   unregister_chrdev_region( smt_state->smt_devid, 1 );
out_dev_class_destroy:
   class_destroy( smt_state->smt_class );
   smt_state->smt_class = NULL;
out:
   return ret;
}

/* Termination of the device.
*/
static int vc_smt_remove_test( void )
{
   int ret;

   if ( smt_state == NULL )
   {
      /* Nothing to do.
      */
      ret = 0;
      goto out;
   }

   /* Remove the sharedmemory character driver.
   */
	cdev_del( &smt_state->smt_cdev );

   /* Unregister region.
   */
	unregister_chrdev_region( smt_state->smt_devid, 1 );

   ret = 0;
   goto out;


out:
   return ret;
}

/* Driver loading.
*/
static int __init vc_smt_init( void )
{
   int                    ret = 0;
   VCOS_STATUS_T          status;
   VCOS_THREAD_ATTR_T     attrs;
   VCHI_INSTANCE_T        vchi_instance;
   VCHI_CONNECTION_T      *vchi_connection;

   LOG_INFO( "[%s]: start",
             __func__ );

   /* Allocate memory for the state structure.
   */
   smt_state = kzalloc( sizeof( *smt_state ), GFP_KERNEL );
   if ( smt_state == NULL )
   {
      LOG_ERR( "[%s]: failed to allocate memory", __func__ );

      ret = -ENOMEM;
      goto out;
   }

   memset ( smt_state, 0, sizeof( *smt_state ) );

   /* Initialize and create a VCHI connection for the shared memory service
   ** running on videocore.
   */
   ret = vchi_initialise( &vchi_instance );
   if ( ret != 0 )
   {
      LOG_ERR( "[%s]: failed to initialise VCHI instance (ret=%d)",
               __func__,
               ret );

      ret = -EIO;
      goto err_free_mem;
   }

   ret = vchi_connect( NULL, 0, vchi_instance );
   if ( ret != 0 )
   {
      LOG_ERR( "[%s]: failed to connect VCHI instance (ret=%d)",
               __func__,
               ret );

      ret = -EIO;
      goto err_free_mem;
   }

   /* Initialize an instance of the wifi hdmi service.
   */
   smt_state->wifihdmi_handle = vc_vchi_wifihdmi_init( vchi_instance, &vchi_connection, 1 );
   if ( smt_state->wifihdmi_handle == NULL )
   {
      LOG_ERR( "[%s]: failed to initialize shared memory service",
               __func__ );

      ret = -EPERM;
      goto err_free_mem;
   }

   // Lock for io events processing
   status = vcos_mutex_create( &smt_state->smt_lock, "smt_lock" );
   if ( status != VCOS_SUCCESS )
   {
      LOG_ERR( "%s: failed to create io-lock (status=%d)", __func__, status );
      goto err_free_mem;
   }

   // Event for io events processing
   status = vcos_event_create( &smt_state->smt_event, "" );
   if ( status != VCOS_SUCCESS )
   {
      LOG_ERR( "%s: failed to create io-event (status=%d)", __func__, status );
      goto err_delete_evt_lock;
   }

   /* Create a proc directory entry (root).
   */
   smt_state->dir_root = proc_mkdir( PROC_DIR_ROOT_NAME, NULL );
   if ( smt_state->dir_root == NULL )
   {
      LOG_ERR( "[%s]: failed to create \'%s\' directory entry",
               __func__,
               PROC_DIR_ROOT_NAME );

      ret = -EPERM;
      goto err_delete_event;
   }

   smt_state->debug = create_proc_entry( PROC_DEBUG,
                                         0,
                                         smt_state->dir_root );
   if ( smt_state->debug == NULL )
   {
      LOG_ERR( "[%s]: failed to create \'%s\' entry",
               __func__,
               PROC_DEBUG );

      ret = -EPERM;
      goto err_remove_proc_dir;
   }
   else
   {
      smt_state->debug->read_proc = &vc_smt_debug_proc_read;
      smt_state->debug->write_proc = &vc_smt_debug_proc_write;
   }

   smt_state->control = create_proc_entry( PROC_CONTROL,
                                           0,
                                           smt_state->dir_root );
   if ( smt_state->control == NULL )
   {
      LOG_ERR( "[%s]: failed to create \'%s\' entry",
               __func__,
               PROC_CONTROL );

      ret = -EPERM;
      goto err_remove_proc_debug;
   }
   else
   {
      smt_state->control->read_proc  = &vc_smt_ctl_proc_read;
      smt_state->control->write_proc = &vc_smt_ctl_proc_write;
   }

   smt_state->mode = create_proc_entry( PROC_MODE,
                                        0,
                                        smt_state->dir_root );
   if ( smt_state->mode == NULL )
   {
      LOG_ERR( "[%s]: failed to create \'%s\' entry",
               __func__,
               PROC_MODE );

      ret = -EPERM;
      goto err_remove_proc_control;
   }
   else
   {
      smt_state->mode->read_proc  = &vc_smt_mode_proc_read;
      smt_state->mode->write_proc = &vc_smt_mode_proc_write;
   }

   /* Create a shared memory test device.
   */
   ret = vc_smt_create_test();
   if ( ret != 0 )
   {
      LOG_ERR( "[%s]: failed to create shared memory test device",
               __func__);
      goto err_remove_proc_mode;
   }

   // Create the thread which takes care of all io to/from videoocore.
   vcos_thread_attr_init( &attrs );
   vcos_thread_attr_setstacksize( &attrs, 2048 );
   vcos_thread_attr_settimeslice( &attrs, 1 );

   // Create a thread to process the incoming log messages
   status = vcos_thread_create( &smt_state->smt_thread,
                                "SharedMemIntTest",
                                &attrs,
                                vc_smt_ops_waiter,
                                NULL );
   if ( status != VCOS_SUCCESS )
   {
      LOG_ERR( "[%s]: failed to create videocore io thread (status=%d)",
               __func__, status );

      vcos_assert( status == VCOS_SUCCESS );
      ret = -EPERM;
      goto err_remove_proc_smt_device;
   }

   /* Done!
   */
   goto out;


err_remove_proc_smt_device:
   vc_smt_remove_test();
err_remove_proc_mode:
   remove_proc_entry( PROC_MODE, smt_state->dir_root );
err_remove_proc_control:
   remove_proc_entry( PROC_CONTROL, smt_state->dir_root );
err_remove_proc_debug:
   remove_proc_entry( PROC_DEBUG, smt_state->dir_root );
err_remove_proc_dir:
   remove_proc_entry( PROC_DIR_ROOT_NAME, NULL );
err_delete_event:
   vcos_event_delete( &smt_state->smt_event );
err_delete_evt_lock:
   vcos_mutex_delete( &smt_state->smt_lock );
err_free_mem:
   kfree( smt_state );
out:
   LOG_INFO( "[%s]: end - returning %d",
             __func__,
             ret );
   return ret;
}

/* Driver unloading.
*/
static void __exit vc_smt_exit( void )
{
   LOG_INFO( "[%s]: start",
             __func__ );

   vc_smt_remove_test();

   /* Remove all proc entries.
   */
   remove_proc_entry( PROC_DEBUG,          smt_state->dir_root );
   remove_proc_entry( PROC_CONTROL,        smt_state->dir_root );
   remove_proc_entry( PROC_MODE,           smt_state->dir_root );
   remove_proc_entry( PROC_DIR_ROOT_NAME,  NULL );

   vcos_event_delete( &smt_state->smt_event );
   vcos_mutex_delete( &smt_state->smt_lock );

   /* Stop the videocore wifi-hdmi service.
   */
   vc_vchi_wifihdmi_end( &smt_state->wifihdmi_handle );

   /* Free the memory for the state structure.
   */
   kfree( smt_state );

   LOG_INFO( "[%s]: end",
             __func__ );
}

module_init( vc_smt_init );
module_exit( vc_smt_exit );

MODULE_AUTHOR( "Broadcom" );
MODULE_DESCRIPTION( "VideoCore SharedMemory Test Driver" );
MODULE_LICENSE( "GPL v2" );
