/* Copyright (c) 2012-2019, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#include "ipa_i.h"
#include <linux/delay.h>

#define IPA_RAM_UC_SMEM_SIZE 128
#define IPA_HW_INTERFACE_VERSION     0x2000
#define IPA_PKT_FLUSH_TO_US 100
#define IPA_UC_POLL_SLEEP_USEC 100
#define IPA_UC_POLL_MAX_RETRY 10000

/**
 * Mailbox register to Interrupt HWP for CPU cmd
 * Usage of IPA_UC_MAILBOX_m_n doorbell instead of IPA_IRQ_EE_UC_0
 * due to HW limitation.
 *
 */
#define IPA_CPU_2_HW_CMD_MBOX_m          0
#define IPA_CPU_2_HW_CMD_MBOX_n         23

static void ipa_uc_send_dma_addr_on_wq(struct work_struct *work);
static DECLARE_WORK(ipa_uc_send_dma_addr_work,
	ipa_uc_send_dma_addr_on_wq);

/**
 * enum ipa3_cpu_2_hw_commands - Values that represent the commands from the CPU
 * IPA_CPU_2_HW_CMD_NO_OP : No operation is required.
 * IPA_CPU_2_HW_CMD_UPDATE_FLAGS : Update SW flags which defines the behavior
 *                                 of HW.
 * IPA_CPU_2_HW_CMD_DEBUG_RUN_TEST : Launch predefined test over HW.
 * IPA_CPU_2_HW_CMD_DEBUG_GET_INFO : Read HW internal debug information.
 * IPA_CPU_2_HW_CMD_ERR_FATAL : CPU instructs HW to perform error fatal
 *                              handling.
 * IPA_CPU_2_HW_CMD_CLK_GATE : CPU instructs HW to goto Clock Gated state.
 * IPA_CPU_2_HW_CMD_CLK_UNGATE : CPU instructs HW to goto Clock Ungated state.
 * IPA_CPU_2_HW_CMD_MEMCPY : CPU instructs HW to do memcopy using QMB.
 * IPA_CPU_2_HW_CMD_RESET_PIPE : Command to reset a pipe - SW WA for a HW bug.
 * IPA_CPU_2_HW_CMD_GSI_CH_EMPTY : Command to check for GSI channel emptiness.
 * IPA_CPU_2_HW_CMD_REMOTE_IPA_INFO: Command to store remote IPA Info
 */
enum ipa3_cpu_2_hw_commands {
	IPA_CPU_2_HW_CMD_NO_OP                     =
		FEATURE_ENUM_VAL(IPA_HW_FEATURE_COMMON, 0),
	IPA_CPU_2_HW_CMD_UPDATE_FLAGS              =
		FEATURE_ENUM_VAL(IPA_HW_FEATURE_COMMON, 1),
	IPA_CPU_2_HW_CMD_DEBUG_RUN_TEST            =
		FEATURE_ENUM_VAL(IPA_HW_FEATURE_COMMON, 2),
	IPA_CPU_2_HW_CMD_DEBUG_GET_INFO            =
		FEATURE_ENUM_VAL(IPA_HW_FEATURE_COMMON, 3),
	IPA_CPU_2_HW_CMD_ERR_FATAL                 =
		FEATURE_ENUM_VAL(IPA_HW_FEATURE_COMMON, 4),
	IPA_CPU_2_HW_CMD_CLK_GATE                  =
		FEATURE_ENUM_VAL(IPA_HW_FEATURE_COMMON, 5),
	IPA_CPU_2_HW_CMD_CLK_UNGATE                =
		FEATURE_ENUM_VAL(IPA_HW_FEATURE_COMMON, 6),
	IPA_CPU_2_HW_CMD_MEMCPY                    =
		FEATURE_ENUM_VAL(IPA_HW_FEATURE_COMMON, 7),
	IPA_CPU_2_HW_CMD_RESET_PIPE                =
		FEATURE_ENUM_VAL(IPA_HW_FEATURE_COMMON, 8),
	IPA_CPU_2_HW_CMD_REG_WRITE                 =
		FEATURE_ENUM_VAL(IPA_HW_FEATURE_COMMON, 9),
	IPA_CPU_2_HW_CMD_GSI_CH_EMPTY              =
		FEATURE_ENUM_VAL(IPA_HW_FEATURE_COMMON, 10),
	IPA_CPU_2_HW_CMD_REMOTE_IPA_INFO           =
		FEATURE_ENUM_VAL(IPA_HW_FEATURE_COMMON, 11),
	IPA_CPU_2_HW_CMD_DMA_ADDR_INFO           =
		FEATURE_ENUM_VAL(IPA_HW_FEATURE_COMMON, 12),
};

/**
 * enum ipa3_hw_2_cpu_responses -  Values that represent common HW responses
 *  to CPU commands.
 * @IPA_HW_2_CPU_RESPONSE_NO_OP : No operation response
 * @IPA_HW_2_CPU_RESPONSE_INIT_COMPLETED : HW shall send this command once
 *  boot sequence is completed and HW is ready to serve commands from CPU
 * @IPA_HW_2_CPU_RESPONSE_CMD_COMPLETED: Response to CPU commands
 * @IPA_HW_2_CPU_RESPONSE_DEBUG_GET_INFO : Response to
 *  IPA_CPU_2_HW_CMD_DEBUG_GET_INFO command
 */
