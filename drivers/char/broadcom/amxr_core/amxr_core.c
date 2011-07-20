/*****************************************************************************
* Copyright 2006 - 2009 Broadcom Corporation.  All rights reserved.
*
* This program is the proprietary software of Broadcom Corporation and/or
* its licensors, and may only be used, duplicated, modified or distributed
* pursuant to the terms and conditions of a separate, written license
* agreement executed between you and Broadcom (an "Authorized License").
* Except as set forth in an Authorized License, Broadcom grants no license
* (express or implied), right to use, or waiver of any kind with respect to
* the Software, and Broadcom expressly reserves all rights in and to the
* Software and all intellectual property rights therein.  IF YOU HAVE NO
* AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
* WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
* THE SOFTWARE.
*
* Except as expressly set forth in the Authorized License,
* 1. This program, including its structure, sequence and organization,
*    constitutes the valuable trade secrets of Broadcom, and you shall use
*    all reasonable efforts to protect the confidentiality thereof, and to
*    use this information only in connection with your use of Broadcom
*    integrated circuit products.
* 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
*    RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
*    IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS
*    FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS,
*    QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU
*    ASSUME THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
* 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
*    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
*    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
*    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
*****************************************************************************/


/**
*
*  @file    amxr_core.c
*
*  @brief   This file implements the Audio Mixer core framework.
*
*****************************************************************************/

/* ---- Include Files ---------------------------------------------------- */
#include <linux/module.h>
#include <linux/init.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/mm.h>

#include <linux/broadcom/knllog.h>           /* For debugging */
#include <linux/broadcom/amxr.h>             /* Audio mixer API */
#include <linux/errno.h>         	     /* Needed for -EINVAL */

#include "amxr_resamp.h"                     /* Resampler definitions */
#include "amxr_vectadd.h"                    /* Vector add operations */
#include "amxr_vectmul.h"                    /* Vector multiply operations */
#include "llist.h"                           /* Link list definitions */

/* ---- Public Variables ------------------------------------------------- */
/* ---- Private Constants and Types -------------------------------------- */

/* Defines the default connection list size and how much it can grow
 * by each time the list is resized 
 */
#define AMXR_CNXLIST_SIZE_INCREMENT        3

/* Scratch buffer size used for resampling, interleave, deinterleave. 
 * The size should at least accommodate 48khz stereo, which is 
 * 48 * 5 * 2 * 2 bytes = 960 bytes. But since the Linux page size is 
 * 4kb, might as well make the buffer close to 4kb. 
 */
#define AMXR_SCRATCHBUF_BYTES              4000

/* Defines the default clock rate that the audio mixer is executed
 * in usec. The buffer size used in source and destination ports
 * are calculated based on this rate.
 */
#define AMXR_PERIOD_USEC                   5000

/* Defines the sample width in bytes */
#define AMXR_SAMPLE_WIDTH_BYTES            2

/* Initial value for first destination flag. This flag is used as a bit field
 * for multi-channel ports with each bit corresponding to each sub-channel.
 * Thus the initial value is 0xffffffff
 */
#define AMXR_DST_FIRST_INITVAL            (uint32_t)-1

/* Only sampling frequencies which are divisible by eight are serviced
 * by the primary AMXR routine (amxrElapsedTime). Other rates are
 * serviced directly by selected ports.
 */
#define AMXR_SYNC_FREQ( freq )            (((freq) % 8 ) == 0)

#define AMXR_MEM_TYPE GFP_KERNEL

/**
*  Mixer port node structure
*/
struct amxr_port_node
{
   LLIST_NODE                 lnode;         /* List node - must be first */
   AMXR_PORT_INFO             info;          /* Port information */
   AMXR_PORT_CB               cb;            /* User callbacks */
   void                      *privdata;      /* User supplied data for callbacks */
   int                        dst_cnxs;      /* Number of connections as destination */
   int                        src_cnxs;      /* Number of connections as source */
   int                        src_cnx_count; /* Source connection counter used by amxrCoreServiceUnsyncPort ISR for tracking */
   int16_t                   *srcp;          /* Cached source ptr used in ISR */
   int16_t                   *dstp;          /* Cached destination ptr used in ISR */
   uint32_t                   dst_first;     /* Flag to indicate the port is waiting for its first dst frame. For multi-chan ports, used as bit fields */
};

/**
*  Mixer port head structure
*/
struct amxr_port_head
{
   struct mutex               mutex;         /* Protect list */
   LLIST_NODE                 list;          /* List head */
   int                        total;         /* Total number of ports */
};

/**
*  Destination list element used in connection lists
*/
struct amxr_cnxdstl_t
{
   struct amxr_port_node     *portp;         /* Destination port */
   uint16_t                   lossdb;        /* Connection loss value in dB */
   uint16_t                   losslin;       /* Connection loss linear value */
   int                        dst_chans;     /* Number of sub-channels */
   int                        dst_idx;       /* Sub-channel index to output data to */
};

/**
*  Mixer connection list node structure 
*
*  Connections lists are organized by source ports. Each connection list 
*  contains one source port and at least one destination port.
*/
struct amxr_cnxlist_node
{
   LLIST_NODE                 lnode;         /* List head - must be first */
   struct amxr_port_node     *srcportp;      /* Source port */
   struct amxr_cnxdstl_t     *dstlp;         /* Ptr to destination port list */
   int                        cnxs;          /* Number of connected dst ports */
   int                        max_cnxs;      /* Max number of connections, used for realloc of port list */   
   AMXR_CONNECT_TYPE          cnx_type;      /* Connection type */
   int                        src_chans;     /* Source channels */
   int                        src_idx;       /* Source sub-channel index */
   AMXR_RESAMPLE_TABENTRY    *resamptblp;    /* Pointer to resampler info. */
   void                      *resampbufp;    /* Pointer to resampler data buffer */
};

/**
*  Mixer connection head structure 
*/
struct amxr_cnxlist_head
{
   struct mutex               mutex;         /* Protect cnx list and connection related data */
   LLIST_NODE                 list;          /* List head for active connections */
   LLIST_NODE                 disabled_list; /* List head for disabled connections */
   int                        total;         /* Total number of connections */
   int                        disabled_total;/* Total number of disabled connections */
};

/**
*  Client state list node
*/
struct amxr_client_node
{
   LLIST_NODE                 lnode;         /* List node - must be first */
};

/* Procfs file name */
#define AMXR_PROC_NAME           "amxr"

/* knllog traces used for debugging */
#define AMXR_TRACE_ENABLED       0
#if AMXR_TRACE_ENABLED
#define AMXR_TRACE               KNLLOG
#else
#define AMXR_TRACE(c...)
#endif

/* Console log */
#define AMXR_LOG( fmt, args... ) KNLLOG( fmt, ## args )

/* Extra debug logs */
#define AMXR_DEBUG_SUPPORT       0
#if AMXR_DEBUG_SUPPORT
#define AMXR_DEBUG               AMXR_LOG
#else
#define AMXR_DEBUG(c...)
#endif

/* Other constants */
#define AMXR_NO_FREE_MEM         0
#define AMXR_FREE_MEM            1

/* ---- Private Variables ------------------------------------------------ */

/* Mixer ports list */
static struct amxr_port_head      gPorts;

/* Mixer connection lists list */
static struct amxr_cnxlist_head   gCnxs;

/* Mixer math operations */
static AMXR_RESAMP_FNC            gResampFnc;
static AMXR_ADD_FNC               gAddFnc;
static AMXR_MPYQ16_FNC            gMpyCFnc;
static AMXR_MACQ16_FNC            gMpyCAddFnc;

/* Scratch buffer used for resampling, de-interleave & interleave ops */
static int16_t                   *gScratchBufp;


/**
*  dB to 16-bit Q16 multiplier map. Used in applying software gains.
*  65535 is used to approximate 0db. If 0db is requested, 
*  should skip gain operation to save Mhz.
*/
#define AMXR_MAX_Q16_LOSS  63
static const uint16_t q16GainMap[AMXR_MAX_Q16_LOSS + 1] =
{
   65535, 58409, 52057, 46396, 41350, 36854, 32846, 29274,
   26090, 23253, 20724, 18471, 16462, 14672, 13076, 11654,
   10387,  9257,  8250,  7353,  6554,  5841,  5206,  4640,
    4135,  3685,  3285,  2927,  2609,  2325,  2072,  1847,
    1646,  1467,  1308,  1165,  1039,   926,   825,   735,
     655,   584,   521,   464,   414,   369,   328,   293,
     261,   233,   207,   185,   165,   147,   131,   117,
     104,    93,    83,    74,    66,    58,    52,    46
};

/* ---- Private Function Prototypes -------------------------------------- */
static int     amxr_alloc_resampler( struct amxr_cnxlist_node *cnxlp, AMXR_RESAMP_TYPE rtype );
static void    amxr_free_resamp( struct amxr_cnxlist_node *cnxlp );
static int     amxr_add_cnx( struct amxr_port_node *srcportp, struct amxr_port_node *dstportp, AMXR_CONNECT_DESC desc, AMXR_RESAMP_TYPE rtype );
static int     amxr_add_cnx_unsafe( struct amxr_port_node *srcportp, struct amxr_port_node *dstportp, AMXR_CONNECT_DESC desc, AMXR_RESAMP_TYPE rtype, unsigned int lossdb );
static int     amxr_del_cnxl_unsafe( struct amxr_cnxlist_node *cnxlp, int free );
static int     amxr_del_dst_from_cnxl_unsafe( struct amxr_cnxlist_node *cnxlp, struct amxr_port_node *dstportp );
static void    amxr_free_cnxl_lnode( struct amxr_cnxlist_node *cnxlp ); 
static void    amxr_dec_dst_cnxcount_unsafe( struct amxr_port_node *dstportp );
static int     amxr_save_disabled_cnx_unsafe( struct amxr_port_node *srcportp, struct amxr_cnxdstl_t *dstl_itemp, AMXR_CONNECT_DESC desc );
static void    amxr_del_dstport_unsafe( struct amxr_cnxlist_node *cnxlp, int i, LLIST_NODE *listp, int *count, int free );
static void    amxr_info_from_cnxl( AMXR_PORT_CNXINFO **cnxinfopp, struct amxr_cnxlist_node *cnxlp, struct amxr_port_node *dstportp, int active );
static int     amxr_set_cnxloss( struct amxr_cnxlist_node *cnxlp, struct amxr_port_node *srcportp, struct amxr_port_node *dstportp, AMXR_CONNECT_DESC desc, unsigned int db );
static int     amxr_get_cnxloss( struct amxr_cnxlist_node *cnxlp, struct amxr_port_node *srcportp, struct amxr_port_node *dstportp, AMXR_CONNECT_DESC desc, unsigned int *db );

/* ---- Functions -------------------------------------------------------- */

/***************************************************************************/
/**
*  Translates a port ID to get a pointer to port node.
*
*  @return NULL on error, otherwise valid ptr.
*/
static inline struct amxr_port_node *getPort(
   AMXR_PORT_ID id                /*<< (i) Port ID */
)
{
   struct amxr_port_node *portp, *foundportp;
   
   /* Scan list to validate port ID is valid */

   foundportp = NULL;
   mutex_lock( &gPorts.mutex );
   
   llist_foreach_item( portp, &gPorts.list, lnode )
   {
      if ( portp == id )
      {
         foundportp = portp;
         break;
      }
   }
   mutex_unlock( &gPorts.mutex );
   return foundportp;
}

/***************************************************************************/
/**
*  Translates a port ID to get a pointer to port node without validation.
*/
static inline struct amxr_port_node *getPort_nocheck(
   AMXR_PORT_ID id                /*<< (i) Port ID */
)
{
   return id;
}

/***************************************************************************/
/**
*  Translates a port pointer to port ID
*/
static inline AMXR_PORT_ID getId(
   struct amxr_port_node *portp   /*<< (i) Port pointer */
)
{
   return portp;
}

/***************************************************************************/
/**
*  Helper routine to scan destination list for a matching port
*  
*  @return  1 if found otherwise 0
*/
static inline int amxr_find_dstport(
   const struct amxr_port_node    *portp,    /**< Port to look for */
   const struct amxr_cnxlist_node *cnxlp,    /**< Connection list */
   int                            *idxp      /**< (o) found index */
)
{
   struct amxr_cnxdstl_t *dstlp = cnxlp->dstlp;
   int                    i;
   for ( i = 0; i < cnxlp->cnxs; i++ )
   {
      if ( dstlp[i].portp == portp )
      {
         if ( idxp )
         {
            *idxp = i; 
         }
         return 1;
      }
   }
   return 0;
}

