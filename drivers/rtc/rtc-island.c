/* drivers/rtc/rtc-bcmhana.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * BCMHANA RTC Driver
 */

/*
 * Frameworks:
 *
 *    - SMP:          Fully supported.    Locking is in place where necessary.
 *    - GPIO:         Fully supported.    No GPIOs are used.
 *    - MMU:          Partiall done.      CHAL layer is broken needs interface like kona keypad
 *    - Dynamic /dev: Not applicable.
 *    - Suspend:      Implemented.        Suspend and resume are implemented and should work.
 *    - Clocks:       Not done.
 *    - Power:        Not done.
 *
 */

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/rtc.h>
#include <linux/clk.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/errno.h>

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
    mdelay(1);

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

int chal_rtc_init(CHAL_RTC_HANDLE_t *handle, uint32_t bblBar, uint32_t rtcBar)
{
    if (!handle)
    {
        return -1;
    }

    handle->bar = bblBar;
    handle->wr_data  = handle->bar + BBL_APB_WR_DATA_OFFSET;
    handle->rd_data  = handle->bar + BBL_APB_RD_DATA_OFFSET;
    handle->ctrl_sts = handle->bar + BBL_APB_CTRL_STS_OFFSET;
    handle->int_sts  = handle->bar + BBL_APB_INT_STS_OFFSET;

    handle->rtcBaseAddr = rtcBar;

    return 0;
}

struct bcmhana_rtc
{
    CHAL_RTC_HANDLE_t *handle;
    struct rtc_device *dev;
    void __iomem  *base;
    unsigned int irq1;
    unsigned int irq2;
    unsigned int max_user_freq;
   struct clk *clock;
};
    struct bcmhana_rtc *rtc;
CHAL_RTC_HANDLE_t foo;

static unsigned int epoch = 1970;
static DEFINE_SPINLOCK( bcmhana_rtc_lock );

/* IRQ Handlers */

/*
 * RTC IRQ hanlder. This routine is invoked when a RTC oneshot timer completes
 */
static irqreturn_t
rtc_alm_isr( int irq, void *data )
{
    struct bcmhana_rtc *rdev = data;

    /* Disable alarm interrupts because they are oneshot */
    chal_rtc_intDisable( rtc->handle, CHAL_RTC_INT_MATCH);
    chal_rtc_intStatusClr( rtc->handle, CHAL_RTC_INT_MATCH);
    if ( chal_rtc_matchInterruptValGet(rtc->handle) )
    {
        pr_debug( "%s: oneshot interrupted\n", __func__ );
        rtc_update_irq( rdev->dev, 1, RTC_AF | RTC_IRQF );
    }
    return IRQ_HANDLED;
}

/*
 * RTC IRQ hanlder. This routine is invoked when periodic interrupts occur
 */
static irqreturn_t
rtc_per_isr( int irq, void *data )
{
    struct bcmhana_rtc *rdev = data;
    pr_debug( "%s: periodic interrupted\n", __func__ );
    chal_rtc_intStatusClr( rtc->handle, CHAL_RTC_INT_PER);
    if ( chal_rtc_periodInterruptValGet(rtc->handle)  )
    {
        pr_debug( "%s: periodic interrupted\n", __func__ );
        rtc_update_irq( rdev->dev, 1, RTC_PF | RTC_IRQF );
    }
    return IRQ_HANDLED;
}

/* Update control registers */
static void
bcmhana_rtc_setaie( int to )
{
    pr_debug( "%s: aie=%d\n", __func__, to );

    if ( to )
    {
        chal_rtc_intEnable(rtc->handle,  CHAL_RTC_INT_MATCH);
    }
    else
    {
        chal_rtc_intDisable(rtc->handle,  CHAL_RTC_INT_MATCH);
    }
}

static int
bcmhana_rtc_setpie( struct device *dev, int enabled )
{
    pr_debug( "%s: pie=%d\n", __func__, enabled );

    spin_lock_irq( &bcmhana_rtc_lock );

    if ( enabled )
    {
        chal_rtc_intEnable( rtc->handle, CHAL_RTC_INT_PER ); /* enables the interrupt */
    }
    else
    {
        chal_rtc_intDisable( rtc->handle, CHAL_RTC_INT_PER );  /* disables the interrupt */
    }
    spin_unlock_irq( &bcmhana_rtc_lock );

    return 0;
}

