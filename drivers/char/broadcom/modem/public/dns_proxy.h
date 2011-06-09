//*********************************************************************
//
//      Copyright © 2005-2008 Broadcom Corporation
//
//      This program is the proprietary software of Broadcom Corporation
//      and/or its licensors, and may only be used, duplicated, modified
//      or distributed pursuant to the terms and conditions of a separate,
//      written license agreement executed between you and Broadcom (an
//      "Authorized License").  Except as set forth in an Authorized
//      License, Broadcom grants no license (express or implied), right
//      to use, or waiver of any kind with respect to the Software, and
//      Broadcom expressly reserves all rights in and to the Software and
//      all intellectual property rights therein.  IF YOU HAVE NO
//      AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE
//      IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE
//      ALL USE OF THE SOFTWARE.
//
//      Except as expressly set forth in the Authorized License,
//
//      1.      This program, including its structure, sequence and
//              organization, constitutes the valuable trade secrets
//              of Broadcom, and you shall use all reasonable efforts
//              to protect the confidentiality thereof, and to use
//              this information only in connection with your use
//              of Broadcom integrated circuit products.
//
//      2.      TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE
//              IS PROVIDED "AS IS" AND WITH ALL FAULTS AND BROADCOM
//              MAKES NO PROMISES, REPRESENTATIONS OR WARRANTIES,
//              EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE,
//              WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
//              DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE,
//              MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
//              PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
//              COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
//              CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE
//              RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
//
//      3.      TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT
//              SHALL BROADCOM OR ITS LICENSORS BE LIABLE FOR
//              (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
//              EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
//              WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE
//              SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF THE
//              POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
//              EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE
//              ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
//              LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE
//              OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
//
//***************************************************************************
/**
*
*  @file   dns_proxy.h
*
*  @brief  Defines the interface used for IPC between the dns proxy and the client.
*
*
****************************************************************************/
#ifndef DNS_PROXY_H__
#define DNS_PROXY_H__

#ifdef __cplusplus
extern "C" {
#endif

// name of local socket used for IPC
#define DNS_PROXY_SOCKET_NAME "/data/misc/dns_proxy_sckt_name"

// maximum buffer length either way
#define DNS_PROXY_SCKT_BUFFER_MAX_LEN 200

// request supported by dns proxy
typedef enum
{
     DNS_PROXY_SEND_CMD
} DnsProxy_Request_Type_t;

// request structure
// data follows this structure 
typedef struct
{
    DnsProxy_Request_Type_t requestType;
    size_t dataLen;         // length of data
} DnsProxy_Request_t;

//response status code from dns proxy
typedef enum
{
    DNS_PROXY_SENDCMD_OK,
    DNS_PROXY_SENDCMD_FAILED
} DnsProxy_Response_status_t;

// response structure
typedef struct 
{
    DnsProxy_Response_status_t status;
} DnsProxy_Response_t;


#ifdef __cplusplus
}
#endif

#endif //DNS_PROXY_H__
