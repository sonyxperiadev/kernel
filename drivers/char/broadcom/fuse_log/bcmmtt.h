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
extern "C" {
#endif				/* __cplusplus */

#define MTTLOG_FrameSync0	0xA5	/* frame sync byte 0 */
#define MTTLOG_FrameSync1	0xC3	/* frame sync byte 1 */

/**
 *	Log loss type
 **/
#define	MEMORY_FULL_ONCE	0x01	/* malloc failure */
#define	QUEUE_FULL_ONCE		0x02	/* Not used */
#define	SIOBUF_FULL_ONCE	0x04	/* output FIFO buffer full */
#define	SIOSEM_CONFLICT_ONCE	0x08	/* Not used */
#define	DEREF_FAIL_ONCE		0x10	/* Not used */
#define	INITLOG_CONFLICT_ONCE	0x20	/* other reason */

#define BCMLOG_LOGLOSS_SIZE	128

#define MTT_HEADER_SIZE         13
#define MTT_PAYLOAD_CS_SIZE     2

/**
 *  for tracking log loss
 **/
	struct BMCLOG_Error_t {
		unsigned int logLostStartTime;
		unsigned int logLostSioFull;
		unsigned int logLostSioSem;
		unsigned int msgLostMem;
		unsigned int msgLostQue;
		unsigned int copyLostMem;
		unsigned int msgLostInit;
	};

/**
 *	Return the buffer length required to frame a string of length 'size'.  Return -1
 *	on error (e.g., 'size' <= 0)
 *
 *	@param	size	(in)	return length of a buffer to frame string of size 'size'
 *	@return	int		frame size, or -1 on error
 **/
	int BCMMTT_GetRequiredFrameLength(int size);

/**
 *	Return the MTT frame header required for MTT signal payload of specified size
 *
 *	@param	inPayloadSize (in)	size of MTT payload to be sent
 *	@param	outFrameHdrBuf (out)	buffer where frame header should be stored
 *					(allocated by caller)
 *	@return	int			size of header written to outFrameHdrBuf or -1 on error
 **/
	int BCMMTT_MakeMTTSignalHeader(unsigned short inPayloadSize,
				       unsigned char *outFrameHdrBuf);

/**
 *	Frame a string for output to MTT.  Characters not in [0x20, 0x7e]
 *	are converted to blanks (0x20).
 *
 *	@param	p_dest	(out)	pointer to destination buffer
 *	@param	p_src	(in)	pointer to source buffer
 *	@param	buflen	(in)	size of the destination buffer
 *	@return	int		frame length, or -1 on error
 **/
	int BCMMTT_FrameString(char *p_dest, const char *p_src, int buflen);

/**
 *	Frame ap crash string for output to MTT.
 *
 *	@param	p_header(out)	pointer to destination header buffer
 *	@param	p_trailer(out)	pointer to destination trailer buffer
 *	@param	p_src	(in)	pointer to source buffer
 *	@return	int		string length, or 0 on error
 **/
	int BCMMTT_FrameString_nocopy(char *p_header, char *p_trailer,
				      const char *p_src);

/**
 *	compute 16-bit checksum of 8-bit byte array
 *	@param	(in)	pointer to byte array
 *	@param	(in)	number of bytes in array
 *	@return	int	checksum
 **/
	unsigned short MTTLOG_Checksum16(unsigned char *data,
					 unsigned long len);

/**
 *	read CPU time
 *	@return	unsigned int		CPU time in millisecond
 **/
	unsigned int MTTLOG_GetTime(void);

/**
 *	Function to track log loss
 *	@param	err_code	(in)	reason of the log loss
 **/
	void BCMLOG_RecordLogError(unsigned short err_code);

/**
 *	Function return log loss status
 **/
	unsigned char BCMLOG_IsLogError(void);

/**
 *	Function clear log loss status
 **/
	void BCMLOG_ClearLogError(void);

/**
 *	Function build log loss MTT message
 *	@param	ptr	(in)	allocated memory pointer
 **/
	unsigned int BCMLOG_BuildLogLossMessage(char *ptr);

#ifdef __cplusplus
}
#endif				/* __cplusplus */
#endif				/* __BCMMTT_H__ */