enum ipa3_hw_2_cpu_responses {
	IPA_HW_2_CPU_RESPONSE_NO_OP          =
		FEATURE_ENUM_VAL(IPA_HW_FEATURE_COMMON, 0),
	IPA_HW_2_CPU_RESPONSE_INIT_COMPLETED =
		FEATURE_ENUM_VAL(IPA_HW_FEATURE_COMMON, 1),
	IPA_HW_2_CPU_RESPONSE_CMD_COMPLETED  =
		FEATURE_ENUM_VAL(IPA_HW_FEATURE_COMMON, 2),
	IPA_HW_2_CPU_RESPONSE_DEBUG_GET_INFO =
		FEATURE_ENUM_VAL(IPA_HW_FEATURE_COMMON, 3),
};

/**
 * struct IpaHwMemCopyData_t - Structure holding the parameters
 * for IPA_CPU_2_HW_CMD_MEMCPY command.
 *
 * The parameters are passed as immediate params in the shared memory
 */
struct IpaHwMemCopyData_t  {
	u32 destination_addr;
	u32 source_addr;
	u32 dest_buffer_size;
	u32 source_buffer_size;
};

/**
 * struct IpaHwRegWriteCmdData_t - holds the parameters for
 * IPA_CPU_2_HW_CMD_REG_WRITE command. Parameters are
 * sent as 64b immediate parameters.
 * @RegisterAddress: RG10 register address where the value needs to be written
 * @RegisterValue: 32-Bit value to be written into the register
 */
struct IpaHwRegWriteCmdData_t {
	u32 RegisterAddress;
	u32 RegisterValue;
};

/**
 * union IpaHwCpuCmdCompletedResponseData_t - Structure holding the parameters
 * for IPA_HW_2_CPU_RESPONSE_CMD_COMPLETED response.
 * @originalCmdOp : The original command opcode
 * @status : 0 for success indication, otherwise failure
 * @reserved : Reserved
 *
 * Parameters are sent as 32b immediate parameters.
 */
union IpaHwCpuCmdCompletedResponseData_t {
	struct IpaHwCpuCmdCompletedResponseParams_t {
		u32 originalCmdOp:8;
		u32 status:8;
		u32 reserved:16;
	} __packed params;
	u32 raw32b;
} __packed;

/**
 * union IpaHwUpdateFlagsCmdData_t - Structure holding the parameters for
 * IPA_CPU_2_HW_CMD_UPDATE_FLAGS command
 * @newFlags: SW flags defined the behavior of HW.
 *	This field is expected to be used as bitmask for enum ipa3_hw_flags
 */
union IpaHwUpdateFlagsCmdData_t {
	struct IpaHwUpdateFlagsCmdParams_t {
		u32 newFlags;
	} params;
	u32 raw32b;
};

/**
 * union IpaHwChkChEmptyCmdData_t -  Structure holding the parameters for
 *  IPA_CPU_2_HW_CMD_GSI_CH_EMPTY command. Parameters are sent as 32b
 *  immediate parameters.
 * @ee_n : EE owner of the channel
 * @vir_ch_id : GSI virtual channel ID of the channel to checked of emptiness
 * @reserved_02_04 : Reserved
 */
union IpaHwChkChEmptyCmdData_t {
	struct IpaHwChkChEmptyCmdParams_t {
		u8 ee_n;
		u8 vir_ch_id;
		u16 reserved_02_04;
	} __packed params;
	u32 raw32b;
} __packed;


/**
 * Structure holding the parameters for IPA_CPU_2_HW_CMD_REMOTE_IPA_INFO
 * command.
 * @remoteIPAAddr: 5G IPA address : uC proxies Q6 doorbell to this address
 * @mboxN: mbox on which Q6 will interrupt uC
 */
struct IpaHwDbAddrInfo_t {
	u32 remoteIPAAddr;
	uint32_t mboxN;
} __packed;

static DEFINE_MUTEX(uc_loaded_nb_lock);
static BLOCKING_NOTIFIER_HEAD(uc_loaded_notifier);

struct ipa3_uc_hdlrs ipa3_uc_hdlrs[IPA_HW_NUM_FEATURES] = { { 0 } };

const char *ipa_hw_error_str(enum ipa3_hw_errors err_type)
{
	const char *str;

	switch (err_type) {
	case IPA_HW_ERROR_NONE:
		str = "IPA_HW_ERROR_NONE";
		break;
	case IPA_HW_INVALID_DOORBELL_ERROR:
		str = "IPA_HW_INVALID_DOORBELL_ERROR";
		break;
	case IPA_HW_DMA_ERROR:
		str = "IPA_HW_DMA_ERROR";
		break;
	case IPA_HW_FATAL_SYSTEM_ERROR:
		str = "IPA_HW_FATAL_SYSTEM_ERROR";
		break;
	case IPA_HW_INVALID_OPCODE:
		str = "IPA_HW_INVALID_OPCODE";
		break;
	case IPA_HW_INVALID_PARAMS:
		str = "IPA_HW_INVALID_PARAMS";
		break;
	case IPA_HW_CONS_DISABLE_CMD_GSI_STOP_FAILURE:
		str = "IPA_HW_CONS_DISABLE_CMD_GSI_STOP_FAILURE";
		break;
	case IPA_HW_PROD_DISABLE_CMD_GSI_STOP_FAILURE:
		str = "IPA_HW_PROD_DISABLE_CMD_GSI_STOP_FAILURE";
		break;
	case IPA_HW_GSI_CH_NOT_EMPTY_FAILURE:
		str = "IPA_HW_GSI_CH_NOT_EMPTY_FAILURE";
		break;
	default:
		str = "INVALID ipa_hw_errors type";
	}

	return str;
}