static int
bcmhana_rtc_setfreq( struct device *dev, int freq )
{
    chal_RTC_PER_INTERVAL_e interval = 0xffffffff;   /* invalid */

    pr_debug( "%s: freq=%d\n", __func__, freq );
    spin_lock_irq( &bcmhana_rtc_lock );
    switch ( freq )
    {
    case 1:
        interval = CHAL_RTC_PER_INTERVAL_1000ms;
        break;
    case 2:
        interval = CHAL_RTC_PER_INTERVAL_500ms;
        break;
    case 4:
        interval = CHAL_RTC_PER_INTERVAL_250ms;
        break;
    case 8:
        interval = CHAL_RTC_PER_INTERVAL_125ms;
        break;
    }
    spin_unlock_irq( &bcmhana_rtc_lock );

    if ( interval != 0xffffffff )
    {
        pr_debug( "%s: OKAY freq=%d interval=%d\n", __func__, freq, interval );
       chal_rtc_periodInterruptValSet( rtc->handle, interval );
    }
    else
    {
        pr_debug( "%s: BAD freq=%d\n", __func__, freq );
        return -EINVAL;
    }
    return 0;
}

static int
bcmhana_rtc_getfreq( struct device *dev, int *freq )
{
    chal_RTC_PER_INTERVAL_e interval;
    *freq = 0xffffffff;          /* invalid */

    spin_lock_irq( &bcmhana_rtc_lock );
    interval = chal_rtc_readReg(rtc->handle,  RTC_PERIODIC_TIMER_ADDR );
    switch ( interval )
    {
    case CHAL_RTC_PER_INTERVAL_125ms:   /* avoid compiler warnings */
        *freq = 8;
        break;
    case CHAL_RTC_PER_INTERVAL_250ms:
        *freq = 4;
        break;
    case CHAL_RTC_PER_INTERVAL_500ms:
        *freq = 2;
        break;
    case CHAL_RTC_PER_INTERVAL_1000ms:
        *freq = 1;
        break;
    case CHAL_RTC_PER_INTERVAL_2000ms:
    case CHAL_RTC_PER_INTERVAL_4000ms:
    case CHAL_RTC_PER_INTERVAL_8000ms:
    case CHAL_RTC_PER_INTERVAL_16000ms:
    case CHAL_RTC_PER_INTERVAL_32000ms:
    case CHAL_RTC_PER_INTERVAL_64000ms:
    case CHAL_RTC_PER_INTERVAL_128000ms:
    case CHAL_RTC_PER_INTERVAL_256000ms:
        break;
    }
    spin_unlock_irq( &bcmhana_rtc_lock );
    if ( *freq == 0xffffffff )
    {
        pr_debug( "%s: Bad interval=%d\n", __func__, interval );
        return -EINVAL;
    }
    pr_debug( "%s: interval=%d, freq=%d\n", __func__, interval, *freq );
    return 0;
}

/* Time read/write */

static int
bcmhana_rtc_gettime( struct device *dev, struct rtc_time *rtc_tm )
{
    unsigned int epoch_sec, elapsed_sec;

    epoch_sec = mktime( epoch, 1, 1, 0, 0, 0 );
    elapsed_sec = chal_rtc_secGet(rtc->handle);

    pr_debug( "%s: epoch_sec=%u, elapsed_sec=%u\n", __func__, epoch_sec, elapsed_sec );
    rtc_time_to_tm( epoch_sec + elapsed_sec, rtc_tm );

    pr_debug( "read time 0x%02x.0x%02x.0x%02x 0x%02x/0x%02x/0x%02x\n",
              rtc_tm->tm_year, rtc_tm->tm_mon, rtc_tm->tm_mday, rtc_tm->tm_hour, rtc_tm->tm_min, rtc_tm->tm_sec );

    return 0;
}

static int
bcmhana_rtc_settime( struct device *dev, struct rtc_time *time )
{
    unsigned int epoch_sec, current_sec;

    epoch_sec = mktime( epoch, 1, 1, 0, 0, 0 );
    current_sec = mktime( time->tm_year + 1900, time->tm_mon + 1, time->tm_mday, time->tm_hour, time->tm_min, time->tm_sec );

    chal_rtc_secSet(rtc->handle,  current_sec - epoch_sec );
    chal_rtc_ctrlSet( rtc->handle, CHAL_RTC_CTRL_RUN );

    pr_debug( "%s: current_sec=%u, epoch_sec=%u\n", __func__, current_sec, epoch_sec );

    pr_debug( "set time %02d.%02d.%02d %02d/%02d/%02d\n", time->tm_year, time->tm_mon, time->tm_mday, time->tm_hour, time->tm_min, time->tm_sec );

    return 0;
}

