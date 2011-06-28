/*****************************************************************************
* Copyright 2009 Broadcom Corporation.  All rights reserved.
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
*  @file    amxr.h
*
*****************************************************************************/
#if !defined( AMXR_H )
#define AMXR_H

/* ---- Include Files ---------------------------------------------------- */
#if defined( __KERNEL__ )
#include <linux/types.h>               /* Needed for standard types */
#else
#include <stdint.h>
#endif

/* ---- Constants and Types ---------------------------------------------- */

/* Mixer port ID type */
typedef void * AMXR_PORT_ID;

/**
*  Audio mixer port information.
*
*  A mixer port is the conduit for ferrying samples in two directions,
*  and thus a port provides both source and destination data info.
*
*  Samples are expected to 16-bits wide.
*/
typedef struct amxr_port_info
{
   char           name[32];            /* Name string */
   int            dst_hz;              /* Current destination sampling frequency in Hz */
   int            dst_chans;           /* Number of channels, i.e. mono = 1 */
   int            dst_bytes;           /* destination period size in bytes */
   int            dst_cnxs;            /* Number of destination connections */
   int            src_hz;              /* Current source sampling frequency in Hz */
   int            src_chans;           /* Number of channels, i.e. mono = 1 */
   int            src_bytes;           /* Source period size in bytes */
   int            src_cnxs;            /* Number of source connections */
}
AMXR_PORT_INFO;

/* Audio mixer general information */
typedef struct amxr_info
{
   int ports;                          /* Number of registered ports */
   int cnx_lists;                      /* Number of internal connections lists */
   int clients;                        /* Number of clients */
}
AMXR_INFO;

/* Connection types */
typedef enum amxr_connect_type
{
   AMXR_CONNECT_STRAIGHT_THROUGH,      /* No interleave and de-interleave needed */
   AMXR_CONNECT_EXTRACT_AND_DEPOSIT,   /* Extract and deposit single sub-channel */
   AMXR_CONNECT_MULTI_TO_MONO_CONVERT, /* Convert multi-channels into single mono channel by averaging */
   AMXR_CONNECT_MONO_TO_MULT_DUPLICATE,/* Duplicate mono channel into multi-channel output */
}
AMXR_CONNECT_TYPE;

/**
*  Mixer connection descriptor is formed by the following bit fields:
*
*     type        - (4 bits) connection type (AMXR_CONNECT_TYPE)
*     src_chans   - (4 bits) number of source interleaved sub-channels
*     src_idx     - (4 bits) source sub-channel index to operate on, only applicable to recomposition
*     dst_chans   - (4 bits) number of destination interleaved sub-channels
*     dst_idx     - (4 bits) destination sub-channel index to operate on, only applicable to recomposition
*
* | Reserved (31-20) | type (19-16) | src_chans (15-12) | src_idx (11-8) | dst_chans (7-4) | dst_idx (3-0) |
*
*/
typedef uint32_t AMXR_CONNECT_DESC;

#define AMXR_CONNECT_TYPE_SHIFT        16
#define AMXR_CONNECT_SRC_CHANS_SHIFT   12
#define AMXR_CONNECT_SRC_IDX_SHIFT     8
#define AMXR_CONNECT_DST_CHANS_SHIFT   4
#define AMXR_CONNECT_DST_IDX_SHIFT     0

#define AMXR_CONNECT_TYPE_BMASK        0x0f0000
#define AMXR_CONNECT_SRC_CHANS_BMASK   0x00f000
#define AMXR_CONNECT_SRC_IDX_BMASK     0x000f00
#define AMXR_CONNECT_DST_CHANS_BMASK   0x0000f0
#define AMXR_CONNECT_DST_IDX_BMASK     0x00000f

/* Helper macros to create connection descriptors */
#define AMXR_CREATE_DESC( type, src_chans, src_idx, dst_chans, dst_idx ) \
   ((((type) << AMXR_CONNECT_TYPE_SHIFT ) & AMXR_CONNECT_TYPE_BMASK) | \
    (((src_chans) << AMXR_CONNECT_SRC_CHANS_SHIFT ) & AMXR_CONNECT_SRC_CHANS_BMASK) | \
    (((src_idx) << AMXR_CONNECT_SRC_IDX_SHIFT ) & AMXR_CONNECT_SRC_IDX_BMASK) | \
    (((dst_chans) << AMXR_CONNECT_DST_CHANS_SHIFT ) & AMXR_CONNECT_DST_CHANS_BMASK) | \
    (((dst_idx) << AMXR_CONNECT_DST_IDX_SHIFT ) & AMXR_CONNECT_DST_IDX_BMASK))

