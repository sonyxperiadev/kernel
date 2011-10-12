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

#include <whdmi.h>

// ---- Private Function Prototypes ------------------------------------------

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
   WHDMI_REMOVE_UNUSED_PARAM_WARNING (callback);
   WHDMI_REMOVE_UNUSED_PARAM_WARNING (callback_param);

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
   WHDMI_REMOVE_UNUSED_PARAM_WARNING (km_socket_handle);
   WHDMI_REMOVE_UNUSED_PARAM_WARNING (data_len);
   WHDMI_REMOVE_UNUSED_PARAM_WARNING (data);

   return 0;
}