static void ipa3_log_evt_hdlr(void)
{
	int i;

	if (!ipa3_ctx->uc_ctx.uc_event_top_ofst) {
		ipa3_ctx->uc_ctx.uc_event_top_ofst =
			ipa3_ctx->uc_ctx.uc_sram_mmio->eventParams;
		if (ipa3_ctx->uc_ctx.uc_event_top_ofst +
			sizeof(struct IpaHwEventLogInfoData_t) >=
			ipa3_ctx->ctrl->ipa_reg_base_ofst +
			ipahal_get_reg_n_ofst(
				IPA_SW_AREA_RAM_DIRECT_ACCESS_n, 0) +
			ipa3_ctx->smem_sz) {
			IPAERR("uc_top 0x%x outside SRAM\n",
				ipa3_ctx->uc_ctx.uc_event_top_ofst);
			goto bad_uc_top_ofst;
		}

		ipa3_ctx->uc_ctx.uc_event_top_mmio = ioremap(
			ipa3_ctx->ipa_wrapper_base +
			ipa3_ctx->uc_ctx.uc_event_top_ofst,
			sizeof(struct IpaHwEventLogInfoData_t));
		if (!ipa3_ctx->uc_ctx.uc_event_top_mmio) {
			IPAERR("fail to ioremap uc top\n");
			goto bad_uc_top_ofst;
		}

		for (i = 0; i < IPA_HW_NUM_FEATURES; i++) {
			if (ipa3_uc_hdlrs[i].ipa_uc_event_log_info_hdlr)
				ipa3_uc_hdlrs[i].ipa_uc_event_log_info_hdlr
					(ipa3_ctx->uc_ctx.uc_event_top_mmio);
		}
	} else {

		if (ipa3_ctx->uc_ctx.uc_sram_mmio->eventParams !=
			ipa3_ctx->uc_ctx.uc_event_top_ofst) {
			IPAERR("uc top ofst changed new=%u cur=%u\n",
				ipa3_ctx->uc_ctx.uc_sram_mmio->eventParams,
				ipa3_ctx->uc_ctx.uc_event_top_ofst);
		}
	}

	return;

bad_uc_top_ofst:
	ipa3_ctx->uc_ctx.uc_event_top_ofst = 0;
}

/**
 * ipa3_uc_state_check() - Check the status of the uC interface
 *
 * Return value: 0 if the uC is loaded, interface is initialized
 *               and there was no recent failure in one of the commands.
 *               A negative value is returned otherwise.
 */
int ipa3_uc_state_check(void)
{
	if (!ipa3_ctx->uc_ctx.uc_inited) {
		IPAERR("uC interface not initialized\n");
		return -EFAULT;
	}

	if (!ipa3_ctx->uc_ctx.uc_loaded) {
		IPAERR("uC is not loaded\n");
		return -EFAULT;
	}

	if (ipa3_ctx->uc_ctx.uc_failed) {
		IPAERR("uC has failed its last command\n");
		return -EFAULT;
	}

	return 0;
}

/**
 * ipa3_uc_loaded_check() - Check the uC has been loaded
 *
 * Return value: 1 if the uC is loaded, 0 otherwise
 */
int ipa3_uc_loaded_check(void)
{
	return ipa3_ctx->uc_ctx.uc_loaded;
}
EXPORT_SYMBOL(ipa3_uc_loaded_check);

/**
 * ipa3_uc_register_ready_cb() - register a uC ready callback notifier block
 * @nb: notifier
 *
 * Register a callback to be called when uC is ready to receive commands. uC is
 * considered to be ready when it sends %IPA_HW_2_CPU_RESPONSE_INIT_COMPLETED.
 *
 * Return: 0 on successful registration, negative errno otherwise
 *
 * See blocking_notifier_chain_register() for possible errno values
 */
int ipa3_uc_register_ready_cb(struct notifier_block *nb)
{
	int rc;

	mutex_lock(&uc_loaded_nb_lock);

	rc = blocking_notifier_chain_register(&uc_loaded_notifier, nb);
	if (!rc && ipa3_ctx->uc_ctx.uc_loaded)
		(void) nb->notifier_call(nb, false, ipa3_ctx);

	mutex_unlock(&uc_loaded_nb_lock);

	return rc;
}
EXPORT_SYMBOL(ipa3_uc_register_ready_cb);

/**
 * ipa3_uc_unregister_ready_cb() - unregister a uC ready callback
 * @nb: notifier
 *
 * Unregister a uC loaded notifier block that was previously registered by
 * ipa3_uc_register_ready_cb().
 *
 * Return: 0 on successful unregistration, negative errno otherwise
 *
 * See blocking_notifier_chain_unregister() for possible errno values
 */
int ipa3_uc_unregister_ready_cb(struct notifier_block *nb)
{
	return blocking_notifier_chain_unregister(&uc_loaded_notifier, nb);
}
EXPORT_SYMBOL(ipa3_uc_unregister_ready_cb);