/* Helper macros to parse connection descriptor */
#define AMXR_CONNECT_GET_TYPE(d)       (AMXR_CONNECT_TYPE)(((d) & AMXR_CONNECT_TYPE_BMASK) >> AMXR_CONNECT_TYPE_SHIFT)
#define AMXR_CONNECT_GET_SRC_CHANS(d)  (((d) & AMXR_CONNECT_SRC_CHANS_BMASK) >> AMXR_CONNECT_SRC_CHANS_SHIFT)
#define AMXR_CONNECT_GET_SRC_IDX(d)    (((d) & AMXR_CONNECT_SRC_IDX_BMASK) >> AMXR_CONNECT_SRC_IDX_SHIFT)
#define AMXR_CONNECT_GET_DST_CHANS(d)  (((d) & AMXR_CONNECT_DST_CHANS_BMASK) >> AMXR_CONNECT_DST_CHANS_SHIFT)
#define AMXR_CONNECT_GET_DST_IDX(d)    (((d) & AMXR_CONNECT_DST_IDX_BMASK) >> AMXR_CONNECT_DST_IDX_SHIFT)

/* The following descriptors are defined for the common connection descriptors */
#define AMXR_CONNECT_MONO2MONO       AMXR_CREATE_DESC( AMXR_CONNECT_STRAIGHT_THROUGH, 1, 0, 1, 0 )
#define AMXR_CONNECT_STEREO2STEREO   AMXR_CREATE_DESC( AMXR_CONNECT_STRAIGHT_THROUGH, 2, 0, 2, 0 )
#define AMXR_CONNECT_STEREOSPLITL    AMXR_CREATE_DESC( AMXR_CONNECT_EXTRACT_AND_DEPOSIT, 2, 0, 1, 0 ) /* Left is always first sub-channel */
#define AMXR_CONNECT_STEREOSPLITR    AMXR_CREATE_DESC( AMXR_CONNECT_EXTRACT_AND_DEPOSIT, 2, 1, 1, 0 ) /* Right is always second sub-channel */
#define AMXR_CONNECT_MONO2STEREOL    AMXR_CREATE_DESC( AMXR_CONNECT_EXTRACT_AND_DEPOSIT, 1, 0, 2, 0 ) /* Left is always first sub-channel */
#define AMXR_CONNECT_MONO2STEREOR    AMXR_CREATE_DESC( AMXR_CONNECT_EXTRACT_AND_DEPOSIT, 1, 0, 2, 1 ) /* Right is always second sub-channel */
#define AMXR_CONNECT_STEREO2MONO     AMXR_CREATE_DESC( AMXR_CONNECT_MULTI_TO_MONO_CONVERT, 2, 0, 1, 0 )
#define AMXR_CONNECT_MONO2STEREO     AMXR_CREATE_DESC( AMXR_CONNECT_MONO_TO_MULT_DUPLICATE, 1, 0, 2, 0 )

/* A port's connection information */
typedef struct amxr_port_cnxinfo
{
   AMXR_PORT_ID      port;             /* Port ID */
   int               lossdb;           /* Connection loss */
   AMXR_CONNECT_DESC desc;             /* Connection descriptor */
   int               decim;            /* Decimation factor */
   int               inter;            /* Interpolation factor */
   int               active;           /* 1 if active, otherwise 0 */
}
AMXR_PORT_CNXINFO;

/* Connection list */
typedef struct amxr_cnxs
{
   int               cnxs;             /* Number of connected ports */
   AMXR_PORT_CNXINFO list[1];          /* Variable list of connected ports */
}
AMXR_CNXS;

/* Mixer client handle */
typedef int          AMXR_HDL;

#if defined( __KERNEL__ )

/* MakeDefs: Off */

