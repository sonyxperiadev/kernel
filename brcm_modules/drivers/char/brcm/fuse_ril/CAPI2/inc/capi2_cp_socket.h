/****************************************************************************
*
*     Copyright (c) 2008 Broadcom Corporation
*           All Rights Reserved
*
*     No portions of this material may be reproduced in any form without the
*     written permission of:
*
*           Broadcom Corporation
*           16215 Alton Parkway
*           P.O. Box 57013
*           Irvine, California 92619-7013
*
*     All information contained in this document is Broadcom Corporation
*     company private, proprietary, and trade secret.
*
****************************************************************************/
/**
*
*   @file   capi2_cp_socket.h
*
*   @brief  This file defines the socket capi2 api's for communication processor.
*
****************************************************************************/

#ifndef CAPI2_CP_SOCKET_H
#define CAPI2_CP_SOCKET_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _SOCKCALL_H		// avoid duplicate defines with sockcall.h

	#define SYS_SOCKETNULL -1  /**< error return from sys_socket. */
	#define INVALID_SOCKET -1  /**< WINsock-ish synonym for SYS_SOCKETNULL */
	#define SOCKET_ERROR   -1  /**< error return from send(), sendto(), et.al. */

#endif	//#ifndef _SOCKCALL_H

#ifndef _IPPORT_H_		// avoid duplicate defines with ipport.h

	/*********************************************************************************************/
	/*                         To Enable/Disable TCP_ZEROCOPY package option                     */
	/*                         =============================================                     */
	/* (Note: See socketapi.h for details to enable TCP/UDP ZEROCOPY option for a socket.)       */
	/*                                                                                           */
	/* By default, the TCP_ZEROCOPY package option is enabled and built in the current stack.    */
	/* To disable the TCP_ZEROCOPY package option, simply comment out "#define TCP_ZEROCOPY".    */
	/* and open up the following line - "#undef TCP_ZEROCOPY"                                    */
	/*                                                                                           */
	#define TCP_ZEROCOPY            /* By default, TCP_ZEROCOPY package option is enabled.       */
	//#undef TCP_ZEROCOPY             /* to disable                                              */     
	/*                                                                                           */
	/*********************************************************************************************/

#endif	//#ifndef _IPPORT_H_

#ifndef NPTYPES_H	// avoid duplicate defines with nptypes.h

	typedef unsigned long ip_addr;

#endif

#ifndef SOCKET_H	// avoid duplicate defines with socket.h

/// Structure used by kernel to store most addresses.
typedef struct
{
   u_short   sa_family;     /**< address family */
   char   sa_data[14];      /**< up to 14 bytes of direct address */
} sockaddr;

/// Structure used for manipulating linger option.
typedef struct
{
   u_short   l_onoff;      /**< option on/off */
   u_short   l_linger;     /**< linger time */
} linger;

/*
 * Signal Types
 */
#define SOCK_SIG_READABLE	1
#define SOCK_SIG_WRITABLE	2
#define SOCK_SIG_ERROR		3
#define SOCK_SIG_CONNECTED	4

/*
 * Types
 */
#define   SOCK_STREAM   1      /**< stream socket */
#define   SOCK_DGRAM    2      /**< datagram socket */
#ifndef NPPORT
#define   SOCK_RAW      3      /**< raw-protocol interface */
#define   SOCK_RDM      4      /**< reliably-delivered message */
#define   SOCK_SEQPACKET 5     /**< sequenced packet stream */
#endif /* NPPORT*/

/*
 * Option flags per-socket.
 */

#define   SO_DEBUG       0x0001      /**< turn on debugging info recording */
#define   SO_ACCEPTCONN  0x0002      /**< socket has had listen()          */
#define   SO_REUSEADDR   0x0004      /**< allow local address reuse        */
#define   SO_KEEPALIVE   0x0008      /**< keep connections alive           */
#define   SO_DONTROUTE   0x0010      /**< just use interface addresses     */
#define   SO_BROADCAST   0x0020      /**< permit sending of broadcast msgs */
#define   SO_USELOOPBACK 0x0040      /**< bypass hardware when possible    */
#define   SO_LINGER      0x0080      /**< linger on close if data present  */
#define   SO_OOBINLINE   0x0100      /**< leave received OOB data in line  */
#ifdef TCP_ZEROCOPY
// SO_ZEROCOPY is only present if the stack has been built with the TCP_ZEROCOPY package option defined in ipport.h.
#define   SO_ZEROCOPY    0x0200      /**< socket option to enable TCP/UDP ZEROCOPY  */
#endif

/*
 * Additional options, not kept in so_options.
 */

#define SO_SNDBUF    0x1001      /**< send buffer size */
#define SO_RCVBUF    0x1002      /**< receive buffer size */
#define SO_SNDLOWAT  0x1003      /**< send low-water mark */
#define SO_RCVLOWAT  0x1004      /**< receive low-water mark */
#define SO_SNDTIMEO  0x1005      /**< send timeout */
#define SO_RCVTIMEO  0x1006      /**< receive timeout */
#define SO_ERROR     0x1007      /**< get error status and clear */
#define SO_TYPE      0x1008      /**< get socket type */
#define SO_HOPCNT    0x1009      /**< Hop count to get to dst   */
#define SO_MAXMSG    0x1010      /**< get TCP_MSS (max segment size) */

/* ...And some netport additions to setsockopt: */
#define SO_RXDATA    0x1011      /**< get count of bytes in sb_rcv */
#define SO_MYADDR    0x1012      /**< return my IP address */
#define SO_NBIO      0x1013      /**< set socket into NON-blocking mode */
#define SO_BIO       0x1014      /**< set socket into blocking mode */
#define SO_NONBLOCK  0x1015      /**< set/get blocking mode via optval param */

/* Broadcom ZEROCOPY implementation allows TCP/UDP zerocopy to share    */
/* the same callback as registered for SO_SIGNAL, i.e. so_signal(). As  */
/* such, the caller does NOT need to register a different callback      */
/* through setsockopt() using SO_CALLBACK, instead, it is required to   */
/* call setsockopt() with optname SO_ZEROCOPY and optval 1 for the      */
/* socket enabling this feature.                                        */
#define SO_CALLBACK  0x1016      /**< set/get zero_copy callback routine  */

#define SO_SIGNAL	 0x1017		 /**< set the signal notify callback routine */
#define SO_CONTEXT	 0x1018		 /**< reserved to store acctId for multiple pdp context support */
#define SO_CONTEXT2	 0x1019		 /**< set the application specific context */

/*
 * Address families.
 */

#define  AF_UNSPEC   0     /**< unspecified */
#define  AF_NS       1     /**< local to host (pipes, portals) */
#define  AF_INET     2     /**< internetwork: UDP, TCP, etc. */
#if 0
#define  AF_IMPLINK  3     /**< arpanet imp addresses */
#define  AF_PUP      4     /**< pup protocols: e.g. BSP */
#define  AF_CHAOS    5     /**< mit CHAOS protocols */
#define  AF_NS       6     /**< XEROX NS protocols */
#define  AF_NBS      7     /**< nbs protocols */
#define  AF_ECMA     8     /**< european computer manufacturers */
#define  AF_DATAKIT  9     /**< datakit protocols */
#define  AF_CCITT    10    /**< CCITT protocols, X.25 etc */
#define  AF_SNA      11    /**< IBM SNA */
#define  AF_DECnet   12    /**< DECnet */
#define  AF_DLI      13    /**< Direct data link interface */
#define  AF_LAT      14    /**< LAT */
#define  AF_HYLINK   15    /**< NSC Hyperchannel */
#define  AF_APPLETALK 16   /**< Apple Talk */
#endif
#define  AF_MAX      (AF_INET + 1)

