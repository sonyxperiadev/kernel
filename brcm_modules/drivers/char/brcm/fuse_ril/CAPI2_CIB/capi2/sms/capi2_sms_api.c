/****************************************************************************
*
*     Copyright (c) 2007-2008 Broadcom Corporation
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
*   @file   capi2_sms_api.c
*
*   @brief  This file implementa the interface for CAPI2 SMS API.
*
****************************************************************************/
/**

*   @defgroup   CAPI2_SMSAPIGroup   Short Message Services
*   @ingroup    CAPI2_SMSGroup
*
*   @brief      This group defines the interfaces to the SMS system and provides
*				API documentation needed to create short message service applications.  
*				This group also deals with Cell Broadcast and VoiceMail indication services.
*				The APIs provided enables the user to write applications to create, store,
*				save, send and display SMS and Cell Broadcst messages.
*	
****************************************************************************/
#include "mobcom_types.h"
#include "resultcode.h"
#include "common_defs.h"
#include "uelbs_api.h"
#include "ms_database_def.h"
#include "capi2_taskmsgs.h"
#include "common_sim.h"
#include "sim_def.h"
#include "capi2_mstypes.h"
#include "capi2_sms_ds.h"
#include "capi2_reqrep.h"
#include "capi2_sms_api.h"


//-------------------------------------------------
// Function Prototype
//-------------------------------------------------

/**
 * @addtogroup CAPI2_SMSAPIGroup
 * @{
 */


/** @} */