/**
*  Mixer port callbacks. Each port can choose to implement all or some
*  of the callbacks.
*
*  Two callbacks are used by the mixer to obtain destination and
*  source data pointers, and two other callbacks are used to indicate
*  when the mixer has finished its business with the pointers. All data
*  pointers point to 16-bit signed data.
*
*  Destination and source directions are relative to the port. A destination
*  pointer is used to write data into the port, where a source point is
*  used to read data from the port.
*/
typedef struct amxr_port_cb
{
   /* This callback returns a pointer to the audio source data for 
    * one frame period and it takes a frame size in bytes as a parameter.
    * It may be called more than once in a frame period with the same 
    * or different frame size. The callback may return a valid pointer 
    * or a NULL pointer if the frame size is invalid. If a NULL pointer 
    * is returned, the Audio Mixer will not make use of the pointer.
    * 
    * Atomic operation. 
    */
   int16_t *(*getsrc)(
      int   bytes,                     /**< (i) size of the buffer in bytes */
      void *privdata                   /**< (i) user supplied data */
   );

   /* This callback indicates when the Audio Mixer is done with the 
    * source data for one audio frame. It is only called once per frame 
    * period and is thus typically used for timing and buffer management.
    * 
    * Atomic operation. 
    */
   void  (*srcdone)(
      int   bytes,                     /**< (i) size of the buffer in bytes */
      void *privdata                   /**< (i) user supplied data */
   );

   /* This callback returns a pointer to the destination audio buffer
    * for one frame period and takes a frame size in bytes as a parameter.
    * It may be called more than once in a frame period with the same or
    * different frame size. The callback may return a valid pointer or a
    * NULL pointer if the frame size is invalid. If a NULL pointer is
    * returned, the Audio Mixer will not make use of the pointer. 
    *
    * Atomic operation. 
    */
   int16_t *(*getdst)(
      int   bytes,                     /**< (i) size of the buffer in bytes */
      void *privdata                   /**< (i) user supplied data */
   );

   /* This callback indicates when the Audio Mixer is done with the 
    * destination buffer for one audio frame. It is only called once per
    * frame period and is typically used for timing and buffer management.
    * 
    * Atomic operation. 
    */
   void  (*dstdone)(
      int   bytes,                     /**< (i) size of the buffer in bytes */
      void *privdata                   /**< (i) user supplied data */
   );

   /* This callback is indicates when the last destination connection to
    * the port is removed. This callback may be useful to trigger a flush
    * of a port’s destination buffers.
    */
   void  (*dstcnxsremoved)(
      void *privdata                   /**< (i) user supplied data */
   );
}
AMXR_PORT_CB;

/***************************************************************************/
/**
*  Generic resampler. Provided an appropriate filter is designed,
*  this resampler can accommodate any resampling ratio of
*  interpfac/decimfac.
*/
typedef void (*AMXR_RESAMP_FNC)(
   int16_t       *insamp,           /**< (i) Ptr to input samples */
   int16_t       *outsamp,          /**< (o) Ptr to output samples */
   int16_t        numsamp,          /**< (i) Number of samples to generate */
   const int16_t *filtcoeff,        /**< (i) Ptr to filter coefficients */
   int            filtlen,          /**< (i) Filter length */
   int            interpfac,        /**< (i) Interpolation factor */
   int            decimfac          /**< (i) Decimation factor */
);

typedef void (*AMXR_ADD_FNC)(
   int16_t       *dstp,             /**< (o) Ptr to vector sum */
   const int16_t *src1p,            /**< (i) Ptr to vector summand 1 */
   const int16_t *src2p,            /**< (i) Ptr to vector summand 2 */
   int            numsamp           /**< (i) Number of samples to add */
);

typedef void (*AMXR_MPYQ16_FNC)(
   int16_t       *dstp,             /**< (o) Ptr to output samples */
   const int16_t *srcp,             /**< (i) Ptr to input samples */
   int            numsamp,          /**< (i) Number of samples to add */
   uint16_t       q16gain           /**< (i) Q16 linear gain value to multiply with */
);

typedef void (*AMXR_MACQ16_FNC)(
   int16_t       *dstp,             /**< (o) Ptr to output samples */
   const int16_t *srcp,             /**< (i) Ptr to input samples */
   int            numsamp,          /**< (i) Number of samples to add */
   uint16_t       q16gain           /**< (i) Q16 linear gain value to multiply with */
);

