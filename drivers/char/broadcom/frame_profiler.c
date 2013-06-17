/*****************************************************************************
* Copyright 2003 - 2011 Broadcom Corporation.  All rights reserved.
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
*****************************************************************************

*****************************************************************************
*
*  @file    frame_profiler
*
*  @brief   Keeps a history and statistics of the processing time of frames.
*           To add another profiler, define another PROFILER variable and
*           pointer to it. Export and extern the pointer.
****************************************************************************/

/* ----------------------------------------------------------------------- */
/*                            Include Files                                */
/* ----------------------------------------------------------------------- */

#include <linux/version.h>
#include <linux/types.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/errno.h>
#include <linux/broadcom/knllog.h>
#include <linux/broadcom/timer.h>
#include <linux/kthread.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/sysctl.h>
#include <linux/broadcom/gos/gos.h>
#include <asm/uaccess.h>
#include <linux/broadcom/gos/std/stdlib.h>
#include <linux/broadcom/frame_profiler.h>

/* ----------------------------------------------------------------------- */
/*                  Private Constants and Types                            */
/* ----------------------------------------------------------------------- */

#define HISTORY_SIZE                50
#define AVG_RANGE                   (HISTORY_SIZE/2)
#define MAX_PROFILERS               10

typedef struct
{
   unsigned int currTime;
   unsigned int maxTime;
   unsigned int avgPercentTime;
   unsigned int maxAvgPercentTime;
   unsigned int avgWeightedTime;
   unsigned int maxAvgWeightedTime;

} STATS;

typedef struct
{
   int enable;
   int inuse;
   unsigned int alphaFactor; /* Weightage of current value to the average (in %) */
   unsigned int avgSum;

} STATE;

typedef struct
{
   unsigned int history[HISTORY_SIZE];
   timer_tick_count_t start[HISTORY_SIZE];
   timer_tick_count_t end[HISTORY_SIZE];
   timer_tick_count_t start_temp[HISTORY_SIZE];
   timer_tick_count_t end_temp[HISTORY_SIZE];
   timer_tick_count_t *pstart;
   timer_tick_count_t *pend;

} DATA;

typedef struct
{
   STATS profilerStats;
   STATE profilerState;
   DATA profilerData;
   char procName[25];
   struct task_struct *profilerTask;
   struct proc_dir_entry *procDir;
   struct proc_dir_entry *procEnable;
   struct proc_dir_entry *procReset;
   struct proc_dir_entry *procAlphafactor;
   GOS_SEM semData;

} PROFILER;

/* ----------------------------------------------------------------------- */
/*                  Private Variables                                      */
/* ----------------------------------------------------------------------- */

static PROFILER gProfilerArray[MAX_PROFILERS];
static int gInitializeProfilerArray = 0;
static GOS_SEM FP_semWrite;

/* ----------------------------------------------------------------------- */
/*                  Private Function Prototypes                            */
/* ----------------------------------------------------------------------- */

static int profilerThread( void *data );

/* ----------------------------------------------------------------------- */
/*                  Private Function                                       */
/* ----------------------------------------------------------------------- */

/****************************************************************************
* 
* microSec
*
****************************************************************************/

unsigned int microSec( timer_tick_count_t ticks )
{
   return ( ticks / (timer_get_tick_rate() / 1000000) );
}

/****************************************************************************
*
*  readProcHistory
*
****************************************************************************/

static int readProcHistory( char *buf, char **start, off_t offset, int count, int *eof, void *data )
{
   PROFILER *instance = ( PROFILER * )data;
   int len = 0;
   int i;
   (void) start; (void) offset; (void) count;      /* avoid compiler warning */

   gosSemTake( instance->semData );
   len += sprintf( buf+len, "\n********************* Summary of History ************************\n\n" );
   len += sprintf( buf+len, "Processing Time             Start Time               End Time    \n\n" );

   for ( i = 0; i < HISTORY_SIZE; i++ )
   {
      len += sprintf( buf+len, "%15u %22u %22u \n", instance->profilerData.history[i], \
         microSec(instance->profilerData.start[HISTORY_SIZE-1-i]), microSec(instance->profilerData.end[HISTORY_SIZE-1-i]) );
   }
   gosSemGive( instance->semData );

   *eof = 1;
   return len+1;
}

