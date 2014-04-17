/*****************************************************************************
* Copyright 2004 - 2008 Broadcom Corporation.  All rights reserved.
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


/*
*
*****************************************************************************
*
*  knllog.c
*
*  PURPOSE:
*  Kernel realtime event logger (when printk's just don't do the job...)
*
*  NOTES:
*
*****************************************************************************/


/* ---- Include Files ---------------------------------------------------- */

#include <linux/module.h>
#include <linux/spinlock.h>
#include <linux/string.h>
#include <linux/vmalloc.h>
#include <linux/net.h>
#include <linux/proc_fs.h>
#include <linux/in.h>
#include <linux/file.h>
#include <linux/fs.h>
#include <linux/delay.h>
#include <linux/fcntl.h>
#include <linux/syscalls.h>
#include <linux/sysctl.h>
#include <linux/sysrq.h>
#include <linux/inet.h>
#include <linux/jiffies.h>
#include <linux/completion.h>
#include <linux/sched.h>
#include <linux/broadcom/knllog.h>

#include <mach/profile_timer.h>

#if defined ( CONFIG_BCM_PERFCNT_SUPPORT )
#include <linux/broadcom/perfcnt.h>
#endif

#include <linux/version.h>
#include <asm/uaccess.h>
//#include <cfg_global.h>

/* ---- Constants and Types ---------------------------------------------- */

#define KNLLOG_DEFAULT_ENTRIES 1000  /* The buffer size in number of log entries */


/*
 * MAXARGS can be temporarily changed to a lower number if logging mips
 * or excessive log buffer memory becomes an issue. Leave this as a compile
 * time setting rather than runtime to avoid excessive calculations with
 * variable length buffers.
 */
#define MAXARGS   6        /* max number of parameters */

/* The following is used by knllog_dump_mem */
#define MAX_BYTES_PER_LINE  (( MAXARGS - 2 ) * sizeof( int ))

/* A single log entry */
typedef struct
{
   unsigned int time;      /* time of the event */
#if defined ( CONFIG_SMP )
   unsigned int cpuid;     /* cpu number */
#endif
#if defined ( CONFIG_BCM_PERFCNT_SUPPORT )
   PERFCNT_CNTRS cntrs;
#endif
   const char *function;   /* function that KNLLOG() was called from */
   const char *fmt;        /* string (does not include function name) */
   int data[MAXARGS];      /* start of data items */
}
KNLLOG_ENTRY;

/* The log object */
typedef struct
{
   KNLLOG_ENTRY *bufp;     /* The log buffer */
   int entries;            /* The current number of allocated entries bufp points to */
   int clear;              /* flag to empty the buffer */
   int deltatime;          /* flag to show delta time */
   int msec;               /* flag to show time (and delta time) in milliseconds */
   int idx;                /* index of next write */
   int wrap;               /* buffer has wrapped flag */
   int dump;               /* forced dump using oops dump function */
   int enable;             /* logging to buffer enabled flag */
   int oldenable;          /* saved knllog.enable state */
   int maxargs;            /* max number of arguments for information */
   int capture_and_stop;   /* flag to fill the buffer once then stop */

   int logfile;            /* flag to log to 'filename' */
   char filename[128];     /* filename to dump to */
   struct file *file;      /* file handle */
   char ipaddr[32];        /* ip address string "x.x.x.x" */
   int port;               /* port to send to */
   struct socket *sock;    /* socket to dump to (not in sysctl) */
   int sleeptime;          /* sleep time in msec between event dumps */
   int dumping;            /* 'log being dumped now' flag */
#ifdef CONFIG_BCM_KNLLOG_IRQ
   int info;               /* write to print irq_sched_enable flag values */
#endif
   int eventsDumped;       /* current dump event count */
   int eventIdx;           /* current event index */
   int eventsToDump;       /* number of events to dump */
   int lasttime;           /* last timestamp */
   int starttime;          /* time of first event dumped */
   int oops;               /* debug oops dump of knllog by forcing an oops now */
#if defined ( CONFIG_BCM_PERFCNT_SUPPORT )
   int use_perfcnt;        /* include h/w performance counters in log entry */
#endif
}
KNLLOG_OBJ;

/* storage for the log object */
static KNLLOG_OBJ knllog;
static long knllogThreadPid = 0;
static struct completion knllogExited;
DECLARE_WAIT_QUEUE_HEAD(knllogWakeQ);
static int knllogWake;

/*
 * The contents of the following string isn't used, just the address, to allow knllog
 * to know that special data packing is used for the knllog_dump_mem command.
 *
 * Make it at least be a null terminated string so that if some code tries to use
 * the pointer as a fmt it won't crash.
 */
static  char *knllogDumpMemFmt = "dump_mem";

/* storage for the spinlock that protects against log collisions by different threads. */
static DEFINE_SPINLOCK(knllock);

/* sysctl */
static  struct ctl_table_header    *gSysCtlHeader;
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,31)
static int proc_do_knllog_intvec_readonly(ctl_table *table, int write,
           void __user *buffer, size_t *lenp, loff_t *ppos );
static int proc_do_knllog_intvec_enable(ctl_table *table, int write,
           void __user *buffer, size_t *lenp, loff_t *ppos );
static int proc_do_knllog_intvec_entries(ctl_table *table, int write,
           void __user *buffer, size_t *lenp, loff_t *ppos );
static int proc_do_knllog_intvec_clear(ctl_table *table, int write,
           void __user *buffer, size_t *lenp, loff_t *ppos );
static int proc_do_knllog_intvec_dump(ctl_table *table, int write,
           void __user *buffer, size_t *lenp, loff_t *ppos );
static int proc_do_knllog_intvec_oops(ctl_table *table, int write,
           void __user *buffer, size_t *lenp, loff_t *ppos );
