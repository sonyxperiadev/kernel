/*****************************************************************************
*  Copyright 2011 Broadcom Corporation.  All rights reserved.
*
*  Unless you and Broadcom execute a separate written software license
*  agreement governing use of this software, this software is licensed to you
*  under the terms of the GNU General Public License version 2, available at
*  http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
*
*  Notwithstanding the above, under no circumstances may you combine this
*  software in any way with any other Broadcom software provided under a
*  license other than the GPL, without Broadcom's express prior written
*  consent.
*
*****************************************************************************/

/* ---- Include Files ---------------------------------------------------- */
#include <linux/version.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/proc_fs.h>
#include <linux/device.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/ioctl.h>
#include <linux/cdev.h>
#include <linux/semaphore.h>
#include <linux/slab.h>
#include <linux/poll.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/timer.h>

#include <linux/in_route.h>
#include <linux/file.h>
#include <linux/udp.h>
#include <net/sock.h>
#include <net/ip.h>
#include <net/route.h>

#include <linux/broadcom/whdmi_ioctl.h>
#include <linux/broadcom/whdmi.h>

#include <asm/uaccess.h>

#include <interface/vcos/vcos.h>
#include <vc_sm_defs.h>
#include <vc_sm_knl.h>
#include <vc_vchi_wifihdmi.h>

/* ---- Private Constants and Types -------------------------------------- */

#define WHDMI_DEVICE_NAME       "wifi-hdmi"
#define WHDMI_MINOR_NUM         0
#define WHDMI_NUM_DEVICES       1

#define WHDMI_PACKET_HEADERS    (16 + 20 + 8)
#define WHDMI_PACKET_OVERHEAD   (WHDMI_PACKET_HEADERS + 4)

#define WHDMI_TX_STATS_DELAY_SEC    1
#define WHDMI_TX_STATS_SMOOTHING    0.9f

#define WHDMI_BUFFER_MAX_SIZE       1600

typedef enum whdmi_socket_type
{
    WHDMI_SOCKET_TYPE_TCP_LISTENING = 0,
    WHDMI_SOCKET_TYPE_TCP_CONN,
    WHDMI_SOCKET_TYPE_UDP_TX,
    WHDMI_SOCKET_TYPE_UDP
} WHDMI_SOCKET_TYPE;

/* An element of the linked list */
typedef struct whdmi_read_buffer_element
{
   struct whdmi_read_buffer_element         *next;
   int                                       msg_length;
   WHDMI_MSG_ALL                             msg;
} WHDMI_READ_BUFFER_ELEMENT;


typedef struct whdmi_read_buffer
{
   struct whdmi_read_buffer_element         *first;
   struct whdmi_read_buffer_element         *last;
} WHDMI_READ_BUFFER;


/* An element containing socket information */
typedef struct whdmi_socket_element
{
    struct whdmi_socket_element            *next;
    struct whdmi_socket_element            *prev;
    WHDMI_SOCKET_TYPE                       socket_type;
    int                                     km_socket_handle;
    int                                     socket_handle;
} WHDMI_SOCKET_ELEMENT;

/* Special socket element for UDP tx sockets */
typedef struct whdmi_socket_element_udp_tx
{
    WHDMI_SOCKET_ELEMENT                    element;
    struct socket                          *sock;
    unsigned long                           dest_addr_cached;
    unsigned short                          dest_port_cached;
    struct flowi                            fl_cached; 
} WHDMI_SOCKET_ELEMENT_UDP_TX;

typedef struct whdmi_socket_list
{
    struct whdmi_socket_element             *first;
    struct whdmi_socket_element             *last;
} WHDMI_SOCKET_LIST;

struct whdmi_tx_stats
{
    unsigned int        bytes_sent;
    float               average_byte_rate;
    struct timer_list   timer;
    unsigned long       connected_client;
};

/* ---- Private Function Prototypes -------------------------------------- */

static int          whdmi_open( struct inode *inode, struct file *file );
static int          whdmi_release( struct inode *inode, struct file *file );
static ssize_t      whdmi_read( struct file *file, char __user *buffer, size_t count, loff_t *ppos );
static long         whdmi_ioctl( struct file *file, unsigned int cmd, unsigned long arg );
static unsigned int whdmi_poll( struct file *file, struct poll_table_struct *poll_table );

static int          whdmi_proc_read( char *page, char **start, off_t off, int count, int *eof, void *data );
static int          whdmi_proc_write( struct file *file, const char __user *buffer, unsigned long count, void *data );

static int          chain_buffer_element( WHDMI_READ_BUFFER_ELEMENT *buf_element );
static void         free_buffer_elements( void );
static int          chain_socket_element( WHDMI_SOCKET_ELEMENT *socket_element );
static WHDMI_SOCKET_ELEMENT *remove_socket_element( WHDMI_SOCKET_ELEMENT *socket_element_to_remove );
static WHDMI_SOCKET_ELEMENT* find_socket_element_by_km_handle( int km_socket_handle );
static WHDMI_SOCKET_ELEMENT* find_socket_element_by_socket_handle( int socket_handle );
static void         free_socket_elements( void );

static void whdmi_timer_fn( unsigned long arg );
static int fast_udp_send_to( WHDMI_SOCKET_ELEMENT_UDP_TX *socket_element, unsigned long dest_addr, unsigned short dest_port,
                             int data_len, uint8_t *data );

/* ---- Private Variables ------------------------------------------------- */

static int gDriverMajor;
static struct cdev whdmi_cdev;
static dev_t whdmi_dev;
static struct semaphore sem;

static struct kmem_cache *kmem_cache = NULL;
static int open_count = 0;

static WHDMI_READ_BUFFER read_buffer;
static WHDMI_SOCKET_LIST socket_list;

static WHDMI_CALLBACK registered_callback = NULL;
static void *registered_callback_param = NULL;

static struct whdmi_tx_stats    tx_stats;

static VC_VCHI_WIFIHDMI_HANDLE_T whvcsvc_handle;

/* For debugging purposes */
static int dbg = 0;

#ifdef CONFIG_SYSFS
static struct class  * whdmi_class;
static struct device * whdmi_device;
static struct proc_dir_entry * proc_entry = NULL;
#endif

/* Wait Queue for implementing select() */
static DECLARE_WAIT_QUEUE_HEAD(read_wq);

/* File Operations (these are the device driver entry points) */
static struct file_operations gfops =
{
   .owner          = THIS_MODULE,
   .open           = whdmi_open,
   .release        = whdmi_release,
   .read           = whdmi_read,
   .unlocked_ioctl = whdmi_ioctl,
   .poll           = whdmi_poll,
};


/* ---- Public Variables ------------------------------------------------- */

/* ---- Functions -------------------------------------------------------- */

/***************************************************************************/
/**
*  Driver open routine
*
*  @return
*     0              Success
*     -ERESTARTSYS   Call interrupted
*/
static int whdmi_open(
   struct inode *inode,             /**< (io) Pointer to inode info */
   struct file  *file               /**< (io) File structure pointer */
)
{
    int err = 0;

    if ( down_interruptible( &sem ) )
    {
        /* Failed to acquire semaphore */
        return -ERESTARTSYS;
    }

    if ( open_count < 1 )
    {
        open_count++;
    }
    else
    {
        /* Device opened more than once, not permitted */
        err = -EPERM;
    }

    /* release the semaphore */
    up( &sem );

    if ( err == 0 )
    {
        /* Do any other initialization here */
        memset( &read_buffer, sizeof(read_buffer), 0 );
        memset( &socket_list, sizeof(socket_list), 0 );

    }

    return err;
}

/***************************************************************************/
/**
*  Driver release routine
*
*  @return
*     0        Success
*     -ve      Error code
*/
static int whdmi_release(
   struct inode *inode,             /**< (io) Pointer to inode info */
   struct file  *file               /**< (io) File structure pointer */
)
{
    int err = 0;
    WHDMI_READ_BUFFER_ELEMENT           *buf_element;

    if ( down_interruptible( &sem ) )
    {
        /* Failed to acquire semaphore */
        return -ERESTARTSYS;
    }

    /* Flush all pending messages to the user */
    buf_element = read_buffer.first;

    while( buf_element )
    {
        /* Free all the elements */
        read_buffer.first = buf_element->next;
        kmem_cache_free( kmem_cache, buf_element );
        buf_element = read_buffer.first;
    }

    memset( &read_buffer, 0, sizeof(read_buffer) );

    open_count--;

    /* release the semaphore */
    up( &sem );

    return err;
}

/***************************************************************************/
/**
*  Driver read method
*
*  @return
*     >= 0           Number of bytes read
*     -ve            Error code
*/
static ssize_t whdmi_read(
   struct file *file,
   char __user *buffer,
   size_t count,
   loff_t *ppos
)
{
    WHDMI_READ_BUFFER_ELEMENT           *buf_element;
    ssize_t                              bytes_to_copy;

    if ( dbg )
    {
        printk( KERN_INFO "WHDMI: whdmi_read() called\n" );
    }

    if ( down_interruptible( &sem ) )
    {
        /* Failed to acquire semaphore */
        return -ERESTARTSYS;
    }

    while ( read_buffer.first == NULL )
    {
        up( &sem );

        if ( file->f_flags & O_NONBLOCK )
        {
            /* Don't want to block so return EAGAIN */
            return -EAGAIN;
        }

        if ( wait_event_interruptible( read_wq, (read_buffer.first != NULL) ) )
        {
            return -ERESTARTSYS;
        }

        /* Re-acquire the lock before looping */
        if ( down_interruptible( &sem ) )
        {
            /* Failed to acquire semaphore */
            return -ERESTARTSYS;
        }
    }

    /* Dequeue the data */
    buf_element = read_buffer.first;
    read_buffer.first = buf_element->next;
    
    if ( read_buffer.last == buf_element )
    {
        /* Last element is this current one, so after dequing nothing should be left */
        read_buffer.last = NULL;
    }

    /* Can release the semaphore now */
    up( &sem );

    /* Copy the contents to the user buffer */
    bytes_to_copy = ( count < buf_element->msg_length) ? count : buf_element->msg_length;

    if ( copy_to_user( buffer, &buf_element->msg, bytes_to_copy ) )
    {
        /* Something bad happened while copying the data */
        kmem_cache_free( kmem_cache, buf_element );
        return -EFAULT;
    }
    else
    {
        /* Copy the data to the user buffer if appropriate as well */
        // copy_buffer_element_data( buf_element, buffer, count, bytes_to_copy )
    }

    /* Free the buffer element */
    kmem_cache_free( kmem_cache, buf_element );
   
    return bytes_to_copy;
}

