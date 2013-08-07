/****************************************************************************
*
*     Copyright (c) 2009 Broadcom Corporation
*
*   Unless you and Broadcom execute a separate written software license
*   agreement governing use of this software, this software is licensed to you
*   under the terms of the GNU General Public License version 2, available
*    at http://www.gnu.org/licenses/old-licenses/gpl-2.0.html (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a license
* other than the GPL, without Broadcom's express prior written consent.
*
****************************************************************************/
#include <linux/kernel.h>
#include <linux/io.h>
#include <linux/broadcom/ipcinterface.h>
#include <linux/regulator/consumer.h>
#include <linux/regulator/driver.h>
#include <linux/err.h>
#include <linux/unistd.h>
#include <linux/reboot.h>
#include <plat/kona_reset_reason.h>


#include "mobcom_types.h"
#include "resultcode.h"
#include "taskmsgs.h"

#include <linux/broadcom/ipcproperties.h>
#include "rpc_global.h"
#include "rpc_ipc.h"

#include "xdr_porting_layer.h"
#include "xdr.h"
#include "rpc_api.h"
#include "rpc_sync_api.h"

#include "sys_api.h"
#include "sys_common_rpc.h"
#include "sys_gen_rpc.h"
#include "sys_rpc.h"
#include "sysrpc_init.h"

#define THREE_VOLTS_IN_MICRO_VOLTS 3000000
#define THREE_PT_THREE_VOLTS_IN_MICRO_VOLTS 3300000
#define THREE_PT_ONE_VOLTS_IN_MICRO_VOLTS 3100000
#define TWO_PT_FIVE_VOLTS_IN_MICRO_VOLTS 2500000
#define ONE_PT_EIGHT_VOLTS_IN_MICRO_VOLTS 1800000

typedef struct {
	struct regulator *handle;
	Boolean isSimInit;
	PMU_SIMLDO_t simLdo;
	char devName[64];
} RegulatorInfo_t;

RegulatorInfo_t gRegulatorList[2] = { {NULL, FALSE, SIMLDO1, "sim_vcc"},
{NULL, FALSE, SIMLDO2, "sim2_vcc"}
};

#define REG_INDEX(a) ((a == SIMLDO2) ? 1 : 0)

#define DBG_INFO	KERN_INFO
#define DBG_ERROR	KERN_ERR

#define KRIL_DEBUG(level, fmt, args...) printk(level fmt, ##args)

static void SysRpc_OpenRegulator(PMU_SIMLDO_t ldo);

/* initialize sysrpc interface */
void KRIL_SysRpc_Init(void)
{
	static int inited = 0;

	if (!inited) {
		/* make sure we can't be re-initialized... */
		inited = 1;

		KRIL_DEBUG(DBG_INFO, " calling SYS_InitRpc\n");
		SYS_InitRpc();

		SysRpc_OpenRegulator(SIMLDO1);
		SysRpc_OpenRegulator(SIMLDO2);

	}
}

void SysRpc_OpenRegulator(PMU_SIMLDO_t ldo)
{
	RegulatorInfo_t *curReg = &gRegulatorList[REG_INDEX(ldo)];

	curReg->handle = regulator_get(NULL, curReg->devName);
	if (IS_ERR(curReg->handle)) {
		KRIL_DEBUG(DBG_ERROR, " **regulator_get (dev=%s) FAILED h=%p\n",
			   curReg->devName, curReg->handle);
	} else {
		KRIL_DEBUG(DBG_INFO,
			   " **regulator_get (dev=%s) PASS handle=%p\n",
			   curReg->devName, curReg->handle);
	}
}