/*
 * Protocol families, same as address families for now.
 */
#define  PF_UNSPEC   AF_UNSPEC
#define  PF_UNIX     AF_UNIX
#define  PF_INET     AF_INET
#define  PF_IMPLINK  AF_IMPLINK
#define  PF_PUP      AF_PUP
#define  PF_CHAOS    AF_CHAOS
#define  PF_NS       AF_NS
#define  PF_NBS      AF_NBS
#define  PF_ECMA     AF_ECMA
#define  PF_DATAKIT  AF_DATAKIT
#define  PF_CCITT    AF_CCITT
#define  PF_SNA      AF_SNA
#define  PF_DECnet   AF_DECnet
#define  PF_DLI      AF_DLI
#define  PF_LAT      AF_LAT
#define  PF_HYLINK   AF_HYLINK
#define  PF_APPLETALK   AF_APPLETALK
#define  PF_MAX      AF_MAX

/*
 * Maximum queue length specifiable by listen.
 */
#define   SOMAXCONN   5

#define   MSG_OOB			0x1     /**< process out-of-band data */
#define   MSG_PEEK			0x2     /**< peek at incoming message */
#define   MSG_DONTROUTE		0x4		/**< send without using routing tables */
#define   MSG_NEWPIPE		0x8		/**< New pipe for recvfrom call   */
#define   MSG_EOR			0x10    /**< data completes record */
#define   MSG_DONTWAIT		0x20    /**< this message should be nonblocking */

#define   MSG_MAXIOVLEN   16

#endif	//#ifndef SOCKET_H

/// Socket option value types, used by CAPI2_SOCKET_GetSockOpt and
/// CAPI2_SOCKET_SetSockOpt.
typedef union
{
	linger	optval_linger;
	Int32	optval_int32;
	Int16	optval_int16;

} SockOptVal_t;


//typedef void (*socket_sigcb)(struct socket * so, int sig_id, int error);
typedef void (*socket_sigcb)(void * so, int sig_id, int error);

/**
*	The payload for @a ::MSG_CAPI2_SOCKET_SIGNAL_IND.  This message is
*	sent from the AP to the CP when the SO_SIGNAL callback is called,
*	and indicates a	socket connect, read, write, or error event.
*
*	The socket callback prototype:
*
*	void socketSignalCallback(struct socket * so, int sig_id, int va_param)
*/
typedef struct
{
	socket_sigcb	callback;	///< The socket signal callback, on the CP side.
	void*			socket;		///< Treated as a handle to a socket structure, which is a valid structure on the AP.
	Int32			sig_id;		///< Signal ID.
	Int32			error;		///< Socket error code.

} SocketSignalInd_t;

/// The input parameter structure passed for CAPI2_SOCKET_Send().
typedef struct
{
	/// A socket descriptor created with t_socket().
	Int32	descriptor;
	char*	msg;			///< A pointer to the data to be sent.

	/// The length of the data, in bytes.
	/// @note	If the data buffer is longer than a single segment, this is
	///			the length of the current segment only.
	UInt32	msgLen;

	/// Flags that control how the data is to be sent.
	/// The flags parameter is formed from the bitwise OR of zero or 
	/// more of the following:
	///	- ::MSG_OOB: Sends out-of-band data.
	///		Only SOCK_STREAM sockets support out-of-band data.
	///	- ::MSG_DONTROUTE: The SO_DONTROUTE option is turned on
	///		for the duration of the operation. It is used only by
	//		diagnostic or routing programs.					
	UInt16	flags;

} SocketSendReq_t;

/// The input parameter structure passed for CAPI2_SOCKET_Recv().
typedef struct
{
	/// The identifier of the socket from which the messages are received.
	/// The socket is created with t_socket().
	Int32 descriptor;

	/// The maximum buffer size of each segment that is received.
	UInt32 bufferLength;

	/// Flags formed by ORing zero or more of the following:
	///	- ::MSG_OOB Reads any out-of-band data present on the socket,
	///		rather than the regular in-band data.
	///	- ::MSG_PEEK Looks at the data present on the socket.
	///		The data is returned, but not consumed, so a subsequent
	///		receive operation will see the same data.
	UInt16 flags;

} SocketRecvReq_t;

/// Structure to pass the recv response data.
typedef struct
{
	char*	buffer;		///< Received buffer.

	/// The number of bytes received, if successful.
	/// 	-1 If not successful. The internal socket variable errno is set to one of the errors listed in ipport.h. The value of errno 
	/// 	can be retrieved by a call to t_errno(socket).
	///
	/// @note	Note that if the buffer is separated into segments,
	///			numBytes refers to the number of bytes in this segment.
	Int32	numBytes;

} SocketRecvRsp_t;

/**
 * @addtogroup CAPI2_CP_APIGroup
 * @{
 */

//***************************************************************************************
/**
This function creates an endpoint for communication and returns a descriptor.

SOCK_STREAM sockets are full duplex byte streams, similar to UNIX pipes. A stream socket must be in a connected state 
before it can send or receive data.  

A connection to another socket is created with a connect() call. When connected, data can be transferred using 
send() and recv(). When a session has been completed, socketclose() can be performed. Out-of-band data can also be 
transmitted and received, as described in the send() and recv() documentation.

The communications protocols used to implement a SOCK_STREAM ensure that data is not lost or duplicated. 
If a piece of data (for which the peer protocol has buffer space) cannot be transmitted successfully 
within a reasonable length of time, the connection is considered broken. In this case, calls return 
with a value of -1 and ETIMEDOUT is written to the internal variable errno.

The protocols optionally keep sockets warm by forcing transmissions roughly every minute in the absence of other activity. 
An error is indicated if no response has been received on an otherwise idle connection for an extended period, 
for example, five minutes.

SOCK_DGRAM sockets allow datagrams to be sent to correspondents named in sendto() calls. Datagrams are generally 
received with recvfrom(), which returns the next datagram with its return address.  

The operation of sockets is controlled by socket-level options. These options are defined in the file socket.h. 
The getsockopt() and setsockopt() functions are used to get and set options, respectively 

	@param tid		(in) Unique exchange/transaction id which is passed back in the response
	@param clientID (in) Client ID
	@param domain	(in)	Specifies a communications domain within which communication takes place. It selects the protocol
							family that should be used. The protocolfamily is typically the same as the address family for the addresses
							supplied in later operations on the socket. These families are defined in the include file socket.h. 
							The only currently understood format is PF_INET (ARPA Internet protocols).
	@param type		(in)	Specifies the semantics of communication. Currently allowed types are:
							SOCK_STREAM: (TCP) Provides sequenced, reliable, two-way connection-based byte streams. An out-of-band
							data transmission mechanism can be supported.
							SOCK_DGRAM: (UDP) Supports datagrams, connectionless,unreliable messages of a fixed (typically small) maximum length.
	@param protocol	(in)	Is the protocol to use. It must be set to zero for ARM IP.

	@return		None.

	@note
	The async response is as follows:
	@n@b Responses 
	@n@b MsgType_t :	::MSG_SOCKET_OPEN_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	descriptor: A descriptor for the accepted socket, if successful.
						-1 If not successful.
**/
void CAPI2_SOCKET_Open(UInt32 tid, UInt8 clientID, UInt8 domain, UInt8 type, UInt8 protocol);

