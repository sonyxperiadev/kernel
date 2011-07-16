/*****************************************************************************
* Copyright 2010 - 2011 Broadcom Corporation.  All rights reserved.
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

#ifndef _BCM_HAPTICS_SETTINGS_H_
#define _BCM_HAPTICS_SETTINGS_H_

#define HAL_PORT_NAME	"halaudio.audioh7"
#define EPT_PORT_NAME	"haptics.bind0"

#define BCM_HAPTICS_SETTINGS \
{  \
   .halaudio_port_name = HAL_PORT_NAME,  \
   .ept_port_name      = EPT_PORT_NAME,  \
}

#endif /* _BCM_HAPTICS_SETTINGS_H_ */
