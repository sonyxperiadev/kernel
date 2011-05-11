//*********************************************************************
//
//	Copyright © 2008 Broadcom Corporation
//	
//	This program is the proprietary software of Broadcom Corporation 
//	and/or its licensors, and may only be used, duplicated, modified 
//	or distributed pursuant to the terms and conditions of a separate, 
//	written license agreement executed between you and Broadcom (an 
//	"Authorized License").  Except as set forth in an Authorized 
//	License, Broadcom grants no license (express or implied), right 
//	to use, or waiver of any kind with respect to the Software, and 
//	Broadcom expressly reserves all rights in and to the Software and 
//	all intellectual property rights therein.  IF YOU HAVE NO 
//	AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE 
//	IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE 
//	ALL USE OF THE SOFTWARE.  
//	
//	Except as expressly set forth in the Authorized License,
//	
//	1.	This program, including its structure, sequence and 
//		organization, constitutes the valuable trade secrets 
//		of Broadcom, and you shall use all reasonable efforts 
//		to protect the confidentiality thereof, and to use 
//		this information only in connection with your use 
//		of Broadcom integrated circuit products.
//	
//	2.	TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE 
//		IS PROVIDED "AS IS" AND WITH ALL FAULTS AND BROADCOM 
//		MAKES NO PROMISES, REPRESENTATIONS OR WARRANTIES, 
//		EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, 
//		WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY 
//		DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, 
//		MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A 
//		PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR 
//		COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR 
//		CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE 
//		RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.  
//
//	3.	TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT 
//		SHALL BROADCOM OR ITS LICENSORS BE LIABLE FOR 
//		(i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR 
//		EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY 
//		WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE 
//		SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF THE 
//		POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN 
//		EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE 
//		ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE 
//		LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE 
//		OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
//
//***************************************************************************
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

/**
 * @addtogroup CAPI2_CP_APIGroup
 * @{
 */

#ifndef _SOCKCALL_H		// avoid duplicate defines with sockcall.h

	#define SYS_SOCKETNULL -1  /* error return from sys_socket. */
	#define INVALID_SOCKET -1  /* WINsock-ish synonym for SYS_SOCKETNULL */
	#define SOCKET_ERROR   -1  /* error return from send(), sendto(), et.al. */

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

/**
 * Structure used by kernel to store most
 * addresses.
 */
typedef struct
{
   u_short   sa_family;      /* address family */
   char   sa_data[14];      /* up to 14 bytes of direct address */
} sockaddr;

/*
 * Structure used for manipulating linger option.
 */
