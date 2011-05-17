/****************************************************************************
*
*     Copyright (c) 2007-2010 Broadcom Corporation
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
*   @file   ms_database_log_types.h
*
*   @brief  This file contains the data types for binary logging purpose
*
****************************************************************************/

#ifndef _MS_DATABASE_LOG_TYPES_H_
#define _MS_DATABASE_LOG_TYPES_H_



/**************************************************************************************/
/******************* Data types for internal logging **********************************/
/**************************************************************************************/

typedef struct {
	ClientInfo_t clientInfo;
	MS_Element_t elemType;
	Boolean isGet;
	PhoneNumber_t phoneNum;
} T_MS_LOCAL_CC_ELEM_TEL_NUMBER;

// This defines the generic data type for any data payload of a UInt8
typedef struct {
	ClientInfo_t clientInfo;
	MS_Element_t elemType;
	Boolean isGet;
	UInt8 defaultVal;
} T_MS_DEFAULT_UINT8;


#endif // _MS_DATABASE_LOG_TYPES_H_
