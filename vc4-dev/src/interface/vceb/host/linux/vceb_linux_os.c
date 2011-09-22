/*****************************************************************************
* Copyright 2008 - 2010 Broadcom Corporation.  All rights reserved.
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


#include <linux/semaphore.h>
#include <linux/memory.h>
#include <linux/delay.h>
#include <linux/string.h>
#include <linux/slab.h>

#include "vceb_types.h"
#include "vceb_os.h"

#define VCEB_LINUX_OS_ENABLE_DEBUG

/******************************************************************************
 The printf support is optional - might be useful for bringup and debug
 *****************************************************************************/

void vceb_printf( const uint32_t level, const char *format, ... )
{
   va_list args;

   /* Initialize the varargs mechanism */
   va_start(args, format);

   if ( level == VCEB_PRINT_LEVEL_ERROR )
   {
       printk( KERN_ERR );
       vprintk( format, args );
   }
   #ifdef VCEB_LINUX_OS_ENABLE_DEBUG
   else
   if ( level == VCEB_PRINT_LEVEL_TRACE )
   {
       vprintk( format, args );
   }
   #endif
   else
   {
       printk( KERN_INFO );
       vprintk( format, args );
   }

   va_end (args);
}
EXPORT_SYMBOL( vceb_printf );

/******************************************************************************
 Function to delay for x micro seconds
 *****************************************************************************/

void vceb_delay( const uint32_t time_in_usecs )
{
   uint32_t local_time_in_usecs = time_in_usecs;

   if ( local_time_in_usecs > 1000 )
   {
       uint32_t local_time_in_msecs = local_time_in_usecs / 1000;

       if ( local_time_in_msecs < 20 )
       {
           mdelay( local_time_in_msecs );
       }
       else
       {
           // For longer delays, we don't want to consume 100% of the CPU, so use msleep
           msleep( local_time_in_msecs );
       }       
       local_time_in_usecs -= ( local_time_in_msecs * 1000 );
   }

   if ( local_time_in_usecs > 0 )
   {
      udelay( local_time_in_usecs );
   }
}
EXPORT_SYMBOL( vceb_delay );

/******************************************************************************
 The semaphores are supplied on the off chance that you are running VCEB on a rich OS that might preempt access to any of the 
 functions inside this module. 
 *****************************************************************************/

//Function to create a semaphore
// Returns NULL if failed, otherwise a pointer to the semaphore
void *vceb_semaphore_create( const uint32_t initial_count )
{
   struct semaphore *mysema = kmalloc( sizeof( struct semaphore ), GFP_KERNEL );

   if( NULL != mysema )
   {
      sema_init( mysema, initial_count );  
   }   

   return (void *)mysema;
}
EXPORT_SYMBOL( vceb_semaphore_create );

//Function to delete a semaphore
void vceb_semaphore_delete( void *mysema )
{
   if( NULL != mysema )
   {
      struct semaphore *semaphore_ptr = (struct semaphore *)mysema;

      kfree( semaphore_ptr );
   }   
}
EXPORT_SYMBOL( vceb_semaphore_delete );


//Function to obtain a semaphore
int32_t vceb_semaphore_obtain(void *mysema,
                              const uint32_t block )
{
   int32_t success = -1;

   if( NULL != mysema )
   {
      struct semaphore *semaphore_ptr = (struct semaphore *)mysema;

      if( block )
      {
         down( semaphore_ptr );
         success = 0;
      }
      else
      {
         if( 0 == down_trylock( semaphore_ptr ) )
         {
            success = 0;
         }
      }
   }

   return success;
}
EXPORT_SYMBOL( vceb_semaphore_obtain );

//Function to release a semaphore
int32_t vceb_semaphore_release( void *mysema )
{
   int32_t success = -1;

   if( NULL != mysema )
   {
      struct semaphore *semaphore_ptr = (struct semaphore *)mysema;

      up( semaphore_ptr );

      success = 0;
   }

   return success;
}
EXPORT_SYMBOL( vceb_semaphore_release );


/******************************************************************************
 Memory functions
 *****************************************************************************/

void *vceb_calloc( const uint32_t size_in_bytes )
{
   void *ptr = kmalloc( size_in_bytes, GFP_KERNEL );

   if( ptr )
      memset( ptr, 0, size_in_bytes );

   return ptr;
}
EXPORT_SYMBOL( vceb_calloc );

extern void vceb_free( void *ptr )
{
   kfree( ptr );
}
EXPORT_SYMBOL( vceb_free );

/******************************************************************************
 String functions
 *****************************************************************************/

//Function to compare 2 strings
//returns 0 if the strings are a match
//Note - is case sensitive
int vceb_strcmp( const char *str1, const char *str2 )
{
   return strcmp( str1, str2 );
}
EXPORT_SYMBOL( vceb_strcmp );

//Function to copy 2 strings
//Returns destination ptr
size_t vceb_strlcpy( char *dest, const char *src, const size_t max_size )
{
   return strlcpy( dest, src, max_size );
}
EXPORT_SYMBOL( vceb_strlcpy );


/* ************************************ The End ***************************************** */