//***************************************************************************************
/**
	Function response for CAPI2_SOCKET_Open.
	@param		tid (in) Unique exchange/transaction id which is passed in the request
	@param		clientID (in) Client ID
	@param		descriptor (in) The response value. A descriptor for the accepted socket,
								if successful. -1 if not successful.
	@return		Not Applicable
	@note
	Return or Response is not applicable
	@n@b Response to CP will be notified via ::MSG_SOCKET_OPEN_RSP
**/
void CAPI2_SOCKET_Open_RSP(UInt32 tid, UInt8 clientID, Int32 descriptor);

//***************************************************************************************
/**
This function assigns a name to an unnamed socket. When a socket is created with socket(), 
it exists in a name space (address family) but has no name assigned.

int t_bind(long socket, struct sockaddr *name)

@param tid			(in) Unique exchange/transaction id which is passed in the request
@param clientID		(in) Client ID
@param descriptor	(in) Is the identifier of the unnamed socket to be bound.
@param addr			(in) Is the IP address and port number to be assigned to socket.

	@note
	The async response is as follows:
	@n@b Responses 
	@n@b MsgType_t :	::MSG_SOCKET_BIND_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	Returns one of the following:
						0 If successful.
						-1 If not successful. The internal socket variable errno
						is set to one of the errors listed in ipport.h. 
						The value of errno can be retrieved by a call to t_errno(socket).
**/
void CAPI2_SOCKET_Bind(UInt32 tid, UInt8 clientID, Int32 descriptor, sockaddr *addr);

//***************************************************************************************
/**
	Function response for CAPI2_SOCKET_Bind.
	@param		tid (in) Unique exchange/transaction id which is passed in the request
	@param		clientID (in) Client ID
	@param		status (in) The response value. 
			0 If successful.
			-1 If not successful. The internal socket variable errno is set to one of the errors listed in ipport.h. 
				The value of errno can be retrieved by a call to t_errno(socket).
	@return		Not Applicable
	@note
	Return or Response is not applicable
	@n@b Response to CP will be notified via ::MSG_SOCKET_BIND_RSP
**/
void CAPI2_SOCKET_Bind_RSP(UInt32 tid, UInt8 clientID, Int8 status);

//***************************************************************************************
/**
This function tells the socket library that socket is going to be used for accepting connections from other hosts.

To accept connections:

1.	A socket is created with socket().
2.	A backlog for incoming connections is specified with listen().
3.	The connections are then accepted with accept().

The listen() call applies only to SOCK_STREAM sockets.

int t_listen(long socket, int backlog)
	@param tid			(in) Unique exchange/transaction id which is passed in the request
	@param clientID		(in) Client ID
	@param descriptor	(in) Is the socket used for accepting connections.
	@param backlog		(in) Defines the maximum length to which the queue of pending connections can grow. 
							If a connection request arrives when the queue is full, the client receives the error message ECONNREFUSED.

	@note
	The async response is as follows:
	@n@b Responses 
	@n@b MsgType_t :	::MSG_SOCKET_LISTEN_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	Returns one of the following:
						0 If successful.
						-1 If not successful. The internal socket variable errno
						is set to one of the errors listed in ipport.h. 
						The value of errno can be retrieved by a call to t_errno(socket).
**/
void CAPI2_SOCKET_Listen(UInt32 tid, UInt8 clientID, Int32 descriptor, UInt32 backlog);

//***************************************************************************************
/**
	Function response for CAPI2_SOCKET_Listen.
	@param		tid (in) Unique exchange/transaction id which is passed in the request
	@param		clientID (in) Client ID
	@param		status (in) The response value. 
			0 If successful.
			-1 If not successful. The internal socket variable errno is set to one of the errors listed in ipport.h. 
				The value of errno can be retrieved by a call to t_errno(socket).
	@return		Not Applicable
	@note
	Return or Response is not applicable
	@n@b Response to CP will be notified via ::MSG_SOCKET_LISTEN_RSP
**/
void CAPI2_SOCKET_Listen_RSP(UInt32 tid, UInt8 clientID, Int8 status);

//***************************************************************************************
/**
This function is used to accept a connection from a remote host.

The accept() function is used with connection-based socket types, currently with SOCK_STREAM.  
This function extracts the first connection on the queue of pending connections, creates
a new socket with the same properties as socket, and allocates a new socket descriptor for the socket.

If no pending connections are present on the queue and the socket is not marked as nonblocking, 
accept() blocks the caller until a connection is present. If the socket is marked as nonblocking 
and no pending connections are present on the queue, accept() returns -1 and sets the socket errno to EWOULDBLOCK.

The accepted socket is used to read data to and write data from the socket that connected to this one. 
It is not used to accept more connections. The original socket, socket, remains open for accepting further 
connections.

It is possible to select() a socket for the purposes of doing a accept() by selecting it for read.

long t_accept(long socket, struct sockaddr *addr)

	@param tid			(in) Unique exchange/transaction id which is passed in the request
	@param clientID		(in) Client ID
	@param descriptor	(in) Is a socket created with t_socket(),
								bound to an address with t_bind(),
								and is waiting for connections after a t_listen().

	@note
	The async response is as follows:
	@n@b Responses 
	@n@b MsgType_t :	::MSG_SOCKET_ACCEPT_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	The returned IP address and port number of the connecting entity
						(as known to the communications layer).
						It also contains one of the following:
						descriptor: A descriptor for the accepted socket if successful.
						-1: If not successful. On failure, the t_accept() function sets
						an internal socket variable, errno, to one of the errors listed in
						ipport.h. The value of errno can be retrieved by a call to t_errno(socket).
**/
void CAPI2_SOCKET_Accept(UInt32 tid, UInt8 clientID, Int32 descriptor);

//***************************************************************************************
/**
	Function response for CAPI2_SOCKET_Accept.
	@param		tid (in) Unique exchange/transaction id which is passed in the request
	@param		clientID (in) Client ID
	@param	name	(in) Socket address, which includes the IP address and port number
						of the connecting entity (as known to the communications layer).
	@param	acceptDescriptor (in) A descriptor for the accepted socket if successful, or
						-1: If not successful. On failure, the t_accept() function sets
						an internal socket variable, errno, to one of the errors listed in
						ipport.h. The value of errno can be retrieved by a call to t_errno(socket).

	@return		Not Applicable
	@note
	Return or Response is not applicable
	@n@b Response to CP will be notified via ::MSG_SOCKET_ACCEPT_RSP
**/
void CAPI2_SOCKET_Accept_RSP(UInt32 tid, UInt8 clientID, sockaddr* name, Int32 acceptDescriptor);

