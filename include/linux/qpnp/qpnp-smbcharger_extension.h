/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */
/*
 * Copyright (C) 2015 Sony Mobile Communications Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation.
 */

#ifndef __QPNP_SMBCHARGER_EXTENSION_H
#define __QPNP_SMBCHARGER_EXTENSION_H

#ifdef CONFIG_QPNP_SMBCHARGER_EXTENSION

#define ADPT_ALLOWANCE_MASK		SMB_MASK(2, 0)
#define USBIN_ADPT_ALLOW_5V		0x0
#define USBIN_ADPT_ALLOW_5V_TO_9V	0x2
#define REMOVE_DELAY_MS			2000

#define USBIN_CHGR_CFG_MASK		SMB_MASK(2, 0)
#define USBIN_ADAPTER_5V		0x0

#define VFLOAT_CMP_CFG_REG		0xF5
#define VFLOAT_CMP_MASK			SMB_MASK(5, 0)
#define VFLOAT_CMP_SUB_8_VAL		0x08

#define VFLOAT_CCMP_CFG_REG		0xFA
#define VFLOAT_CCMP_SL_FV_COMP_MASK	SMB_MASK(3, 0)
#define VFLOAT_CCMP_HOT_SL_FV_COMP	0x08

#define AICL_INIT_MASK			BIT(7)
#define AICL_ADC_MASK			BIT(6)
#define AICL_ADC_ON			BIT(6)

#define PCC_CFG				0xF1
#define PCC_550MA_VAL			0x04
#define PCC_MASK			SMB_MASK(2, 0)

/*
 * somc_chg_apsd_rerun_request() - Request asynchronous APSD rerun
 *		The api is to perform APSD rerun.
 * @return:	0: queued successfully.
 */
int somc_chg_apsd_rerun_request(void);
#else
int somc_chg_apsd_rerun_request(void)
{
	return -ENODEV;
}
#endif

#endif
