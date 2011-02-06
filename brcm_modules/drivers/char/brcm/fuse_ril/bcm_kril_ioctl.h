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
*   software in any way with any other Broadcom software provided under a license 
*   other than the GPL, without Broadcom's express prior written consent.
*
****************************************************************************/

#ifndef _BCM_KRIL_IOCTL_H
#define _BCM_KRIL_IOCTL_H


#define BCM_RILIO_MAGIC 'A'

#define BCM_COMMAND_IOC_NR 1
#define BCM_RESULT_IOC_NR  2

#define BCM_RILIO_SET_COMMAND      _IOR(BCM_RILIO_MAGIC, BCM_COMMAND_IOC_NR, KRIL_Command_t)
#define BCM_RILIO_GET_RESPONSE     _IOR(BCM_RILIO_MAGIC, BCM_RESULT_IOC_NR, KRIL_Response_t)

#define KRIL_COMMAND_IOC_MAXNR	1

Int32 KRIL_SendCmd(UInt32 cmd, UInt32 arg);

#define KRIL_REQUEST_QUERY_SMS_IN_SIM             2001
#define KRIL_REQUEST_QUERY_SIM_EMERGENCY_NUMBER   2002

#endif //_BCM_KRIL_IOCTL_H
