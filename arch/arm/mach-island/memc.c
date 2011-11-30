/*****************************************************************************
* Copyright 2010 - 2011 Broadcom Corporation.  All rights reserved.
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

#include <linux/version.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/delay.h>
#include <linux/sysctl.h>
#include <linux/kthread.h>
#include <asm-generic/div64.h>
#include <mach/rdb/brcm_rdb_csr.h>
#include <linux/io.h>
#include <mach/rdb/brcm_rdb_sysmap.h>
#include <mach/rdb/brcm_rdb_map.h>

#ifndef CONFIG_MAP_LITTLE_ISLAND_MODE
#define NUM_MEMC                  2
#else
#define NUM_MEMC                  1
#endif
#define DEFAULT_PERIOD            400000000

typedef struct
{
   volatile uint32_t control;
   volatile uint32_t period;
   volatile uint32_t done;
   volatile uint32_t read_data_cycles;
   volatile uint32_t write_data_cycles;
   volatile uint32_t page_hit_data_cycles;
   volatile uint32_t page_miss_data_cycles;
   volatile uint32_t page_hit_transactions;
   volatile uint32_t page_miss_transactions;
   volatile uint32_t auto_ref_cycles;
   volatile uint32_t bank_active_cycles;
   volatile uint32_t entries_min_max;
   volatile uint32_t entries_urgent;
   volatile uint32_t entries_waw;
   volatile uint32_t entries_rar;
   volatile uint32_t entries_addr_coh;
   volatile uint32_t latency_urgent_times;
   /* WARNING: This is not a memory mapped register */
   volatile uint32_t total_cycles;

} MEMC_STATISTICS;

typedef enum
{
   STATS_MIN = 0,
   STATS_MAX,
   STATS_AVG,
   STATS_CUR,
   STATS_NUM

} STATS_COLLECTED;

typedef struct
{
   uint16_t          memcNum;
   uint32_t          period;
   uint32_t          running;
   volatile uint32_t killWorkerThread;
   uint32_t          iterations;
   volatile uint32_t memcPowerDownReg;

   struct   ctl_table_header *sysCtlHeader;
   struct   proc_dir_entry   *procDir;
   void    *memc_base_ioremap;
   volatile MEMC_STATISTICS  *memcStatistics;
   volatile uint32_t         *memcPowerDown;
   struct   task_struct      *thread;
  
   MEMC_STATISTICS statsCollected[ STATS_NUM ];

} MEMC_STATE;

static MEMC_STATE gMemcState[ NUM_MEMC ];

static int proc_enable( ctl_table *table, int write, void __user *buffer, size_t *lenp, loff_t *ppos );
static int proc_period( ctl_table *table, int write, void __user *buffer, size_t *lenp, loff_t *ppos );

static struct ctl_table gSysCtlChild0[] =
{
   {
      .procname      = "stats_period",
      .data          = &gMemcState[0].period,
      .maxlen        = sizeof(int),
      .mode          = 0644,
      .proc_handler  = &proc_period,
      .extra1        = &gMemcState[0]
   },
   {
      .procname      = "stats_enable",
      .data          = &gMemcState[0].running,
      .maxlen        = sizeof(int),
      .mode          = 0644,
      .proc_handler  = &proc_enable,
      .extra1        = &gMemcState[0]
   },
   {}
};

static struct ctl_table gSysCtlChild1[] =
{
   {
      .procname      = "stats_period",
      .data          = &gMemcState[1].period,
      .maxlen        = sizeof(int),
      .mode          = 0644,
      .proc_handler  = &proc_period,
      .extra1        = &gMemcState[1]
   },
   {
      .procname      = "stats_enable",
      .data          = &gMemcState[1].running,
      .maxlen        = sizeof(int),
      .mode          = 0644,
      .proc_handler  = &proc_enable,
      .extra1        = &gMemcState[1]
   },
   {}
};

static struct ctl_table gSysCtl0[] =
{
    {
      .procname   = "memc0",
      .mode       = 0555,
      .child      = gSysCtlChild0,
    },
    {}
};

static struct ctl_table gSysCtl1[] =
{
    {
      .procname   = "memc1",
      .mode       = 0555,
      .child      = gSysCtlChild1,
    },
    {}
};

/****************************************************************************
*
*  stats_copy
*
***************************************************************************/

