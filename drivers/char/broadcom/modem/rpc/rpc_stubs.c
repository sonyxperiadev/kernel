/****************************************************************************
*
*     Copyright (c) 2009 Broadcom Corporation
*
*   Unless you and Broadcom execute a separate written software license 
*   agreement governing use of this software, this software is licensed to you 
*   under the terms of the GNU General Public License version 2, available 
*    at http://www.gnu.org/licenses/old-licenses/gpl-2.0.html (the "GPL"). 
*
*   Notwithstanding the above, under no circumstances may you combine this 
*   software in any way with any other Broadcom software provided under a license 
*   other than the GPL, without Broadcom's express prior written consent.
*
****************************************************************************/
/**
*
*   @file   rpc_stubs.c
*
*   @brief  This file includes stubs of functions from the Nucleus AP build
*           required for RPC.
*
*
****************************************************************************/
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/device.h>

#include <linux/fs.h>
#include <linux/fcntl.h>
#include <linux/list.h>
#include <linux/ioctl.h>
#include <linux/types.h>
#include <linux/unistd.h>
#include <linux/errno.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/poll.h>
#include <linux/slab.h>
#include <linux/jiffies.h> 
#include <asm/pgtable.h>
#include <linux/io.h>

#include "mobcom_types.h"

#define MAX_CLIENT_NUM              30		
#define AP_CLIENT_ID_START          64
#define AP_CLIENT_ID_END            99
#define	FIRST_USER_CLIENT_ID        AP_CLIENT_ID_START
#define	LAST_USER_CLIENT_ID         AP_CLIENT_ID_END

static unsigned char gClientIdIndex = 0;
unsigned char gSysClientIDs[255]={0};
void kRpcDebugPrintf(char* fmt, ...);

// ******************************************************************************
/**	
*   Assign unique client id
*
*	@return 		    unique client ID.
*
*   @note Copy of function in taskmsgs_util.c
*
********************************************************************************/
unsigned char SYS_GenClientID(void)
{
	unsigned char clientID, i;
	
	kRpcDebugPrintf("SYS_GenClientID current=%d",(FIRST_USER_CLIENT_ID + gClientIdIndex));

	clientID = FIRST_USER_CLIENT_ID + gClientIdIndex;

	for(i=FIRST_USER_CLIENT_ID;i<(FIRST_USER_CLIENT_ID + gClientIdIndex);i++)
	{
		if(gSysClientIDs[i] == 0)
		{
			gSysClientIDs[i] = 1;
	
			printk("SYS_GenClientID Found free slot=%d",i);
			return i;
		}
	}

	gSysClientIDs[clientID] = 1;

	printk("SYS_GenClientID Add new slot=%d",clientID);

	gClientIdIndex++;

	return clientID;
}


void SYS_ReleaseClientID(unsigned char clientID)
{
	if(clientID >= FIRST_USER_CLIENT_ID && clientID < (FIRST_USER_CLIENT_ID + gClientIdIndex) )
	{
		gSysClientIDs[clientID] = 0;
	
		kRpcDebugPrintf("SYS_ReleaseClientID released id=%d",clientID);
	}
	else
		kRpcDebugPrintf("SYS_ReleaseClientID (Not found) id=%d",clientID);

}

// ******************************************************************************
/**	
*   Check for registered client ID
*
*	@return 		    TRUE if registered client ID, FALSE otherwise
*
*   @note Copy of function in taskmsgs_util.c
*
********************************************************************************/
Boolean SYS_IsRegisteredClientID(UInt8 clientID)
{
	//Log_DebugPrintf(LOGID_MISC, "SYS_IsRegisteredClientID clientID=%d gClientIdIndex=%d gClientIdIndex2=%d", clientID, gSysClientIDs[clientID], gSysClientIDs[clientID+1]);
	return (gSysClientIDs[clientID] > 0)?TRUE: FALSE;
}



/************************************** Ring Buffer Logging ************************************************/
#define MAX_VAL 0xFF
#define MAX_LOG_SIZE 128
typedef struct
{
    UInt32 id;
	char logData[MAX_LOG_SIZE];
}LogDataElem_t;

spinlock_t  mLogLock;
LogDataElem_t gLogData[MAX_VAL];