/****************************************************************************
*
*  readProcStats
*
****************************************************************************/

static int readProcStats( char *buf, char **start, off_t offset, int count, int *eof, void *data )
{
   PROFILER *instance = ( PROFILER * )data;
   int len = 0;
   (void) start; (void) offset; (void) count;      /* avoid compiler warning */

   len += sprintf( buf+len, "\n********************* Summary of Statistics *********************\n\n" );
   len += sprintf( buf+len, "Statistic                            Current         Maximum \n\n" );
   len += sprintf( buf+len, "%-27s: %15u %15u \n", "Processing Time (us)", instance->profilerStats.currTime, instance->profilerStats.maxTime );
   len += sprintf( buf+len, "%-27s: %15u %15u \n", "Average Percent Time (us)", instance->profilerStats.avgPercentTime, instance->profilerStats.maxAvgPercentTime );
   len += sprintf( buf+len, "%-27s: %15u %15u \n", "Average Weighted Time (us)", instance->profilerStats.avgWeightedTime, instance->profilerStats.maxAvgWeightedTime );
   
   *eof = 1;
   return len+1;
}

/****************************************************************************
*
*  readProcEnable
*
****************************************************************************/

int readProcEnable( char *buf, char **start, off_t offset, int count, int *eof, void *data ) 
{
   PROFILER *instance = ( PROFILER * )data;
   int len = 0;
   (void) start; (void) offset; (void) count;      /* avoid compiler warning */

   len = sprintf( buf+len, "Enable: %d \n", instance->profilerState.enable );
   return len;
}

/****************************************************************************
*
*  readProcAlphafactor
*
****************************************************************************/

int readProcAlphafactor( char *buf, char **start, off_t offset, int count, int *eof, void *data ) 
{
   PROFILER *instance = ( PROFILER * )data;
   int len = 0;
   (void) start; (void) offset; (void) count;      /* avoid compiler warning */

   len = sprintf( buf+len, "Alpha-factor: %d \n", instance->profilerState.alphaFactor );
   return len;
}

/****************************************************************************
*
*  writeProcEnable
*
****************************************************************************/

int writeProcEnable( struct file *file, const char *buf, int count, void *data )
{
   PROFILER *instance = ( PROFILER * )data;
   char input;

   if ( count > sizeof( instance->profilerState.enable ) )
   {
       count = sizeof( instance->profilerState.enable );
   }
 
   if ( copy_from_user( &input, buf, count ) )
   {
       return -EFAULT;
   }
   
   gosSemTake( instance->semData );
   instance->profilerState.enable = atoi(&input);
   gosSemGive( instance->semData );

   return count;
}


/****************************************************************************
*
*  writeProcAlphafactor
*
****************************************************************************/

int writeProcAlphafactor( struct file *file, const char *buf, int count, void *data )
{
   PROFILER *instance = ( PROFILER * )data;
   char input[5];

   if ( count > sizeof( instance->profilerState.alphaFactor ) )
   {
       count = sizeof( instance->profilerState.alphaFactor );
   }
 
   if ( copy_from_user( input, buf, count ) )
   {
       return -EFAULT;
   }
   
   gosSemTake( instance->semData );
   instance->profilerState.alphaFactor = atoi(&input[0]);
   gosSemGive( instance->semData );

   return count;
}
/****************************************************************************
*
*  writeProcReset
*
****************************************************************************/

