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

#include <linux/module.h>
#include <linux/uaccess.h>

#include "mobcom_types.h"
#include "cirbuf.h"



//based on csl_aud_buf.c

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//Buffer model 1
//  pBuf                 *pRdIndex(0..size)           *pWrite (0..size)
//  |                    |                            |                 
//	BBBBBBBBBBBBBBBBBBBBBxxxxxxxxxxxxxxxxxxxxxxxxxxxxxBBBBBBBBBBBBBBB
//
//Buffer model 2
//  pBuf                   *pWrIndex         *pRdIndex          
//  |                      |                  |                  
//	xxxxxxxxxxxxxxxxxxxxxxxBBBBBBBBBBBBBBBBBBBxxxxxxxxxxxxxxxxxxxxxxx
//
//   x - valid data B - invalid data  size_to_read= all x
//
//
//--------------------------------------------------------------------


int	CBufReset(TCirBuf *pcbuf)
{
	*pcbuf->pWrIndex = *pcbuf->pRdIndex = 0;
	return 0;
}

int	CBufInit(TCirBuf *pcbuf, void *pbuf, UInt32	size, UInt16 *pWrIndex, UInt16 *pRdIndex)
{
	pcbuf->pBuf = (UInt16 *)pbuf;
	pcbuf->buffer_size = size;
	pcbuf->pWrIndex = pWrIndex;
	pcbuf->pRdIndex = pRdIndex;
	pcbuf->size_mask = (size -1);
	CBufReset(pcbuf);
	return 0;
}



//data available for read
// 0 ... size-1
Int32 CBufReadAvaiable(TCirBuf * pcbuf)
{
        return ((*pcbuf->pWrIndex - *pcbuf->pRdIndex) & pcbuf->size_mask);
}

//space available for write
// 0 ... size-1
Int32 CBufWriteAvaiable(TCirBuf * pcbuf)
{
	//minus 1 to prevent RdIndex == WrIndex for write
	Int32	samples = (pcbuf->buffer_size - CBufReadAvaiable(pcbuf)) -1;
//	if (samples == pcbuf->buffer_size) //prevent RdIndex == WrIndex for write
//		samples = pcbuf->buffer_size - 1;
	return (samples>=0)?samples:0;
}

//Increase write index
Int32 CBufIncWrIndex(TCirBuf * pcbuf, UInt32 size_samples)
{
        return (*pcbuf->pWrIndex = (*pcbuf->pWrIndex + size_samples) % pcbuf->buffer_size);
}

//Increase read index
Int32 CBufIncRdIndex(TCirBuf * pcbuf, UInt32 size_samples)
{
        return (*pcbuf->pRdIndex = (*pcbuf->pRdIndex + size_samples) % pcbuf->buffer_size);
}


//
//Request buffers for write
//Parameters:
//		size_samples -- request size
//		buffer1_ptr, buffer2_ptr -- buffer block address
//		size1_ptr, size2_ptr -- block size
//Return:
//		size available (<= request size: size_samples)
//
UInt32 CBufGetWrBlocks(TCirBuf * pcbuf,		UInt32 size_samples,
                                 char ** buffer1_ptr, UInt32 * size1_ptr,
                                 char ** buffer2_ptr, UInt32 * size2_ptr)
{
        Int32 index;
        const UInt32 space = CBufWriteAvaiable(pcbuf);

	if(space<=0)
	{
              *buffer1_ptr = NULL;
              *size1_ptr = 0;
              *buffer2_ptr = NULL;
              *size2_ptr = 0;
		return 0;
	}
	
        if (size_samples > space) {
                size_samples = space;
        }

        index = (*pcbuf->pWrIndex);

        if ( (index + size_samples) > pcbuf->buffer_size) {
                Int32 chunk = pcbuf->buffer_size - index;

                *buffer1_ptr = (char *)&pcbuf->pBuf[index];
                *size1_ptr = chunk;

                *buffer2_ptr = (char *)&pcbuf->pBuf[0];
                *size2_ptr = size_samples - chunk;


        } else {
                *buffer1_ptr = (char *)&pcbuf->pBuf[index];
                *size1_ptr = size_samples;

                *buffer2_ptr = NULL;
                *size2_ptr = 0;
        }

        return size_samples;

}


UInt32 CBufWrite(TCirBuf * pcbuf, const char __user * _buffer, UInt32 size_bytes)
{
        char * buffer1;
        char * buffer2;
        Int32 size1;
        Int32 size2;
        UInt32 written;
        const UInt32 size_samples = (size_bytes >> 1);

	if(size_samples==0)
		return 0;

        written = CBufGetWrBlocks(pcbuf, size_samples, &buffer1, &size1, &buffer2, &size2);

	 copy_from_user(buffer1, _buffer, (size1 << 1)); // bytes
        if (0 < size2) 
	 {
            _buffer = ((UInt8 *)_buffer) + (size1 << 1);
            copy_from_user(buffer2, _buffer, (size2 << 1)); // bytes
        }

        CBufIncWrIndex(pcbuf, written);


        return (written << 1); // bytes
}





//
//Request buffers for read
//Parameters:
//		size_samples -- request size
//		buffer1_ptr, buffer2_ptr -- buffer block address
//		size1_ptr, size2_ptr -- block size
//Return:
//		size available (<= request size: size_samples)
//
UInt32 CBufGetRdBlocks(TCirBuf * pcbuf,		UInt32 size_samples,
                                 char ** buffer1_ptr, UInt32 * size1_ptr,
                                 char ** buffer2_ptr, UInt32 * size2_ptr)
{
        Int32 index;
        const UInt32 space = CBufReadAvaiable(pcbuf);

        if (size_samples > space) {
                size_samples = space;
        }

        index = (*pcbuf->pRdIndex);

        if ( (index + size_samples) > pcbuf->buffer_size) {
                Int32 chunk = pcbuf->buffer_size - index;

                *buffer1_ptr = (char *)&pcbuf->pBuf[index];
                *size1_ptr = chunk;

                *buffer2_ptr = (char *)&pcbuf->pBuf[0];
                *size2_ptr = size_samples - chunk;


        } else {
                *buffer1_ptr = (char *)&pcbuf->pBuf[index];
                *size1_ptr = size_samples;

                *buffer2_ptr = NULL;
                *size2_ptr = 0;
        }

        return size_samples;

}


UInt32 CBufRead(TCirBuf * pcbuf, char __user * _buffer, UInt32 size_bytes)
{
        char * buffer1;
        char * buffer2;
        Int32 size1;
        Int32 size2;
        UInt32 read;
        const UInt32 size_samples = (size_bytes >> 1);

	if(size_samples==0)
		return 0;

        read = CBufGetRdBlocks(pcbuf, size_samples, &buffer1, &size1, &buffer2, &size2);

	  copy_to_user(_buffer, buffer1, (size1 << 1)); // bytes
        if (0 < size2) 
		{
            _buffer = ((UInt8 *)_buffer) + (size1 << 1);
            copy_to_user(_buffer, buffer2, (size2 << 1)); // bytes
        }

        CBufIncRdIndex(pcbuf, read);


        return (read << 1); // bytes
}