static void stats_copy( MEMC_STATISTICS *dst, volatile MEMC_STATISTICS *src )
{
   dst->read_data_cycles      = src->read_data_cycles;
   dst->write_data_cycles     = src->write_data_cycles;
   dst->page_hit_data_cycles  = src->page_hit_data_cycles;
   dst->page_miss_data_cycles = src->page_miss_data_cycles;
   dst->page_hit_transactions = src->page_hit_transactions;
   dst->page_miss_transactions= src->page_miss_transactions;
   dst->auto_ref_cycles       = src->auto_ref_cycles;
   dst->bank_active_cycles    = src->bank_active_cycles;
   dst->entries_min_max       = src->entries_min_max;
   dst->entries_urgent        = src->entries_urgent;
   dst->entries_waw           = src->entries_waw;
   dst->entries_rar           = src->entries_rar;
   dst->entries_addr_coh      = src->entries_addr_coh;
   dst->latency_urgent_times  = src->latency_urgent_times;

   dst->total_cycles = src->page_hit_data_cycles + src->page_miss_data_cycles +
                       src->auto_ref_cycles + src->bank_active_cycles;
}

/****************************************************************************
*
* div64
*
***************************************************************************/

static uint64_t div64( uint64_t v, uint32_t d )
{
   uint64_t n;
   uint32_t rem;

   n = v;
   rem = __div64_32( &n, d );

   return n;
}

/****************************************************************************
*
*  worker_thread
*
***************************************************************************/

static void worker_thread( void *pArg )
{
   MEMC_STATE *pS = (MEMC_STATE*)pArg;

   while( !pS->killWorkerThread )
   {
      /* Disable statistics */
      pS->memcStatistics->control = 0x0;

      msleep(500);

      /* Enable statistics */
      pS->memcStatistics->control = 0x1;

      /* Wait for statistics collection to finish */
      while( !pS->memcStatistics->done )
      {
         msleep(100);
      }

      pS->iterations++;

      /* Statistics collection complete. Save information */
      stats_copy( &pS->statsCollected[ STATS_CUR ], pS->memcStatistics );

      /* Update min and max based on total cycles */
      if( pS->statsCollected[ STATS_MIN ].total_cycles > pS->statsCollected[ STATS_CUR ].total_cycles )
      {
         stats_copy( &pS->statsCollected[ STATS_MIN ], pS->memcStatistics );
      }
      else if( pS->statsCollected[ STATS_MAX ].total_cycles < pS->statsCollected[ STATS_CUR ].total_cycles )
      {
         stats_copy( &pS->statsCollected[ STATS_MAX ], pS->memcStatistics );
      }

      /* Update average statistics */
      {
         MEMC_STATISTICS *sa = &pS->statsCollected[ STATS_AVG ];
         MEMC_STATISTICS *sc = &pS->statsCollected[ STATS_CUR ];
         uint64_t iter = pS->iterations - 1;

         sa->read_data_cycles = div64( ((uint64_t)sa->read_data_cycles * iter) + (uint64_t)sc->read_data_cycles, pS->iterations );
         sa->write_data_cycles = div64( ((uint64_t)sa->write_data_cycles * iter) + sc->write_data_cycles, pS->iterations );
         sa->page_hit_data_cycles = div64( ((uint64_t)sa->page_hit_data_cycles * iter) + sc->page_hit_data_cycles, pS->iterations );
         sa->page_miss_data_cycles = div64( ((uint64_t)sa->page_miss_data_cycles * iter) + sc->page_miss_data_cycles, pS->iterations );
         sa->page_hit_transactions = div64( ((uint64_t)sa->page_hit_transactions * iter) + sc->page_hit_transactions, pS->iterations );
         sa->page_miss_transactions = div64( ((uint64_t)sa->page_miss_transactions * iter) + sc->page_miss_transactions, pS->iterations );
         sa->auto_ref_cycles = div64( ((uint64_t)sa->auto_ref_cycles * iter) + sc->auto_ref_cycles, pS->iterations );
         sa->bank_active_cycles = div64( ((uint64_t)sa->bank_active_cycles * iter) + sc->bank_active_cycles, pS->iterations );

         sa->total_cycles = div64( ((uint64_t)sa->total_cycles * iter) + sc->total_cycles, pS->iterations );

         {
            uint32_t a_max_entries_urgent = (sa->entries_min_max>>16) & 0xff;
            uint32_t a_min_entries_valid  = (sa->entries_min_max>>8) & 0xff;
            uint32_t a_max_entries_valid  = (sa->entries_min_max>>0) & 0xff;
            uint32_t c_max_entries_urgent = (sc->entries_min_max>>16) & 0xff;
            uint32_t c_min_entries_valid  = (sc->entries_min_max>>8) & 0xff;
            uint32_t c_max_entries_valid  = (sc->entries_min_max>>0) & 0xff;

            a_max_entries_urgent = div64( ((uint64_t)a_max_entries_urgent * iter) + c_max_entries_urgent, pS->iterations );
            a_min_entries_valid = div64( ((uint64_t)a_min_entries_valid * iter) + c_min_entries_valid, pS->iterations );
            a_max_entries_valid = div64( ((uint64_t)a_max_entries_valid * iter) + c_max_entries_valid, pS->iterations );

            sa->entries_min_max = (a_max_entries_urgent<<16) | (a_min_entries_valid<<8) | a_max_entries_valid;
         }

         sa->entries_urgent = div64( ((uint64_t)sa->entries_urgent * iter) + sc->entries_urgent, pS->iterations );
         sa->entries_waw = div64( ((uint64_t)sa->entries_waw * iter) + sc->entries_waw, pS->iterations );
         sa->entries_rar = div64( ((uint64_t)sa->entries_rar * iter) + sc->entries_rar, pS->iterations );
         sa->entries_addr_coh = div64( ((uint64_t)sa->entries_addr_coh * iter) + sc->entries_addr_coh, pS->iterations );

         {
            uint32_t a_max_time = (sa->latency_urgent_times>>16) & 0xffff;
            uint32_t a_min_time = (sa->latency_urgent_times>>0) & 0xffff;
            uint32_t c_max_time = (sc->latency_urgent_times>>16) & 0xffff;
            uint32_t c_min_time = (sc->latency_urgent_times>>0) & 0xffff;

            a_max_time = div64( ((uint64_t)a_max_time * iter) + c_max_time, pS->iterations );
            a_min_time = div64( ((uint64_t)a_min_time * iter) + c_min_time, pS->iterations );

            sa->latency_urgent_times = (a_max_time<<16) | a_min_time;
         }
      }
   }

   /* Revive previous power down settings */
   *pS->memcPowerDown = pS->memcPowerDownReg;

   pS->thread = NULL;
   printk( "MEMC Worker Thread Exited!\n" );
}