typedef struct
{
   u_short   l_onoff;      /* option on/off */
   u_short   l_linger;      /* linger time */
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
#define   SOCK_STREAM   1      /* stream socket */
#define   SOCK_DGRAM    2      /* datagram socket */
#ifndef NPPORT
#define   SOCK_RAW      3      /* raw-protocol interface */
#define   SOCK_RDM      4      /* reliably-delivered message */
#define   SOCK_SEQPACKET 5     /* sequenced packet stream */
#endif /* NPPORT*/

/*
 * Option flags per-socket.
 */

#define   SO_DEBUG       0x0001      /* turn on debugging info recording */
#define   SO_ACCEPTCONN  0x0002      /* socket has had listen()          */
#define   SO_REUSEADDR   0x0004      /* allow local address reuse        */
#define   SO_KEEPALIVE   0x0008      /* keep connections alive           */
#define   SO_DONTROUTE   0x0010      /* just use interface addresses     */
#define   SO_BROADCAST   0x0020      /* permit sending of broadcast msgs */
#define   SO_USELOOPBACK 0x0040      /* bypass hardware when possible    */
#define   SO_LINGER      0x0080      /* linger on close if data present  */
#define   SO_OOBINLINE   0x0100      /* leave received OOB data in line  */
#ifdef TCP_ZEROCOPY
// SO_ZEROCOPY is only present if the stack has been built with the TCP_ZEROCOPY package option defined in ipport.h.
#define   SO_ZEROCOPY    0x0200      /* socket option to enable TCP/UDP ZEROCOPY  */
#endif

/*
 * Additional options, not kept in so_options.
 */

#define SO_SNDBUF    0x1001      /* send buffer size */
#define SO_RCVBUF    0x1002      /* receive buffer size */
#define SO_SNDLOWAT  0x1003      /* send low-water mark */
#define SO_RCVLOWAT  0x1004      /* receive low-water mark */
#define SO_SNDTIMEO  0x1005      /* send timeout */
#define SO_RCVTIMEO  0x1006      /* receive timeout */
#define SO_ERROR     0x1007      /* get error status and clear */
#define SO_TYPE      0x1008      /* get socket type */
#define SO_HOPCNT    0x1009      /* Hop count to get to dst   */
#define SO_MAXMSG    0x1010      /* get TCP_MSS (max segment size) */

/* ...And some netport additions to setsockopt: */
#define SO_RXDATA    0x1011      /* get count of bytes in sb_rcv */
#define SO_MYADDR    0x1012      /* return my IP address */
#define SO_NBIO      0x1013      /* set socket into NON-blocking mode */
#define SO_BIO       0x1014      /* set socket into blocking mode */
#define SO_NONBLOCK  0x1015      /* set/get blocking mode via optval param */

/* Broadcom ZEROCOPY implementation allows TCP/UDP zerocopy to share    */
/* the same callback as registered for SO_SIGNAL, i.e. so_signal(). As  */
/* such, the caller does NOT need to register a different callback      */
/* through setsockopt() using SO_CALLBACK, instead, it is required to   */
/* call setsockopt() with optname SO_ZEROCOPY and optval 1 for the      */
/* socket enabling this feature.                                        */
#define SO_CALLBACK  0x1016      /* set/get zero_copy callback routine  */

#define SO_SIGNAL	 0x1017		 /* set the signal notify callback routine */
#define SO_CONTEXT	 0x1018		 /* reserved to store acctId for multiple pdp context support */
#define SO_CONTEXT2	 0x1019		 /* set the application specific context */

/*
 * Address families.
 */

#define  AF_UNSPEC   0     /* unspecified */
#define  AF_NS       1     /* local to host (pipes, portals) */
#define  AF_INET     2     /* internetwork: UDP, TCP, etc. */
#if 0
#define  AF_IMPLINK  3     /* arpanet imp addresses */
#define  AF_PUP      4     /* pup protocols: e.g. BSP */
#define  AF_CHAOS    5     /* mit CHAOS protocols */
#define  AF_NS       6     /* XEROX NS protocols */
#define  AF_NBS      7     /* nbs protocols */
#define  AF_ECMA     8     /* european computer manufacturers */
#define  AF_DATAKIT  9     /* datakit protocols */
#define  AF_CCITT    10    /* CCITT protocols, X.25 etc */
#define  AF_SNA      11    /* IBM SNA */
#define  AF_DECnet   12    /* DECnet */
#define  AF_DLI      13    /* Direct data link interface */
#define  AF_LAT      14    /* LAT */
#define  AF_HYLINK   15    /* NSC Hyperchannel */
#define  AF_APPLETALK 16   /* Apple Talk */
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

#define   MSG_OOB      0x1      /* process out-of-band data */
#define   MSG_PEEK   0x2      /* peek at incoming message */
#define   MSG_DONTROUTE   0x4      /* send without using routing tables */
#define   MSG_NEWPIPE   0x8      /* New pipe for recvfrom call   */
#define   MSG_EOR      0x10      /* data completes record */
#define   MSG_DONTWAIT   0x20      /* this message should be nonblocking */

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
*	The payload for @a   MSG_CAPI2_SOCKET_SIGNAL_IND.  This message is
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
	///	-   MSG_OOB: Sends out-of-band data.
	///		Only SOCK_STREAM sockets support out-of-band data.
	///	-   MSG_DONTROUTE: The SO_DONTROUTE option is turned on
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
	///	-   MSG_OOB Reads any out-of-band data present on the socket,
	///		rather than the regular in-band data.
	///	-   MSG_PEEK Looks at the data present on the socket.
	///		The data is returned, but not consumed, so a subsequent
	///		receive operation will see the same data.
	UInt16 flags;

} SocketRecvReq_t;

/**
*	Structure to pass the recv response data.
*/
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

/* [oy - function commented out]
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
**/
//void CAPI2_SOCKET_Select(UInt32 tid, UInt8 clientID, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, long timeout);

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