/***************************************************************************/
/**
*  Obtain audio mixer port information
*
*  @return
*     0     On success
*     -ve   Failure code
*/
int amxrCoreGetPortInfo(
   AMXR_HDL        hdl,           /*<< (i) Mixer client handle */
   AMXR_PORT_ID    port,          /*<< (i) Port id */
   AMXR_PORT_INFO *info           /*<< (o) Ptr to port info structure */
)
{
   struct amxr_port_node     *portp;
   struct amxr_cnxlist_node  *cnxlp;

   portp = getPort( port );
   if ( portp == NULL )
   {
      return -EINVAL;
   }

   portp->info.dst_cnxs = portp->dst_cnxs;
   portp->info.src_cnxs = portp->src_cnxs;

   mutex_lock( &gCnxs.mutex );

   /* Scan disabled list to determine non-active connection */
   llist_foreach_item( cnxlp, &gCnxs.disabled_list, lnode )
   {
      if ( amxr_find_dstport( portp, cnxlp, NULL ) )
      {
         portp->info.dst_cnxs++;
      }

      if ( cnxlp->srcportp == portp )
      {
         portp->info.src_cnxs++;
      }
   }

   mutex_unlock( &gCnxs.mutex );


   memcpy( info, &portp->info, sizeof(*info) );

   return 0;
}

/***************************************************************************/
/**
*  Obtain general audio mixer information, such as the number of
*  registered users.
*
*  @return
*     0     On success
*     -ve   Failure code
*/
int amxrCoreGetInfo(
   AMXR_HDL     hdl,              /*<< (i) Mixer client handle */
   AMXR_INFO   *info              /*<< (o) Ptr to info structure */
)
{
   memset( info, 0, sizeof(*info) );
   info->ports       = gPorts.total;
   info->cnx_lists   = gCnxs.total;
   return 0;
}

/***************************************************************************/
/**
*  Set connection loss to attenuate samples between source and destination
*  ports. Loss range is from 0 db to mute in 1db increments. 
*
*  @return
*     0           On success
*     -EINVAL     No such connection found
*     -ve         Other errors
*/
int amxrCoreSetCnxLoss(
   AMXR_HDL           hdl,        /*<< (i) client handle */
   AMXR_PORT_ID       src_port,   /*<< (i) source port id */
   AMXR_PORT_ID       dst_port,   /*<< (i) destination port id */
   AMXR_CONNECT_DESC  desc,       /*<< (i) Connection descriptor */
   unsigned int       db          /*<< (i) Loss amount in dB */
)
{
   struct amxr_port_node     *srcportp, *dstportp;
   struct amxr_cnxlist_node  *cnxlp;
   int                        rc;

   srcportp = getPort( src_port );
   dstportp = getPort( dst_port );
   if ( srcportp == NULL || dstportp == NULL )
   {
      return -EINVAL;
   }

   mutex_lock( &gCnxs.mutex );

   rc = -EINVAL; /* No matching cnx found */

   /* Scan active cnx list */
   llist_foreach_item( cnxlp, &gCnxs.list, lnode )
   {
      if ( amxr_set_cnxloss( cnxlp, srcportp, dstportp, desc, db ))
      {
         rc = 0; /* Successfully set loss */
         goto backout;
      }
   }

   /* Scan disabled cnx list */
   llist_foreach_item( cnxlp, &gCnxs.disabled_list, lnode )
   {
      if ( amxr_set_cnxloss( cnxlp, srcportp, dstportp, desc, db ))
      {
         rc = 0; /* Successfully set loss */
         goto backout;
      }
   }

backout:
   mutex_unlock( &gCnxs.mutex );
   return rc;
}

/***************************************************************************/
/**
*  Read connection loss amount in dB.
*
*  @return
*     0           On success
*     -EINVAL     No such connection found
*     -ve         Other errors
*/
int amxrCoreGetCnxLoss(
   AMXR_HDL           hdl,        /*<< (i) client handle */
   AMXR_PORT_ID       src_port,   /*<< (i) source port id */
   AMXR_PORT_ID       dst_port,   /*<< (i) destination port id */
   AMXR_CONNECT_DESC  desc,       /*<< (i) Connection descriptor */
   unsigned int      *db          /*<< (o) Pointer to store attenuation amount */
)
{
   struct amxr_port_node     *srcportp, *dstportp;
   struct amxr_cnxlist_node  *cnxlp;
   int                        rc;

   srcportp = getPort( src_port );
   dstportp = getPort( dst_port );
   if ( srcportp == NULL || dstportp == NULL )
   {
      return -EINVAL;
   }

   mutex_lock( &gCnxs.mutex );

   rc = -EINVAL;  /* no matching cnx found */

   llist_foreach_item( cnxlp, &gCnxs.list, lnode )
   {
      if ( amxr_get_cnxloss( cnxlp, srcportp, dstportp, desc, db ))
      {
         rc = 0; /* Successfully got loss value */
         goto backout;
      }
   }

   llist_foreach_item( cnxlp, &gCnxs.disabled_list, lnode )
   {
      if ( amxr_get_cnxloss( cnxlp, srcportp, dstportp, desc, db ))
      {
         rc = 0; /* Successfully got loss value */
         goto backout;
      }
   }

backout:
   mutex_unlock( &gCnxs.mutex );
   return rc;
}

/***************************************************************************/
/**
*  Make a simplex connection from source to destination ports.
*
*  @return
*     0           On success or connection already exists
*     -ve         On general failure
*/
int amxrCoreConnect(
   AMXR_HDL           hdl,        /*<< (i) client handle */
   AMXR_PORT_ID       src_port,   /*<< (i) source port id */
   AMXR_PORT_ID       dst_port,   /*<< (i) destination port id */
   AMXR_CONNECT_DESC  desc        /*<< (i) Connection descriptor */
)
{
   struct amxr_port_node  *srcportp, *dstportp;
   AMXR_CONNECT_TYPE       conn_type;
   AMXR_RESAMP_TYPE        rtype;
   int                     src_chans, src_idx, dst_chans, dst_idx;
   int                     err = 0;

   srcportp = getPort( src_port );
   dstportp = getPort( dst_port );
   if ( srcportp == NULL || dstportp == NULL )
   {
      return -EINVAL;
   }

   AMXR_DEBUG( "src=%s dst=%s desc=0x%x", 
         srcportp->info.name, dstportp->info.name, desc );

   conn_type = AMXR_CONNECT_GET_TYPE( desc );
   src_chans = AMXR_CONNECT_GET_SRC_CHANS( desc );
   src_idx   = AMXR_CONNECT_GET_SRC_IDX( desc );
   dst_chans = AMXR_CONNECT_GET_DST_CHANS( desc );
   dst_idx   = AMXR_CONNECT_GET_DST_IDX( desc );

   /** Validate connection properties based on port characteristics */

   if ( srcportp->info.src_chans != src_chans 
   ||   src_idx >= src_chans
   ||   dstportp->info.dst_chans != dst_chans 
   ||   dst_idx >= dst_chans )
   {
      AMXR_LOG( "connection properties are incompatible with ports: "
            "requested (src_chans=%d src_idx=%d dst_chans=%d dst_idx=%d) "
            "actual (src_chans=%d dst_chans=%i)",
            src_chans, src_idx, dst_chans, dst_idx,
            srcportp->info.src_chans, dstportp->info.dst_chans );
      return -EINVAL;
   }

   if ( src_chans != dst_chans )
   {
      /* Error check connection type given that source and destination ports 
       * have different number of sub-channels.
       */
      if (( conn_type == AMXR_CONNECT_STRAIGHT_THROUGH )
      ||  ( conn_type == AMXR_CONNECT_MULTI_TO_MONO_CONVERT && dst_chans != 1 )
      ||  ( conn_type == AMXR_CONNECT_MONO_TO_MULT_DUPLICATE && src_chans != 1 ))
      {
         AMXR_LOG( "inconsistent multi-channel interleave/deinterleave requested: conn_type=%d src_chans=%d dst_chans=%d",
               conn_type, src_chans, dst_chans );
         return -EINVAL;
      }
   }
   else
   {
      /* Error check that straight through is requested */
      if ( conn_type != AMXR_CONNECT_STRAIGHT_THROUGH )
      {
         AMXR_LOG( "should be connected as straight-through: conn_type=%d src_chans=%d dst_chans=%d",
               conn_type, src_chans, dst_chans );
         return -EINVAL;
      }
   }

   /* Check whether resampling between src and dst ports is feasible */
   err = amxr_check_resample_rates( srcportp->info.src_hz, dstportp->info.dst_hz, &rtype );
   if ( err )
   {
      AMXR_LOG( "resampling not supported between src_hz=%i and dst_hz=%i",
            srcportp->info.src_hz, dstportp->info.dst_hz );
      return err;
   }

   /* Do not support resampling for multi-channel straight through cnxs. */
   if ( conn_type == AMXR_CONNECT_STRAIGHT_THROUGH
   &&   src_chans > 1 
   &&   rtype )
   {
      /* FIXME: resampling for multi-channel straight-through may be supported
       *        by breaking it into multiple connections. This should be done
       *        automatically here.
       */

      AMXR_LOG( "Resampling for multi-channel straight-through unsupported. Split into separate connections with recomposition." );
      return -EINVAL;
   }

   return amxr_add_cnx( srcportp, dstportp, desc, rtype );
}

/***************************************************************************/
/**
*  Remove simplex connections between source and destination ports.
*
*  @return
*     0           On success
*     -ve         On general failure
*/
int amxrCoreDisconnect(
   AMXR_HDL           hdl,        /*<< (i) Mixer client handle */
   AMXR_PORT_ID       src_port,   /*<< (i) source port id */
   AMXR_PORT_ID       dst_port    /*<< (i) destination port id */
)
{
   struct amxr_port_node      *srcportp, *dstportp;
   struct amxr_cnxlist_node   *cnxlp, *tmpcnxlp;
   int                         delcnx;

   srcportp = getPort( src_port );
   dstportp = getPort( dst_port );
   if ( srcportp == NULL || dstportp == NULL )
   {
      return -EINVAL;
   }

   AMXR_DEBUG( "src=%s dst=%s", srcportp->info.name, dstportp->info.name );

   /* Protect connection related data in ports */
   mutex_lock( &gCnxs.mutex );
   /* Scan active connections */
   delcnx = 0;
   llist_foreach_item_safe( cnxlp, tmpcnxlp, &gCnxs.list, lnode )
   {
      if ( cnxlp->srcportp == srcportp )
      {
         if ( amxr_del_dst_from_cnxl_unsafe( cnxlp, dstportp ))
         {
            delcnx = 1; /* cnx deleted */
         }
      }
   }

   /* Scan disabled connections */
   llist_foreach_item_safe( cnxlp, tmpcnxlp, &gCnxs.disabled_list, lnode )
   {
      if ( cnxlp->srcportp == srcportp )
      {
         int i;
         if ( amxr_find_dstport( dstportp, cnxlp, &i ))
         {
            amxr_del_dstport_unsafe( cnxlp, i, &gCnxs.disabled_list, 
                  &gCnxs.disabled_total, AMXR_FREE_MEM );
            delcnx = 1; /* cnx deleted */
         }
      }
   }
   mutex_unlock( &gCnxs.mutex );

   if ( !delcnx )
   {
      /* Cannot find connection to delete. */
      return -EINVAL;
   }

   return 0;
}


/***************************************************************************/
/**
*  Query the connection list by source port. All destination ports connected
*  to the specified source port will be returned.
*
*  @return
*     0           On success
*     -ve         On general failure
*/
int amxrCoreGetCnxListBySrc(
   AMXR_HDL             hdl,        /*<< (i) Mixer client handle */
   AMXR_PORT_ID         src_port,   /*<< (i) Source port */
   AMXR_CNXS           *cnxlistp,   /*<< (o) Ptr to store cnx list */
   int                  maxlen      /*<< (i) Max length in bytes */
)
{
   struct amxr_port_node     *srcportp;
   struct amxr_cnxlist_node  *cnxlp;
   int                        err, mem_required;
   AMXR_PORT_CNXINFO         *portinfop;
   int                        cnxs;

   srcportp = getPort( src_port );
   if ( srcportp == NULL )
   {
      return -EINVAL;
   }

   mutex_lock( &gCnxs.mutex );

   mem_required = sizeof(*cnxlistp) + 
      sizeof(cnxlistp->list[0]) * ( srcportp->src_cnxs - 1 );
   cnxs = srcportp->src_cnxs;

   /* Scan disabled list to determine required memory */
   llist_foreach_item( cnxlp, &gCnxs.disabled_list, lnode )
   {
      if ( cnxlp->srcportp == srcportp )
      {
         mem_required += sizeof(cnxlistp->list[0]);
         cnxs++;
      }
   }

   if ( maxlen < mem_required )
   {
      err = -ENOMEM;
      goto backout;
   }

   cnxlistp->cnxs = cnxs;
   if ( cnxlistp->cnxs )
   {
      /* Scan all connections */
      portinfop = &cnxlistp->list[0];
      llist_foreach_item( cnxlp, &gCnxs.list, lnode )
      {
         if ( cnxlp->srcportp == srcportp )
         {
            amxr_info_from_cnxl( &portinfop, cnxlp, 
                  NULL /* no dstport filter */, 1 /* active */ );
         }
      }

      /* Scan disabled connections */
      llist_foreach_item( cnxlp, &gCnxs.disabled_list, lnode )
      {
         if ( cnxlp->srcportp == srcportp )
         {
            amxr_info_from_cnxl( &portinfop, cnxlp, 
                  NULL /*no dstport filter */, 0 /* inactive */ );
         }
      }
   }

backout:
   mutex_unlock( &gCnxs.mutex );
   return err;
}

