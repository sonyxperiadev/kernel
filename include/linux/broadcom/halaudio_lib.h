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
*  @file    halaudio_lib.h
*
*  @brief   HAL Audio kernel library definitions
*
*****************************************************************************/
#if !defined( HALAUDIO_LIB_H )
#define HALAUDIO_LIB_H

/* ---- Include Files ---------------------------------------------------- */

#include <linux/types.h>                  /* Needed for standard types */
#include <linux/broadcom/halaudio.h>      /* HAL Audio API */


/* ---- Constants and Types ---------------------------------------------- */
#if defined( __KERNEL__ )

/* The maximum number of input samples that can be processed by
 * the software equalizer at one time.
 */
#define HALAUDIO_SWEQU_MAX_SAMPLES  480   /* Max 48 KHz stereo */

typedef struct halaudio_sinectl
{
   int                  freq;             /* fc in Hz */
   uint16_t             phase;            /* current phase */
   uint16_t             phaseDelta;       /* delta to next phase */
}
HALAUDIO_SINECTL;

typedef struct halaudio_rw_buf
{
   int                  buf_allocsz;      /* Allocated buffer size */
   char                *bufp;             /* Pointer to alloc'd memory byte buffer */
   char                *posp;             /* Current position pointer into buffer */
   int                  tot_bytes;        /* Total data stored in buffer */
   int                  remain_bytes;     /* Remainder bytes to process */
}
HALAUDIO_RW_BUF;

typedef struct halaudio_write
{
   int                  request;          /* Write request */
   unsigned int         flush_count;      /* Number of flush frames following write */
   HALAUDIO_RW_BUF      buf[2];           /* Double buffer */
   int                  curr_bufidx;      /* Current buffer index */
   HALAUDIO_CODEC_IORW_CB usercb;         /* User callback( samples, data ) */
   void                *userdata;         /* User data */
}
HALAUDIO_WRITE;

#endif

/* ---- Variable Externs ------------------------------------------ */
#if defined( __KERNEL__ )
#endif

/* ---- Function Prototypes --------------------------------------- */

#if defined( __KERNEL__ )

/***************************************************************************/
/**
*  Helper routine that implements the buffer write.
*
*  @return  Nothing.
*
*  @remarks
*     This routine expects to run in an atomic context and is not
*     thread safe.
*/
void __halAudioWriteWorker(
   HALAUDIO_WRITE *writestp,     /*<< (io) write state pointer */
   int16_t        *bufp,         /*<< (o)  output buffer pointer */
   int             frame_size    /*<< (i)  frame size in bytes */
);

/***************************************************************************/
/**
*  Helper routine to service any write request. This version of the
*  write should be called by drivers in their ISRs to implement
*  the write functionality.
*
*  @return  Nothing.
*
*  @remarks
*     This routine expects to run in an atomic context and is not
*     thread safe.
*/
static inline void halAudioWriteService(
   HALAUDIO_WRITE *writestp,     /*<< (io) write state pointer */
   int16_t        *bufp,         /*<< (o)  output buffer pointer */
   int             frame_size    /*<< (i)  frame size in bytes */
)
{
   if ( writestp->request )
   {
      __halAudioWriteWorker( writestp, bufp, frame_size );
   }
}

/***************************************************************************/
/**
*  Helper routine used to make write requests. This routine is intended
*  to be called in a HAL Audio driver's write callback.
*
*  @return  0 for success, otherwise -ve error code.
*/
int halAudioWriteRequest(
   HALAUDIO_WRITE         *writestp,   /*<< (io) write state pointer */
   int                     bytes,      /**< (i)  Number of bytes to write */
   const char             *audiobuf,   /**< (i)  Pointer to audio samples */
   HALAUDIO_CODEC_IORW_CB  usercb,     /**< (i)  User callback to request for more data */
   void                   *userdata    /**< (i)  User data */
);

