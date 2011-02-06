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

#ifndef _BCM_KRIL_COMMON_H
#define _BCM_KRIL_COMMON_H

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/fcntl.h>
#include <linux/list.h>
#include <asm/uaccess.h>
#include <asm/current.h>
#include <linux/ioctl.h>
#include <linux/types.h>
#include <linux/unistd.h>
#include <linux/errno.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/poll.h>
#ifdef CONFIG_HAS_WAKELOCK
#include <linux/wakelock.h>
#endif
#include "ril.h"
#include "bcm_kril.h"

#ifdef CONFIG_BRCM_FUSE_RIL_CIB
// for CIB, these headers are needed before capi2_mstypes.h is included
#include "mobcom_types.h"
#include "resultcode.h"
#include "common_defs.h"
#include "uelbs_api.h"
#include "ms_database_def.h"
#include "capi2_taskmsgs.h"
#include "common_sim.h"
#include "sim_def.h"
#include "netreg_def.h"
#else
#include "capi2_mstypes.h"
#endif

// for now, rpc_internal_api.h needs to be at this point in the
// header file list to ensure correct version of capi2_gen_mids.h
// is included; order dependency will go away once unified taskmsgs.h
// is completed in CIB
#include "rpc_internal_api.h"


#include "capi2_phonectrl_api.h"
#ifndef CONFIG_BRCM_FUSE_RIL_CIB
#include "capi2_cc_api.h"
#endif
#include "capi2_ss_api.h"
#include "capi2_ss_ds.h"
#include "capi2_stk_ds.h"
#include "capi2_sms_api.h"
#include "capi2_sms_ds.h"
#include "capi2_stk_api.h"
#include "bcmlog.h"

#ifndef CONFIG_BRCM_FUSE_RIL_CIB
#include <linux/broadcom/IPCInterface.h>
#endif

#include <linux/broadcom/bcm_kril_Interface.h>
#include <linux/broadcom/bcm_fuse_net_if.h>

#include "bcm_kril_debug.h"

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#endif //_BCM_KRIL_COMMON_H