static int
bcmhana_rtc_getalarm( struct device *dev, struct rtc_wkalrm *alrm )
{
    unsigned int epoch_sec, elapsed_sec, alarm_elapsed_sec;
    chal_rtc_TIME_t alm_reg_secs;
    struct rtc_time *alm_tm = &alrm->time;
    alrm->enabled = chal_rtc_intIsEnabled ( rtc->handle, CHAL_RTC_INT_MATCH );
//    alrm->pending = ( bbl_readReg( RTC_INTERRUPT_STATUS_ADDR ) & RTC_CMD_ONESHOT_INTERRUPT_STATUS ) ? 1 : 0;

    epoch_sec = mktime( epoch, 1, 1, 0, 0, 0 );
    elapsed_sec = chal_rtc_secGet(rtc->handle);

    alm_reg_secs =  chal_rtc_matchInterruptValGet(rtc->handle);

    /* Handle carry over */
    if ((elapsed_sec & 0x0ffff) > alm_reg_secs)
    {
        elapsed_sec += 0x10000;
    }
    elapsed_sec &= ~0xffff; /* clear lower 16 bits for 16-bit alarm match register below */
    alarm_elapsed_sec = elapsed_sec + alm_reg_secs;
    pr_debug( "%s: epoch_sec=%u, elapsed_sec=%u, alm_reg_secs=%lu=0x%lx, alarm_elapsed_sec=%u=0x%x\n",
              __func__, epoch_sec, elapsed_sec, (unsigned long) alm_reg_secs, (unsigned long) alm_reg_secs, alarm_elapsed_sec, alarm_elapsed_sec );

    rtc_time_to_tm( epoch_sec + alarm_elapsed_sec, alm_tm );
    pr_debug( "read alarm %02x %02x.%02x.%02x %02x/%02x/%02x\n",
              alrm->enabled, alm_tm->tm_year, alm_tm->tm_mon, alm_tm->tm_mday, alm_tm->tm_hour, alm_tm->tm_min, alm_tm->tm_sec );

    return 0;
}

static int
bcmhana_rtc_setalarm( struct device *dev, struct rtc_wkalrm *alrm )
{
    unsigned int epoch_sec, elapsed_sec;
    struct rtc_time *time = &alrm->time;
    chal_rtc_TIME_t alm_secs;

    pr_debug( "%s: %d, %02x/%02x/%02x %02x.%02x.%02x\n",
              __func__, alrm->enabled, time->tm_mday & 0xff, time->tm_mon & 0xff, time->tm_year & 0xff, time->tm_hour & 0xff, time->tm_min & 0xff,
              time->tm_sec );

    epoch_sec = mktime( epoch, 1, 1, 0, 0, 0 );
    elapsed_sec = chal_rtc_secGet(rtc->handle);
    alm_secs = mktime( time->tm_year + 1900, time->tm_mon + 1, time->tm_mday, time->tm_hour, time->tm_min, time->tm_sec );

    pr_debug( "%s: epoch_sec=%u, elapsed_sec=%u, alm_secs=%lu\n", __func__, epoch_sec, elapsed_sec, (unsigned long)alm_secs );

    chal_rtc_intDisable( rtc->handle, CHAL_RTC_INT_MATCH );
    chal_rtc_intStatusClr ( rtc->handle, CHAL_RTC_INT_MATCH );

    chal_rtc_matchInterruptValSet( rtc->handle, alm_secs );

    if (alrm->enabled) {
        chal_rtc_intEnable( rtc->handle, CHAL_RTC_INT_MATCH );
    }


    return 0;
}

static int
bcmhana_rtc_proc( struct device *dev, struct seq_file *seq )
{
    seq_printf( seq, "\nperiodic timer: 0x%x\n", chal_rtc_readReg( rtc->handle, RTC_PERIODIC_TIMER_ADDR ) );
    seq_printf( seq, "match register: 0x%x\n", chal_rtc_readReg( rtc->handle, RTC_MATCH_REGISTER_ADDR ) );
    seq_printf( seq, "clear intr register: 0x%x\n", chal_rtc_readReg( rtc->handle, RTC_CLEAR_INTR_ADDR ) );
    seq_printf( seq, "intr status register: 0x%x\n", chal_rtc_readReg( rtc->handle, RTC_INTERRUPT_STATUS_ADDR ) );
    seq_printf( seq, "control addr register: 0x%x\n",chal_rtc_readReg( rtc->handle, RTC_CONTROL_ADDR ) );
    return 0;
}

