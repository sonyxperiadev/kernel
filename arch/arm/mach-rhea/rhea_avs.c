/*******************************************************************************
 * Copyright 2010,2011 Broadcom Corporation.  All rights reserved.
 *
 *	@file	arch/arm/mach-rhea/rhea_avs.c
 *
 * Unless you and Broadcom execute a separate written software license agreement
 * governing use of this software, this software is licensed to you under the
 * terms of the GNU General Public License version 2, available at
 * http://www.gnu.org/copyleft/gpl.html (the "GPL").
 *
 * Notwithstanding the above, under no circumstances may you combine this
 * software in any way with any other Broadcom software provided under a license
 * other than the GPL, without Broadcom's express prior written consent.
 *****************************************************************************/

#include <linux/kernel.h>
#include <linux/err.h>
#ifdef CONFIG_KONA_OTP
#include <plat/bcm_otp.h>
#endif
#include <mach/rhea_avs.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/module.h>
#include <mach/cpu.h>

/*#define KONA_AVS_DEBUG*/

/*Should we move this to avs_param ?? */
#define VM_VAL_MASK	(0xFF)
#define VM0_VAL_SHIFT	(8)
#define VM1_VAL_SHIFT	(16)
#define VM2_VAL_SHIFT	(24)
#define VM3_VAL_SHIFT	(0)

#define AVS_ATE_MONTH_MASK	(0xF)
#define AVS_ATE_YEAR_MASK	(0xF0)
#define AVS_ATE_VAL_MASK	(0xF00)
#define AVS_ATE_CRC_MASK	(0xF000)

#define AVS_ATE_MONTH_SHIFT	(0)
#define AVS_ATE_YEAR_SHIFT	(4)
#define AVS_ATE_VAL_SHIFT	(8)
#define AVS_ATE_CRC_SHIFT	(12)

#define AVS_ATE_YEAR_2012	(2)
#define AVS_ATE_MONTH_JUNE	(6)

#define avs_dbg(level, args...) \
	do { \
		if (debug_mask & level) { \
			pr_info(args); \
		} \
	} while (0)

enum {
	AVS_LOG_ERR  = 1 << 0,
	AVS_LOG_WARN = 1 << 1,
	AVS_LOG_INIT = 1 << 2,
	AVS_LOG_FLOW = 1 << 3,
	AVS_LOG_INFO = 1 << 4,
};

struct avs_info {
	u32 vm0_val;
	u32 vm1_val;
	u32 vm2_val;
	u32 vm3_val;
	u32 vm_silicon_type;

	u32 silicon_type;

	u32 ate_silicon_type;
	u32 freq;
	u32 avs_ate_val;
	u32 ate_crc;
	u32 year;
	u32 month;
	struct rhea_avs_pdata *pdata;
};

struct avs_info avs_info = {.silicon_type = SILICON_TYPE_SLOW, };
static int debug_mask = AVS_LOG_ERR | AVS_LOG_WARN | AVS_LOG_INIT;

module_param_named(silicon_type, avs_info.silicon_type, int, S_IRUGO);
module_param_named(avs_vm0_val, avs_info.vm0_val, int, S_IRUGO | S_IWUSR
			| S_IWGRP);
module_param_named(avs_vm1_val, avs_info.vm1_val, int, S_IRUGO | S_IWUSR
			| S_IWGRP);
module_param_named(avs_vm2_val, avs_info.vm2_val, int, S_IRUGO | S_IWUSR
			| S_IWGRP);
module_param_named(avs_vm3_val, avs_info.vm3_val, int, S_IRUGO | S_IWUSR
			| S_IWGRP);
module_param_named(avs_ate_val, avs_info.avs_ate_val, int, S_IRUGO | S_IWUSR
			| S_IWGRP);
module_param_named(year, avs_info.year, int, S_IRUGO | S_IWUSR
			| S_IWGRP);
module_param_named(month, avs_info.month, int, S_IRUGO | S_IWUSR
			| S_IWGRP);
