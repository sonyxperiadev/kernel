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
/**
*
*  @file    whdmi.h
*
*  @brief   Defines the WIFI HDMI API
*
*****************************************************************************/
#if !defined( WHDMI_H )
#define WHDMI_H

/* ---- Include Files ---------------------------------------------------- */
#if defined( __KERNEL__ )
#include <linux/types.h>	/* Needed for standard types */
#else
#include <stdint.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* ---- Constants and Types ---------------------------------------------- */

/*
 * Using the WIFI HDMI Driver in kernel space is very straightforward. The
 * kernel space module will register a callback function with the WIFI 
 * HDMI Driver via whdmi_set_callback(). The callback function has the following
 * function prototype:
 *
 *  typedef void (* WHDMI_CALLBACK) (WHDMI_EVENT event, WHDMI_EVENT_PARAM *param, void* callback_param);
 *
 * All the APIs available to the Kernel module requires the passing of an arbitrary
 * socket handle int km_socket_handle that is determined by the caller to perform
 * the whdmi_create_xxx_socket(). So contrary to regular socket() call where the
 * handle is returned, the Kernel module decides what handle to use. Similarly,
 * when an incoming TCP connection comes in, the Kernel module will be notified
 * of such connection through the callback event, and would pass back the desired
 * socket handle for the new socket upon returning from the callback function.
 *
*/

/* List of events to notify through callback function */
	typedef enum whdmi_event {
		WHDMI_EVENT_SOCKET_INCOMING = 0,	/* There is an incoming socket */
		WHDMI_EVENT_SOCKET_DISCONNECTED,	/* Remote-side has closed the socket */
		WHDMI_EVENT_SOCKET_DATA_AVAIL,	/* Data is available on socket */
		WHDMI_EVENT_SOCKET_CLOSED,	/* Socket has been closed (by userspace app) */
		WHDMI_EVENT_START_SERVICE,	/* User application wishes to start the service */
		WHDMI_EVENT_STOP_SERVICE,	/* User application wishes to stop the service */
		WHDMI_EVENT_AUDIO_STREAM_STATUS	/* User application checks the status of the audio stream */
	} WHDMI_EVENT;

/* Socket has been accepted */
	typedef struct whdmi_event_socket_incoming_param {
		int parent_km_socket_handle;	/* Incoming socket's parent socket handle */
		unsigned long client_addr;	/* Client address in host order */
		unsigned short client_port;	/* Client port in host order */
		int km_socket_handle_returned;	/* To be filled by callback function once the callback returns */
	} WHDMI_EVENT_SOCKET_INCOMING_PARAM;

/* Socket has been terminated remotely */
	typedef struct whdmi_event_disconnected_param {
		int km_socket_handle;	/* Kernel module's socket handle */
	} WHDMI_EVENT_SOCKET_DISCONNECTED_PARAM;

/* Data is available on the socket */
	typedef struct whdmi_event_data_avail_param {
		int km_socket_handle;	/* Kernel module's socket handle */
		int data_len;	/* Length of the data */
		unsigned char __user *data;	/* Pointer to data (user space, source of data validated), must call copy_from_user */
	} WHDMI_EVENT_SOCKET_DATA_AVAIL_PARAM;

/* Socket has been closed */
	typedef struct whdmi_event_socket_closed_param {
		int km_socket_handle;	/* Kernel module's socket handle that was closed */
	} WHDMI_EVENT_SOCKET_CLOSED_PARAM;

/* Service Started */
	typedef struct whdmi_event_start_service_param {
		int not_used;	/* Place-holder for future value */
	} WHDMI_EVENT_START_SERVICE_PARAM;

/* Service Stopped */
	typedef struct whdmi_event_stop_service_param {
		int not_used;	/* Place-holder for future value */
	} WHDMI_EVENT_STOP_SERVICE_PARAM;

/* Check status of audio stream */
	typedef struct whdmi_event_audio_stream_status_param {
		int enabled;	/* To be filled by callback function once the callback returns */
	} WHDMI_EVENT_AUDIO_STREAM_STATUS_PARAM;

