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


#ifndef VCEB_OS_H_
#define VCEB_OS_H_

#include <stdarg.h>
#include "vceb_types.h"

/******************************************************************************
 The printf support is optional - might be useful for bringup and debug
 *****************************************************************************/

#define VCEB_PRINT_LEVEL_TRACE   0x00000001
#define VCEB_PRINT_LEVEL_INFO    0x00000002
#define VCEB_PRINT_LEVEL_ERROR   0x00000003

extern void vceb_printf( const uint32_t level, const char *format, ... );

/******************************************************************************
 Function to malloc / free memory
 *****************************************************************************/

extern void *vceb_calloc( const uint32_t size_in_bytes );

extern void vceb_free( void *ptr );

/******************************************************************************
 Function for comparing strings
 *****************************************************************************/

//Function to compare 2 strings
//returns 0 if the strings are a match
//Note - is case sensitive
extern int vceb_strcmp( const char *str1, const char *str2 );

//Function to copy 2 strings
//Returns destination ptr
extern size_t vceb_strlcpy( char *dest, const char *src, const size_t max_size );

/******************************************************************************
 Function to delay for x micro seconds
 *****************************************************************************/

extern void vceb_delay( const uint32_t time_in_usecs );

/******************************************************************************
 The semaphores are supplied on the off chance that you are running VCEB on a rich OS that might preempt access to any of the 
 functions inside this module. 
 *****************************************************************************/

//Function to create a semaphore
// Returns NULL if failed, otherwise a pointer to the semaphore
extern void *vceb_semaphore_create( const uint32_t initial_count );

//Function to delete a semaphore
extern void vceb_semaphore_delete( void *semaphore );

//Function to obtain a semaphore
extern int32_t vceb_semaphore_obtain(  void *semaphore,
                                       const uint32_t block );

//Function to release a semaphore
extern int32_t vceb_semaphore_release( void *semaphore );

#endif // VCEB_TYPES_H_