/****************************************************************************
*
*  proc_enable
*
***************************************************************************/

static int proc_enable( ctl_table *table, int write,
      void __user *buffer, size_t *lenp, loff_t *ppos )
{
   int rc;

   /* Process integer operation */
   rc = proc_dointvec( table, write, buffer, lenp, ppos );

   if( write )
   {
      MEMC_STATE *pS = (MEMC_STATE*)table->extra1;

      if( pS->running )
      {
         if( pS->thread == NULL )
         {
            char s[6];
            sprintf( s, "memc%u", pS->memcNum );

            memset( &pS->statsCollected[ STATS_MAX ], 0, sizeof( MEMC_STATISTICS ) );
            memset( &pS->statsCollected[ STATS_MIN ], 0xff, sizeof( MEMC_STATISTICS ) );
            memset( &pS->statsCollected[ STATS_CUR ], 0, sizeof( MEMC_STATISTICS ) );
            pS->iterations = 0;
            pS->killWorkerThread = 0;

            /* Disable clock gating and power down mode */

            /* Save the old setting */
            pS->memcPowerDownReg = *pS->memcPowerDown;

            *pS->memcPowerDown &= ~CSR_DDR_SW_POWER_DOWN_CONTROL_POWER_DOWN_ENABLE_MASK;
            *pS->memcPowerDown |= CSR_DDR_SW_POWER_DOWN_CONTROL_DISABLE_CLOCK_GATING_MASK;

            pS->thread = kthread_run( (void *)worker_thread, pS, s );
            if( IS_ERR(pS->thread) )
            {
               printk(KERN_ERR "MEMC Statistics Collection kthread_run failed\n");
            }
         }
         else
         {
            printk( "MEMC Statistics Collection already started.\n" );
         }
      }
      else
      {
         pS->killWorkerThread = 1;
      }
   }

   return 0;
}

/****************************************************************************
*
*  proc_period
*
***************************************************************************/

static int proc_period( ctl_table *table, int write,
      void __user *buffer, size_t *lenp, loff_t *ppos )
{
   int rc;

   /* Process integer operation */
   rc = proc_dointvec( table, write, buffer, lenp, ppos );

   if ( write )
   {
      MEMC_STATE *pS = (MEMC_STATE*)table->extra1;
      pS->memcStatistics->period = pS->period;
   }

   return rc;
}

