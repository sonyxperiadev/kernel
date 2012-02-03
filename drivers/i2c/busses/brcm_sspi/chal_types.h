/*****************************************************************************
*  Copyright 2001 - 2008 Broadcom Corporation.  All rights reserved.
*
*  Unless you and Broadcom execute a separate written software license
*  agreement governing use of this software, this software is licensed to you
*  under the terms of the GNU General Public License version 2, available at
*  http://www.gnu.org/licenses/old-license/gpl-2.0.html (the "GPL").
*
*  Notwithstanding the above, under no circumstances may you combine this
*  software in any way with any other Broadcom software provided under a
*  license other than the GPL, without Broadcom's express prior written
*  consent.
*
******************************************************************************/
/**
*
*  @file   chal_types.h
*
*  @brief  All variable types used by cHAL are defined here
*
*  @note
******************************************************************************/

#ifndef _CHAL_TYPES_H_
#define _CHAL_TYPES_H_

#ifdef __cplusplus
extern "C" {
#endif
/**
 *@defgroup cHAL_Interface
 *@{
 */

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef NULL
#define NULL 0
#endif

/* Needed for RDB register structures*/

/*
 *
 * Generic cHAL handler
 *
 */
#ifndef CHAL_HANDLE
	typedef void *CHAL_HANDLE;    /**< void pointer (32 bits wide)*/
#endif

/*
 *
 * Generic CHAL ISR type
 *
 */
#ifndef CHAL_ISREntry_t
	typedef void (*CHAL_ISREntry_t) (void);
#endif

/*
 *
 * Generic semaphore/mutex type
 *
 */
#ifndef CHAL_CriticalSectionControlObject_t
	typedef void *CHAL_CriticalSectionControlObject_t;
#endif

/*
 *
 * Generic status codes for CHAL
 *
 */
#ifndef ChalStatusCodes_t
/* RTOS glue Status/Error codes  */
	typedef enum {
		CHAL_OP_OK = 0,
		CHAL_OP_FAILED,
		CHAL_OP_INVALID,
		CHAL_OP_INVALID_PARAMETER,
		CHAL_OP_RTOS_INTF_NOT_REGISTERED,
		CHAL_OP_DRIVER_NOT_REGISTERED,
		/* Add more error codes here to match the RTOS glue */
		CHAL_OP_WRONG_ORDER,

		CHAL_OP_MAX = 255
	} ChalStatusCodes_t;
#endif

/** @} */

#ifdef __cplusplus
}
#endif
#endif				/* _CHAL_TYPES_H_ */
