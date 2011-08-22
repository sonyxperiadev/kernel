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
*  @file    halaudio_lib.c
*
*  @brief   This file implements the HAL Audio utlity library
*
*****************************************************************************/

/* ---- Include Files ---------------------------------------------------- */

#include <linux/types.h>                     /* For stdint types: uint8_t, etc. */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/string.h>
#include <linux/vmalloc.h>                   /* For memory alloc */
#include <linux/semaphore.h>                 /* For down_interruptible, up, etc. */
#include <linux/broadcom/halaudio.h>         /* HAL Audio API */
#include <linux/broadcom/halaudio_lib.h>     /* HAL Audio common library API */
#include <asm/atomic.h>                      /* Atomic operations */

/* ---- Public Variables ------------------------------------------------- */

/* ---- Private Constants and Types -------------------------------------- */

/* ---- Private Variables ------------------------------------------------ */

/* ---- Private Function Prototypes -------------------------------------- */

/* ---- Functions -------------------------------------------------------- */

/***************************************************************************/
/**
*  Helper routine to format memory dump strings
*
*  @return  Nothing.
*
*  @remarks Called by proc routine.
*/
void halAudioPrintMemory(
   char          *str,              /*<< (o)  Ptr to output string buffer */
   int           *offset,           /*<< (io) Ptr to length offset into string */
   void          *datap,            /*<< (i)  Ptr to memory to dump out */
   int            max_length,       /*<< (i)  Maximum number of words to dump out */
   int            hexformat,        /*<< (i)  1 to output hex, 0 for integer */
   int            line_length,      /*<< (i)  Number of words per line */
   int            word_width,       /*<< (i)  Word width in bytes, supports 1, 2 and 4 */
   int            print_addr,       /*<< (i)  1 to enable address prefix, otherwise 0 */
   unsigned long  start_addr        /*<< (i)  If address prefix request, start with following address */
)
{
   int       i , j;
   uint8_t  *data8p;
   uint16_t *data16p;
   uint32_t *data32p;

   i        = 0;
   data8p   = datap;
   data16p  = datap;
   data32p  = datap;

   while ( max_length > 0 )
   {
      if ( print_addr )
      {
         *offset    += sprintf( str + *offset, "%.8lx:", start_addr );
         start_addr += word_width * line_length;
      }

      for ( j = 0; (j < line_length) && max_length; j++, max_length--, i++ )
      {
         if ( word_width == 1 )
         {
            /* 8-bit words */
            if ( hexformat )
            {
               *offset += sprintf( str + *offset, " %.2hx", data8p[i] );
            }
            else
            {
               *offset += sprintf( str + *offset, " %hd", data8p[i] );
            }
         }
         else if ( word_width == 2 )
         {
            /* 16-bit words */
            if ( hexformat )
            {
               *offset += sprintf( str + *offset, " %.4hx", data16p[i] );
            }
            else
            {
               *offset += sprintf( str + *offset, " %hd", data16p[i] );
            }
         }
         else
         {
            /* 32-bit words */
            if ( hexformat )
            {
               *offset += sprintf( str + *offset, " %.8x", data32p[i] );
            }
            else
            {
               *offset += sprintf( str + *offset, " %d", data32p[i] );
            }
         }
      }

      *offset += sprintf( str + *offset, "\n" );
   }
}
EXPORT_SYMBOL( halAudioPrintMemory );

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
)
{
   int      i, j;
   uint16_t dataval;

   for ( i = 0; i < length; i += chans )
   {
      dataval = (*rampseed)++;
      for ( j = 0; j < chans; j++ )
      {
         *bufp = dataval;
         bufp++;
      }
   }
}
EXPORT_SYMBOL( halAudioGenerateRamp );

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
)
{
   int i;
   for ( i = 0; i < length; i++ )
   {
      uint16_t expected = refp[i] - *delta;
      if ( bufp[i] != expected )
      {
         *delta = refp[i] - bufp[i];   /* reset delta seen */
         (*errs)++;
      }
   }
}
EXPORT_SYMBOL( halAudioCompareData );

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
)
{
   cblkp->freq       = freq;
   cblkp->phase      = 0;
   if ( samp_freq )
   {
      cblkp->phaseDelta = (cblkp->freq << 16) / samp_freq;
   }
   else
   {
      cblkp->phaseDelta = 0;
   }
}
EXPORT_SYMBOL( halAudioSineConfig );

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
)
{
   int p, a, b, c, d, phase;
   int i, j;

   for ( i = 0; i < samples; i += chans )
   {
      int16_t acc;

      phase = cblkp->phase;
      if ( phase < 0x4000 )
      {
         p = ~phase;
      }
      else if ( phase < 0x8000 )
      {
         p = phase;
      }
      else if ( phase < 0xC000 )
      {
         p = ~phase;
      }
      else
      {
         p = phase;
      }

      p = p & 0x3fff;
      a = (p * p) >> 14;
      b = 8058 - ((a * 537) >> 14);
      c = 40289 - ((a * b) >> 14);
      d = 32767 - ((a * c) >> 14);

      if ( phase > 0x8000 )
      {
         d = -d;
      }

      cblkp->phase += cblkp->phaseDelta;

      acc = (int16_t)( d >> 1 );   /* signal scaled down by 2 */

      for ( j = 0; j < chans; j++ )
      {
         *dstp = acc;
         dstp++;
      }
   }
}
EXPORT_SYMBOL( halAudioSine );

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
)
{
   HALAUDIO_RW_BUF  *curbuf, *otherbuf, *tmpbuf;
   int               bytes_written, len;
   int8_t           *datap;

   bytes_written  = 0;
   curbuf         = &writestp->buf[writestp->curr_bufidx];
   otherbuf       = &writestp->buf[writestp->curr_bufidx ^ 1];
   datap          = (int8_t *)bufp;

   do 
   {
      len = frame_size - bytes_written; 
      if ( curbuf->remain_bytes < len )
      {
         len = curbuf->remain_bytes;
      }

      memcpy( datap, curbuf->posp, len );

      curbuf->posp         += len;
      datap                += len;
      curbuf->remain_bytes -= len;
      bytes_written        += len;

      /* Low on samples, ask for more */
      if ( writestp->usercb && otherbuf->remain_bytes == 0 )
      {
         writestp->usercb( curbuf->tot_bytes, writestp->userdata );
         writestp->usercb = NULL; /* Clear callback to limit calling to once */
      }

      /* Buffer depleted, switch to next buffer. */
      if ( curbuf->remain_bytes == 0 )
      {
         tmpbuf   = curbuf;      /* Swap double buffers */
         curbuf   = otherbuf;
         otherbuf = tmpbuf; 

         writestp->curr_bufidx ^= 1;
      }
   }
   while ( bytes_written < frame_size && curbuf->remain_bytes > 0 );

   /* Flush partial and full frames after writestp complete */
   if ( bytes_written < frame_size && writestp->flush_count )
   {
      memset( datap, 0, frame_size - bytes_written );
      writestp->flush_count--;

      if ( writestp->flush_count == 0 )
      {
         writestp->request = 0;
      }
   }
}
EXPORT_SYMBOL( __halAudioWriteWorker );

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
)
{
   unsigned long  flags;
   void          *free_bufp;

   if ( writestp->usercb )
   {
      return -EBUSY;
   }

   free_bufp = NULL;

   /* Entering critical section */
   local_irq_save( flags );
   {
      HALAUDIO_RW_BUF *idlebuf;
      int8_t          *newbufp;

      idlebuf = &writestp->buf[writestp->curr_bufidx];
      if ( idlebuf->remain_bytes )
      {
         /* There is data in this buffer, fill the other buffer */
         idlebuf = &writestp->buf[writestp->curr_bufidx ^ 1];
      }
      else
      {
         /* This is the first active buffer (or it underflowed) */
      }

      if ( bytes > idlebuf->buf_allocsz )
      {
         /* Re-allocate buffer to accommodate data */
         local_irq_restore( flags );
         newbufp = vmalloc( bytes );
         if ( !newbufp )
         {
            return -ENOMEM;
         }
         local_irq_save( flags );

         free_bufp = idlebuf->bufp;
         idlebuf->bufp = newbufp;
      }

      memcpy( idlebuf->bufp, audiobuf, bytes );

      idlebuf->remain_bytes   = bytes;
      idlebuf->tot_bytes      = bytes;
      idlebuf->posp           = idlebuf->bufp;

      writestp->request       = 1;
      writestp->flush_count   = 3;  /* flush up to three frames to clear double buffer */
      writestp->usercb        = usercb;
      writestp->userdata      = userdata;
   }
   local_irq_restore( flags );

   if ( free_bufp )
   {
      vfree( free_bufp );
   }

   return 0;
}
EXPORT_SYMBOL( halAudioWriteRequest );