/**
*  Structure containing function pointer types for the Audio Mixer API. 
*  This structure is used to install a proprietary implementation of the
*  Audio Mixer.
*
*  The function pointer type must match the parameters of the actual API.
*  Refer to the API function prototypes for the explanation of the 
*  parameters and description of the API.
*/
typedef struct amxr_api_funcs
{
   int (*getPortInfo)( AMXR_HDL, AMXR_PORT_ID, AMXR_PORT_INFO * );
   int (*getInfo)( AMXR_HDL, AMXR_INFO * );
   int (*setCnxLoss)( AMXR_HDL, AMXR_PORT_ID, AMXR_PORT_ID, AMXR_CONNECT_DESC, unsigned int );
   int (*getCnxLoss)( AMXR_HDL, AMXR_PORT_ID, AMXR_PORT_ID, AMXR_CONNECT_DESC, unsigned int * );
   int (*connect)( AMXR_HDL, AMXR_PORT_ID, AMXR_PORT_ID, AMXR_CONNECT_DESC );
   int (*disconnect)( AMXR_HDL, AMXR_PORT_ID, AMXR_PORT_ID );
   int (*getCnxListBySrc)( AMXR_HDL, AMXR_PORT_ID, AMXR_CNXS *, int );
   int (*getCnxListByDst)( AMXR_HDL, AMXR_PORT_ID, AMXR_CNXS *, int );

   /* Kernel only API */
   int (*createPort)( const char *, AMXR_PORT_CB *, void *, int, int, int, int, int, int, AMXR_PORT_ID * );
   int (*removePort)( AMXR_PORT_ID );
   int (*setPortDstFreq)( AMXR_PORT_ID, int, int );
   int (*setPortSrcFreq)( AMXR_PORT_ID, int, int );
   int (*setPortDstChannels)( AMXR_PORT_ID, int, int );
   int (*setPortSrcChannels)( AMXR_PORT_ID, int, int );
   void (*serviceUnsyncPort)( AMXR_PORT_ID );
}
AMXR_API_FUNCS;

/* MakeDefs: On */

#endif   /* __KERNEL__ */

/* ---- Variable Externs ------------------------------------------------- */

/* ---- Function Prototypes ---------------------------------------------- */

