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

#include <mach/rdb/brcm_rdb_sysmap_a9.h>
#include <mach/rdb/brcm_rdb_bbl_apb.h>
#include <mach/rdb/brcm_rdb_bbl_config.h>
#include <mach/rdb/brcm_rdb_bbl_pwseq.h>
#include <mach/rdb/brcm_rdb_bbl_rtc.h>
#include <mach/rdb/brcm_rdb_fmon.h>
#include <mach/rdb/brcm_rdb_bbl_vmon.h>


static void iosetbit32(void *addr, unsigned int bits)
{
    iowrite32(ioread32(addr) | bits,  addr);
}

static void ioclrbit32(void *addr, unsigned int bits)
{
    iowrite32(ioread32(addr) & ~bits,  addr);
}

/* ---- Private Constants and Types ---------------------------------------- */
typedef uint64_t bbl_TIME_t;   /* Time in second */

/* BBL_APB Block */
typedef struct
{
   uint32_t addr;       /* Address of BBL APB Slave */
   uint32_t wr_data;    /* Data to be written into BBL APB Slave */
   uint32_t rd_data;    /* Data read from BBL APB Interface */
   uint32_t ctrl_sts;   /* APB Control & Status Register */
   uint32_t int_sts;    /* Status register indicating interrupt source from BBL */
} bbl_APB_REG_t;

/* BBL_APB Interrupts */
typedef enum
{
   BBL_INT_CMD_DONE      = 0,  /* APB command done interrupt */
   BBL_INT_RTC_PER       = 1,  /* RTC periodic interrupt */
   BBL_INT_RTC_MATCH     = 2,  /* RTC match interrupt */
   BBL_INT_VFM_FMON_HIGH = 3,  /* high frequency monitor tamper interrupt */
   BBL_INT_VFM_FMON_LOW  = 4,  /* low frequency monitor tamper interrupt */
   BBL_INT_VFM_VMON_HIGH = 5,  /* high voltage monitor tamper itnerrupt */
   BBL_INT_VFM_VMON_LOW  = 6,  /* low voltage monitor tamper interrupt */
   BBL_INT_WDG_TIMER     = 7,  /* watchdog timer interrupt */
   BBL_INT_WDG_UTC       = 8   /* uptime counter interrupt */
} bbl_INT_e;

/* BBL_CONFIG Block */
typedef enum
{
   BBL_CONFIG_RTC     = 0,  /* RTC */
   BBL_CONFIG_PWSEQ   = 1,  /* PWSEQ */
   BBL_CONFIG_VFM     = 2,  /* VFM */
   BBL_CONFIG_WDG     = 3,  /* Watchdog */
   BBL_CONFIG_BBRAM   = 4   /* BBRAM */
} bbl_CFG_e;

/* BBL_RTC Block - CTRL */
typedef enum
{
   BBL_RTC_CTRL_LOCK  = 0,  /* lock thye real-time clock counters, SET_DIV and SEC_0 */
   BBL_RTC_CTRL_UNLOCK,     /* unlock the real-time clock counters, SET_DIV and SEC_0 */
   BBL_RTC_CTRL_STOP,       /* halt the real-time clock */
   BBL_RTC_CTRL_RUN         /* start the real-time clock */
} bbl_RTC_CTRL_e;

/* BBL_RTC Block - Interrupt */
typedef enum
{
   BBL_RTC_INT_PER    = 0,  /* Periodic interrupt */
   BBL_RTC_INT_MATCH  = 1,  /* Match interrupt */
   BBL_RTC_INT_NONE
} bbl_RTC_INT_e;

/* BBL_RTC - Monotonic timer clock */
typedef enum
{
   BBL_RTC_MTC_LOCK    = 0, /* Locks writes to MTC.  Sticky bit */
   BBL_RTC_MTC_INCR         /* increment MTC by INCR_AMT, even when locked */
} bbl_RTC_MTC_CTRL_e;

/* BBL_RTC - Periodic interrupt interval */
typedef enum
{
   BBL_RTC_PER_INTERVAL_125ms     = 0x00000001,  /* Time interval 125ms */
   BBL_RTC_PER_INTERVAL_250ms     = 0x00000002,  /* Time interval 250ms */
   BBL_RTC_PER_INTERVAL_500ms     = 0x00000004,  /* Time interval 500ms */
   BBL_RTC_PER_INTERVAL_1000ms    = 0x00000008,  /* Time interval 1 sec */
   BBL_RTC_PER_INTERVAL_2000ms    = 0x00000010,  /* Time interval 2 sec */
   BBL_RTC_PER_INTERVAL_4000ms    = 0x00000020,  /* Time interval 4 sec */
   BBL_RTC_PER_INTERVAL_8000ms    = 0x00000040,  /* Time interval 8 sec */
   BBL_RTC_PER_INTERVAL_16000ms   = 0x00000080,  /* Time interval 16 sec */
   BBL_RTC_PER_INTERVAL_32000ms   = 0x00000100,  /* Time interval 32 sec */
   BBL_RTC_PER_INTERVAL_64000ms   = 0x00000200,  /* Time interval 64 sec */
   BBL_RTC_PER_INTERVAL_128000ms  = 0x00000400,  /* Time interval 128 sec */
   BBL_RTC_PER_INTERVAL_256000ms  = 0x00000800   /* Time internal 256 sec */
} bbl_RTC_PER_INTERVAL_e;