/**
This function creates a socket connection.

Generally, SOCK_STREAM sockets can successfully use connect() only once.  SOCK_DGRAM sockets can use connect() 
multiple times to change their association.  Datagram sockets may dissolve the association by connecting to an 
invalid address, such as a null address.

For asynchronous connect, socket option SO_SIGNAL can be used by application calling setsockopt() to get notified(by setting a callback 
function) for the "socket connection done" with signals such as SOCK_SIG_CONNECTED.

int t_connect(long socket, struct sockaddr *name)

	@param tid			(in) Unique exchange/transaction id which is passed in the request
	@param clientID		(in) Client ID
	@param descriptor	(in)	Is created by t_socket(). It is bound to an IP address
						and port number using t_bind().If the type, as determined 
						when the socket was created using t_socket(), is SOCK_DGRAM,
						t_connect() specifies the peer with which the socket 
						is to be associated. This is the address to which datagrams
						are sent and is the only address from which datagrams are received.
						If the type is SOCK_STREAM, t_connect() attempts to make a 
						connection to another socket.
	@param name	(in)	Is an address in the communications space of the remote socket.
						Each communications space interprets the name parameter 
						in its own way.

	@note
	The async response is as follows:
	@n@b Responses 
	@n@b MsgType_t :	::MSG_SOCKET_CONNECT_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	0 If successful.
						-1 If not successful. The internal socket variable errno
						is set to one of the errors listed in ipport.h. 
						The value of errno can be retrieved by a call to t_errno(socket).
**/
void CAPI2_SOCKET_Connect(UInt32 tid, UInt8 clientID, Int32 descriptor, sockaddr *name);

//***************************************************************************************
/**
	Function response for CAPI2_SOCKET_Accept.
	@param	tid (in) Unique exchange/transaction id which is passed in the request
	@param	clientID (in) Client ID
	@param	status (in) The response value. 
				0 If successful.
				-1 If not successful. The internal socket variable errno is set
					to one of the errors listed in ipport.h. 
					The value of errno can be retrieved by a call to t_errno(socket).
	@return		Not Applicable
	@note
	Return or Response is not applicable
	@n@b Response to CP will be notified via ::MSG_SOCKET_CONNECT_RSP
**/
void CAPI2_SOCKET_Connect_RSP(UInt32 tid, UInt8 clientID, Int8 status);

//***************************************************************************************
/**
This function returns the IP addressing information of the connected host.

int t_getpeername(long socket, struct sockaddr *name)

@param tid			(in) Unique exchange/transaction id which is passed in the request
@param clientID		(in) Client ID
@param descriptor	(in) Is the socket on which addressing information for the remote, connectedhost is returned.

	@note
	The async response is as follows:
	@n@b Responses 
	@n@b MsgType_t :	::MSG_SOCKET_GETPEERNAME_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	The status of the operation and the name of the specified socket.
						The "name" contains IP address and port number information for socket.
						If the status is -1, the operation was not successful.
						The internal socket variable errno is set to one of the
						errors listed in ipport.h. 
						The value of errno can be retrieved by a call to t_errno(socket).
**/
void CAPI2_SOCKET_GetPeerName(UInt32 tid, UInt8 clientID, Int32 descriptor);

//***************************************************************************************
/**
	Function response for CAPI2_SOCKET_GetPeerName.
	@param	tid (in) Unique exchange/transaction id which is passed in the request
	@param	clientID (in) Client ID
	@param	peerName	(in) Socket address, which includes the IP address and port number
						information for the socket.
	@param	status (in) Status of the request.
 						0 If successful.
						-1 If not successful. The internal socket variable errno
						is set to one of the errors listed in ipport.h. 
						The value of errno can be retrieved by a call to t_errno(socket).
	@return		Not Applicable
	@note
	Return or Response is not applicable
	@n@b Response to CP will be notified via ::MSG_SOCKET_GETPEERNAME_RSP
**/
void CAPI2_SOCKET_GetPeerName_RSP(UInt32 tid, UInt8 clientID, sockaddr* peerName, Int8 status);

//***************************************************************************************
/**
This function returns the current name for the specified socket.

int t_getsockname(long socket, struct sockaddr *name)

@param tid			(in) Unique exchange/transaction id which is passed in the request
@param clientID		(in) Client ID
@param descriptor	(in)	Is the identifier of the socket to be named.

	@note
	The async response is as follows:
	@n@b Responses 
	@n@b MsgType_t :	::MSG_SOCKET_GETSOCKNAME_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	The status of the operation and the name of the specified socket.
						The "name" contains IP address and port number information for socket.
						If the status is -1, the operation was not successful.
						The internal socket variable errno is set to one of the
						errors listed in ipport.h. 
						The value of errno can be retrieved by a call to t_errno(socket).
**/
void CAPI2_SOCKET_GetSockName(UInt32 tid, UInt8 clientID, Int32 descriptor);

//***************************************************************************************
/**
	Function response for CAPI2_SOCKET_GetSockName.
	@param	tid (in) Unique exchange/transaction id which is passed in the request
	@param	clientID (in) Client ID
	@param	sockName	(in) Socket address, which includes the IP address and port number
						information for the socket.
	@param	status (in) Status of the request.
 						0 If successful.
						-1 If not successful. The internal socket variable errno
						is set to one of the errors listed in ipport.h. 
						The value of errno can be retrieved by a call to t_errno(socket).
	@return		Not Applicable
	@note
	Return or Response is not applicable
	@n@b Response to CP will be notified via ::MSG_SOCKET_GETSOCKNAME_RSP
**/
void CAPI2_SOCKET_GetSockName_RSP(UInt32 tid, UInt8 clientID, sockaddr* sockName, Int8 status);