/***************************************************************************/
/**
*  Helper routine used to flush write buffers.
*
*  @return  0 for success, otherwise -ve error code.
*/
int halAudioWriteFlush(
   HALAUDIO_WRITE *writestp      /*<< (io) write state pointer */
)
{
   unsigned long flags;

   /* Flush existing write request, if any */
   local_irq_save( flags );
   if ( writestp->request )
   {
      /* Free any waiting user callback */
      if ( writestp->usercb )
      {
         writestp->usercb( writestp->buf[writestp->curr_bufidx].tot_bytes, writestp->userdata );
         writestp->usercb = NULL;
      }

      writestp->buf[0].remain_bytes = 0;
      writestp->buf[1].remain_bytes = 0;
      writestp->curr_bufidx         = 0;
      writestp->flush_count         = 0;
      writestp->request             = 0;
   }
   local_irq_restore( flags );

   return 0;
}
EXPORT_SYMBOL( halAudioWriteFlush );

/***************************************************************************/
/**
*  Helper routine used to cleanup write resources. This routine is intended
*  to be called in a HAL Audio driver's exit callback.
*
*  @return  0 for success, otherwise -ve error code.
*/
int halAudioWriteFree(
   HALAUDIO_WRITE *writestp      /*<< (io) write state pointer */
)
{
   int j;

   for ( j = 0; j < sizeof(writestp->buf)/sizeof(writestp->buf[0]); j++ )
   {
      if ( writestp->buf[j].bufp )
      {
         vfree( writestp->buf[j].bufp );
         writestp->buf[j].bufp = NULL;
      }
   }

   return 0;
}
EXPORT_SYMBOL( halAudioWriteFree );