static void ipa3_uc_event_handler(enum ipa_irq_type interrupt,
				 void *private_data,
				 void *interrupt_data)
{
	union IpaHwErrorEventData_t evt;
	u8 feature;

	WARN_ON(private_data != ipa3_ctx);

	IPA_ACTIVE_CLIENTS_INC_SIMPLE();

	IPADBG("uC evt opcode=%u\n",
		ipa3_ctx->uc_ctx.uc_sram_mmio->eventOp);


	feature = EXTRACT_UC_FEATURE(ipa3_ctx->uc_ctx.uc_sram_mmio->eventOp);

	if (feature >= IPA_HW_FEATURE_MAX) {
		IPAERR("Invalid feature %u for event %u\n",
			feature, ipa3_ctx->uc_ctx.uc_sram_mmio->eventOp);
		IPA_ACTIVE_CLIENTS_DEC_SIMPLE();
		return;
	}
	/* Feature specific handling */
	if (ipa3_uc_hdlrs[feature].ipa_uc_event_hdlr)
		ipa3_uc_hdlrs[feature].ipa_uc_event_hdlr
			(ipa3_ctx->uc_ctx.uc_sram_mmio);

	/* General handling */
	if (ipa3_ctx->uc_ctx.uc_sram_mmio->eventOp ==
	    IPA_HW_2_CPU_EVENT_ERROR) {
		evt.raw32b = ipa3_ctx->uc_ctx.uc_sram_mmio->eventParams;
		IPAERR("uC Error, evt errorType = %s\n",
			ipa_hw_error_str(evt.params.errorType));
		ipa3_ctx->uc_ctx.uc_failed = true;
		ipa3_ctx->uc_ctx.uc_error_type = evt.params.errorType;
		ipa3_ctx->uc_ctx.uc_error_timestamp =
			ipahal_read_reg(IPA_TAG_TIMER);
		/* Unexpected UC hardware state */
		BUG();
	} else if (ipa3_ctx->uc_ctx.uc_sram_mmio->eventOp ==
		IPA_HW_2_CPU_EVENT_LOG_INFO) {
		IPADBG("uC evt log info ofst=0x%x\n",
			ipa3_ctx->uc_ctx.uc_sram_mmio->eventParams);
		ipa3_log_evt_hdlr();
	} else {
		IPADBG("unsupported uC evt opcode=%u\n",
				ipa3_ctx->uc_ctx.uc_sram_mmio->eventOp);
	}
	IPA_ACTIVE_CLIENTS_DEC_SIMPLE();

}

int ipa3_uc_panic_notifier(struct notifier_block *this,
		unsigned long event, void *ptr)
{
	int result = 0;
	struct ipa_active_client_logging_info log_info;

	IPADBG("this=%pK evt=%lu ptr=%pK\n", this, event, ptr);

	result = ipa3_uc_state_check();
	if (result)
		goto fail;

	IPA_ACTIVE_CLIENTS_PREP_SIMPLE(log_info);
	if (ipa3_inc_client_enable_clks_no_block(&log_info))
		goto fail;

	ipa3_ctx->uc_ctx.uc_sram_mmio->cmdOp =
		IPA_CPU_2_HW_CMD_ERR_FATAL;
	ipa3_ctx->uc_ctx.pending_cmd = ipa3_ctx->uc_ctx.uc_sram_mmio->cmdOp;
	/* ensure write to shared memory is done before triggering uc */
	wmb();
	ipahal_write_reg_n(IPA_IRQ_EE_UC_n, 0, 0x1);

	/* give uc enough time to save state */
	udelay(IPA_PKT_FLUSH_TO_US);

	IPA_ACTIVE_CLIENTS_DEC_SIMPLE();
	IPADBG("err_fatal issued\n");

fail:
	return NOTIFY_DONE;
}