#ifdef CONFIG_BCM_KNLLOG_IRQ
static int proc_do_knllog_intvec_info(ctl_table *table, int write,
           void __user *buffer, size_t *lenp, loff_t *ppos );
#endif
#else
static int proc_do_knllog_intvec_readonly(ctl_table *table, int write, struct file *filp,
           void __user *buffer, size_t *lenp, loff_t *ppos );
static int proc_do_knllog_intvec_enable(ctl_table *table, int write, struct file *filp,
           void __user *buffer, size_t *lenp, loff_t *ppos );
static int proc_do_knllog_intvec_entries(ctl_table *table, int write, struct file *filp,
           void __user *buffer, size_t *lenp, loff_t *ppos );
static int proc_do_knllog_intvec_clear(ctl_table *table, int write, struct file *filp,
           void __user *buffer, size_t *lenp, loff_t *ppos );
static int proc_do_knllog_intvec_dump(ctl_table *table, int write, struct file *filp,
           void __user *buffer, size_t *lenp, loff_t *ppos );
static int proc_do_knllog_intvec_oops(ctl_table *table, int write, struct file *filp,
           void __user *buffer, size_t *lenp, loff_t *ppos );
#ifdef CONFIG_BCM_KNLLOG_IRQ
static int proc_do_knllog_intvec_info(ctl_table *table, int write, struct file *filp,
           void __user *buffer, size_t *lenp, loff_t *ppos );
#endif
#endif

#ifdef CONFIG_BCM_KNLLOG_IRQ
/*
 * A global flag that code inside CONFIG_BCM_KNLLOG_IRQ checks to decide to log or not.
 * This allows a powerful debug feature to be enabled at runtime i.e. "echo 1 >irq_sched_enable"
 */
int gKnllogIrqSchedEnable = 0;

EXPORT_SYMBOL( gKnllogIrqSchedEnable );
#endif

static struct ctl_table gSysCtlKnlLog[] = {
   {
      .procname      = "enable",
      .data          = &knllog.enable,
      .maxlen        = sizeof( int ),
      .mode          = 0644,
      .proc_handler  = &proc_do_knllog_intvec_enable
   },
   {
      .procname      = "entries",
      .data          = &knllog.entries,
      .maxlen        = sizeof( int ),
      .mode          = 0644,
      .proc_handler  = &proc_do_knllog_intvec_entries
   },
   {
      .procname      = "dump",
      .data          = &knllog.dump,
      .maxlen        = sizeof( int ),
      .mode          = 0644,
      .proc_handler  = &proc_do_knllog_intvec_dump
   },
   {
      .procname      = "clear",
      .data          = &knllog.clear,
      .maxlen        = sizeof( int ),
      .mode          = 0644,
      .proc_handler  = &proc_do_knllog_intvec_clear
   },
   {
      .procname      = "deltatime",
      .data          = &knllog.deltatime,
      .maxlen        = sizeof( int ),
      .mode          = 0644,
      .proc_handler  = &proc_dointvec
   },
   {
      .procname      = "idx",
      .data          = &knllog.idx,
      .maxlen        = sizeof( int ),
      .mode          = 0644,
      .proc_handler  = &proc_do_knllog_intvec_readonly
   },
   {
      .procname      = "wrap",
      .data          = &knllog.wrap,
      .maxlen        = sizeof( int ),
      .mode          = 0644,
      .proc_handler  = &proc_do_knllog_intvec_readonly
   },
   {
      .procname      = "maxargs",
      .data          = &knllog.maxargs,
      .maxlen        = sizeof( int ),
      .mode          = 0644,
      .proc_handler  = &proc_dointvec
   },
   {
      .procname      = "capture_and_stop",
      .data          = &knllog.capture_and_stop,
      .maxlen        = sizeof( int ),
      .mode          = 0644,
      .proc_handler  = &proc_dointvec
   },
   {
      .procname      = "ipaddr",
      .data          = &knllog.ipaddr,
      .maxlen        = sizeof( knllog.ipaddr ),
      .mode          = 0644,
      .proc_handler  = &proc_dostring
   },
   {
      .procname      = "ipport",
      .data          = &knllog.port,
      .maxlen        = sizeof( knllog.port ),
      .mode          = 0644,
      .proc_handler  = &proc_dointvec
   },
   {
      .procname      = "filename",
      .data          = &knllog.filename,
      .maxlen        = sizeof( knllog.filename ),
      .mode          = 0644,
      .proc_handler  = &proc_dostring
   },
   {
      .procname      = "logfile",
      .data          = &knllog.logfile,
      .maxlen        = sizeof( knllog.logfile ),
      .mode          = 0644,
      .proc_handler  = &proc_dointvec
   },
   {
      .procname      = "sleeptime",
      .data          = &knllog.sleeptime,
      .maxlen        = sizeof( knllog.sleeptime ),
      .mode          = 0644,
      .proc_handler  = &proc_dointvec
   },
   {
      .procname      = "oops",
      .data          = &knllog.oops,
      .maxlen        = sizeof( knllog.oops ),
      .mode          = 0644,
      .proc_handler  = &proc_do_knllog_intvec_oops
   },
   {
      .procname      = "dumping",
      .data          = &knllog.dumping,
      .maxlen        = sizeof( knllog.dumping ),
      .mode          = 0644,
      .proc_handler  = &proc_dointvec
   },
   {
      .procname      = "msec",
      .data          = &knllog.msec,
      .maxlen        = sizeof( int ),
      .mode          = 0644,
      .proc_handler  = &proc_dointvec
   },
#ifdef CONFIG_BCM_KNLLOG_IRQ
   {
      .procname      = "irq_sched_enable",
      .data          = &gKnllogIrqSchedEnable,
      .maxlen        = sizeof( int ),
      .mode          = 0644,
      .proc_handler  = &proc_dointvec
   },
   {
      .procname      = "info",
      .data          = &knllog.info,
      .maxlen        = sizeof( knllog.info ),
      .mode          = 0644,
      .proc_handler  = &proc_do_knllog_intvec_info
   },
#endif
#ifdef CONFIG_BCM_PERFCNT_SUPPORT
   {
      .procname      = "use_perfcnt",
      .data          = &knllog.use_perfcnt,
      .maxlen        = sizeof( int ),
      .mode          = 0644,
      .proc_handler  = &proc_dointvec
   },
#endif