module_param_named(ate_crc, avs_info.ate_crc, int, S_IRUGO | S_IWUSR
			| S_IWGRP);

module_param_named(debug_mask, debug_mask, int, S_IRUGO | S_IWUSR |
		S_IWGRP);

struct trigger_avs {
	int dummy;
};

#define __param_check_trigger_avs(name, p, type) \
	static inline struct type *__check_##name(void) { return (p); }

#define param_check_trigger_avs(name, p) \
	__param_check_trigger_avs(name, p, trigger_avs)

static int param_set_trigger_avs(const char *val,
			const struct kernel_param *kp);
static struct kernel_param_ops param_ops_trigger_avs = {
	.set = param_set_trigger_avs,
};

static struct trigger_avs trigger_avs;
module_param_named(trigger_avs, trigger_avs, trigger_avs,
				S_IWUSR | S_IWGRP);


struct vm_val {
	u32 val0;
	u32 val1;
};

struct ate_val {
	u32 val0;
	u32 reserved;
};

#if defined(KONA_AVS_DEBUG)

static int otp_read(int row, struct vm_val *vm_val)
{
	avs_dbg(AVS_LOG_INFO, "%s:row = %d\n", __func__, row);
	if (row < 0)
		return -EINVAL;
	vm_val->val0 =
	    (146 << VM0_VAL_SHIFT) | (180 << VM1_VAL_SHIFT) |
			(95 << VM2_VAL_SHIFT);
	vm_val->val1 = 170;

	return 0;
}
#endif

u32 rhea_avs_get_solicon_type(void)
{
	BUG_ON(avs_info.pdata == NULL);
	return avs_info.silicon_type;
}
EXPORT_SYMBOL(rhea_avs_get_solicon_type);

/**
 * converts interger to string radix 2 (binary
 * number string)
 */
static void int2bin(unsigned int num, char *str)
{
	int i = 0;
	int j = 0;
	char temp[33];

	while (num != 0) {
		temp[i] = num % 2 ? '1' : '0';
		num /= 2;
		i++;
	}
	str[i] = 0;
	temp[i] = 0;

	/* reverse the string */
	while (i) {
		i--;
		str[j] = temp[i];
		j++;
	}
}

/**
 * 4-bit Linear feeback shift register implementation
 * based on primitive polynomial x^4 + x + 1
 *
 * bitstring should be null terminate
 * bit stream (exp: "10111111")
 */
static void cal_crc(const char *bitstring, char *crc_res)
{
	char crc[4];
	int i;
	int len;
	char do_invert;

	memset(crc, 0x0, sizeof(crc));
	len = strlen(bitstring);

	for (i = 0; i < len; ++i) {
		do_invert = ('1' == bitstring[i]) ^ crc[3];
		crc[3] = crc[2];
		crc[2] = crc[1];
		crc[1] = crc[0] ^ do_invert;
		crc[0] = do_invert;
	}

	for (i = 0; i < 4; ++i)
		crc_res[3-i] = crc[i] ? '1' : '0';
	crc_res[4] = 0; /* Null Terminated */
}

static bool avs_ate_programmed(struct avs_info *avs_inf_ptr)
{

	if ((avs_inf_ptr->avs_ate_val == 0) &&
			(avs_inf_ptr->ate_crc == 0) &&
			(avs_inf_ptr->year == 0) &&
			(avs_inf_ptr->month == 0)) {
		avs_dbg(AVS_LOG_ERR, "%s:AVS_ATE_BIN and CRC fields are 0\n",
				__func__);
		return false;
	}
	return true;
}