static void ipa3_uc_response_hdlr(enum ipa_irq_type interrupt,
				void *private_data,
				void *interrupt_data)
{
	union IpaHwCpuCmdCompletedResponseData_t uc_rsp;
	u8 feature;
	int res;
	int i;

	WARN_ON(private_data != ipa3_ctx);
	IPA_ACTIVE_CLIENTS_INC_SIMPLE();
	IPADBG("uC rsp opcode=%u\n",
			ipa3_ctx->uc_ctx.uc_sram_mmio->responseOp);

	feature = EXTRACT_UC_FEATURE(ipa3_ctx->uc_ctx.uc_sram_mmio->responseOp);

	if (feature >= IPA_HW_FEATURE_MAX) {
		IPAERR("Invalid feature %u for event %u\n",
			feature, ipa3_ctx->uc_ctx.uc_sram_mmio->eventOp);
		IPA_ACTIVE_CLIENTS_DEC_SIMPLE();
		return;
	}

	/* Feature specific handling */
	if (ipa3_uc_hdlrs[feature].ipa3_uc_response_hdlr) {
		res = ipa3_uc_hdlrs[feature].ipa3_uc_response_hdlr(
			ipa3_ctx->uc_ctx.uc_sram_mmio,
			&ipa3_ctx->uc_ctx.uc_status);
		if (res == 0) {
			IPADBG("feature %d specific response handler\n",
				feature);
			complete_all(&ipa3_ctx->uc_ctx.uc_completion);
			IPA_ACTIVE_CLIENTS_DEC_SIMPLE();
			return;
		}
	}

	/* General handling */
	if (ipa3_ctx->uc_ctx.uc_sram_mmio->responseOp ==
			IPA_HW_2_CPU_RESPONSE_INIT_COMPLETED) {

		if (ipa3_ctx->uc_ctx.uc_loaded) {
			IPADBG("uC resp op INIT_COMPLETED is unexpected\n");
			IPA_ACTIVE_CLIENTS_DEC_SIMPLE();
			return;
		}

		mutex_lock(&uc_loaded_nb_lock);

		ipa3_ctx->uc_ctx.uc_loaded = true;

		(void) blocking_notifier_call_chain(&uc_loaded_notifier, true,
			ipa3_ctx);

		mutex_unlock(&uc_loaded_nb_lock);

		IPADBG("IPA uC loaded\n");
		/*
		 * The proxy vote is held until uC is loaded to ensure that
		 * IPA_HW_2_CPU_RESPONSE_INIT_COMPLETED is received.
		 */
		ipa3_proxy_clk_unvote();

		/*
		 * Sending the dma address command to uC for storing debug info
		 */
		if (ipa3_ctx->ipa_hw_type == IPA_HW_v4_2)
			queue_work(ipa3_ctx->power_mgmt_wq,
				&ipa_uc_send_dma_addr_work);

		for (i = 0; i < IPA_HW_NUM_FEATURES; i++) {
			if (ipa3_uc_hdlrs[i].ipa_uc_loaded_hdlr)
				ipa3_uc_hdlrs[i].ipa_uc_loaded_hdlr();
		}
	} else if (ipa3_ctx->uc_ctx.uc_sram_mmio->responseOp ==
		   IPA_HW_2_CPU_RESPONSE_CMD_COMPLETED) {
		uc_rsp.raw32b = ipa3_ctx->uc_ctx.uc_sram_mmio->responseParams;
		IPADBG("uC cmd response opcode=%u status=%u\n",
		       uc_rsp.params.originalCmdOp,
		       uc_rsp.params.status);
		if (uc_rsp.params.originalCmdOp ==
		    ipa3_ctx->uc_ctx.pending_cmd) {
			ipa3_ctx->uc_ctx.uc_status = uc_rsp.params.status;
			complete_all(&ipa3_ctx->uc_ctx.uc_completion);
		} else {
			IPAERR("Expected cmd=%u rcvd cmd=%u\n",
			       ipa3_ctx->uc_ctx.pending_cmd,
			       uc_rsp.params.originalCmdOp);
		}
	} else {
		IPAERR("Unsupported uC rsp opcode = %u\n",
		       ipa3_ctx->uc_ctx.uc_sram_mmio->responseOp);
	}
	IPA_ACTIVE_CLIENTS_DEC_SIMPLE();
}

void ipa3_uc_map_cntr_reg_notify(void)
{
	IPAWANDBG("complete the mapping of G_RD_CNTR register\n");
	IPA_ACTIVE_CLIENTS_INC_SPECIAL("QMI_IPA_UC");
	ipa3_uc_send_cmd(0,
		IPA_CPU_2_HW_CMD_DEBUG_GET_INFO,
		IPA_HW_2_CPU_RESPONSE_CMD_COMPLETED,
		false, 6*HZ);
	IPA_ACTIVE_CLIENTS_DEC_SPECIAL("QMI_IPA_UC");
}

