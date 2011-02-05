/*****************************************************************************
*
*    (c) 2001-2010 Broadcom Corporation
*
* This program is the proprietary software of Broadcom Corporation and/or
* its licensors, and may only be used, duplicated, modified or distributed
* pursuant to the terms and conditions of a separate, written license
* agreement executed between you and Broadcom (an "Authorized License").
* Except as set forth in an Authorized License, Broadcom grants no license
* (express or implied), right to use, or waiver of any kind with respect to
* the Software, and Broadcom expressly reserves all rights in and to the
* Software and all intellectual property rights therein.
* IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS
* SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE
* ALL USE OF THE SOFTWARE.  
*
* Except as expressly set forth in the Authorized License,
*
* 1. This program, including its structure, sequence and organization,
*    constitutes the valuable trade secrets of Broadcom, and you shall use all
*    reasonable efforts to protect the confidentiality thereof, and to use
*    this information only in connection with your use of Broadcom integrated
*    circuit products.
*
* 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
*    RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
*    IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS
*    FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS,
*    QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU
*    ASSUME THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
*    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
*    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
*    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
*
*****************************************************************************/
/**
*
*   @file   osdal_os_driver.h
*
*   @brief  For DMA driver. Use the standard abstraction. 
*
****************************************************************************/

#ifndef OSDAL_OS_DRIVER_H
#define OSDAL_OS_DRIVER_H

#define __ATHENA__
#include <plat/osabstract/ossemaphore.h>
#include <plat/osabstract/ostypes.h>
#include <plat/osabstract/ostask.h>
#include <plat/osabstract/osqueue.h>
#define FUSE_APPS_PROCESSOR
#include "osdal_os.h"                 // abstract os types
#if defined(_ATHENA_) && defined(FUSE_APPS_PROCESSOR)
#include "osdal_os_service.h"         // abstract drivers  
#else
#define OSDAL_CLK_ENABLE_DMAC  0 // dummy
#define OSDAL_CLK_Enable(a)
#define OSDAL_CLK_Disable(a)
#endif

/* OS Task */
#define OSDAL_TASK_CREATE(e,n,p,s)      OSTASK_Create( (TEntry_t)e,(TName_t)n,(TPriority_t)p,(TStackSize_t)s )
#define OSDAL_TASK_DELETE(t)            OSTASK_Destroy( (Task_t)t )

/* OS Semaphore */
#define OSDAL_SEMA_CREATE(c,m)          OSSEMAPHORE_Create( (SCount_t)c,(OSSuspend_t)m )
#define OSDAL_SEMA_DELETE(s)            OSSEMAPHORE_Destroy( (Semaphore_t)s )
#define OSDAL_SEMA_COUNT(s)             OSSEMAPHORE_GetCnt( (Semaphore_t)s )
#define OSDAL_SEMA_NAMECHANGE(s,n)      OSSEMAPHORE_ChangeName( (Semaphore_t)s,(const char*)n )

/* OS INTERRUPT */
#define OSDAL_IRQ_CREATE(e,n,p,s)       OSINTERRUPT_Create( (IEntry_t)e,(IName_t)n,(IPriority_t)p,(IStackSize_t)s )
#define OSDAL_IRQ_DELETE(isr)           OSINTERRUPT_Destroy( (Interrupt_t)isr )
#define OSDAL_IRQ_TRIGGER(isr)          OSINTERRUPT_Trigger( (Interrupt_t)isr )
#define OSDAL_IRQ_REGISTER(id,isr)      IRQ_Register( (InterruptId_t)id, (isr_t)isr )
#define OSDAL_IRQ_DEREGISTER(id)        IRQ_Deregister( (InterruptId_t)id )

#endif

