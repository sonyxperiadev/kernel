/****************************************************************************
*
*     Copyright (c) 2007-2008 Broadcom Corporation
*
*   Unless you and Broadcom execute a separate written software license 
*   agreement governing use of this software, this software is licensed to you 
*   under the terms of the GNU General Public License version 2, available 
*    at http://www.gnu.org/licenses/old-licenses/gpl-2.0.html (the "GPL"). 
*
*   Notwithstanding the above, under no circumstances may you combine this 
*   software in any way with any other Broadcom software provided under a license 
*   other than the GPL, without Broadcom's express prior written consent.
*
****************************************************************************/
//capi2_ipconfig.h

#ifndef __CAPI2_IPCONFIG
#define __CAPI2_IPCONFIG

#include "capi2_pch_api.h"
#include "xdr_porting_layer.h"

/////////////////////////////////////////////////////////////////////////
//Internal Use only

#define IPCPDEBUG(x)	CAPI2_TRACE x

#define MIN_CHALLENGE_LENGTH  32
#define MAX_CHALLENGE_LENGTH  64
#define MAX_RESPONSE_LENGTH   49   /* sufficient for MD5 & MSCHAP */
#define CHAP_HEADERLEN     4
#define CHAP_CHALLENGE     1
#define CHAP_RESPONSE      2
#define MD5_SIGNATURE_SIZE 16   /* 16 bytes in a MD5 message digest */
#define MAXSECRETLEN 32   /* max length of password or secret */

#define  PPP_LCP  0xc021      /* Link Control Protocol */
#define  PPP_IPCP 0x8021      /* IP Control Packet */
#define  PPP_UPAP 0xc023      /* User/Password Authentication Protocol */
#define  PPP_CHAP 0xc223      /* Cryptographic Handshake Protocol */

#define UPAP_AUTHREQ    1       /* Authenticate-Request */
#define CI_DNSADDR_PRI  129   /* Primary DNS Server (RFC 1877) */
#define CI_DNSADDR_SEC  131   /* Secondary DNS Server (RFC 1877) */

#define CONFREQ   1  /* Configuration Request */
#define CONFACK   2  /* Configuration Ack */
#define CONFNAK   3  /* Configuration Nak */
#define CI_ADDR      3   /* his address? */

/////////////////////////////////

#define GETCHAR(c, cp) { \
   (c) = *(cp)++; \
}

#define PUTCHAR(c, cp) { \
   *(u_char*)(cp)++ = (u_char)(c); \
}


#define GETSHORT(s, cp) { \
   (s) = (u_short)(*(cp)++ << 8); \
   (s) |= (u_short)(*(cp)++); \
}
#define PUTSHORT(s, cp) { \
   *(u_char*)(cp)++ = (u_char)((s) >> 8); \
   *(u_char*)(cp)++ = (u_char)((s) & 0x00ff); \
}

#define   GETLONG(l, cp)   {   \
   (l) = (u_long)*(cp)++ << 8; \
   (l) |= *(cp)++; (l) <<= 8; \
   (l) |= *(cp)++; (l) <<= 8; \
   (l) |= *(cp)++; \
}

#define PUTLONG(l, cp) { \
   *(cp)++ = (u_char)((l) >> 24L); \
   *(cp)++ = (u_char)((l) >> 16L); \
   *(cp)++ = (u_char)((l) >> 8L); \
   *(cp)++ = (u_char)(l); \
}


#define INCPTR(n, cp)   ((cp) += (n))
#define DECPTR(n, cp)   ((cp) -= (n))

typedef struct 
{
    UInt8 flag;
	UInt8 len;
	UInt8 content[(MAX_CHALLENGE_LENGTH+MAXSECRETLEN+CHAP_HEADERLEN+1)];
								/* max length for challenge and secret name + 1 byte for length */
	                           	/* + 4 bytes for code, id, length                               */
} CHAP_ChallengeOptions_t;

typedef struct 
{
    UInt8 flag;      			/* Shall we send this option to the network? 1=yes, 0=no */
	UInt8 len;
	UInt8 content[(MAX_CHALLENGE_LENGTH+MAXSECRETLEN+CHAP_HEADERLEN+1)];	
} CHAP_ResponseOptions_t;

////////////////////// PAP //////////////////////////////////

#define MAX_USERLENGTH 32
#define UPAP_HEADERLEN  4

typedef struct {
    u_char flag;      			/* Shall we send this option to the network? 1=yes, 0=no */
	u_char len;
	u_char content[(2*MAX_USERLENGTH)+UPAP_HEADERLEN+2];
								/* max length for username and password + 2 bytes for length */
	                           	/* + 4 bytes for code, id, length                            */
} PAP_CnfgOptions_t;

#endif