//***************************************************************************************
/**
This function sets the options associated with a socket.

int t_setsockopt(long socket, int optname, void *optval)

@param descriptor	(in)	Is the identifier of the socket to be changed.
@param optname		(in)	Is the name of the option to be set.
<ul>
<li>SO_BIO			This sets the socket to blocking mode. Operations on the socket are blocked until completion.
<li>SO_BROADCAST	This boolean value requests permission to send broadcast datagrams on the socket. Broadcast was a privileged operation in
					earlier versions of the system.
<li>SO_CALLBACK		This registers a callback function for use with the TCP Zero-Copy API. 
<li>SO_DONTROUTE	This boolean value indicates that outgoing messages must bypass the standard routing facilities. Instead, messages are 
					directed to the appropriate network interface according to the network portion of the destination address.
<li>SO_KEEPALIVE	This boolean value enables the periodic transmission of messages on a connected socket. If the connected party fails to 
					respond to these messages, the connection is considered broken.
<li>SO_LINGER		This option controls the action taken when unsent messages are queued on socket and a t_socketclose() is performed.
					If the socket promises reliable delivery of data and SO_LINGER is set, the system blocks the process on the t_socketclose()
					attempt until it is able to transmit the data or until it decides it is unable to deliver the information 
					(a timeout period, termed the linger interval, is specified in the t_setsockopt() call when SO_LINGER is requested).
					If SO_LINGER is disabled and a t_socketclose() is issued, the system will process the close in a manner that allows 
					the process to continue as quickly as possible. SO_LINGER uses a pointer to a struct linger parameter,
					defined in socket.h, that specifies the desired state of the option and the linger interval.
<li>SO_NBIO			This sets the socket to nonblocking mode. If further operations on the socket cannot complete immediately, 
					they return -1 and set the socket errno variable to EWOULDBLOCK.
<li>SO_NOBLOCK		This boolean value enables or disables blocking mode on the socket. If optval is set to zero, the socket is in blocking mode. 
					If optval is set to a nonzero value, the socket is in nonblocking mode.
<li>SO_OOBINLINE	With protocols that support out-of-band data, this boolean option requests that out-of-band data be placed in the normal data 
					input queue as received. It will then be accessible with t_recv().
<li>SO_REUSEADDR	This boolean value indicates that the rules used in validating addresses supplied in a t_bind() call must allow reuse of 
					local addresses.
<li>SO_SNDBUF, SO_RCVBUF	These are options to adjust the normal buffer sizes allocated for output and input buffers, respectively. 
					The buffer size can be increased for high-volume connections or can be decreased to limit the possible backlog of incoming data. 
					The system places an absolute limit of 8KB on these values.
<li>SO_CONTEXT		Used by applications to associate their context id (account id) with the socket descriptor.
<li>SO_CONTEXT2		NOT to be used by applications; for internal CAPI2 use only. 
					The Comms processor socket signal callback function pointer is saved here
					when a callback is setup via SO_SIGNAL. The Apps processor cannot call the CP
					callback directly, so it passes this callback as a handle to the CP via
					CAPI2_SOCKET_SignalInd().
<li>SO_SIGNAL		Used by applications to setup callback functions for asynchrounous connect,read and write. 
					The callback function prototype:
					void socketSignalCallback(struct socket * so, int sig_id, int error)
					where
					so: Pointer to socket structure. Application need call long fd = SO2LONG(so) to convert it to the socket descriptor.
					sig_id: Socket signal id (SOCK_SIG_READABLE, SOCK_SIG_WRITABLE, SOCK_SIG_ERROR or SOCK_SIG_CONNECTED).
					error: Socket error such as ENOBUFS(1), ECONNRESET(8) or ESHUTDOWN(15). 0 if no error.
</ul>

	@param tid		(in) Unique exchange/transaction id which is passed in the request
	@param clientID	(in) Client ID
	@param optval	(in)	Is the value the option will be set to. The parameter should
						be nonzero to enable a boolean option, or zero 
						if the option is to be disabled. Most socket-level
						options take an int parameter for optval.

	@note
	The async response is as follows:
	@n@b Responses 
	@n@b MsgType_t :	::MSG_SOCKET_SETSOCKOPT_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	The response value. 
						0 If successful.
						-1 If not successful. The internal socket variable errno is set
						to one of the errors listed in ipport.h. 
						The value of errno can be retrieved by a call to t_errno(socket).
**/
void CAPI2_SOCKET_SetSockOpt(UInt32 tid, UInt8 clientID, Int32 descriptor, UInt16 optname, SockOptVal_t* optval);

//***************************************************************************************
/**
	Function response for CAPI2_SOCKET_SetSockOpt.
	@param	tid (in) Unique exchange/transaction id which is passed in the request
	@param	clientID (in) Client ID
	@param	status (in) The response value. 
						0 If successful.
						-1 If not successful. The internal socket variable errno
						is set to one of the errors listed in ipport.h. 
						The value of errno can be retrieved by a call to t_errno(socket).
	@return		Not Applicable
	@note
	Return or Response is not applicable
	@n@b Response to CP will be notified via ::MSG_SOCKET_SETSOCKOPT_RSP
**/
void CAPI2_SOCKET_SetSockOpt_RSP(UInt32 tid, UInt8 clientID, Int8 status);

/**
This function returns the options associated with a socket.

int t_getsockopt(long socket, int optname, void *optval)

	@param tid		(in) Unique exchange/transaction id which is passed in the request
	@param clientID	(in) Client ID
	@param descriptor	(in)	Is the socket for which the option values are returned.
	@param optname	(in)	Is the name of the option to be examined. The options are discussed in more detail below.
<ul>
<li>SO_BIO			This returns the value 1 if the socket is currently set to be blocking.
<li>SO_BROADCAST	If optval is nonzero, this indicates that datagrams may be broadcast on this socket.
<li>SO_DONTROUTE	If optval is nonzero, this indicates that outgoing messages must bypass the standard routing facilities. Instead, messages 
					are directed to the appropriate network interface according to the network portion of the destination address.
<li>SO_ERROR		This returns any pending error on the socket and clears the error status. It can be used to check for asynchronous errors 
					on connected datagram sockets or for other asynchronous errors.
<li>SO_KEEPALIVE	If optval is nonzero, this indicates that periodic transmission of messages on a connected socket is enabled.
<li>SO_LINGER		This indicates the action taken when unsent messages are queued on socket and a t_socketclose() is performed. If the socket 
					promises reliable delivery of data and SO_LINGER is set, the system blocks the process on the t_socketclose() attempt until 
					it is able to transmit the data or until it decides it is unable to deliver the information. A timeout period, known as the 
					linger interval, is specified in the t_setsockopt() call when SO_LINGER is requested.
					If SO_LINGER is disabled and a t_socketclose() is issued, the system processes the close in a manner that allows the process 
					to continue as quickly as possible. SO_LINGER uses a pointer to a struct linger parameter, defined in socket.h, that 
					specifies the desired state of the option and the linger interval.
<li>SO_MAXMSG		This returns the TCP maximum segment size (TCP_MSS) as defined in tcpport.h.
<li>SO_MYADDR		This returns the IP address of the primary network interface for this host.
<li>SO_NBIO, SO_NONBLOCK These return the value 1 if the socket is currently set to be nonblocking.
<li>SO_OOBINLINE	If optval is nonzero, this option indicates that out-of-band data is placed in the normal data input queue as received. 
					It is then accessible through t_recv() calls without the ::MSG_OOB flag. This is valid with protocolsthat support out-of-band data.
<li>SO_REUSEADDR	If optval is nonzero, SO_REUSADDR indicates that the rules used in validating addresses supplied in a t_bind() call allow 
					reuse of local addresses.
<li>SO_RXDATA		This returns the number of characters currently available for reading from the socket.
<li>SO_SNDBUF and SO_RCVBUF These return the buffer sizes allocated for output and input buffers, respectively.
<li>SO_TYPE			This returns the type of the socket, such as SOCK_STREAM. It is useful for servers that inherit sockets on startup.
<li>SO_CONTEXT		Used by applications to get the optval of context id (or account id) associated with the socket descriptor
<li>SO_CONTEXT2		Used by applications to associate any arbitary context pointer with the socket descriptor.
</ul>

  @note
	The async response is as follows:
	@n@b Responses 
	@n@b MsgType_t :	::MSG_SOCKET_GETSOCKOPT_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	The socket option value and status:
						0 If successful.
						-1 If not successful. The internal socket variable errno is set
						to one of the errors listed in ipport.h. 
						The value of errno can be retrieved by a call to t_errno(socket).
**/
void CAPI2_SOCKET_GetSockOpt(UInt32 tid, UInt8 clientID, Int32 descriptor, UInt16 optname);

