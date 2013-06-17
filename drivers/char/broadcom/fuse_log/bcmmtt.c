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
*   software in any way with any other Broadcom software provided under a
*   license other than the GPL, without Broadcom's express prior written
*   consent.
*
****************************************************************************/

#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/broadcom/csl_types.h>

#include "bcmmtt.h"
#include "bcmlog.h"

/**
 *	MTT frame constants
 **/
/* message type SDL (high bit indicates AP)*/
static const unsigned short MTTLOG_MsgTypeSDL = 0x8001;
/* payload is ASCII string (high bit indicates AP)*/
static const unsigned short MTTLOG_MsgTypeASCII = 0x8002;
/* MTT version*/
static const unsigned char MTTLOG_MttVersion = 1;
/* number of frame (overhead) bytes*/
static const int MTTLOG_NumFrameBytes = 15;
/* index of length byte 0*/
static const int MTTLOG_FrameLenByte0 = 10;
/* index of length byte 1*/
static const int MTTLOG_FrameLenByte1 = 11;

static unsigned char cp_crash_frame_counter;

/**
 *	return length of null-terminated string
 *	@param	(in)	pointer to null-terminated string
 *	@return	int	string length excluding terminator
 **/
static int MTTLOG_StringLen(const char *c)
{
	int i = 0;
	while (*c++)
		i++;
	return i;
}

/**
 *	read CPU time
 *	@return	unsigned int		CPU time in millisecond
 **/
static unsigned int MTTLOG_GetTime(void)
{
	unsigned long long t;

	t = cpu_clock(0);
	do_div(t, 1000000);

	return (unsigned long)t;
}

/**
 *	compute 16-bit checksum of 8-bit byte array
 *	@param	(in)	pointer to byte array
 *	@param	(in)	number of bytes in array
 *	@return	int	checksum
 **/
unsigned short MTTLOG_Checksum16(unsigned char *data, unsigned long len)
{
	unsigned short csum = 0;
	unsigned long i;

	for (i = 0; i < len; i++)
		csum = csum + (unsigned short)(data[i]);

	return csum;
}

/**
 *	Return the buffer length required to frame a string of length 'size'.  Return -1
 *	on error (e.g., 'size' <= 0)
 *
 *	@param	size	(in)	return length of a buffer to frame string of size 'size'
 *	@return	int		frame size, or -1 on error
 **/
int BCMMTT_GetRequiredFrameLength(int size)
{
	if (size > 0)
		return size + MTTLOG_NumFrameBytes;

	return -1;
}

/**
 *	Return the MTT frame header required for MTT signal payload of specified size
 *
 *	@param	inPayloadSize	(in)	size of MTT payload to be sent
 *	@param	outFrameHdrBuf	(out)	buffer where frame header should be stored
 *					(allocated by caller)
 *	@return	int			size of header written to outFrameHdrBuf or -1 on error
 **/
int BCMMTT_MakeMTTSignalHeader(unsigned short inPayloadSize,
			       unsigned char *outFrameHdrBuf)
{
	unsigned char *pHbuf;
	unsigned short i;
	unsigned long trace_time_stamp;

	trace_time_stamp = MTTLOG_GetTime();

	pHbuf = outFrameHdrBuf;

	*pHbuf++ = MTTLOG_FrameSync0;
	*pHbuf++ = MTTLOG_FrameSync1;

	if (CpCrashDumpInProgress())
		*pHbuf++ = cp_crash_frame_counter++;
	else
		pHbuf++;

	*pHbuf++ = MTTLOG_MttVersion;
	*pHbuf++ = trace_time_stamp >> 24;
	*pHbuf++ = (trace_time_stamp >> 16) & 0xFF;
	*pHbuf++ = (trace_time_stamp >> 8) & 0xFF;
	*pHbuf++ = trace_time_stamp & 0xFF;
	i = MTTLOG_MsgTypeSDL;
	*pHbuf++ = i >> 8;
	*pHbuf++ = i & 0xFF;
	*pHbuf++ = inPayloadSize >> 8;
	*pHbuf++ = inPayloadSize & 0xFF;
	/* done at logging driver to assure sequential increase of*/
	if (CpCrashDumpInProgress())
		*pHbuf++ = MTTLOG_Checksum16(outFrameHdrBuf, 12);
	else
		pHbuf++;

	return (int)(pHbuf - outFrameHdrBuf);
}

/**
 *	Frame a string for output to MTT.  Characters not in [0x20, 0x7e]
 *	are converted to blanks (0x20).
 *
 *	@param	p_dest	(out)	pointer to destination buffer
 *	@param	p_src	(in)	pointer to source buffer
 *	@param	buflen	(in)	size of the destination buffer
 *	@return	int		frame length, or -1 on error
 **/
int BCMMTT_FrameString(char *p_dest, const char *p_src, int buflen)
{
	int slen = MTTLOG_StringLen(p_src);
	int n;
	char *pSbuf;
	unsigned long systime;

	if (slen == 0)
		return 0;


	if (slen > buflen - MTTLOG_NumFrameBytes)
		return 0;


	systime = MTTLOG_GetTime();

	pSbuf = p_dest;

	*pSbuf++ = MTTLOG_FrameSync0;
	*pSbuf++ = MTTLOG_FrameSync1;
	if (CpCrashDumpInProgress())
		*pSbuf++ = cp_crash_frame_counter++;
	else
		pSbuf++;
	*pSbuf++ = MTTLOG_MttVersion;

	*pSbuf++ = (systime >> 24);
	*pSbuf++ = (systime >> 16) & 0xff;
	*pSbuf++ = (systime >> 8) & 0xff;
	*pSbuf++ = (systime) & 0xff;

	*pSbuf++ = MTTLOG_MsgTypeASCII >> 8;
	*pSbuf++ = MTTLOG_MsgTypeASCII & 0xFF;

	*pSbuf++ = slen >> 8;
	*pSbuf++ = slen & 0xFF;

	if (CpCrashDumpInProgress()) {
		n = pSbuf - p_dest;
		*pSbuf++ = MTTLOG_Checksum16((unsigned char *)p_dest, n);
	} else
		pSbuf++;

	while (*p_src) {
		/* [0x20 .. 0x7E] is range of 'printable' characters*/
		if (*p_src < 0x20 || *p_src > 0x7E) {
			*pSbuf++ = ' ';
			p_src++;
		} else
			*pSbuf++ = *p_src++;
	}

	n = MTTLOG_Checksum16((unsigned char *)(pSbuf - slen), slen);

	*pSbuf++ = n >> 8;
	*pSbuf++ = n & 0xFF;

	return MTTLOG_NumFrameBytes + slen;
}