Result_t Handle_CAPI2_SYSRPC_PMU_IsSIMReady(RPC_Msg_t *pReqMsg,
					    PMU_SIMLDO_t simldo)
{
	Result_t result = RESULT_OK;
	SYS_ReqRep_t data;
	int ret = 0;
	RegulatorInfo_t *curReg = &gRegulatorList[REG_INDEX(simldo)];

	if (!IS_ERR(curReg->handle)) {
		ret = regulator_is_enabled(curReg->handle);
		KRIL_DEBUG(DBG_INFO,
			   "  Handle_CAPI2_PMU_IsSIMReady ldo=%d handle=%p ret=%d\n\n",
			   (int)simldo, curReg->handle, (int)ret);
	} else
		KRIL_DEBUG(DBG_INFO,
			   " regulator_is_enabled Invalid Handle  %d\n",
			   (int)IS_ERR(curReg->handle));

	memset(&data, 0, sizeof(SYS_ReqRep_t));
	data.req_rep_u.CAPI2_SYSRPC_PMU_IsSIMReady_Rsp.val = curReg->isSimInit;	/*(Boolean)(regulator_is_enabled > 0); */
	data.result = result;
	Send_SYS_RspForRequest(pReqMsg, MSG_PMU_IS_SIM_READY_RSP, &data);

	KRIL_DEBUG(DBG_INFO, " Handle_CAPI2_PMU_IsSIMReady DONE active=%d\n",
		   (int)curReg->isSimInit);
	return result;
}

Result_t Handle_CAPI2_SYSRPC_PMU_ActivateSIM(RPC_Msg_t *pReqMsg,
					     PMU_SIMLDO_t simldo,
					     PMU_SIMVolt_t volt)
{
	Result_t result = RESULT_OK;
	SYS_ReqRep_t data;
	int simMicroVolts = 0;
	int ret;
	RegulatorInfo_t *curReg = &gRegulatorList[REG_INDEX(simldo)];

	memset(&data, 0, sizeof(SYS_ReqRep_t));
	data.result = result;

	if (IS_ERR(curReg->handle)) {
		KRIL_DEBUG(DBG_ERROR,
			   " enter Handle_CAPI2_PMU_ActivateSIM Invalid Handle ldo=%d handle=%p active=%d\n",
			   simldo, curReg->handle, curReg->isSimInit);
		Send_SYS_RspForRequest(pReqMsg, MSG_PMU_ACTIVATE_SIM_RSP,
				       &data);
		return result;
	}

	KRIL_DEBUG(DBG_INFO,
		   " enter Handle_CAPI2_PMU_ActivateSIM ldo=%d handle=%p active=%d\n",
		   simldo, curReg->handle, curReg->isSimInit);

	switch (volt) {
	case PMU_SIM3P0Volt:
		{
			KRIL_DEBUG(DBG_INFO, " PMU_SIM3P0Volt\n");
			/* 3.0 V SIM */
			simMicroVolts = THREE_VOLTS_IN_MICRO_VOLTS;
			break;
		}

	case PMU_SIM1P8Volt:
		{
			KRIL_DEBUG(DBG_INFO, " PMU_SIM1P8Volt\n");
			/* 1.8 V SIM */
			simMicroVolts = ONE_PT_EIGHT_VOLTS_IN_MICRO_VOLTS;
			break;
		}

	case PMU_SIM0P0Volt:
		{
			simMicroVolts = 0;

			KRIL_DEBUG(DBG_INFO,
				   " ** PMU_SIM0P0Volt - turn off regulator (FORCE)\n");

			if (curReg->isSimInit) {
				curReg->isSimInit = FALSE;
				ret = regulator_disable(curReg->handle);
				KRIL_DEBUG(DBG_INFO,
					   " regulator_disable returned 0x%x\n",
					   ret);
			}
			break;
		}

	default:
		{
			KRIL_DEBUG(DBG_INFO,
				   " unrecognized value for volt %d\n",
				   (int)volt);
			break;
		}

	}

	if (simMicroVolts > 0) {
		ret =
		    regulator_set_voltage(curReg->handle, simMicroVolts,
					  simMicroVolts);
		KRIL_DEBUG(DBG_INFO, " regulator_set_voltage returned %d\n",
			   ret);

		ret = regulator_enable(curReg->handle);
		KRIL_DEBUG(DBG_INFO, " regulator_enable returned %d\n", ret);
		/*Set SIMLDO mode to LPM in DSM*/
		ret = regulator_set_mode(curReg->handle, REGULATOR_MODE_IDLE);
		KRIL_DEBUG(DBG_INFO, "regulator_set_mode returned %d\n", ret);

		curReg->isSimInit = TRUE;
	}
	/*PMU_ActivateSIM(volt); */

	data.result = result;
	Send_SYS_RspForRequest(pReqMsg, MSG_PMU_ACTIVATE_SIM_RSP, &data);

	return result;
}


