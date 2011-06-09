/****************************************************************************
*
*     Copyright (c) 2007 Broadcom Corporation
*           All Rights Reserved
*
*     No portions of this material may be reproduced in any form without the
*     written permission of:
*
*           Broadcom Corporation
*           16215 Alton Parkway
*           P.O. Box 57013
*           Irvine, California 92619-7013
*
*     All information contained in this document is Broadcom Corporation
*     company private, proprietary, and trade secret.
*
****************************************************************************/
/**
*
*   @file   lcs_cplane_shared_def.h
*
*   @brief  This header provides the definition of the data types used by control plane Location Service API.
*
****************************************************************************/

#ifndef LCS_CPLANE_SHARED_DEF_H__
#define LCS_CPLANE_SHARED_DEF_H__		///< Include guard.

//The following header files should be included before include lcs_cplane_shared_def.h
// "mobcom_types.h"
// "taskmsgs.h"


/**
	The PDU data of the control plane messages.
**/
typedef struct
{
	UInt32 mDataLen;	///< The length of the data
	UInt8 * mData;      ///< The data buffer.
} LcsPduData_t;

/**
	The data of LCS messages.
**/
typedef struct
{
    ClientInfo_t mClientInfo;    ///< The registered handler client info
	UInt32 mDataLen;	///< The length of the data
	UInt8 * mData;      ///< The data buffer.
} LcsMsgData_t;

#endif	// LCS_CPLANE_SHARED_DEF_H__