/***************************************************************************/
/**
*  Driver ioctl method to support user library API.
*
*  @return
*     >= 0           Number of bytes write
*     -ve            Error code
*/
static long whdmi_ioctl(
   struct file   *file,             /**< (io) File structure pointer */
   unsigned int   cmd,              /**< (i)  IOCTL command */
   unsigned long  arg               /**< (i)  User argument */
)
{
    unsigned int                    cmdnr, size;
    int                             retval;
    union whdmi_ioctl_params        ioctl_params;
    WHDMI_SOCKET_ELEMENT           *socket_element;
    WHDMI_SOCKET_ELEMENT_UDP_TX    *socket_element_udp_tx;
    WHDMI_EVENT_PARAM               whdmi_event;
    int                             i;

    if ( _IOC_TYPE( cmd ) != WHDMI_MAGIC_TYPE )
    {
        /* Invalid magic type */
        printk( KERN_ERR "WHDMI: ioctl invalid type\n" );
        return -ENOTTY;
    }

    /* Grab the command number and size */
    cmdnr       = _IOC_NR( cmd );
    size        = _IOC_SIZE( cmd );

    if ( cmdnr > WHDMI_CMD_LAST )
    {
        /* Invalid command number */
        printk( KERN_ERR "WHDMI: ioctl invalid command number\n" );
        return -ENOTTY;
    }

    if ( size > sizeof(ioctl_params) )
    {
        /* Invalid size */
        printk( KERN_ERR "WHDMI: ioctl invalid size\n" );
        return -ENOTTY;
    }

    if ( ( _IOC_DIR( cmd ) & _IOC_READ ) && !access_ok( VERIFY_READ, (void __user *)arg, size ) )
    {
        /* Could not read from user space */
        printk( KERN_ERR "WHDMI: Could not read from user space\n" );
        return -EFAULT;
    }

    /* Read the params */
    memset( &ioctl_params, 0, sizeof( ioctl_params ) );
    retval = copy_from_user( &ioctl_params, (void __user *)arg, size );
    if ( retval != 0 )
    {
        /* Failed to copy from user space */
        printk( KERN_ERR "WHDMI: Could not copy from user space\n" );
        return retval;
    }

    switch ( cmdnr )
    {
        case WHDMI_CMD_SOCKET_CREATED:
            /* Find the created socket based on socket handle */
            socket_element = find_socket_element_by_socket_handle( ioctl_params.socket_created.socket_handle );

            if ( socket_element != NULL )
            {
                /* If we found a matching socket element, then the socket already exists! */
                printk( KERN_ERR "WHDMI: ioctl WHDMI_CMD_SOCKET_CREATED created an existing socket %d!\n", socket_element->socket_handle );
                return -EFAULT;
            }

            /* Find the socket element based on the Kernel module handle (whdmi handle) */
            socket_element = find_socket_element_by_km_handle( ioctl_params.socket_created.whdmi_msg_handle );

            if ( socket_element == NULL )
            {
                /* We could not find the matching request */
                printk( KERN_ERR "WHDMI: ioctl WHDMI_CMD_SOCKET_CREATED called on unsuspecting message handle!\n" );
                return -EFAULT;
            }

            /* Update the socket handle */
            socket_element->socket_handle = ioctl_params.socket_created.socket_handle;

            /* Get a reference to the sock struct for UDP TX sockets */
            if ( socket_element->socket_type == WHDMI_SOCKET_TYPE_UDP_TX )
            {
                /* Socket element is actually a UDP TX element */
                socket_element_udp_tx = (WHDMI_SOCKET_ELEMENT_UDP_TX *)socket_element;

                socket_element_udp_tx->sock = sockfd_lookup( ioctl_params.socket_created.socket_handle, &retval );

                if ( !socket_element_udp_tx->sock )
                {
                   printk( KERN_ERR "WHDMI: ioctl WHDMI_CMD_SOCKET_CREATED could not call sockfd_lookup!\n" );
                   return retval;
                }

                /* Don't keep the fd locked */
                sockfd_put( socket_element_udp_tx->sock );
            }

            if (dbg)
            {
                printk( KERN_INFO "WHDMI: ioctl WHDMI_CMD_SOCKET_CREATED successfully created socket handle %d!\n", ioctl_params.socket_created.socket_handle );
            }

            break;
        case WHDMI_CMD_SOCKET_ACCEPTED:
            if (dbg)
            {
                printk( KERN_INFO "WHDMI: ioctl WHDMI_CMD_SOCKET_ACCEPTED socket handle %d from parent %d. Client %lu.%lu.%lu.%lu:%d\n",
                        ioctl_params.socket_accepted.socket_handle,
                        ioctl_params.socket_accepted.parent_socket_handle,
                        (ioctl_params.socket_accepted.client_addr >> 24) & 0x000000FF,
                        (ioctl_params.socket_accepted.client_addr >> 16) & 0x000000FF,
                        (ioctl_params.socket_accepted.client_addr >> 8) & 0x000000FF,
                        ioctl_params.socket_accepted.client_addr & 0x000000FF,
                        ioctl_params.socket_accepted.client_port );
            }

            /* Find the accepted socket based on socket handle */
            socket_element = find_socket_element_by_socket_handle( ioctl_params.socket_accepted.socket_handle );

            if ( socket_element != NULL )
            {
                /* If we found a matching socket element, then the socket already exists! */
                printk( KERN_ERR "WHDMI: ioctl WHDMI_CMD_SOCKET_ACCEPTED accepted an existing socket %d!\n", socket_element->socket_handle );
                return -EFAULT;
            }

            /* Verify the parent socket exists */
            socket_element = find_socket_element_by_socket_handle( ioctl_params.socket_accepted.parent_socket_handle );

            if ( socket_element == NULL )
            {
                /* If we can't find the parent socket, then we can't accept it! */
                printk( KERN_ERR "WHDMI: ioctl WHDMI_CMD_SOCKET_ACCEPTED unknown parent socket %d!\n", ioctl_params.socket_accepted.parent_socket_handle );
                return -EFAULT;
            }

            /* Call the callback function to notify of incoming socket */
            memset( &whdmi_event, 0, sizeof(whdmi_event) );
            whdmi_event.socket_incoming.parent_km_socket_handle = socket_element->km_socket_handle;
            whdmi_event.socket_incoming.client_addr = ioctl_params.socket_accepted.client_addr;
            whdmi_event.socket_incoming.client_port = ioctl_params.socket_accepted.client_port;

            /* Prior to callback, make sure we have allocated the socket element */
            socket_element = (WHDMI_SOCKET_ELEMENT *)kmalloc(sizeof(WHDMI_SOCKET_ELEMENT), GFP_KERNEL);

            if ( !socket_element )
            {
                /* No memory for the socket element */
                printk( KERN_ERR "WHDMI: Failed kmalloc!\n" );
                return -ENOMEM;
            }

            /* Callback to notify the incoming socket connection. This is blocking call. */
            if ( registered_callback )
            {
                registered_callback( WHDMI_EVENT_SOCKET_INCOMING, &whdmi_event, registered_callback_param );
            }
            else
            {
                /* For now, fake a socket handle */
                i = 1;

                while ( 1 )
                {
                    /* Try find a free km handle from 0 */
                    if ( find_socket_element_by_km_handle( i ) == NULL )
                    {
                        break;
                    }
                    i++;
                }

                whdmi_event.socket_incoming.km_socket_handle_returned = i;
            }

            /* Once the callback is returned, we will see what handle is provided */
            if ( whdmi_event.socket_incoming.km_socket_handle_returned )
            {
                /* At this point, we are ready to add this socket to our socket list */

                /* Zero the socket element */
                memset( socket_element, 0, sizeof( WHDMI_SOCKET_ELEMENT ) );

                /* Fill in the contents of the socket element */
                socket_element->socket_type             = WHDMI_SOCKET_TYPE_TCP_CONN;
                socket_element->km_socket_handle        = whdmi_event.socket_incoming.km_socket_handle_returned;
                socket_element->socket_handle           = ioctl_params.socket_accepted.socket_handle;

                /* Chain the socket element */
                chain_socket_element( socket_element );

                /* Store the client connection info in network presentation order */
                tx_stats.connected_client = htonl(ioctl_params.socket_accepted.client_addr);
            }
            else
            {
                /* We have failed to accept the socket */
                kfree( socket_element ); 

                return -EFAULT;
            }

            break;

        case WHDMI_CMD_SOCKET_DISCONNECTED:
            if (dbg)
            {
                printk( KERN_INFO "WHDMI: ioctl WHDMI_CMD_SOCKET_DISCONNECTED socket handle %d\n",
                        ioctl_params.socket_disconnected.socket_handle );
            }

            /* Find the socket element */
            socket_element = find_socket_element_by_socket_handle( ioctl_params.socket_disconnected.socket_handle );

            if ( socket_element == NULL )
            {
                /* If we can't find the parent socket, then we can't accept it! */
                printk( KERN_ERR "WHDMI: ioctl WHDMI_CMD_SOCKET_DISCONNECTED unknown socket %d!\n", ioctl_params.socket_disconnected.socket_handle );
                return -EFAULT;
            }

            /* Clear the client connection information */
            tx_stats.connected_client = 0;

            /* Remove the socket element */
            remove_socket_element( socket_element );

            /* Prepare the callback event */
            memset( &whdmi_event, 0, sizeof(whdmi_event) );
            whdmi_event.socket_disconnected.km_socket_handle = socket_element->km_socket_handle;

            if ( registered_callback )
            {
                registered_callback( WHDMI_EVENT_SOCKET_DISCONNECTED, &whdmi_event, registered_callback_param );
            }

            /* Free the socket element */
            kfree( socket_element );

            break;

        case WHDMI_CMD_SOCKET_DATA_AVAIL:

            /* Find the socket element */
            socket_element = find_socket_element_by_socket_handle( ioctl_params.socket_data_avail.socket_handle );

            if ( socket_element == NULL )
            {
                /* If we can't find the parent socket, then we can't accept it! */
                printk( KERN_ERR "WHDMI: ioctl WHDMI_CMD_SOCKET_DATA_AVAIL unknown socket %d!\n", ioctl_params.socket_data_avail.socket_handle );
                return -EFAULT;
            }

            if ( !access_ok( VERIFY_READ, (void __user *)ioctl_params.socket_data_avail.data, ioctl_params.socket_data_avail.data_len ) )
            {
                /* Could not read from user space */
                printk( KERN_ERR "WHDMI: Could not read from user space\n" );
                return -EFAULT;
            }

            if (dbg)
            {
                uint8_t     *packet_buffer = NULL;
                char        data_buffer[50];
                int         i;

                packet_buffer = kzalloc( WHDMI_BUFFER_MAX_SIZE * sizeof(uint8_t), GFP_KERNEL );
                if ( packet_buffer != NULL )
                {
                   /* Copy data from user and print the content */
                   retval = copy_from_user( packet_buffer, (void __user *)ioctl_params.socket_data_avail.data, ioctl_params.socket_data_avail.data_len );
                   if ( retval == 0 )
                   {
                      printk( KERN_INFO "WHDMI: ioctl WHDMI_CMD_SOCKET_DATA_AVAIL socket handle %d, data_len %d\n",
                              ioctl_params.socket_data_avail.socket_handle,
                              ioctl_params.socket_data_avail.data_len );

                      /* Format and print the data in xx:xx:xx:xx format */
                      /* Can use http://home2.paulschou.net/tools/xlate/ to convert to ASCII */
                      for ( i = 0; i < ioctl_params.socket_data_avail.data_len; i++ )
                      {
                          if ( (i % 16 ) == 0 )
                          {
                              if ( i != 0 )
                              {
                                  printk( KERN_INFO "%s\n", data_buffer );
                              }
                              memset( data_buffer, 0, sizeof( data_buffer ) );
                          }

                          sprintf( data_buffer, "%s%02x:", data_buffer, packet_buffer[i] );
                      }
                      printk( KERN_INFO "%s\n", data_buffer );
                   }

                   kfree( packet_buffer );
                   packet_buffer = NULL;
                }
            }

            /* Prepare the callback event */
            memset( &whdmi_event, 0, sizeof(whdmi_event) );
            whdmi_event.socket_data_avail.km_socket_handle  = socket_element->km_socket_handle;
            whdmi_event.socket_data_avail.data_len          = ioctl_params.socket_data_avail.data_len;
            whdmi_event.socket_data_avail.data              = (void __user *)ioctl_params.socket_data_avail.data;

            if ( registered_callback )
            {
                registered_callback( WHDMI_EVENT_SOCKET_DATA_AVAIL, &whdmi_event, registered_callback_param );
            }

            break;

        case WHDMI_CMD_SOCKET_CLOSED:

            /* Find the socket element */
            socket_element = find_socket_element_by_socket_handle( ioctl_params.socket_closed.socket_handle );

            if ( socket_element )
            {
                /* Found the socket element of interest */
                remove_socket_element( socket_element );

                /* Prepare callback event */
                memset( &whdmi_event, 0, sizeof(whdmi_event) );
                whdmi_event.socket_closed.km_socket_handle  = socket_element->km_socket_handle;

                if ( registered_callback )
                {
                    registered_callback( WHDMI_EVENT_SOCKET_CLOSED, &whdmi_event, registered_callback_param );
                }

                /* Free the socket element */
                kfree( socket_element );
            }
            else
            {
                /* If we can't find the parent socket, then we can't accept it! */
                printk( KERN_ERR "WHDMI: ioctl WHDMI_CMD_SOCKET_CLOSED unknown socket %d!\n", ioctl_params.socket_closed.socket_handle );
                return -EFAULT;
            }
            break;

        case WHDMI_CMD_START_SERVICE:
        case WHDMI_CMD_STOP_SERVICE:
            /* Both commands should require us to start in a clean slate, so close all existing sockets */
            free_buffer_elements();
            free_socket_elements();

            if ( cmdnr == WHDMI_CMD_START_SERVICE )
            {
                /* Clear the tx stats counter */
                tx_stats.bytes_sent = 0;
                tx_stats.average_byte_rate = 0;

                /* Prepare to start the timer */
                tx_stats.timer.expires = jiffies + (HZ * WHDMI_TX_STATS_DELAY_SEC);

                /* Kick-start the timer */
                add_timer(&tx_stats.timer);

                /* Prepare callback event */
                memset( &whdmi_event, 0, sizeof(whdmi_event) );

                if ( registered_callback )
                {
                    registered_callback( WHDMI_EVENT_START_SERVICE, &whdmi_event, registered_callback_param );
                }
            }
            else
            {
                /* Stop the timer */
                del_timer(&tx_stats.timer);

                /* Prepare callback event */
                memset( &whdmi_event, 0, sizeof(whdmi_event) );

                if ( registered_callback )
                {
                    registered_callback( WHDMI_EVENT_STOP_SERVICE, &whdmi_event, registered_callback_param );
                }
            }
            break;

        default:
            /* Unhandled command */
            return -ENOTTY;
    }

   return 0;
}

