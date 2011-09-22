/*****************************************************************************
* Copyright 2008 - 2010 Broadcom Corporation.  All rights reserved.
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2, available at
* http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a
* license other than the GPL, without Broadcom's express prior written
* consent.
*****************************************************************************/




#ifndef VCEC_2ND_STAGE_CODE_H_
#define VCEC_2ND_STAGE_CODE_H_

#include "vceb_types.h"

/******************************************************************************
 Global defines for accessing the 2nd stage code - stored in an OS independant way usually
 The implementation for these functions is stored in the individual host port
 *****************************************************************************/

   extern void vceb_2nd_stage_code_init( const char * const instance_name );

   extern void *vceb_2nd_stage_code_get_ptr( const char * const instance_name );

   extern uint32_t vceb_2nd_stage_code_get_size( const char * const instance_name );

#endif // VCEC_2ND_STAGE_CODE_H_

