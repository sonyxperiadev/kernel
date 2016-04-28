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

#if !defined( WHDMI_IOCTL_H )
#define WHDMI_IOCTL_H

/* ---- Include Files ---------------------------------------------------- */

#if defined( __KERNEL__ )
#include <linux/types.h>	/* Needed for standard types */
#else
#include <stdint.h>
#endif

#include <linux/ioctl.h>

/* ---- Constants and Types ---------------------------------------------- */

typedef int WHDMI_MSG_HANDLE;

/* Type define used to create unique IOCTL number */
#define WHDMI_MAGIC_TYPE                  'x'

/* Maximum packet size to transmit */
#define WHDMI_MAX_PACKET_SIZE             1500

/* IOCTL commands */
enum whdmi_cmd_e {
	WHDMI_CMD_SOCKET_CREATED = 0x23,	/* Start at 0x23 arbitrarily */
	WHDMI_CMD_SOCKET_ACCEPTED,
	WHDMI_CMD_SOCKET_DISCONNECTED,
	WHDMI_CMD_SOCKET_DATA_AVAIL,
	WHDMI_CMD_SOCKET_CLOSED,
	WHDMI_CMD_START_SERVICE,
	WHDMI_CMD_STOP_SERVICE,
	WHDMI_CMD_AUDIO_STREAM_STATE,
	WHDMI_CMD_LAST		/* Do not delete */
};

/* IOCTL Data structures */
struct whdmi_ioctl_socket_created {
	WHDMI_MSG_HANDLE whdmi_msg_handle;	/* The handle given in the message from WIFI HDMI Driver */
	int socket_handle;	/* The newly created socket handle */
};

struct whdmi_ioctl_socket_accepted {
	int socket_handle;	/* The newly accepted socket handle */
	int parent_socket_handle;	/* The parent socket of the newly accepted socket */
	unsigned long client_addr;	/* Client address in host order */
	unsigned short client_port;	/* Client port in host order */
};

struct whdmi_ioctl_socket_disconnected {
	int socket_handle;	/* The socket handle that disconnected */
};

struct whdmi_ioctl_socket_data_avail {
	int socket_handle;	/* The socket handle containing the message */
	int data_len;		/* Length of the data */
	unsigned char *data;	/* Pointer to data (user space) */
};

struct whdmi_ioctl_socket_closed {
	int socket_handle;	/* The socket handle that closed */
};

/**
*  Union of all the different IOCTL parameter structures to determine
*  max stack variable size
*/
union whdmi_ioctl_params {
	struct whdmi_ioctl_socket_created socket_created;
	struct whdmi_ioctl_socket_accepted socket_accepted;
	struct whdmi_ioctl_socket_disconnected socket_disconnected;
	struct whdmi_ioctl_socket_data_avail socket_data_avail;
	struct whdmi_ioctl_socket_closed socket_closed;
};

/* IOCTL numbers */
#define WHDMI_IOCTL_SOCKET_CREATED          _IOW( WHDMI_MAGIC_TYPE, WHDMI_CMD_SOCKET_CREATED, struct whdmi_ioctl_socket_created )
#define WHDMI_IOCTL_SOCKET_ACCEPTED         _IOW( WHDMI_MAGIC_TYPE, WHDMI_CMD_SOCKET_ACCEPTED, struct whdmi_ioctl_socket_accepted )
#define WHDMI_IOCTL_SOCKET_DISCONNECTED     _IOW( WHDMI_MAGIC_TYPE, WHDMI_CMD_SOCKET_DISCONNECTED, struct whdmi_ioctl_socket_disconnected )
#define WHDMI_IOCTL_SOCKET_DATA_AVAIL       _IOW( WHDMI_MAGIC_TYPE, WHDMI_CMD_SOCKET_DATA_AVAIL, struct whdmi_ioctl_socket_data_avail )
#define WHDMI_IOCTL_SOCKET_CLOSED           _IOW( WHDMI_MAGIC_TYPE, WHDMI_CMD_SOCKET_CLOSED, struct whdmi_ioctl_socket_closed )
#define WHDMI_IOCTL_START_SERVICE           _IOW( WHDMI_MAGIC_TYPE, WHDMI_CMD_START_SERVICE, int )
#define WHDMI_IOCTL_STOP_SERVICE            _IOW( WHDMI_MAGIC_TYPE, WHDMI_CMD_STOP_SERVICE, int )
#define WHDMI_IOCTL_AUDIO_STREAM_STATE      _IOR( WHDMI_MAGIC_TYPE, WHDMI_CMD_AUDIO_STREAM_STATE, int )