   {}
};

static struct ctl_table gSysCtl[] = {
   {
      .procname = "knllog",
      .mode     = 0555,
      .child    = gSysCtlKnlLog
   },
   {}
};

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,31)
static int proc_do_knllog_intvec_readonly(ctl_table *table, int write,
           void __user *buffer, size_t *lenp, loff_t *ppos )
#else
static int proc_do_knllog_intvec_readonly(ctl_table *table, int write, struct file *filp,
           void __user *buffer, size_t *lenp, loff_t *ppos )
#endif
{
   if ( write )
   {
      printk(KERN_WARNING "Read-only entry!\n");
      return -EINVAL;
   }

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,31)
   return proc_dointvec(table, write, buffer, lenp, ppos ); /* No special processing for read. */
#else
   return proc_dointvec(table, write, filp, buffer, lenp, ppos ); /* No special processing for read. */
#endif
}

/*
 * If we are writing the entries field, we stop logging, free the old buffer, reallocate the new buffer,
 * and enable logging again if it was on previously.
 */
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,31)
static int proc_do_knllog_intvec_entries(ctl_table *table, int write,
           void __user *buffer, size_t *lenp, loff_t *ppos )
#else
static int proc_do_knllog_intvec_entries(ctl_table *table, int write, struct file *filp,
           void __user *buffer, size_t *lenp, loff_t *ppos )
#endif
{
   int rc;

   if ( !table || !table->data )
      return -EINVAL;

   if (knllog.dumping)
   {
      printk(KERN_WARNING "Please wait for dump to complete...\n");
      return -EINVAL;
   }
   if ( write )
   {
      int oldenable = knllog.enable;

      /* Stop logging when resizing buffer */
      knllog.enable = 0;

      if ( knllog.bufp != NULL )
      {
         vfree(knllog.bufp);
         knllog.bufp = NULL;
      }

      /* get value from buffer into knllog.entries */
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,31)
      rc = proc_dointvec(table, write, buffer, lenp, ppos );
#else
      rc = proc_dointvec(table, write, filp, buffer, lenp, ppos );
#endif
      if (rc < 0)
         return rc;

      if ( knllog.entries == 0)
      {
         printk(KERN_WARNING "Cannot set 0 entries, restoring size to default\n");
         knllog.entries = KNLLOG_DEFAULT_ENTRIES;
      }

      knllog.idx = 0;
      knllog.wrap = 0;
      knllog.enable = 0;
      knllog.bufp = (KNLLOG_ENTRY *)vmalloc(sizeof(KNLLOG_ENTRY) * knllog.entries);
      if (knllog.bufp == NULL)
      {
         printk(KERN_ERR "%s: Cannot allocate memory for knllog buffer size=%u\n",
                __FUNCTION__, sizeof(KNLLOG_ENTRY) * knllog.entries);
         return -ENOMEM;;
      }
      knllog.enable = oldenable;
   }
   else
   {
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,31)
      rc = proc_dointvec(table, write, buffer, lenp, ppos ); /* No special processing for read. */
#else
      rc = proc_dointvec(table, write, filp, buffer, lenp, ppos ); /* No special processing for read. */
#endif
   }
   return rc;
}

/* If we are enabling, clear the buffer to avoid old data being misinterpreted. */
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,31)
static int proc_do_knllog_intvec_enable(ctl_table *table, int write,
           void __user *buffer, size_t *lenp, loff_t *ppos )
#else
static int proc_do_knllog_intvec_enable(ctl_table *table, int write, struct file *filp,
           void __user *buffer, size_t *lenp, loff_t *ppos )
#endif
{
   int rc = 0;

   if ( !table || !table->data )
      return -EINVAL;

   if (knllog.dumping)
   {
      printk(KERN_WARNING "Please wait for dump to complete...\n");
      return -EINVAL;
   }
   if ( write )
   {
      /* get value from buffer into knllog.enable */
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,31)
      rc = proc_dointvec(table, write, buffer, lenp, ppos );
#else
      rc = proc_dointvec(table, write, filp, buffer, lenp, ppos );
#endif
      if (rc < 0)
         return rc;

      if (knllog.enable)
      {
         knllog_enable();
         knllog_clear();
      }
   }
   else
   {
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,31)
      rc = proc_dointvec(table, write, buffer, lenp, ppos ); /* No special processing for read. */
#else
      rc = proc_dointvec(table, write, filp, buffer, lenp, ppos ); /* No special processing for read. */
#endif
   }
   return rc;
}

/*
 * If we are writing the clear field, we stop logging, clear the buffer,
 * and enable logging again if it was on previously.
 */
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,31)
static int proc_do_knllog_intvec_clear(ctl_table *table, int write,
           void __user *buffer, size_t *lenp, loff_t *ppos )
#else
static int proc_do_knllog_intvec_clear(ctl_table *table, int write, struct file *filp,
           void __user *buffer, size_t *lenp, loff_t *ppos )