/* BBL_VFM - interrupt */
typedef enum
{
   BBL_VFM_INT_VMON_HIGH = 0,
   BBL_VFM_INT_VMON_LOW  = 1,
   BBL_VFM_INT_FMON_HIGH = 2,
   BBL_VFM_INT_FMON_LOW  = 3,
   BBL_VFM_INT_NONE
} bbl_VFM_INT_e;

/* BBL_PWSEQ - delay */
typedef enum
{
   BBL_PWSEQ_DELAY_1     = BBL_PWSEQ_DLY1_OFFSET,
   BBL_PWSEQ_DELAY_2     = BBL_PWSEQ_DLY2_OFFSET,
   BBL_PWSEQ_DELAY_NONE
} bbl_PWSEQ_DELAY_e;

/* BBL_PWSEQ - tigger */
typedef enum
{
   BBL_PWSEQ_TRIG_0         = 0,
   BBL_PWSEQ_TRIG_1         = 1,
   BBL_PWSEQ_TRIG_2         = 2,
   BBL_PWSEQ_TRIG_RTC_MATCH = 3,
   BBL_PWSEQ_TRIG_RTC_PER   = 4,
   BBL_PWSEQ_TRIG_NONE
} bbl_PWSEQ_TRIG_e;

/* BBL_PWSEQ - event */
typedef enum
{
   BBL_PWSEQ_EVENT_0     = BBL_PWSEQ_EVENT0_OFFSET,
   BBL_PWSEQ_EVENT_1     = BBL_PWSEQ_EVENT1_OFFSET,
   BBL_PWSEQ_EVENT_2     = BBL_PWSEQ_EVENT2_OFFSET,
   BBL_PWSEQ_EVENT_NONE
} bbl_PWSEQ_EVENT_e;

/* BBL_PWSEQ - reset */
typedef enum
{
   BBL_PWSEQ_RESET_EVENT_0  = 0,
   BBL_PWSEQ_RESET_EVENT_1  = 1,
   BBL_PWSEQ_RESET_EVENT_2  = 2,
   BBL_PWSEQ_RESET_ONESHOT  = 3,
   BBL_PWSEQ_RESET_SEQ_0    = 4,
   BBL_PWSEQ_RESET_SEQ_1    = 5,
   BBL_PWSEQ_RESET_SEQ_2    = 6,
   BBL_PWSEQ_RESET_TIMEOUT  = 8,
   BBL_PWSEQ_RESET_NONE
} bbl_PWSEQ_RESET_e;

#define BBL_RTC_RESET_ACCESS_STATUS     0xACCE55ED

static   bbl_APB_REG_t *pBblApbReg;

/* Macros for pBblApbReg->addr register */
#define  bbl_REG_SET_APB_SLAVE_ADDRESS(add)  ( iosetbit32 ( &pBblApbReg->addr, ( (add) & BBL_APB_ADDR_APB_ADDR_MASK ) ) )

/* Macros for pBblApbReg->wr_data register */
#define  bbl_REG_GET_WR_DATA()                ( ioread32 ( &pBblApbReg->wr_data ) )
#define  bbl_REG_SET_WR_DATA(val)             ( iowrite32 ( (val), &pBblApbReg->wr_data ) )

/* Macros for pBblApbReg->rd_data register */
#define  bbl_REG_GET_RD_DATA()                ( ioread32 ( &pBblApbReg->rd_data ) )

/* Macros for pBblApbReg->ctrl_sts register */
static void bbl_REG_COMMAND_WRITE(void)
{
    ioclrbit32 ( &pBblApbReg->ctrl_sts, BBL_APB_CTRL_STS_BBL_CMD_START_MASK | BBL_APB_CTRL_STS_BBL_APB_CMD_MASK );
    iosetbit32 ( &pBblApbReg->ctrl_sts, BBL_APB_CTRL_STS_BBL_CMD_START_MASK | BBL_APB_CTRL_STS_BBL_APB_CMD_MASK );
}

static void bbl_REG_COMMAND_READ(void)
{
    ioclrbit32 ( &pBblApbReg->ctrl_sts, BBL_APB_CTRL_STS_BBL_CMD_START_MASK | BBL_APB_CTRL_STS_BBL_APB_CMD_MASK );
    iosetbit32 ( &pBblApbReg->ctrl_sts, BBL_APB_CTRL_STS_BBL_CMD_START_MASK );
}

