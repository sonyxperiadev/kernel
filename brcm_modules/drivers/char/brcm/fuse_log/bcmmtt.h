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
 *  @file   bcmmtt.h
 *
 *  @brief  Support for Broadcom Mobile Trace Terminal (MTT).
 *
 *	@note	Defines message frame formats used by MTT.
 *
 **/

#ifndef __BCMMTT_H__
#define __BCMMTT_H__

#ifdef __cplusplus
extern "C" 
{
#endif // __cplusplus

/**
 *	Return the buffer length required to frame a string of length 'size'.  Return -1
 *	on error (e.g., 'size' <= 0)
 *
 *	@param	size	(in)	return length of a buffer to frame string of size 'size'
 *	@return	int				frame size, or -1 on error
 **/
int BCMMTT_GetRequiredFrameLength( int size ) ;

/**
 *	Return the MTT frame header required for MTT signal payload of specified size
 *
 *	@param	inPayloadSize (in)	    size of MTT payload to be sent
 *	@param	inFrameHdrBufSize (in)  size of outFrameHdrBuf in bytes
 *	@param	outFrameHdrBuf (out)    buffer where frame header should be stored
 *                                  (allocated by caller)
 *	@return	int				    size of header written to outFrameHdrBuf or -1 on error
 **/
int BCMMTT_MakeMTTSignalHeader( unsigned short inPayloadSize, int inFrameHdrBufSize, unsigned char* outFrameHdrBuf );

/**
 *	Frame a string for output to MTT.  Characters not in [0x20, 0x7e]
 *	are converted to blanks (0x20).
 *
 *	@param	p_dest	(out)	pointer to destination buffer
 *	@param	p_src	(in)	pointer to source buffer
 *	@param	buflen	(in)	size of the destination buffer
 *	@return	int				frame length, or -1 on error
 **/
int BCMMTT_FrameString( char* p_dest, char* p_src, int buflen ) ;

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __BCMMTT_H__