#endif
{
   int rc = 0;

   if ( !table || !table->data )
      return -EINVAL;

   if (knllog.dumping)
   {
      printk(KERN_WARNING "Please wait for dump to complete...\n");
      return -EINVAL;
   }
   if ( write )
   {
      knllog_clear();
   }
   else
   {
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,31)
      rc = proc_dointvec(table, write, buffer, lenp, ppos ); /* No special processing for read. */
#else
      rc = proc_dointvec(table, write, filp, buffer, lenp, ppos ); /* No special processing for read. */
#endif
   }
   return rc;
}

/*
 * If we are writing the clear field, we stop logging, clear the buffer,
 * and enable logging again if it was on previously.
 */
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,31)
static int proc_do_knllog_intvec_oops(ctl_table *table, int write,
           void __user *buffer, size_t *lenp, loff_t *ppos )
#else
static int proc_do_knllog_intvec_oops(ctl_table *table, int write, struct file *filp,
           void __user *buffer, size_t *lenp, loff_t *ppos )
#endif
{
   int rc = 0;

   if ( !table || !table->data )
      return -EINVAL;

   if ( write )
   {
      volatile int *x = NULL;
      *x = 0;  /* Force an exception */
   }
   else
   {
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,31)
      rc = proc_dointvec(table, write, buffer, lenp, ppos ); /* No special processing for read. */
#else
      rc = proc_dointvec(table, write, filp, buffer, lenp, ppos ); /* No special processing for read. */
#endif
   }
   return rc;
}

#ifdef CONFIG_BCM_KNLLOG_IRQ
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,31)
static int proc_do_knllog_intvec_info(ctl_table *table, int write,
           void __user *buffer, size_t *lenp, loff_t *ppos )
#else
static int proc_do_knllog_intvec_info(ctl_table *table, int write, struct file *filp,
           void __user *buffer, size_t *lenp, loff_t *ppos )
#endif
{
   int rc = 0;

   if ( !table || !table->data )
      return -EINVAL;

   if ( write )
   {
      printk(KERN_INFO "Settings for irq_sched_enable flags\n\
            KNLLOG_THREAD     0x%02x   (Log thread scheduling)\n\
            KNLLOG_IRQ        0x%02x   (Log hard IRQ activity)\n\
            KNLLOG_SOFTIRQ    0x%02x   (Log Soft IRQ activity)\n\
            KNLLOG_TASKLET    0x%02x   (Log tasklet activity)\n\
            KNLLOG_TIMER      0x%02x   (Log timer callbacks)\n\
            KNLLOG_FB         0x%02x   (Frame buffer updates)\n\
            KNLLOG_DMA        0x%02x   (DMA transfer times)\n\
            KNLLOG_PROFILING  0x%02x   (Additional profiling)\n",

            KNLLOG_THREAD, KNLLOG_IRQ, KNLLOG_SOFTIRQ, KNLLOG_TASKLET, 
            KNLLOG_TIMER, KNLLOG_FB, KNLLOG_DMA, KNLLOG_PROFILING);
   }
   else
   {
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,31)
      rc = proc_dointvec(table, write, buffer, lenp, ppos ); /* No special processing for read. */
#else
      rc = proc_dointvec(table, write, filp, buffer, lenp, ppos ); /* No special processing for read. */
#endif
   }
   return rc;
}
#endif


/* dump the log buffer */
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,31)
static int proc_do_knllog_intvec_dump(ctl_table *table, int write,
           void __user *buffer, size_t *lenp, loff_t *ppos )
#else
static int proc_do_knllog_intvec_dump(ctl_table *table, int write, struct file *filp,
           void __user *buffer, size_t *lenp, loff_t *ppos )
#endif
{
   int rc = 0;

   if ( !table || !table->data )
      return -EINVAL;

   if ( write )
   {
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,31)
      rc = proc_dointvec(table, write, buffer, lenp, ppos);
#else
      rc = proc_dointvec(table, write, filp, buffer, lenp, ppos);
#endif
      if (rc < 0)
      {
         return rc;
      }

      if (knllog.dump == 0)
      {
         knllog.dumping = 0;
         knllog.enable = knllog.oldenable;
         printk(KERN_NOTICE "Dumping disabled\n");
         return 0;
      }

      /* Stop logging when dumping buffer or log regions and timestamps may not make sense */
      knllog.oldenable = knllog.enable;
      knllog.enable = 0;
      knllog.dumping = 1;

      if (knllog.bufp == NULL)
      {
         printk(KERN_ERR "Error: No log buffer allocated\n");
         knllog.enable = knllog.oldenable;
         knllog.dumping = 0;
         return -EINVAL;
      }

      /* local dump in a tight loop for oops and reliability */
      if (knllog.sleeptime < 0)
      {
         knllog_dump();
         knllog.enable = knllog.oldenable;
         knllog.dumping = 0;
         return rc;
      }

      /* wakeup kernel thread to perform dump. */
      knllogWake = 1;
      wake_up_interruptible(&knllogWakeQ);
      return rc;
   }
   else
   {
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,31)
      rc = proc_dointvec(table, write, buffer, lenp, ppos ); /* No special processing for read. */
#else
      rc = proc_dointvec(table, write, filp, buffer, lenp, ppos ); /* No special processing for read. */
#endif
      return rc;
   }
   return rc;
}


/* helper routine to obtain hardware time stamp */
static int getTime( void )
{
   return ( timer_get_tick_count() );
}