int writeProcReset( struct file *file, const char *buf, int count, void *data )
{
   PROFILER *instance = ( PROFILER * )data;
   char input;

   if ( count > sizeof( int ) )
   {
       count = sizeof( int );
   }
 
   if ( copy_from_user( &input, buf, count ) )
   {
       return -EFAULT;
   }

   gosSemTake( instance->semData );
   if ( atoi(&input) )
   {
      memset( &instance->profilerStats, 0, sizeof( STATS ));
      memset( &instance->profilerData, 0, sizeof( DATA ));
      instance->profilerState.alphaFactor = ( 1<< 4 );   /* 1/16 in Q8 number */
      instance->profilerData.pstart = instance->profilerData.start_temp;
      instance->profilerData.pend = instance->profilerData.end_temp;
      instance->profilerState.avgSum = 0;
   }
   gosSemGive( instance->semData );
   return count;
}

/****************************************************************************
*
*  create_profiler_proc
*  
****************************************************************************/
int create_profiler_proc( PROFILER *instance )
{
   instance->procDir = proc_mkdir( instance->procName, NULL );
   if ( instance->procDir == NULL )
   {
      printk( KERN_ERR "%s: Failed to create main directory for proc entries\n", __FUNCTION__ );
      return 1;
   }

   create_proc_read_entry( "history", 0, instance->procDir, readProcHistory, (void *)instance );
   create_proc_read_entry( "stats", 0, instance->procDir, readProcStats, (void *)instance );

   /********** Creating Enable Proc **********/
   instance->procEnable = create_proc_entry( "enable", 0644, instance->procDir );
   if ( instance->procEnable == NULL)
   {
      printk( KERN_ERR "%s: Failed to create Enable directory for proc entries\n", __FUNCTION__ );
      return 1;
   }
   instance->procEnable->data = instance;
   instance->procEnable->write_proc = (write_proc_t *)writeProcEnable;
   instance->procEnable->read_proc = (read_proc_t *)readProcEnable;

   /********** Creating Reset Proc **********/
   instance->procReset = create_proc_entry( "reset", 0644, instance->procDir );
   if ( instance->procReset == NULL)
   {
      printk( KERN_ERR "%s: Failed to create Reset directory for proc entries\n", __FUNCTION__ );
      return 1;
   }
   instance->procReset->data = instance;
   instance->procReset->write_proc = (write_proc_t *)writeProcReset;

   /********** Creating Alphafactor Proc **********/
   instance->procAlphafactor = create_proc_entry( "alpha-factor", 0644, instance->procDir );
   if ( instance->procAlphafactor == NULL)
   {
      printk( KERN_ERR "%s: Failed to create Alphafactor directory for proc entries\n", __FUNCTION__ );
      return 1;
   }
   instance->procAlphafactor->data = instance;
   instance->procAlphafactor->write_proc = (write_proc_t *)writeProcAlphafactor;
   instance->procAlphafactor->read_proc = (read_proc_t *)readProcAlphafactor;

   return 0;
}

/****************************************************************************
*
*  remove_profiler_proc
*  
****************************************************************************/
void remove_profiler_proc( PROFILER *instance )
{
   remove_proc_entry( "stats", instance->procDir );
   remove_proc_entry( "history", instance->procDir );
   remove_proc_entry( "enable", instance->procDir );
   remove_proc_entry( "reset", instance->procDir );
   remove_proc_entry( "alpha-factor", instance->procDir );
   remove_proc_entry( instance->procName, NULL );
}

/****************************************************************************
*
* getFrameProfiler
* 
***************************************************************************/

FRAME_PROFILER_HANDLE getFrameProfiler( void )
{
   int rc;
   PROFILER *profiler_ptr;

   if ( !gInitializeProfilerArray )
   {
      memset( &gProfilerArray, 0, MAX_PROFILERS*sizeof( PROFILER ));
      gInitializeProfilerArray = 1;
         
      rc = gosSemAlloc( "FP_semWrite", 1, &FP_semWrite );
      if ( rc )
      {
         printk( KERN_ERR "Failed to create profiler thread semaphore\n" );
         return 0;
      }
   }

   gosSemTake( FP_semWrite );
   for ( profiler_ptr = gProfilerArray; profiler_ptr < gProfilerArray + MAX_PROFILERS; profiler_ptr++ )
   {
      if ( !profiler_ptr->profilerState.inuse )
      {

         profiler_ptr->profilerState.inuse = 1;
         gosSemGive( FP_semWrite );
         return profiler_ptr;
      }
   }
   gosSemGive( FP_semWrite );
   return 0;
}