Result_t Handle_CAPI2_SYS_SoftResetSystem(RPC_Msg_t *pReqMsg, UInt32 param)
{
	Result_t result = RESULT_OK;
	SYS_ReqRep_t data;

	memset(&data, 0, sizeof(SYS_ReqRep_t));

	data.result = result;
	Send_SYS_RspForRequest(pReqMsg, MSG_SYS_SOFT_RESET_SYSTEM_RSP, &data);

	switch (param) {
	case SYS_HALT:                  /* 0x002 */
		kernel_halt();
		break;

	case SYS_POWER_OFF:             /* 0x003 */
		kernel_power_off();
		break;

	case SYS_RESTART:      /* 0x0001 or SYS_DOWN  */
	default:
		kernel_restart(NULL);
		break;
	}

	return result;
}

Result_t SetLDORegulator(enum SYS_LDO_Cmd_Type_t cmdType,
	const char *name, unsigned int mode)
{
	Result_t result = RESULT_OK;
	struct regulator *reg_handle;

	printk(KERN_INFO "Excuting LDO %s cmd type=%d\n",
		name, (int)cmdType);
	reg_handle = regulator_get(NULL, name);
	if (reg_handle != NULL) {
		if (cmdType == SYS_LDO_OFF) {
			printk(KERN_INFO "Turn off LDO\n");
			regulator_disable(reg_handle);
		} else if (cmdType == SYS_LDO_ON) {
			printk(KERN_INFO "Turn on LDO\n");
			if(regulator_enable(reg_handle)) {
				pr_err("%s: regulator enable failed\n",
						__func__);
				result = RESULT_ERROR;
				goto out;
			}
			/*Set LDO mode to LPM */
			printk(KERN_INFO "Set mode LDO\n");
			regulator_set_mode(reg_handle,
				mode);
		} else {
			printk(KERN_INFO "Invalid command type - not updated\n");
		}
		regulator_put(reg_handle);
	} else {
		printk(KERN_INFO "LDO handle is not valid!\n");
		result = RESULT_ERROR;
	}
	return result;
out:
	regulator_put(reg_handle);
	return result;
}

Result_t Handle_SYS_APSystemCmd(RPC_Msg_t *pReqMsg, UInt32 cmd,
	UInt32 param1, UInt32 param2, UInt32 param3)
{
	Result_t result = RESULT_OK;
	SYS_ReqRep_t data;

	memset(&data, 0, sizeof(SYS_ReqRep_t));

	switch (cmd) {
	case AP_SYS_CMD_RFLDO:
		printk(KERN_INFO "Excuting SYS_AP_CMD_RFLDO cmdType=%d\n",
			(int)param1);
		result = SetLDORegulator((enum SYS_LDO_Cmd_Type_t)param1,
				"rf", REGULATOR_MODE_STANDBY);
		break;

	case AP_SYS_CMD_SIMLDO:
		printk(KERN_INFO "Excuting SYS_AP_CMD_SIMLDO cmdType=%d\n",
			(int)param1);
		if ((SetLDORegulator((enum SYS_LDO_Cmd_Type_t)param1,
			"sim_vcc", REGULATOR_MODE_IDLE) == RESULT_ERROR) ||
			(SetLDORegulator((enum SYS_LDO_Cmd_Type_t)param1,
				"sim2_vcc", REGULATOR_MODE_IDLE) ==
					RESULT_ERROR)) {
			result = RESULT_ERROR;
		}
		break;

	case AP_SYS_CMD_SIM1LDO:
		printk(KERN_INFO "Excuting SYS_AP_CMD_SIMLDO cmdType=%d\n",
			(int)param1);
		result = SetLDORegulator((enum SYS_LDO_Cmd_Type_t)param1,
			"sim_vcc", REGULATOR_MODE_IDLE);
		break;

	case AP_SYS_CMD_SIM2LDO:
		printk(KERN_INFO "Excuting SYS_AP_CMD_SIMLDO cmdType=%d\n",
			(int)param1);
		result = SetLDORegulator((enum SYS_LDO_Cmd_Type_t)param1,
			"sim2_vcc", REGULATOR_MODE_IDLE);
		break;

	default:
		printk(KERN_INFO "Unhandled AP SYS CMD %d\n", (int)cmd);
		break;
	}

	data.result = result;
	Send_SYS_RspForRequest(pReqMsg, MSG_AP_SYS_CMD_RSP, &data);

	return result;
}