/***************************************************************************/
/**
*  Query the connection list by destination port. All source ports connected
*  to the specified destination port will be returned.
*
*  @return
*     0           On success
*     -ve         On general failure
*/
int amxrCoreGetCnxListByDst(
   AMXR_HDL             hdl,        /*<< (i) Mixer client handle */
   AMXR_PORT_ID         dst_port,   /*<< (i) Dst port */
   AMXR_CNXS           *cnxlistp,   /*<< (o) Ptr to store cnx list */
   int                  maxlen      /*<< (i) Max length in bytes */
)
{
   struct amxr_port_node     *dstportp;
   struct amxr_cnxlist_node  *cnxlp;
   int                        err, mem_required, cnxs;
   AMXR_PORT_CNXINFO         *portinfop;

   dstportp = getPort( dst_port );
   if ( dstportp == NULL )
   {
      return -EINVAL;
   }

   mutex_lock( &gCnxs.mutex );

   mem_required = sizeof(*cnxlistp) + 
      sizeof(cnxlistp->list[0]) * ( dstportp->dst_cnxs - 1 );
   cnxs = dstportp->dst_cnxs;

   /* Scan disabled list to determine required memory */
   llist_foreach_item( cnxlp, &gCnxs.disabled_list, lnode )
   {
      if ( amxr_find_dstport( dstportp, cnxlp, NULL ) )
      {
         mem_required += sizeof(cnxlistp->list[0]);
         cnxs++;
      }
   }

   if ( maxlen < mem_required )
   {
      err = -ENOMEM;
      goto backout;
   }

   cnxlistp->cnxs = cnxs;
   if ( cnxlistp->cnxs )
   {
      /* Scan all connections for source ports */
      portinfop = &cnxlistp->list[0];
      llist_foreach_item( cnxlp, &gCnxs.list, lnode )
      {
         amxr_info_from_cnxl( &portinfop, cnxlp, dstportp, 1 /* active */ );
      }

      /* Scan disabled connections */
      llist_foreach_item( cnxlp, &gCnxs.disabled_list, lnode )
      {
         amxr_info_from_cnxl( &portinfop, cnxlp, dstportp, 0 /* active */ );
      }
   }

backout:
   mutex_unlock( &gCnxs.mutex );
   return err;
}

/***************************************************************************/
/**
*  Create a mixer port.
*
*  @return
*     0        On success
*     -ve      Error code
*/
int amxrCoreCreatePort(
   const char       *name,          /*<< (i) Name string */
   AMXR_PORT_CB     *cb,            /*<< (i) Callbacks */
   void             *privdata,      /*<< (i) Private data passed back to callbacks */
   int               dst_hz,        /*<< (i) Current destination sampling frequency in Hz */
   int               dst_chans,     /*<< (i) Number of channels, i.e. mono = 1 */
   int               dst_bytes,     /*<< (i) Destination period size in bytes */
   int               src_hz,        /*<< (i) Current source sampling frequency in Hz */
   int               src_chans,     /*<< (i) Number of channels, i.e. mono = 1 */
   int               src_bytes,     /*<< (i) Source period size in bytes */
   AMXR_PORT_ID     *portidp        /*<< (o) Ptr to port ID */
)
{
   struct amxr_port_node  *portp;
   unsigned long           state;

   /* Sanity checks */
   if ( cb == NULL )
   {
      return -EINVAL;
   }
   if (( src_hz && cb->getsrc == NULL )
   ||  ( dst_hz && cb->getdst == NULL ))
   {
      /* Missing callbacks */
      return -EINVAL;
   }

   portp = kmalloc( sizeof(*portp), AMXR_MEM_TYPE );
   if ( portp == NULL )
   {
      return -ENOMEM;
   }

   AMXR_DEBUG( "%s", name );

   memset( portp, 0, sizeof(*portp) );
   strncpy( portp->info.name, name, sizeof(portp->info.name) );
   memcpy( &portp->cb, cb, sizeof(portp->cb) );

   portp->privdata         = privdata;
   portp->info.dst_hz      = dst_hz;
   portp->info.dst_chans   = dst_chans;
   portp->info.dst_bytes   = dst_bytes;
   portp->info.src_hz      = src_hz;
   portp->info.src_chans   = src_chans;
   portp->info.src_bytes   = src_bytes;

   portp->dst_cnxs = 0;
   portp->src_cnxs = 0;
   portp->src_cnx_count = 0;

   mutex_lock( &gPorts.mutex );

   /* Critical section needed because port list scanned in ISR */
   local_irq_save(state);
   llist_add_tail( &portp->lnode, &gPorts.list );
   gPorts.total++;
   local_irq_restore( state );

   mutex_unlock( &gPorts.mutex );

   *portidp = getId( portp );

   return 0;
}

/***************************************************************************/
/**
*  Remove an existing mixer port. All connections to this port will also
*  be removed.
*
*  @return
*     0           On success
*     -ve         On general failure
*
*  @remarks
*     Typically ports are not removed unless the entire user is removed.
*     Although it is conceivable that the API supports dynamic creation and
*     deletion of ports, it is not practical for most applications since
*     applications will have to constantly query what ports still exists 
*     and keep track of valid port IDs. 
*/
int amxrCoreRemovePort(
   AMXR_PORT_ID port              /*<< (i) Port to remove */
)
{
   struct amxr_port_node     *portp;
   struct amxr_cnxlist_node  *cnxlp, *tmpcnxlp;
   unsigned long              state;
   int                        err = 0;

   portp = getPort( port );
   if ( portp == NULL )
   {
      return -EINVAL;
   }

   AMXR_DEBUG( "%s", portp->info.name );

   mutex_lock( &gPorts.mutex );
   
   /* Remove connections where this port is the source */
   if ( portp->src_cnxs )
   {
      mutex_lock( &gCnxs.mutex );
      llist_foreach_item_safe( cnxlp, tmpcnxlp, &gCnxs.list, lnode )
      {
         if ( cnxlp->srcportp == portp )
         {
            amxr_del_cnxl_unsafe( cnxlp, AMXR_FREE_MEM );
         }
      }
      mutex_unlock( &gCnxs.mutex );
   }

   /* Remove connections where this port is the destination */
   if ( portp->dst_cnxs )
   {
      mutex_lock( &gCnxs.mutex );
      llist_foreach_item_safe( cnxlp, tmpcnxlp, &gCnxs.list, lnode )
      {
         /* Attempt to delete destination port if it exists */
         amxr_del_dst_from_cnxl_unsafe( cnxlp, portp );
      }
      mutex_unlock( &gCnxs.mutex );
   }

   /* Remove disabled connections */
   llist_foreach_item_safe( cnxlp, tmpcnxlp, &gCnxs.disabled_list, lnode )
   {
      int i;

      /* Cleanup connections where this port is a source */
      if ( cnxlp->srcportp == portp )
      {
         llist_del( &cnxlp->lnode, &gCnxs.disabled_list );
         gCnxs.disabled_total--;
         amxr_free_cnxl_lnode( cnxlp );
         continue;
      }

      /* Cleanup connections where this port is a destination */
      if ( amxr_find_dstport( portp, cnxlp, &i ))
      {
         amxr_del_dstport_unsafe( cnxlp, i, &gCnxs.disabled_list, &gCnxs.disabled_total, AMXR_FREE_MEM );
      }
   }

   if ( portp->dst_cnxs || portp->src_cnxs )
   {
      AMXR_LOG( "leftover cnxs on port %s, dst_cnxs=%d src_cnxs%d. Port not deleted.",
            portp->info.name, portp->dst_cnxs, portp->src_cnxs );
      err = -EFAULT;
      goto backout;
   }

   /* Critical section needed because port list scanned in ISR */
   local_irq_save( state );
   llist_del( &portp->lnode, &gPorts.list );
   gPorts.total--;
   local_irq_restore( state );

   kfree( portp );

backout:
   mutex_unlock( &gPorts.mutex );
   return err;
}

/***************************************************************************/
/**
*  Set destination port sampling frequency. Connections using the port
*  are also updated accordingly.
*
*  @return
*     0           On success
*     -ve         On general failure
*/
int amxrCoreSetPortDstFreq(
   AMXR_PORT_ID   portid,           /*<< (i) Destination port id */
   int            dst_hz,           /*<< (i) Destination sampling frequency in Hz */
   int            dst_bytes         /*<< (i) Destination period size in bytes */
)
{
   struct amxr_port_node  *portp;
   unsigned long           state;
   int                     err = 0;
   int                     chg_dsthz;
   
   portp = getPort( portid );
   if ( portp == NULL )
   {
      return -EINVAL;
   }

   mutex_lock( &gPorts.mutex );

   AMXR_DEBUG( "%s dst_hz=%i dst_bytes=%i", portp->info.name, dst_hz, dst_bytes );

   chg_dsthz = dst_hz != portp->info.dst_hz;

   /* Update destination port info. Connections with this destation port will glitch. */
   local_irq_save(state);
   portp->info.dst_hz      = dst_hz;
   portp->info.dst_bytes   = dst_bytes;
   local_irq_restore( state );

   if ( chg_dsthz )
   {
      struct amxr_cnxlist_node   *cnxlp, *tmpcnxlp;
      struct amxr_port_node      *srcportp;
      AMXR_RESAMP_TYPE            rtype;
      AMXR_CONNECT_DESC           desc;
      struct amxr_cnxdstl_t      *dstlp;
      int                         i;
      struct amxr_cnxdstl_t       dstl_item;

      mutex_lock( &gCnxs.mutex );
      /* Scan active list */
      llist_foreach_item_safe( cnxlp, tmpcnxlp, &gCnxs.list, lnode )
      {
         if ( !amxr_find_dstport( portp, cnxlp, &i ))
         {
            continue;
         }

         /* Delete found connection and replace with a new one using
          * a different resampler. If resampling is impossible, then 
          * don't bother creating a new connection.
          */
         dstlp = cnxlp->dstlp;
         srcportp = cnxlp->srcportp;
         memcpy( &dstl_item, &dstlp[i], sizeof(dstl_item) );
         desc = AMXR_CREATE_DESC( cnxlp->cnx_type, cnxlp->src_chans, cnxlp->src_idx, dstl_item.dst_chans, dstl_item.dst_idx );

         /* Delete found connection. Not safe to use cnxlp nor dstlp[i] 
          * after this point because the memory may have been freed.
          */
         amxr_del_dst_from_cnxl_unsafe( cnxlp, portp );

         err = amxr_check_resample_rates( srcportp->info.src_hz, dst_hz, &rtype );
         if ( !err )
         {
            err = amxr_add_cnx_unsafe( srcportp, portp, desc, rtype, dstl_item.lossdb );
            if ( err )
            {
               AMXR_LOG( "failed to recreate connection between src=%s and dst=%s src_hz=%d dst_hz=%d",
                     srcportp->info.name, portp->info.name, srcportp->info.src_hz, dst_hz );
               goto fallthrough;
            }
         }
         else
         {
            /* Move connection to the disabled list */
            err = amxr_save_disabled_cnx_unsafe( srcportp, &dstl_item, desc );
            if ( err )
            {
               AMXR_LOG( "failed to move connection to disabled list (src=%s and dst=%s src_hz=%d dst_hz=%d)",
                     srcportp->info.name, portp->info.name, srcportp->info.src_hz, dst_hz );
               goto fallthrough;
            }
         }
      }
      
      /* Scan disabled connections to find those that can be re-enabled */
      llist_foreach_item_safe( cnxlp, tmpcnxlp, &gCnxs.disabled_list, lnode )
      {
         /* Find connections containing specific destination port */
         if ( amxr_find_dstport( portp, cnxlp, &i ))
         {
            srcportp = cnxlp->srcportp;

            /* Check if resampler exists */
            if ( amxr_check_resample_rates( srcportp->info.src_hz, dst_hz, &rtype ) == 0 )
            {
               dstlp = cnxlp->dstlp;
               memcpy( &dstl_item, &dstlp[i], sizeof(dstl_item) );
               desc = AMXR_CREATE_DESC( cnxlp->cnx_type, cnxlp->src_chans, cnxlp->src_idx, dstl_item.dst_chans, dstl_item.dst_idx );

               /* Remove cnx from disabled list. Not safe to use dstlp after this point */
               amxr_del_dstport_unsafe( cnxlp, i, &gCnxs.disabled_list, 
                     &gCnxs.disabled_total, AMXR_FREE_MEM );

               err = amxr_add_cnx_unsafe( srcportp, portp, desc, rtype, dstl_item.lossdb );
               if ( err )
               {
                  AMXR_LOG( "failed to re-enable connection between src=%s and dst=%s src_hz=%d dst_hz=%d",
                        srcportp->info.name, portp->info.name, srcportp->info.src_hz, dst_hz );
                  goto fallthrough;
               }
            }
         }
      }
fallthrough:
      mutex_unlock( &gCnxs.mutex );
   }

   mutex_unlock( &gPorts.mutex );
   return err;
}