static int avs_read_vm_otp(struct avs_info *avs_inf_ptr)
{
	struct vm_val vm_val;
	int ret = -EINVAL;

	if (avs_inf_ptr->pdata->flags & AVS_TYPE_BOOT) {
		avs_dbg(AVS_LOG_ERR,
				"%s:AVS_TYPE_BOOT not supported !!!\n",
				__func__);
		return -EINVAL;
	}

	if (avs_inf_ptr->pdata->flags & AVS_READ_FROM_MEM) {
		void __iomem *mem_ptr;
		avs_dbg(AVS_LOG_INIT, "%s: AVS_READ_FROM_MEM => mem adr = %x\n",
				__func__,
				avs_inf_ptr->pdata->avs_mon_addr);
		BUG_ON(avs_inf_ptr->pdata->avs_mon_addr == 0);
		mem_ptr =
		    ioremap_nocache(avs_inf_ptr->pdata->avs_mon_addr,
				    sizeof(struct vm_val));
		avs_dbg(AVS_LOG_INIT,
				"%s: AVS_READ_FROM_MEM => virtual addr = %p\n",
				__func__, mem_ptr);
		if (mem_ptr) {
			memcpy(&vm_val, mem_ptr, sizeof(struct vm_val));
			iounmap(mem_ptr);
			ret = 0;
		} else {
			ret = -ENOMEM;
			BUG_ON(mem_ptr == NULL);
		}
	} else {
		avs_dbg(AVS_LOG_INIT,
				"%s: AVS_READ_FROM_OTP => row = %x\n",
				__func__,
				avs_inf_ptr->pdata->avs_mon_addr);
#if defined(KONA_AVS_DEBUG) || defined(CONFIG_KONA_OTP)
		ret = otp_read(avs_inf_ptr->pdata->avs_mon_addr, &vm_val);
#endif
	}

	if (!ret) {

		avs_dbg(AVS_LOG_INIT, "%s:opt:val0 = %x val1 = %x\n", __func__,
				vm_val.val0,
				vm_val.val1);
		avs_inf_ptr->vm0_val =
		    (vm_val.val0 >> VM0_VAL_SHIFT) & VM_VAL_MASK;
		avs_inf_ptr->vm1_val =
		    (vm_val.val0 >> VM1_VAL_SHIFT) & VM_VAL_MASK;
		avs_inf_ptr->vm2_val =
		    (vm_val.val0 >> VM2_VAL_SHIFT) & VM_VAL_MASK;
		avs_inf_ptr->vm3_val =
		    (vm_val.val1 >> VM3_VAL_SHIFT) & VM_VAL_MASK;

		avs_dbg(AVS_LOG_INIT, "%s:vm0_val = %d"
				"vm1_val= %d vm2_val = %d vm3_val = %d\n",
				__func__,
				avs_inf_ptr->vm0_val,
				avs_inf_ptr->vm1_val,
				avs_inf_ptr->vm2_val,
				avs_inf_ptr->vm3_val);
	}
	return ret;
}