/* decode an event into a symbolic string */
static void decodeEvent(KNLLOG_ENTRY *ep, char **linepp, int linelen, int eventnum)
{
   char *line = *linepp;
   char field[160];

   *line = '\0';
   if (eventnum == 0)
   {
      knllog.lasttime = ep->time;
      knllog.starttime = ep->time;
   }
#if defined ( CONFIG_SMP )
   snprintf(field, sizeof(field), "CPU_%u: ", ep->cpuid);
   strlcat(line, field, linelen );
#endif

   if ( knllog.msec )
   {
       int ticks = ep->time - knllog.starttime;
       int usecs = ticks / ( timer_get_tick_rate() / 1000000 );
       snprintf(field, sizeof(field), "%6d.%03d: ", usecs / 1000u, usecs % 1000u);
   }
   else
   {
       snprintf(field, sizeof(field), "%10u: ", ep->time);
   }
   strlcat(line, field, linelen );

   if ( knllog.deltatime)
   {
      if ( knllog.msec )
      {
          int deltaticks = ep->time - knllog.lasttime;
          int deltausecs = deltaticks / ( timer_get_tick_rate() / 1000000 );
          snprintf(field, sizeof(field), "%6d.%03d: ", deltausecs / 1000u, deltausecs % 1000u);
      }
      else
      {
          snprintf(field, sizeof(field), "%10u: ", ep->time - knllog.lasttime);
      }
      strlcat(line, field, linelen );
   }

#if defined ( CONFIG_BCM_PERFCNT_SUPPORT )
   if (knllog.use_perfcnt)
   {
         #if defined ( CONFIG_ARCH_BCMRING )

         snprintf(field, sizeof(field), "%10u: ", ep->cntrs.ccr);
         strlcat(line, field, linelen );
         snprintf(field, sizeof(field), "%10u: ", ep->cntrs.cr0);
         strlcat(line, field, linelen );
         snprintf(field, sizeof(field), "%10u: ", ep->cntrs.cr1);
         strlcat(line, field, linelen );
         snprintf(field, sizeof(field), "%10u: ", ep->cntrs.bus_idle);
         strlcat(line, field, linelen );

         #elif defined ( CONFIG_ARCH_BCMHANA )

         snprintf(field, sizeof(field), "%10u: ", ep->cntrs.ccr);
         strlcat(line, field, linelen );
         snprintf(field, sizeof(field), "%10u: ", ep->cntrs.cr[0]);
         strlcat(line, field, linelen );
         snprintf(field, sizeof(field), "%10u: ", ep->cntrs.cr[1]);
         strlcat(line, field, linelen );

         #endif
   }
#endif
   snprintf(field, sizeof(field), "%s: ", ep->function);
   strlcat(line, field, linelen );

   if ( ep->fmt == knllogDumpMemFmt )
   {
       int      i;
       int      numBytes;
       uint8_t *data;

       snprintf( field, sizeof( field ), "%08x: ", ep->data[0]);
       strlcat(line, field, linelen );

       numBytes = ep->data[1];
       data = (uint8_t *)&ep->data[2];

       for ( i = 0; i < MAX_BYTES_PER_LINE; i++ )
       {
           if ( i < numBytes )
           {
               snprintf( field, sizeof( field ), "%02x ", data[ i ]);
               strlcat(line, field, linelen );
           }
           else
           {
               strlcat(line, "   ", linelen );
           }
       }
       for ( i = 0; i < numBytes; i++ )
       {
           if (( data[i] >= ' ' ) && ( data[i] <= '~' ))
           {
               field[i] = data[i];
           }
           else
           {
               field[i] = '.';
           }
       }
       field[ numBytes ] = '\0';
       strlcat(line, field, linelen );
   }
   else
   {
       /* Make sure the data parameter list length matches 'MAXARGS' defined at the top of this file */
       scnprintf( field, sizeof(field), ep->fmt, ep->data[0], ep->data[1], ep->data[2], ep->data[3], ep->data[4], ep->data[5] );
       strlcat(line, field, linelen );
   }
   /* #3823 - Remove all trailing newlines that exist */
   {
      int i;
      int checkcnt = strlen(line); /* timestamp/func/etc guarantee nonzero linelen here */
      char replaceChar = '\0';
      for (i = 0; i < checkcnt; i++)
      {
         if ((line[checkcnt - 1 - i] != '\n'))
         {
            /* Replace non-eol newlines with spaces */
            replaceChar = ' ';
         }
         else
         {
            line[checkcnt - 1 - i] = replaceChar;
         }
      }
   }
   /* Add a newline if necessary */
   if (line[strlen(line)-1] != '\n')
   {
      strlcat(line, "\n", linelen );
   }
   /* At this point there will be a single trailing newline */
   line[linelen - 1] = '\0';   /* in case of overflow */
}
#if CONFIG_NET
/****************************************************************************
*
*  socksend
*
*   send on a socket.
*
***************************************************************************/
static int socksend(char *bufp, int size)
{
   struct msghdr msg;
   struct iovec iov;
   int rc = -1;
   iov.iov_base = bufp;
   iov.iov_len = size;
   rc = kernel_sendmsg(knllog.sock, &msg, (struct kvec *) &iov, 1, size);
   if (rc < 0)
   {
      printk(KERN_ERR "sock send, rc=%d\n", rc);
   }
   return rc;
}
#endif

/* Get a banner to print to the various output mechanisms */
static void getBanner(char **linepp, int maxlen, int events)
{
   char tmp[160];
   char *tmpp = tmp;

   snprintf(*linepp, maxlen, "Kernel Log Dump Start (logging disabled while dumping)...\n");
   snprintf(tmpp, sizeof(tmp), "%u ticks/sec, %u (0x%08x) max_timestamp, %d entries\n", timer_get_tick_rate(), ~0, ~0, events);
   strncat(*linepp, tmpp, maxlen);
#if defined ( XCONFIG_BCM_PERFCNT_SUPPORT )
   if (knllog.use_perfcnt)
   {
      snprintf(tmpp, sizeof(tmp),
               "ticks%s/cycles/%s/%s/bus_idle\n",
               knllog.deltatime ? "/deltas" : "",
               perfcnt_get_evtstr0(),
               perfcnt_get_evtstr1() );
      strncat(*linepp, tmpp, maxlen);
   }
   else
#endif
   if (knllog.deltatime)
   {
      snprintf(tmpp, sizeof(tmp), "ticks/deltas\n");
      strncat(*linepp, tmpp, maxlen);
   #if !defined ( CONFIG_ARCH_BCMHANA )
      snprintf(tmpp, sizeof(tmp), "/bus_idle");
      strncat(*linepp, tmpp, maxlen);
   #endif
      snprintf(tmpp, sizeof(tmp), "\n");
      strncat(*linepp, tmpp, maxlen);
   }

   (*linepp)[maxlen-1] = '\0';
}