/***************************************************************************/
/**
*  Set source port sampling frequency and expected frame size.
*  Connections using the port are also updated accordingly.
*
*  @return
*     0           On success
*     -ve         On general failure
*/
int amxrCoreSetPortSrcFreq(
   AMXR_PORT_ID   portid,           /*<< (i) Source port id */
   int            src_hz,           /*<< (i) Source sampling frequency in Hz */
   int            src_bytes         /*<< (i) Source period size in bytes */
)
{
   struct amxr_port_node  *portp;
   unsigned long           state;
   int                     err = 0;
   int                     chg_srchz;
   
   portp = getPort( portid );
   if ( portp == NULL )
   {
      return -EINVAL;
   }

   mutex_lock( &gPorts.mutex );

   AMXR_DEBUG( "%s src_hz=%i src_bytes=%i", portp->info.name, src_hz, src_bytes );

   chg_srchz = src_hz != portp->info.src_hz;

   /* Update source port info. Connections with this source port will glitch. */
   local_irq_save( state );
   portp->info.src_hz      = src_hz;
   portp->info.src_bytes   = src_bytes;
   local_irq_restore( state );

   if ( chg_srchz )
   {
      struct amxr_cnxlist_node   *cnxlp, *tmpcnxlp;
      struct amxr_port_node      *dstportp; 
      AMXR_RESAMP_TYPE            rtype;

      mutex_lock( &gCnxs.mutex );
      
      /* Scan active connections */
      llist_foreach_item_safe( cnxlp, tmpcnxlp, &gCnxs.list, lnode )
      {
         if ( cnxlp->srcportp == portp )
         {
            dstportp = cnxlp->dstlp->portp; /* first dst port in list */
            if ( amxr_check_resample_rates( src_hz, dstportp->info.dst_hz, &rtype ) == 0 )
            {
               /* Update resampling information for the connection list */
               err = amxr_alloc_resampler( cnxlp, rtype );
               if ( err )
               {
                  AMXR_LOG( "failed to update with new resampling info for source port %s src_hz=%d dst_hz=%d",
                        portp->info.name, src_hz, dstportp->info.dst_hz );
                  goto fallthrough; /* quit on error */
               }
            }
            else
            {
               /* Delete cnxlist from active list */
               amxr_del_cnxl_unsafe( cnxlp, AMXR_NO_FREE_MEM );
               
               /* Move the list to the disabled group */
               llist_add_head( &cnxlp->lnode, &gCnxs.disabled_list );
               gCnxs.disabled_total++;
            }
         }
      }

      /* Scan disabled connection to find those that can be re-enabled */
      llist_foreach_item_safe( cnxlp, tmpcnxlp, &gCnxs.disabled_list, lnode )
      {
         if ( cnxlp->srcportp == portp )
         {
            dstportp = cnxlp->dstlp->portp; /* first dst port in list */
            if ( amxr_check_resample_rates( src_hz, dstportp->info.dst_hz, &rtype ) == 0 )
            {
               struct amxr_cnxdstl_t *dstlp;
               AMXR_CONNECT_DESC desc;
               int i;
               
               /* Disintegrate cnxlist and re-add to active list */
               dstlp = cnxlp->dstlp;
               for ( i = 0; i < cnxlp->cnxs; i++ )
               {
                  desc = AMXR_CREATE_DESC( cnxlp->cnx_type, cnxlp->src_chans, cnxlp->src_idx, dstlp[i].dst_chans, dstlp[i].dst_idx );
                  err = amxr_add_cnx_unsafe( portp, dstlp[i].portp, desc, rtype, dstlp[i].lossdb );
                  if ( err )
                  {
                     AMXR_LOG( "failed to re-add cnx between src=%s and dst=%s src_hz=%d dst_hz=%d",
                           portp->info.name, dstlp[i].portp->info.name, src_hz, dstlp[i].portp->info.dst_hz );
                     goto fallthrough;
                  }
               }

               /* Cleanup memory */
               llist_del( &cnxlp->lnode, &gCnxs.disabled_list );
               gCnxs.disabled_total--;
               amxr_free_cnxl_lnode( cnxlp );
            }
         }
      }
fallthrough:
      mutex_unlock( &gCnxs.mutex );
   }

   mutex_unlock( &gPorts.mutex );
   return err;
}

/***************************************************************************/
/**
*  Set the number of channels supported by the destination port.
*
*  @return
*     0           On success
*     -ve         On general failure
*
*  @remarks    Not all existing connections with this source port
*              will be maintained after changing the number of channels.
*/
int amxrCoreSetPortDstChannels(
   AMXR_PORT_ID   portid,           /*<< (i) Destination port id */
   int            dst_chans,        /*<< (i) Number of channels: 1 for mono, 2 for stereo, etc */
   int            dst_bytes         /*<< (i) Destination period size in bytes */
)
{
   struct amxr_port_node  *portp;
   unsigned long           state;
   int                     err = 0;
   int                     changed;

   portp = getPort( portid );
   if ( portp == NULL )
   {
      return -EINVAL;
   }

   mutex_lock( &gPorts.mutex );

   AMXR_DEBUG( "%s dst_chans=%i dst_bytes=%i", portp->info.name, dst_chans, dst_bytes );

   changed = dst_chans != portp->info.dst_chans;

   /* Update destination port info. Connections with this destation port will glitch. */
   local_irq_save( state );
   portp->info.dst_chans   = dst_chans;
   portp->info.dst_bytes   = dst_bytes;
   local_irq_restore( state );

   /* Deal with connections where this port is a destination */
   if ( changed && portp->dst_cnxs )
   {
      struct amxr_cnxlist_node   *cnxlp, *tmpcnxlp;

      mutex_lock( &gCnxs.mutex );
      llist_foreach_item_safe( cnxlp, tmpcnxlp, &gCnxs.list, lnode )
      {
         struct amxr_cnxdstl_t *dstlp = cnxlp->dstlp;
         int i;

         /* Delete found connection and replace with a new one using
          * an appropriate connection descriptor if supported.
          */
         if ( amxr_find_dstport( portp, cnxlp, &i ))
         {
            struct amxr_port_node *srcportp;
            AMXR_RESAMP_TYPE     rtype;
            AMXR_CONNECT_DESC    desc;
            AMXR_CONNECT_TYPE    cnx_type;
            int                  dst_idx, supported;
            int                  src_idx, src_chans;
            unsigned int         lossdb;

            dst_idx     = dstlp[i].dst_idx;
            src_chans   = cnxlp->src_chans;
            src_idx     = cnxlp->src_idx;
            srcportp    = cnxlp->srcportp;
            cnx_type    = cnxlp->cnx_type;
            rtype       = cnxlp->resamptblp->rtype;
            lossdb      = dstlp[i].lossdb;

            /* Delete found connection. Not safe to use cnxlp nor dstlp[i] 
             * after this point because the memory may have been freed.
             */
            amxr_del_dst_from_cnxl_unsafe( cnxlp, portp );

            /* Transform the connection descriptor to deal with changed 
             * dst chans. Not all connection types can be easily
             * transformed and thus they may have to be dropped.
             */
            supported = 1;
            if ( cnx_type == AMXR_CONNECT_STRAIGHT_THROUGH )
            {
               if ( dst_chans == 1 )
               {
                  /* Transitioned to mono destination */
                  cnx_type = AMXR_CONNECT_MULTI_TO_MONO_CONVERT;
               }
               else
               {
                  if ( src_chans == 1 )
                  {
                     /* Transitioned to multi-channel destination */
                     cnx_type = AMXR_CONNECT_MONO_TO_MULT_DUPLICATE;
                  }
                  else
                  {
                     /* Multi-channel to multi-channel cnx not supported */
                     supported = 0;
                  }
               }
            }
            else if ( cnx_type == AMXR_CONNECT_MULTI_TO_MONO_CONVERT )
            {
               if ( src_chans == dst_chans )
               {
                  /* Transitioned to multi-channel dst that matches src */
                  cnx_type = AMXR_CONNECT_STRAIGHT_THROUGH;
               }
               else
               {
                  /* Multi-channel to multi-channel cnx not supported */
                  supported = 0;
               }
            }
            else if ( cnx_type == AMXR_CONNECT_MONO_TO_MULT_DUPLICATE )
            {
               /* Regardless of destination port channel config, the new 
                * connection is supported as a new mono-to-multi-duplicate
                * connection.
                */

               if ( dst_chans == 1 )
               {
                  /* Use straight through for mono to mono */
                  cnx_type = AMXR_CONNECT_STRAIGHT_THROUGH;
               }
            }
            else
            {
               /* FIXME: transforming a AMXR_CONNECT_EXTRACT_AND_DEPOSIT
                *        connection is currently not supported because of the 
                *        complexity of figuring out the different permutations
                *        of such connections. Although in practice, the 
                *        extract-and-deposit connections are mostly used to de-
                *        interleave stereo into mono components and vice versa.
                */
               supported = 0;
            }
               
            if ( supported )
            {
               AMXR_DEBUG( "new cnx: cnx_type=%i src_chans=%i dst_chans=%i", cnx_type, src_chans, dst_chans );

               desc = AMXR_CREATE_DESC( cnx_type, src_chans, src_idx, dst_chans, dst_idx );

               err = amxr_add_cnx_unsafe( srcportp, portp, desc, rtype, lossdb );
               if ( err )
               {
                  AMXR_LOG( "failed to recreate connection between src=%s and dst=%s src_chans=%d dst_chans=%d",
                        srcportp->info.name, portp->info.name, src_chans, dst_chans );
                  mutex_unlock( &gPorts.mutex );
                  return err;
               }
            }
         }
      }
      mutex_unlock( &gCnxs.mutex );
   }

   mutex_unlock( &gPorts.mutex );
   return err;
}

/***************************************************************************/
/**
*  Set the number of channels supported by the destination port.
*
*  @return
*     0           On success
*     -ve         On general failure
*
*  @remarks    Not all existing connections with this source port
*              will be maintained after changing the number of channels.
*/
int amxrCoreSetPortSrcChannels(
   AMXR_PORT_ID   portid,           /*<< (i) Source port id */
   int            src_chans,        /*<< (i) Number of src channels: 1 for mono, 2 for stereo, etc. */
   int            src_bytes         /*<< (i) Source period size in bytes */
)
{
   struct amxr_port_node  *portp;
   unsigned long           state;
   int                     changed;
   int                     err = 0;
   
   portp = getPort( portid );
   if ( portp == NULL )
   {
      return -EINVAL;
   }

   mutex_lock( &gPorts.mutex );

   AMXR_DEBUG( "%s src_chans=%i src_bytes=%i", portp->info.name, src_chans, src_bytes );

   changed = src_chans != portp->info.src_chans;

   /* Update source port info. Connections with this source port will glitch. */
   local_irq_save( state );
   portp->info.src_chans   = src_chans;
   portp->info.src_bytes   = src_bytes;
   local_irq_restore( state );

   /* Deal with connections where the port is a source. */
   if ( changed && portp->src_cnxs )
   {
      struct amxr_cnxlist_node   *cnxlp, *tmpcnxlp;

      mutex_lock( &gCnxs.mutex );
      llist_foreach_item_safe( cnxlp, tmpcnxlp, &gCnxs.list, lnode )
      {
         if ( cnxlp->srcportp == portp )
         {
            struct amxr_port_node *dstportp = cnxlp->dstlp->portp; /* first dst port in list */
            AMXR_CONNECT_TYPE    cnx_type;
            int                  supported;
            int                  dst_chans;

            cnx_type    = cnxlp->cnx_type;
            dst_chans   = dstportp->info.dst_chans;

            /* Transform the connection descriptor to deal with changed 
             * src chans. Not all connection types can be easily
             * transformed and thus they may have to be dropped.
             */
            supported = 1;
            if ( cnx_type == AMXR_CONNECT_STRAIGHT_THROUGH )
            {
               if ( src_chans == 1 )
               {
                  /* Transitioned to mono source */
                  cnx_type = AMXR_CONNECT_MONO_TO_MULT_DUPLICATE;
               }
               else
               {
                  if ( dst_chans == 1 )
                  {
                     /* Transitioned to multi-channel source */
                     cnx_type = AMXR_CONNECT_MULTI_TO_MONO_CONVERT;
                  }
                  else
                  {
                     /* Mis-matched multi-channel cnx not supported */
                     supported = 0;
                  }
               }
            }
            else if ( cnx_type == AMXR_CONNECT_MULTI_TO_MONO_CONVERT )
            {
               /* Regardless of source port channel configuration, the new 
                * connection is supported as a new multi-to-mono-convert
                * connection.
                */

               if ( src_chans == 1 )
               {
                  /* Use straight-through for mono to mono */
                  cnx_type = AMXR_CONNECT_STRAIGHT_THROUGH;
               }
            }
            else if ( cnx_type == AMXR_CONNECT_MONO_TO_MULT_DUPLICATE )
            {
               if ( src_chans == dst_chans )
               {
                  /* Transitioned to multi-channel src that matches dst */
                  cnx_type = AMXR_CONNECT_STRAIGHT_THROUGH;
               }
               else
               {
                  supported = 0;
               }
            }
            else
            {
               /* FIXME: transforming a AMXR_CONNECT_EXTRACT_AND_DEPOSIT
                *        connection is currently not supported because of the 
                *        complexity of figuring out the different permutations
                *        of such connections. Although in practice, the 
                *        extract-and-deposit connections are mostly used to de-
                *        interleave stereo into mono components and vice versa.
                */
               supported = 0;
            }

            if ( supported )
            {
               local_irq_save( state );
               cnxlp->cnx_type   = cnx_type;
               cnxlp->src_chans  = src_chans;
               local_irq_restore( state );
            }
            else
            {
               /* Delete connection list since type is not transformable */
               amxr_del_cnxl_unsafe( cnxlp, AMXR_FREE_MEM );
            }
         }
      }
      mutex_unlock( &gCnxs.mutex );
   }

   mutex_unlock( &gPorts.mutex );
   return err;
}