#ifdef __cplusplus
extern "C"
{
#endif

#if !defined( SWIG ) && !defined( MAKEDEFS )

/* Forward prototypes */
int amxrDisconnect( AMXR_HDL hdl, AMXR_PORT_ID src_port, AMXR_PORT_ID dst_port );

/***************************************************************************/
/**
*  Allocates a client handle to allow the client to make use of the
*  audio mixer resources.
*
*  @return
*     >= 0        On success, valid client handle
*     -ve         On general failure
*/
AMXR_HDL amxrAllocateClient( void );

/***************************************************************************/
/**
*  Frees client handle and performs related cleanup. However, it is 
*  responsibility of the application to delete connections it created 
*  because they are not automatically deleted by this API call.
*
*  @return
*     0           On success
*     -ve         On general failure
*/
int amxrFreeClient(
   AMXR_HDL hdl                     /**< (i) client handle */
);

/***************************************************************************/
/**
*  Queries for port ID by name string.
*
*  @return
*     0        Found matching port
*     -EINVAL  Port not found
*     -ve      Other errors
*/
int amxrQueryPortByName(
   AMXR_HDL       hdl,              /**< (i) Mixer client handle */
   const char    *name,             /**< (i) Name of port */
   AMXR_PORT_ID  *id                /**< (o) Ptr to store found port ID */
);

/***************************************************************************/
/**
*  Obtain audio mixer port information
*
*  @return
*     0     On success
*     -ve   Failure code
*/
int amxrGetPortInfo(
   AMXR_HDL        hdl,             /**< (i) Mixer client handle */
   AMXR_PORT_ID    port,            /**< (i) Port id */
   AMXR_PORT_INFO *info             /**< (o) Ptr to port info structure */
);

/***************************************************************************/
/**
*  Obtain audio mixer port information by name
*
*  @return
*     0     On success
*     -ve   Failure code
*/
static inline int amxrGetPortInfoByName(
   AMXR_HDL          hdl,           /**< (i) Mixer client handle */
   const char       *name,          /**< (i) Port name string */
   AMXR_PORT_INFO   *info           /**< (o) Ptr to port info structure */
)
{
   AMXR_PORT_ID port;
   int            err;
   err = amxrQueryPortByName( hdl, name, &port );
   if ( err )
   {
      return err;
   }
   return amxrGetPortInfo( hdl, port, info );
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
int amxrGetInfo(
   AMXR_HDL     hdl,                /**< (i) Mixer client handle */
   AMXR_INFO   *info                /**< (o) Ptr to info structure */
);

/***************************************************************************/
/**
*  Set connection loss to attenuate samples between source and destination
*  ports. Loss range is from 0 db to mute in 1db steps.
*
*  @return
*     0           On success
*     -EINVAL     No such connection found
*     -ve         Other errors
*/
int amxrSetCnxLoss(
   AMXR_HDL             hdl,        /**< (i) client handle */
   AMXR_PORT_ID         src_port,   /**< (i) source port id */
   AMXR_PORT_ID         dst_port,   /**< (i) destination port id */
   AMXR_CONNECT_DESC    desc,       /**< (i) Connection descriptor */
   unsigned int         db          /**< (i) Loss amount in dB */
);

/***************************************************************************/
/**
*  Set connection loss by name string
*
*  @return
*     0           On success
*     -EINVAL     No such connection found
*     -ve         Other errors
*/
static inline int amxrSetCnxLossByName(
   AMXR_HDL             hdl,        /**< (i) client handle */
   const char          *src_name,   /**< (i) source port name */
   const char          *dst_name,   /**< (i) destination port name */
   AMXR_CONNECT_DESC    desc,       /**< (i) Connection descriptor */
   unsigned int         db          /**< (i) Loss amount in dB */
)
{
   AMXR_PORT_ID src_port, dst_port;
   int            err;
   err = amxrQueryPortByName( hdl, src_name, &src_port );
   if ( err )
   {
      return err;
   }
   err = amxrQueryPortByName( hdl, dst_name, &dst_port );
   if ( err )
   {
      return err;
   }
   return amxrSetCnxLoss( hdl, src_port, dst_port, desc, db );
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
int amxrGetCnxLoss(
   AMXR_HDL             hdl,        /**< (i) client handle */
   AMXR_PORT_ID         src_port,   /**< (i) source port id */
   AMXR_PORT_ID         dst_port,   /**< (i) destination port id */
   AMXR_CONNECT_DESC    desc,       /**< (i) Connection descriptor */
   unsigned int        *db          /**< (o) Pointer to store attenuation amount */
);

/***************************************************************************/
/**
*  Read connection loss amount in dB by name string.
*
*  @return
*     0           On success
*     -EINVAL     No such connection found
*     -ve         Other errors
*/
static inline int amxrGetCnxLossByName(
   AMXR_HDL             hdl,        /**< (i) client handle */
   const char          *src_name,   /**< (i) source port name */
   const char          *dst_name,   /**< (i) destination port name */
   AMXR_CONNECT_DESC    desc,       /**< (i) Connection descriptor */
   unsigned int        *db          /**< (o) Pointer to store attenuation amount */
)
{
   AMXR_PORT_ID src_port, dst_port;
   int            err;
   err = amxrQueryPortByName( hdl, src_name, &src_port );
   if ( err )
   {
      return err;
   }
   err = amxrQueryPortByName( hdl, dst_name, &dst_port );
   if ( err )
   {
      return err;
   }
   return amxrGetCnxLoss( hdl, src_port, dst_port, desc, db );
}

/***************************************************************************/
/**
*  Make a simplex connection from source to destination ports.
*
*  @return
*     0           On success
*     -ve         On general failure
*/
int amxrConnect(
   AMXR_HDL             hdl,        /**< (i) client handle */
   AMXR_PORT_ID         src_port,   /**< (i) source port id */
   AMXR_PORT_ID         dst_port,   /**< (i) destination port id */
   AMXR_CONNECT_DESC    desc        /**< (i) Connection descriptor */
);

/***************************************************************************/
/**
*  Make a simplex connection from source to destination ports by name.
*
*  @return
*     0           On success
*     -ve         On general failure
*/
static inline int amxrConnectByName(
   AMXR_HDL             hdl,        /**< (i) client handle */
   const char          *src_name,   /**< (i) source port name */
   const char          *dst_name,   /**< (i) destination port name */
   AMXR_CONNECT_DESC    desc        /**< (i) Connection descriptor */
)
{
   AMXR_PORT_ID src_port, dst_port;
   int            err;
   err = amxrQueryPortByName( hdl, src_name, &src_port );
   if ( err )
   {
      return err;
   }
   err = amxrQueryPortByName( hdl, dst_name, &dst_port );
   if ( err )
   {
      return err;
   }
   return amxrConnect( hdl, src_port, dst_port, desc );
}

/***************************************************************************/
/**
*  Make a duplex connection between source and destination ports.
*
*  @return
*     0           On success
*     -ve         On general failure
*/
static inline int amxrConnectAll(
   AMXR_HDL             hdl,        /**< (i) Mixer client handle */
   AMXR_PORT_ID         port1,      /**< (i) First port id */
   AMXR_PORT_ID         port2,      /**< (i) Second port id */
   AMXR_CONNECT_DESC    desc        /**< (i) Connection descriptor */
)
{
   int err;
   err = amxrConnect( hdl, port1, port2, desc );
   if ( !err )
   {
      AMXR_CONNECT_TYPE cnx_type;
      int               src_chans, src_idx, dst_chans, dst_idx;

      /* Invert descriptor */
      cnx_type  = AMXR_CONNECT_GET_TYPE( desc );
      src_chans = AMXR_CONNECT_GET_SRC_CHANS( desc );
      src_idx   = AMXR_CONNECT_GET_SRC_IDX( desc );
      dst_chans = AMXR_CONNECT_GET_DST_CHANS( desc );
      dst_idx   = AMXR_CONNECT_GET_DST_IDX( desc );

      if ( cnx_type == AMXR_CONNECT_MULTI_TO_MONO_CONVERT )
      {
         cnx_type = AMXR_CONNECT_MONO_TO_MULT_DUPLICATE;
      }
      else if ( cnx_type == AMXR_CONNECT_MONO_TO_MULT_DUPLICATE )
      {
         cnx_type = AMXR_CONNECT_MULTI_TO_MONO_CONVERT;
      }

      desc = AMXR_CREATE_DESC( cnx_type, dst_chans, dst_idx, src_chans, src_idx );

      err = amxrConnect( hdl, port2, port1, desc );
      if ( err )
      {
         /* Disconnect first simplex connection */
         amxrDisconnect( hdl, port1, port2 );
      }
   }
   return err;
}

/***************************************************************************/
/**
*  Make a duplex connection between source and destination ports by name.
*
*  @return
*     0           On success
*     -ve         On general failure
*/
static inline int amxrConnectAllByName(
   AMXR_HDL             hdl,        /**< (i) Mixer client handle */
   const char          *port1_name, /**< (i) First port name */
   const char          *port2_name, /**< (i) Second port name */
   AMXR_CONNECT_DESC    desc        /**< (i) Connection descriptor */
)
{
   AMXR_PORT_ID port1, port2;
   int            err;
   err = amxrQueryPortByName( hdl, port1_name, &port1 );
   if ( err )
   {
      return err;
   }
   err = amxrQueryPortByName( hdl, port2_name, &port2 );
   if ( err )
   {
      return err;
   }
   return amxrConnectAll( hdl, port1, port2, desc );
}

/***************************************************************************/
/**
*  Remove simplex connections between source and destination ports.
*
*  @return
*     0           On success
*     -ve         On general failure
*/
int amxrDisconnect(
   AMXR_HDL             hdl,        /**< (i) Mixer client handle */
   AMXR_PORT_ID         src_port,   /**< (i) source port id */
   AMXR_PORT_ID         dst_port    /**< (i) destination port id */
);

/***************************************************************************/
/**
*  Remove simplex connections between source and destination ports
*  by name.
*
*  @return
*     0           On success
*     -ve         On general failure
*/
static inline int amxrDisconnectByName(
   AMXR_HDL             hdl,        /**< (i) Mixer client handle */
   const char          *src_name,   /**< (i) source port name */
   const char          *dst_name    /**< (i) destination port name */
)
{
   AMXR_PORT_ID src_port, dst_port;
   int            err;
   err = amxrQueryPortByName( hdl, src_name, &src_port );
   if ( err )
   {
      return err;
   }
   err = amxrQueryPortByName( hdl, dst_name, &dst_port );
   if ( err )
   {
      return err;
   }
   return amxrDisconnect( hdl, src_port, dst_port );
}

/***************************************************************************/
/**
*  Remove all connections between source and destination ports.
*
*  @return
*     0           On success
*     -ve         On general failure
*/
static inline int amxrDisconnectAll(
   AMXR_HDL             hdl,        /**< (i) Mixer client handle */
   AMXR_PORT_ID         port1,      /**< (i) First port id */
   AMXR_PORT_ID         port2       /**< (i) Second port id */
)
{
   int err;
   err = amxrDisconnect( hdl, port1, port2 );
   if ( !err )
   {
      err = amxrDisconnect( hdl, port2, port1 );
   }
   return err;
}

/***************************************************************************/
/**
*  Remove all connections between source and destination ports by name
*
*  @return
*     0           On success
*     -ve         On general failure
*/
static inline int amxrDisconnectAllByName(
   AMXR_HDL             hdl,        /**< (i) Mixer client handle */
   const char          *port1_name, /**< (i) First port name */
   const char          *port2_name  /**< (i) Second port name */
)
{
   AMXR_PORT_ID port1, port2;
   int            err;
   err = amxrQueryPortByName( hdl, port1_name, &port1 );
   if ( err )
   {
      return err;
   }
   err = amxrQueryPortByName( hdl, port2_name, &port2 );
   if ( err )
   {
      return err;
   }
   return amxrDisconnectAll( hdl, port1, port2 );
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
int amxrGetCnxListBySrc(
   AMXR_HDL             hdl,        /**< (i) Mixer client handle */
   AMXR_PORT_ID         src_port,   /**< (i) Source port */
   AMXR_CNXS           *cnxlist,    /**< (o) Ptr to store cnx list */
   int                  maxlen      /**< (i) Max length in bytes */
);

/***************************************************************************/
/**
*  Query the connection list by source port that is specified by name
*  string. All destination ports connected to the specified source port 
*  will be returned.
*
*  @return
*     0           On success
*     -ve         On general failure
*/
static inline int amxrGetCnxListBySrcByName(
   AMXR_HDL             hdl,        /**< (i) Mixer client handle */
   const char          *port_name,  /**< (i) Source port name */
   AMXR_CNXS           *cnxlist,    /**< (o) Ptr to store cnx list */
   int                  maxlen      /**< (i) Max length in bytes */
)
{
   AMXR_PORT_ID port;
   int          err;
   err = amxrQueryPortByName( hdl, port_name, &port );
   if ( err )
   {
      return err;
   }
   return amxrGetCnxListBySrc( hdl, port, cnxlist, maxlen );
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
int amxrGetCnxListByDst(
   AMXR_HDL             hdl,        /**< (i) Mixer client handle */
   AMXR_PORT_ID         dst_port,   /**< (i) Dst port */
   AMXR_CNXS           *cnxlist,    /**< (o) Ptr to store cnx list */
   int                  maxlen      /**< (i) Max length in bytes */
);

/***************************************************************************/
/**
*  Query the connection list by destination port that is specified by 
*  name string. All source ports connected to the specified destination 
*  port will be returned.
*
*  @return
*     0           On success
*     -ve         On general failure
*/
static inline int amxrGetCnxListByDstByName(
   AMXR_HDL             hdl,        /**< (i) Mixer client handle */
   const char          *port_name,  /**< (i) Dst port name */
   AMXR_CNXS           *cnxlist,    /**< (o) Ptr to store cnx list */
   int                  maxlen      /**< (i) Max length in bytes */
)
{
   AMXR_PORT_ID port;
   int          err;
   err = amxrQueryPortByName( hdl, port_name, &port );
   if ( err )
   {
      return err;
   }
   return amxrGetCnxListByDst( hdl, port, cnxlist, maxlen );
}

#ifdef __cplusplus
}
#endif

#if defined( __KERNEL__ )

/***************************************************************************/
/**
*  Queries for name string by Port ID
*
*  @return
*     0        Found matching port
*     -EINVAL  Port not found
*     -ve      Other errors
*/
int amxrQueryNameById(
   AMXR_PORT_ID    id,              /**< (i) Port ID */
   const char    **name             /**< (o) Name of port */
);

/***************************************************************************/
/**
*  Create a mixer port.
*
*  @return
*     0        On success
*     -ve      Error code
*/
int amxrCreatePort(
   const char       *name,          /**< (i) Name string */
   AMXR_PORT_CB     *cb,            /**< (i) Callbacks */
   void             *privdata,      /**< (i) Private data passed back to callbacks */
   int               dst_hz,        /**< (i) Current destination sampling frequency in Hz */
   int               dst_chans,     /**< (i) Number of channels, i.e. mono = 1 */
   int               dst_bytes,     /*<< (i) Destination period size in bytes */
   int               src_hz,        /**< (i) Current source sampling frequency in Hz */
   int               src_chans,     /**< (i) Number of channels, i.e. mono = 1 */
   int               src_bytes,     /*<< (i) Source period size in bytes */
   AMXR_PORT_ID     *portidp        /**< (o) Ptr to port ID */
);

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
int amxrRemovePort(
   AMXR_PORT_ID port                /**< (i) Port to remove */
);

/***************************************************************************/
/**
*  Set destination port sampling frequency. Connections using the
*  port are also updated accordingly.
*
*  @return
*     0           On success
*     -ve         On general failure
*/
int amxrSetPortDstFreq(
   AMXR_PORT_ID   portid,           /**< (i) Destination port id */
   int            dst_hz,           /*<< (i) Destination sampling frequency in Hz */
   int            dst_bytes         /*<< (i) Destination period size in bytes */
);

/***************************************************************************/
/**
*  Set source port sampling frequency. Connections using the port
*  are also updated accordingly.
*
*  @return
*     0           On success
*     -ve         On general failure
*/
int amxrSetPortSrcFreq(
   AMXR_PORT_ID   portid,           /**< (i) Source port id */
   int            src_hz,           /*<< (i) Source sampling frequency in Hz */
   int            src_bytes         /*<< (i) Source period size in bytes */
);

/***************************************************************************/
/**
*  Set port sampling frequencies. Connections using the port
*  are also updated accordingly.
*
*  @return
*     0           On success
*     -ve         On general failure
*
*  @remarks
*     Source and destination sampling frequencies may be asymmetric.
*/
static inline int amxrSetPortFreq(
   AMXR_PORT_ID   portid,           /**< (i) port id */
   int            dst_hz,           /**< (i) Destination sampling frequency in Hz */
   int            dst_bytes,        /*<< (i) Destination period size in bytes */
   int            src_hz,           /**< (i) Source sampling frequency in Hz */
   int            src_bytes         /*<< (i) Source period size in bytes */
)
{
   int err;
   err = amxrSetPortDstFreq( portid, dst_hz, dst_bytes );
   if ( !err )
   {
      err = amxrSetPortSrcFreq( portid, src_hz, src_bytes );
   }
   return err;
}

/***************************************************************************/
/**
*  Set the channel configuration for a destination port.
*  Connections using the port are updated, where appropriate.
*
*  @return
*     0           On success
*     -ve         On general failure
*
*  @remarks    Not all existing connections with this source port
*              will be maintained after changing the number of channels.
*/
int amxrSetPortDstChannels(
   AMXR_PORT_ID   portid,           /**< (i) Destination port id */
   int            dst_chans,        /*<< (i) Number of channels: 1 for mono, 2 for stereo, etc */
   int            dst_bytes         /*<< (i) Destination period size in bytes */
);

/***************************************************************************/
/**
*  Set the channel configuration for a source port.
*  Connections using the port are updated, where appropriate.
*
*  @return
*     0           On success
*     -ve         On general failure
*
*  @remarks    Not all existing connections with this source port
*              will be maintained after changing the number of channels.
*/
int amxrSetPortSrcChannels(
   AMXR_PORT_ID   portid,           /**< (i) Source port id */
   int            src_chans,        /*<< (i) Number of src channels: 1 for mono, 2 for stereo, etc. */
   int            src_bytes         /*<< (i) Source period size in bytes */
);

/***************************************************************************/
/**
*  This routine executes the mixer. The time elapsed since the last
*  execution trigger is passed to the mixer.
*
*  @remarks
*     Typically, this routine is called by HAL Audio, which is clocked
*     by hardware.
*
*     This routine runs in an ATOMIC context!
*/
void amxrElapsedTime(
   int elapsed_usec                 /**< (i) Amount of time elapsed since last call in usec */
);

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
void amxrServiceUnsyncPort( 
   AMXR_PORT_ID id                  /*<< (i) Port ID of port to service */
);

/***************************************************************************/
/**
*  Setup proprietary Audio Mixer implementation
*
*  @return    none
*/
void amxrSetApiFuncs( 
   const AMXR_API_FUNCS *funcs      /**< (i) Ptr to API functions. 
                                             If NULL, clears registration */
);

#endif   /* __KERNEL__ */

#endif   /* SWIG */

#endif   /* AMXR_H */