static int
bcmhana_rtc_ioctl( struct device *dev, unsigned int cmd, unsigned long arg )
{
    switch ( cmd )
    {
    case RTC_AIE_OFF:
        spin_lock_irq( &bcmhana_rtc_lock );
        bcmhana_rtc_setaie( 0 );
        spin_unlock_irq( &bcmhana_rtc_lock );
        return 0;
    case RTC_AIE_ON:
        spin_lock_irq( &bcmhana_rtc_lock );
        bcmhana_rtc_setaie( 1 );
        spin_unlock_irq( &bcmhana_rtc_lock );
        return 0;
    case RTC_PIE_OFF:
        spin_lock_irq( &bcmhana_rtc_lock );
        chal_rtc_intDisable( rtc->handle, CHAL_RTC_INT_PER );
        spin_unlock_irq( &bcmhana_rtc_lock );
        return 0;
    case RTC_PIE_ON:
        spin_lock_irq( &bcmhana_rtc_lock );
        chal_rtc_intEnable( rtc->handle, CHAL_RTC_INT_PER );
        spin_unlock_irq( &bcmhana_rtc_lock );
        return 0;
    case RTC_IRQP_READ:
    {
        int freq;
        int ret = bcmhana_rtc_getfreq( dev, &freq );
        if ( ret != 0 )
        {
            return ret;
        }
        return put_user( freq, ( unsigned long * ) arg );
    }
    case RTC_IRQP_SET:
        return bcmhana_rtc_setfreq( dev, ( int ) arg );
    }
    return -ENOIOCTLCMD;
}

static void
bcmhana_rtc_release( struct device *dev )
{
    bcmhana_rtc_setaie( 0 );
    bcmhana_rtc_setpie( dev, 0 );
}

static const struct rtc_class_ops bcmhana_rtcops = {
    .ioctl = bcmhana_rtc_ioctl,
    .release = bcmhana_rtc_release,
    .read_time = bcmhana_rtc_gettime,
    .set_time = bcmhana_rtc_settime,
    .read_alarm = bcmhana_rtc_getalarm,
    .set_alarm = bcmhana_rtc_setalarm,
    .irq_set_freq = bcmhana_rtc_setfreq,
    .irq_set_state = bcmhana_rtc_setpie,
    .proc = bcmhana_rtc_proc,
};

static void bcmhana_rtc_enable( struct platform_device *pdev, int en )
{
    if ( !en )
    {
        chal_rtc_ctrlSet( rtc->handle, CHAL_RTC_CTRL_STOP );
    }
    else
    {
        chal_rtc_ctrlSet( rtc->handle, CHAL_RTC_CTRL_RUN );
    }
}

static int __exit
bcmhana_rtc_remove( struct platform_device *dev )
{
    rtc_device_unregister( rtc->dev );
    device_init_wakeup( &dev->dev, 0 );

    platform_set_drvdata( dev, NULL );

    bcmhana_rtc_setpie( &dev->dev, 0 );
    bcmhana_rtc_setaie( 0 );

    free_irq( rtc->irq2, rtc );
    free_irq( rtc->irq1, rtc );

    clk_disable(rtc->clock);
    clk_put(rtc->clock);

    return 0;
}