/****************************************************************************
*
*  readProc
*
***************************************************************************/
#define PRINT_STAT_PERCENT( stat, avg, max, min, cur ) \
   len += sprintf( buf+len, "%-22s: %10u (%2u%%) %10u (%2u%%) %10u (%2u%%) %10u (%2u%%)\n", \
         stat, avg, avg/(pS->period/100), max, max/(pS->period/100), min, min/(pS->period/100), cur, cur/(pS->period/100) );
#define PRINT_STAT( stat, avg, max, min, cur ) \
   len += sprintf( buf+len, "%-22s: %10u         %10u         %10u         %10u        \n", \
         stat, avg, max, min, cur );

static int readProc( char *buf, char **start, off_t offset, int count, int *eof, void *data )
{
   int len = 0;
   MEMC_STATE *pS = (MEMC_STATE*)data;

   MEMC_STATISTICS *savg = &pS->statsCollected[ STATS_AVG ];
   MEMC_STATISTICS *smax = &pS->statsCollected[ STATS_MAX ];
   MEMC_STATISTICS *smin = &pS->statsCollected[ STATS_MIN ];
   MEMC_STATISTICS *scur = &pS->statsCollected[ STATS_CUR ];

   (void) start; (void) offset; (void) count;      /* avoid compiler warning */

   len += sprintf( buf+len, "MEMC Memory Statitistics\n\n" );

   len += sprintf( buf+len, "Done      : %s\n", pS->memcStatistics->done ? "Completed" : "Collecting..." );
   len += sprintf( buf+len, "Period    : %u\n\n", pS->memcStatistics->period );
   len += sprintf( buf+len, "Iterations: %u\n\n", pS->iterations );

   len += sprintf( buf+len, "                        Avg              Max              Min              Cur\n" );

   PRINT_STAT_PERCENT( "page_hit_data_cycles", savg->page_hit_data_cycles, smax->page_hit_data_cycles, smin->page_hit_data_cycles, scur->page_hit_data_cycles );
   PRINT_STAT_PERCENT( "page_miss_data_cycles", savg->page_miss_data_cycles, smax->page_miss_data_cycles, smin->page_miss_data_cycles, scur->page_miss_data_cycles );
   PRINT_STAT_PERCENT( "auto_ref_cycles", savg->auto_ref_cycles, smax->auto_ref_cycles, smin->auto_ref_cycles, scur->auto_ref_cycles );
   PRINT_STAT_PERCENT( "bank_active_cycles", savg->bank_active_cycles, smax->bank_active_cycles, smin->bank_active_cycles, scur->bank_active_cycles );
   PRINT_STAT_PERCENT( "Total Cycles", savg->total_cycles, smax->total_cycles, smin->total_cycles, scur->total_cycles );

   len += sprintf( buf+len, "\n" );

   PRINT_STAT_PERCENT( "read_data_cycles", savg->read_data_cycles, smax->read_data_cycles, smin->read_data_cycles, scur->read_data_cycles );
   PRINT_STAT_PERCENT( "write_data_cycles", savg->write_data_cycles, smax->write_data_cycles, smin->write_data_cycles, scur->write_data_cycles );

   len += sprintf( buf+len, "\n" );

   PRINT_STAT( "page_hit_transactions", savg->page_hit_transactions, smax->page_hit_transactions, smin->page_hit_transactions, scur->page_hit_transactions );
   PRINT_STAT( "page_miss_transactions", savg->page_miss_transactions, smax->page_miss_transactions, smin->page_miss_transactions, scur->page_miss_transactions );


   len += sprintf( buf+len, "entries_min_max       :\n");
   PRINT_STAT( "   max_entries_urgent", (savg->entries_min_max>>16) & 0xff, (smax->entries_min_max>>16) & 0xff, (smin->entries_min_max>>16) & 0xff, (scur->entries_min_max>>16) & 0xff );
   PRINT_STAT( "   min_entries_valid", (savg->entries_min_max>>8) & 0xff, (smax->entries_min_max>>8) & 0xff, (smin->entries_min_max>>8) & 0xff, (scur->entries_min_max>>8) & 0xff );
   PRINT_STAT( "   max_entries_valid", (savg->entries_min_max>>0) & 0xff, (smax->entries_min_max>>0) & 0xff, (smin->entries_min_max>>0) & 0xff, (scur->entries_min_max>>0) & 0xff );

   PRINT_STAT( "entries_urgent", savg->entries_urgent, smax->entries_urgent, smin->entries_urgent, scur->entries_urgent );
   PRINT_STAT( "entries_waw", savg->entries_waw, smax->entries_waw, smin->entries_waw, scur->entries_waw );
   PRINT_STAT( "entries_rar", savg->entries_rar, smax->entries_rar, smin->entries_rar, scur->entries_rar );
   PRINT_STAT( "entries_addr_coh", savg->entries_addr_coh, smax->entries_addr_coh, smin->entries_addr_coh, scur->entries_addr_coh );

   len += sprintf( buf+len, "latency_urgent_times  :\n" );
   PRINT_STAT( "   max_time", (savg->latency_urgent_times>>16) & 0xffff, (smax->latency_urgent_times>>16) & 0xffff, (smin->latency_urgent_times>>16) & 0xffff, (scur->latency_urgent_times>>16) & 0xffff);
   PRINT_STAT( "   min_time", (savg->latency_urgent_times>>0) & 0xffff, (smax->latency_urgent_times>>0) & 0xffff, (smin->latency_urgent_times>>0) & 0xffff, (scur->latency_urgent_times>>0) & 0xffff);

   *eof = 1;
   return len+1;
}