static int bbl_REG_IS_COMMAND_DONE(void)
{
    return ioread32 ( &pBblApbReg->ctrl_sts ) & BBL_APB_CTRL_STS_BBL_CMD_DONE_MASK;
}

static void bbl_REG_COMMAND_DONE_CLEAR()
{
    iosetbit32( &pBblApbReg->ctrl_sts, BBL_APB_CTRL_STS_BBL_CMD_DONE_MASK );
}

#define RTC_PERIODIC_TIMER_ADDR     (BBL_RTC_BASE_ADDR+BBL_RTC_PER_OFFSET)
#define RTC_MATCH_REGISTER_ADDR     (BBL_RTC_BASE_ADDR+BBL_RTC_MATCH_OFFSET)
#define RTC_CLEAR_INTR_ADDR         (BBL_RTC_BASE_ADDR+BBL_RTC_CLR_INT_OFFSET)
#define RTC_INTERRUPT_STATUS_ADDR   (BBL_RTC_BASE_ADDR+BBL_RTC_INT_STS_OFFSET)
#define RTC_CONTROL_ADDR            (BBL_RTC_BASE_ADDR+BBL_RTC_CTRL_OFFSET)

/* ---- Public Variable Externs -------------------------------------------- */

/* ---- Private Function Prototypes ---------------------------------------- */
static inline void bbl_rtc_ctrlSet ( bbl_RTC_CTRL_e cmd );
static inline int bbl_rtc_intIsEnabled ( bbl_RTC_INT_e interrupt );

/* ---- Public Function Prototypes ----------------------------------------- */

/* ---- Indirect register access functions --------------------------------- */

static inline void bbl_readyForCommand ( void )
{
    printk( "BBL: Waiting to issue command \n" );
    while ( bbl_REG_IS_COMMAND_DONE() ) udelay ( 1 );
    bbl_REG_COMMAND_DONE_CLEAR();
    printk( "BBL: Issuing command \n" );
}

static inline uint32_t bbl_readReg ( uint32_t regAddr )
{
    /* Make sure BBL interface is ready to accept commands */
    bbl_readyForCommand ();
    bbl_REG_SET_APB_SLAVE_ADDRESS ( regAddr );
    bbl_REG_COMMAND_READ();
    /* Wait until command is processed */
    bbl_readyForCommand ();
    /* Read the data */
    return ( bbl_REG_GET_RD_DATA () );
}

static inline void bbl_writeReg ( uint32_t regAddr, uint32_t val )
{
    /* Make sure BBL interface is ready to accept commands */
    bbl_readyForCommand ();
    bbl_REG_SET_APB_SLAVE_ADDRESS ( regAddr );
    bbl_REG_SET_WR_DATA ( val );
    bbl_REG_COMMAND_WRITE ();
}

/****************************************************************************/
/**
*  @brief   Set the RTC clock divider value
*
*  @return  none
*/
/****************************************************************************/
static inline void bbl_rtc_divSet ( uint32_t val )
{
   bbl_rtc_ctrlSet( BBL_RTC_CTRL_STOP );
   bbl_writeReg ( BBL_RTC_BASE_ADDR | BBL_RTC_SET_DIV_OFFSET, val );
   /*bbl_rtc_ctrlSet( BBL_RTC_CTRL_RUN );*/
}

/****************************************************************************/
/**
*  @brief   Get the RTC clock divider value
*
*  @return  Clock divider value
*/
/****************************************************************************/
static inline uint32_t bbl_rtc_divGet ( void )
{
   return ( bbl_readReg ( BBL_RTC_BASE_ADDR | BBL_RTC_SET_DIV_OFFSET ) );
}

/****************************************************************************/
/**
*  @brief   Set the RTC seconds value
*
*  @return  none
*/
/****************************************************************************/
static inline void bbl_rtc_secSet ( uint32_t val )
{
   bbl_rtc_ctrlSet( BBL_RTC_CTRL_STOP );
   bbl_writeReg ( BBL_RTC_BASE_ADDR | BBL_RTC_SEC_0_OFFSET, val );
   /*bbl_rtc_ctrlSet( BBL_RTC_CTRL_RUN );*/
}

/****************************************************************************/
/**
*  @brief   Get the RTC seconds value
*
*  @return  RTC seconds value
*/
/****************************************************************************/
static inline uint32_t bbl_rtc_secGet ( void )
{
   return ( bbl_readReg ( BBL_RTC_BASE_ADDR | BBL_RTC_SEC_0_OFFSET ) );
}