static int avs_read_ate_otp(struct avs_info *avs_inf_ptr)
{
	struct ate_val ate_val;
	int ret = -EINVAL;
	void __iomem *mem_ptr;

	avs_dbg(AVS_LOG_FLOW, "%s\n", __func__);

	if (avs_inf_ptr->pdata->flags & AVS_TYPE_BOOT) {
		avs_dbg(AVS_LOG_ERR, "%s:AVS_TYPE_BOOT not supported !!!\n",
				__func__);
		return -EINVAL;
	}

	if (avs_inf_ptr->pdata->flags & AVS_READ_FROM_MEM) {
		BUG_ON(avs_inf_ptr->pdata->avs_ate_addr == 0);
		avs_dbg(AVS_LOG_INIT,
				"%s: AVS_READ_FROM_MEM => mem adr = %x\n",
				__func__,
				avs_inf_ptr->pdata->avs_ate_addr);
		mem_ptr = ioremap_nocache(avs_inf_ptr->pdata->avs_ate_addr,
				sizeof(ate_val));
		avs_dbg(AVS_LOG_INIT,
				"%s:AVS_READ_FROM_MEM => virtual addr = %p\n",
				__func__, mem_ptr);
		if (mem_ptr) {
			memcpy(&ate_val, mem_ptr, sizeof(ate_val));
			iounmap(mem_ptr);
			ret = 0;
		} else {
			ret = -ENOMEM;
			BUG_ON(mem_ptr == NULL);
		}
	}
	if (!ret) {
		avs_dbg(AVS_LOG_INIT, "%s:ATE val0 = %x\n", __func__,
				ate_val.val0);
		avs_inf_ptr->year = ((ate_val.val0 & AVS_ATE_YEAR_MASK) >>
				AVS_ATE_YEAR_SHIFT);
		avs_inf_ptr->month = ((ate_val.val0 & AVS_ATE_MONTH_MASK) >>
				AVS_ATE_MONTH_SHIFT);
		/**
		 * if year and month field is 0, we will assume June 2012
		 * This is just for debug print purpose only
		 */
		avs_dbg(AVS_LOG_INFO, "AVS Year & Month of Manufacturing:"
				"%d %d\n",
				((avs_inf_ptr->year == 0) ? 2012 :
				 (2010 + avs_inf_ptr->year)),
				((avs_inf_ptr->month == 0) ? 6 :
				 avs_inf_ptr->month));

		avs_inf_ptr->avs_ate_val = ((ate_val.val0 & AVS_ATE_VAL_MASK) >>
				AVS_ATE_VAL_SHIFT);
		avs_inf_ptr->ate_crc = ((ate_val.val0 & AVS_ATE_CRC_MASK) >>
				AVS_ATE_CRC_SHIFT);
		avs_dbg(AVS_LOG_INIT, "ATE_AVS_BIN[3:0]=0x%x CRC[3:0]=0x%x\n",
				avs_inf_ptr->avs_ate_val,
				avs_inf_ptr->ate_crc);
	}
	return ret;
}

static int avs_ate_get_silicon_type(struct avs_info *avs_inf_ptr)
{
	struct rhea_avs_pdata *pdata = avs_inf_ptr->pdata;
	char str[33];
	char pack[60];
	char crc[5];
	u32 temp1;
	u32 temp2;
	long crc_val;
	int err = -EINVAL;

	memset(pack, 0, sizeof(pack));

	if (!avs_ate_programmed(avs_inf_ptr))
		return -EINVAL;

	/**
	 * pack {ATE_AVS_BIN[3:0], Year[3:0], Month[3:0], VM3[7:0], VM2[7:0],
	 * VM1[7:0],VM0[7:0]} and calculate CRC
	 */
	temp1 = ((avs_inf_ptr->avs_ate_val << 8) | (avs_inf_ptr->year << 4) |
			(avs_inf_ptr->month));
	avs_dbg(AVS_LOG_INFO, "pack [ATE:YEAR:MONTH] = 0x%x\n", temp1);
	int2bin(temp1, str);
	strcat(pack, str);
	temp2 = ((avs_inf_ptr->vm3_val << 24) |
			(avs_inf_ptr->vm2_val << 16)|
			(avs_inf_ptr->vm1_val << 8) |
			(avs_inf_ptr->vm0_val));
	avs_dbg(AVS_LOG_INFO, "pack [VM3:2:1:0] = 0x%x\n", temp2);
	int2bin(temp2, str);
	strcat(pack, str);
	avs_dbg(AVS_LOG_INFO, "packed [ATE:VM] string for CRC : %s\n", pack);

	cal_crc(pack, crc);
	err = kstrtol(crc, 2, &crc_val);
	avs_dbg(AVS_LOG_INIT, "Calcualted ATE CRC value = %x\n", (u32)crc_val);

	/**
	 * if CRC fails, we will assume default silicon type (Slow
	 * silicon). Frequency will be determined by the PLL configuration
	 * in fail case
	 */

	if (!err && avs_inf_ptr->ate_crc != crc_val) {
		avs_dbg(AVS_LOG_ERR, "ATE CRC Failed\n");
		avs_inf_ptr->ate_silicon_type = pdata->ate_default_silicon_type;
		avs_inf_ptr->freq = -1;
	} else {
		avs_inf_ptr->ate_silicon_type =
			((pdata->ate_lut[avs_inf_ptr->avs_ate_val].silicon_type
			 == ATE_FIELD_RESERVED) ? SILICON_TYPE_SLOW :
			 pdata->ate_lut[avs_inf_ptr->avs_ate_val].silicon_type);

		avs_inf_ptr->freq =
			((pdata->ate_lut[avs_inf_ptr->avs_ate_val].freq
			  == ATE_FIELD_RESERVED) ? -1 :
			 pdata->ate_lut[avs_inf_ptr->avs_ate_val].freq);
	}
	avs_dbg(AVS_LOG_INIT, "%s: return silicon type %d freq %d\n",
			__func__,
			avs_inf_ptr->ate_silicon_type,
			avs_inf_ptr->freq);
	return 0;
}