/***************************************************************************/
/**
*  This helper routine is called by the main mixer routine to resample
*  data. 
*
*  @remarks
*     This routine is expected to be run in an atomic context.
*/
static void amxr_resample(
   const int16_t                *srcp,       /*<< (i)  Input data to resample */
   int                           src_samps,  /*<< (i)  Input data size in samples */
   int16_t                      *dstp,       /*<< (o)  Ptr to store resampled output */
   int                           dst_samps,  /*<< (i)  Output data size in samples */
   const AMXR_RESAMPLE_TABENTRY *resamptblp, /*<< (i)  Resample table */
   int16_t                      *resampbufp  /*<< (io) Resample buffer containing history */
)
{
   int filterlen = resamptblp->filterlen;

   /* Copy new input samples to resample */
   memcpy( &resampbufp[filterlen], srcp, src_samps * sizeof(int16_t) );

   gResampFnc( &resampbufp[filterlen], 
         dstp,          /* output samples */
         dst_samps,     /* number of 16-bit samples to output */
         resamptblp->coeffs,
         filterlen, 
         resamptblp->inter_ratio,
         resamptblp->decim_ratio );

   /* Update resample buffer history */
   memcpy( resampbufp, srcp + src_samps - filterlen, filterlen * sizeof(int16_t) );
}

/***************************************************************************/
/**
*  This helper routine is used to mix data assuming source and destination 
*  buffers are of the same format.
*
*  @remarks
*     This routine is expected to be run in an atomic context.
*/
static inline void amxr_output_samefmt_data(
   int16_t          *dstp,             /*<< (o) dst data ptr */
   int16_t          *srcp,             /*<< (i) src data ptr */
   int               samples,          /*<< (i) output length in samples */
   uint32_t          dst_first,        /*<< (i) Flag to indicate first frame to output */
   uint16_t          lossdb,           /*<< (i) Loss in dB */
   uint16_t          losslin           /*<< (i) Loss in linear gain value */
)
{
   if ( lossdb )
   {
      /* Apply loss */
      if ( dst_first )
      {
         gMpyCFnc( dstp, srcp, samples, losslin );
      }
      else
      {
         /* Vector add new samples with gain */
         gMpyCAddFnc( dstp, srcp, samples, losslin );
      }
   }
   else
   {
      if ( dst_first )
      {
         /* Copy new samples for first frame */
         memcpy( dstp, srcp, samples * sizeof(int16_t) );
      }
      else
      {
         /* Vector add new samples */
         gAddFnc( dstp, dstp, srcp, samples );
      }
   }
}

/***************************************************************************/
/**
*  This helper routine is used to extract a sub-channel from an interleaved
*  multi-channel source buffer to create a mono buffer.
*
*  @remarks
*     This routine is expected to be run in an atomic context.
*/
static inline void amxr_extract_to_mono(
   int16_t *dstp,                   /*<< (o) destination linear buffer ptr */
   int16_t *srcp,                   /*<< (i) source buffer to extract from */
   int      len,                    /*<< (i) output len in 16-bit samples */
   int      src_chans,              /*<< (i) number of sub-channels in source data */
   int      src_idx                 /*<< (i) sub-channel index to extract */
)
{
   int i;
   srcp += src_idx;
   for ( i = 0; i < len; i++ )
   {
      dstp[i]  = *srcp;
      srcp    += src_chans;
   }
}

/***************************************************************************/
/**
*  This helper routine is used to deposit samples into an interleaved
*  sub-channel. 
*
*  @remarks
*/
static inline void amxr_deposit_sub_channel(
   int16_t       *dstp,             /*<< (o) destination interleaved buffer ptr */
   const int16_t *srcp,             /*<< (i) source buffer to extract from */
   int            samples,          /*<< (i) number of samples to deposit */
   int            dst_chans,        /*<< (i) number of sub-channels in source data */
   int            dst_idx           /*<< (i) sub-channel index to extract */
)
{
   int k;
   dstp += dst_idx;
   for ( k = 0; k < samples; k++ )
   {
      *dstp = *srcp++;
      dstp += dst_chans;
   }
}

/***************************************************************************/
/**
*  This routine is called by the main mixer routine to perform straight
*  through mixing, which should be the most common form of connection.
*
*  @remarks
*     This routine is expected to be run in an atomic context.
*/
static void amxr_straight_through_mix(
   struct amxr_cnxlist_node   *cnxlp,     /*<< (i) cnx list pointer */
   struct amxr_port_node      *targetdstp,/*<< (i) targeted dest port. If set only service this dst port */
   int16_t                    *scratchp   /*<< (o) scratch buffer pointer */
)
{
   struct amxr_port_node     *dstportp, *srcportp;
   struct amxr_cnxdstl_t     *dstlp;
   int16_t                   *srcp;
   int16_t                   *dstp;
   int                        i, cnxs;
   AMXR_RESAMPLE_TABENTRY    *resamptblp;
   int                        dst_samps;
   int                        src_samps;

   srcportp    = cnxlp->srcportp;
   dstlp       = cnxlp->dstlp;
   cnxs        = cnxlp->cnxs;
   resamptblp  = cnxlp->resamptblp;
   srcp        = srcportp->srcp;

   /* Frame size based on first dst port for now. May have to implement
    * differently when variable frame size is required. Regardless
    * of variable or fixed frame size, every party in the list
    * is expected to use the same output frame size.
    */
   dst_samps   = dstlp->portp->info.dst_bytes/2;
   src_samps   = srcportp->info.src_bytes/2;

   /* Resampling required */
   if ( resamptblp->rtype )
   {
      amxr_resample( srcp, src_samps, gScratchBufp /* output */, 
            dst_samps, resamptblp, 
            cnxlp->resampbufp /* resample area and history */ );

      srcp = scratchp;        /* New source pointer */
   }

   /* Walk through destination list */
   for ( i = 0; i < cnxs; i++, dstlp++ )
   {
      dstportp = dstlp->portp;
      dstp     = dstportp->dstp;

      if ( targetdstp )
      {
         if ( dstportp != targetdstp )
         {
            /* Skip non-matching destination */
            continue;
         }
      }

      if ( dstp )
      {
         amxr_output_samefmt_data( dstp, srcp, dst_samps, dstportp->dst_first,
               dstlp->lossdb, dstlp->losslin );
         dstportp->dst_first = 0;
      }
   }
}

/***************************************************************************/
/**
*  This routine is called by the main mixer routine to perform the extract 
*  and deposit mixing.
*
*  @remarks
*     This routine is expected to be run in an atomic context.
*/
static void amxr_extract_and_deposit_mix(
   struct amxr_cnxlist_node   *cnxlp,     /*<< (i) cnx list pointer */
   struct amxr_port_node      *targetdstp,/*<< (i) targeted dest port. If set only service this dst port */
   int16_t                    *scratchp   /*<< (o) scratch buffer pointer */
)
{
   int                        src_chans, src_idx, dst_chans, dst_idx;
   int                        dst_samps, src_samps;
   struct amxr_cnxdstl_t     *dstlp;
   int                        i, cnxs;
   AMXR_RESAMPLE_TABENTRY    *resamptblp;
   int16_t                   *srcp, *dstp;
   struct amxr_port_node     *dstportp, *srcportp;

   dst_idx     = 0;  /* fix warning for some compilers */
   srcportp    = cnxlp->srcportp;
   dstlp       = cnxlp->dstlp;
   cnxs        = cnxlp->cnxs;
   resamptblp  = cnxlp->resamptblp;
   srcp        = srcportp->srcp;

   /* Frame size based on first dst port for now. May have to implement
    * differently when variable frame size is required. Regardless
    * of variable or fixed frame size, every party in the list
    * is expected to use the same output frame size.
    */
   dst_samps   = dstlp->portp->info.dst_bytes/2;
   src_samps   = srcportp->info.src_bytes/2;

   src_chans   = cnxlp->src_chans;
   src_idx     = cnxlp->src_idx;

   /* Dst channels based on first in list consistent with dst_samps */
   dst_chans   = dstlp->dst_chans;  

   /* Extract sub-channel to create mono buffer */
   if ( src_chans > 1 )
   {
      src_samps = src_samps / src_chans;

      amxr_extract_to_mono( scratchp, srcp, src_samps, src_chans, src_idx );

      srcp      = scratchp;         /* New source pointer */
      scratchp += src_samps;
   }
   if ( dst_chans > 1 )
   {
      dst_samps = dst_samps / dst_chans;
   }

   /* Resampling required */
   if ( resamptblp->rtype )
   {
      amxr_resample( srcp, src_samps, scratchp /* output */,
            dst_samps, resamptblp, 
            cnxlp->resampbufp       /* resample area and history */ );

      srcp      = scratchp;         /* New source pointer */
      scratchp += dst_samps;
   }

   /* Walk through destination list */
   for ( i = 0; i < cnxs; i++, dstlp++ )
   {
      dstportp = dstlp->portp;
      dstp     = dstportp->dstp;

      if ( targetdstp )
      {
         if ( dstportp != targetdstp )
         {
            /* Skip non-matching destination */
            continue;
         }
      }

      if ( dstp )
      {
         uint32_t bmask;

         dst_chans = dstlp->dst_chans;
         bmask     = AMXR_DST_FIRST_INITVAL;

         /* Extract sub-channel to create mono buffer */
         if ( dst_chans > 1 )
         {
            /* Adjust bit mask based on sub-channel of interest */
            dst_idx  = dstlp->dst_idx;
            bmask    = 1 << dst_idx;

            /* Only extract if there is real data */
            if ( !( dstportp->dst_first & bmask ))
            {
               amxr_extract_to_mono( scratchp, dstp, dst_samps, dst_chans, dst_idx );
            }

            dstp = scratchp;        /* New destination pointer */
         }

         amxr_output_samefmt_data( dstp, srcp, dst_samps, dstportp->dst_first & bmask,
               dstlp->lossdb, dstlp->losslin );
         dstportp->dst_first &= ~bmask;

         /* Deposit samples into the requested sub-channel */
         if ( dst_chans > 1 )
         {
            amxr_deposit_sub_channel( dstportp->dstp, dstp, dst_samps, dst_chans, dst_idx );
         }
      }
   }
}

/***************************************************************************/
/**
*  This routine is called by the main mixer routine to convert a multi-
*  channel source into a mono output by averaging
*
*  @remarks
*     This routine is expected to be run in an atomic context.
*/
static void amxr_convert_to_mono_mix(
   struct amxr_cnxlist_node   *cnxlp,     /*<< (i) cnx list pointer */
   struct amxr_port_node      *targetdstp,/*<< (i) targeted dest port. If set only service this dst port */
   int16_t                    *scratchp   /*<< (o) scratch buffer pointer */
)
{
   int                        src_chans;
   int                        dst_samps, src_samps;
   struct amxr_cnxdstl_t     *dstlp;
   int                        i, j, cnxs;
   AMXR_RESAMPLE_TABENTRY    *resamptblp;
   int16_t                   *srcp, *dstp;
   struct amxr_port_node     *dstportp, *srcportp;
   int32_t                    acc;

   srcportp    = cnxlp->srcportp;
   dstlp       = cnxlp->dstlp;
   cnxs        = cnxlp->cnxs;
   resamptblp  = cnxlp->resamptblp;
   srcp        = srcportp->srcp;

   /* Frame size based on first dst port for now. May have to implement
    * differently when variable frame size is required. Regardless
    * of variable or fixed frame size, every party in the list
    * is expected to use the same output frame size.
    */
   dst_samps   = dstlp->portp->info.dst_bytes/2;

   src_chans   = cnxlp->src_chans;
   src_samps   = srcportp->info.src_bytes/2;
   src_samps   = src_samps / src_chans;

   /* Average sub-channels into mono buffer */
   dstp = scratchp;
   for ( i = 0; i < src_samps; i++ )
   {
      acc = 0;
      for ( j = 0; j < src_chans; j++ )
      {
         acc += *srcp++;
      }
      acc      = acc / src_chans;
      *dstp++  = (int16_t)acc;
   }
   srcp     = scratchp;          /* New source pointer */
   scratchp = dstp;

   /* Resampling required */
   if ( resamptblp->rtype )
   {
      amxr_resample( srcp, src_samps, scratchp /* output */,
            dst_samps, resamptblp, 
            cnxlp->resampbufp    /* resample area and history */ );

      srcp = scratchp;           /* New source pointer */
   }

   /* Walk through destination list */
   for ( i = 0; i < cnxs; i++, dstlp++ )
   {
      dstportp = dstlp->portp;
      dstp     = dstportp->dstp;

      if ( targetdstp )
      {
         if ( dstportp != targetdstp )
         {
            /* Skip non-matching destination */
            continue;
         }
      }

      if ( dstp )
      {
         amxr_output_samefmt_data( dstp, srcp, dst_samps, dstportp->dst_first,
               dstlp->lossdb, dstlp->losslin );
         dstportp->dst_first = 0;
      }
   }
}