/***************************************************************************/
/**
*  Driver poll method
*
*  @return
*     mask        Mask containing operations that can be performed without blocking
*/
static unsigned int whdmi_poll(
   struct file *file,
   struct poll_table_struct *poll_table
)
{
    unsigned int mask = 0;

    down( &sem );

    poll_wait( file, &read_wq, poll_table);

    if ( read_buffer.first != NULL )
    {
        mask |= POLLIN | POLLRDNORM;        /* readable */
    }

    up( &sem );

    return mask;
}

/***************************************************************************/
/**
*  Chain the socket element to socket element list
*
*  @return
*    0          if everything is successful
*    -ve        if something bad happened
*/
static int chain_socket_element(
    WHDMI_SOCKET_ELEMENT        *socket_element
)
{
    /* Lock semaphore and chain socket */
    if ( down_interruptible( &sem ) )
    {
        /* Failed to acquire semaphore */
        kfree( socket_element );
        return -ERESTARTSYS;
    }

    /* Clear the socket element's prevoius and next element */
    socket_element->prev = NULL;
    socket_element->next = NULL;

    /* Add it to the queue */ 
    if ( !socket_list.first )
    {
        /* This is the first element to queue up */
        socket_list.first   = socket_element;
        socket_list.last    = socket_element;
    }
    else
    {
        /* Make sure the last element exists */
        if ( socket_list.last )
        {
            socket_list.last->next = socket_element;
            socket_element->prev = socket_list.last;
            socket_list.last = socket_element;
        }
        else
        {
            /* Something is really messed up!! */
            printk( KERN_ERR "WHDMI: Unexpected last element!\n" );
            kfree( socket_element );
        }
    }

    up( &sem );

    return 0;
}

/***************************************************************************/
/**
*  Finds the socket element by Kernel Module's socket handle 
*
*  @return
*     struct socket_element* matching the Kernel Module's socket handle 
*     null if not found
*/
static WHDMI_SOCKET_ELEMENT *find_socket_element_by_km_handle(
    int                      km_socket_handle
)
{
    WHDMI_SOCKET_ELEMENT    *socket_element = NULL;

    /* Lock semaphore and chain socket */
    if ( down_interruptible( &sem ) )
    {
        /* Failed to acquire semaphore */
        return NULL;
    }

    socket_element = socket_list.first;
 
    while (socket_element)
    {
        if ( socket_element->km_socket_handle == km_socket_handle )
        {
            /* found the matching socket element */
            break;
        }

        socket_element = socket_element->next;
    }

    up( &sem );

    return socket_element;
}

/***************************************************************************/
/**
*  Finds the socket element by socket handle
*
*  @return
*     struct socket_element* matching the socket handle
*     null if not found
*/
static WHDMI_SOCKET_ELEMENT *find_socket_element_by_socket_handle(
    int                      socket_handle
)
{
    WHDMI_SOCKET_ELEMENT    *socket_element = NULL;

    /* Lock semaphore and chain socket */
    if ( down_interruptible( &sem ) )
    {
        /* Failed to acquire semaphore */
        return NULL;
    }

    socket_element = socket_list.first;

    while (socket_element)
    {

        if (socket_element->socket_handle == socket_handle)
        {
            /* found the matching socket element */
            break;
        }

        socket_element = socket_element->next;
    }

    up( &sem );

    return socket_element;
}