/****************************************************************************
*
*  memc_init
*
***************************************************************************/

static int __init memc_init( void )
{
   int i;

   printk( "%s: called\n", __func__ );

   for( i = 0; i < NUM_MEMC; i++ )
   {
      char s[6];
      MEMC_STATE *pS = &gMemcState[ i ];

      sprintf( s, "memc%u", i );

      pS->memcNum          = i;
      pS->period           = DEFAULT_PERIOD;
      pS->running          = 0;
      pS->killWorkerThread = 0;
      pS->iterations       = 0;
      pS->thread           = NULL;
      if( i == 0 )
      {
         pS->sysCtlHeader = register_sysctl_table( gSysCtl0 );

         pS->memc_base_ioremap = ioremap( SYS_EMI_OPEN_BASE_ADDR, SZ_4K );
         if( !pS->memc_base_ioremap )
         {
            printk( KERN_ERR "MEMC Statistics cannot map MEMC regster base\n" );
            return -1;
         }
      }
      else
      {
         pS->sysCtlHeader = register_sysctl_table( gSysCtl1 );

         pS->memc_base_ioremap = ioremap( VC4_EMI_OPEN_BASE_ADDR, SZ_4K );
         if( !pS->memc_base_ioremap )
         {
            printk( KERN_ERR "MEMC Statistics cannot map MEMC regster base\n" );
            iounmap( gMemcState[0].memc_base_ioremap );
            return -1;
         }
      }

      /* procfs directory and files */
      pS->procDir = proc_mkdir( s, NULL );
      if( pS->procDir == NULL )
      {
         printk( KERN_ERR "%s: Failed to create directory for proc entries\n", __FUNCTION__ );
         iounmap( gMemcState[0].memc_base_ioremap );
         iounmap( gMemcState[1].memc_base_ioremap );
         return -1;
      }

      create_proc_read_entry( "stats", 0, pS->procDir, readProc, pS );

      pS->memcStatistics = (volatile MEMC_STATISTICS*)(pS->memc_base_ioremap + CSR_STATISTICS_CONTROL_OFFSET);
      pS->memcPowerDown  = (volatile uint32_t *)(pS->memc_base_ioremap + CSR_DDR_SW_POWER_DOWN_CONTROL_OFFSET);
      pS->memcStatistics->period = pS->period;
   }

  return 0;
}

/****************************************************************************
*
*  memc_exit
*
***************************************************************************/

static void __exit memc_exit( void )
{
   int i;

   for( i = 0; i < NUM_MEMC; i++ )
   {
      char s[6];
      MEMC_STATE *pS = &gMemcState[ i ];

      iounmap( pS->memc_base_ioremap );

      sprintf( s, "memc%u", i );
      
      remove_proc_entry( "stats", pS->procDir );
      remove_proc_entry( s, NULL );

      if( pS->sysCtlHeader )
      {
         unregister_sysctl_table( pS->sysCtlHeader );
      }
   }
}

module_init( memc_init );
module_exit( memc_exit );

MODULE_AUTHOR( "Raymond Ngun" );
MODULE_DESCRIPTION( "MEMC Statistics Module" );
MODULE_LICENSE( "GPL" );