/****************************************************************************
*
* removeFrameProfiler
* 
***************************************************************************/

void removeFrameProfiler( PROFILER *instance )
{
   memset( instance, 0, sizeof( PROFILER ));
   instance = NULL;
}

/****************************************************************************
*
* frame_profiler_init
* 
***************************************************************************/

FRAME_PROFILER_HANDLE frame_profiler_init( char proc_name[] )
{
   int rc;

   PROFILER *instance = getFrameProfiler();
   if ( instance == NULL )
   {
      printk( KERN_ERR "ERROR allocating profilers\n" );
      return 0;
   }

   strcpy( instance->procName, proc_name );
   instance->profilerState.enable = 1;
   instance->profilerState.alphaFactor = ( 1<< 4 );  /* 1/16 in Q8 number */
   instance->profilerData.pstart = instance->profilerData.start_temp;
   instance->profilerData.pend = instance->profilerData.end_temp;

   rc = create_profiler_proc( instance );
   if ( rc )
   {
      printk( KERN_ERR "Failed to create profiler procs\n");
      return 0;
   }

   if (( instance->profilerTask == NULL ) || IS_ERR( instance->profilerTask ))
   {
      instance->profilerTask = kthread_run( profilerThread, instance, instance->procName );

      if ( IS_ERR( instance->profilerTask ))
      {
         printk( KERN_ERR "Failed to start profiler thread: %ld\n", PTR_ERR( instance->profilerTask ));
         remove_profiler_proc( instance );
         return 0;
      }
   }

   rc = gosSemAlloc( "FP_semData", 1, &instance->semData );
   if ( rc )
   {
      printk( KERN_ERR "Failed to create profiler thread semaphore\n" );
      remove_profiler_proc( instance );
      kthread_stop( instance->profilerTask );
      return 0;
   }
   return instance;
}

/****************************************************************************
* 
* frame_profiler_exit
*
****************************************************************************/

int frame_profiler_exit( FRAME_PROFILER_HANDLE profilerHandle )
{
   PROFILER *instance = (PROFILER *) profilerHandle;
   int rc = 0;

   remove_profiler_proc( instance );

   rc = gosSemFree( instance->semData );
   if ( rc )
   {
      printk( KERN_ERR "Failed to delete profiler thread semaphore\n" );
   }

   kthread_stop( instance->profilerTask );
   removeFrameProfiler( instance );

   return rc;
}

/****************************************************************************
* 
* FP_startProfiling
*
****************************************************************************/

void FP_startProfiling( FRAME_PROFILER_HANDLE profilerHandle )
{
   PROFILER *instance = (PROFILER *) profilerHandle;

   if ( instance->profilerState.enable )
   {
      *instance->profilerData.pstart = timer_get_tick_count();
      instance->profilerData.pstart++;

      if ( instance->profilerData.pstart == ( instance->profilerData.start_temp + HISTORY_SIZE ))
      {
         instance->profilerData.pstart = instance->profilerData.start_temp;
         gosSemTake( instance->semData );
         memcpy( &instance->profilerData.start[0], &instance->profilerData.start_temp[0], HISTORY_SIZE*sizeof( &instance->profilerData.start_temp[0]) );
         gosSemGive( instance->semData );
      }
   }
}

/****************************************************************************
* 
* FP_stopProfiling
*
****************************************************************************/

void FP_stopProfiling( FRAME_PROFILER_HANDLE profilerHandle )
{
   PROFILER *instance = (PROFILER *) profilerHandle;

   if ( instance->profilerState.enable )
   {
      *instance->profilerData.pend = timer_get_tick_count();
      instance->profilerData.pend++;

      if ( instance->profilerData.pend == ( instance->profilerData.end_temp + HISTORY_SIZE ))
      {
         instance->profilerData.pend = instance->profilerData.end_temp;
         gosSemTake( instance->semData );
         memcpy( &instance->profilerData.end[0], &instance->profilerData.end_temp[0], HISTORY_SIZE*sizeof( &instance->profilerData.end_temp[0]) );
         gosSemGive( instance->semData );
         wake_up_process( instance->profilerTask );
      }
   }
}

