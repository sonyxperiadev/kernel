/*****************************************************************************
* Copyright 2006 - 2011 Broadcom Corporation.  All rights reserved.
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

/*****************************************************************************
*
*  frame_profiler.h
*
*  PURPOSE:
*
*  This file contains the functions for profiling.
*
*  NOTES:
*
*****************************************************************************/

#ifndef FRAME_PROFILER_H
#define FRAME_PROFILER_H

#define INVALID_PROFILER_HANDLE     0

typedef void* FRAME_PROFILER_HANDLE;

FRAME_PROFILER_HANDLE frame_profiler_init( char [] );
int frame_profiler_exit( FRAME_PROFILER_HANDLE );
void FP_startProfiling( FRAME_PROFILER_HANDLE);
void FP_stopProfiling( FRAME_PROFILER_HANDLE );

void FP_setEnable( FRAME_PROFILER_HANDLE, int );
void FP_setReset( FRAME_PROFILER_HANDLE, int );
void FP_setAlphaFactor( FRAME_PROFILER_HANDLE, int );

unsigned int FP_getCurrTime ( FRAME_PROFILER_HANDLE );
unsigned int FP_getMaxTime ( FRAME_PROFILER_HANDLE );
unsigned int FP_getAvgPercentTime ( FRAME_PROFILER_HANDLE );
unsigned int FP_getMaxAvgPercentTime ( FRAME_PROFILER_HANDLE );
unsigned int FP_getAvgWeightedTime ( FRAME_PROFILER_HANDLE );
unsigned int FP_getMaxAvgWeightedTime ( FRAME_PROFILER_HANDLE );

#endif