//***************************************************************************************
/**
	Function response for CAPI2_SOCKET_GetSockOpt.
	@param	tid (in) Unique exchange/transaction id which is passed in the request
	@param	clientID (in) Client ID
	@param	optval	(in) The requested option data.
						Other than SO_LINGER, most socket-level options 
						take a pointer to an int parameter for optval.
	@param	status (in) The status can be one of the following:
						0 If successful.
						-1 If not successful. The internal socket variable errno
						is set to one of the errors listed in ipport.h. 
						The value of errno can be retrieved by a call to t_errno(socket).
	@return		Not Applicable
	@note

	Return or Response is not applicable
	@n@b Response to CP will be notified via ::MSG_SOCKET_GETSOCKOPT_RSP
**/
void CAPI2_SOCKET_GetSockOpt_RSP(UInt32 tid, UInt8 clientID, SockOptVal_t* optval, Int8 status);

//***************************************************************************************
/**
	Transmits a message.
	You can only use the send() function when the socket is in a connected state.
	The sendto() function can be used at any time.  

	If the socket does not have enough buffer space available to hold the message
	being sent, the send() functions block, unless the socket has been placed in
	nonblocking input/output mode.

int t_send(int s, char *msg, int len, int flags)

	@param tid		(in) Unique exchange/transaction id which is passed in the request
	@param clientID	(in) Client ID
	@param sockSendReq	(in) Socket send parameters. See SocketSendReq_t for details.

	@note
	The async response is as follows:
	@n@b Responses 
	@n@b MsgType_t :	::MSG_SOCKET_SEND_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	The status is one of the following:
						number The number of bytes received, if successful.
						-1 If not successful. The internal socket variable errno is set
						to one of the errors listed in ipport.h. 
						The value of errno can be retrieved by a call to t_errno(socket).
**/
void CAPI2_SOCKET_Send(UInt32 tid, UInt8 clientID, SocketSendReq_t* sockSendReq);

//***************************************************************************************
/**
	Function response for CAPI2_SOCKET_Send.
	@param	tid (in) Unique exchange/transaction id which is passed in the request
	@param	clientID (in) Client ID
	@param	bytesSent (in) The bytesSent is one of the following:
						number The number of bytes received, if successful.
						-1 If not successful. The internal socket variable errno is set
						to one of the errors listed in ipport.h. 
						The value of errno can be retrieved by a call to t_errno(socket).
	@return		Not Applicable
	@note
	Return or Response is not applicable
	@n@b Response to CP will be notified via ::MSG_SOCKET_SEND_RSP
**/
void CAPI2_SOCKET_Send_RSP(UInt32 tid, UInt8 clientID, Int32 bytesSent);

//***************************************************************************************
/**
	Transmits a message.

	You can only use the send() function when the socket is in a connected state.
	The sendto() function can be used at any time.  

	If the socket does not have enough buffer space available to hold the message
	being sent, the send() functions block, unless the socket has been placed in
	nonblocking input/output mode.

int t_sendto(int s, char *msg, int len, int flags, struct sockaddr *to)

	@param tid		(in) Unique exchange/transaction id which is passed in the request
	@param clientID	(in) Client ID
	@param sockSendReq	(in) Socket send parameters. See SocketSendReq_t for details.
	@param to	(in)	Is the destination to which the data is to be sent.

	@note
	The async response is as follows:
	@n@b Responses 
	@n@b MsgType_t :	::MSG_SOCKET_SENDTO_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	The status is one of the following:
						number The number of bytes received, if successful.
						-1 If not successful. The internal socket variable errno is set
						to one of the errors listed in ipport.h. 
						The value of errno can be retrieved by a call to t_errno(socket).
**/
void CAPI2_SOCKET_SendTo(UInt32 tid, UInt8 clientID, SocketSendReq_t* sockSendReq, sockaddr* to);

//***************************************************************************************
/**
	Function response for CAPI2_SOCKET_SendTo.
	@param	tid (in) Unique exchange/transaction id which is passed in the request
	@param	clientID (in) Client ID
	@param	bytesSent (in) The bytesSent is one of the following:
						number The number of bytes received, if successful.
						-1 If not successful. The internal socket variable errno is set
						to one of the errors listed in ipport.h. 
						The value of errno can be retrieved by a call to t_errno(socket).
	@return		Not Applicable
	@note
	Return or Response is not applicable
	@n@b Response to CP will be notified via ::MSG_SOCKET_SENDTO_RSP
**/
void CAPI2_SOCKET_SendTo_RSP(UInt32 tid, UInt8 clientID, Int32 bytesSent);

//***************************************************************************************
/**
	Receives messages from another socket.

	You can only use the recv() function on a connected socket (see connect()).  The recvfrom() function 
	can be used to receive data on a socket, whether it is in a connected state or not.

	If no messages are available at the socket and the socket is blocking, the receive call waits for a message to arrive. 
	If the socket is nonblocking -1 is returned, with the external socket errno set to EWOULDBLOCK.

	You can use the select() function to determine when more data arrives.

	For asynchronous receive, socket option SO_SIGNAL can be used by application to get notified
	(by setting a callback function) for the "socket recieved data" with SOCK_SIG_READABLE signal.

int t_recv(long socket, char *buffer, int length, int flags)
int t_recvfrom(long socket, char *buffer, int length, int flags,struct sockaddr *from)

	@param tid		(in) Unique exchange/transaction id which is passed in the request
	@param clientID	(in) Client ID
	@param sockRecvReq (in) The socket receive parameters. See SocketRecvReq_t for details.

	@note
	The async response is as follows:
	@n@b Responses 
	@n@b MsgType_t :	::MSG_SOCKET_RECV_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	A structure containing the number of bytes received, or
						-1 if not successful. The internal socket variable errno is set
						to one of the errors listed in ipport.h. 
						The value of errno can be retrieved by a call to t_errno(socket).
						and a context pointer.
						The structure also contains a context pointer, which is used in the
						case of large buffer transfers. See SocketRecvReq_t for details.
**/
void CAPI2_SOCKET_Recv(UInt32 tid, UInt8 clientID, SocketRecvReq_t* sockRecvReq);

//***************************************************************************************
/**
	Function response for CAPI2_SOCKET_Recv.
	@param		tid (in) Unique exchange/transaction id which is passed in the request
	@param		clientID (in) Client ID
	@param		recvRspData (in) A structure containing the number of bytes received, or
						-1 if not successful. The internal socket variable errno is set
						to one of the errors listed in ipport.h. 
						The value of errno can be retrieved by a call to t_errno(socket).
						and a context pointer.
						The structure also contains a context pointer, which is used in the
						case of large buffer transfers. See SocketRecvReq_t for details.
	@return		Not Applicable
	@note
	Return or Response is not applicable
	@n@b Response to CP will be notified via ::MSG_SOCKET_RECV_RSP
**/
void CAPI2_SOCKET_Recv_RSP(UInt32 tid, UInt8 clientID, SocketRecvRsp_t* recvRspData);