/* Output a string to one of the various output mechanisms */
static int outputStr(char *str, int verbose)
{
#if CONFIG_NET
   if (knllog.sock)
   {
      int rc = socksend(str, strlen(str));
      if (rc != 0)
      {
         return rc;
      }
      if (verbose)
      {
         printk(str);
      }
   }
   else
#endif
   if (knllog.logfile)
   {
      if (strcmp(knllog.filename, "console") == 0)
      {
         printk(str);
      }
      else
      {
         if (knllog.file)
         {
            mm_segment_t old_fs = get_fs();
            set_fs(KERNEL_DS);
            knllog.file->f_op->write(knllog.file, str, strlen(str), &knllog.file->f_pos);
            set_fs(old_fs);
            if (verbose)
            {
               printk(str);
            }
         }
      }
   }
   return 0;
}
/****************************************************************************
*
*  knllog_thread
*
*   Worker thread to transfer data to the file or socket.
*
***************************************************************************/
static int knllog_thread(void *data)
{
   daemonize("knllog");

   while (1)
   {
      if (0 == wait_event_interruptible(knllogWakeQ, knllogWake))
      {
         /*
          * Waking here is a signal to dump the log. The inner
          * loop below will sleep until the log is dumped and
          * then will return here to wait for another dump command.
          */
         knllogWake = 0;

         if (!knllog.wrap)
         {
            knllog.eventIdx = 0;
            knllog.eventsToDump = knllog.idx;
         }
         else
         {
            knllog.eventIdx = knllog.idx;
            knllog.eventsToDump = knllog.entries;
         }
         knllog.eventsDumped = 0;


#if CONFIG_NET
         if (!knllog.logfile)
         {
            struct sockaddr_in serverAddr;
            int rc = sock_create_kern(AF_INET, SOCK_STREAM, IPPROTO_TCP, &knllog.sock);
            if (rc < 0)
            {
               printk(KERN_ERR "knllog: socket create failed rc=%d\n", rc);
               goto cleanup;
            }
            printk(KERN_NOTICE "socket created\n");

            /* connect to the server */
            memset(&serverAddr, 0, sizeof(serverAddr));
            serverAddr.sin_family = AF_INET;
            serverAddr.sin_port = htons((short) knllog.port);
            if ((u32) - 1 == (serverAddr.sin_addr.s_addr = in_aton(knllog.ipaddr)))
            {
               printk(KERN_ERR "knllog:  Unknown server name '%s'.\n", knllog.ipaddr);
               sock_release(knllog.sock);
               knllog.sock = NULL;
               return (-1);
            }

            rc = knllog.sock->ops->connect(knllog.sock, (struct sockaddr *) &serverAddr, sizeof(serverAddr), 0);
            if (rc < 0)
            {
               printk(KERN_ERR "knllog: Can't connect to server. rc = %d\n", rc);
               sock_release(knllog.sock);
               knllog.sock = NULL;
               return rc;
            }
            printk(KERN_NOTICE "knllog: connected to server. rc = %d\n", rc);
         }
         else                   /* log to file */
#endif
         {
            /* "console" is a special filename to trigger timed printk's */
            if (strcmp(knllog.filename, "console") != 0)
            {
               mm_segment_t old_fs = get_fs();
               set_fs(KERNEL_DS);
               printk(KERN_NOTICE "Calling open on %s\n", knllog.filename);
               knllog.file = filp_open(knllog.filename, O_TRUNC | O_WRONLY | O_CREAT, 0644);
               set_fs(old_fs);
               if (!knllog.file || !knllog.file->f_op || !knllog.file->f_op->write)
               {
                  printk(KERN_ERR "Could not open %s\n", knllog.filename);
                  return (-1);
               }
            }
            else
            {
               printk(KERN_NOTICE "Dumping to console\n");
            }
         }

         {
            char line[320];
            char *linep = line;
            getBanner(&linep, sizeof(line), knllog.eventsToDump);
            outputStr(line, 1);
         }

         while (knllog.dumping && (knllog.eventsDumped < knllog.eventsToDump))
         {
            KNLLOG_ENTRY *ep = &knllog.bufp[knllog.eventIdx++];

            char line[160];
            char *linep = line;
            decodeEvent(ep, &linep, sizeof(line), knllog.eventsDumped);
            outputStr(linep, 0);

            if (knllog.eventIdx >= knllog.entries)
            {
               knllog.eventIdx = 0;
            }
            knllog.lasttime = ep->time;

            knllog.eventsDumped++;

            if (knllog.eventsDumped % 100 == 0)
            {
               msleep(knllog.sleeptime);
            }

         }
         outputStr("Kernel Log Dump End...\n", 1);

 cleanup:
#if CONFIG_NET
         if (knllog.sock)
         {
            printk(KERN_NOTICE "Calling sock_release\n");
            sock_release(knllog.sock);
            knllog.sock = NULL;
         }
#endif
         if (knllog.file)
         {
            mm_segment_t old_fs = get_fs();
            set_fs(KERNEL_DS);
            printk(KERN_NOTICE "Calling close on %s\n", knllog.filename);
            filp_close(knllog.file, current->files);
            set_fs(old_fs);
            knllog.file = NULL;
         }
         knllog.enable = knllog.oldenable;
         knllog.dumping = 0;
      }
      else
      {
         printk(KERN_ERR "wait_event_interruptible returned non-zero code\n");
      }
   }
   complete_and_exit(&knllogExited, 0);
}