/***************************************************************************/
/**
*  Removes the socket element
*
*  @return
*     struct socket_element* that has been dequeued
*     null if not found
*/
static WHDMI_SOCKET_ELEMENT *remove_socket_element(
    WHDMI_SOCKET_ELEMENT    *socket_element_to_remove
)
{
    WHDMI_SOCKET_ELEMENT    *socket_element = NULL;

    /* Lock semaphore and chain socket */
    if ( down_interruptible( &sem ) )
    {
        /* Failed to acquire semaphore */
        return NULL;
    }

    socket_element = socket_list.first;

    while (socket_element)
    {
        if (socket_element == socket_element_to_remove)
        {
            /* found the matching socket element */

            /* If the socket to remove is the first element */
            if ( socket_list.first == socket_element )
            {
                socket_list.first = socket_element->next;
            }

            /* If the socket to remove is the last element */
            if ( socket_list.last == socket_element )
            {
                socket_list.last = socket_element->prev;
            }

            /* If the previous element exists */
            if ( socket_element->prev )
            {
                socket_element->prev->next = socket_element->next;
            }
             
            /* If the next element exists */
            if ( socket_element->next )
            {
                socket_element->next->prev = socket_element->prev;
            }

            break;
        }

        socket_element = socket_element->next;
    }

    up( &sem );

    return socket_element;
}


/***************************************************************************/
/**
*  Free all socket elements 
*
*  @return
*   Nothing
*/
static void free_socket_elements( void )
{
    WHDMI_SOCKET_ELEMENT    *socket_element = NULL;
    WHDMI_SOCKET_ELEMENT    *socket_element_to_free;

    /* Lock semaphore and chain socket */
    if ( down_interruptible( &sem ) )
    {
        /* Failed to acquire semaphore */
        return;
    }

    socket_element = socket_list.first;

    while (socket_element)
    {
        /* Prepare the next element */
        socket_element_to_free = socket_element;
        socket_element = socket_element->next;

        /* Free the element */
        kfree(socket_element_to_free);
    }

    socket_list.first   = NULL;
    socket_list.last    = NULL;

    up( &sem );
}

/***************************************************************************/
/**
*  Chain the buffer element to read queue 
*
*  @return
*    0          if everything is successful
*    -ve        if something bad happened
*/
static int chain_buffer_element(
    WHDMI_READ_BUFFER_ELEMENT              *buf_element
)
{
    /* Lock semaphore and chain packet */
    if ( down_interruptible( &sem ) )
    {
        /* Failed to acquire semaphore */
        kmem_cache_free( kmem_cache, buf_element );
        return -ERESTARTSYS;
    }

    /* Add it to the queue */ 
    if ( !read_buffer.first )
    {
        /* This is the first element to queue up */
        read_buffer.first   = buf_element;
        read_buffer.last    = buf_element;
    }
    else
    {
        /* Make sure the last element exists */
        if ( read_buffer.last )
        {
            read_buffer.last->next = buf_element;
            read_buffer.last = buf_element;
        }
        else
        {
            /* Something is really messed up!! */
            printk( KERN_ERR "WHDMI: Unexpected last element!\n" );
            kmem_cache_free( kmem_cache, buf_element );
        }
    }

    up( &sem );

    /* Wake up any reader */
    wake_up_interruptible( &read_wq );

    return 0;
}

/***************************************************************************/
/**
*  Free buffer elements 
*
*  @return
*   Nothing
*/
static void free_buffer_elements( void )
{
     WHDMI_READ_BUFFER_ELEMENT              *buf_element = NULL;
     WHDMI_READ_BUFFER_ELEMENT              *buf_element_to_free;

    /* Lock semaphore and chain packet */
    if ( down_interruptible( &sem ) )
    {
        return;
    }

    buf_element = read_buffer.first;

    while( buf_element )
    {
        buf_element_to_free = buf_element;
        buf_element = buf_element->next;

        kmem_cache_free( kmem_cache, buf_element_to_free );
    }

    read_buffer.first = NULL;
    read_buffer.last = NULL;

    up( &sem );
}

/***************************************************************************/
/**
*  Read from proc entry
*
*  @return
*     0           Everything ok
*    -ve          Something bad happened
*/
static int whdmi_proc_read(
    char *page,
    char **start,
    off_t off,
    int count,
    int *eof,
    void *data
)
{
    int len = 0;

    len += sprintf( &page[len], "Connected client: %pI4\n", &tx_stats.connected_client );
    len += sprintf( &page[len], "Average TX Rate: %d bits/s\n", (unsigned int)(tx_stats.average_byte_rate * 8) );
    *eof = 1;

    return len;
}