//***************************************************************************************
/**
These functions are used to receive messages from another socket.

int t_recvfrom(long socket, char *buffer, int length, int flags,struct sockaddr *from)
	@param tid		(in) Unique exchange/transaction id which is passed in the request
	@param clientID	(in) Client ID
	@param sockRecvReq (in) The socket receive parameters. See SocketRecvReq_t for details.
	@param from		(in)	Is either NULL, or points to a struct sockaddr that will be
							filled in by t_recvfrom() with the source address 
							of the message.

	@note
	The async response is as follows:
	@n@b Responses 
	@n@b MsgType_t :	::MSG_SOCKET_RECV_FROM_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	A structure containing the number of bytes received, or
						-1 if not successful. The internal socket variable errno is set
						to one of the errors listed in ipport.h. 
						The value of errno can be retrieved by a call to t_errno(socket).
						and a context pointer.
						The structure also contains a context pointer, which is used in the
						case of large buffer transfers. See SocketRecvReq_t for details.

  @return		Returns one of the following:
			number The number of bytes received, if successful.
			-1 If not successful. The internal socket variable errno is set to one of the errors listed in ipport.h. The value of errno 
			can be retrieved by a call to t_errno(socket).
**/
void CAPI2_SOCKET_RecvFrom(UInt32 tid, UInt8 clientID, SocketRecvReq_t* sockRecvReq, sockaddr* from);

//***************************************************************************************
/**
	Function response for CAPI2_SOCKET_RecvFrom.
	@param	tid (in) Unique exchange/transaction id which is passed in the request
	@param	clientID (in) Client ID
	@param	recvRspData The number of bytes received, if successful.
			-1 If not successful. The internal socket variable errno is set to one of the errors listed in ipport.h. The value of errno 
			can be retrieved by a call to t_errno(socket).
	@return		Not Applicable
	@note
	Return or Response is not applicable
	@n@b Response to CP will be notified via ::MSG_SOCKET_RECVFROM_RSP
**/
void CAPI2_SOCKET_RecvFrom_RSP(UInt32 tid, UInt8 clientID, SocketRecvRsp_t* recvRspData);

//***************************************************************************************
/**
@cond
This function examines the input/output descriptor sets (whose addresses are passed in readfds, writefds, and exceptfds) 
to see if some of their descriptors are ready for reading, ready for writing, or have an exceptional condition pending.
On return, select() replaces the given descriptor sets with subsets consisting of the descriptors that are ready for 
the requested operation. The total number of ready descriptors in all the sets is returned.

You can give the parameters readfds, writefds, and exceptfds as NULL pointers if no descriptors are of interest.  

To determine if a call to accept() will return immediately, call select(),passing the socket as a member of 
the readfds set.

Note:
Under rare circumstances, select() can indicate that a descriptor is ready for writing when, in fact, 
an attempt to write would block. This can happen if system resources necessary for a write are subsequently exhausted 
after the select has returned or are otherwise unavailable. If an application deems it critical that writes to a 
socket descriptor do not block, it should set the descriptor for nonblocking input/output using the SO_NBIO request 
to setsockopt().

The descriptors are stored within the fd_set structures as opaque objects. 
The macros below are provided for manipulating such structures.  

The behavior of these macros is undefined if an invalid descriptor value is passed.

FD_ZERO FD_ZERO(fd_set *fdset)
This macro initializes a descriptor set fdset to the null set.

FD_SET FD_SET(long fd, fd_set *fdset)
This macro includes a particular socket descriptor fd in fdset.

FD_CLR FD_CLR(long fd, fd_set *fdset)
This macro removes fd from fdset.

FD_ISSET FD_ISSET(long fd,fd_set*fdset)
This macro is nonzero if fd is a member of fdset, and zero otherwise.

int t_select(fd_set *readfds, fd_set *writefds, fd_set *exceptfds, long timeout)

	@param tid		(in) Unique exchange/transaction id which is passed in the request
	@param clientID	(in) Client ID
	@param a	(in)	Is the set of socket descriptors to be tested for available data to be read.
	@param b	(in)	Is the set of socket descriptors to be tested for available buffer space for write operations.
	@param c	(in)	Is the set of socket descriptors to be tested for pending exceptional conditions (if out-of-band data is 
						available to be read). 
	@param d	(in)	Is the wait interval in cticks clock ticks. If timeout is neither 0 nor -1, it specifies the maximum number of 
						clock ticks (at TPS ticks per second) to wait for the selection to complete. If timeout is zero, t_select()
						modifies the descriptor sets to indicate which are ready for the requested operation, and returns immediately. 
						If timeout is -1, t_select() blocks until at least one of the requested operations is ready, and there is no timeout.
			

	@note
	The async response is as follows:
	@n@b Responses 
	@n@b MsgType_t :	::MSG_SOCKET_SELECT_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	Returns one of the following:
						+value A positive value indicates the number of ready descriptors in the descriptor sets.
						0 Indicates that the time limit referred to by timeout has expired.
						-1 If not successful.
@endcond
**/
//void CAPI2_SOCKET_Select(UInt32 tid, UInt8 clientID, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, long timeout);

//***************************************************************************************
/**
This function deletes a descriptor from the reference table. On the close of a socket, associated naming information 
and queued data are discarded.

This is just close() on traditional sockets systems.

int t_socketclose(long socket)

  	@param tid		(in) Unique exchange/transaction id which is passed in the request
	@param clientID	(in) Client ID
	@param descriptor	(in)	Is the identifier of the socket to be closed.

	@note
	The async response is as follows:
	@n@b Responses 
	@n@b MsgType_t :	::MSG_SOCKET_CLOSE_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	Returns one of the following:
						0 If successful.
						-1 If not successful. The internal socket variable errno
						is set to one of the errors listed in ipport.h. 
						The value of errno can be retrieved by a call to t_errno(socket).
**/
void CAPI2_SOCKET_Close(UInt32 tid, UInt8 clientID, Int32 descriptor);

//***************************************************************************************
/**
	Function response for CAPI2_SOCKET_Close.
	@param		tid (in) Unique exchange/transaction id which is passed in the request
	@param		clientID (in) Client ID
	@param		status (in) The response value. 
			0 If successful.
			-1 If not successful. The internal socket variable errno is set to one of the errors listed in ipport.h. 
				The value of errno can be retrieved by a call to t_errno(socket).
	@return		Not Applicable
	@note
	Return or Response is not applicable
	@n@b Response to CP will be notified via ::MSG_SOCKET_CLOSE_RSP
**/
void CAPI2_SOCKET_Close_RSP(UInt32 tid, UInt8 clientID, Int8 status);

/**
This function causes all or part of a full duplex connection on the socket associated with socket to be shutdown.

int t_shutdown(long socket, int how)

	@param tid		(in) Unique exchange/transaction id which is passed in the request
	@param clientID	(in) Client ID
	@param descriptor	(in)	Is the connection to be shut down.
	@param how	(in)	Is the method of shut down, specified as follows:
							0 Further receives are disallowed
							1 Further sends are disallowed
							2 Further sends and receives are disallowed.

	@note
	The async response is as follows:
	@n@b Responses 
	@n@b MsgType_t :	::MSG_SOCKET_SHUTDOWN_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	Returns one of the following:
						0 If successful.
						-1 If not successful. The internal socket variable errno
						is set to one of the errors listed in ipport.h. 
						The value of errno can be retrieved by a call to t_errno(socket).
**/
void CAPI2_SOCKET_Shutdown(UInt32 tid, UInt8 clientID, Int32 descriptor, UInt8 how);