/***************************************************************************/
/**
*  This function implements a FIR filter.
*
*  @return  Nothing
*
*  @remarks the accumulator is 64 bits.  For each output sample the 64 bit
*           accumulator is saturated to a 32 bit value.  The output samples
*           are Q15 format numbers.
*/
#define MIN_SINT16 (-32768)
#define MAX_SINT16 32767

#define FIR_SINT16_LIMIT(x) \
         if ( (x) < (MIN_SINT16) ) x = (MIN_SINT16); \
         if ( (x) > (MAX_SINT16) ) x = (MAX_SINT16)

static void fir(
   int16_t *x,    /**< (i) Ptr to input vector, dimension n+k-1
                           History of size n-1 must precede the k input
                           samples  */
   int16_t *y,    /**< (o) Ptr to output vector, dimension k */
   int32_t *h,    /**< (i) Ptr to filter coefficients, dimension n */
   int n,         /**< (i) filter order */
   int k          /**< (i) vector dimension */
)
{
   int64_t a;  /* 64 bit accumulator */
   int i,j;

   for ( j = 0; j < k; j++, x += 1 )
   {
      a = 0;
      for ( i = 0; i < n; i += 1 )
      {
         a += (int64_t)( x[-i] * (int16_t)h[i] );
      }
      a >>= 15;
      FIR_SINT16_LIMIT(a);
      y[j] = (int16_t) a;
   }
}

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
)
{
   int32_t xBufp[(HALAUDIO_SWEQU_MAX_SAMPLES + HALAUDIO_EQU_COEFS_MAX_NUM)/2];
   int16_t *xp, *xBuf16p;

   if ( order > 0 )
   {
      /* Copy history and samples */
      xBuf16p = (int16_t *)xBufp;
      memcpy( xBuf16p, histp, order * sizeof(histp[0]) );
      xp = &xBuf16p[order];
      memcpy( xp, samp, len * sizeof(xp[0]) );

      fir( xp,       /* new samples preceded by history*/
           samp,     /* output samples */
           coeff,    /* coefficients */
           order,    /* order, N */
           len );    /* number of samples, k */

      /* Update the history */
      memcpy( histp, &xBuf16p[len], order * sizeof(histp[0]) );
   }
}

EXPORT_SYMBOL( halAudioEquProcess );
