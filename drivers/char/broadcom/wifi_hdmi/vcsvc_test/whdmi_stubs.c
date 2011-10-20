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

#include <linux/broadcom/whdmi.h>

// ---- Private Function Prototypes ------------------------------------------

#define LOG_INFO( fmt, arg... )      printk( KERN_INFO "[I] " fmt "\n", ##arg )


WHDMI_CALLBACK whdmi_stubs__callback = NULL;
void *whdmi_stubs__callback_param = NULL;

/* Special macro to be used to remove warnings on parameter not used within
** a function.
*/
#define WHDMI_REMOVE_UNUSED_PARAM_WARNING( X ) \
   whdmi_unused_params_killer( (void *)&(X) );

void *whdmi_unused_params_killer( void *param )
{
   return param;
}


/***************************************************************************/
/**
*  Set callback function 
*
*  @return  0 on success, otherwise -ve error.
*   -EAGAIN - Not ready to handle operation, try again later
*   -EINVAL - Invalid agrument
*/
int whdmi_set_callback( WHDMI_CALLBACK callback,
                        void *callback_param )
{
   whdmi_stubs__callback = callback;
   whdmi_stubs__callback_param = callback_param;

   return 0;
}

/***************************************************************************/
/**
*  Instructs WIFI HDMI Driver to create a UDP socket 
*
*  @return  0 on success, otherwise -ve error.
*   -EAGAIN - Not ready to handle operation, try again later
*   -ENOMEM - Not enough memory to carry out operation
*/
int whdmi_create_udp_socket( int km_socket_handle,
                             unsigned short port,
                             int queue_size )
{  
   WHDMI_REMOVE_UNUSED_PARAM_WARNING (km_socket_handle);
   WHDMI_REMOVE_UNUSED_PARAM_WARNING (port);
   WHDMI_REMOVE_UNUSED_PARAM_WARNING (queue_size);

   return 0;  
}

/***************************************************************************/
/**
*  Instructs WIFI HDMI Driver to create a TX-only UDP socket
*
*  @return  0 on success, otherwise -ve error.
*   -EAGAIN - Not ready to handle operation, try again later
*   -ENOMEM - Not enough memory to carry out operation
*/
int whdmi_create_udp_tx_socket( int km_socket_handle,
                                unsigned short port,
                                int queue_size )
{
   WHDMI_REMOVE_UNUSED_PARAM_WARNING (km_socket_handle);
   WHDMI_REMOVE_UNUSED_PARAM_WARNING (port);
   WHDMI_REMOVE_UNUSED_PARAM_WARNING (queue_size);

   return 0;
}

/***************************************************************************/
/**
*  Instructs WIFI HDMI Driver to send a packet through UDP 
*
*  @return  0 on success, otherwise -ve error.
*   -EAGAIN - Not ready to handle operation, try again later
*   -EINVAL - Invalid argument
*   -ENOMEM - Not enough memory to carry out operation
*/
int whdmi_udp_send_to( int km_socket_handle,
                       unsigned long dest_addr,
                       unsigned short dest_port,
                       int data_len,
                       uint8_t *data )
{
   WHDMI_REMOVE_UNUSED_PARAM_WARNING (km_socket_handle);
   WHDMI_REMOVE_UNUSED_PARAM_WARNING (dest_addr);
   WHDMI_REMOVE_UNUSED_PARAM_WARNING (dest_port);
   WHDMI_REMOVE_UNUSED_PARAM_WARNING (data_len);
   WHDMI_REMOVE_UNUSED_PARAM_WARNING (data);

   return 0;   
}

/***************************************************************************/
/**
*  Instructs WIFI HDMI Driver to send a packet through TCP 
*
*  @return  0 on success, otherwise -ve error.
*   -EAGAIN - Not ready to handle operation, try again later
*   -EINVAL - Invalid argument
*   -ENOMEM - Not enough memory to carry out operation
*/
int whdmi_tcp_send( int km_socket_handle,
                    int data_len,
                    uint8_t *data )
{
   int ix;

   WHDMI_REMOVE_UNUSED_PARAM_WARNING (km_socket_handle);

   LOG_INFO( "[%s]: control sending %u bytes",
             __func__,
             data_len );

   LOG_INFO( "[%s]: [start] ====>\n",
             __func__ );     
   for ( ix = 0 ; ix < data_len ; ix++ )
   {
      LOG_INFO( "%c", *(data+ix) );      
   }
   LOG_INFO( "\n[%s]: <=== [end]\n",
             __func__ );     

   return 0;
}