/***************************************************************************/
/**
*  Write to proc entry 
*
*  @return
*     0           Everything ok
*    -ve          Something bad happened
*/
static int whdmi_proc_write(
    struct file *file,
    const char __user *buffer,
    unsigned long count,
    void *data
)
{
#define CMD_CREATE_TCP_LISTENING_SOCKET     "whdmi_create_tcp_listening_socket"
#define CMD_CREATE_UDP_SOCKET               "whdmi_create_udp_socket"
#define CMD_CREATE_UDP_TX_SOCKET            "whdmi_create_udp_tx_socket"
#define CMD_TCP_SEND                        "whdmi_tcp_send"
#define CMD_UDP_SEND_TO                     "whdmi_udp_send_to"
#define CMD_UDP_BULK_SEND_TO                "whdmi_udp_bulk_send_to"
#define CMD_CLOSE_SOCKET                    "whdmi_close_socket"

    unsigned char *local_buffer = NULL;
    int read_count = 0;
    int retval = 0;

    /* Arguments to various commands */
    int             km_socket_handle;
    unsigned short  port;
    unsigned long   dest_addr;
    short unsigned int dest_addr_array[4];
    unsigned short  dest_port;
    uint8_t        *data_ptr;
    int             queue_len;
    int             data_len;
    int             i;

    /* For bulk tx */
    int             bulk_tx;
    int             bulk_send_count;

    local_buffer = kzalloc ( WHDMI_BUFFER_MAX_SIZE * sizeof(uint8_t), GFP_KERNEL );
    if ( local_buffer == NULL )
    {
        printk(KERN_ERR "WHDMI: failed allocating proc buffer\n" );
        return -ENOMEM;
    }

    if ( count > WHDMI_BUFFER_MAX_SIZE - 1 )
    {
        printk(KERN_ERR "WHDMI: proc write failed, max length = %d\n", sizeof(local_buffer) );
        return -ENOMEM;
    }

    memset(local_buffer, 0, WHDMI_BUFFER_MAX_SIZE);

    if ( copy_from_user( local_buffer, buffer, count ) )
    {
        return -EFAULT;
    }

    /* Parse through the commands */
    if ( !strncmp( "cmd=", &local_buffer[read_count], strlen("cmd=") ) )
    {
        read_count += strlen("cmd=");

        if ( !strncmp( CMD_CREATE_TCP_LISTENING_SOCKET, &local_buffer[read_count], strlen(CMD_CREATE_TCP_LISTENING_SOCKET) ) )
        {
            read_count += strlen(CMD_CREATE_TCP_LISTENING_SOCKET) + 1;

            if ( sscanf( &local_buffer[read_count], "%d %hu %d", &km_socket_handle, &port, &queue_len ) == 3 )
            {
                retval = whdmi_create_tcp_listening_socket( km_socket_handle, port, queue_len );
                if (retval)
                    printk(KERN_INFO "WHDMI: %s returns %d\n", CMD_CREATE_TCP_LISTENING_SOCKET, retval);
            }
            else
            {
                printk(KERN_ERR "WHDMI: Failed to read input arguments\n" );
                return -EFAULT;
            }
        }
        else if ( !strncmp( CMD_CREATE_UDP_SOCKET, &local_buffer[read_count], strlen(CMD_CREATE_UDP_SOCKET) ) )
        {
            read_count += strlen(CMD_CREATE_UDP_SOCKET) + 1;

            if ( sscanf( &local_buffer[read_count], "%d %hu %d", &km_socket_handle, &port, &queue_len ) == 3 )
            {
                retval = whdmi_create_udp_socket( km_socket_handle, port, queue_len );
                if (retval)
                    printk(KERN_INFO "WHDMI: %s returns %d\n", CMD_CREATE_UDP_SOCKET, retval);
            }
            else
            {
                printk(KERN_ERR "WHDMI: Failed to read input arguments\n" );
                return -EFAULT;
            }
        }
        else if ( !strncmp( CMD_CREATE_UDP_TX_SOCKET, &local_buffer[read_count], strlen(CMD_CREATE_UDP_TX_SOCKET) ) )
        {
            read_count += strlen(CMD_CREATE_UDP_TX_SOCKET) + 1;

            if ( sscanf( &local_buffer[read_count], "%d %hu %d", &km_socket_handle, &port, &queue_len ) == 3 )
            {
                retval = whdmi_create_udp_tx_socket( km_socket_handle, port, queue_len );
                if (retval)
                    printk(KERN_INFO "WHDMI: %s returns %d\n", CMD_CREATE_UDP_TX_SOCKET, retval);
            }
            else
            {
                printk(KERN_ERR "WHDMI: Failed to read input arguments\n" );
                return -EFAULT;
            }
        }
        else if ( !strncmp( CMD_TCP_SEND, &local_buffer[read_count], strlen(CMD_TCP_SEND) ) )
        {
            read_count += strlen( CMD_TCP_SEND ) + 1;
            if ( sscanf( &local_buffer[read_count], "%d %d ", &km_socket_handle, &data_len ) == 2 )
            {
                /* Search for where the data starts */
                data_ptr = (uint8_t *)strstr( &local_buffer[read_count], "data=" );

                if ( data_ptr == NULL )
                {
                    printk(KERN_ERR "WHDMI: Could not find data to send\n" );
                    return -EFAULT;
                }

                data_ptr += strlen("data=");

                retval = whdmi_tcp_send( km_socket_handle, data_len, data_ptr );
                if (retval)
                    printk(KERN_INFO "WHDMI: %s returns %d\n", CMD_TCP_SEND, retval);
            }
            else
            {
                printk(KERN_ERR "WHDMI: Failed to read input arguments\n" );
                return -EFAULT;
            }
        }
        else if ( !strncmp( CMD_UDP_SEND_TO, &local_buffer[read_count], strlen(CMD_UDP_SEND_TO) ) || 
                  !strncmp( CMD_UDP_BULK_SEND_TO, &local_buffer[read_count], strlen(CMD_UDP_BULK_SEND_TO) ) )
        {

            if ( !strncmp( CMD_UDP_SEND_TO, &local_buffer[read_count], strlen(CMD_UDP_SEND_TO) ) )
            {
                read_count += strlen( CMD_UDP_SEND_TO ) + 1;
                bulk_tx = 0;

                if ( sscanf( &local_buffer[read_count], "%d %3hu.%3hu.%3hu.%3hu:%hu %d ",
                             &km_socket_handle,
                             &dest_addr_array[0],
                             &dest_addr_array[1],
                             &dest_addr_array[2],
                             &dest_addr_array[3],
                             &dest_port,
                             &data_len ) != 7 )
                {
                    printk(KERN_ERR "WHDMI: Failed to read input arguments\n" );
                    return -EFAULT;
                }
            }
            else
            {
                read_count += strlen( CMD_UDP_BULK_SEND_TO ) + 1;
                bulk_tx = 1;

                if ( sscanf( &local_buffer[read_count], "%d %3hu.%3hu.%3hu.%3hu:%hu %d %d ",
                             &km_socket_handle,
                             &dest_addr_array[0],
                             &dest_addr_array[1],
                             &dest_addr_array[2],
                             &dest_addr_array[3],
                             &dest_port,
                             &bulk_send_count,
                             &data_len ) != 8 )
                {
                    printk(KERN_ERR "WHDMI: Failed to read input arguments\n" );
                    return -EFAULT;
                }
            }

            dest_addr = ((dest_addr_array[0] & 0x00FF) << 24) | ((dest_addr_array[1] & 0x00FF) << 16) | (( dest_addr_array[2] & 0x00FF) << 8) | (dest_addr_array[3] & 0x00FF);

            /* Search for where the data starts */
            data_ptr = (uint8_t *)strstr( &local_buffer[read_count], "data=" );

            if ( data_ptr == NULL )
            {
                printk(KERN_ERR "WHDMI: Could not find data to send\n" );
                return -EFAULT;
            }

            data_ptr += strlen("data=");

            if ( !bulk_tx )
            {
                retval = whdmi_udp_send_to( km_socket_handle, dest_addr, dest_port, data_len, data_ptr );
            }
            else
            {
                /* Perform bulk send of packets */
                for (i = 0; i < bulk_send_count; i++)
                {
                    retval = whdmi_udp_send_to( km_socket_handle, dest_addr, dest_port, data_len, data_ptr );
                    if (retval)
                        break;
                }
            }
            if (retval)
            {
                if (bulk_tx)
                {
                    printk(KERN_INFO "WHDMI: %s returns %d\n", CMD_UDP_BULK_SEND_TO, retval);
                }
                else
                {
                    printk(KERN_INFO "WHDMI: %s returns %d\n", CMD_UDP_SEND_TO, retval);
                }
            }
        }
        else if ( !strncmp( CMD_CLOSE_SOCKET, &local_buffer[read_count], strlen(CMD_CLOSE_SOCKET) ) )
        {
            read_count += strlen(CMD_CLOSE_SOCKET) + 1;

            if ( sscanf( &local_buffer[read_count], "%d", &km_socket_handle ) == 1 )
            {
                retval = whdmi_close_socket( km_socket_handle );
                if (retval)
                    printk(KERN_INFO "WHDMI: %s returns %d\n", CMD_CLOSE_SOCKET, retval);
            }
            else
            {
                printk(KERN_ERR "WHDMI: Failed to read input arguments\n" );
                return -EFAULT;
            }
        }
    }
    else if ( !strncmp( "dbg=", &local_buffer[read_count], strlen("dbg=") ) )
    {
        read_count += 4;
        if ( sscanf( &local_buffer[read_count], "%d", &dbg ) != 1 )
        {
            return -EFAULT;
        }
    }
    else if ( !strncmp( "vcstats", &local_buffer[read_count], strlen("vcstats") ) )
    {
       VC_WIFIHDMI_MODE_T mode;
       VC_WIFIHDMI_STATS_T stats;
       uint32_t trans_id;

       memset ( &mode, 0, sizeof(mode) );
       memset ( &stats, 0, sizeof(stats) );

       mode.wifihdmi = 1;
       vc_vchi_wifihdmi_stats( whvcsvc_handle,
                               &mode, 
                               &stats,
                               &trans_id );

       printk( KERN_INFO "stats tx:%d, tx-frames:%d, tx-dst-cnt:%d, tx-missed:%d, tx-busy:%d, tx-rec:%d\n",
               stats.tx_cnt,
               stats.tx_frame_cnt,
               stats.tx_dst_cnt,
               stats.tx_miss_cnt,
               stats.tx_busy_cnt,
               stats.tx_rec_cnt );
    }

    kfree ( local_buffer );
    local_buffer = NULL;

    return count;
}

/***************************************************************************/
/**
*  Platform support constructor
*/
static int __init whdmi_init( void )
{
   int err;
   VCHI_INSTANCE_T vchi_instance;
   VCHI_CONNECTION_T *vchi_connection;

   // printk( KERN_ERR "%s: setting up.\n", __FUNCTION__ );

   /* Allocate the major number */
   err = alloc_chrdev_region( &whdmi_dev,
                               WHDMI_MINOR_NUM,
                               WHDMI_NUM_DEVICES,
                               WHDMI_DEVICE_NAME );
   if ( err < 0 )
   {
      printk( KERN_ERR "WHDMI: Failed to allocate device major number\n" );
      err = -EFAULT;
      goto err_return;
   }
   gDriverMajor = MAJOR( whdmi_dev );

   /* Setup the cdev structure */
   cdev_init( &whdmi_cdev, &gfops );
   whdmi_cdev.owner = THIS_MODULE;
   whdmi_cdev.ops = &gfops;

   err = cdev_add( &whdmi_cdev,
                    whdmi_dev,
                    WHDMI_NUM_DEVICES );
   if ( err < 0 )
   {
      printk( KERN_ERR "WHDMI: Failed to add character device\n" );
      err = -EFAULT;
      goto err_unregister_chrdev_region;
   }

#ifdef CONFIG_SYSFS
   // printk( KERN_ERR "%s: creating class.\n", __FUNCTION__ );
   whdmi_class = class_create( THIS_MODULE, "bcm-wifi-hdmi" );
   if(IS_ERR( whdmi_class ))
   {
      printk(KERN_ERR "WHDMI: Class create failed\n");
      err = -EFAULT;
      goto err_cdev_del;
   }

   // printk( KERN_ERR "%s: creating device: %s.\n", __FUNCTION__, WHDMI_DEVICE_NAME );
   whdmi_device = device_create( whdmi_class,
                                 NULL,
                                 whdmi_dev,
                                 NULL,
                                 WHDMI_DEVICE_NAME );
   if( IS_ERR(whdmi_device) )
   {
      printk(KERN_ERR "WHDMI: Device create failed\n");
      err = -EFAULT;
      goto err_class_destroy;
   }

   /* Create the proc entry */
   proc_entry = create_proc_entry( WHDMI_DEVICE_NAME,
                                   (S_IWUSR | S_IWGRP | S_IRUSR | S_IRGRP ),
                                    NULL );
   if ( proc_entry )
   {
      proc_entry->write_proc = whdmi_proc_write;
      proc_entry->read_proc = whdmi_proc_read;
   }
   else
   {
      printk(KERN_ERR "WHDMI: Proc entry create failed\n");
      err = -EFAULT;
      goto err_dev_destroy;
   }

#endif

   /* Initialize the semaphore that will be used by the driver */
   sema_init( &sem, 1 );

   /* Allocate cache manager for messages to user space */
   if ( !kmem_cache )
   {
      kmem_cache = kmem_cache_create( WHDMI_DEVICE_NAME,
                                      sizeof(WHDMI_READ_BUFFER_ELEMENT),    /* size of each cache element */
                                      0,                                    /* offset */
                                      0,                                    /* flags */
                                      NULL );                               /* constructor/destructor */

      if( !kmem_cache )
      {
         printk(KERN_ERR "WHDMI: Failed to allocate memory cache\n");
         err = -ENOMEM;
         goto err_proc_destroy;
      }
   }

   /* Prepare the stats structure, and setup the timer */
   memset( &tx_stats, 0, sizeof(tx_stats) );
   init_timer( &tx_stats.timer );
   tx_stats.timer.data = (unsigned long)&tx_stats;
   tx_stats.timer.function = whdmi_timer_fn;

   /* Initialize an instance of the wifi hdmi videocore service.
   */
   err = vchi_initialise( &vchi_instance );
   if ( err != 0 )
   {
      printk(KERN_ERR "[%s]: failed to initialise VCHI instance (err=%d)",
             __func__,
             err );

      err = -EIO;
      goto err_proc_destroy;
   }

   err = vchi_connect( NULL, 0, vchi_instance );
   if ( err != 0 )
   {
      printk(KERN_ERR "[%s]: failed to connect VCHI instance (err=%d)",
             __func__,
             err );

      err = -EIO;
      goto err_proc_destroy;
   }

   whvcsvc_handle = vc_vchi_wifihdmi_init( vchi_instance, &vchi_connection, 1 );
   if ( whvcsvc_handle == NULL )
   {
      printk(KERN_ERR "[%s]: failed to initialize wifi-hdmi videocore service",
             __func__ );

      err = -EPERM;
      goto err_proc_destroy;
   }

   /* All is well...
   */
   printk( KERN_INFO "WIFI HDMI Driver loaded\n" );
   return 0;

err_proc_destroy:
#ifdef CONFIG_SYSFS
   if ( proc_entry )
   {
      remove_proc_entry( WHDMI_DEVICE_NAME, proc_entry );
      proc_entry = NULL;
   }
err_dev_destroy:
   device_destroy( whdmi_class, whdmi_dev );
err_class_destroy:
   class_destroy( whdmi_class );
err_cdev_del:
   cdev_del( &whdmi_cdev );
#endif
err_unregister_chrdev_region:
   unregister_chrdev_region( whdmi_dev,
                             WHDMI_NUM_DEVICES );
err_return:
   return err;
}