/* Messages from WIFI HDMI Driver */
typedef enum whdmi_msg_e {
	WHDMI_MSG_TYPE_CREATE_TCP_LISTENING_SOCKET = 0,
	WHDMI_MSG_TYPE_CREATE_UDP_SOCKET,
	WHDMI_MSG_TYPE_CREATE_UDP_TX_SOCKET,
	WHDMI_MSG_TYPE_TCP_SEND,
	WHDMI_MSG_TYPE_UDP_SEND_TO,
	WHDMI_MSG_TYPE_CLOSE_SOCKET,
	WHDMI_MSG_TYPE_LAST
} WHDMI_MSG_TYPE;

typedef struct whdmi_msg_hdr {
	WHDMI_MSG_TYPE msg_type;	/* Message type */
	int msg_length;		/* Length of the entire message, including this header */
} WHDMI_MSG_HDR;

typedef struct whdmi_msg_create_tcp_listening_socket {
	WHDMI_MSG_HDR msg_hdr;
	WHDMI_MSG_HANDLE msg_handle;	/* Handle from WIFI HDMI Driver */
	unsigned short port;	/* Port to bind to */
	int queue_len;		/* TCP listening socket backlog queue */
} WHDMI_MSG_CREATE_TCP_LISTENING_SOCKET;

typedef struct whdmi_msg_create_udp_socket {
	WHDMI_MSG_HDR msg_hdr;
	WHDMI_MSG_HANDLE msg_handle;	/* Handle from WIFI HDMI Driver */
	unsigned short port;	/* Port to bind to */
	int queue_size;		/* Size of the receive queue depth */
} WHDMI_MSG_CREATE_UDP_SOCKET;

typedef struct whdmi_msg_create_udp_tx_socket {
	WHDMI_MSG_HDR msg_hdr;
	WHDMI_MSG_HANDLE msg_handle;	/* Handle from WIFI HDMI Driver */
	unsigned short port;	/* Port to bind to */
	int queue_size;		/* Size of the receive queue depth */
} WHDMI_MSG_CREATE_UDP_TX_SOCKET;

typedef struct whdmi_msg_tcp_send {
	WHDMI_MSG_HDR msg_hdr;
	int socket_handle;	/* Socket handle to use */
	int data_len;		/* Length of data to send */
	uint8_t data[WHDMI_MAX_PACKET_SIZE];	/* Buffer containing data to send */
} WHDMI_MSG_TCP_SEND;

typedef struct whdmi_msg_udp_send_to {
	WHDMI_MSG_HDR msg_hdr;
	int socket_handle;	/* Socket handle to use */
	unsigned long dest_addr;	/* Destination address to send to (in host order) */
	unsigned short dest_port;	/* Destination port (in host order) */
	int data_len;		/* Length of data to send */
	uint8_t data[WHDMI_MAX_PACKET_SIZE];	/* Buffer containing data to send */
} WHDMI_MSG_UDP_SEND_TO;

typedef struct whdmi_msg_close_socket {
	WHDMI_MSG_HDR msg_hdr;
	int socket_handle;	/* Socket handle to close */
} WHDMI_MSG_CLOSE_SOCKET;

typedef union whdmi_msg_all {
	WHDMI_MSG_HDR hdr;
	WHDMI_MSG_CREATE_TCP_LISTENING_SOCKET create_tcp_listening_socket;
	WHDMI_MSG_CREATE_UDP_SOCKET create_udp_socket;
	WHDMI_MSG_CREATE_UDP_TX_SOCKET create_udp_tx_socket;
	WHDMI_MSG_TCP_SEND tcp_send;
	WHDMI_MSG_UDP_SEND_TO udp_send_to;
	WHDMI_MSG_CLOSE_SOCKET close_socket;
} WHDMI_MSG_ALL;

/* ---- Variable Externs ------------------------------------------ */

/* ---- Function Prototypes --------------------------------------- */

#endif /* WHDMI_IOCTL_H */