/****************************************************************************
* 
* percentAvg - Numerator is the sum of AVG_RANGE elements processing times. 
*              When a new time is added, the oldest time is deleted. 
*              Denominator is the time it takes to processes first element to the last element.
*
****************************************************************************/

unsigned int percentAvg( PROFILER *instance, int index )
{
   instance->profilerState.avgSum += instance->profilerData.history[1] - instance->profilerData.history[AVG_RANGE];

   if ( (index - AVG_RANGE + 1) < 0 )
   {
      return (( instance->profilerState.avgSum*10000 ) / ( microSec(instance->profilerData.start[index] - instance->profilerData.start[index - AVG_RANGE + 1 + HISTORY_SIZE] ) + 1));
   }
   else
   {
      return (( instance->profilerState.avgSum*10000 ) / ( microSec(instance->profilerData.start[index] - instance->profilerData.start[index - AVG_RANGE + 1] ) + 1));
   }
}

/****************************************************************************
* 
* weightedAvg
*
****************************************************************************/

unsigned int weightedAvg( PROFILER *instance )
{
   if ( !instance->profilerStats.avgWeightedTime )
   {
      return instance->profilerStats.currTime;
   }
   else
   {
      return ((instance->profilerStats.currTime * instance->profilerState.alphaFactor) + (instance->profilerStats.avgWeightedTime * ((1 << 8) - instance->profilerState.alphaFactor))) >> 8;
   }
}

/****************************************************************************
* 
* FP_setEnable
*
****************************************************************************/

void FP_setEnable( FRAME_PROFILER_HANDLE profilerHandle, int enable )
{
   PROFILER *instance = (PROFILER *) profilerHandle;

   gosSemTake( instance->semData );
   instance->profilerState.enable = enable;
   gosSemGive( instance->semData );
}

/****************************************************************************
* 
* FP_setReset
*
****************************************************************************/

void FP_setReset( FRAME_PROFILER_HANDLE profilerHandle, int reset )
{
   PROFILER *instance = (PROFILER *) profilerHandle;

   gosSemTake( instance->semData );
   if ( reset )
   {
      memset( &instance->profilerStats, 0, sizeof( STATS ));
      memset( &instance->profilerData, 0, sizeof( DATA ));
      instance->profilerState.alphaFactor = ( 1<< 4 );   /* 1/16 in Q8 number */
      instance->profilerData.pstart = instance->profilerData.start_temp;
      instance->profilerData.pend = instance->profilerData.end_temp;
      instance->profilerState.avgSum = 0;
   }
   gosSemGive( instance->semData );
}

/****************************************************************************
* 
* FP_setAlphaFactor
*
****************************************************************************/

void FP_setAlphaFactor( FRAME_PROFILER_HANDLE profilerHandle, int alphafactor )
{
   PROFILER *instance = (PROFILER *) profilerHandle;

   gosSemTake( instance->semData );
   instance->profilerState.alphaFactor= alphafactor;
   gosSemGive( instance->semData );
}

/****************************************************************************
* 
* FP_getCurrTime
*
****************************************************************************/

unsigned int FP_getCurrTime( FRAME_PROFILER_HANDLE profilerHandle )
{
   PROFILER *instance = (PROFILER *) profilerHandle;
   return instance->profilerStats.currTime;
}

/****************************************************************************
* 
* FP_getMaxTime
*
****************************************************************************/

unsigned int FP_getMaxTime( FRAME_PROFILER_HANDLE profilerHandle )
{
   PROFILER *instance = (PROFILER *) profilerHandle;
   return instance->profilerStats.maxTime;
}

