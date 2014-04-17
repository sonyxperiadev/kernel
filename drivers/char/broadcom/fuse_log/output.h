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
*   software in any way with any other Broadcom software provided
*   under a license
*   other than the GPL, without Broadcom's express prior written consent.
*
****************************************************************************/

#ifndef __BCMLOG_OUTPUT_H__
#define __BCMLOG_OUTPUT_H__

#include "fifo.h"

/*#define BCMLOG_DEBUG_FLAG 1 */

/**
 *	Minimum and maximum number of bytes per RNDIS transfer.
 *
 **/
#define BCMLOG_OUTPUT_XFER_MAXBYTES		8192
#define BCMLOG_OUTPUT_XFER_MINBYTES		  64

#define BCMLOG_OUTPUT_MAX_LOG_PATHNAME    64

#define FUSE_LOG_CHANNEL	8

/**
 *	Output bytes to host
 *	@param  pUserBuf			(in)	pointer to user buffer
 *	@param	userBufSz			(in)	number of bytes
 *    @param  might_has_mtthead        (in) inside the user buffer,
 *                1 if there might be one or more MTT header
 *                0 if there in no MTT header
 **/
void BCMLOG_Output(unsigned char *pUserBuf, unsigned long userBufSz,
		   unsigned int might_has_mtthead);

/**
 *	Initialize output module
 *	@return	int zero if success, nonzero if error
 **/
BCMLOG_Fifo_t *BCMLOG_OutputInit(unsigned char *buffer, int buffer_size);
int Get_SDCARD_Available(void);
char *Get_SDCARD_LastFile(void);

#endif /* __BCMLOG_OUTPUT_H__ */