/***************************************************************************/
/**
*  Helper routine used to flush write buffers.
*
*  @return  0 for success, otherwise -ve error code.
*/
int halAudioWriteFlush(
   HALAUDIO_WRITE *writestp      /*<< (io) write state pointer */
);

/***************************************************************************/
/**
*  Helper routine used to cleanup write resources. This routine is intended
*  to be called in a HAL Audio driver's exit callback.
*
*  @return  0 for success, otherwise -ve error code.
*/
int halAudioWriteFree(
   HALAUDIO_WRITE *writestp      /*<< (io) write state pointer */
);

/***************************************************************************/
/**
*  Ramp generation. Data is duplicated for stereo and higher order
*  interleaved channels
*
*  @return  None
*/
void halAudioGenerateRamp(
   uint16_t    *bufp,            /**< (o)  Ptr to output buffer */
   uint16_t    *rampseed,        /**< (io) Ptr to ramp seed */
   int          length,          /**< (i)  Number of samples to generate */
   int          chans            /**< (i)  Number of interleave channels: 1 for
                                           mono, 2 for stereo, etc.*/
);

/***************************************************************************/
/**
*  Compare to buffers of samples, one delayed relative to other by a
*  fixed constant.
*
*  @return  None
*/
void halAudioCompareData( 
   const uint16_t *bufp,   /**< (i)  Pointer to samples to check (rx buffer) */
   const uint16_t *refp,   /**< (i)  Pointer to reference samples (tx buffer) */
   int             length, /**< (i)  Number of samples */
   int            *errs,   /**< (o)  Pointer to number of errors */
   int            *delta   /**< (io) Pointer to delta constant between tx and rx samples */
);

/***************************************************************************/
/**
*  Configure the sine generation state
*
*  @return  None
*/
void halAudioSineConfig(
   HALAUDIO_SINECTL  *cblkp,     /**< (o)  Ptr to sine generation control block */
   int                freq,      /**< (i)  Carrier frequency */
   int                samp_freq  /**< (i)  Sampling frequency */
);

/***************************************************************************/
/**
*  Integer math sine wave approximation
*
*  @return  16-bit sample
*/
void halAudioSine(
   int16_t           *dstp,      /**< (o)  Output pointer */
   HALAUDIO_SINECTL  *cblkp,     /**< (io) Ptr to sine generation control block */
   int               samples,    /**< (i)  Number of samples to generate */
   int               chans       /**< (i)  Number of interleave channels: 1 for
                                           mono, 2 for stereo, etc.*/
);

/***************************************************************************/
/**
*  Helper routine to format memory dump strings
*
*  @return  Nothing.
*
*  @remarks Intended to be used in procfs methods to display memory
*           contents.
*/
void halAudioPrintMemory(
   char          *str,              /**< (o)  Ptr to output string buffer */
   int           *offset,           /**< (io) Ptr to length offset into string */
   void          *datap,            /**< (i)  Ptr to memory to dump out */
   int            max_length,       /**< (i)  Maximum number of words to dump out */
   int            hexformat,        /**< (i)  1 to output hex, 0 for integer */
   int            line_length,      /**< (i)  Number of words per line */
   int            word_width,       /**< (i)  Word width in bytes, supports 1, 2 and 4 */
   int            print_addr,       /**< (i)  1 to enable address prefix, otherwise 0 */
   unsigned long  start_addr        /**< (i)  If address prefix request, start with following address */
);

/***************************************************************************/
/**
*  Software Equalizer Function
*
*  @return  Nothing
*/
void halAudioEquProcess(
   int16_t *samp,                   /**< (io) input and output samples */
   int32_t *coeff,                  /**< (i) pointer filter coefficients */
   int16_t *histp,                  /**< (i) pointer to history */
   int order,                       /**< (i) filter order */
   int len                          /**< (i) number of samples to process */
);

#endif   /* __KERNEL__ */

#endif /* HALAUDIO_LIB_H */