/* A union of event parameters */
	typedef union whdmi_event_param {
		WHDMI_EVENT_SOCKET_INCOMING_PARAM socket_incoming;
		WHDMI_EVENT_SOCKET_DISCONNECTED_PARAM socket_disconnected;
		WHDMI_EVENT_SOCKET_DATA_AVAIL_PARAM socket_data_avail;
		WHDMI_EVENT_SOCKET_CLOSED_PARAM socket_closed;
		WHDMI_EVENT_START_SERVICE_PARAM start_service;
		WHDMI_EVENT_STOP_SERVICE_PARAM stop_service;
		WHDMI_EVENT_AUDIO_STREAM_STATUS_PARAM audio_stream_status;
	} WHDMI_EVENT_PARAM;

	typedef void *(*WHDMI_CALLBACK) (WHDMI_EVENT event,
					 WHDMI_EVENT_PARAM * param,
					 void *callback_param);

/* ---- Variable Externs ------------------------------------------------- */

/* ---- Function Prototypes ---------------------------------------------- */

/* The following API is currently only available to kernel drivers.
 */

#if defined( __KERNEL__ )

/***************************************************************************/
/**
*  Set callback function 
*
*  @return  0 on success, otherwise -ve error.
*   -EAGAIN - Not ready to handle operation, try again later
*   -EINVAL - Invalid agrument
*/
	int whdmi_set_callback(WHDMI_CALLBACK callback,	/* Callback function to be registered */
			       void *callback_param	/* Callback parameter to be passed back */
	    );

/***************************************************************************/
/**
*  Instructs WIFI HDMI Driver to create a TCP listening socket 
*
*  @return  0 on success, otherwise -ve error.
*   -EAGAIN - Not ready to handle operation, try again later
*   -ENOMEM - Not enough memory to carry out operation
*/
	int whdmi_create_tcp_listening_socket(int km_socket_handle,	/* Kernel module's own handle for the new socket */
					      unsigned short port,	/* Port to listen on */
					      int queue_len	/* TCP listening socket backlog queue */
	    );

/***************************************************************************/
/**
*  Instructs WIFI HDMI Driver to create a UDP socket 
*
*  @return  0 on success, otherwise -ve error.
*   -EAGAIN - Not ready to handle operation, try again later
*   -ENOMEM - Not enough memory to carry out operation
*/
	int whdmi_create_udp_socket(int km_socket_handle,	/* Kernel module's own handle for the new socket */
				    unsigned short port,	/* Port to bind to */
				    int queue_size	/* Size of the receive queue depth */
	    );

/***************************************************************************/
/**
*  Instructs WIFI HDMI Driver to create a TX-only UDP socket
*
*  @return  0 on success, otherwise -ve error.
*   -EAGAIN - Not ready to handle operation, try again later
*   -ENOMEM - Not enough memory to carry out operation
*/
	int whdmi_create_udp_tx_socket(int km_socket_handle,	/* Kernel module's own handle for the new socket */
				       unsigned short port,	/* Port to bind to */
				       int queue_size	/* Size of the receive queue depth */
	    );

/***************************************************************************/
/**
*  Instructs WIFI HDMI Driver to send a packet through TCP 
*
*  @return  0 on success, otherwise -ve error.
*   -EAGAIN - Not ready to handle operation, try again later
*   -EINVAL - Invalid argument
*   -ENOMEM - Not enough memory to carry out operation
*/
	int whdmi_tcp_send(int km_socket_handle,	/* Kernel module's own handle for the socket */
			   int data_len, uint8_t *data);

/***************************************************************************/
/**
*  Instructs WIFI HDMI Driver to send a packet through UDP 
*
*  @return  0 on success, otherwise -ve error.
*   -EAGAIN - Not ready to handle operation, try again later
*   -EINVAL - Invalid argument
*   -ENOMEM - Not enough memory to carry out operation
*/
	int whdmi_udp_send_to(int km_socket_handle,	/* Kernel module's own handle for the socket */
			      unsigned long dest_addr,	/* Destination address to send to (in host order) */
			      unsigned short dest_port,	/* Destination port (in host order) */
			      int data_len,	/* Length of data to send */
			      uint8_t *data	/* Buffer containing data to send */
	    );

/***************************************************************************/
/**
*  Instructs WIFI HDMI Driver to close socket 
*
*  @return  0 on success, otherwise -ve error.
*   -EAGAIN - Not ready to handle operation, try again later
*   -EINVAL - Invalid argument
*/
	int whdmi_close_socket(int km_socket_handle	/* Kernel module's own handle for the socket */
	    );

#endif				/* __KERNEL__ */

#ifdef __cplusplus
}
#endif
#endif				/* WHDMI_H */