static int ipa3_uc_send_cmd_64b_param(u32 cmd_lo, u32 cmd_hi, u32 opcode,
	u32 expected_status, bool polling_mode, unsigned long timeout_jiffies)
{
	int index;
	union IpaHwCpuCmdCompletedResponseData_t uc_rsp;
	int retries = 0;
	u32 uc_error_type;

send_cmd_lock:
	mutex_lock(&ipa3_ctx->uc_ctx.uc_lock);

	if (ipa3_uc_state_check()) {
		IPADBG("uC send command aborted\n");
		mutex_unlock(&ipa3_ctx->uc_ctx.uc_lock);
		return -EBADF;
	}
send_cmd:
	init_completion(&ipa3_ctx->uc_ctx.uc_completion);

	ipa3_ctx->uc_ctx.uc_sram_mmio->cmdParams = cmd_lo;
	ipa3_ctx->uc_ctx.uc_sram_mmio->cmdParams_hi = cmd_hi;
	ipa3_ctx->uc_ctx.uc_sram_mmio->cmdOp = opcode;
	ipa3_ctx->uc_ctx.pending_cmd = opcode;
	ipa3_ctx->uc_ctx.uc_sram_mmio->responseOp = 0;
	ipa3_ctx->uc_ctx.uc_sram_mmio->responseParams = 0;

	ipa3_ctx->uc_ctx.uc_status = 0;

	/* ensure write to shared memory is done before triggering uc */
	wmb();

	ipahal_write_reg_n(IPA_IRQ_EE_UC_n, 0, 0x1);

	if (polling_mode) {
		struct IpaHwSharedMemCommonMapping_t *uc_sram_ptr =
			ipa3_ctx->uc_ctx.uc_sram_mmio;
		for (index = 0; index < IPA_UC_POLL_MAX_RETRY; index++) {
			if (uc_sram_ptr->responseOp ==
			    IPA_HW_2_CPU_RESPONSE_CMD_COMPLETED) {
				uc_rsp.raw32b = uc_sram_ptr->responseParams;
				if (uc_rsp.params.originalCmdOp ==
					ipa3_ctx->uc_ctx.pending_cmd) {
					ipa3_ctx->uc_ctx.uc_status =
						uc_rsp.params.status;
					break;
				}
			}
			usleep_range(IPA_UC_POLL_SLEEP_USEC,
				IPA_UC_POLL_SLEEP_USEC);
		}

		if (index == IPA_UC_POLL_MAX_RETRY) {
			IPAERR("uC max polling retries reached\n");
			if (ipa3_ctx->uc_ctx.uc_failed) {
				uc_error_type = ipa3_ctx->uc_ctx.uc_error_type;
				IPAERR("uC reported on Error, errorType = %s\n",
					ipa_hw_error_str(uc_error_type));
			}
			mutex_unlock(&ipa3_ctx->uc_ctx.uc_lock);
			/* Unexpected UC hardware state */
			BUG();
		}
	} else {
		if (wait_for_completion_timeout(&ipa3_ctx->uc_ctx.uc_completion,
			timeout_jiffies) == 0) {
			IPAERR("uC timed out\n");
			if (ipa3_ctx->uc_ctx.uc_failed) {
				uc_error_type = ipa3_ctx->uc_ctx.uc_error_type;
				IPAERR("uC reported on Error, errorType = %s\n",
					ipa_hw_error_str(uc_error_type));
			}
			mutex_unlock(&ipa3_ctx->uc_ctx.uc_lock);
			/* Unexpected UC hardware state */
			BUG();
		}
	}

	if (ipa3_ctx->uc_ctx.uc_status != expected_status) {
		if (ipa3_ctx->uc_ctx.uc_status ==
		    IPA_HW_PROD_DISABLE_CMD_GSI_STOP_FAILURE ||
		    ipa3_ctx->uc_ctx.uc_status ==
		    IPA_HW_CONS_DISABLE_CMD_GSI_STOP_FAILURE ||
		    ipa3_ctx->uc_ctx.uc_status ==
		    IPA_HW_CONS_STOP_FAILURE ||
		    ipa3_ctx->uc_ctx.uc_status ==
		    IPA_HW_PROD_STOP_FAILURE) {
			retries++;
			if (retries == IPA_GSI_CHANNEL_STOP_MAX_RETRY) {
				IPAERR("Failed after %d tries\n", retries);
				mutex_unlock(&ipa3_ctx->uc_ctx.uc_lock);
				/* Unexpected UC hardware state */
				BUG();
			}
			mutex_unlock(&ipa3_ctx->uc_ctx.uc_lock);
			if (ipa3_ctx->uc_ctx.uc_status ==
			    IPA_HW_PROD_DISABLE_CMD_GSI_STOP_FAILURE)
				ipa3_inject_dma_task_for_gsi();
			/* sleep for short period to flush IPA */
			usleep_range(IPA_GSI_CHANNEL_STOP_SLEEP_MIN_USEC,
				IPA_GSI_CHANNEL_STOP_SLEEP_MAX_USEC);
			goto send_cmd_lock;
		}

		if (ipa3_ctx->uc_ctx.uc_status ==
			IPA_HW_GSI_CH_NOT_EMPTY_FAILURE) {
			retries++;
			if (retries >= IPA_GSI_CHANNEL_EMPTY_MAX_RETRY) {
				IPAERR("Failed after %d tries\n", retries);
				mutex_unlock(&ipa3_ctx->uc_ctx.uc_lock);
				return -EFAULT;
			}
			usleep_range(
			IPA_GSI_CHANNEL_EMPTY_SLEEP_MIN_USEC,
			IPA_GSI_CHANNEL_EMPTY_SLEEP_MAX_USEC);
			goto send_cmd;
		}

		IPAERR("Recevied status %u, Expected status %u\n",
			ipa3_ctx->uc_ctx.uc_status, expected_status);
		mutex_unlock(&ipa3_ctx->uc_ctx.uc_lock);
		return -EFAULT;
	}

	mutex_unlock(&ipa3_ctx->uc_ctx.uc_lock);

	IPADBG("uC cmd %u send succeeded\n", opcode);

	return 0;
}

/**
 * ipa3_uc_interface_init() - Initialize the interface with the uC
 *
 * Return value: 0 on success, negative value otherwise
 */