static int __devinit bcmhana_rtc_probe(struct platform_device *dev)
{
    struct resource *res;
    int ret;

    /* We only accept one device, and it must have an id of -1 */
    if (dev->id != -1)
        return -ENODEV;

    res = platform_get_resource(dev, IORESOURCE_MEM, 0);
    if (!res) {
        ret = -ENODEV;
        goto err_out;
    }

    rtc = kzalloc(sizeof(struct bcmhana_rtc), GFP_KERNEL);
    if (!rtc) {
        ret = -ENOMEM;
        goto err_out;
    }

    rtc->clock = clk_get(&dev->dev, "bbl_apb_clk");
    if (rtc->clock < 0)
    {
        ret = -ENXIO;
        goto err_free;
    }


    rtc->irq1 = platform_get_irq(dev, 0);
    if (rtc->irq1 < 0) {
        ret = -ENXIO;
        goto err_free;
    }
    rtc->irq2 = platform_get_irq(dev, 1);
    if (rtc->irq2 < 0) {
        ret = -ENXIO;
        goto err_free;
    }
    rtc->base = ioremap(res->start, resource_size(res));

    rtc->handle = &foo;
    chal_rtc_init(rtc->handle, (uint32_t)rtc->base, 0x00002000);

    if (!rtc->base) {
        ret = -ENOMEM;
        goto err_free;
    }

    bcmhana_rtc_enable( dev, 1 );

    bcmhana_rtc_setfreq( &dev->dev, 1);

    device_init_wakeup( &dev->dev, 1 );

    chal_rtc_intDisable( rtc->handle, CHAL_RTC_INT_MATCH );
    chal_rtc_intDisable( rtc->handle, CHAL_RTC_INT_PER );
    chal_rtc_intStatusClr ( rtc->handle, CHAL_RTC_INT_MATCH );
    chal_rtc_intStatusClr( rtc->handle, CHAL_RTC_INT_PER );

    rtc->dev = rtc_device_register( "bcmhana", &dev->dev, &bcmhana_rtcops, THIS_MODULE );

    if ( IS_ERR( rtc ) )
    {
        ret = PTR_ERR( rtc );
        pr_debug( "cannot attach rtc\n" );
        goto err_device_unregister;
    }

    rtc->max_user_freq = 8;

    ret = request_irq(rtc->irq1, rtc_alm_isr, 0,
                      "bcmhana_rtc", rtc);
    if (ret) {
        dev_printk(KERN_ERR, &rtc->dev->dev,
                   "cannot register IRQ%d for periodic rtc\n", rtc->irq1);
        goto err_irq;
    }

    ret = request_irq(rtc->irq2, rtc_per_isr, 0,
                      "bcmhana_rtc", rtc);
    if (ret) {
        dev_printk(KERN_ERR, &rtc->dev->dev,
                   "cannot register IRQ%d for match rtc\n", rtc->irq2);
        goto err_irq;
    }

    chal_rtc_ctrlSet( rtc->handle, CHAL_RTC_CTRL_RUN );
    clk_enable(rtc->clock);

    printk(KERN_INFO "RTC: driver initialized properly\n");

    return 0;

err_irq:
    free_irq(rtc->irq1, rtc);
    free_irq(rtc->irq2, rtc);
err_device_unregister:
    iounmap(rtc->base);
err_free:
    kfree(rtc);
err_out:
    return ret;
}


#ifdef CONFIG_PM

/* RTC Power management control */

static int period_cnt;

static int
bcmhana_rtc_suspend( struct platform_device *pdev, pm_message_t state )
{
     struct bcmhana_rtc *rtc = platform_get_drvdata( pdev );
     period_cnt = chal_rtc_readReg( rtc->handle, RTC_PERIODIC_TIMER_ADDR );
    return 0;
}

static int
bcmhana_rtc_resume( struct platform_device *pdev )
{
     struct bcmhana_rtc *rtc = platform_get_drvdata( pdev );
    chal_rtc_writeReg( rtc->handle, RTC_PERIODIC_TIMER_ADDR, period_cnt );
    return 0;
}
#else
#define bcmhana_rtc_suspend NULL
#define bcmhana_rtc_resume  NULL
#endif

static struct platform_driver bcmhana_rtcdrv = {
    .remove = __exit_p( bcmhana_rtc_remove ),
    .suspend = bcmhana_rtc_suspend,
    .resume = bcmhana_rtc_resume,
    .driver = {
        .name = "bcmhana-rtc",
        .owner = THIS_MODULE,
    },
};

static char __initdata banner[] = "BCMHANA RTC, (c) 2009 Broadcom Corporation\n";

static int __init
bcmhana_rtc_init( void )
{
    printk( banner );
    return platform_driver_probe( &bcmhana_rtcdrv, bcmhana_rtc_probe );
}

static void __exit
bcmhana_rtc_exit( void )
{
    platform_driver_unregister( &bcmhana_rtcdrv );
}

module_init( bcmhana_rtc_init );
module_exit( bcmhana_rtc_exit );

MODULE_DESCRIPTION( "Broadcom BCMHANA RTC Driver" );
MODULE_AUTHOR( "Broadcom Corporation" );
MODULE_LICENSE( "GPL" );
MODULE_ALIAS( "platform:bcmhana-rtc" );
