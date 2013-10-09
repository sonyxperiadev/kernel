/****************************************************************************
*
* Copyright 2012 --2013 Broadcom Corporation.
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2, available at
* http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
*
*****************************************************************************/

#ifndef __CDC_H__
#define __CDC_H__


/*CDC command ids - enum values matches with command value*/
enum {
	CDC_CMD_RED = 1, /*Request to enter dormant (RED)*/
	CDC_CMD_REDCAN, /*Cancel request to enter drmnt (REDCAN)*/
	CDC_CMD_CDCE, /*Core dormant critical entry (CDCE)*/
	CDC_CMD_FDCE, /*Full dormant critical entry (FDCE)*/
	CDC_CMD_MDEC, /*Master Dormant exit complete (MDEC)*/
	CDC_CMD_SDEC, /*Slave Dormant exit complete (SDEC)*/
	CDC_CMD_MAX,
};

/*CDC cmd response/state ids
enum value matches with actual response value*/
enum {
	CDC_STATUS_POR = 1, /*Power on reset (POR)*/
	CDC_STATUS_RESFDM = (1 << 1), /*Resuming from full drmnt as a master*/
	CDC_STATUS_RESFDWAIT = (1 << 2), /*Full drmnt not ready to resume*/
	CDC_STATUS_RESFDS = (1 << 3), /*Resuming from full drmnt as a slave*/

	CDC_STATUS_RUN = (1 << 4), /*Running*/
	CDC_STATUS_NRFD = (1 << 5), /*Not ready for drmnt*/
	CDC_STATUS_RFD = (1 << 6), /*Ready for drmnt (RFD)*/
	CDC_STATUS_CENE = (1 << 7), /*Critical entry NOT established*/

	CDC_STATUS_RFDLC = (1 << 8), /*Ready for drmnt last core*/
	CDC_STATUS_RESCDWAIT = (1 << 9), /*Core drmnt not ready to resume*/
	CDC_STATUS_RESCD = (1 << 10), /*Resuming from core drmnt*/
	CDC_STATUS_CEOK = (1 << 11), /*Critical entry established*/

	CDC_STATUS_FDCEOK = (1 << 12), /*Full drmnt critical entry established*/
	CDC_STATUS_RESFDM_SHORT = (1 << 13), /*Resuming from shallow drmnt
						as a master*/
	CDC_STATUS_WAIT_CD_POK_STRONG = (1 << 14), /* ??*/
	CDC_STATUS_WAIT_CD_POK_WEAK = (1 << 15), /* ??*/

	CDC_STATUS_RESDFS_SHORT = (1 << 16), /*Resuming from shallow drmnt
							as a slave*/
	CDC_STATUS_RESVD = (1 << 17), /*Reserved*/
	CDC_STATUS_CD_CLAMP_ASSERT = (1 << 18), /*Core drmnt clamp assert*/
	CDC_STATUS_CLUSTER_WAIT_IDLE = (1 << 19), /*cluster wait idle*/

	CDC_STATUS_CLUSTER_DORMANT = (1 << 20), /*Cluster drmnt*/
	CDC_STATUS_CORE_DORMANT = (1 << 21), /*core drmnt*/
	CDC_STATUS_RESFD_SHORT_WAIT = (1 << 22), /*shallow drmnt not ready to
							resume*/
	CDC_STATUS_ERR, /*Error (ERR) - ???*/
};

/*CDC power status*/
enum {
	CDC_PWR_NORMAL,
	CDC_PWR_RSVD,
	CDC_PWR_DRMNT_L2_ON,
	CDC_PWR_DRMNT_L2_OFF,
};

/*CDC FSM ctrl*/
enum {
	FSM_CLR_TIMEOUT_INT = 1,
	FSM_CLR_L2_IS_ON =  1 << 1,
	FSM_CLR_FIRST_TO_POLL =  1 << 2,
	FSM_CLR_WAIT_IDLE_TIMEOUT =  1 << 3,
	FSM_CLR_ALL_STATUS = 0xF
};

/*CDC overrides...*/
enum {
	WAIT_IDLE_TIMEOUT,
	WAIT_IDLE_TIMEOUT_STATUS,
	STBYWFIL2_STATUS,
	STBYWFIL2_OVERRIDE,
	ARMSYSIDLE_TIMER,
	IS_IDLE_OVERRIDE
};

/*CDC counter timer values...*/
enum {
	FD_RESET_TIMER,
	CD_RESET_TIMER,
	WEAK_SWITCH_TIMER,
	STRONG_SWITCH_TIMER,
};



struct cdc_pdata {
	u32 flags;
	u32 nr_cpus;
};

int cdc_send_cmd(u32 cmd);
int cdc_send_cmd_for_core(u32 cmd, int cpu);
int cdc_acp_active(bool active);
int cdc_core_timer_in_use(bool in_use);
int cdc_get_status(void);
int cdc_get_status_for_core(int cpu);
int cdc_get_pwr_status(void);
int cdc_set_pwr_status(u32 status);
int cdc_get_usr_reg(void);
int cdc_set_usr_reg(u32 set_mask);
int cdc_clr_usr_reg(u32 clr_mask);
int cdc_set_fsm_ctrl(u32 fsm_ctrl);
int cdc_get_override(u32 type);
int cdc_set_override(u32 type, u32 val);
int cdc_dbg_bus_sel(u32 sel);
int cdc_get_dbg_bus_val(void);
int cdc_send_cmd_early(u32 cmd, int cpu);
int cdc_set_reset_counter(int type, u32 val);
int cdc_set_switch_counter(int type, u32 val);
int cdc_master_clk_gating_en(bool en);
int cdc_assert_reset_in_state(u32 states);
int cdc_assert_cdcbusy_in_state(u32 states);
int cdc_enable_isolation_in_state(u32 states);
int cdc_disable_cluster_dormant(bool disable);
#endif /*__CDC_H__*/