static void __exit whdmi_exit( void )
{
   // printk( KERN_ERR "%s: called...\n", __FUNCTION__ );
#ifdef CONFIG_SYSFS
   if ( proc_entry )
   {
      remove_proc_entry( WHDMI_DEVICE_NAME, proc_entry );
      proc_entry = NULL;
   }
   device_destroy( whdmi_class, whdmi_dev );
   class_destroy( whdmi_class );
#endif

   cdev_del( &whdmi_cdev );
   unregister_chrdev_region( whdmi_dev,
                             WHDMI_NUM_DEVICES );

   /* Stop the videocore wifi-hdmi service.
   */
   if ( whvcsvc_handle )
   {
      vc_vchi_wifihdmi_end( &whvcsvc_handle );
   }

   /* Free all allocated memory */

   /* Destroy the memory cache */
   if ( kmem_cache )
   {
      kmem_cache_destroy( kmem_cache );
      kmem_cache = NULL;
   }
}

/***************************************************************************/
/**
*  Set Callback function
*
*  @return
*     0           Everything ok
*    -ve          Something bad happened
*/
int whdmi_set_callback(
    WHDMI_CALLBACK      callback,
    void                *callback_param
)
{
    /* Overwrite the callback function */
    registered_callback = callback;
    registered_callback_param = callback_param;

    return 0;
}

/***************************************************************************/
/**
*  Tells user app to create a TCP listening socket
*
*  @return
*     0           Everything ok
*    -ve          Something bad happened
*/
int whdmi_create_tcp_listening_socket(
    int             km_socket_handle,
    unsigned short  port,
    int             queue_len
)
{
    WHDMI_READ_BUFFER_ELEMENT               *buf_element;
    WHDMI_MSG_CREATE_TCP_LISTENING_SOCKET   *msg_to_send;
    WHDMI_MSG_HDR                           *msg_hdr;
    WHDMI_SOCKET_ELEMENT                    *socket_element;

    if ( !open_count )
    {
        /* No user app is going to do the work for us, so return EAGAIN */
        printk( KERN_ERR "WHDMI: No user app is connected, try again later!\n" );
        return -EAGAIN;
    }

    /* Make sure the socket handle is not already in use */
    socket_element = find_socket_element_by_km_handle( km_socket_handle );

    if ( socket_element )
    {
        /* No memory for the socket element */
        printk( KERN_ERR "WHDMI: Socket handle 0x%08x already in use!\n", km_socket_handle );
        return -EINVAL;
    }

    /* Allocate a socket element to add to the list */
    socket_element = (WHDMI_SOCKET_ELEMENT *)kmalloc(sizeof(WHDMI_SOCKET_ELEMENT), GFP_KERNEL);

    if ( !socket_element )
    {
        /* No memory for the socket element */
        printk( KERN_ERR "WHDMI: Failed kmalloc!\n" );
        return -ENOMEM;
    }

    /* Zero the socket element */
    memset( socket_element, 0, sizeof( WHDMI_SOCKET_ELEMENT ) );

    /* Allocate memory for the message to user */
    buf_element = (WHDMI_READ_BUFFER_ELEMENT *)kmem_cache_alloc( kmem_cache,
                                                                 GFP_KERNEL );
    if ( !buf_element )
    {
        /* No memory for the message */
        printk( KERN_ERR "WHDMI: Failed kmem_cache_alloc!\n" );
        kfree(socket_element);
        return -ENOMEM;
    }

    /* Set up pointers for easier manipulation */
    msg_hdr     = &buf_element->msg.hdr;
    msg_to_send = &buf_element->msg.create_tcp_listening_socket;

    /* Fill in the contents of the socket element */
    socket_element->socket_type             = WHDMI_SOCKET_TYPE_TCP_LISTENING;
    socket_element->km_socket_handle        = km_socket_handle;
    socket_element->socket_handle           = -1;

    /* Chain the socket element */
    chain_socket_element( socket_element );

    /* Fill in the rest of the message */
    msg_hdr->msg_type       = WHDMI_MSG_TYPE_CREATE_TCP_LISTENING_SOCKET;
    msg_hdr->msg_length     = sizeof( WHDMI_MSG_CREATE_TCP_LISTENING_SOCKET );

    msg_to_send->msg_handle = (WHDMI_MSG_HANDLE)km_socket_handle;
    msg_to_send->port       = port;
    msg_to_send->queue_len  = queue_len;

    /* Fill in the total message length and clear the next packet */
    buf_element->msg_length = msg_hdr->msg_length;
    buf_element->next       = NULL;

    /* Chain the buffer element to the read buffer */
    chain_buffer_element( buf_element );

    return 0;
}
 
/***************************************************************************/
/**
*  Tells user app to create a UDP socket
*
*  @return
*     0           Everything ok
*    -ve          Something bad happened
*/
int whdmi_create_udp_socket(
    int             km_socket_handle,
    unsigned short  port,
    int             queue_size
)
{
    WHDMI_READ_BUFFER_ELEMENT               *buf_element;
    WHDMI_MSG_CREATE_UDP_SOCKET             *msg_to_send;
    WHDMI_MSG_HDR                           *msg_hdr;
    WHDMI_SOCKET_ELEMENT                    *socket_element;

    if ( !open_count )
    {
        /* No user app is going to do the work for us, so return EAGAIN */
        printk( KERN_ERR "WHDMI: No user app is connected, try again later!\n" );
        return -EAGAIN;
    }

    /* Make sure the socket handle is not already in use */
    socket_element = find_socket_element_by_km_handle( km_socket_handle );

    if ( socket_element )
    {
        /* No memory for the socket element */
        printk( KERN_ERR "WHDMI: Socket handle 0x%08x already in use!\n", km_socket_handle );
        return -EINVAL;
    }

    /* Allocate a socket element to add to the list */
    socket_element = (WHDMI_SOCKET_ELEMENT *)kmalloc(sizeof(WHDMI_SOCKET_ELEMENT), GFP_KERNEL);

    if ( !socket_element )
    {
        /* No memory for the socket element */
        printk( KERN_ERR "WHDMI: Failed kmalloc!\n" );
        return -ENOMEM;
    }

    /* Zero the socket element */
    memset( socket_element, 0, sizeof( WHDMI_SOCKET_ELEMENT ) );

    /* Allocate memory for the message to user */
    buf_element = (WHDMI_READ_BUFFER_ELEMENT *)kmem_cache_alloc( kmem_cache,
                                                                 GFP_KERNEL );
    if ( !buf_element )
    {
        /* No memory for the message */
        printk( KERN_ERR "WHDMI: Failed kmem_cache_alloc!\n" );
        kfree(socket_element);
        return -ENOMEM;
    }

    /* Set up pointers for easier manipulation */
    msg_hdr     = &buf_element->msg.hdr;
    msg_to_send = &buf_element->msg.create_udp_socket;

    /* Fill in the contents of the socket element */
    socket_element->socket_type             = WHDMI_SOCKET_TYPE_UDP;
    socket_element->km_socket_handle        = km_socket_handle;
    socket_element->socket_handle           = -1;

    /* Chain the socket element */
    chain_socket_element( socket_element );

    /* Fill in the rest of the message */
    msg_hdr->msg_type       = WHDMI_MSG_TYPE_CREATE_UDP_SOCKET;
    msg_hdr->msg_length     = sizeof( WHDMI_MSG_CREATE_UDP_SOCKET );

    msg_to_send->msg_handle = (WHDMI_MSG_HANDLE)km_socket_handle;
    msg_to_send->port       = port;
    msg_to_send->queue_size = queue_size;

    /* Fill in the total message length and clear the next packet */
    buf_element->msg_length = msg_hdr->msg_length;
    buf_element->next       = NULL;

    /* Chain the buffer element to the read buffer */
    chain_buffer_element( buf_element );

    return 0;
}