static u32 avs_vm_find_silicon_type(struct avs_info *avs_inf_ptr, u32 vm_index,
				   u32 *lut)
{
	struct rhea_avs_pdata *pdata = avs_inf_ptr->pdata;
	int silicon_type_idx = -1;
	int i;

	if (pdata->silicon_type_notify && lut) {
		for (i = 0; i < VM_BIN_LUT_SIZE - 1; i++) {
			if (vm_index >= lut[i] && vm_index < lut[i + 1]) {
				silicon_type_idx = i;
				break;
			}
		}
	}
	if (silicon_type_idx == -1)
		return SILICON_TYPE_SLOW;

	return pdata->silicon_type_lut[silicon_type_idx];
}

static u32 avs_vm_get_silicon_type(struct avs_info *avs_inf_ptr,
				   int chip_rev_id)
{
	struct rhea_avs_pdata *pdata = avs_inf_ptr->pdata;
	u32 (*lut)[VM_BIN_LUT_SIZE];
	u32 type_vm0, type_vm1, type_vm2, type_vm3;
	u32 silicon_type;

	if (chip_rev_id == RHEA_CHIP_REV_B0)
		lut = pdata->vm_bin_B0_lut;
	else if (chip_rev_id == RHEA_CHIP_REV_B1)
		lut = pdata->vm_bin_B1_lut;
	else {
		WARN_ON(1);
		return SILICON_TYPE_SLOW;
	}

	type_vm0 = avs_vm_find_silicon_type(avs_inf_ptr,
			avs_inf_ptr->vm0_val,
			&lut[0][0]);
	type_vm1 = avs_vm_find_silicon_type(avs_inf_ptr,
			avs_inf_ptr->vm1_val,
			&lut[1][0]);
	type_vm2 = avs_vm_find_silicon_type(avs_inf_ptr,
			avs_inf_ptr->vm2_val,
			&lut[2][0]);
	type_vm3 = avs_vm_find_silicon_type(avs_inf_ptr,
			avs_inf_ptr->vm3_val,
			&lut[3][0]);
	avs_dbg(AVS_LOG_INIT, "%s: silicon types vm[0-3] : %d %d %d %d\n",
			__func__, type_vm0, type_vm1, type_vm2, type_vm3);

	silicon_type = min(min(type_vm0, type_vm1), min(type_vm2, type_vm3));
	avs_dbg(AVS_LOG_INIT, "%s: return silicon type %d\n",
			__func__, silicon_type);

	return silicon_type;
}