#define INC_INDEX(x) ( (x+1) >= MAX_VAL )?0:(x+1)

typedef struct
{
	int ri;
	int wi;
	int isAvail;
}mRingBuffer_t;

mRingBuffer_t gLogBuffer={0};

void InitRingBuffer()
{
	static int first_time = 1;
	if(first_time)
	{
		first_time = 0;
		spin_lock_init( &mLogLock ) ;
	}
}

int getNextWriteIndex(mRingBuffer_t *p)
{
	unsigned long       irql ;
	int cur_wi;

	InitRingBuffer();
	spin_lock_irqsave( &mLogLock, irql ) ;
	if(p->wi == p->ri && p->isAvail)
	{
		p->ri = INC_INDEX(p->ri);
	}

	cur_wi = p->wi;
	
	p->wi = INC_INDEX(p->wi);
	p->isAvail = 1;
	spin_unlock_irqrestore( &mLogLock, irql ) ;    
	return cur_wi;
}

int peekNextReadIndex(mRingBuffer_t *p)
{
	unsigned long       irql ;
	int cur_ri;
	InitRingBuffer();
	spin_lock_irqsave( &mLogLock, irql ) ;
	if( p->ri == p->wi && p->isAvail == 0)
	{
		spin_unlock_irqrestore( &mLogLock, irql ) ;    
		return -1;
	}
	cur_ri = p->ri;
	spin_unlock_irqrestore( &mLogLock, irql ) ;    
	return cur_ri;
}

int getNextReadIndex(mRingBuffer_t *p)
{
	unsigned long       irql ;
	int cur_ri;

	InitRingBuffer();
	spin_lock_irqsave( &mLogLock, irql ) ;
	if( p->ri == p->wi && p->isAvail == 0)
	{
		spin_unlock_irqrestore( &mLogLock, irql ) ;    
		return -1;
	}
	
	cur_ri = p->ri;
	
	p->ri = INC_INDEX(p->ri);

	if(p->ri == p->wi)
	{
		p->isAvail = 0;
	}
	spin_unlock_irqrestore( &mLogLock, irql ) ;    
	return cur_ri;
}


void kRpcDebugPrintf(char* fmt, ...)
{
	va_list ap;
	char* buf;
	int index = 0;
	
	index = getNextWriteIndex(&gLogBuffer);

	buf = gLogData[index].logData;
	va_start(ap, fmt);
	vsnprintf(buf, (MAX_LOG_SIZE-1), fmt, ap);
	va_end(ap);
	//printk("Write: w:%d r:%d\n",gLogBuffer.wi,gLogBuffer.ri);
}

ssize_t kRpcReadLogData(char *destBuf, size_t len)
{
	ssize_t ret = 0;
	int index = 0;

	index = peekNextReadIndex(&gLogBuffer);

	if(index != -1)
	{
		size_t i;
		char* logbuf;
		int logsize;
		logbuf = gLogData[index].logData;
		logsize = strlen(logbuf);
		i = min_t(size_t, len, logsize);
		ret = copy_to_user(destBuf, logbuf, i) ? -EFAULT : i;
		getNextReadIndex(&gLogBuffer);
		//printk("Read: w:%d r:%d\n",gLogBuffer.wi,gLogBuffer.ri);
	}

	return ret;
}

int RpcLog_DetailLogEnabled()
{
	return 0;
}

//JW, to do, hack
Boolean IsBasicCapi2LoggingEnable(void)
{
	return true;
}

#define MAX_BUF_SIZE 1024
static char buf[MAX_BUF_SIZE];
int RpcLog_DebugPrintf(char* fmt, ...)
{
#ifdef CONFIG_BRCM_UNIFIED_LOGGING
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, MAX_BUF_SIZE, fmt, ap);
    va_end(ap);
    KRIL_DEBUG(DBG_INFO, "TS[%ld]%s\n", TIMER_GetValue(), buf);
#else
    if(IsBasicCapi2LoggingEnable())
    {
		va_list ap;
		char* buf;
		int index = 0;
		
		index = getNextWriteIndex(&gLogBuffer);

		buf = gLogData[index].logData;
		va_start(ap, fmt);
		vsnprintf(buf, (MAX_LOG_SIZE-1), fmt, ap);
		va_end(ap);
    }
#endif
    return 1;
}