/***************************************************************************/
/**
*  Tells user app to create a UDP TX only socket
*
*  @return
*     0           Everything ok
*    -ve          Something bad happened
*/
int whdmi_create_udp_tx_socket(
    int             km_socket_handle,
    unsigned short  port,
    int             queue_size
)
{
    WHDMI_READ_BUFFER_ELEMENT               *buf_element;
    WHDMI_MSG_CREATE_UDP_TX_SOCKET          *msg_to_send;
    WHDMI_MSG_HDR                           *msg_hdr;
    WHDMI_SOCKET_ELEMENT                    *socket_element;
    WHDMI_SOCKET_ELEMENT_UDP_TX             *socket_element_udp_tx;

    if ( !open_count )
    {
        /* No user app is going to do the work for us, so return EAGAIN */
        printk( KERN_ERR "WHDMI: No user app is connected, try again later!\n" );
        return -EAGAIN;
    }

    /* Make sure the socket handle is not already in use */
    socket_element = find_socket_element_by_km_handle( km_socket_handle );

    if ( socket_element )
    {
        /* No memory for the socket element */
        printk( KERN_ERR "WHDMI: Socket handle 0x%08x already in use!\n", km_socket_handle );
        return -EINVAL;
    }

    /* Allocate a socket element to add to the list */
    socket_element_udp_tx = (WHDMI_SOCKET_ELEMENT_UDP_TX *)kmalloc(sizeof(WHDMI_SOCKET_ELEMENT_UDP_TX), GFP_KERNEL);

    if ( !socket_element_udp_tx )
    {
        /* No memory for the socket element */
        printk( KERN_ERR "WHDMI: Failed kmalloc!\n" );
        return -ENOMEM;
    }

    /* Zero the socket element */
    memset( socket_element_udp_tx, 0, sizeof( WHDMI_SOCKET_ELEMENT_UDP_TX ) );

    /* Make the socket element generic */
    socket_element = (WHDMI_SOCKET_ELEMENT *)socket_element_udp_tx;

    /* Allocate memory for the message to user */
    buf_element = (WHDMI_READ_BUFFER_ELEMENT *)kmem_cache_alloc( kmem_cache,
                                                                 GFP_KERNEL );
    if ( !buf_element )
    {
        /* No memory for the message */
        printk( KERN_ERR "WHDMI: Failed kmem_cache_alloc!\n" );
        kfree(socket_element);
        return -ENOMEM;
    }

    /* Set up pointers for easier manipulation */
    msg_hdr     = &buf_element->msg.hdr;
    msg_to_send = &buf_element->msg.create_udp_tx_socket;

    /* Fill in the contents of the socket element */
    socket_element->socket_type             = WHDMI_SOCKET_TYPE_UDP_TX;
    socket_element->km_socket_handle        = km_socket_handle;
    socket_element->socket_handle           = -1;

    /* Chain the socket element */
    chain_socket_element( socket_element );

    /* Fill in the rest of the message */
    msg_hdr->msg_type       = WHDMI_MSG_TYPE_CREATE_UDP_TX_SOCKET;
    msg_hdr->msg_length     = sizeof( WHDMI_MSG_CREATE_UDP_TX_SOCKET );

    msg_to_send->msg_handle = (WHDMI_MSG_HANDLE)km_socket_handle;
    msg_to_send->port       = port;
    msg_to_send->queue_size = queue_size;

    /* Fill in the total message length and clear the next packet */
    buf_element->msg_length = msg_hdr->msg_length;
    buf_element->next       = NULL;

    /* Chain the buffer element to the read buffer */
    chain_buffer_element( buf_element );

    return 0;

}

/***************************************************************************/
/**
*  Tells user app to send a packet through a TCP socket
*
*  @return
*     0           Everything ok
*    -ve          Something bad happened
*/
int whdmi_tcp_send(
    int             km_socket_handle,
    int             data_len,
    uint8_t        *data
)
{
    WHDMI_READ_BUFFER_ELEMENT               *buf_element;
    WHDMI_MSG_TCP_SEND                      *msg_to_send;
    WHDMI_MSG_HDR                           *msg_hdr;
    WHDMI_SOCKET_ELEMENT                    *socket_element;

    if ( !open_count )
    {
        /* No user app is going to do the work for us, so return EAGAIN */
        printk( KERN_ERR "WHDMI: No user app is connected, try again later!\n" );
        return -EAGAIN;
    }

    socket_element = find_socket_element_by_km_handle( km_socket_handle );

    if ( socket_element == NULL )
    {
        /* Could not find socket handle of interest */
        printk( KERN_ERR "WHDMI: Cannot find socket handle 0x%08x", km_socket_handle );
        return -EINVAL;
    }

    if ( socket_element->socket_type != WHDMI_SOCKET_TYPE_TCP_CONN )
    {
        /* Could not find socket handle of interest */
        printk( KERN_ERR "WHDMI: Cannot send TCP packet on non-TCP connection socket!" );
        return -EINVAL;
    }

    if ( data_len > WHDMI_MAX_PACKET_SIZE )
    {
        /* Data to send is bigger than our send buffer */
        printk( KERN_ERR "WHDMI: TCP send exceeds maximum size %d", WHDMI_MAX_PACKET_SIZE );
        return -EINVAL;
    }

    /* Allocate memory for the message to user */
    buf_element = (WHDMI_READ_BUFFER_ELEMENT *)kmem_cache_alloc( kmem_cache,
                                                                 GFP_KERNEL );
    if ( !buf_element )
    {
        /* No memory for the message */
        printk( KERN_ERR "WHDMI: Failed kmem_cache_alloc!\n" );
        kfree(socket_element);
        return -ENOMEM;
    }

    /* Set up pointers for easier manipulation */
    msg_hdr     = &buf_element->msg.hdr;
    msg_to_send = &buf_element->msg.tcp_send;

    /* Fill in the rest of the message */
    msg_hdr->msg_type       = WHDMI_MSG_TYPE_TCP_SEND;
    msg_hdr->msg_length     = sizeof( WHDMI_MSG_TCP_SEND ) - WHDMI_MAX_PACKET_SIZE  + data_len;

    msg_to_send->socket_handle  = socket_element->socket_handle;
    msg_to_send->data_len       = data_len;

    /* Copy the data to buffer */
    memcpy( msg_to_send->data, data, data_len );

    /* Fill in the total message length and clear the next packet */
    buf_element->msg_length = msg_hdr->msg_length;
    buf_element->next       = NULL;

    /* Chain the buffer element to the read buffer */
    chain_buffer_element( buf_element );

    return 0;
}

/***************************************************************************/
/**
*  Tells user app to close the socket
*
*  @return
*     0           Everything ok
*    -ve          Something bad happened
*/
int whdmi_close_socket(
    int             km_socket_handle
)
{
    WHDMI_READ_BUFFER_ELEMENT               *buf_element;
    WHDMI_MSG_CLOSE_SOCKET                  *msg_to_send;
    WHDMI_MSG_HDR                           *msg_hdr;
    WHDMI_SOCKET_ELEMENT                    *socket_element;

    if ( !open_count )
    {
        /* No user app is going to do the work for us, so return EAGAIN */
        printk( KERN_ERR "WHDMI: No user app is connected, try again later!\n" );
        return -EAGAIN;
    }

    /* Make sure the socket handle is present */
    socket_element = find_socket_element_by_km_handle( km_socket_handle );

    if ( socket_element == NULL )
    {
        /* Could not find socket */
        printk( KERN_ERR "WHDMI: Socket handle 0x%08x does not exist!\n", km_socket_handle );
        return -EINVAL;
    }

    /* Allocate memory for the message to user */
    buf_element = (WHDMI_READ_BUFFER_ELEMENT *)kmem_cache_alloc( kmem_cache,
                                                                 GFP_KERNEL );
    if ( !buf_element )
    {
        /* No memory for the message */
        printk( KERN_ERR "WHDMI: Failed kmem_cache_alloc!\n" );
        kfree(socket_element);
        return -ENOMEM;
    }

    /* At this point, dequeue the socket element */
    remove_socket_element( socket_element );

    /* Set up pointers for easier manipulation */
    msg_hdr     = &buf_element->msg.hdr;
    msg_to_send = &buf_element->msg.close_socket;

    /* Fill in the rest of the message */
    msg_hdr->msg_type       = WHDMI_MSG_TYPE_CLOSE_SOCKET;
    msg_hdr->msg_length     = sizeof( WHDMI_MSG_CLOSE_SOCKET );

    msg_to_send->socket_handle = socket_element->socket_handle;

    /* Fill in the total message length and clear the next packet */
    buf_element->msg_length = msg_hdr->msg_length;
    buf_element->next       = NULL;

    /* Chain the buffer element to the read buffer */
    chain_buffer_element( buf_element );

    /* Free the socket element */
    kfree( socket_element );

    return 0;
}