/****************************************************************************/
/**
*  @brief   Sets RTC control
*
*  @return  none
*/
/****************************************************************************/
static inline void bbl_rtc_ctrlSet ( bbl_RTC_CTRL_e cmd )
{
   uint32_t val;
   uint32_t addr;

   addr = BBL_RTC_BASE_ADDR | BBL_RTC_CTRL_OFFSET;

   /* Read the data */
   val = bbl_readReg ( addr );

   switch ( cmd )
   {
      case BBL_RTC_CTRL_LOCK:
         /* Set the RTC lock bit */
 	      val |= BBL_RTC_CTRL_BBL_RTC_LOCK_MASK;
         break;
      case BBL_RTC_CTRL_UNLOCK:
         /* Clear the RTC lock bit */
         val &= ~BBL_RTC_CTRL_BBL_RTC_LOCK_MASK;
         break;
      case BBL_RTC_CTRL_STOP:
         /* Set the RTC stop bit */
 	      val |= BBL_RTC_CTRL_BBL_RTC_STOP_MASK;
         break;
      case BBL_RTC_CTRL_RUN:
         /* Clear the RTC stop bit */
         val &= ~BBL_RTC_CTRL_BBL_RTC_STOP_MASK;
         break;
   }

   /* Write the data */
   bbl_writeReg ( addr, val );
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
static inline void bbl_rtc_periodInterruptValSet ( bbl_RTC_PER_INTERVAL_e interval )
{
   bbl_writeReg( (BBL_RTC_BASE_ADDR | BBL_RTC_PER_OFFSET), interval );
}

/****************************************************************************/
/**
*  @brief   Gets the perioic timer configuration
*
*  @return  Perioidic interrupt value
*/
/****************************************************************************/
static inline uint32_t bbl_rtc_periodInterruptValGet ( void )
{
   return( bbl_readReg( BBL_RTC_BASE_ADDR | BBL_RTC_PER_OFFSET ) );
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
static inline void bbl_rtc_matchInterruptValSet ( bbl_TIME_t sec )
{
   bbl_writeReg( (BBL_RTC_BASE_ADDR | BBL_RTC_MATCH_OFFSET), (sec & BBL_RTC_MATCH_BBL_RTC_MATCH_MASK) );
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
static inline uint32_t bbl_rtc_matchInterruptValGet ( void )
{
   return( bbl_readReg( BBL_RTC_BASE_ADDR | BBL_RTC_MATCH_OFFSET ) );
}

/****************************************************************************/
/**
*  @brief   Clears a RTC interrupt
*
*  @return  none
*/
/****************************************************************************/
static inline void bbl_rtc_intStatusClr ( bbl_RTC_INT_e interrupt )
{
   bbl_writeReg( (BBL_RTC_BASE_ADDR | BBL_RTC_CTRL_OFFSET), (1<<interrupt) );
}

/****************************************************************************/
/**
*  @brief   Get the status of a RTC interrupt
*
*  @return  1: an interrupt occurred
*           0: no interrupt occurred
*/
/****************************************************************************/
static inline int bbl_rtc_intStatusGet ( bbl_RTC_INT_e interrupt )
{
   return( ( bbl_readReg( BBL_RTC_BASE_ADDR | BBL_RTC_INT_STS_OFFSET ) & (1<<interrupt) ) ? 1 : 0 );
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
static inline bbl_RTC_INT_e bbl_rtc_interruptStatusGet ( void )
{
   if( bbl_rtc_intStatusGet( BBL_RTC_INT_PER ) & bbl_rtc_intIsEnabled( BBL_RTC_INT_PER ) )
   {
      return BBL_RTC_INT_PER;
   }
   else if( bbl_rtc_intStatusGet( BBL_RTC_INT_MATCH ) & bbl_rtc_intIsEnabled( BBL_RTC_INT_MATCH ) )
   {
      return BBL_RTC_INT_MATCH;
   }
   return BBL_RTC_INT_NONE;
}

/****************************************************************************/
/**
*  @brief   Enable/disable a RTC interrupt
*
*  @return  none
*/
/****************************************************************************/
static inline void bbl_rtc_intEnable ( bbl_RTC_INT_e interrupt )
{
   uint32_t val, addr;

   addr = BBL_RTC_BASE_ADDR | BBL_RTC_INT_ENABLE_OFFSET;

   val = bbl_readReg ( addr );
   val |= (1 << interrupt);
   bbl_writeReg ( addr, val );
}

static inline void bbl_rtc_intDisable ( bbl_RTC_INT_e interrupt )
{
   uint32_t val, addr;

   addr = BBL_RTC_BASE_ADDR | BBL_RTC_INT_ENABLE_OFFSET;

   val = bbl_readReg ( addr );
   val &= ~(1 << interrupt);
   bbl_writeReg ( addr, val );
}

/****************************************************************************/
/**
*  @brief   Check whether RTC interrupt is enabled
*
*  @return  1: interrupt is enabled
*           0: interrupt is disabled
*/
/****************************************************************************/
static inline int bbl_rtc_intIsEnabled ( bbl_RTC_INT_e interrupt )
{
   return( ( bbl_readReg ( BBL_RTC_BASE_ADDR | BBL_RTC_INT_ENABLE_OFFSET ) & (1<<interrupt) ) ? 1 : 0 );
}

/****************************************************************************/
/**
*  @brief   Checks whether BBL reset has been asserted
*
*  @return  1: reset is asserted
*           0: reset is not asserted
*/
/****************************************************************************/
static inline int bbl_rtc_rstIsAsserted ( void )
{
   return( ( bbl_readReg ( BBL_RTC_BASE_ADDR | BBL_RTC_RESET_ACCESS_OFFSET ) == BBL_RTC_RESET_ACCESS_STATUS ) ? 1 : 0 );
}

/****************************************************************************/
/**
*  @brief   Set monotonic counter control
*
*  @return  none
*/
/****************************************************************************/
static inline void bbl_rtc_mtcCtrlSet ( bbl_RTC_MTC_CTRL_e cmd )
{
   uint32_t val, addr;

   addr = BBL_RTC_BASE_ADDR | BBL_RTC_MTC_CTRL_OFFSET;

   /* Read the data */
   val = bbl_readReg ( addr );

   switch ( cmd )
   {
      case BBL_RTC_MTC_LOCK:
         /* locks monotonic counter - sticky bit */
 	      val |= BBL_RTC_MTC_CTRL_BBL_MT_CTR_LOCK_MASK;
         break;
      case BBL_RTC_MTC_INCR:
         /* increment monotonic counter by incr_amt - self clear */
         val |= BBL_RTC_MTC_CTRL_BBL_MT_CTR_INCR_MASK;
         break;
   }

   /* Write the data */
   bbl_writeReg ( addr, val );
}

/****************************************************************************/
/**
*  @brief   Set monotonic counter increment amount
*
*  @return  none
*/
/****************************************************************************/
static inline void bbl_rtc_mtcIncrSet ( uint16_t amount )
{
   uint32_t val, addr;

   addr = BBL_RTC_BASE_ADDR | BBL_RTC_MTC_CTRL_OFFSET;

   /* Read the data */
   val = bbl_readReg ( addr );

   val &= ~BBL_RTC_MTC_CTRL_BBL_MT_CTR_INCR_AMT_MASK;
   val |= (uint32_t) amount << BBL_RTC_MTC_CTRL_BBL_MT_CTR_INCR_AMT_SHIFT;

   /* Write the data */
   bbl_writeReg ( addr, val );
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
static inline void bbl_rtc_mtcTimeSet ( bbl_TIME_t time )
{
   bbl_writeReg ( BBL_RTC_BASE_ADDR | BBL_RTC_MTC_MSB_OFFSET, time >> 32 );
   bbl_writeReg ( BBL_RTC_BASE_ADDR | BBL_RTC_MTC_LSB_OFFSET, time & 0xffffffff );
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
static inline bbl_TIME_t bbl_rtc_mtcTimeGet ( void )
{
   uint32_t mtcMsb, mtcLsb;
   bbl_TIME_t time;

   mtcMsb = bbl_readReg ( BBL_RTC_BASE_ADDR | BBL_RTC_MTC_MSB_OFFSET );
   mtcLsb = bbl_readReg ( BBL_RTC_BASE_ADDR | BBL_RTC_MTC_LSB_OFFSET );

   time = ((bbl_TIME_t)mtcMsb) << 32 | (bbl_TIME_t) mtcLsb;

   return time;
}

/****************************************************************************/
/**
*  @brief   Enable/disable SEQ0 (RTC alarm out) to also trigger the
*           BBL_PWREN[1:0] pins
*
*  @return  none
*/
/****************************************************************************/
static inline void bbl_pwseq_alarmOutEnable ( void )
{
   uint32_t addr, val;

   addr = BBL_PWSEQ_BASE_ADDR | BBL_PWSEQ_TIMEOUT_OFFSET;
   val = bbl_readReg( addr );
   val |= BBL_PWSEQ_TIMEOUT_OFFSET;
   bbl_writeReg( addr, val );
}

static inline void bbl_pwseq_alarmOutDisable ( void )
{
   uint32_t addr, val;

   addr = BBL_PWSEQ_BASE_ADDR | BBL_PWSEQ_TIMEOUT_OFFSET;
   val = bbl_readReg( addr );
   val &= ~BBL_PWSEQ_TIMEOUT_OFFSET;
   bbl_writeReg( addr, val );
}

/****************************************************************************/
/**
*  @brief   Check if RTC alarm out is enabled
*
*  @return  1: RTC alarm out is enabled
*           0: RTC alarm out is disabled
*/
/****************************************************************************/
static inline uint32_t bbl_pwseq_alarmOutIsEnable ( void )
{
   return( ( bbl_readReg ( BBL_PWSEQ_BASE_ADDR | BBL_PWSEQ_TIMEOUT_OFFSET ) & BBL_PWSEQ_TIMEOUT_OFFSET ) ? 1 : 0 );
}


/****************************************************************************/
/**
*  @brief   A common function to update BBL clock disable register
*
*/
/****************************************************************************/
static inline void bbl_cfg_clkEnable ( bbl_CFG_e device, uint32_t enable )
{
   uint32_t val;
   uint32_t addr;

   addr = BBL_CONFIG_BASE_ADDR | BBL_CONFIG_CLK_DISABLE_OFFSET;

   /* Read the data */
   val = bbl_readReg ( addr );

   /* enable/disable clock */
   if( enable )
   {
      val &= ~(1 << device);
   }
   else
   {
      val |= (1 << device);
   }

   /* Write the data */
   bbl_writeReg ( addr, val );
}

/****************************************************************************/
/**
*  @brief   A common function to soft reset APB peripherals
*
*/
/****************************************************************************/
static inline void bbl_cfg_softReset ( bbl_CFG_e device )
{
   /* Write the data */
   bbl_writeReg ( BBL_CONFIG_BASE_ADDR | BBL_CONFIG_SOFT_RST_OFFSET, (1 << device) );
}

struct bcmhana_rtc
{
    struct device *dev;
    void __iomem  *base;
    unsigned int irq1;
    unsigned int irq2;
    unsigned int max_user_freq;
};

static unsigned int epoch = 1970;
static DEFINE_SPINLOCK( bcmhana_rtc_lock );
static struct clk *bbl_clk;

/* IRQ Handlers */

/*
 * RTC IRQ hanlder. This routine is invoked when a RTC oneshot timer completes
 */
static irqreturn_t
rtc_alm_isr( int irq, void *data )
{
    struct rtc_device *rdev = data;

    /* Disable alarm interrupts because they are oneshot */
    bbl_rtc_intDisable( BBL_RTC_INT_MATCH);
    bbl_rtc_intStatusClr( BBL_RTC_INT_MATCH);
    if ( bbl_rtc_matchInterruptValGet() )
    {
        pr_debug( "%s: oneshot interrupted\n", __func__ );
        rtc_update_irq( rdev, 1, RTC_AF | RTC_IRQF );
    }
    return IRQ_HANDLED;
}

/*
 * RTC IRQ hanlder. This routine is invoked when periodic interrupts occur
 */
static irqreturn_t
rtc_per_isr( int irq, void *data )
{
    struct rtc_device *rdev = data;
    if ( bbl_rtc_periodInterruptValGet()  )
    {
        pr_debug( "%s: periodic interrupted\n", __func__ );
        rtc_update_irq( rdev, 1, RTC_PF | RTC_IRQF );
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
        bbl_rtc_intEnable( BBL_RTC_INT_MATCH);
    }
    else
    {
        bbl_rtc_intDisable( BBL_RTC_INT_MATCH); 
    }
}

static int
bcmhana_rtc_setpie( struct device *dev, int enabled )
{
    pr_debug( "%s: pie=%d\n", __func__, enabled );

    spin_lock_irq( &bcmhana_rtc_lock );

    if ( enabled )
    {
        bbl_rtc_intEnable( BBL_RTC_INT_PER ); /* enables the interrupt */
    }
    else
    {
        bbl_rtc_intDisable( BBL_RTC_INT_PER );  /* disables the interrupt */
    }
    spin_unlock_irq( &bcmhana_rtc_lock );

    return 0;
}

static int
bcmhana_rtc_setfreq( struct device *dev, int freq )
{
    bbl_RTC_PER_INTERVAL_e interval = 0xffffffff;   /* invalid */

    pr_debug( "%s: freq=%d\n", __func__, freq );
    spin_lock_irq( &bcmhana_rtc_lock );
    switch ( freq )
    {
    case 1:
        interval = BBL_RTC_PER_INTERVAL_1000ms;
        break;
    case 2:
        interval = BBL_RTC_PER_INTERVAL_2000ms;
        break;
    case 4:
        interval = BBL_RTC_PER_INTERVAL_4000ms;
        break;
    case 8:
        interval = BBL_RTC_PER_INTERVAL_8000ms;
        break;
    case 16:
        interval = BBL_RTC_PER_INTERVAL_16000ms;
        break;
    case 32:
        interval = BBL_RTC_PER_INTERVAL_32000ms;
        break;
    case 64:
        interval = BBL_RTC_PER_INTERVAL_64000ms;
        break;
    case 128:
        interval = BBL_RTC_PER_INTERVAL_128000ms;
        break;
    }
    spin_unlock_irq( &bcmhana_rtc_lock );

    if ( interval != 0xffffffff )
    {
        pr_debug( "%s: OKAY freq=%d interval=%d\n", __func__, freq, interval );
        bbl_rtc_periodInterruptValSet( interval );
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
    bbl_RTC_PER_INTERVAL_e interval;
    *freq = 0xffffffff;          /* invalid */

    spin_lock_irq( &bcmhana_rtc_lock );
    interval = bbl_readReg( RTC_PERIODIC_TIMER_ADDR );
    switch ( interval )
    {
    case BBL_RTC_PER_INTERVAL_125ms:   /* avoid compiler warnings */
    case BBL_RTC_PER_INTERVAL_250ms:
    case BBL_RTC_PER_INTERVAL_500ms:
        break;
    case BBL_RTC_PER_INTERVAL_1000ms:
        *freq = 1;
        break;
    case BBL_RTC_PER_INTERVAL_2000ms:
        *freq = 2;
        break;
    case BBL_RTC_PER_INTERVAL_4000ms:
        *freq = 4;
        break;
    case BBL_RTC_PER_INTERVAL_8000ms:
        *freq = 8;
        break;
    case BBL_RTC_PER_INTERVAL_16000ms:
        *freq = 16;
        break;
    case BBL_RTC_PER_INTERVAL_32000ms:
        *freq = 32;
        break;
    case BBL_RTC_PER_INTERVAL_64000ms:
        *freq = 64;
        break;
    case BBL_RTC_PER_INTERVAL_128000ms:
        *freq = 128;
        break;
    case BBL_RTC_PER_INTERVAL_256000ms:
        *freq = 256;
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
    elapsed_sec = bbl_rtc_mtcTimeGet();

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

    bbl_rtc_mtcCtrlSet( current_sec - epoch_sec );

    pr_debug( "%s: current_sec=%u, epoch_sec=%u\n", __func__, current_sec, epoch_sec );

    pr_debug( "set time %02d.%02d.%02d %02d/%02d/%02d\n", time->tm_year, time->tm_mon, time->tm_mday, time->tm_hour, time->tm_min, time->tm_sec );

    return 0;
}

static int
bcmhana_rtc_getalarm( struct device *dev, struct rtc_wkalrm *alrm )
{
    unsigned int epoch_sec, elapsed_sec, alarm_elapsed_sec;
    bbl_TIME_t alm_reg_secs;
    struct rtc_time *alm_tm = &alrm->time;
    alrm->enabled = bbl_rtc_intIsEnabled ( BBL_RTC_INT_MATCH );
//    alrm->pending = ( bbl_readReg( RTC_INTERRUPT_STATUS_ADDR ) & RTC_CMD_ONESHOT_INTERRUPT_STATUS ) ? 1 : 0;

    epoch_sec = mktime( epoch, 1, 1, 0, 0, 0 );
    elapsed_sec = bbl_rtc_mtcTimeGet();

    alm_reg_secs =  bbl_rtc_matchInterruptValGet();

    /* Handle carry over */
    if ((elapsed_sec & 0x0ffff) > alm_reg_secs)
    {
        elapsed_sec += 0x10000;
    }
    elapsed_sec &= ~0xffff; /* clear lower 16 bits for 16-bit alarm match register below */
    alarm_elapsed_sec = elapsed_sec + alm_reg_secs;
    pr_debug( "%s: epoch_sec=%u, elapsed_sec=%u, alm_reg_secs=%u=0x%x, alarm_elapsed_sec=%u=0x%x\n",
              __func__, epoch_sec, elapsed_sec, alm_reg_secs, alm_reg_secs, alarm_elapsed_sec, alarm_elapsed_sec );

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
    bbl_TIME_t alm_secs;

    pr_debug( "%s: %d, %02x/%02x/%02x %02x.%02x.%02x\n",
              __func__, alrm->enabled, time->tm_mday & 0xff, time->tm_mon & 0xff, time->tm_year & 0xff, time->tm_hour & 0xff, time->tm_min & 0xff,
              time->tm_sec );

    epoch_sec = mktime( epoch, 1, 1, 0, 0, 0 );
    elapsed_sec = bbl_rtc_mtcTimeGet();
    alm_secs = mktime( time->tm_year + 1900, time->tm_mon + 1, time->tm_mday, time->tm_hour, time->tm_min, time->tm_sec );

    pr_debug( "%s: epoch_sec=%u, elapsed_sec=%u, alm_secs=%u\n", __func__, epoch_sec, elapsed_sec, alm_secs );

    bbl_rtc_intDisable( BBL_RTC_INT_MATCH );
    bbl_rtc_intStatusClr ( BBL_RTC_INT_MATCH );

    bbl_rtc_matchInterruptValSet( alm_secs );

    if (alrm->enabled) {
        bbl_rtc_intEnable( BBL_RTC_INT_MATCH );
    }


    return 0;
}

static int
bcmhana_rtc_proc( struct device *dev, struct seq_file *seq )
{
    seq_printf( seq, "\nperiodic timer: 0x%x\n", bbl_readReg( RTC_PERIODIC_TIMER_ADDR ) );
    seq_printf( seq, "match register: 0x%x\n", bbl_readReg( RTC_MATCH_REGISTER_ADDR ) );
    seq_printf( seq, "clear intr register: 0x%x\n", bbl_readReg( RTC_CLEAR_INTR_ADDR ) );
    seq_printf( seq, "intr status register: 0x%x\n", bbl_readReg( RTC_INTERRUPT_STATUS_ADDR ) );
    seq_printf( seq, "control addr register: 0x%x\n", bbl_readReg( RTC_CONTROL_ADDR ) );
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
        bbl_rtc_intDisable( BBL_RTC_INT_PER );
        spin_unlock_irq( &bcmhana_rtc_lock );
        return 0;
    case RTC_PIE_ON:
        spin_lock_irq( &bcmhana_rtc_lock );
        bbl_rtc_intEnable( BBL_RTC_INT_PER );
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
        bbl_rtc_ctrlSet( BBL_RTC_CTRL_STOP );
    }
    else
    {
        bbl_rtc_ctrlSet( BBL_RTC_CTRL_RUN );
    }
}

static int __exit
bcmhana_rtc_remove( struct platform_device *dev )
{
    struct rtc_device *rtc = platform_get_drvdata( dev );

    device_init_wakeup( &dev->dev, 0 );

    platform_set_drvdata( dev, NULL );

    bcmhana_rtc_setpie( &dev->dev, 0 );
    bcmhana_rtc_setaie( 0 );

    //  free_irq( rtc->irq2, rtc );
    // free_irq( rtc->irq1, rtc );
    rtc_device_unregister( rtc );
   
    clk_disable(bbl_clk);
    clk_put(bbl_clk);

    return 0;
}

static int __devinit bcmhana_rtc_probe(struct platform_device *dev)
{
    struct bcmhana_rtc *rtc;
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

    rtc->dev = &dev->dev;
    rtc->irq1 = platform_get_irq(dev, 0);
    if (rtc->irq1 < 0) {
        ret = -ENXIO;
        goto err_free;
    }
    rtc->irq2 = platform_get_irq(dev, 0);
    if (rtc->irq2 < 0) {
        ret = -ENXIO;
        goto err_free;
    }
    rtc->base = ioremap(res->start, resource_size(res));

    pBblApbReg = rtc->base;

    printk( "RTC got here 6 VA:0x%08p PA:0x%08p \n", pBblApbReg, res->start );

    printk( "RTC addr    0x%08x\n", pBblApbReg->addr);      /* Address of BBL APB Slave */
    printk( "RTC wr_data 0x%08x\n", pBblApbReg-> wr_data);  /* Data to be written into BBL APB Slave */
    printk( "RTC rd_data 0x%08x\n", pBblApbReg->rd_data);   /* Data read from BBL APB Interface */
    printk( "RTC ctrl    0x%08x\n", pBblApbReg->ctrl_sts);  /* APB Control & Status Register */
    printk( "RTC int     0x%08x\n", pBblApbReg->int_sts);   /* Status register indicating interrupt source from BBL */

    if (!rtc->base) {
        ret = -ENOMEM;
        goto err_free;
    }

    printk( "RTC got here 7\n" );
    bcmhana_rtc_enable( dev, 1 );

    printk( "RTC got here 8\n" );
    bcmhana_rtc_setfreq( &dev->dev, 1 );

    printk( "RTC got here 9\n" );
    device_init_wakeup( &dev->dev, 1 );

    printk( "RTC got here 10\n" );
    bbl_rtc_intDisable( BBL_RTC_INT_MATCH );
    printk( "RTC got here 11\n" );
    bbl_rtc_intDisable( BBL_RTC_INT_PER );
    printk( "RTC got here 12\n" );
    bbl_rtc_intStatusClr ( BBL_RTC_INT_MATCH );
    printk( "RTC got here 13\n" );
    bbl_rtc_intStatusClr( BBL_RTC_INT_PER );
    printk( "RTC got here 14\n" );
    rtc = rtc_device_register( "bcmhana", &dev->dev, &bcmhana_rtcops, THIS_MODULE );

    printk( "RTC got here 15\n" );
    if ( IS_ERR( rtc ) )
    {
        ret = PTR_ERR( rtc );
        pr_debug( "cannot attach rtc\n" );
        goto err_device_unregister;
    }

    printk( "RTC got here 16\n" );
    rtc->max_user_freq = 128;

    printk( "RTC got here 17\n" );
    ret = request_irq(rtc->irq1, rtc_per_isr, IRQF_DISABLED,
                      "bcmhana_rtc", rtc);
    printk( "RTC got here 18\n" );
    if (ret) {
        dev_printk(KERN_ERR, rtc->dev,
                   "cannot register IRQ%d for watchdog\n", rtc->irq1);
        goto err_irq;
    }

    printk( "RTC got here 19\n" );
    ret = request_irq(rtc->irq2, rtc_alm_isr, IRQF_DISABLED,
                      "bcmhana_rtc", rtc);
    printk( "RTC got here 20\n" );
    if (ret) {
        dev_printk(KERN_ERR, rtc->dev,
                   "cannot register IRQ%d for watchdog\n", rtc->irq2);
        goto err_irq;
    }
    printk( "RTC got here 21\n" );

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
    period_cnt = bbl_readReg( RTC_PERIODIC_TIMER_ADDR );
    return 0;
}

static int
bcmhana_rtc_resume( struct platform_device *pdev )
{
    bbl_writeReg( RTC_PERIODIC_TIMER_ADDR, period_cnt );
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
