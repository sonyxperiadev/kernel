/***************************************************************************
 *     Copyright (c) 2002-2008, Broadcom Corporation
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
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
 
#include <stdarg.h>
#include <mach/csp/chal_display.h>

#ifndef BCM_DBG_H__
#define BCM_DBG_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef CONFIG_ENABLE_DEBUG
	#define BCM_DBG_ENTER()		chal_display_enter( __FUNCTION__,  __FILE__, __LINE__ )
	#define BCM_DBG_EXIT()		chal_display_exit( __FUNCTION__, __FILE__, __LINE__ )
	#define BCM_DBG_MSG( x )   	chal_display_message x
	#define BCM_DBG_WRN( x )	chal_display_warning x
	#define BCM_DBG_OUT( x )   	chal_display x
	#define BCM_DBG_ERR( x )  	chal_display_error x
	#define BCM_LOG_ERR( x )	chal_display_fatal_error x
	#define BCM_ASSERT_ERR( x ) if (!(x)) { BCM_DBG_ERR(( "%s [%s @%d] '%s'", __FUNCTION__, __FILE__, __LINE__, #x)); chal_fail(); }
	#define BCM_CHECK_ERR( x )  BCM_ASSERT_ERR( ((x) == BCM_SUCCESS) )
#else /* #ifndef CONFIG_ENABLE_DEBUG */
	#define BCM_DBG_ENTER()	
	#define BCM_DBG_EXIT()	
	#define BCM_DBG_MSG( x )   
	#define BCM_DBG_WRN( x )	
	#define BCM_DBG_OUT( x )   
	#define BCM_DBG_ERR( x )  
	#define BCM_LOG_ERR( x )
	#define BCM_ASSERT_ERR( x ) if (!(x)) { chal_fail(); }
	#define BCM_CHECK_ERR( x )  BCM_ASSERT_ERR( ((x) == BCM_SUCCESS) )
#endif /* CONFIG_ENABLE_DEBUG */


#ifdef __cplusplus
}
#endif

#endif /* BCM_DBG_H__ */