#ifdef CONFIG_MAGIC_SYSRQ
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,36)
static void sysrq_handle_knllog(int key, struct tty_struct *tty)
#else
static void sysrq_handle_knllog(int key)
#endif
{
   knllog_dump();
   return;
}

static struct sysrq_key_op sysrq_knllog_op = {
	.handler	= sysrq_handle_knllog,
	.help_msg	= "(z)knllog dump",
	.action_msg	= "DUMP",
};
#endif

/*
 * Initialize by setting up the sysctl and proc/knllog entries, allocating
 * default storage and resetting the variables.
 */
void knllog_init(void)
{
   /* register sysctl table */
   printk(KERN_NOTICE "%s\n", __FUNCTION__);

#ifdef CONFIG_MAGIC_SYSRQ
   register_sysrq_key('z', &sysrq_knllog_op);
#endif
   gSysCtlHeader = register_sysctl_table( gSysCtl );
   if ( gSysCtlHeader == NULL )
   {
      printk(KERN_ERR "%s: could not register sysctl table\n", __FUNCTION__);
   }

   memset(&knllog, 0, sizeof(KNLLOG_OBJ));   /* zero entries, idx, wrap, and enable */
   knllog.entries = 0;
   knllog.idx = 0;
   knllog.wrap = 0;
   knllog.enable = 0;   /* Logging on by default */
   knllog.entries = KNLLOG_DEFAULT_ENTRIES;
   knllog.maxargs = MAXARGS;
   knllog.bufp = (KNLLOG_ENTRY *)vmalloc(sizeof(KNLLOG_ENTRY) * knllog.entries);
   if (knllog.bufp == NULL)
   {
      knllog.entries = 0;
      knllog.enable = 0;
      printk(KERN_ERR "%s: Cannot allocate memory for knllog buffer\n", __FUNCTION__);   /* i.e. don't enable it. */
      return;
   }
   strcpy(knllog.filename, "/tmp/knllog.txt");  /* default local file name */
   knllog.logfile = 1;                          /* log to local file by default */
   strcpy(knllog.ipaddr, "10.136.49.91");       /* dummy host ip address */
   knllog.port = 4000;                          /* default logging ip:port */
   knllog.sleeptime = 0;                        /* default sleep time between events */
   knllog.dumping = 0;                          /* not dumping at init */

#if defined ( CONFIG_BCM_PERFCNT_SUPPORT )
   knllog.use_perfcnt = 0;                      /* don't use perf counters by default */
#endif

   /* Create dumping to file or socket thread */
   init_completion(&knllogExited);
   knllogThreadPid = kernel_thread(knllog_thread, 0, 0);

#if 0 /* TEST */
   KNLLOG("testnoarg\n", 0);
   KNLLOG("test %d\n", 0);
   KNLLOG("test2 val1=0x%x, val2=%d", 1, 2);
   KNLLOG("test3 val1=%d val2=0x%08x, val3=%05d", 1,2,3);
   KNLLOG("test4 %d %d %d %d\n", 0xf000,2,3,4);
   KNLLOG("test4 0x%x 0x%04x 0x%08x %u", 0xf000,2,3,4);
   KNLLOG("test5 %d\n\n\n\n\n\n", 0);
   KNLLOG("test6\n\n\n\n\n\n");
   KNLLOG("test7\nbla1\nbla2\n%d\n\n\n\n\n\n", 1234);
   KNLLOG("\n\n\n");
   KNLLOG("\n");
   KNLLOG("");
   {
       uint8_t  mem[] =
       {
           'T', 'h', 'i', 's', ' ', 'i', 's', ' ', 'a', ' ', 't', 'e', 's', 't', 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 126, 127, 128, 129, 130
       };

       KNLLOG_DUMP_MEM( 0, mem, sizeof( mem ));
   }
#endif
}

/* Exit and cleanup (probably not done) */
void knllog_exit(void)
{
   if ( gSysCtlHeader != NULL )
   {
      unregister_sysctl_table( gSysCtlHeader );
   }

#ifdef CONFIG_MAGIC_SYSRQ
   unregister_sysrq_key('z', &sysrq_knllog_op);
#endif
   if (knllog.bufp)
   {
      vfree(knllog.bufp);
      knllog.bufp = NULL;
   }
}

inline void knllog_add_entry( const char *function, const char *fmt, va_list args )
{
   int i;
   unsigned long flags;
   KNLLOG_ENTRY *entryp;
   int *datap;             /* pointer to data items */

   if (!knllog.enable)     /* intentionally don't check bufp each time to save cycles */
   {
      return;
   }
   spin_lock_irqsave(&knllock, flags);
   entryp = &knllog.bufp[knllog.idx];
   entryp->time = getTime();

#if defined ( CONFIG_BCM_PERFCNT_SUPPORT )
   perfcnt_availability_check();
   perfcnt_read( &entryp->cntrs );
#endif

#if defined ( CONFIG_SMP )
   entryp->cpuid = smp_processor_id();
#endif

   entryp->function = function;
   entryp->fmt = fmt;
   datap = &entryp->data[0];
   for ( i=0; i<MAXARGS; i++ )
   {
      *datap++ = va_arg(args, int);
   }
   if (++knllog.idx >= knllog.entries)
   {
      knllog.idx = 0;
      knllog.wrap = 1;
      if (knllog.capture_and_stop)
      {
         /*
          * Disable logging in capture and stop mode, buffer full.
          * Also reenable the flag for next time.
          */
         knllog.capture_and_stop = 0;
         knllog.enable = 0;
         printk(KERN_NOTICE "knllog capture_and_stop done...\n");
      }
   }
   spin_unlock_irqrestore(&knllock, flags);
}

