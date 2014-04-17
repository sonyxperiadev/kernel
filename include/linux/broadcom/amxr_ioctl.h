/*****************************************************************************
* Copyright 2003 - 2009 Broadcom Corporation.  All rights reserved.
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
*  @file    amxr_ioctl.h
*
*  @brief   Audio Mixer User IOCTL API definitions
*
*****************************************************************************/
#if !defined( AMXR_IOCTL_H )
#define AMXR_IOCTL_H

/* ---- Include Files ---------------------------------------------------- */

#if defined( __KERNEL__ )
#include <linux/types.h>            /* Needed for standard types */
#else
#include <stdint.h>
#endif

#include <linux/ioctl.h>
#include <linux/broadcom/amxr.h>

/* ---- Constants and Types ---------------------------------------------- */

/* Type define used to create unique IOCTL number */
#define AMXR_MAGIC_TYPE              'M'

/* IOCTL commands */
enum amxr_cmd_e
{
   AMXR_CMD_QUERY_PORT = 0x30,      /* Start at 0x30 arbitrarily */
   AMXR_CMD_GET_PORT_INFO,
   AMXR_CMD_GET_INFO,
   AMXR_CMD_SET_CNX_LOSS,
   AMXR_CMD_GET_CNX_LOSS,
   AMXR_CMD_CONNECT,
   AMXR_CMD_DISCONNECT,
   AMXR_CMD_GET_CNXLIST_BY_SRC,
   AMXR_CMD_GET_CNXLIST_BY_DST,
   AMXR_CMD_LAST                    /* Do no delete */
};

/* IOCTL Data structures */
struct amxr_ioctl_queryport
{
   char           name[32];         /*<< (i) User name to query with */
   AMXR_PORT_ID  *id;               /*<< (o) Ptr to store found port ID */
};

struct amxr_ioctl_queryportinfo
{
   AMXR_PORT_ID    port;            /*<< (i) Port id */
   AMXR_PORT_INFO *info;            /*<< (o) Ptr to port info structure */
};

struct amxr_ioctl_setcnxloss
{
   AMXR_PORT_ID         src_port;   /*<< (i) source port id */
   AMXR_PORT_ID         dst_port;   /*<< (i) destination port id */
   AMXR_CONNECT_DESC    desc;       /*<< (i) Connection descriptor */
   unsigned int         db;         /*<< (i) Loss amount in dB */
};

struct amxr_ioctl_getcnxloss
{
   AMXR_PORT_ID         src_port;   /*<< (i) source port id */
   AMXR_PORT_ID         dst_port;   /*<< (i) destination port id */
   AMXR_CONNECT_DESC    desc;       /*<< (i) Connection descriptor */
   unsigned int        *db;         /*<< (o) Loss amount in dB */
};

struct amxr_ioctl_connect
{
   AMXR_PORT_ID       src_port;     /*<< (i) source port id */
   AMXR_PORT_ID       dst_port;     /*<< (i) destination port id */
   AMXR_CONNECT_DESC  desc;         /*<< (i) Connection descriptor */
};

struct amxr_ioctl_disconnect
{
   AMXR_PORT_ID       src_port;     /*<< (i) source port id */
   AMXR_PORT_ID       dst_port;     /*<< (i) destination port id */
};

struct amxr_ioctl_get_cnxlist
{
   AMXR_PORT_ID         port;       /*<< (i) port id */
   AMXR_CNXS           *cnxlist;    /*<< (o) ptr to store cnx list */
   int                  maxlen;     /*<< (i) max length in bytes */
};

/* IOCTL numbers */
#define AMXR_IOCTL_QUERY_PORT    _IOR( AMXR_MAGIC_TYPE, AMXR_CMD_QUERY_PORT, struct amxr_ioctl_queryport )
#define AMXR_IOCTL_GET_PORT_INFO _IOR( AMXR_MAGIC_TYPE, AMXR_CMD_GET_PORT_INFO, struct amxr_ioctl_queryportinfo )
#define AMXR_IOCTL_GET_INFO      _IO( AMXR_MAGIC_TYPE, AMXR_CMD_GET_INFO )
#define AMXR_IOCTL_SET_CNX_LOSS  _IOR( AMXR_MAGIC_TYPE, AMXR_CMD_SET_CNX_LOSS, struct amxr_ioctl_setcnxloss )
#define AMXR_IOCTL_GET_CNX_LOSS  _IOR( AMXR_MAGIC_TYPE, AMXR_CMD_GET_CNX_LOSS, struct amxr_ioctl_getcnxloss )
#define AMXR_IOCTL_CONNECT       _IOR( AMXR_MAGIC_TYPE, AMXR_CMD_CONNECT, struct amxr_ioctl_connect )
#define AMXR_IOCTL_DISCONNECT    _IOR( AMXR_MAGIC_TYPE, AMXR_CMD_DISCONNECT, struct amxr_ioctl_disconnect )
#define AMXR_IOCTL_GET_CNXLIST_BY_SRC _IOR( AMXR_MAGIC_TYPE, AMXR_CMD_GET_CNXLIST_BY_SRC, struct amxr_ioctl_get_cnxlist )
#define AMXR_IOCTL_GET_CNXLIST_BY_DST _IOR( AMXR_MAGIC_TYPE, AMXR_CMD_GET_CNXLIST_BY_DST, struct amxr_ioctl_get_cnxlist )

/* ---- Variable Externs ------------------------------------------ */

/* ---- Function Prototypes --------------------------------------- */

#endif /* AMXR_IOCTL_H */