/***************************************************************************/
/**
*  Instructs WIFI HDMI Driver to create a TCP listening socket 
*
*  @return  0 on success, otherwise -ve error.
*   -EAGAIN - Not ready to handle operation, try again later
*   -ENOMEM - Not enough memory to carry out operation
*/
int whdmi_create_tcp_listening_socket( int km_socket_handle,
                                       unsigned short port,
                                       int queue_len )
{
   WHDMI_REMOVE_UNUSED_PARAM_WARNING (km_socket_handle);
   WHDMI_REMOVE_UNUSED_PARAM_WARNING (port);
   WHDMI_REMOVE_UNUSED_PARAM_WARNING (queue_len);

   return 0;
}

/***************************************************************************/
/**
*  Instructs WIFI HDMI Driver to close socket 
*
*  @return  0 on success, otherwise -ve error.
*   -EAGAIN - Not ready to handle operation, try again later
*   -EINVAL - Invalid argument
*/
int whdmi_close_socket( int km_socket_handle )
{
   WHDMI_REMOVE_UNUSED_PARAM_WARNING (km_socket_handle);

   return 0;
}

/***************************************************************************/
/**
*   
*/
int whdmi_incoming_socket( int km_socket_handle, int socket_port )
{
   WHDMI_EVENT_PARAM event;
   WHDMI_EVENT_SOCKET_INCOMING_PARAM *param;
   
   param = (WHDMI_EVENT_SOCKET_INCOMING_PARAM *) &event.socket_incoming;

   param->parent_km_socket_handle = km_socket_handle;
   param->client_addr             = 0x7F000001;
   param->client_port             = socket_port;
   
   if ( whdmi_stubs__callback )
   {
      whdmi_stubs__callback( WHDMI_EVENT_SOCKET_INCOMING,
                             (WHDMI_EVENT_PARAM *) &event,
                             whdmi_stubs__callback_param );
   }

   return 0;
}

/***************************************************************************/
/**
*   
*/
#define WHDMI_CANNED_DESCRIBE  "DESCRIBE rtsp://192.168.1.107:1234/stream1 RTSP/1.0\r\nCSeq: 0\r\nUser-Agent: brcm-vowifi/1.0\r\nAccept: application/sdp\r\nAccept-Language: en-KR\r\n"
#define WHDMI_CANNED_SETUP     "SETUP rtsp://192.168.1.107:1234/stream1/video RTSP/1.0\r\nCSeq: 1\r\nUser-Agent: brcm-vowifi/1.0\r\nTransport: RTP/AVP;unicast;client_port=7898-7899\r\n"
#define WHDMI_CANNED_PLAY      "PLAY rtsp://192.168.1.107:1234/stream1 RTSP/1.0\r\nCSeq: 3\r\nUser-Agent: brcm-vowifi/1.0\r\nSession: 12345678\r\nRange:npt=0.000-\r\n"

int whdmi_data_on_socket( int km_socket_handle, int canned_data )
{
   WHDMI_EVENT_PARAM event;
   WHDMI_EVENT_SOCKET_DATA_AVAIL_PARAM *param;
   char *canned = NULL;

   switch ( canned_data )
   {
      case 2:
         canned = (char *)WHDMI_CANNED_SETUP;
      break;

      case 3:
         canned = (char *)WHDMI_CANNED_PLAY;
      break;

      case 1:
      default:
         canned = (char *)WHDMI_CANNED_DESCRIBE;
      break;
   }

   param = (WHDMI_EVENT_SOCKET_DATA_AVAIL_PARAM *) &event.socket_data_avail;

   param->km_socket_handle        = km_socket_handle;
   param->data_len                = strlen( (const char *)canned );
   param->data                    = (unsigned char *)canned;
   
   if ( whdmi_stubs__callback )
   {
      whdmi_stubs__callback( WHDMI_EVENT_SOCKET_DATA_AVAIL,
                             (WHDMI_EVENT_PARAM *) &event,
                             whdmi_stubs__callback_param );
   }

   return 0;
}