static int avs_find_silicon_type(void)
{
	int ate_enabled = 0;
	int ret = 0;

	if (!avs_info.pdata)
		return  -EPERM;

	ate_enabled = avs_info.pdata->flags & AVS_ATE_FEATURE_ENABLE;

	if (ate_enabled)
		ret = avs_ate_get_silicon_type(&avs_info);

	if (ate_enabled && ret) {
		/**
		 * case 1: All ATE fields are 0 (including YEAR and month)
		 * In this case use B0 table to determine the silicon type
		 * And if its FF demote it to TT and TT to SS
		 */
		avs_info.silicon_type = avs_vm_get_silicon_type(&avs_info,
				RHEA_CHIP_REV_B0);
		if (avs_info.silicon_type == SILICON_TYPE_FAST)
			avs_info.silicon_type = SILICON_TYPE_TYPICAL;
		else
			avs_info.silicon_type = SILICON_TYPE_SLOW;
		avs_info.freq = -1;
	} else if (ate_enabled && !ret && (avs_info.freq != -1)) {
		/**
		 * Case 2: ATE fields are programmed and CRC passed.
		 * In this case if {year,month} <= {2012, June}, use
		 * B0 VM index table and ATE lut to determine silicon
		 * type else use B1 VM index table
		 */
		if ((avs_info.year <= AVS_ATE_YEAR_2012) &&
				(avs_info.month <= AVS_ATE_MONTH_JUNE)) {
			avs_info.vm_silicon_type = avs_vm_get_silicon_type(
					&avs_info,
					RHEA_CHIP_REV_B0);
		} else {
			avs_info.vm_silicon_type = avs_vm_get_silicon_type(
					&avs_info,
					RHEA_CHIP_REV_B1);
		}
		avs_info.silicon_type = min(avs_info.ate_silicon_type,
					avs_info.vm_silicon_type);
	} else {
		/**
		 * case 3: ATE fields are progammed but CRC failed
		 * In this case avs_ate_get_silicon_type() would have
		 * set silicon type to worst case SILICON_TYPE_SLOW
		 * so avs driver will just report slow silicon
		 * for this case
		 */
	}

	if (avs_info.pdata->silicon_type_notify)
		avs_info.pdata->silicon_type_notify(avs_info.silicon_type,
				avs_info.freq);

	avs_dbg(AVS_LOG_INIT,
			"%s: silicon type vm: %d  ate: %d"
			"silicon type: %d\n",
			__func__, avs_info.vm_silicon_type,
			avs_info.ate_silicon_type,
			avs_info.silicon_type);

	return 0;
}

static int param_set_trigger_avs(const char *val, const struct kernel_param *kp)
{
	int trig;
	int ret = -1;

	avs_dbg(AVS_LOG_FLOW, "%s\n", __func__);
	if (!val)
		return -EINVAL;
	if (!avs_info.pdata) {
		avs_dbg(AVS_LOG_ERR,
				"%s : invalid paltform data !!\n", __func__);
		return  -EPERM;
	}
	ret = sscanf(val, "%d", &trig);
	avs_dbg(AVS_LOG_INFO, "%s, trig:%d\n", __func__, trig);
	if (trig == 1)
		avs_find_silicon_type();

	return 0;
}

static int rhea_avs_drv_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct rhea_avs_pdata *pdata = pdev->dev.platform_data;

	avs_dbg(AVS_LOG_INIT, "%s\n", __func__);

	if (!pdata) {
		avs_dbg(AVS_LOG_ERR,
				"%s : invalid paltform data !!\n", __func__);
		ret = -EPERM;
		goto error;
	}

	avs_info.pdata = pdata;

	BUG_ON((pdata->flags & AVS_TYPE_OPEN)
	       && (pdata->flags & AVS_TYPE_BOOT));
	BUG_ON((pdata->flags & AVS_READ_FROM_OTP)
	       && (pdata->flags & AVS_READ_FROM_MEM));

	ret = avs_read_vm_otp(&avs_info);
	if (ret)
		goto error;
	ret = avs_read_ate_otp(&avs_info);
	if (ret)
		goto error;

	avs_find_silicon_type();

error:
	return ret;
}

static int __devexit rhea_avs_drv_remove(struct platform_device *pdev)
{
	return 0;
}

static struct platform_driver rhea_avs_driver = {
	.probe = rhea_avs_drv_probe,
	.remove = __devexit_p(rhea_avs_drv_remove),
	.driver = {.name = "rhea-avs",},
};

static int __init rhea_avs_drv_init(void)
{
	return platform_driver_register(&rhea_avs_driver);
}

subsys_initcall_sync(rhea_avs_drv_init);

static void __exit rhea_avs_drv_exit(void)
{
	platform_driver_unregister(&rhea_avs_driver);
}

module_exit(rhea_avs_drv_exit);

MODULE_ALIAS("platform:rhea_avs_drv");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("AVS driver for BRCM Kona based Chipsets");