/***************************************************************************/
/**
*  This routine is called by the main mixer routine to duplicate a mono
*  source into a multi-channel output
*
*  @remarks
*     This routine is expected to be run in an atomic context.
*/
static void amxr_duplicate_mono_mix(
   struct amxr_cnxlist_node   *cnxlp,     /*<< (i) cnx list pointer */
   struct amxr_port_node      *targetdstp,/*<< (i) targeted dest port. If set only service this dst port */
   int16_t                    *scratchp   /*<< (o) scratch buffer pointer */
)
{
   int                        dst_chans;
   int                        dst_samps, src_samps;
   struct amxr_cnxdstl_t     *dstlp;
   int                        i, j, cnxs;
   AMXR_RESAMPLE_TABENTRY    *resamptblp;
   int16_t                   *srcp, *dstp;
   struct amxr_port_node     *dstportp, *srcportp;

   srcportp    = cnxlp->srcportp;
   dstlp       = cnxlp->dstlp;
   cnxs        = cnxlp->cnxs;
   resamptblp  = cnxlp->resamptblp;
   srcp        = srcportp->srcp;

   /* Frame size based on first dst port for now. May have to implement
    * differently when variable frame size is required. Regardless
    * of variable or fixed frame size, every party in the list
    * is expected to use the same output frame size.
    */
   dst_chans   = dstlp->dst_chans;
   dst_samps   = dstlp->portp->info.dst_bytes / 2 / dst_chans;
   src_samps   = srcportp->info.src_bytes/2;

   /* Resampling required */
   if ( resamptblp->rtype )
   {
      amxr_resample( srcp, src_samps, scratchp /* output */,
            dst_samps, resamptblp, 
            cnxlp->resampbufp    /* resample area and history */ );

      srcp      = scratchp;      /* New source pointer */
      scratchp += dst_samps;
   }

   /* Walk through destination list */
   for ( i = 0; i < cnxs; i++, dstlp++ )
   {
      dstportp = dstlp->portp;
      dstp     = dstportp->dstp;

      if ( targetdstp )
      {
         if ( dstportp != targetdstp )
         {
            /* Skip non-matching destination */
            continue;
         }
      }

      if ( dstp )
      {
         uint32_t bmask;

         dst_chans   = dstlp->dst_chans;

         /* Deposit samples into all sub-channels */
         for ( j = 0, bmask = 1; j < dst_chans; j++, bmask <<= 1 )
         {
            /* Only extract if there is real data */
            if ( !( dstportp->dst_first & bmask ))
            {
               amxr_extract_to_mono( scratchp, dstp, dst_samps, dst_chans, j );
            }

            amxr_output_samefmt_data( scratchp, srcp, dst_samps, dstportp->dst_first & bmask,
                  dstlp->lossdb, dstlp->losslin );
            dstportp->dst_first &= ~bmask;

            amxr_deposit_sub_channel( dstp, scratchp, dst_samps, dst_chans, j );
         }
         dstportp->dst_first = 0;
      }
   }
}

/***************************************************************************/
/**
*  This routine is called by the main mixer routine to process the
*  connection.
*
*  @remarks
*     This routine is expected to be run in an atomic context.
*/
static inline void amxr_process_cnxl(
   struct amxr_cnxlist_node   *cnxlp,     /*<< (i) cnx list pointer */
   struct amxr_port_node      *targetdstp,/*<< (i) targeted dest port. If set only service this dst port */
   int16_t                    *scratchp   /*<< (o) scratch buffer pointer */
)
{
   AMXR_CONNECT_TYPE cnx_type = cnxlp->cnx_type;

   if ( cnx_type == AMXR_CONNECT_STRAIGHT_THROUGH )
   {
      amxr_straight_through_mix( cnxlp, targetdstp, scratchp );
   }
   else if ( cnx_type == AMXR_CONNECT_EXTRACT_AND_DEPOSIT )
   {
      amxr_extract_and_deposit_mix( cnxlp, targetdstp, scratchp );
   }
   else if ( cnx_type == AMXR_CONNECT_MULTI_TO_MONO_CONVERT )
   {
      amxr_convert_to_mono_mix( cnxlp, targetdstp, scratchp );
   }
   else if ( cnx_type == AMXR_CONNECT_MONO_TO_MULT_DUPLICATE )
   {
      amxr_duplicate_mono_mix( cnxlp, targetdstp, scratchp );
   }
}

/***************************************************************************/
/**
*  This routine executes the mixer and services all connections at sampling
*  frequencies that are multiples of 8. 
*
*  @remarks
*     Typically, this routine is called by HAL Audio, which is clocked 
*     by hardware.
*
*     This routine runs in an ATOMIC context!
*
*     This routine is expected to be called every 5ms. In the future it 
*     may be able to handle elapsed time that is greater than 5ms.
*/
void amxrElapsedTime( 
   int elapsed_usec                 /*<< (i) Amount of time elapsed since last call in usec */
)
{
   struct amxr_port_node      *portp;
   struct amxr_cnxlist_node   *cnxlp;

   /* Cache src and dst pointers for connected ports. This step minimizes 
    * the number of times getsrc and getdst callbacks are called. However, in
    * some circumstances the getsrc and getdst callbacks have to be called 
    * multiple times, such as when the destination port frame size is variable.
    */
   llist_foreach_item( portp, &gPorts.list, lnode )
   {
      if ( portp->src_cnxs && AMXR_SYNC_FREQ( portp->info.src_hz ))
      {
         portp->srcp = portp->cb.getsrc( portp->info.src_bytes, portp->privdata );
      }
      if ( portp->dst_cnxs && AMXR_SYNC_FREQ( portp->info.dst_hz ))
      {
         portp->dstp = portp->cb.getdst( portp->info.dst_bytes, portp->privdata );
         /* awaiting first frame to deposit with bit mask */
         portp->dst_first  = (1 << portp->info.dst_chans) - 1;
      }
   }

   /* Service connections */
   llist_foreach_item( cnxlp, &gCnxs.list, lnode )
   {
      struct amxr_port_node *srcportp = cnxlp->srcportp;

      if ( srcportp->srcp == NULL || !AMXR_SYNC_FREQ( srcportp->info.src_hz ))
      {
         /* No source data available or incompatible sampling frequency.
          * Skip connection
          */
         continue;
      }

      amxr_process_cnxl( cnxlp, NULL, gScratchBufp );
   }

   /* Issue done callbacks */
   llist_foreach_item( portp, &gPorts.list, lnode )
   {
      if ( portp->cb.srcdone && AMXR_SYNC_FREQ( portp->info.src_hz ))
      {
         portp->cb.srcdone( portp->info.src_bytes, portp->privdata );
      }
      if ( portp->cb.dstdone && AMXR_SYNC_FREQ( portp->info.dst_hz ))
      {
         portp->cb.dstdone( portp->info.dst_bytes, portp->privdata );
      }
   }
}
EXPORT_SYMBOL( amxrElapsedTime );

/***************************************************************************/
/**
*  This routine services all connections to and from a particular port
*  that are at "irregular" sampling frequencies, which are defined as not 
*  a multiple of 8.
*  
*  @remarks
*     Typically, this routine is called by HAL Audio, which is clocked 
*     by hardware.
*
*     This routine runs in an ATOMIC context!
*/
void amxrCoreServiceUnsyncPort( 
   AMXR_PORT_ID id                  /*<< (i) Port ID of port to service */
)
{
   struct amxr_port_node    *portp;
   struct amxr_cnxlist_node *cnxlp;

   /* Cache src and dst pointers */
   llist_foreach_item( portp, &gPorts.list, lnode )
   {
      if ( portp->src_cnxs && !AMXR_SYNC_FREQ( portp->info.src_hz ))
      {
         portp->srcp = portp->cb.getsrc( portp->info.src_bytes, portp->privdata );
      }
      if ( portp->dst_cnxs && !AMXR_SYNC_FREQ( portp->info.dst_hz ))
      {
         portp->dstp = portp->cb.getdst( portp->info.dst_bytes, portp->privdata );
         portp->dst_first  = AMXR_DST_FIRST_INITVAL;  /* awaiting first frame to deposit */
      }
   }

   portp = getPort_nocheck( id );

   /* Service connections */
   llist_foreach_item( cnxlp, &gCnxs.list, lnode )
   {
      struct amxr_port_node *srcportp = cnxlp->srcportp;

      if ( srcportp->srcp == NULL || AMXR_SYNC_FREQ( srcportp->info.src_hz ))
      {
         /* No source data available or connection running at frequencies 
          * that are divisible by 8. Skip
          */
         continue;
      }

      if ( srcportp == portp || amxr_find_dstport( portp, cnxlp, NULL ))
      {
         /* If this port is a dst port in the connection, only process for this
          * port and leave alone. If this port is a src port, then process the 
          * entire list 
          */
         amxr_process_cnxl( cnxlp, (srcportp == portp) ? NULL : portp, gScratchBufp );

         AMXR_TRACE( "src_cnx_count=%i src_cnxs=%i srcportp=0x%lx portp=0x%lx\n",
               srcportp->src_cnx_count, srcportp->src_cnxs, (unsigned long) srcportp, (unsigned long)portp );

         /* Only send source done it is actually done */
         srcportp->src_cnx_count--;
         if ( srcportp->src_cnx_count <= 0 )
         {
            if ( srcportp->cb.srcdone )
            {
               srcportp->cb.srcdone( srcportp->info.src_bytes, srcportp->privdata );
            }
            srcportp->src_cnx_count = srcportp->src_cnxs;
         }
      }
   }

   /* Isse dstdone callback */
   if ( portp->cb.dstdone && !AMXR_SYNC_FREQ( portp->info.dst_hz ))
   {
      portp->cb.dstdone( portp->info.dst_bytes, portp->privdata );
   }
}

/***************************************************************************/
/**
*  Decrements a ports destination connection count. When the count reaches
*  0, trigger callback if registered.
*/
static void amxr_dec_dst_cnxcount_unsafe(
   struct amxr_port_node    *dstportp   /*<< (io) Ptr to destination port */
)
{
   dstportp->dst_cnxs--;
   if ( dstportp->dst_cnxs == 0 )
   {
      if ( dstportp->cb.dstcnxsremoved )
      {
         /* Last destination connection removed */
         dstportp->cb.dstcnxsremoved( dstportp->privdata );
      }
   }
}

/***************************************************************************/
/**
*  Frees a connection list node. 
*
*  @remarks
*     The node is assumed to be unconnected
*     with any list. This routine just does the memory cleanup.
*/
static void amxr_free_cnxl_lnode(
   struct amxr_cnxlist_node *cnxlp   /*<< (i) Ptr to a cnx list node */
)
{
   /* Free memory resources */
   amxr_free_resamp( cnxlp );
   kfree( cnxlp->dstlp );
   kfree( cnxlp );
}

/***************************************************************************/
/**
*  Deletes a destination port from a connection list if it is found. If
*  the destination port is the only connection in the list, the list is 
*  deleted as well.
*
*  @return
*     1     - Destination port deleted
*     0     - Destination port not found
*
*  @remarks
*     This routine is NOT thread safe. It is expected that the caller 
*     aquires the connection list mutex.
*/
static int amxr_del_dst_from_cnxl_unsafe(
   struct amxr_cnxlist_node *cnxlp,     /*<< (i) Ptr to a cnx list node */
   struct amxr_port_node    *dstportp   /*<< (i) Ptr to destination port */
)
{
   struct amxr_port_node   *srcportp;
   unsigned long            state;
   int                      i, delcnx;
   struct amxr_cnxdstl_t   *dstlp;

   delcnx   = 0;
   srcportp = cnxlp->srcportp;
   dstlp    = cnxlp->dstlp;

   /* Find matching destination port */
   for ( i = 0; i < cnxlp->cnxs; i++ )
   {
      if ( dstportp == dstlp[i].portp )
      {
         delcnx = 1;
         local_irq_save( state );
         /* Squish out destination port */
         amxr_del_dstport_unsafe( cnxlp, i, &gCnxs.list, &gCnxs.total, AMXR_NO_FREE_MEM );

         /* Some accounting */
         srcportp->src_cnxs--;
         srcportp->src_cnx_count--;
         local_irq_restore( state );

         if ( cnxlp->cnxs == 0 )
         {
            /* Free memory outside of interrupt disable context */
            amxr_free_cnxl_lnode( cnxlp );
         }

         amxr_dec_dst_cnxcount_unsafe( dstportp );

         /* Done */
         break;
      }
   }

   return delcnx;
}