//***************************************************************************************
/**
	Function response for CAPI2_SOCKET_Shutdown.
	@param		tid (in) Unique exchange/transaction id which is passed in the request
	@param		clientID (in) Client ID
	@param		status (in) The response value. 
					0 If successful.
					-1 If not successful. The internal socket variable errno is set
						to one of the errors listed in ipport.h. 
						The value of errno can be retrieved by a call to t_errno(socket).
	@return		Not Applicable
	@note
	Return or Response is not applicable
	@n@b Response to CP will be notified via ::MSG_SOCKET_SHUTDOWN_RSP
**/
void CAPI2_SOCKET_Shutdown_RSP(UInt32 tid, UInt8 clientID, Int8 status);

//***************************************************************************************
/**
This function is used to retrieve the current value of the error flag associated with a socket. 
The error status is not reset by this call.

int t_errno (long s)

	@param tid		(in) Unique exchange/transaction id which is passed in the request
	@param clientID	(in) Client ID
	@param descriptor	(in)	Is a socket descriptor.

	@note
	The async response is as follows:
	@n@b Responses 
	@n@b MsgType_t :	::MSG_SOCKET_ERRNO_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	The current error value associated with this socket.
**/
void CAPI2_SOCKET_Errno(UInt32 tid, UInt8 clientID, Int32 descriptor);

//***************************************************************************************
/**
	Function response for CAPI2_SOCKET_Errno.
	@param		tid (in) Unique exchange/transaction id which is passed in the request
	@param		clientID (in) Client ID
	@param		error (in) The current error value associated with this socket.
	@return		Not Applicable
	@note
	Return or Response is not applicable
	@n@b Response to CP will be notified via ::MSG_SOCKET_ERRNO_RSP
**/
void CAPI2_SOCKET_Errno_RSP(UInt32 tid, UInt8 clientID, Int32 error);

//***************************************************************************************
/**
*	Converts between 32-bit long socket descriptors and the actual
*	socket structure pointers. On most systems they just cast the values. They 
*	are provided to support 48 bit "large model" 386/pentium builds, which 
*	should define these macros in ipport.h 
*
long SO2LONG (struct socket* so)

	@param tid		(in) Unique exchange/transaction id which is passed in the request
	@param clientID	(in) Client ID
	@param socket	(in)	Address of a socket structure.

	@note
	The async response is as follows:
	@n@b Responses 
	@n@b MsgType_t :	::MSG_SOCKET_SO2LONG_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	The socket descriptor.
**/
void CAPI2_SOCKET_SO2LONG(UInt32 tid, UInt8 clientID, Int32 socket);

//***************************************************************************************
/**
	Function response for CAPI2_SOCKET_SO2LONG.
	@param		tid (in) Unique exchange/transaction id which is passed in the request
	@param		clientID (in) Client ID
	@param		descriptor (in) The socket descriptor.
	@return		Not Applicable
	@note
	Return or Response is not applicable
	@n@b Response to CP will be notified via ::MSG_SOCKET_SO2LONG_RSP
**/
void CAPI2_SOCKET_SO2LONG_RSP(UInt32 tid, UInt8 clientID, Int32 descriptor);

//***************************************************************************************
/**
This function gets available socket send buffer space.  
It returns send buffer space if available, otherwise return ENP_PARM (-10) error.

int getSocketSendBufferSpace(long sd)

 	@param tid		(in) Unique exchange/transaction id which is passed in the request
	@param clientID	(in) Client ID
	@param descriptor (in)	Is a socket descriptor. 

	@note
	The async response is as follows:
	@n@b Responses 
	@n@b MsgType_t :	::MSG_SOCKET_SO2LONG_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	The send buffer space if available, otherwise returns
						ENP_PARM (-10) error.
**/
void CAPI2_SOCKET_GetSocketSendBufferSpace(UInt32 tid, UInt8 clientID, Int32 descriptor);

//***************************************************************************************
/**
	Function response for CAPI2_SOCKET_GetSocketSendBufferSpace.
	@param		tid (in) Unique exchange/transaction id which is passed in the request
	@param		clientID (in) Client ID
	@param		bufferSpace (in) The buffer space available for a send operation.
	@return		Not Applicable
	@note
	Return or Response is not applicable
	@n@b Response to CP will be notified via ::MSG_SOCKET_GET_SOCKET_SEND_BUFFER_SPACE_RSP
**/
void CAPI2_SOCKET_GetSocketSendBufferSpace_RSP(UInt32 tid, UInt8 clientID, Int32 bufferSpace);

//***************************************************************************************
/**
This function parses an IP address string and converts it into an ip_addr type.

char * parse_ipad(ip_addr * ipout,   // pointer to IP address to set
				  unsigned * sbits,  // default subnet bit number
				  char * stringin);  // buffer with ascii to parse

 	@param tid		(in) Unique exchange/transaction id which is passed in the request
	@param clientID	(in) Client ID
	@param inIPAddressStr (in)	The IP address in string format (e.g., "255.255.255.255"). 

	@note
	The async response is as follows:
	@n@b Responses 
	@n@b MsgType_t :	::MSG_SOCKET_PARSE_IPAD_RSP
	@n@b Result_t :		::RESULT_OK, ::RESULT_ERROR
	@n@b ResultData :	The corresponding ip_addr, subnet bits, and error string.
**/
void CAPI2_SOCKET_ParseIPAddr(UInt32 tid, UInt8 clientID, char* inIPAddressStr);

//***************************************************************************************
/**
	Function response for CAPI2_SOCKET_ParseIPAddr.
	@param		tid (in) Unique exchange/transaction id which is passed in the request
	@param		clientID (in) Client ID
	@param		ipAddress (in)	The equivalent IP Address.
	@param		subnetBits (in)	The default subnet bit number.
	@param		errorStr (in)	A null-terminated error string, or NULL if no error occurred.
	@return		Not Applicable
	@note
	Return or Response is not applicable
	@n@b Response to CP will be notified via ::MSG_SOCKET_PARSE_IPAD_RSP
**/
void CAPI2_SOCKET_ParseIPAddr_RSP(UInt32 tid, UInt8 clientID, ip_addr* ipAddress, unsigned subnetBits, char* errorStr);

//***************************************************************************************
/**
	This function is an "unsolicited" message from the AP to the CP, indicating that
	the socket signal callback (setup via CAPI2_SetSockOpt(..., SO_SIGNAL)) has been
	called.

	@param		tid (in) Unique exchange/transaction id which is passed in the request
	@param		clientID (in) Client ID
	@param		sockSignalInd (in) The socket signal indication callback parameters,
									which includes the CP callback function pointer.
**/
void CAPI2_SOCKET_SignalInd(UInt32 tid, UInt8 clientID, SocketSignalInd_t *sockSignalInd);


/** @} */


#ifdef __cplusplus
}
#endif

#endif	//CAPI2_CP_SOCKET_H