/****************************************************************************
* 
* FP_getAvgPercentTime
*
****************************************************************************/

unsigned int FP_getAvgPercentTime( FRAME_PROFILER_HANDLE profilerHandle )
{
   PROFILER *instance = (PROFILER *) profilerHandle;
   return instance->profilerStats.avgPercentTime;
}

/****************************************************************************
* 
* FP_getMaxAvgPercentTime
*
****************************************************************************/

unsigned int FP_getMaxAvgPercentTime( FRAME_PROFILER_HANDLE profilerHandle )
{
   PROFILER *instance = (PROFILER *) profilerHandle;
   return instance->profilerStats.maxAvgPercentTime;
}

/****************************************************************************
* 
* FP_getAvgWeightedTime
*
****************************************************************************/

unsigned int FP_getAvgWeightedTime( FRAME_PROFILER_HANDLE profilerHandle )
{
   PROFILER *instance = (PROFILER *) profilerHandle;
   return instance->profilerStats.avgWeightedTime;
}

/****************************************************************************
* 
* FP_getMaxAvgWeightedTime
*
****************************************************************************/

unsigned int FP_getMaxAvgWeightedTime( FRAME_PROFILER_HANDLE profilerHandle )
{
   PROFILER *instance = (PROFILER *) profilerHandle;
   return instance->profilerStats.maxAvgWeightedTime;
}

/****************************************************************************
* 
* profilerThread
*
****************************************************************************/

int profilerThread( void *data )
{
   PROFILER *instance = ( PROFILER * )data;
   int index;
   int i;

   KNLLOG( "************* Starting Profiler Thread **************\n" );

   while ( 1 )
   {
      set_current_state( TASK_INTERRUPTIBLE );
      schedule();

      if ( kthread_should_stop() )
      {
         break;
      }

      gosSemTake( instance->semData );

      for ( index = 0; index < (HISTORY_SIZE); index++ )
      {
         instance->profilerStats.currTime = microSec( instance->profilerData.end[index] - instance->profilerData.start[index] );
     
         for ( i = (HISTORY_SIZE-1); i > 0; i-- )
         {
            instance->profilerData.history[i] = instance->profilerData.history[i-1];
         }

         instance->profilerData.history[0] = instance->profilerStats.currTime;
         
         if ( instance->profilerStats.maxTime < instance->profilerStats.currTime )
         {
            instance->profilerStats.maxTime = instance->profilerStats.currTime;
         }

         instance->profilerStats.avgPercentTime = percentAvg(instance, index);

         if ( instance->profilerStats.maxAvgPercentTime < instance->profilerStats.avgPercentTime )
         {
            instance->profilerStats.maxAvgPercentTime = instance->profilerStats.avgPercentTime;
         }

         instance->profilerStats.avgWeightedTime = weightedAvg(instance);
         
         if ( instance->profilerStats.maxAvgWeightedTime < instance->profilerStats.avgWeightedTime )
         {
            instance->profilerStats.maxAvgWeightedTime = instance->profilerStats.avgWeightedTime;
         }
      }
      gosSemGive( instance->semData );
   }
   KNLLOG( "************* Exiting Profiler Thread **************\n" );
   return 0;
}

EXPORT_SYMBOL( frame_profiler_init );
EXPORT_SYMBOL( frame_profiler_exit );
EXPORT_SYMBOL( FP_startProfiling );
EXPORT_SYMBOL( FP_stopProfiling );
EXPORT_SYMBOL( FP_setEnable );
EXPORT_SYMBOL( FP_setReset );
EXPORT_SYMBOL( FP_setAlphaFactor );
EXPORT_SYMBOL( FP_getCurrTime );
EXPORT_SYMBOL( FP_getMaxTime );
EXPORT_SYMBOL( FP_getAvgPercentTime );
EXPORT_SYMBOL( FP_getMaxAvgPercentTime );
EXPORT_SYMBOL( FP_getAvgWeightedTime );
EXPORT_SYMBOL( FP_getMaxAvgWeightedTime );