/***************************************************************************/
/**
*  Deletes an entire connection list 
*
*  @return
*     0     Connection list successfully deleted
*     -ve   Error code
*
*  @remarks
*     This routine is NOT thread safe. It is expected that the caller 
*     aquires the connection list mutex.
*/
static int amxr_del_cnxl_unsafe(
   struct amxr_cnxlist_node *cnxlp,  /**< (i) Ptr to connection list to delete */
   int                       free    /**< (i) AMXR_FREE_MEM to free cnxlist memory, 
                                              otherwise use AMXR_NO_FREE_MEM */
)
{
   struct amxr_port_node     *srcportp;
   unsigned long              state;
   int                        i;
   struct amxr_cnxdstl_t     *dstlp;

   srcportp = cnxlp->srcportp;

   local_irq_save( state );
   llist_del( &cnxlp->lnode, &gCnxs.list );
   gCnxs.total--;

   /* Do some accounting */
   srcportp->src_cnxs -= cnxlp->cnxs;
   if ( srcportp->src_cnxs < 0 )
   {
      srcportp->src_cnxs = 0;
   }
   srcportp->src_cnx_count -= cnxlp->cnxs;
   if ( srcportp->src_cnx_count < 0 )
   {
      srcportp->src_cnx_count = 0;
   }
   local_irq_restore( state );

   dstlp = cnxlp->dstlp;
   for ( i = 0; i < cnxlp->cnxs; i++ )
   {
      amxr_dec_dst_cnxcount_unsafe( dstlp[i].portp );
   }

   if ( free )
   {
      /* Free memory */
      amxr_free_cnxl_lnode( cnxlp );
   }

   return 0;
}

/***************************************************************************/
/**
*  Helper routine to initialize a destination list element.
*
*  @return Nothing
*/
static void amxr_init_cnxdstl_element(
   struct amxr_cnxdstl_t  *dstelemp,   /**< (o) Ptr to dst list element */
   struct amxr_port_node  *dstportp,   /**< (i) Ptr to dst port node */
   int                     dst_chans,  /**< (i) number of sub-channels */
   int                     dst_idx,    /**< (i) sub-channel index to operate on */
   unsigned int            lossdb      /**< (i) connection loss value in dB */
)
{
   uint16_t losslin;

   if ( lossdb <= AMXR_MAX_Q16_LOSS )
   {
      losslin = q16GainMap[lossdb];
   }
   else
   {
      losslin = 0;   /* mute */
   }

   memset( dstelemp, 0, sizeof(*dstelemp) );
   dstelemp->portp      = dstportp;
   dstelemp->lossdb     = lossdb;
   dstelemp->losslin    = losslin;
   dstelemp->dst_chans  = dst_chans;
   dstelemp->dst_idx    = dst_idx;
}

/***************************************************************************/
/**
*  Creates a mixer connection between source and destination ports without
*  providing any error checking whether the connection is feasible.
*
*  @return
*     0     Connection successfully connected
*     -ve   Failed to connect, error code
*
*  @remarks
*/
static int amxr_add_cnx(
   struct amxr_port_node   *srcportp,  /*<< (i) Ptr to source port */
   struct amxr_port_node   *dstportp,  /*<< (i) Ptr to destination port */
   AMXR_CONNECT_DESC        desc,      /*<< (i) Connection descriptor */
   AMXR_RESAMP_TYPE         rtype      /*<< (i) Resampler type */
)
{
   int err;

   mutex_lock( &gCnxs.mutex );
   err = amxr_add_cnx_unsafe( srcportp, dstportp, desc, rtype, 0 /* default no loss */ );
   mutex_unlock( &gCnxs.mutex );

   return err;
}

/***************************************************************************/
/**
*  Creates a mixer connection between source and destination ports without
*  providing any error checking whether the connection is feasible.
*
*  @return
*     0     Connection successfully connected
*     -ve   Failed to connect, error code
*
*  @remarks This routine is not thread safe.
*/
static int amxr_add_cnx_unsafe(
   struct amxr_port_node   *srcportp,  /*<< (i) Ptr to source port */
   struct amxr_port_node   *dstportp,  /*<< (i) Ptr to destination port */
   AMXR_CONNECT_DESC        desc,      /*<< (i) Connection descriptor */
   AMXR_RESAMP_TYPE         rtype,     /*<< (i) Resampler type */
   unsigned int             lossdb     /**< (i) Connection loss in dB */
)
{
   struct amxr_cnxlist_node  *cnxlp;
   int                        foundcnx, err;
   unsigned long              state;
   struct amxr_cnxdstl_t     *dstlp;
   AMXR_CONNECT_TYPE          cnx_type;
   int                        src_chans, src_idx, dst_chans, dst_idx;

   cnx_type   = AMXR_CONNECT_GET_TYPE( desc );
   src_chans  = AMXR_CONNECT_GET_SRC_CHANS( desc );
   src_idx    = AMXR_CONNECT_GET_SRC_IDX( desc );
   dst_chans  = AMXR_CONNECT_GET_DST_CHANS( desc );
   dst_idx    = AMXR_CONNECT_GET_DST_IDX( desc );

   /* Determine if connection already exists */

   foundcnx = 0;
   if ( srcportp->src_cnxs )
   {
      llist_foreach_item( cnxlp, &gCnxs.list, lnode )
      {
         if ( cnxlp->srcportp == srcportp 
         &&   cnxlp->resamptblp->rtype == rtype
         &&   cnxlp->cnx_type == cnx_type 
         &&   cnxlp->src_idx == src_idx )
         {
            int i;

            /* Found appropriate list to join. */
            foundcnx = 1;

            /* Check that connection does not already exist */
            dstlp = cnxlp->dstlp;
            for ( i = 0; i < cnxlp->cnxs; i++ )
            {
               if ( dstportp == dstlp[i].portp && dst_idx == dstlp[i].dst_idx )
               {
                  return 0;
               }
            }

            break;   /* quit searching */
         }
      }
   }
   
   if ( foundcnx )
   {
      /* Found appropriate connection list to join */

      if ( cnxlp->cnxs < cnxlp->max_cnxs )
      {
         /* Sufficient room left, add destination to connection list */
         local_irq_save( state );
         amxr_init_cnxdstl_element( &dstlp[cnxlp->cnxs], dstportp, dst_chans, dst_idx, lossdb ); 
         cnxlp->cnxs++;
         local_irq_restore( state );
      }
      else
      {
         struct amxr_cnxdstl_t *newlistp;
         int                    max_cnxs;

         /* Need to resize list */
         max_cnxs = cnxlp->max_cnxs + AMXR_CNXLIST_SIZE_INCREMENT;
         newlistp = kmalloc( max_cnxs * sizeof(newlistp[0]), AMXR_MEM_TYPE );
         if ( newlistp == NULL )
         {
            return -ENOMEM;
         }

         memset( newlistp, 0, max_cnxs * sizeof(newlistp[0]) );
         memcpy( newlistp, dstlp, sizeof(newlistp[0]) * cnxlp->max_cnxs );

         amxr_init_cnxdstl_element( &newlistp[cnxlp->cnxs], dstportp, dst_chans, dst_idx, lossdb );
         
         /* Replace old list with new */
         local_irq_save( state );
         cnxlp->dstlp    = newlistp;
         cnxlp->max_cnxs = max_cnxs;
         cnxlp->cnxs++;
         local_irq_restore( state );

         kfree( dstlp );
      }
   }
   else
   {
      /* Create new connection list */
      cnxlp = kmalloc( sizeof(*cnxlp), AMXR_MEM_TYPE );
      if ( cnxlp == NULL )
      {
         return -ENOMEM;
      }

      memset( cnxlp, 0, sizeof(*cnxlp) );

      dstlp = kmalloc( AMXR_CNXLIST_SIZE_INCREMENT * sizeof(dstlp[0]), AMXR_MEM_TYPE );
      if ( dstlp == NULL )
      {
         kfree( cnxlp );
         return -ENOMEM;
      }
      memset( dstlp, 0, AMXR_CNXLIST_SIZE_INCREMENT * sizeof(dstlp[0]) );
      amxr_init_cnxdstl_element( &dstlp[0], dstportp, dst_chans, dst_idx, lossdb );

      cnxlp->srcportp   = srcportp;
      cnxlp->dstlp      = dstlp;
      cnxlp->max_cnxs   = AMXR_CNXLIST_SIZE_INCREMENT;
      cnxlp->cnx_type   = cnx_type;
      cnxlp->src_chans  = src_chans;
      cnxlp->src_idx    = src_idx;

      cnxlp->cnxs++;

      /* Setup resampler */
      err = amxr_alloc_resampler( cnxlp, rtype );
      if ( err )
      {
         kfree( cnxlp->dstlp );
         kfree( cnxlp );
         return err;
      }

      /* New connection lists created are added to the beginning 
       * to avoid an infinite looping scenario where the caller is 
       * scanning all lists for a particular connection that is
       * being added.
       */ 
      local_irq_save( state );
      if ( cnx_type == AMXR_CONNECT_EXTRACT_AND_DEPOSIT )
      {
         /* Reorder extract and deposit connections to end of list.
          * This is to avoid destination buffer clearing problems
          * when mixing an extract-and-deposit connection with another 
          * type of connection. For example, suppose the first connection
          * is to deposit a mono channel into the left channel of a stereo
          * buffer. The second connection is to mix a stereo source into the
          * same stereo buffer. The issue is that only the left channel is 
          * initialize. The right channel is uninitialized. To optimize
          * the second (straight-through) connection, it either overwrites
          * or adds to the buffer, neither is appropriate. The solution is
          * to reorder the two connections.
          */
         llist_add_tail( &cnxlp->lnode, &gCnxs.list );
      }
      else
      {
         llist_add_head( &cnxlp->lnode, &gCnxs.list );
      }
      gCnxs.total++;
      local_irq_restore( state );
   }

   /* Update source and destination port connection counters */
   local_irq_save( state );
   srcportp->src_cnxs++;
   srcportp->src_cnx_count++;
   dstportp->dst_cnxs++;
   local_irq_restore( state );

   return 0;
}

/***************************************************************************/
/**
*  Saves a mixer connection between source and destination ports to the
*  disabled list.
*
*  @return
*     0     Connection successfully connected
*     -ve   Failed to connect, error code
*
*  @remarks
*
*  This routine is derived from amxr_add_cnx().
*
*  A connection is disabled because no resampler exists to 
*  support it. It may be re-enabled later when one of the ports changes
*  sampling frequency and the new frequency is compatible with existing
*  resamplers.
*
*  Critical sections (i.e. disabling of interrupts) are not used in this
*  routine because the ISR does not modify nor access the disabled list.
*/
static int amxr_save_disabled_cnx_unsafe(
   struct amxr_port_node   *srcportp,  /**< (i) Ptr to source port */
   struct amxr_cnxdstl_t   *dstl_itemp,/**< (i) Ptr to dst item structure */
   AMXR_CONNECT_DESC        desc       /**< (i) Connection descriptor */
)
{
   struct amxr_cnxlist_node  *cnxlp;
   int                        err = 0;
   struct amxr_cnxdstl_t     *dstlp;
   AMXR_CONNECT_TYPE          cnx_type;
   int                        src_chans, src_idx;

   cnx_type   = AMXR_CONNECT_GET_TYPE( desc );
   src_chans  = AMXR_CONNECT_GET_SRC_CHANS( desc );
   src_idx    = AMXR_CONNECT_GET_SRC_IDX( desc );

   /* To simplify management of the disabled list, saved cnxs are not merged
    * with existing connections. A new connection list is always created. This
    * is an acceptable trade-off because the expectation is that there won't
    * be that many disabled cnxs at any one time. 
    */

   /* Create new connection list */
   cnxlp = kmalloc( sizeof(*cnxlp), AMXR_MEM_TYPE );
   if ( cnxlp == NULL )
   {
      return -ENOMEM;
   }

   memset( cnxlp, 0, sizeof(*cnxlp) );

   dstlp = kmalloc( AMXR_CNXLIST_SIZE_INCREMENT * sizeof(dstlp[0]), AMXR_MEM_TYPE );
   if ( dstlp == NULL )
   {
      kfree( cnxlp );
      return -ENOMEM;
   }
   
   /* initialize first dst list item */
   memcpy( dstlp, dstl_itemp, sizeof(dstlp[0]) );

   cnxlp->srcportp   = srcportp;
   cnxlp->dstlp      = dstlp;
   cnxlp->max_cnxs   = AMXR_CNXLIST_SIZE_INCREMENT;
   cnxlp->cnx_type   = cnx_type;
   cnxlp->src_chans  = src_chans;
   cnxlp->src_idx    = src_idx;

   cnxlp->cnxs++;

   /* Setup default 1:1 resampler as a placeholder */
   err = amxr_alloc_resampler( cnxlp, AMXR_RESAMP_TYPE_NONE );
   if ( err )
   {
      kfree( cnxlp->dstlp );
      kfree( cnxlp );
      return err;
   }

   /* New connection lists created are added to the beginning 
    * to avoid an infinite looping scenario where the caller is 
    * scanning all lists for a particular connection that is
    * being added.
    */ 
   llist_add_head( &cnxlp->lnode, &gCnxs.disabled_list );
   gCnxs.disabled_total++;

   return 0;
}

