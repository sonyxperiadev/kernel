/***************************************************************************
 *     Copyright (c) 2003-2008, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *   Interrupt test.
 *
 * Revision History:
 *
 * $brcm_Log: $
 ***************************************************************************/
#ifndef CHAL_DISPLAY_H__
#define CHAL_DISPLAY_H__

#ifdef __cplusplus
extern "C" {
#endif

void chal_display_enter(const char *func, char *file, long line);
void chal_display_exit(const char *func, char *file, long line);
void chal_display(const char *fmt, ...);
void chal_display_message(const char *fmt, ...);
void chal_display_error(const char *fmt, ...);
void chal_display_warning(const char *fmt, ...);
void chal_display_success(const char *fmt, ...);
void chal_display_fatal_error(const char *fmt, ...);

#ifdef __cplusplus
}
#endif

#endif /* CHAL_DISPLAY_H__ */