/* Log an entry if buffer allocated and enabled. */
void knllog_ventry(const char *function, const char *fmt, va_list args)
{
   if (!knllog.enable)     /* intentionally don't check bufp each time to save cycles */
   {
      return;
   }
   knllog_add_entry( function, fmt, args );
}

/* Log an entry if buffer allocated and enabled. */
void knllog_entry(const char *function, const char *fmt, ...)
{
   va_list args;

   if (!knllog.enable)     /* intentionally don't check bufp each time to save cycles */
   {
      return;
   }
   va_start( args, fmt );
   knllog_add_entry( function, fmt, args );
   va_end( args );
}

/* Dump the log buffer. */
void knllog_dump(void)
{
   char banner[320];
   char *bannerp = banner;
   int i, start, entries;
   knllog.lasttime = 0;
   if (knllog.bufp == NULL)
   {
      printk(KERN_ERR "Error: No log buffer allocated\n");
      return;
   }

   if (!knllog.wrap)
   {
      start = 0;
      entries = knllog.idx;
   }
   else
   {
      start = knllog.idx;
      entries = knllog.entries;
   }
   getBanner(&bannerp, sizeof(banner), entries);
   printk(bannerp);

   for (i = 0; i < entries; i++)
   {
      char line[160];
      char *linep = line;
      KNLLOG_ENTRY *ep = &knllog.bufp[start++];
      decodeEvent(ep, &linep, sizeof(line), i);

      /* Print the event */
      printk(linep);

      if (start >= knllog.entries)
      {
         start = 0;
      }
      knllog.lasttime = ep->time;
   }
   printk(KERN_NOTICE "Kernel Log Dump End...\n");
}

/* Enable logging under program control. */
void knllog_enable(void)
{
   unsigned long flags;
   spin_lock_irqsave(&knllock, flags);
   knllog.enable = 1;
   spin_unlock_irqrestore(&knllock, flags);
}

/* Disable logging under program control. */
void knllog_disable(void)
{
   unsigned long flags;
   spin_lock_irqsave(&knllock, flags);
   knllog.enable = 0;
   spin_unlock_irqrestore(&knllock, flags);
}

/* Clear the log under program control. */
void knllog_clear(void)
{
   unsigned long flags;
   spin_lock_irqsave(&knllock, flags);
   knllog.idx = 0;
   knllog.wrap = 0;
   spin_unlock_irqrestore(&knllock, flags);
}

/* Return the buffer index under program control. */
int knllog_idx(void)
{
   return knllog.idx;
}

/* Return the wrap flag under program control. */
int knllog_wrap(void)
{
   return knllog.wrap;
}
/* Return the number of entries under program control. */
int knllog_entries(void)
{
   return knllog.entries;
}
/*
 * Set flag to stop the capture when the buffer is full.
 * Flag is cleared automatically when logging is stopped.
 */
void knllog_capture_and_stop(void)
{
   unsigned long flags;
   spin_lock_irqsave(&knllock, flags);
   knllog.capture_and_stop = 1;
   spin_unlock_irqrestore(&knllock, flags);
}
/*
 * Set the sleep time. Intended to be
 * used to set the time negative for an
 * oops dump in a tight printk loop.
 */
void knllog_sleeptime(int msec)
{
   unsigned long flags;
   spin_lock_irqsave(&knllock, flags);
   knllog.sleeptime = msec;
   spin_unlock_irqrestore(&knllock, flags);
}

/****************************************************************************
*
*   Dumps a region of memory. This is done by using a special fmt and
*   packing upto 16 bytes of data into 4 ints.
*
*   So the net effect is that 16 bytes of data can be dumped per knllog
*   entry.
*
*   In terms of arguments, we pack things as follows:
*
*   fmt - points to knllogDumpMemFmt
*
*   data[0] - Address of the first byte of data
*   data[1] - number of bytes used by this line
*   data[2..5]  - 16 bytes of data packed packed 4 per int.
*
***************************************************************************/

void knllog_dump_mem( const char *function, uint32_t addr, const void *voidMem, size_t numBytes )
{
    int             arg[ MAXARGS ];
    const uint8_t  *mem = voidMem;

    while ( numBytes > 0 )
    {
        size_t  bytesThisLine = numBytes;

        if ( bytesThisLine > MAX_BYTES_PER_LINE )
        {
            bytesThisLine = MAX_BYTES_PER_LINE;
        }

        arg[0] = addr;
        arg[1] = bytesThisLine;

        memcpy( &arg[2], mem, bytesThisLine );

        /* Make sure the data parameter list length matches 'MAXARGS' defined at the top of this file */
        knllog_entry( function, knllogDumpMemFmt, arg[0], arg[1], arg[2], arg[3], arg[4], arg[5] );

        mem      += bytesThisLine;
        addr     += bytesThisLine;
        numBytes -= bytesThisLine;
    }
}

/***************************************************************************/

EXPORT_SYMBOL(knllog_init);
EXPORT_SYMBOL(knllog_exit);
EXPORT_SYMBOL(knllog_entry);
EXPORT_SYMBOL(knllog_ventry);
EXPORT_SYMBOL(knllog_dump);
EXPORT_SYMBOL(knllog_clear);
EXPORT_SYMBOL(knllog_dump_mem);
EXPORT_SYMBOL(knllog_disable);
EXPORT_SYMBOL(knllog_sleeptime);




