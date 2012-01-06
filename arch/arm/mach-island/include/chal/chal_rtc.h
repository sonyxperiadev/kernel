/*****************************************************************************
*  Copyright 2001 - 2011 Broadcom Corporation.  All rights reserved.
*
*  Unless you and Broadcom execute a separate written software license
*  agreement governing use of this software, this software is licensed to you
*  under the terms of the GNU General Public License version 2, available at
*  http://www.gnu.org/licenses/old-license/gpl-2.0.html (the "GPL").
*
*  Notwithstanding the above, under no circumstances may you combine this
*  software in any way with any other Broadcom software provided under a
*  license other than the GPL, without Broadcom's express prior written
*  consent.
*
*****************************************************************************/

/****************************************************************************/
/**
*  @file    chal_rtc.h
*
*  @brief   RTC driver interface header file.
*
*  @note
*
****************************************************************************/
#ifndef _CHAL_RTC_H
#define _CHAL_RTC_H

#ifdef __cplusplus
extern "C" {
#endif

/* ---- Include Files ---------------------------------------------------- */

#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/io.h>

#include <chal/chal_defs.h>
/* --- START: CHAL compatability shim --- */
static void CHAL_REG_SETBIT32(uint32_t addr, unsigned int bits)
{
    iowrite32(ioread32((void *)addr) | bits,  (void *)addr);
}

#define CHAL_REG_WRITE32(x,y) iowrite32(y,x)
#define CHAL_REG_READ32(x) ioread32(x)
#define chal_dbg_printf(x,...) pr_debug(__VA_ARGS__)

#include <mach/rdb/brcm_rdb_bbl_apb.h>
#include <mach/rdb/brcm_rdb_bbl_rtc.h>
#include <mach/rdb/brcm_rdb_sysmap.h>
#include <mach/rdb/brcm_rdb_map.h>

#define RTC_PERIODIC_TIMER_ADDR     (rtc->handle->rtcBaseAddr+BBL_RTC_PER_OFFSET)
#define RTC_MATCH_REGISTER_ADDR     (rtc->handle->rtcBaseAddr+BBL_RTC_MATCH_OFFSET)
#define RTC_CLEAR_INTR_ADDR         (rtc->handle->rtcBaseAddr+BBL_RTC_CLR_INT_OFFSET)
#define RTC_INTERRUPT_STATUS_ADDR   (rtc->handle->rtcBaseAddr+BBL_RTC_INT_STS_OFFSET)
#define RTC_CONTROL_ADDR            (rtc->handle->rtcBaseAddr+BBL_RTC_CTRL_OFFSET)
/* --- END:  CHAL compatability shim --- */

/* ---- Public Constants and Types --------------------------------------- */

/*
 * Handle for RTC controller operations.
 */
struct chal_rtc_handle
{
   uint32_t rtcBaseAddr; /* Base address of the RTC */
   uint32_t bar;         /* Base address to use for register accesses */
   uint32_t wr_data;     /* Data to be written into BBL APB Slave */
   uint32_t rd_data;     /* Data read from BBL APB Interface */
   uint32_t ctrl_sts;    /* APB Control & Status Register */
   uint32_t int_sts;     /* Status register indicating interrupt source from BBL */
};
typedef struct chal_rtc_handle CHAL_RTC_HANDLE_t;

typedef uint64_t chal_rtc_TIME_t;   /* Time in seconds */

/* RTC Block - CTRL */
typedef enum
{
   CHAL_RTC_CTRL_LOCK  = 0,  /* lock the real-time clock counters, SET_DIV and SEC_0 */
   CHAL_RTC_CTRL_UNLOCK,     /* unlock the real-time clock counters, SET_DIV and SEC_0 */
   CHAL_RTC_CTRL_STOP,       /* halt the real-time clock */
   CHAL_RTC_CTRL_RUN         /* start the real-time clock */
} chal_RTC_CTRL_e;

/* RTC Block - Interrupt types */
typedef enum
{
   CHAL_RTC_INT_PER    = 0,  /* Periodic interrupt */
   CHAL_RTC_INT_MATCH  = 1,  /* Match interrupt */
   CHAL_RTC_INT_NONE,
} chal_RTC_INT_e;

/* RTC - Monotonic timer clock */
typedef enum
{
   CHAL_RTC_MTC_LOCK    = 0, /* Locks writes to MTC.  Sticky bit */
   CHAL_RTC_MTC_INCR         /* increment MTC by INCR_AMT, even when locked */
} chal_RTC_MTC_CTRL_e;

/* RTC - Periodic interrupt interval */
typedef enum
{
   CHAL_RTC_PER_INTERVAL_125ms     = 0x00000001,  /* Time interval 125ms */
   CHAL_RTC_PER_INTERVAL_250ms     = 0x00000002,  /* Time interval 250ms */
   CHAL_RTC_PER_INTERVAL_500ms     = 0x00000004,  /* Time interval 500ms */
   CHAL_RTC_PER_INTERVAL_1000ms    = 0x00000008,  /* Time interval 1 sec */
   CHAL_RTC_PER_INTERVAL_2000ms    = 0x00000010,  /* Time interval 2 sec */
   CHAL_RTC_PER_INTERVAL_4000ms    = 0x00000020,  /* Time interval 4 sec */
   CHAL_RTC_PER_INTERVAL_8000ms    = 0x00000040,  /* Time interval 8 sec */
   CHAL_RTC_PER_INTERVAL_16000ms   = 0x00000080,  /* Time interval 16 sec */
   CHAL_RTC_PER_INTERVAL_32000ms   = 0x00000100,  /* Time interval 32 sec */
   CHAL_RTC_PER_INTERVAL_64000ms   = 0x00000200,  /* Time interval 64 sec */
   CHAL_RTC_PER_INTERVAL_128000ms  = 0x00000400,  /* Time interval 128 sec */
   CHAL_RTC_PER_INTERVAL_256000ms  = 0x00000800   /* Time interval 256 sec */
} chal_RTC_PER_INTERVAL_e;

#define CHAL_RTC_RESET_ACCESS_STATUS     0xACCE55ED

/* ---- Public Variable Externs ------------------------------------------ */
/* ---- Public Function Prototypes --------------------------------------- */

/*===========================================================================
* Functions for RTC operations.
* ===========================================================================*/

/****************************************************************************/
/**
*  @brief   Initialize RTC config structure
*
*  @param   handle      (in) Pointer to the RTC initialization structure
*  @param   bblBar      (in) Location of BBL base address register
*  @param   config      (in) Location of RTC base address register 
*
****************************************************************************/
int chal_rtc_init(CHAL_RTC_HANDLE_t *handle, uint32_t bblBar, uint32_t rtcBar);

static inline void      chal_rtc_divSet(CHAL_RTC_HANDLE_t *handle, uint32_t val);
static inline uint32_t  chal_rtc_divGet(CHAL_RTC_HANDLE_t *handle);

static inline void      chal_rtc_secSet(CHAL_RTC_HANDLE_t *handle, uint32_t val);
static inline uint32_t  chal_rtc_secGet(CHAL_RTC_HANDLE_t *handle);

static inline void      chal_rtc_ctrlSet(CHAL_RTC_HANDLE_t *handle, chal_RTC_CTRL_e cmd);

static inline void      chal_rtc_periodInterruptValSet(CHAL_RTC_HANDLE_t *handle, chal_RTC_PER_INTERVAL_e interval);
static inline uint32_t  chal_rtc_periodInterruptValGet(CHAL_RTC_HANDLE_t *handle);

static inline void      chal_rtc_matchInterruptValSet(CHAL_RTC_HANDLE_t *handle, chal_rtc_TIME_t sec);
static inline uint32_t  chal_rtc_matchInterruptValGet(CHAL_RTC_HANDLE_t *handle);
static inline void      chal_rtc_intStatusClr(CHAL_RTC_HANDLE_t *handle, chal_RTC_INT_e interrupt);
static inline void      chal_rtc_intStatusClrVal(CHAL_RTC_HANDLE_t *handle, uint32_t val);
static inline int       chal_rtc_intStatusGet(CHAL_RTC_HANDLE_t *handle, chal_RTC_INT_e interrupt);
static inline uint32_t  chal_rtc_intStatusGetVal(CHAL_RTC_HANDLE_t *handle);
static inline void      chal_rtc_intEnable(CHAL_RTC_HANDLE_t *handle, chal_RTC_INT_e interrupt);
static inline void      chal_rtc_intEnableVal(CHAL_RTC_HANDLE_t *handle, uint32_t bitsOn);
static inline void      chal_rtc_intDisable(CHAL_RTC_HANDLE_t *handle, chal_RTC_INT_e interrupt);
static inline void      chal_rtc_intDisableVal(CHAL_RTC_HANDLE_t *handle, uint32_t bitsOff );

static inline int       chal_rtc_intIsEnabled(CHAL_RTC_HANDLE_t *handle, chal_RTC_INT_e interrupt);
static inline           chal_RTC_INT_e chal_rtc_interruptStatusGet(CHAL_RTC_HANDLE_t *handle);

static inline int       chal_rtc_rstIsAsserted(CHAL_RTC_HANDLE_t *handle);

static inline void      chal_rtc_mtcCtrlSet(CHAL_RTC_HANDLE_t *handle, chal_RTC_MTC_CTRL_e cmd);
static inline void      chal_rtc_mtcIncrSet(CHAL_RTC_HANDLE_t *handle, uint16_t amount);

static inline void      chal_rtc_mtcTimeSet(CHAL_RTC_HANDLE_t *handle, chal_rtc_TIME_t time);
static inline chal_rtc_TIME_t chal_rtc_mtcTimeGet(CHAL_RTC_HANDLE_t *handle);

/* ---- Private Constants and Types ---------------------------------------- */
/* ---- Public Variable Externs -------------------------------------------- */
/* ---- Private Function Prototypes ----------------------------------------- */
static inline uint32_t  chal_rtc_readReg(CHAL_RTC_HANDLE_t *handle, uint32_t regAddr);
static inline void      chal_rtc_readRegIE(CHAL_RTC_HANDLE_t *handle, uint32_t regAddr);
static inline void      chal_rtc_writeReg(CHAL_RTC_HANDLE_t *handle, uint32_t regAddr, uint32_t val);
static inline void      chal_rtc_writeRegNoWait(CHAL_RTC_HANDLE_t *handle, uint32_t regAddr, uint32_t val);

/* Macros for accessing RTC registers through the APB BBL interface*/
#define  chal_rtc_REG_SET_APB_SLAVE_ADDRESS(handle, add) (CHAL_REG_WRITE32(handle->bar, add))
#define  chal_rtc_REG_GET_WR_DATA(handle)         (CHAL_REG_READ32(handle->wr_data))
#define  chal_rtc_REG_SET_WR_DATA(handle, val)    (CHAL_REG_WRITE32(handle->wr_data, val))
#define  chal_rtc_REG_GET_RD_DATA(handle)         (CHAL_REG_READ32(handle->rd_data))
#define  chal_rtc_REG_COMMAND_WRITE(handle)       (CHAL_REG_SETBIT32(handle->ctrl_sts, BBL_APB_CTRL_STS_BBL_CMD_START_MASK | BBL_APB_CTRL_STS_BBL_APB_CMD_MASK))
#define  chal_rtc_REG_COMMAND_READ(handle)        (CHAL_REG_SETBIT32(handle->ctrl_sts, BBL_APB_CTRL_STS_BBL_CMD_START_MASK & ~BBL_APB_CTRL_STS_BBL_APB_CMD_MASK))
#define  chal_rtc_REG_IS_COMMAND_DONE(handle)     (CHAL_REG_READ32(handle->ctrl_sts) & BBL_APB_CTRL_STS_BBL_CMD_DONE_MASK)
#define  chal_rtc_REG_COMMAND_DONE_CLEAR(handle)  (CHAL_REG_SETBIT32(handle->ctrl_sts, BBL_APB_CTRL_STS_BBL_CMD_DONE_MASK))


/* ---- Public Function Prototypes ----------------------------------------- */

/****************************************************************************/
/**
*  @brief   Set the RTC clock divider value. The clock must be set to start
*           running again after this is called. 
*
*  @return  none
*/
/****************************************************************************/
static inline void chal_rtc_divSet(CHAL_RTC_HANDLE_t *handle, uint32_t val)
{
   chal_rtc_ctrlSet(handle, CHAL_RTC_CTRL_STOP);
   chal_rtc_writeReg(handle, handle->rtcBaseAddr + RTC_SET_DIV_OFFSET, val);
}

/****************************************************************************/
/**
*  @brief   Get the RTC clock divider value
*
*  @return  Clock divider value
*/
/****************************************************************************/
static inline uint32_t chal_rtc_divGet(CHAL_RTC_HANDLE_t *handle)
{
   return ( chal_rtc_readReg (handle, handle->rtcBaseAddr + RTC_SET_DIV_OFFSET));
}

/****************************************************************************/
/**
*  @brief   Set the RTC seconds value
*
*  @return  none
*/
/****************************************************************************/
static inline void chal_rtc_secSet(CHAL_RTC_HANDLE_t *handle, uint32_t val)
{
   chal_rtc_ctrlSet(handle, CHAL_RTC_CTRL_STOP);
   chal_dbg_printf(CHAL_DBG_DEBUG, "Write %d to SEC_0\n", val);
   chal_rtc_writeReg (handle, handle->rtcBaseAddr + RTC_SEC_0_OFFSET, val);
}

/****************************************************************************/
/**
*  @brief   Get the RTC seconds value
*
*  @return  RTC seconds value
*/
/****************************************************************************/
static inline uint32_t chal_rtc_secGet(CHAL_RTC_HANDLE_t *handle)
{
    uint32_t val = chal_rtc_readReg(handle, handle->rtcBaseAddr + RTC_SEC_0_OFFSET);
    return val;
}

/****************************************************************************/
/**
*  @brief   Sets RTC control
*
*  @return  none
*/
/****************************************************************************/
static inline void chal_rtc_ctrlSet(CHAL_RTC_HANDLE_t *handle, chal_RTC_CTRL_e cmd)
{
   uint32_t val;
   uint32_t addr;

   addr = handle->rtcBaseAddr + RTC_CTRL_OFFSET;

   /* Read the data */
   val = chal_rtc_readReg (handle, addr);
   val &= 0x3;
   chal_dbg_printf(CHAL_DBG_DEBUG, "Read RTC CTRL: %d\n", val);
   
   switch ( cmd )
   {
      case CHAL_RTC_CTRL_LOCK:
         /* Set the RTC lock bit */
 	      val |= RTC_CTRL_RTC_LOCK_MASK;
         break;
      case CHAL_RTC_CTRL_UNLOCK:
         /* Clear the RTC lock bit */
         val &= ~RTC_CTRL_RTC_LOCK_MASK;
         break;
      case CHAL_RTC_CTRL_STOP:
         /* Set the RTC stop bit */
 	      val |= RTC_CTRL_RTC_STOP_MASK;
         break;  
      case CHAL_RTC_CTRL_RUN:
         /* Clear the RTC stop bit */
         val &= ~RTC_CTRL_RTC_STOP_MASK;
         break;
   }

   /* Write the data */
   chal_dbg_printf(CHAL_DBG_DEBUG, "Write %d to RTC CTRL\n", val);
   chal_rtc_writeReg (handle, addr, val);
}

/****************************************************************************/
/**
*  @brief   Configures a periodic timer to generate timer interrupt after
*           certain time interval
*
*  This function initializes a periodic timer to generate timer interrupt
*  after every time interval.
*
*  @return  none
*/
/****************************************************************************/
static inline void chal_rtc_periodInterruptValSet(CHAL_RTC_HANDLE_t *handle, 
    chal_RTC_PER_INTERVAL_e interval)
{
   chal_dbg_printf(CHAL_DBG_DEBUG, "Write 0x%x to PER\n", interval);
   chal_rtc_writeReg(handle, handle->rtcBaseAddr + RTC_PER_OFFSET, interval);
}

/****************************************************************************/
/**
*  @brief   Gets the perioic timer configuration
*
*  @return  Perioidic interrupt value
*/
/****************************************************************************/
static inline uint32_t chal_rtc_periodInterruptValGet(CHAL_RTC_HANDLE_t *handle)
{
   return( chal_rtc_readReg(handle, handle->rtcBaseAddr + RTC_PER_OFFSET) );
}

/****************************************************************************/
/**
*  @brief   Configures the real time timer to generate an oneshot interrupt
*
*  This function initializes the timer to generate one shot interrupt
*  after certain time period in seconds
*
*  @return  none
*/
/****************************************************************************/
static inline void chal_rtc_matchInterruptValSet(CHAL_RTC_HANDLE_t *handle,
    chal_rtc_TIME_t sec)
{
   chal_rtc_writeReg(handle, handle->rtcBaseAddr + RTC_MATCH_OFFSET, 
       (sec & RTC_MATCH_RTC_MATCH_MASK));
}

/****************************************************************************/
/**
*  @brief   Gets the real time timer to generate an oneshot interrupt
*
*  This function initializes the timer to generate one shot interrupt
*  after certain time period in seconds
*
*  @return  Match interrupt value
*/
/****************************************************************************/
static inline uint32_t chal_rtc_matchInterruptValGet(CHAL_RTC_HANDLE_t *handle)
{
   return( chal_rtc_readReg(handle, handle->rtcBaseAddr + RTC_MATCH_OFFSET) );
}

/****************************************************************************/
/**
*  @brief   Clears a RTC interrupt
*
*  @return  none
*/
/****************************************************************************/
static inline void chal_rtc_intStatusClr(CHAL_RTC_HANDLE_t *handle, 
    chal_RTC_INT_e interrupt)
{
   chal_rtc_writeReg(handle, handle->rtcBaseAddr + RTC_CLR_INT_OFFSET, 
       1 << interrupt);
} 

static inline void chal_rtc_intStatusClrVal(CHAL_RTC_HANDLE_t *handle, 
    uint32_t val)
{
   uint32_t verify = 0;
    
   chal_dbg_printf(CHAL_DBG_DEBUG, "Writing %d to CLR_INT\n", val);
   chal_rtc_writeReg(handle, handle->rtcBaseAddr + RTC_CLR_INT_OFFSET, val);
   
   verify = chal_rtc_readReg(handle, handle->rtcBaseAddr + RTC_CLR_INT_OFFSET);
   //chal_dbg_printf(CHAL_DBG_DEBUG, "Verifying CLR_INT: %d\n", verify);
} 

/****************************************************************************/
/**
*  @brief   Get the status of a RTC interrupt
*
*  @return  1: an interrupt occurred
*           0: no interrupt occurred
*/
/****************************************************************************/
static inline int chal_rtc_intStatusGet(CHAL_RTC_HANDLE_t *handle, 
    chal_RTC_INT_e interrupt)
{
    return( ( chal_rtc_readReg(handle, handle->rtcBaseAddr + RTC_INT_STS_OFFSET) & 
        (1<<interrupt) ) ? 1 : 0 );
}

static inline uint32_t chal_rtc_intStatusGetVal(CHAL_RTC_HANDLE_t *handle)
{
    uint32_t val = chal_rtc_readReg(handle, handle->rtcBaseAddr + RTC_INT_STS_OFFSET);
    return val; 
}


/****************************************************************************/
/**
*  @brief   Gets the status of enabled interrupts
*
*  @return  rtcHw_INTERRUPT_STATUS_NONE      : On no interrupt
*
*           rtcHw_INTERRUPT_STATUS_PERIODIC  : On interrupt
*           rtcHw_INTERRUPT_STATUS_ONESHOT
*/
/****************************************************************************/
static inline chal_RTC_INT_e chal_rtc_interruptStatusGet(CHAL_RTC_HANDLE_t *handle)
{
   if( chal_rtc_intStatusGet(handle, CHAL_RTC_INT_PER) & 
       chal_rtc_intIsEnabled(handle, CHAL_RTC_INT_PER) )
   {
      return CHAL_RTC_INT_PER;
   }
   else if( chal_rtc_intStatusGet(handle, CHAL_RTC_INT_MATCH) & 
       chal_rtc_intIsEnabled(handle, CHAL_RTC_INT_MATCH) )
   {
       return CHAL_RTC_INT_MATCH;
   }
   return CHAL_RTC_INT_NONE;
} 

/****************************************************************************/
/**
*  @brief   Enable/disable a RTC interrupt
*
*  @return  none
*/
/****************************************************************************/
static inline void chal_rtc_intEnable(CHAL_RTC_HANDLE_t *handle, 
    chal_RTC_INT_e interrupt)
{
   uint32_t val, addr;

   addr = handle->rtcBaseAddr + RTC_INT_ENABLE_OFFSET;

   val = chal_rtc_readReg(handle, addr);
   val |= (1 << interrupt);
   chal_dbg_printf(CHAL_DBG_DEBUG, "Write INT_ENABLE: %d\n", val);
   chal_rtc_writeRegNoWait(handle, addr, val);
   
//   val = chal_rtc_readReg(handle, addr);
  // chal_dbg_printf(CHAL_DBG_DEBUG, "Read INT_ENABLE: %d\n", val); 
}

static inline void chal_rtc_intEnableVal(CHAL_RTC_HANDLE_t *handle, uint32_t bitsOn)
{
   uint32_t val, addr;

   addr = handle->rtcBaseAddr + RTC_INT_ENABLE_OFFSET;

   val = chal_rtc_readReg (handle, addr);
   val |= bitsOn;
   chal_dbg_printf(CHAL_DBG_DEBUG, "INT_ENABLE (enabling): %d\n", val);
   chal_rtc_writeReg (handle, addr, val);
}

static inline void chal_rtc_intDisable(CHAL_RTC_HANDLE_t *handle, 
    chal_RTC_INT_e interrupt)
{
   uint32_t val, addr;

   addr = handle->rtcBaseAddr + RTC_INT_ENABLE_OFFSET;

   val = chal_rtc_readReg(handle, addr);
   val &= ~(1 << interrupt);
   chal_dbg_printf(CHAL_DBG_DEBUG, "Write %d to INT_ENABLE (disabling)\n", val);
   chal_rtc_writeReg(handle, addr, val);
}

static inline void chal_rtc_intDisableVal(CHAL_RTC_HANDLE_t *handle, uint32_t bitsOff)
{
   uint32_t val, addr;

   addr = handle->rtcBaseAddr + RTC_INT_ENABLE_OFFSET;

   val = chal_rtc_readReg (handle, addr );
   val &= ~(bitsOff);
   chal_dbg_printf(CHAL_DBG_DEBUG, "Write %d to INT_ENABLE (disabling)\n", val);
   chal_rtc_writeReg (handle, addr, val);
}

/****************************************************************************/
/**
*  @brief   Check whether RTC interrupt is enabled
*
*  @return  1: interrupt is enabled
*           0: interrupt is disabled
*/
/****************************************************************************/
static inline int chal_rtc_intIsEnabled(CHAL_RTC_HANDLE_t *handle, 
    chal_RTC_INT_e interrupt)
{
   return( ( chal_rtc_readReg (handle, handle->rtcBaseAddr + RTC_INT_ENABLE_OFFSET ) 
       & (1<<interrupt) ) ? 1 : 0 );
}

/****************************************************************************/
/**
*  @brief   Checks whether BBL reset has been asserted
*
*  @return  1: reset is asserted
*           0: reset is not asserted
*/
/****************************************************************************/
static inline int chal_rtc_rstIsAsserted(CHAL_RTC_HANDLE_t *handle)
{
   return( ( chal_rtc_readReg (handle, handle->rtcBaseAddr + RTC_RESET_ACCESS_OFFSET ) == 
       CHAL_RTC_RESET_ACCESS_STATUS ) ? 1 : 0 );
}

/****************************************************************************/
/**
*  @brief   Set monotonic counter control
*
*  @return  none
*/
/****************************************************************************/
static inline void chal_rtc_mtcCtrlSet(CHAL_RTC_HANDLE_t *handle, 
    chal_RTC_MTC_CTRL_e cmd)
{
   uint32_t val, addr;

   addr = handle->rtcBaseAddr + RTC_MTC_CTRL_OFFSET;

   /* Read the data */
   val = chal_rtc_readReg (handle, addr );

   switch ( cmd )
   {
      case CHAL_RTC_MTC_LOCK:
         /* locks monotonic counter - sticky bit */
 	      val |= RTC_MTC_CTRL_RTC_MT_CTR_LOCK_MASK;
         break;
      case CHAL_RTC_MTC_INCR:
         /* increment monotonic counter by incr_amt - self clear */
         val |= RTC_MTC_CTRL_RTC_MT_CTR_INCR_MASK;
         break;
   }

   /* Write the data */
   chal_rtc_writeReg (handle, addr, val);
}

/****************************************************************************/
/**
*  @brief   Set monotonic counter increment amount
*
*  @return  none
*/
/****************************************************************************/
static inline void chal_rtc_mtcIncrSet(CHAL_RTC_HANDLE_t *handle, uint16_t amount)
{
   uint32_t val, addr;

   addr = handle->rtcBaseAddr + RTC_MTC_CTRL_OFFSET;

   /* Read the data */
   val = chal_rtc_readReg (handle, addr );

   val &= ~RTC_MTC_CTRL_RTC_MT_CTR_INCR_AMT_MASK;
   val |= (uint32_t) amount << RTC_MTC_CTRL_RTC_MT_CTR_INCR_AMT_SHIFT;

   /* Write the data */
   chal_rtc_writeReg (handle, addr, val);
}

/****************************************************************************/
/**
*  @brief   Sets the current time counts in seconds
*
*  This function gets the current time counts
*
*  @return  Current time count since it started
*/
/****************************************************************************/
static inline void chal_rtc_mtcTimeSet(CHAL_RTC_HANDLE_t *handle, 
    chal_rtc_TIME_t time)
{
   chal_rtc_writeReg (handle, handle->rtcBaseAddr + RTC_MTC_MSB_OFFSET, time >> 32 );
   chal_rtc_writeReg (handle, handle->rtcBaseAddr + RTC_MTC_LSB_OFFSET, time & 0xffffffff );
}

/****************************************************************************/
/**
*  @brief   Gets the current time counts in seconds
*
*  This function gets the current time counts
*
*  @return  Current time count since it started
*/
/****************************************************************************/
static inline chal_rtc_TIME_t chal_rtc_mtcTimeGet(CHAL_RTC_HANDLE_t *handle)
{
   uint32_t mtcMsb, mtcLsb;
   chal_rtc_TIME_t time;

   mtcMsb = chal_rtc_readReg (handle, handle->rtcBaseAddr + RTC_MTC_MSB_OFFSET);
   mtcLsb = chal_rtc_readReg (handle, handle->rtcBaseAddr + RTC_MTC_LSB_OFFSET);

   time = ((chal_rtc_TIME_t)mtcMsb) << 32 | (chal_rtc_TIME_t) mtcLsb;

   return time;
}

/* ==== Private Functions ================================================= */

static inline void chal_rtc_readyForCommand(CHAL_RTC_HANDLE_t *handle)
{
    uint32_t count = 0;
    while (!chal_rtc_REG_IS_COMMAND_DONE(handle))
    {
        udelay (1);
        count++;
    }

    /* This delay is a hack to get around a problem where RTC registers can't be
    read if they occur too soon after a previous read. */
//    #if CFG_GLOBAL_CHIP == BCM11160  
       mdelay(1);
//    #endif

    chal_dbg_printf(CHAL_DBG_DEBUG, "Command completed in %d us\n", count);
    chal_rtc_REG_COMMAND_DONE_CLEAR(handle);
}

/****************************************************************************/
/**
*  @brief   A command to read indirect RTC registers
*
*  @return  Value read
*/
/****************************************************************************/
static inline uint32_t chal_rtc_readReg(CHAL_RTC_HANDLE_t *handle, uint32_t regAddr)
{
   uint32_t ctrlStsVal = 0;
   uint32_t val; 

   ctrlStsVal = CHAL_REG_READ32(handle->ctrl_sts);

   chal_rtc_REG_SET_APB_SLAVE_ADDRESS (handle, regAddr);

   /* Execute read command */
   ctrlStsVal &= ~BBL_APB_CTRL_STS_BBL_APB_CMD_MASK;
   ctrlStsVal |= BBL_APB_CTRL_STS_BBL_CMD_START_MASK;
   chal_dbg_printf(CHAL_DBG_DEBUG, "Write %d to ctrl_sts\n", ctrlStsVal);
   CHAL_REG_WRITE32(handle->ctrl_sts, ctrlStsVal);
   
   /* Wait until command is processed */
   chal_rtc_readyForCommand(handle);
   
   /* Read the data */
   val = chal_rtc_REG_GET_RD_DATA(handle);
   
   return val;
}

/**
 * Reads an RTC register through the RTC APB interface but doesn't wait for the 
 * command to complete by polling the command ready flag in status register. The
 * command complete interrupt is used instead and an ISR must be installed. When
 * the interrupt has fired then the rd_data register can be read by calling 
 * chal_rtc_REG_GET_RD_DATA() to get the value of the register that was read.
 */ 
static inline void chal_rtc_readRegIE(CHAL_RTC_HANDLE_t *handle, uint32_t regAddr)
{
   uint32_t ctrlStsVal = 0;

   ctrlStsVal = CHAL_REG_READ32(handle->ctrl_sts);

   chal_rtc_REG_SET_APB_SLAVE_ADDRESS (handle, regAddr);

   /* Execute read command */
   ctrlStsVal &= ~BBL_APB_CTRL_STS_BBL_APB_CMD_MASK;
   ctrlStsVal |= BBL_APB_CTRL_STS_BBL_CMD_START_MASK;
   /* Command ready interrupt enable is done on every command */
   ctrlStsVal |= BBL_APB_CTRL_STS_APB_CMD_DONE_INT_EN_MASK;
   chal_dbg_printf(CHAL_DBG_DEBUG, "Write %d to ctrl_sts\n", ctrlStsVal);
   CHAL_REG_WRITE32(handle->ctrl_sts, ctrlStsVal);
}

/****************************************************************************/
/**
*  @brief   A command to write to indirect BBL registers
*
*  @return  none
*/
/****************************************************************************/
static inline void chal_rtc_writeReg(CHAL_RTC_HANDLE_t *handle, 
    uint32_t regAddr, uint32_t val)
{
   /* Make sure BBL interface is ready to accept commands */
   chal_rtc_REG_SET_APB_SLAVE_ADDRESS(handle, regAddr);
   CHAL_REG_WRITE32(handle->wr_data, val);
   chal_rtc_REG_COMMAND_WRITE(handle);
   chal_rtc_readyForCommand(handle);
}

static inline void chal_rtc_writeRegNoWait(CHAL_RTC_HANDLE_t *handle, 
    uint32_t regAddr, uint32_t val )
{
   /* Make sure BBL interface is ready to accept commands */
   chal_rtc_REG_SET_APB_SLAVE_ADDRESS(handle, regAddr);
   CHAL_REG_WRITE32(handle->wr_data, val);
   chal_rtc_REG_COMMAND_WRITE(handle);
}

#ifdef __cplusplus
}
#endif

#endif /* _CHAL_RTC_H */