/***************************************************************************/
/**
*  Send a UDP packet through the fast path 
*
*  @return
*     0           Everything ok
*    -ve          Something bad happened
*/
static int fast_udp_send_to(
    WHDMI_SOCKET_ELEMENT_UDP_TX     *socket_element,
    unsigned long                   dest_addr,
    unsigned short                  dest_port,
    int                             data_len,
    uint8_t                         *data
)
{
    struct inet_sock    *inet;
    struct socket       *sock;
    struct sock         *sk;
    struct sk_buff      *skb;
    struct flowi4       *fl4;
    struct udphdr       *udp_hdr;
    struct rtable       *rt = NULL;
    struct net          *net;
    unsigned long       source_addr;
    unsigned short      source_port;
    int                 err;
    int                 length;
    u32                 do_checksum;
    __wsum              csum = 0;

    //do_checksum = CHECKSUM_NONE;
    do_checksum = CHECKSUM_PARTIAL;
    //do_checksum = CHECKSUM_COMPLETE;

    /* Get a reference to the struct socket */
    sock = socket_element->sock;

    /* Get the struct sock */
    sk = sock->sk;

    /* Lock it */
    lock_sock(sk);

    /* Get the inet_sock */
    inet = inet_sk(sk);

    /* Get net */
    net = sock_net(sk);

    /* Determine the source addr and port (already in network order)*/
    source_addr = inet->inet_saddr;
    source_port = inet->inet_sport;

    /* Get the cached flowi4 struct from the socket element */
    fl4 = &socket_element->fl_cached.u.ip4;

    /* Initialize the fl struct if dest_addr and dest_port differ from cached value */
    if ( dest_addr != socket_element->dest_addr_cached || dest_port != socket_element->dest_port_cached )
    {
        flowi4_init_output(fl4, sk->sk_bound_dev_if, sk->sk_mark, RT_TOS(inet->tos),
                           RT_SCOPE_UNIVERSE, sk->sk_protocol,
                           inet_sk_flowi_flags(sk)|FLOWI_FLAG_CAN_SLEEP,
                           htonl(dest_addr), source_addr, htons(dest_port), source_port);
        security_sk_classify_flow(sk, flowi4_to_flowi(fl4)); /* May not be necessary */

        /* cache the destination address and port */
        socket_element->dest_addr_cached = dest_addr;
        socket_element->dest_port_cached = dest_port;
    }

    /* Set up the route manually */
    rt = ip_route_output_flow(net, fl4, sk); 
    if (IS_ERR(rt))
    {
        err = PTR_ERR(rt);

        /* return early here since nothing was set up */
        return err;
    }

    /* Once we have the route deteremined, get the source address we are using */
    source_addr = rt->rt_src;

    /* Allocate the sk_buf (multiple of 4 bytes) */
    length = (((data_len + WHDMI_PACKET_OVERHEAD) / 4) + 1 ) * 4;
    skb = sock_alloc_send_skb(sk, length, 0, &err);
    if ( skb == NULL )
    {
        /* Free the route */
        ip_rt_put(rt);

        /* Unlock the sk */
        release_sock(sk);

        return -ENOMEM;
    }

    /* Prepare the skb content */

    /* Reserve the headers for various lower layers */
    skb_reserve(skb, WHDMI_PACKET_HEADERS);

    /* Copy the payload */
    memcpy( skb->data, data, data_len );
    skb_put(skb, data_len);

    /* Push the UDP header on */
    skb_push(skb, sizeof(struct udphdr));
    skb_reset_transport_header(skb);

    /* Put in some necessary info */
    skb->ip_summed = do_checksum;

    /* Overlay the UDP header */
    udp_hdr = (struct udphdr *)skb_transport_header(skb);

    udp_hdr->source = source_port;
    udp_hdr->dest   = htons(dest_port);
    udp_hdr->len    = htons((sizeof(struct udphdr) + data_len));
    udp_hdr->check  = 0;

    if ( do_checksum == CHECKSUM_COMPLETE )
    {
        /* Do the checksum of the UDP header and the payload */
        csum = csum_partial(skb_transport_header(skb), sizeof(struct udphdr) + data_len, 0);

        /* Do the pseudo checksum and fill in */
        /* Both source and destination address should be in network order */
        udp_hdr->check = csum_tcpudp_magic( source_addr, htonl(dest_addr), (sizeof(struct udphdr) + data_len), IPPROTO_UDP, csum );

        if ( udp_hdr->check == 0 )
        {
            udp_hdr->check = CSUM_MANGLED_0;
        }
    }
    else if ( do_checksum == CHECKSUM_PARTIAL )
    {
        /* For HW checksum offload, fill in the skb where to perform checksum */
        skb->csum_start     = skb_transport_header(skb) - skb->head;
        skb->csum_offset    = offsetof( struct udphdr, check );

        /* Do the pseudo checksum and fill in */
        /* Both source and destination address should be in network order */
        udp_hdr->check  = ~csum_tcpudp_magic( source_addr, htonl(dest_addr), (sizeof(struct udphdr) + data_len), IPPROTO_UDP, 0);
    }

    /* Set the route */
    skb_dst_set(skb, &rt->dst ); 

    /* Set tx timestamp flags */
    sock_tx_timestamp( sk, &skb_shinfo(skb)->tx_flags );

    /* Transmit the packet */
    err = ip_queue_xmit( skb, &socket_element->fl_cached );

    /* Unlock the socket */
    release_sock(sk);

    /* Update stats */
    tx_stats.bytes_sent += sizeof(struct udphdr) + data_len;

    return err;
}


/***************************************************************************/
/**
*  Tells user app to send a packet through a UDP socket
*
*  @return
*     0           Everything ok
*    -ve          Something bad happened
*/
int whdmi_udp_send_to(
    int             km_socket_handle,
    unsigned long   dest_addr,
    unsigned short  dest_port,
    int             data_len,
    uint8_t        *data
)
{
    WHDMI_READ_BUFFER_ELEMENT               *buf_element;
    WHDMI_MSG_UDP_SEND_TO                   *msg_to_send;
    WHDMI_MSG_HDR                           *msg_hdr;
    WHDMI_SOCKET_ELEMENT                    *socket_element;

    if ( !open_count )
    {
        /* No user app is going to do the work for us, so return EAGAIN */
        printk( KERN_ERR "WHDMI: No user app is connected, try again later!\n" );
        return -EAGAIN;
    }

    socket_element = find_socket_element_by_km_handle( km_socket_handle );

    if ( socket_element == NULL )
    {
        /* Could not find socket handle of interest */
        printk( KERN_ERR "WHDMI: Cannot find socket handle 0x%08x", km_socket_handle );
        return -EINVAL;
    }

    if ( socket_element->socket_type != WHDMI_SOCKET_TYPE_UDP &&
         socket_element->socket_type != WHDMI_SOCKET_TYPE_UDP_TX )
    {
        /* Could not find socket handle of interest */
        printk( KERN_ERR "WHDMI: Cannot send UDP packet on non-UDP connection socket!" );
        return -EINVAL;
    }

    if ( data_len > WHDMI_MAX_PACKET_SIZE )
    {
        /* Data to send is bigger than our send buffer */
        printk( KERN_ERR "WHDMI: UDP sendto exceeds maximum size %d", WHDMI_MAX_PACKET_SIZE );
        return -EINVAL;
    }

    if ( socket_element->socket_type == WHDMI_SOCKET_TYPE_UDP_TX )
    {
        /* Do what's necessary to send a UDP packet by-passing user space sockets */
        return fast_udp_send_to( (WHDMI_SOCKET_ELEMENT_UDP_TX *)socket_element,
                                 dest_addr,
                                 dest_port,
                                 data_len,
                                 data );
    }
    else
    {
        /* Allocate memory for the message to user */
        buf_element = (WHDMI_READ_BUFFER_ELEMENT *)kmem_cache_alloc( kmem_cache,
                                                                     GFP_KERNEL );
        if ( !buf_element )
        {
            /* No memory for the message */
            printk( KERN_ERR "WHDMI: Failed kmem_cache_alloc!\n" );
            kfree(socket_element);
            return -ENOMEM;
        }

        /* Set up pointers for easier manipulation */
        msg_hdr     = &buf_element->msg.hdr;
        msg_to_send = &buf_element->msg.udp_send_to;

        /* Fill in the rest of the message */
        msg_hdr->msg_type       = WHDMI_MSG_TYPE_UDP_SEND_TO;
        msg_hdr->msg_length     = sizeof( WHDMI_MSG_UDP_SEND_TO ) - WHDMI_MAX_PACKET_SIZE + data_len;

        msg_to_send->socket_handle  = socket_element->socket_handle;
        msg_to_send->dest_addr      = dest_addr;
        msg_to_send->dest_port      = dest_port;
        msg_to_send->data_len       = data_len;

        /* Copy the data to buffer */
        memcpy( msg_to_send->data, data, data_len );

        /* Fill in the total message length and clear the next packet */
        buf_element->msg_length = msg_hdr->msg_length;
        buf_element->next       = NULL;

        /* Chain the buffer element to the read buffer */
        chain_buffer_element( buf_element );

        /* Update stats */
        tx_stats.bytes_sent += sizeof(struct udphdr) + data_len;
    }

    return 0;
}

/***************************************************************************/
/**
*  Timer Expiry Function 
*
*  @return
*     None
*/
static void whdmi_timer_fn(
    unsigned long arg
)
{
    struct whdmi_tx_stats   *stats = (struct whdmi_tx_stats *)arg;

    stats->average_byte_rate = (WHDMI_TX_STATS_SMOOTHING * (stats->bytes_sent/WHDMI_TX_STATS_DELAY_SEC)) + ((1 - WHDMI_TX_STATS_SMOOTHING)*(stats->average_byte_rate));

    /* Zero the bytes_sent for more accumulation */
    stats->bytes_sent = 0;

    /* Reschedule the timer */
    stats->timer.expires += WHDMI_TX_STATS_DELAY_SEC * HZ;
    add_timer(&stats->timer);
}

module_init( whdmi_init );
module_exit( whdmi_exit );
MODULE_AUTHOR( "Broadcom" );
MODULE_DESCRIPTION( "Wireless HDMI Driver" );
MODULE_LICENSE( "GPL" );