/***************************************************************************/
/**
*  Allocate resampler for a connection list. If a resampler
*  is already in place and a new resampler is requested, the new
*  setting will overwrite old settings. If a resampler is not longer 
*  needed, any previously allocated resampler will be freed.
*
*  @return
*     0        Success, resampler setup
*     -EINVAL  No appropriate resampler can be found
*/
static int amxr_alloc_resampler(
   struct amxr_cnxlist_node   *cnxlp,  /*<< (i) Ptr to connection list */
   AMXR_RESAMP_TYPE            rtype   /*<< (i) Resampler type */
)
{
   unsigned long              state;
   AMXR_RESAMPLE_TABENTRY    *tablep;
   int                        err = 0;

   /* Allocate static resampler memory for the first time */
   if ( cnxlp->resampbufp == NULL )
   {
      cnxlp->resampbufp = kmalloc( AMXR_RESAMP_BUFFER_BYTES, AMXR_MEM_TYPE );
      if ( cnxlp->resampbufp == NULL )
      {
         return -ENOMEM;
      }
   }
   memset( cnxlp->resampbufp, 0, AMXR_RESAMP_BUFFER_BYTES );

   err = amxr_get_resampler( rtype, &tablep );
   if ( err )
   {
      return err;
   }

   local_irq_save( state );
   cnxlp->resamptblp = tablep;
   local_irq_restore( state );

   return 0;
}

/***************************************************************************/
/**
*  Free resampler resources from connection list 
*
*  @return
*     0        Success
*     -ENOMEM  Insufficient memory
*/
static void amxr_free_resamp(
   struct amxr_cnxlist_node *cnxlp   /*<< (io) Ptr to connectionn list */
)
{
   /* A valid connection list will always have a resampler buffer */
   kfree( cnxlp->resampbufp );
   cnxlp->resampbufp = NULL;
}

/***************************************************************************/
/**
*  Helper to release global resources
*
*  @return  Nothing
*/
static void amxr_free_resources( void )
{
   //These two calls do nothing if CONFIG_DEBUG_MUTEXES is not set
   mutex_destroy( gCnxs.mutex );
   mutex_destroy( gPorts.mutex );
   if ( gScratchBufp )
   {
      kfree( gScratchBufp );
      gScratchBufp = NULL;
   }
}

/***************************************************************************/
/**
*  Helper routine to squish out a destination port from a cnxlist. 
*
*  @return  Nothing
*
*  @remark This routine is not thread safe.
*/
static void amxr_del_dstport_unsafe(
   struct amxr_cnxlist_node *cnxlp,    /**< (i)  Ptr to a cnx list node */
   int i,                              /**< (i)  Dst port index in list */
   LLIST_NODE *listp,                  /**< (io) Ptr to list head */
   int *count,                         /**< (o)  Ptr to list's total count */
   int free                            /**< (i)  AMXR_FREE_MEM to free the memory, 
                                                 AMXR_NO_FREE_MEM to not free */
)
{
   struct amxr_cnxdstl_t *dstlp = cnxlp->dstlp;

   /* Squish out destination port */
   memmove( &dstlp[i], &dstlp[i+1], (cnxlp->cnxs-i-1) * sizeof(dstlp[0]) );
   cnxlp->cnxs--;
   if ( cnxlp->cnxs == 0 )
   {
      /* No one left in the list, so remove from cnxs list */
      llist_del( &cnxlp->lnode, listp );
      (*count)--;
      if ( free )
      {
         amxr_free_cnxl_lnode( cnxlp );
      }         
   }
}

/***************************************************************************/
/**
*  Helper routine to initialize connection info from connection list.
*
*  @return Nothing
*
*  @remark This routine is not thread safe.
*/
static void amxr_info_from_cnxl(
   AMXR_PORT_CNXINFO **cnxinfopp,      /**< (o)  Ptr to cnx info structure */
   struct amxr_cnxlist_node *cnxlp,    /**< (i)  Ptr to cnx list */
   struct amxr_port_node *dstportp,    /**< (i)  Ptr to dst port used for filtering */
   int active                          /**< (i)  Flag to indicate active status */
)
{
   struct amxr_cnxdstl_t *dstlp;
   AMXR_PORT_CNXINFO *cnxinfop;
   int i;

   cnxinfop = *cnxinfopp;
   dstlp = cnxlp->dstlp;
   for ( i = 0; i < cnxlp->cnxs; i++, dstlp++ )
   {
      if ( dstportp == NULL || dstlp->portp == dstportp )
      {
         if ( dstlp->portp == dstportp )
         {
            /* Return source port id because we're searching on dst port */
            cnxinfop->port = getId( cnxlp->srcportp );
         }
         else
         {
            cnxinfop->port = getId( dstlp->portp );
         }
         cnxinfop->lossdb = dstlp->lossdb;
         cnxinfop->decim  = cnxlp->resamptblp->decim_ratio;
         cnxinfop->inter  = cnxlp->resamptblp->inter_ratio;
         cnxinfop->desc   = AMXR_CREATE_DESC( cnxlp->cnx_type, cnxlp->src_chans,
               cnxlp->src_idx, dstlp->dst_chans, dstlp->dst_idx );
         cnxinfop->active = active;
         cnxinfop++;
      }
   }
   *cnxinfopp = cnxinfop;
}

/***************************************************************************/
/**
*  Helper function to search and set loss value
*
*  @return 1 if successfully set loss value, otherwise 0
*/
static int amxr_set_cnxloss(
   struct amxr_cnxlist_node *cnxlp, /**<  (io) Ptr to cnx list */
   struct amxr_port_node *srcportp, /**<  (i)  Ptr to src port */
   struct amxr_port_node *dstportp, /**<  (i)  Ptr to dst port */
   AMXR_CONNECT_DESC desc,          /**<  (i)  Connection descriptor */
   unsigned int db                  /**<  (i)  Loss amount in dB */
)
{
   AMXR_CONNECT_TYPE cnx_type;
   int               src_idx, dst_idx;
   uint16_t          losslin;

   if ( db <= AMXR_MAX_Q16_LOSS )
   {
      losslin = q16GainMap[db];
   }
   else
   {
      losslin = 0;   /* mute */
   }

   cnx_type   = AMXR_CONNECT_GET_TYPE( desc );
   src_idx    = AMXR_CONNECT_GET_SRC_IDX( desc );
   dst_idx    = AMXR_CONNECT_GET_DST_IDX( desc );

   if ( cnxlp->srcportp == srcportp
   &&   cnxlp->cnx_type == cnx_type 
   &&   cnxlp->src_idx == src_idx )
   {
      struct amxr_cnxdstl_t *dstlp;
      int i;

      /* Look for precise destination port */
      dstlp = cnxlp->dstlp;
      for ( i = 0; i < cnxlp->cnxs; i++, dstlp++ )
      {
         if ( dstportp == dstlp->portp && dst_idx == dstlp->dst_idx )
         {
            dstlp->lossdb  = db;
            dstlp->losslin = losslin;
            return 1;
         }
      }
   }
   return 0;
}

/***************************************************************************/
/**
*  Helper function to search and get loss value
*
*  @return
*/
static int amxr_get_cnxloss(
   struct amxr_cnxlist_node *cnxlp, /**<  (i)  Ptr to cnx list */
   struct amxr_port_node *srcportp, /**<  (i)  Ptr to src port */
   struct amxr_port_node *dstportp, /**<  (i)  Ptr to dst port */
   AMXR_CONNECT_DESC desc,          /**<  (i)  Connection descriptor */
   unsigned int *db                 /**<  (o)  Ptr to loss variable */
)
{
   AMXR_CONNECT_TYPE cnx_type;
   int               src_idx, dst_idx;

   cnx_type   = AMXR_CONNECT_GET_TYPE( desc );
   src_idx    = AMXR_CONNECT_GET_SRC_IDX( desc );
   dst_idx    = AMXR_CONNECT_GET_DST_IDX( desc );

   if ( cnxlp->srcportp == srcportp
   &&   cnxlp->cnx_type == cnx_type 
   &&   cnxlp->src_idx == src_idx )
   {
      struct amxr_cnxdstl_t *dstlp;
      int i;

      /* Look for precise destination port */
      dstlp = cnxlp->dstlp;
      for ( i = 0; i < cnxlp->cnxs; i++, dstlp++ )
      {
         if ( dstportp == dstlp->portp && dst_idx == dstlp->dst_idx )
         {
            *db = dstlp->lossdb;
            return 1;
         }
      }
   }
   return 0;
}

/***************************************************************************/
/**
*  Driver initialization called when module loaded by kernel
*
*  @return  
*     0              Success
*     -ve            Error code
*/
static int __init amxr_init( void )
{
   AMXR_API_FUNCS apifuncs;
   int            err = 0;

   memset( &gCnxs, 0, sizeof(gCnxs) );
   memset( &gPorts, 0, sizeof(gPorts) );
   llist_head_init( &gCnxs.list );
   llist_head_init( &gPorts.list );
   mutex_init( &gCnxs.mutex );
   mutex_init( &gPorts.mutex );
   

   /* Install default math operators */
   gResampFnc     = amxrCResample;
   gAddFnc        = amxrCVectorAdd;
   gMpyCFnc       = amxrCVectorMpyQ16;
   gMpyCAddFnc    = amxrCVectorMacQ16;

   /* Allocate scratch buffer */
   gScratchBufp   = kmalloc( AMXR_SCRATCHBUF_BYTES, AMXR_MEM_TYPE );
   if ( gScratchBufp == NULL )
   {
      err = -ENOMEM;
      goto cleanup_and_exit;
   }

   memset( &apifuncs, 0, sizeof(apifuncs) );
   apifuncs.getPortInfo          = amxrCoreGetPortInfo;
   apifuncs.getInfo              = amxrCoreGetInfo;
   apifuncs.setCnxLoss           = amxrCoreSetCnxLoss;
   apifuncs.getCnxLoss           = amxrCoreGetCnxLoss;
   apifuncs.connect              = amxrCoreConnect;
   apifuncs.disconnect           = amxrCoreDisconnect;
   apifuncs.getCnxListBySrc      = amxrCoreGetCnxListBySrc;
   apifuncs.getCnxListByDst      = amxrCoreGetCnxListByDst;
   apifuncs.createPort           = amxrCoreCreatePort;
   apifuncs.removePort           = amxrCoreRemovePort;
   apifuncs.setPortDstFreq       = amxrCoreSetPortDstFreq;
   apifuncs.setPortSrcFreq       = amxrCoreSetPortSrcFreq;
   apifuncs.setPortDstChannels   = amxrCoreSetPortDstChannels;
   apifuncs.setPortSrcChannels   = amxrCoreSetPortSrcChannels;
   apifuncs.serviceUnsyncPort    = amxrCoreServiceUnsyncPort;
   amxrSetApiFuncs( &apifuncs );

   return 0;

cleanup_and_exit:
   amxr_free_resources();
   return err;
}

/***************************************************************************/
/**
*  Driver destructor routine. Frees all resources
*/
static void __exit amxr_exit( void )
{
   struct amxr_port_node  *portp, *tmpportp;
   int                     err = 0;

   amxrSetApiFuncs( NULL );

   /* Delete all ports */
   llist_foreach_item_safe( portp, tmpportp, &gPorts.list, lnode )
   {
      err = amxrCoreRemovePort( portp );
      if ( err )
      {
         AMXR_LOG( "failed to delete port %s rc=%i", 
               portp->info.name, err );
      }
   }

   /* Sanity checks */
   if ( gPorts.total )
   {
      AMXR_LOG( "orphaned %i ports", gPorts.total );
   }
   if ( gCnxs.total )
   {
      AMXR_LOG( "orphaned %i connections", gCnxs.total );
   }
   if ( gCnxs.disabled_total )
   {
      AMXR_LOG( "orphaned %i disabled connections", gCnxs.disabled_total );
   }

   amxr_free_resources();
}

module_init( amxr_init );
module_exit( amxr_exit );
MODULE_AUTHOR( "Broadcom" );
MODULE_DESCRIPTION( "Audio Mixer Driver" );
MODULE_LICENSE( "GPL" );