int ipa3_uc_interface_init(void)
{
	int result;
	unsigned long phys_addr;

	if (ipa3_ctx->uc_ctx.uc_inited) {
		IPADBG("uC interface already initialized\n");
		return 0;
	}

	mutex_init(&ipa3_ctx->uc_ctx.uc_lock);
	spin_lock_init(&ipa3_ctx->uc_ctx.uc_spinlock);

	phys_addr = ipa3_ctx->ipa_wrapper_base +
		ipa3_ctx->ctrl->ipa_reg_base_ofst +
		ipahal_get_reg_n_ofst(IPA_SW_AREA_RAM_DIRECT_ACCESS_n, 0);
	ipa3_ctx->uc_ctx.uc_sram_mmio = ioremap(phys_addr,
					       IPA_RAM_UC_SMEM_SIZE);
	if (!ipa3_ctx->uc_ctx.uc_sram_mmio) {
		IPAERR("Fail to ioremap IPA uC SRAM\n");
		result = -ENOMEM;
		goto remap_fail;
	}

	result = ipa3_add_interrupt_handler(IPA_UC_IRQ_0,
		ipa3_uc_event_handler, true,
		ipa3_ctx);
	if (result) {
		IPAERR("Fail to register for UC_IRQ0 rsp interrupt\n");
		result = -EFAULT;
		goto irq_fail0;
	}

	result = ipa3_add_interrupt_handler(IPA_UC_IRQ_1,
		ipa3_uc_response_hdlr, true,
		ipa3_ctx);
	if (result) {
		IPAERR("fail to register for UC_IRQ1 rsp interrupt\n");
		result = -EFAULT;
		goto irq_fail1;
	}

	ipa3_ctx->uc_ctx.uc_inited = true;

	IPADBG("IPA uC interface is initialized\n");
	return 0;

irq_fail1:
	ipa3_remove_interrupt_handler(IPA_UC_IRQ_0);
irq_fail0:
	iounmap(ipa3_ctx->uc_ctx.uc_sram_mmio);
remap_fail:
	return result;
}

/**
 * ipa3_uc_send_cmd() - Send a command to the uC
 *
 * Note1: This function sends command with 32bit parameter and do not
 *	use the higher 32bit of the command parameter (set to zero).
 *
 * Note2: In case the operation times out (No response from the uC) or
 *       polling maximal amount of retries has reached, the logic
 *       considers it as an invalid state of the uC/IPA, and
 *       issues a kernel panic.
 *
 * Returns: 0 on success.
 *          -EINVAL in case of invalid input.
 *          -EBADF in case uC interface is not initialized /
 *                 or the uC has failed previously.
 *          -EFAULT in case the received status doesn't match
 *                  the expected.
 */
int ipa3_uc_send_cmd(u32 cmd, u32 opcode, u32 expected_status,
		    bool polling_mode, unsigned long timeout_jiffies)
{
	return ipa3_uc_send_cmd_64b_param(cmd, 0, opcode,
		expected_status, polling_mode, timeout_jiffies);
}

/**
 * ipa3_uc_register_handlers() - Registers event, response and log event
 *                              handlers for a specific feature.Please note
 *                              that currently only one handler can be
 *                              registered per feature.
 *
 * Return value: None
 */
void ipa3_uc_register_handlers(enum ipa3_hw_features feature,
			      struct ipa3_uc_hdlrs *hdlrs)
{
	if (0 > feature || IPA_HW_FEATURE_MAX <= feature) {
		IPAERR("Feature %u is invalid, not registering hdlrs\n",
		       feature);
		return;
	}

	mutex_lock(&ipa3_ctx->uc_ctx.uc_lock);
	ipa3_uc_hdlrs[feature] = *hdlrs;
	mutex_unlock(&ipa3_ctx->uc_ctx.uc_lock);

	IPADBG("uC handlers registered for feature %u\n", feature);
}

int ipa3_uc_is_gsi_channel_empty(enum ipa_client_type ipa_client)
{
	const struct ipa_gsi_ep_config *gsi_ep_info;
	union IpaHwChkChEmptyCmdData_t cmd;
	int ret;

	gsi_ep_info = ipa3_get_gsi_ep_info(ipa_client);
	if (!gsi_ep_info) {
		IPAERR("Failed getting GSI EP info for client=%d\n",
		       ipa_client);
		return 0;
	}

	if (ipa3_uc_state_check()) {
		IPADBG("uC cannot be used to validate ch emptiness clnt=%d\n"
			, ipa_client);
		return 0;
	}

	cmd.params.ee_n = gsi_ep_info->ee;
	cmd.params.vir_ch_id = gsi_ep_info->ipa_gsi_chan_num;

	IPADBG("uC emptiness check for IPA GSI Channel %d\n",
	       gsi_ep_info->ipa_gsi_chan_num);

	ret = ipa3_uc_send_cmd(cmd.raw32b, IPA_CPU_2_HW_CMD_GSI_CH_EMPTY, 0,
			      false, IPA_TIMEOUT(10));

	return ret;
}


/**
 * ipa3_uc_notify_clk_state() - notify to uC of clock enable / disable
 * @enabled: true if clock are enabled
 *
 * The function uses the uC interface in order to notify uC before IPA clocks
 * are disabled to make sure uC is not in the middle of operation.
 * Also after clocks are enabled ned to notify uC to start processing.
 *
 * Returns: 0 on success, negative on failure
 */
int ipa3_uc_notify_clk_state(bool enabled)
{
	u32 opcode;

	if (ipa3_ctx->ipa_hw_type > IPA_HW_v4_0) {
		IPADBG_LOW("not supported past IPA v4.0\n");
		return 0;
	}

	/*
	 * If the uC interface has not been initialized yet,
	 * don't notify the uC on the enable/disable
	 */
	if (ipa3_uc_state_check()) {
		IPADBG("uC interface will not notify the UC on clock state\n");
		return 0;
	}

	IPADBG("uC clock %s notification\n", (enabled) ? "UNGATE" : "GATE");

	opcode = (enabled) ? IPA_CPU_2_HW_CMD_CLK_UNGATE :
			     IPA_CPU_2_HW_CMD_CLK_GATE;

	return ipa3_uc_send_cmd(0, opcode, 0, true, 0);
}

