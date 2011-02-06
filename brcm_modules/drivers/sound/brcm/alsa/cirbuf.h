/*******************************************************************************************
Copyright 2010 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement 
governing use of this software, this software is licensed to you under the 
terms of the GNU General Public License version 2, available at 
http://www.gnu.org/copyleft/gpl.html (the "GPL"). 

Notwithstanding the above, under no circumstances may you combine this software 
in any way with any other Broadcom software provided under a license other than 
the GPL, without Broadcom's express prior written consent.
*******************************************************************************************/

#ifndef _CIRBUF_H
#define _CIRBUF_H

typedef struct _TCirBuf {
        volatile UInt16  * pBuf;
        volatile UInt16  * pWrIndex;
        volatile UInt16  * pRdIndex;
        UInt32 buffer_size;
        Int32 size_mask;
} TCirBuf, *PTCirBuf;


int	CBufReset(TCirBuf *pcbuf);
int	CBufInit(TCirBuf *pcbuf, void *pbuf, UInt32	size, UInt16 *pWrIndex, UInt16 *pRdIndex);

Int32 CBufReadAvaiable(TCirBuf * pcbuf);

Int32 CBufWriteAvaiable(TCirBuf * pcbuf);

Int32 CBufIncWrIndex(TCirBuf * pcbuf, UInt32 size_samples);

Int32 CBufIncRdIndex(TCirBuf * pcbuf, UInt32 size_samples);

UInt32 CBufGetWrBlocks(TCirBuf * pcbuf,		UInt32 size_samples,
                                 char ** buffer1_ptr, UInt32 * size1_ptr,
                                 char ** buffer2_ptr, UInt32 * size2_ptr);

UInt32 CBufWrite(TCirBuf * pcbuf, const char __user * buffer, UInt32 size_bytes);

UInt32 CBufGetRdBlocks(TCirBuf * pcbuf,		UInt32 size_samples,
                                 char ** buffer1_ptr, UInt32 * size1_ptr,
                                 char ** buffer2_ptr, UInt32 * size2_ptr);

UInt32 CBufRead(TCirBuf * pcbuf, char __user * buffer, UInt32 size_bytes);

#endif //_CIRBUF_H