/**
 * ipa3_uc_update_hw_flags() - send uC the HW flags to be used
 * @flags: This field is expected to be used as bitmask for enum ipa3_hw_flags
 *
 * Returns: 0 on success, negative on failure
 */
int ipa3_uc_update_hw_flags(u32 flags)
{
	union IpaHwUpdateFlagsCmdData_t cmd;

	memset(&cmd, 0, sizeof(cmd));
	cmd.params.newFlags = flags;
	return ipa3_uc_send_cmd(cmd.raw32b, IPA_CPU_2_HW_CMD_UPDATE_FLAGS, 0,
		false, IPA_TIMEOUT(10));
}

/**
 * ipa3_uc_memcpy() - Perform a memcpy action using IPA uC
 * @dest: physical address to store the copied data.
 * @src: physical address of the source data to copy.
 * @len: number of bytes to copy.
 *
 * Returns: 0 on success, negative on failure
 */
int ipa3_uc_memcpy(phys_addr_t dest, phys_addr_t src, int len)
{
	int res;
	struct ipa_mem_buffer mem;
	struct IpaHwMemCopyData_t *cmd;

	IPADBG("dest 0x%pa src 0x%pa len %d\n", &dest, &src, len);
	mem.size = sizeof(cmd);
	mem.base = dma_alloc_coherent(ipa3_ctx->pdev, mem.size, &mem.phys_base,
		GFP_KERNEL);
	if (!mem.base) {
		IPAERR("fail to alloc DMA buff of size %d\n", mem.size);
		return -ENOMEM;
	}
	cmd = (struct IpaHwMemCopyData_t *)mem.base;
	memset(cmd, 0, sizeof(*cmd));
	cmd->destination_addr = dest;
	cmd->dest_buffer_size = len;
	cmd->source_addr = src;
	cmd->source_buffer_size = len;
	res = ipa3_uc_send_cmd((u32)mem.phys_base, IPA_CPU_2_HW_CMD_MEMCPY, 0,
		true, IPA_TIMEOUT(10));
	if (res) {
		IPAERR("ipa3_uc_send_cmd failed %d\n", res);
		goto free_coherent;
	}

	res = 0;
free_coherent:
	dma_free_coherent(ipa3_ctx->pdev, mem.size, mem.base, mem.phys_base);
	return res;
}

int ipa3_uc_send_remote_ipa_info(u32 remote_addr, uint32_t mbox_n)
{
	int res;
	struct ipa_mem_buffer cmd;
	struct IpaHwDbAddrInfo_t *uc_info;

	cmd.size = sizeof(*uc_info);
	cmd.base = dma_alloc_coherent(ipa3_ctx->uc_pdev, cmd.size,
		&cmd.phys_base, GFP_KERNEL);
	if (cmd.base == NULL)
		return -ENOMEM;

	uc_info = (struct IpaHwDbAddrInfo_t *) cmd.base;
	uc_info->remoteIPAAddr = remote_addr;
	uc_info->mboxN = mbox_n;

	res = ipa3_uc_send_cmd((u32)(cmd.phys_base),
		IPA_CPU_2_HW_CMD_REMOTE_IPA_INFO, 0,
		false, 10 * HZ);

	if (res) {
		IPAERR("fail to map 0x%x to mbox %d\n",
			uc_info->remoteIPAAddr,
			uc_info->mboxN);
		goto free_coherent;
	}

	res = 0;
free_coherent:
	dma_free_coherent(ipa3_ctx->uc_pdev, cmd.size, cmd.base, cmd.phys_base);
	return res;
}

static void ipa_uc_send_dma_addr_on_wq(struct work_struct *work)
{
	int res;
	struct ipa_mem_buffer *cmd;
	u32 addr_lo, addr_hi, *value;

	IPA_ACTIVE_CLIENTS_INC_SIMPLE();
	cmd = &ipa3_ctx->uc_dma_addr;
	cmd->size = sizeof(int);
	cmd->base = dma_alloc_coherent(ipa3_ctx->uc_pdev, cmd->size,
			&cmd->phys_base, GFP_KERNEL);
	if (cmd->base == NULL) {
		IPAERR("Memory allocation failed\n");
		IPA_ACTIVE_CLIENTS_DEC_SIMPLE();
		return;
	}

	memset(cmd->base, 0, cmd->size);
	value = (u32 *)cmd->base;
	/* By default expected to write 0xDEADDEAD in this dma addr*/
	*value = 0xDEADDEAD;
	addr_lo = lower_32_bits(cmd->phys_base);
	addr_hi = upper_32_bits(cmd->phys_base);
	res = ipa3_uc_send_cmd_64b_param(addr_lo, addr_hi,
			IPA_CPU_2_HW_CMD_DMA_ADDR_INFO,
			0, false, 10 * HZ);

	if (res) {
		IPAERR("Failed to send command\n");
		goto free_coherent;
	}

	IPA_ACTIVE_CLIENTS_DEC_SIMPLE();
	return;
free_coherent:
	dma_free_coherent(ipa3_ctx->uc_pdev, cmd->size, cmd->base,
							cmd->phys_base);
	cmd->size = 0;
	cmd->base = NULL;
	cmd->phys_base = 0;
	IPA_ACTIVE_CLIENTS_DEC_SIMPLE();
}
