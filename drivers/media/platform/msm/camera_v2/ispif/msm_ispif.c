/* Copyright (c) 2013-2015, The Linux Foundation. All rights reserved.
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

#include <linux/delay.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/jiffies.h>
#include <linux/of.h>
#include <linux/videodev2.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/iopoll.h>
#include <linux/compat.h>
#include <media/msmb_isp.h>

#include "msm_ispif.h"
#include "msm.h"
#include "msm_sd.h"
#include "msm_camera_io_util.h"

#ifdef CONFIG_MSM_ISPIF_V1
#include "msm_ispif_hwreg_v1.h"
#else
#include "msm_ispif_hwreg_v2.h"
#endif

#define V4L2_IDENT_ISPIF                      50001
#define MSM_ISPIF_DRV_NAME                    "msm_ispif"

#define ISPIF_INTF_CMD_DISABLE_FRAME_BOUNDARY 0x00
#define ISPIF_INTF_CMD_ENABLE_FRAME_BOUNDARY  0x01
#define ISPIF_INTF_CMD_DISABLE_IMMEDIATELY    0x02

#define ISPIF_TIMEOUT_SLEEP_US                1000
#define ISPIF_TIMEOUT_ALL_US               1000000
#define ISPIF_SOF_DEBUG_COUNT                    5

#undef CDBG
#ifdef CONFIG_MSMB_CAMERA_DEBUG
#define CDBG(fmt, args...) pr_debug(fmt, ##args)
#else
#define CDBG(fmt, args...) do { } while (0)
#endif

int msm_ispif_get_clk_info(struct ispif_device *ispif_dev,
	struct platform_device *pdev,
	struct msm_cam_clk_info *ahb_clk_info,
	struct msm_cam_clk_info *clk_info);

static void msm_ispif_io_dump_reg(struct ispif_device *ispif)
{
	if (!ispif->enb_dump_reg)
		return;
	msm_camera_io_dump(ispif->base, 0x250);
}


static inline int msm_ispif_is_intf_valid(uint32_t csid_version,
	uint8_t intf_type)
{
	return ((csid_version <= CSID_VERSION_V22 && intf_type != VFE0) ||
		(intf_type >= VFE_MAX)) ? false : true;
}

static struct msm_cam_clk_info ispif_8626_reset_clk_info[] = {
	{"ispif_ahb_clk", NO_SET_RATE},
	{"camss_top_ahb_clk", NO_SET_RATE},
	{"csi0_ahb_clk", NO_SET_RATE},
	{"csi0_src_clk", NO_SET_RATE},
	{"csi0_phy_clk", NO_SET_RATE},
	{"csi0_clk", NO_SET_RATE},
	{"csi0_pix_clk", NO_SET_RATE},
	{"csi0_rdi_clk", NO_SET_RATE},
	{"csi1_ahb_clk", NO_SET_RATE},
	{"csi1_src_clk", NO_SET_RATE},
	{"csi1_phy_clk", NO_SET_RATE},
	{"csi1_clk", NO_SET_RATE},
	{"csi1_pix_clk", NO_SET_RATE},
	{"csi1_rdi_clk", NO_SET_RATE},
	{"camss_vfe_vfe_clk", NO_SET_RATE},
	{"camss_csi_vfe_clk", NO_SET_RATE},
};

static struct msm_cam_clk_info ispif_ahb_clk_info[ISPIF_CLK_INFO_MAX];
static struct msm_cam_clk_info ispif_clk_info[ISPIF_CLK_INFO_MAX];

static int msm_ispif_set_regulator(struct ispif_device *ispif_dev,
	uint8_t enable)
{
	int rc = 0;

	if (enable) {
		if (!ispif_dev->fs_vfe0) {
			ispif_dev->fs_vfe0 = regulator_get(
				&ispif_dev->pdev->dev, "vfe0-vdd");
			if (IS_ERR_OR_NULL(ispif_dev->fs_vfe0)) {
				pr_err("%s: Regulator vfe0 get failed %ld\n",
					__func__,
					PTR_ERR(ispif_dev->fs_vfe0));
				rc = -ENODEV;
				goto vfe0_reg_get_failed;
			}
		}

		if (!ispif_dev->fs_vfe1) {
			ispif_dev->fs_vfe1 = regulator_get(
				&ispif_dev->pdev->dev, "vfe1-vdd");
			if (IS_ERR_OR_NULL(ispif_dev->fs_vfe1)) {
				pr_err("%s: Regulator vfe1 get failed %ld\n",
					__func__,
					PTR_ERR(ispif_dev->fs_vfe1));
				rc = -ENODEV;
				goto vfe1_reg_get_failed;
			}
		}

		if (ispif_dev->fs_vfe0) {
			rc = regulator_enable(ispif_dev->fs_vfe0);
			if (rc) {
				pr_err("%s: Regulator enable for vfe0 failed\n",
					__func__);
				goto fs_vfe0_en_failed;
			}
		}

		if (ispif_dev->fs_vfe1) {
			rc = regulator_enable(ispif_dev->fs_vfe1);
			if (rc) {
				pr_err("%s: Regulator enable for vfe0 failed\n",
					__func__);
				goto fs_vfe1_en_failed;
			}
		}
	} else {
		if (ispif_dev->fs_vfe0) {
			regulator_disable(ispif_dev->fs_vfe0);
			regulator_put(ispif_dev->fs_vfe0);
			ispif_dev->fs_vfe0 = NULL;
		}

		if (ispif_dev->fs_vfe1) {
			regulator_disable(ispif_dev->fs_vfe1);
			regulator_put(ispif_dev->fs_vfe1);
			ispif_dev->fs_vfe1 = NULL;
		}
		if (ispif_dev->fs_camss) {
			regulator_disable(ispif_dev->fs_camss);
			regulator_put(ispif_dev->fs_camss);
			ispif_dev->fs_camss = NULL;
		}
		if (ispif_dev->fs_mmagic_camss) {
			regulator_disable(ispif_dev->fs_mmagic_camss);
			regulator_put(ispif_dev->fs_mmagic_camss);
			ispif_dev->fs_mmagic_camss = NULL;
		}
	}
	return rc;
fs_vfe1_en_failed:
	regulator_disable(ispif_dev->fs_vfe0);
fs_vfe0_en_failed:
	regulator_put(ispif_dev->fs_vfe1);
	ispif_dev->fs_vfe1 = NULL;
vfe1_reg_get_failed:
	regulator_put(ispif_dev->fs_vfe0);
	ispif_dev->fs_vfe0 = NULL;
vfe0_reg_get_failed:
	return rc;
}

static int msm_ispif_reset_hw(struct ispif_device *ispif, int release)
{
	int rc = 0, i;
	long timeout = 0;
	struct clk *reset_clk1[ARRAY_SIZE(ispif_8626_reset_clk_info)];
	ispif->clk_idx = 0;

	rc = msm_ispif_get_clk_info(ispif, ispif->pdev,
		ispif_ahb_clk_info, ispif_clk_info);
	if (rc < 0) {
		pr_err("%s: msm_isp_get_clk_info() failed", __func__);
		return -EFAULT;
	}

	/* Turn ON regulators before enabling the clocks*/
	rc = msm_ispif_set_regulator(ispif, 1);
	if (rc < 0) {
		pr_err("%s: ispif enable regulator failed", __func__);
			return -EFAULT;
	}

	rc = msm_cam_clk_enable(&ispif->pdev->dev,
		ispif_clk_info, ispif->clk,
		ispif->num_clk, 1);
	if (rc < 0) {
		pr_err("%s: cannot enable clock, error = %d\n",
			__func__, rc);
		rc = msm_cam_clk_enable(&ispif->pdev->dev,
			ispif_8626_reset_clk_info, reset_clk1,
			ARRAY_SIZE(ispif_8626_reset_clk_info), 1);
		if (rc < 0) {
			pr_err("%s: cannot enable clock, error = %d",
				__func__, rc);
		} else {
			/* This is set when device is 8x26 */
			ispif->clk_idx = 2;
		}
	} else {
		/* This is set when device is 8974 */
		ispif->clk_idx = 1;
	}

	if (release) {
		for (i = 0; i < ispif->vfe_info.num_vfe; i++) {
			msm_camera_io_w_mb(ISPIF_STOP_INTF_IMMEDIATELY,
				ispif->base + ISPIF_VFE_m_INTF_CMD_0(i));
			msm_camera_io_w_mb(ISPIF_STOP_INTF_IMMEDIATELY,
				ispif->base + ISPIF_VFE_m_INTF_CMD_1(i));
		}
		msm_camera_io_w_mb(ISPIF_IRQ_GLOBAL_CLEAR_CMD,
			ispif->base + ISPIF_IRQ_GLOBAL_CLEAR_CMD_ADDR);
	}
	init_completion(&ispif->reset_complete[VFE0]);
	if (ispif->hw_num_isps > 1)
		init_completion(&ispif->reset_complete[VFE1]);

	/* initiate reset of ISPIF */
	msm_camera_io_w(ISPIF_RST_CMD_MASK,
				ispif->base + ISPIF_RST_CMD_ADDR);


	timeout = wait_for_completion_timeout(
			&ispif->reset_complete[VFE0], msecs_to_jiffies(500));
	CDBG("%s: VFE0 done\n", __func__);

	if (timeout <= 0) {
		pr_err("%s: VFE0 reset wait timeout\n", __func__);
		rc = msm_cam_clk_enable(&ispif->pdev->dev,
			ispif_clk_info, ispif->clk,
			ispif->num_clk, 0);
		if (rc < 0) {
			rc = msm_cam_clk_enable(&ispif->pdev->dev,
				ispif_8626_reset_clk_info, reset_clk1,
				ARRAY_SIZE(ispif_8626_reset_clk_info), 0);
			if (rc < 0)
				pr_err("%s: VFE0 reset wait timeout\n",
					__func__);
		}
		/* Turn OFF regulators */
		rc = msm_ispif_set_regulator(ispif, 0);
		return -ETIMEDOUT;
	}

	if (ispif->hw_num_isps > 1) {
		msm_camera_io_w(ISPIF_RST_CMD_1_MASK,
					ispif->base + ISPIF_RST_CMD_1_ADDR);
		timeout = wait_for_completion_timeout(
				&ispif->reset_complete[VFE1],
				msecs_to_jiffies(500));
		CDBG("%s: VFE1 done\n", __func__);
		if (timeout <= 0) {
			pr_err("%s: VFE1 reset wait timeout\n", __func__);
		rc = msm_cam_clk_enable(&ispif->pdev->dev,
			ispif_clk_info, ispif->clk,
			ispif->num_clk, 0);
			/* Turn OFF regulators */
			rc = msm_ispif_set_regulator(ispif, 0);
			return -ETIMEDOUT;
		}
	}

	if (ispif->clk_idx == 1) {
		rc = msm_cam_clk_enable(&ispif->pdev->dev,
			ispif_clk_info, ispif->clk,
			ispif->num_clk, 0);
		if (rc < 0) {
			pr_err("%s: cannot disable clock, error = %d",
				__func__, rc);
		}
	}

	if (ispif->clk_idx == 2) {
		rc = msm_cam_clk_enable(&ispif->pdev->dev,
			ispif_8626_reset_clk_info, reset_clk1,
			ARRAY_SIZE(ispif_8626_reset_clk_info), 0);
		if (rc < 0) {
			pr_err("%s: cannot disable clock, error = %d",
				__func__, rc);
		}
	}

	/* Turn OFF regulators after enabling the clocks*/
	rc = msm_ispif_set_regulator(ispif, 0);
	if (rc < 0) {
		pr_err("%s: ispif disable regulator failed", __func__);
			return -EFAULT;
	}

	return rc;
}

int msm_ispif_get_clk_info(struct ispif_device *ispif_dev,
	struct platform_device *pdev,
	struct msm_cam_clk_info *ahb_clk_info,
	struct msm_cam_clk_info *clk_info)
{
	uint32_t count, num_ahb_clk = 0;
	const char *rate = NULL;
	int i, rc;

	struct device_node *of_node;
	of_node = pdev->dev.of_node;

	count = of_property_count_strings(of_node, "clock-names");

	CDBG("count = %d\n", count);
	if (count == 0) {
		pr_err("no clocks found in device tree, count=%d", count);
		return 0;
	}

	if (count > ISPIF_CLK_INFO_MAX) {
		pr_err("invalid count=%d, max is %d\n", count,
			ISPIF_CLK_INFO_MAX);
		return -EINVAL;
	}

	for (i = 0; i < count; i++) {
		rc = of_property_read_string_index(of_node, "clock-names",
				i, &(clk_info[i].clk_name));
		if (rc < 0) {
			pr_err("%s reading clock-name failed index %d\n",
				__func__, i);
			return rc;
		}

		rc = of_property_read_string_index(of_node, "qcom,clock-rates",
			i, &rate);
		CDBG("clock-names[%d] = %s, clk_rate = %s\n",
			i, clk_info[i].clk_name, rate);
		if (rc < 0) {
			pr_err("%s reading clock-rate failed index %d\n",
				__func__, i);
			return rc;
		}

		if (!strcmp(rate, "-1") || !strcmp(rate, "0"))
			clk_info[i].clk_rate = NO_SET_RATE;
		else if (!strcmp(rate, "-2"))
			clk_info[i].clk_rate = INIT_RATE;
		else
			rc = kstrtol(rate, 10, &clk_info[i].clk_rate);

		if (strnstr(clk_info[i].clk_name, "ahb",
			strlen(clk_info[i].clk_name))) {
			ahb_clk_info[num_ahb_clk].clk_name =
				clk_info[i].clk_name;
			ahb_clk_info[num_ahb_clk].clk_rate =
				clk_info[i].clk_rate;
			CDBG("clk_name[%d]= %s, clk_rate = %ld\n",
				num_ahb_clk, ahb_clk_info[num_ahb_clk].clk_name,
				ahb_clk_info[num_ahb_clk].clk_rate);
			num_ahb_clk++;
		}
	}
	CDBG("%s: num_ahb_clk %d num_clk %d\n", __func__, num_ahb_clk, count);
	ispif_dev->num_ahb_clk = num_ahb_clk;
	ispif_dev->num_clk = count;
	return 0;
}

static int msm_ispif_clk_ahb_enable(struct ispif_device *ispif, int enable)
{
	int rc = 0;

	if (ispif->csid_version < CSID_VERSION_V30) {
		/* Older ISPIF versiond don't need ahb clokc */
		return 0;
	}

	rc = msm_cam_clk_enable(&ispif->pdev->dev,
		ispif_ahb_clk_info, ispif->ahb_clk,
		ispif->num_ahb_clk, enable);
	if (rc < 0) {
		pr_err("%s: cannot enable clock, error = %d",
			__func__, rc);
	}

	return rc;
}

static int msm_ispif_reset(struct ispif_device *ispif)
{
	int rc = 0;
	int i;

	BUG_ON(!ispif);

	memset(ispif->sof_count, 0, sizeof(ispif->sof_count));
	for (i = 0; i < ispif->vfe_info.num_vfe; i++) {

		msm_camera_io_w(1 << PIX0_LINE_BUF_EN_BIT,
			ispif->base + ISPIF_VFE_m_CTRL_0(i));
		msm_camera_io_w(0, ispif->base + ISPIF_VFE_m_IRQ_MASK_0(i));
		msm_camera_io_w(0, ispif->base + ISPIF_VFE_m_IRQ_MASK_1(i));
		msm_camera_io_w(0, ispif->base + ISPIF_VFE_m_IRQ_MASK_2(i));
		msm_camera_io_w(0xFFFFFFFF, ispif->base +
			ISPIF_VFE_m_IRQ_CLEAR_0(i));
		msm_camera_io_w(0xFFFFFFFF, ispif->base +
			ISPIF_VFE_m_IRQ_CLEAR_1(i));
		msm_camera_io_w(0xFFFFFFFF, ispif->base +
			ISPIF_VFE_m_IRQ_CLEAR_2(i));

		msm_camera_io_w(0, ispif->base + ISPIF_VFE_m_INPUT_SEL(i));

		msm_camera_io_w(ISPIF_STOP_INTF_IMMEDIATELY,
			ispif->base + ISPIF_VFE_m_INTF_CMD_0(i));
		msm_camera_io_w(ISPIF_STOP_INTF_IMMEDIATELY,
			ispif->base + ISPIF_VFE_m_INTF_CMD_1(i));
		pr_debug("%s: base %lx", __func__, (unsigned long)ispif->base);
		msm_camera_io_w(0, ispif->base +
			ISPIF_VFE_m_PIX_INTF_n_CID_MASK(i, 0));
		msm_camera_io_w(0, ispif->base +
			ISPIF_VFE_m_PIX_INTF_n_CID_MASK(i, 1));
		msm_camera_io_w(0, ispif->base +
			ISPIF_VFE_m_RDI_INTF_n_CID_MASK(i, 0));
		msm_camera_io_w(0, ispif->base +
			ISPIF_VFE_m_RDI_INTF_n_CID_MASK(i, 1));
		msm_camera_io_w(0, ispif->base +
			ISPIF_VFE_m_RDI_INTF_n_CID_MASK(i, 2));

		msm_camera_io_w(0, ispif->base +
			ISPIF_VFE_m_PIX_INTF_n_CROP(i, 0));
		msm_camera_io_w(0, ispif->base +
			ISPIF_VFE_m_PIX_INTF_n_CROP(i, 1));
	}

	msm_camera_io_w_mb(ISPIF_IRQ_GLOBAL_CLEAR_CMD, ispif->base +
		ISPIF_IRQ_GLOBAL_CLEAR_CMD_ADDR);

	return rc;
}

static int msm_ispif_subdev_g_chip_ident(struct v4l2_subdev *sd,
	struct v4l2_dbg_chip_ident *chip)
{
	BUG_ON(!chip);
	chip->ident = V4L2_IDENT_ISPIF;
	chip->revision = 0;
	return 0;
}

static void msm_ispif_sel_csid_core(struct ispif_device *ispif,
	uint8_t intftype, uint8_t csid, uint8_t vfe_intf)
{
	uint32_t data;

	BUG_ON(!ispif);

	if (!msm_ispif_is_intf_valid(ispif->csid_version, vfe_intf)) {
		pr_err("%s: invalid interface type\n", __func__);
		return;
	}

	data = msm_camera_io_r(ispif->base + ISPIF_VFE_m_INPUT_SEL(vfe_intf));
	switch (intftype) {
	case PIX0:
		data &= ~(BIT(1) | BIT(0));
		data |= (uint32_t)csid;
		break;
	case RDI0:
		data &= ~(BIT(5) | BIT(4));
		data |= (uint32_t)(csid << 4);
		break;
	case PIX1:
		data &= ~(BIT(9) | BIT(8));
		data |= (uint32_t)(csid << 8);
		break;
	case RDI1:
		data &= ~(BIT(13) | BIT(12));
		data |= (uint32_t)(csid << 12);
		break;
	case RDI2:
		data &= ~(BIT(21) | BIT(20));
		data |= (uint32_t)(csid << 20);
		break;
	}

	msm_camera_io_w_mb(data, ispif->base +
		ISPIF_VFE_m_INPUT_SEL(vfe_intf));
}

static void msm_ispif_enable_crop(struct ispif_device *ispif,
	uint8_t intftype, uint8_t vfe_intf, uint16_t start_pixel,
	uint16_t end_pixel)
{
	uint32_t data;
	BUG_ON(!ispif);

	if (!msm_ispif_is_intf_valid(ispif->csid_version, vfe_intf)) {
		pr_err("%s: invalid interface type\n", __func__);
		return;
	}

	data = msm_camera_io_r(ispif->base + ISPIF_VFE_m_CTRL_0(vfe_intf));
	data |= (1 << (intftype + 7));
	if (intftype == PIX0)
		data |= 1 << PIX0_LINE_BUF_EN_BIT;
	msm_camera_io_w(data,
		ispif->base + ISPIF_VFE_m_CTRL_0(vfe_intf));

	if (intftype == PIX0)
		msm_camera_io_w_mb(start_pixel | (end_pixel << 16),
			ispif->base + ISPIF_VFE_m_PIX_INTF_n_CROP(vfe_intf, 0));
	else if (intftype == PIX1)
		msm_camera_io_w_mb(start_pixel | (end_pixel << 16),
			ispif->base + ISPIF_VFE_m_PIX_INTF_n_CROP(vfe_intf, 1));
	else {
		pr_err("%s: invalid intftype=%d\n", __func__, intftype);
		BUG_ON(1);
		return;
	}
}

static void msm_ispif_enable_intf_cids(struct ispif_device *ispif,
	uint8_t intftype, uint16_t cid_mask, uint8_t vfe_intf, uint8_t enable)
{
	uint32_t intf_addr, data;

	BUG_ON(!ispif);

	if (!msm_ispif_is_intf_valid(ispif->csid_version, vfe_intf)) {
		pr_err("%s: invalid interface type\n", __func__);
		return;
	}

	switch (intftype) {
	case PIX0:
		intf_addr = ISPIF_VFE_m_PIX_INTF_n_CID_MASK(vfe_intf, 0);
		break;
	case RDI0:
		intf_addr = ISPIF_VFE_m_RDI_INTF_n_CID_MASK(vfe_intf, 0);
		break;
	case PIX1:
		intf_addr = ISPIF_VFE_m_PIX_INTF_n_CID_MASK(vfe_intf, 1);
		break;
	case RDI1:
		intf_addr = ISPIF_VFE_m_RDI_INTF_n_CID_MASK(vfe_intf, 1);
		break;
	case RDI2:
		intf_addr = ISPIF_VFE_m_RDI_INTF_n_CID_MASK(vfe_intf, 2);
		break;
	default:
		pr_err("%s: invalid intftype=%d\n", __func__, intftype);
		BUG_ON(1);
		return;
	}

	data = msm_camera_io_r(ispif->base + intf_addr);
	if (enable)
		data |= (uint32_t)cid_mask;
	else
		data &= ~((uint32_t)cid_mask);
	msm_camera_io_w_mb(data, ispif->base + intf_addr);
}

static int msm_ispif_validate_intf_status(struct ispif_device *ispif,
	uint8_t intftype, uint8_t vfe_intf)
{
	int rc = 0;
	uint32_t data = 0;

	BUG_ON(!ispif);

	if (!msm_ispif_is_intf_valid(ispif->csid_version, vfe_intf)) {
		pr_err("%s: invalid interface type\n", __func__);
		return -EINVAL;
	}

	switch (intftype) {
	case PIX0:
		data = msm_camera_io_r(ispif->base +
			ISPIF_VFE_m_PIX_INTF_n_STATUS(vfe_intf, 0));
		break;
	case RDI0:
		data = msm_camera_io_r(ispif->base +
			ISPIF_VFE_m_RDI_INTF_n_STATUS(vfe_intf, 0));
		break;
	case PIX1:
		data = msm_camera_io_r(ispif->base +
			ISPIF_VFE_m_PIX_INTF_n_STATUS(vfe_intf, 1));
		break;
	case RDI1:
		data = msm_camera_io_r(ispif->base +
			ISPIF_VFE_m_RDI_INTF_n_STATUS(vfe_intf, 1));
		break;
	case RDI2:
		data = msm_camera_io_r(ispif->base +
			ISPIF_VFE_m_RDI_INTF_n_STATUS(vfe_intf, 2));
		break;
	}
	if ((data & 0xf) != 0xf)
		rc = -EBUSY;
	return rc;
}

static void msm_ispif_select_clk_mux(struct ispif_device *ispif,
	uint8_t intftype, uint8_t csid, uint8_t vfe_intf)
{
	uint32_t data = 0;

	switch (intftype) {
	case PIX0:
		data = msm_camera_io_r(ispif->clk_mux_base);
		data &= ~(0xf << (vfe_intf * 8));
		data |= (csid << (vfe_intf * 8));
		msm_camera_io_w(data, ispif->clk_mux_base);
		break;

	case RDI0:
		data = msm_camera_io_r(ispif->clk_mux_base +
			ISPIF_RDI_CLK_MUX_SEL_ADDR);
		data &= ~(0xf << (vfe_intf * 12));
		data |= (csid << (vfe_intf * 12));
		msm_camera_io_w(data, ispif->clk_mux_base +
			ISPIF_RDI_CLK_MUX_SEL_ADDR);
		break;

	case PIX1:
		data = msm_camera_io_r(ispif->clk_mux_base);
		data &= ~(0xf0 << (vfe_intf * 8));
		data |= (csid << (4 + (vfe_intf * 8)));
		msm_camera_io_w(data, ispif->clk_mux_base);
		break;

	case RDI1:
		data = msm_camera_io_r(ispif->clk_mux_base +
			ISPIF_RDI_CLK_MUX_SEL_ADDR);
		data &= ~(0xf << (4 + (vfe_intf * 12)));
		data |= (csid << (4 + (vfe_intf * 12)));
		msm_camera_io_w(data, ispif->clk_mux_base +
			ISPIF_RDI_CLK_MUX_SEL_ADDR);
		break;

	case RDI2:
		data = msm_camera_io_r(ispif->clk_mux_base +
			ISPIF_RDI_CLK_MUX_SEL_ADDR);
		data &= ~(0xf << (8 + (vfe_intf * 12)));
		data |= (csid << (8 + (vfe_intf * 12)));
		msm_camera_io_w(data, ispif->clk_mux_base +
			ISPIF_RDI_CLK_MUX_SEL_ADDR);
		break;
	}
	CDBG("%s intftype %d data %x\n", __func__, intftype, data);
	mb();
	return;
}

static uint16_t msm_ispif_get_cids_mask_from_cfg(
	struct msm_ispif_params_entry *entry)
{
	int i;
	uint16_t cids_mask = 0;

	BUG_ON(!entry);

	for (i = 0; i < entry->num_cids; i++)
		cids_mask |= (1 << entry->cids[i]);

	return cids_mask;
}

static int msm_ispif_config(struct ispif_device *ispif,
	struct msm_ispif_param_data *params)
{
	int rc = 0, i = 0;
	uint16_t cid_mask;
	enum msm_ispif_intftype intftype;
	enum msm_ispif_vfe_intf vfe_intf;

	BUG_ON(!ispif);
	BUG_ON(!params);

	if (!ispif->base) {
		pr_err("%s: ispif base is NULL\n", __func__);
		rc = -EPERM;
		return rc;
	}

	if (ispif->ispif_state != ISPIF_POWER_UP) {
		pr_err("%s: ispif invalid state %d\n", __func__,
			ispif->ispif_state);
		rc = -EPERM;
		return rc;
	}
	if (params->num > MAX_PARAM_ENTRIES) {
		pr_err("%s: invalid param entries %d\n", __func__,
			params->num);
		rc = -EINVAL;
		return rc;
	}

	for (i = 0; i < params->num; i++) {
		vfe_intf = params->entries[i].vfe_intf;
		if (!msm_ispif_is_intf_valid(ispif->csid_version,
				vfe_intf)) {
			pr_err("%s: invalid interface type\n", __func__);
			return -EINVAL;
		}
		msm_camera_io_w(0x0, ispif->base +
			ISPIF_VFE_m_IRQ_MASK_0(vfe_intf));
		msm_camera_io_w(0x0, ispif->base +
			ISPIF_VFE_m_IRQ_MASK_1(vfe_intf));
		msm_camera_io_w_mb(0x0, ispif->base +
			ISPIF_VFE_m_IRQ_MASK_2(vfe_intf));
	}

	for (i = 0; i < params->num; i++) {
		intftype = params->entries[i].intftype;

		vfe_intf = params->entries[i].vfe_intf;

		CDBG("%s intftype %x, vfe_intf %d, csid %d\n", __func__,
			intftype, vfe_intf, params->entries[i].csid);

		if ((intftype >= INTF_MAX) ||
			(vfe_intf >=  ispif->vfe_info.num_vfe) ||
			(ispif->csid_version <= CSID_VERSION_V22 &&
			(vfe_intf > VFE0))) {
			pr_err("%s: VFEID %d and CSID version %d mismatch\n",
				__func__, vfe_intf, ispif->csid_version);
			return -EINVAL;
		}

		if (ispif->csid_version >= CSID_VERSION_V30)
				msm_ispif_select_clk_mux(ispif, intftype,
				params->entries[i].csid, vfe_intf);

		rc = msm_ispif_validate_intf_status(ispif, intftype, vfe_intf);
		if (rc) {
			pr_err("%s:validate_intf_status failed, rc = %d\n",
				__func__, rc);
			return rc;
		}

		msm_ispif_sel_csid_core(ispif, intftype,
			params->entries[i].csid, vfe_intf);
		cid_mask = msm_ispif_get_cids_mask_from_cfg(
				&params->entries[i]);
		msm_ispif_enable_intf_cids(ispif, intftype,
			cid_mask, vfe_intf, 1);
		if (params->entries[i].crop_enable)
			msm_ispif_enable_crop(ispif, intftype, vfe_intf,
				params->entries[i].crop_start_pixel,
				params->entries[i].crop_end_pixel);
	}

	for (vfe_intf = 0; vfe_intf < 2; vfe_intf++) {
		msm_camera_io_w(ISPIF_IRQ_STATUS_MASK, ispif->base +
			ISPIF_VFE_m_IRQ_MASK_0(vfe_intf));

		msm_camera_io_w(ISPIF_IRQ_STATUS_MASK, ispif->base +
			ISPIF_VFE_m_IRQ_CLEAR_0(vfe_intf));

		msm_camera_io_w(ISPIF_IRQ_STATUS_1_MASK, ispif->base +
			ISPIF_VFE_m_IRQ_MASK_1(vfe_intf));

		msm_camera_io_w(ISPIF_IRQ_STATUS_1_MASK, ispif->base +
			ISPIF_VFE_m_IRQ_CLEAR_1(vfe_intf));

		msm_camera_io_w(ISPIF_IRQ_STATUS_2_MASK, ispif->base +
			ISPIF_VFE_m_IRQ_MASK_2(vfe_intf));

		msm_camera_io_w(ISPIF_IRQ_STATUS_2_MASK, ispif->base +
			ISPIF_VFE_m_IRQ_CLEAR_2(vfe_intf));
	}

	msm_camera_io_w_mb(ISPIF_IRQ_GLOBAL_CLEAR_CMD, ispif->base +
		ISPIF_IRQ_GLOBAL_CLEAR_CMD_ADDR);

	return rc;
}

static void msm_ispif_intf_cmd(struct ispif_device *ispif, uint32_t cmd_bits,
	struct msm_ispif_param_data *params)
{
	uint8_t vc;
	int i, k;
	enum msm_ispif_intftype intf_type;
	enum msm_ispif_cid cid;
	enum msm_ispif_vfe_intf vfe_intf;

	BUG_ON(!ispif);
	BUG_ON(!params);

	for (i = 0; i < params->num; i++) {
		vfe_intf = params->entries[i].vfe_intf;
		if (!msm_ispif_is_intf_valid(ispif->csid_version, vfe_intf)) {
			pr_err("%s: invalid interface type\n", __func__);
			return;
		}
		if (params->entries[i].num_cids > MAX_CID_CH) {
			pr_err("%s: out of range of cid_num %d\n",
				__func__, params->entries[i].num_cids);
			return;
		}
	}

	for (i = 0; i < params->num; i++) {
		intf_type = params->entries[i].intftype;
		vfe_intf = params->entries[i].vfe_intf;
		for (k = 0; k < params->entries[i].num_cids; k++) {
			cid = params->entries[i].cids[k];
			vc = cid / 4;
			if (intf_type == RDI2) {
				/* zero out two bits */
				ispif->applied_intf_cmd[vfe_intf].intf_cmd1 &=
					~(0x3 << (vc * 2 + 8));
				/* set cmd bits */
				ispif->applied_intf_cmd[vfe_intf].intf_cmd1 |=
					(cmd_bits << (vc * 2 + 8));
			} else {
				/* zero 2 bits */
				ispif->applied_intf_cmd[vfe_intf].intf_cmd &=
					~(0x3 << (vc * 2 + intf_type * 8));
				/* set cmd bits */
				ispif->applied_intf_cmd[vfe_intf].intf_cmd |=
					(cmd_bits << (vc * 2 + intf_type * 8));
			}
		}
		/* cmd for PIX0, PIX1, RDI0, RDI1 */
		if (ispif->applied_intf_cmd[vfe_intf].intf_cmd != 0xFFFFFFFF)
			msm_camera_io_w_mb(
				ispif->applied_intf_cmd[vfe_intf].intf_cmd,
				ispif->base + ISPIF_VFE_m_INTF_CMD_0(vfe_intf));

		/* cmd for RDI2 */
		if (ispif->applied_intf_cmd[vfe_intf].intf_cmd1 != 0xFFFFFFFF)
			msm_camera_io_w_mb(
				ispif->applied_intf_cmd[vfe_intf].intf_cmd1,
				ispif->base + ISPIF_VFE_m_INTF_CMD_1(vfe_intf));
	}
}

static int msm_ispif_stop_immediately(struct ispif_device *ispif,
	struct msm_ispif_param_data *params)
{
	int i, rc = 0;
	uint16_t cid_mask = 0;
	BUG_ON(!ispif);
	BUG_ON(!params);

	if (ispif->ispif_state != ISPIF_POWER_UP) {
		pr_err("%s: ispif invalid state %d\n", __func__,
			ispif->ispif_state);
		rc = -EPERM;
		return rc;
	}

	if (params->num > MAX_PARAM_ENTRIES) {
		pr_err("%s: invalid param entries %d\n", __func__,
			params->num);
		rc = -EINVAL;
		return rc;
	}
	msm_ispif_intf_cmd(ispif, ISPIF_INTF_CMD_DISABLE_IMMEDIATELY, params);

	/* after stop the interface we need to unmask the CID enable bits */
	for (i = 0; i < params->num; i++) {
		cid_mask = msm_ispif_get_cids_mask_from_cfg(
			&params->entries[i]);
		msm_ispif_enable_intf_cids(ispif, params->entries[i].intftype,
			cid_mask, params->entries[i].vfe_intf, 0);
	}

	return rc;
}

static int msm_ispif_start_frame_boundary(struct ispif_device *ispif,
	struct msm_ispif_param_data *params)
{
	int rc = 0;

	if (ispif->ispif_state != ISPIF_POWER_UP) {
		pr_err("%s: ispif invalid state %d\n", __func__,
			ispif->ispif_state);
		rc = -EPERM;
		return rc;
	}
	if (params->num > MAX_PARAM_ENTRIES) {
		pr_err("%s: invalid param entries %d\n", __func__,
			params->num);
		rc = -EINVAL;
		return rc;
	}
	msm_ispif_intf_cmd(ispif, ISPIF_INTF_CMD_ENABLE_FRAME_BOUNDARY, params);

	return rc;
}

static int msm_ispif_restart_frame_boundary(struct ispif_device *ispif,
	struct msm_ispif_param_data *params)
{
	int rc = 0, i;
	long timeout = 0;
	uint16_t cid_mask;
	enum msm_ispif_intftype intftype;
	enum msm_ispif_vfe_intf vfe_intf;
	uint32_t vfe_mask = 0;
	uint32_t intf_addr;

	if (ispif->ispif_state != ISPIF_POWER_UP) {
		pr_err("%s: ispif invalid state %d\n", __func__,
			ispif->ispif_state);
		rc = -EPERM;
		return rc;
	}
	if (params->num > MAX_PARAM_ENTRIES) {
		pr_err("%s: invalid param entries %d\n", __func__,
			params->num);
		rc = -EINVAL;
		return rc;
	}

	for (i = 0; i < params->num; i++) {
		vfe_intf = params->entries[i].vfe_intf;
		if (vfe_intf >= VFE_MAX) {
			pr_err("%s: %d invalid i %d vfe_intf %d\n", __func__,
				__LINE__, i, vfe_intf);
			return -EINVAL;
		}
		vfe_mask |= (1 << vfe_intf);
	}

	/* Turn ON regulators before enabling the clocks*/
	rc = msm_ispif_set_regulator(ispif, 1);
	if (rc < 0) {
		pr_err("%s: ispif enable regulator failed", __func__);
			return -EFAULT;
	}

	rc = msm_cam_clk_enable(&ispif->pdev->dev,
		ispif_clk_info, ispif->clk,
		ispif->num_clk, 1);
	if (rc < 0) {
		pr_err("%s: cannot enable clock, error = %d",
			__func__, rc);
			goto disable_regulator;
	}

	if (vfe_mask & (1 << VFE0)) {
		init_completion(&ispif->reset_complete[VFE0]);
		/* initiate reset of ISPIF */
		msm_camera_io_w(0x00001FF9,
				ispif->base + ISPIF_RST_CMD_ADDR);
	}

	if (vfe_mask & (1 << VFE0)) {
		timeout = wait_for_completion_timeout(
			&ispif->reset_complete[VFE0], msecs_to_jiffies(500));
		if (timeout <= 0) {
			pr_err("%s: VFE0 reset wait timeout\n", __func__);
			rc = -ETIMEDOUT;
			goto disable_clk;
		}
	}

	if (ispif->hw_num_isps > 1 && (vfe_mask & (1 << VFE1))) {
			init_completion(&ispif->reset_complete[VFE1]);
			msm_camera_io_w(0x00001FF9,
				ispif->base + ISPIF_RST_CMD_1_ADDR);
	}

	if (ispif->hw_num_isps > 1  && (vfe_mask & (1 << VFE1))) {
		timeout = wait_for_completion_timeout(
				&ispif->reset_complete[VFE1],
				msecs_to_jiffies(500));
		if (timeout <= 0) {
			pr_err("%s: VFE1 reset wait timeout\n", __func__);
			rc = -ETIMEDOUT;
			goto disable_clk;
		}
	}

	pr_info("%s: ISPIF reset hw done, Restarting", __func__);
	rc = msm_cam_clk_enable(&ispif->pdev->dev,
		ispif_clk_info, ispif->clk,
		ispif->num_clk, 0);
	if (rc < 0) {
		pr_err("%s: cannot enable clock, error = %d",
			__func__, rc);
			goto disable_regulator;
	}
	/* Turn OFF regulators after disabling clocks */
	rc = msm_ispif_set_regulator(ispif, 0);
	if (rc < 0) {
		pr_err("%s: ispif disable regulator failed", __func__);
		rc = -EFAULT;
		goto end;
	}

	for (i = 0; i < params->num; i++) {
		intftype = params->entries[i].intftype;
		vfe_intf = params->entries[i].vfe_intf;

		switch (params->entries[0].intftype) {
		case PIX0:
			intf_addr = ISPIF_VFE_m_PIX_INTF_n_STATUS(vfe_intf, 0);
			break;
		case RDI0:
			intf_addr = ISPIF_VFE_m_RDI_INTF_n_STATUS(vfe_intf, 0);
			break;
		case PIX1:
			intf_addr = ISPIF_VFE_m_PIX_INTF_n_STATUS(vfe_intf, 1);
			break;
		case RDI1:
			intf_addr = ISPIF_VFE_m_RDI_INTF_n_STATUS(vfe_intf, 1);
			break;
		case RDI2:
			intf_addr = ISPIF_VFE_m_RDI_INTF_n_STATUS(vfe_intf, 2);
			break;
		default:
			pr_err("%s: invalid intftype=%d\n", __func__,
			params->entries[i].intftype);
			rc = -EPERM;
			goto end;
		}

		msm_ispif_intf_cmd(ispif,
			ISPIF_INTF_CMD_ENABLE_FRAME_BOUNDARY, params);
	}

	for (i = 0; i < params->num; i++) {
		intftype = params->entries[i].intftype;

		vfe_intf = params->entries[i].vfe_intf;


		cid_mask = msm_ispif_get_cids_mask_from_cfg(
			&params->entries[i]);

		msm_ispif_enable_intf_cids(ispif, intftype,
			cid_mask, vfe_intf, 1);
	}
	return rc;

disable_clk:
	msm_cam_clk_enable(&ispif->pdev->dev,
		ispif_clk_info, ispif->clk,
		ispif->num_clk, 0);
disable_regulator:
	/* Turn OFF regulators */
	msm_ispif_set_regulator(ispif, 0);
end:
	return rc;
}

static int msm_ispif_stop_frame_boundary(struct ispif_device *ispif,
	struct msm_ispif_param_data *params)
{
	int i, rc = 0;
	uint16_t cid_mask = 0;
	uint32_t intf_addr;
	enum msm_ispif_vfe_intf vfe_intf;
	uint32_t stop_flag = 0;

	BUG_ON(!ispif);
	BUG_ON(!params);


	if (ispif->ispif_state != ISPIF_POWER_UP) {
		pr_err("%s: ispif invalid state %d\n", __func__,
			ispif->ispif_state);
		rc = -EPERM;
		return rc;
	}

	if (params->num > MAX_PARAM_ENTRIES) {
		pr_err("%s: invalid param entries %d\n", __func__,
			params->num);
		rc = -EINVAL;
		return rc;
	}

	for (i = 0; i < params->num; i++) {
		if (!msm_ispif_is_intf_valid(ispif->csid_version,
				params->entries[i].vfe_intf)) {
			pr_err("%s: invalid interface type\n", __func__);
			rc = -EINVAL;
			goto end;
		}
	}

	msm_ispif_intf_cmd(ispif,
		ISPIF_INTF_CMD_DISABLE_FRAME_BOUNDARY, params);

	for (i = 0; i < params->num; i++) {
		cid_mask =
			msm_ispif_get_cids_mask_from_cfg(&params->entries[i]);
		vfe_intf = params->entries[i].vfe_intf;

		switch (params->entries[i].intftype) {
		case PIX0:
			intf_addr = ISPIF_VFE_m_PIX_INTF_n_STATUS(vfe_intf, 0);
			break;
		case RDI0:
			intf_addr = ISPIF_VFE_m_RDI_INTF_n_STATUS(vfe_intf, 0);
			break;
		case PIX1:
			intf_addr = ISPIF_VFE_m_PIX_INTF_n_STATUS(vfe_intf, 1);
			break;
		case RDI1:
			intf_addr = ISPIF_VFE_m_RDI_INTF_n_STATUS(vfe_intf, 1);
			break;
		case RDI2:
			intf_addr = ISPIF_VFE_m_RDI_INTF_n_STATUS(vfe_intf, 2);
			break;
		default:
			pr_err("%s: invalid intftype=%d\n", __func__,
				params->entries[i].intftype);
			rc = -EPERM;
			goto end;
		}

		rc = readl_poll_timeout(ispif->base + intf_addr, stop_flag,
					(stop_flag & 0xF) == 0xF,
					ISPIF_TIMEOUT_SLEEP_US,
					ISPIF_TIMEOUT_ALL_US);
		if (rc < 0)
			goto end;

		/* disable CIDs in CID_MASK register */
		msm_ispif_enable_intf_cids(ispif, params->entries[i].intftype,
			cid_mask, vfe_intf, 0);
	}

end:
	return rc;
}

static void ispif_process_irq(struct ispif_device *ispif,
	struct ispif_irq_status *out, enum msm_ispif_vfe_intf vfe_id)
{
	BUG_ON(!ispif);
	BUG_ON(!out);

	if (out[vfe_id].ispifIrqStatus0 &
			ISPIF_IRQ_STATUS_PIX_SOF_MASK) {
		if (ispif->ispif_sof_debug < ISPIF_SOF_DEBUG_COUNT)
			pr_err("%s: PIX0 frame id: %u\n", __func__,
				ispif->sof_count[vfe_id].sof_cnt[PIX0]);
		ispif->sof_count[vfe_id].sof_cnt[PIX0]++;
		ispif->ispif_sof_debug++;
	}
	if (out[vfe_id].ispifIrqStatus0 &
			ISPIF_IRQ_STATUS_RDI0_SOF_MASK) {
		if (ispif->ispif_rdi0_debug < ISPIF_SOF_DEBUG_COUNT)
			pr_err("%s: RDI0 frame id: %u\n", __func__,
				ispif->sof_count[vfe_id].sof_cnt[RDI0]);
		ispif->sof_count[vfe_id].sof_cnt[RDI0]++;
		ispif->ispif_rdi0_debug++;
	}
	if (out[vfe_id].ispifIrqStatus1 &
			ISPIF_IRQ_STATUS_RDI1_SOF_MASK) {
		if (ispif->ispif_rdi1_debug < ISPIF_SOF_DEBUG_COUNT)
			pr_err("%s: RDI1 frame id: %u\n", __func__,
				ispif->sof_count[vfe_id].sof_cnt[RDI1]);
		ispif->sof_count[vfe_id].sof_cnt[RDI1]++;
		ispif->ispif_rdi1_debug++;
	}
	if (out[vfe_id].ispifIrqStatus2 &
			ISPIF_IRQ_STATUS_RDI2_SOF_MASK) {
		if (ispif->ispif_rdi2_debug < ISPIF_SOF_DEBUG_COUNT)
			pr_err("%s: RDI2 frame id: %u\n", __func__,
				ispif->sof_count[vfe_id].sof_cnt[RDI2]);
		ispif->sof_count[vfe_id].sof_cnt[RDI2]++;
		ispif->ispif_rdi2_debug++;
	}
}

static inline void msm_ispif_read_irq_status(struct ispif_irq_status *out,
	void *data)
{
	struct ispif_device *ispif = (struct ispif_device *)data;

	BUG_ON(!ispif);
	BUG_ON(!out);

	out[VFE0].ispifIrqStatus0 = msm_camera_io_r(ispif->base +
		ISPIF_VFE_m_IRQ_STATUS_0(VFE0));
	msm_camera_io_w(out[VFE0].ispifIrqStatus0,
		ispif->base + ISPIF_VFE_m_IRQ_CLEAR_0(VFE0));

	out[VFE0].ispifIrqStatus1 = msm_camera_io_r(ispif->base +
		ISPIF_VFE_m_IRQ_STATUS_1(VFE0));
	msm_camera_io_w(out[VFE0].ispifIrqStatus1,
		ispif->base + ISPIF_VFE_m_IRQ_CLEAR_1(VFE0));

	out[VFE0].ispifIrqStatus2 = msm_camera_io_r(ispif->base +
		ISPIF_VFE_m_IRQ_STATUS_2(VFE0));
	msm_camera_io_w_mb(out[VFE0].ispifIrqStatus2,
		ispif->base + ISPIF_VFE_m_IRQ_CLEAR_2(VFE0));

	if (ispif->vfe_info.num_vfe > 1) {
		out[VFE1].ispifIrqStatus0 = msm_camera_io_r(ispif->base +
			ISPIF_VFE_m_IRQ_STATUS_0(VFE1));
		msm_camera_io_w(out[VFE1].ispifIrqStatus0,
			ispif->base + ISPIF_VFE_m_IRQ_CLEAR_0(VFE1));

		out[VFE1].ispifIrqStatus1 = msm_camera_io_r(ispif->base +
			ISPIF_VFE_m_IRQ_STATUS_1(VFE1));
		msm_camera_io_w(out[VFE1].ispifIrqStatus1,
				ispif->base + ISPIF_VFE_m_IRQ_CLEAR_1(VFE1));

		out[VFE1].ispifIrqStatus2 = msm_camera_io_r(ispif->base +
			ISPIF_VFE_m_IRQ_STATUS_2(VFE1));
		msm_camera_io_w_mb(out[VFE1].ispifIrqStatus2,
			ispif->base + ISPIF_VFE_m_IRQ_CLEAR_2(VFE1));
	}
	msm_camera_io_w_mb(ISPIF_IRQ_GLOBAL_CLEAR_CMD, ispif->base +
	ISPIF_IRQ_GLOBAL_CLEAR_CMD_ADDR);

	if (out[VFE0].ispifIrqStatus0 & ISPIF_IRQ_STATUS_MASK) {
		if (out[VFE0].ispifIrqStatus0 & RESET_DONE_IRQ)
			complete(&ispif->reset_complete[VFE0]);

		if (out[VFE0].ispifIrqStatus0 & PIX_INTF_0_OVERFLOW_IRQ)
			pr_err("%s: VFE0 pix0 overflow.\n", __func__);

		if (out[VFE0].ispifIrqStatus0 & RAW_INTF_0_OVERFLOW_IRQ)
			pr_err("%s: VFE0 rdi0 overflow.\n", __func__);

		if (out[VFE0].ispifIrqStatus1 & RAW_INTF_1_OVERFLOW_IRQ)
			pr_err("%s: VFE0 rdi1 overflow.\n", __func__);

		if (out[VFE0].ispifIrqStatus2 & RAW_INTF_2_OVERFLOW_IRQ)
			pr_err("%s: VFE0 rdi2 overflow.\n", __func__);

		ispif_process_irq(ispif, out, VFE0);
	}
	if (ispif->hw_num_isps > 1) {
		if (out[VFE1].ispifIrqStatus0 & RESET_DONE_IRQ)
			complete(&ispif->reset_complete[VFE1]);

		if (out[VFE1].ispifIrqStatus0 & PIX_INTF_0_OVERFLOW_IRQ)
			pr_err("%s: VFE1 pix0 overflow.\n", __func__);

		if (out[VFE1].ispifIrqStatus0 & RAW_INTF_0_OVERFLOW_IRQ)
			pr_err("%s: VFE1 rdi0 overflow.\n", __func__);

		if (out[VFE1].ispifIrqStatus1 & RAW_INTF_1_OVERFLOW_IRQ)
			pr_err("%s: VFE1 rdi1 overflow.\n", __func__);

		if (out[VFE1].ispifIrqStatus2 & RAW_INTF_2_OVERFLOW_IRQ)
			pr_err("%s: VFE1 rdi2 overflow.\n", __func__);

		ispif_process_irq(ispif, out, VFE1);
	}
}

static irqreturn_t msm_io_ispif_irq(int irq_num, void *data)
{
	struct ispif_irq_status irq[VFE_MAX];

	msm_ispif_read_irq_status(irq, data);
	return IRQ_HANDLED;
}

static int msm_ispif_set_vfe_info(struct ispif_device *ispif,
	struct msm_ispif_vfe_info *vfe_info)
{
	if (!vfe_info || (vfe_info->num_vfe == 0) ||
		(vfe_info->num_vfe > ispif->hw_num_isps)) {
		pr_err("Invalid VFE info: %p %d\n", vfe_info,
			   (vfe_info ? vfe_info->num_vfe : 0));
		return -EINVAL;
	}

	memcpy(&ispif->vfe_info, vfe_info, sizeof(struct msm_ispif_vfe_info));

	return 0;
}

static int msm_ispif_init(struct ispif_device *ispif,
	uint32_t csid_version)
{
	int rc = 0;

	BUG_ON(!ispif);

	if (ispif->ispif_state == ISPIF_POWER_UP) {
		pr_err("%s: ispif already initted state = %d\n", __func__,
			ispif->ispif_state);
		rc = -EPERM;
		return rc;
	}

	/* can we set to zero? */
	ispif->applied_intf_cmd[VFE0].intf_cmd  = 0xFFFFFFFF;
	ispif->applied_intf_cmd[VFE0].intf_cmd1 = 0xFFFFFFFF;
	ispif->applied_intf_cmd[VFE1].intf_cmd  = 0xFFFFFFFF;
	ispif->applied_intf_cmd[VFE1].intf_cmd1 = 0xFFFFFFFF;
	memset(ispif->sof_count, 0, sizeof(ispif->sof_count));

	ispif->csid_version = csid_version;

	if (ispif->csid_version >= CSID_VERSION_V30) {
		if (!ispif->clk_mux_mem || !ispif->clk_mux_io) {
			pr_err("%s csi clk mux mem %p io %p\n", __func__,
				ispif->clk_mux_mem, ispif->clk_mux_io);
			rc = -ENOMEM;
			return rc;
		}
		ispif->clk_mux_base = ioremap(ispif->clk_mux_mem->start,
			resource_size(ispif->clk_mux_mem));
		if (!ispif->clk_mux_base) {
			pr_err("%s: clk_mux_mem ioremap failed\n", __func__);
			rc = -ENOMEM;
			return rc;
		}
	}

	ispif->base = ioremap(ispif->mem->start,
		resource_size(ispif->mem));
	if (!ispif->base) {
		rc = -ENOMEM;
		pr_err("%s: nomem\n", __func__);
		goto end;
	}
	rc = request_irq(ispif->irq->start, msm_io_ispif_irq,
		IRQF_TRIGGER_RISING, "ispif", ispif);
	if (rc) {
		pr_err("%s: request_irq error = %d\n", __func__, rc);
		goto error_irq;
	}

	msm_ispif_reset_hw(ispif, 0);

	rc = msm_ispif_clk_ahb_enable(ispif, 1);
	if (rc) {
		pr_err("%s: ahb_clk enable failed", __func__);
		goto error_ahb;
	}

	rc = msm_ispif_reset(ispif);
	if (rc == 0) {
		ispif->ispif_state = ISPIF_POWER_UP;
		CDBG("%s: power up done\n", __func__);
		goto end;
	}

error_ahb:
	free_irq(ispif->irq->start, ispif);
error_irq:
	iounmap(ispif->base);

end:
	return rc;
}

static void msm_ispif_release(struct ispif_device *ispif)
{
	BUG_ON(!ispif);

	if (!ispif->base) {
		pr_err("%s: ispif base is NULL\n", __func__);
		return;
	}

	if (ispif->ispif_state != ISPIF_POWER_UP) {
		pr_err("%s: ispif invalid state %d\n", __func__,
			ispif->ispif_state);
		return;
	}

	/* make sure no streaming going on */
	msm_ispif_reset_hw(ispif, 1);
	msm_ispif_clk_ahb_enable(ispif, 0);

	free_irq(ispif->irq->start, ispif);

	iounmap(ispif->base);

	iounmap(ispif->clk_mux_base);

	ispif->ispif_state = ISPIF_POWER_DOWN;

}

static long msm_ispif_cmd(struct v4l2_subdev *sd, void *arg)
{
	long rc = 0;
	struct ispif_cfg_data *pcdata = (struct ispif_cfg_data *)arg;
	struct ispif_device *ispif =
		(struct ispif_device *)v4l2_get_subdevdata(sd);

	BUG_ON(!sd);
	BUG_ON(!pcdata);

	mutex_lock(&ispif->mutex);
	switch (pcdata->cfg_type) {
	case ISPIF_ENABLE_REG_DUMP:
		ispif->enb_dump_reg = pcdata->reg_dump; /* save dump config */
		break;
	case ISPIF_INIT:
		rc = msm_ispif_init(ispif, pcdata->csid_version);
		msm_ispif_io_dump_reg(ispif);
		break;
	case ISPIF_CFG:
		rc = msm_ispif_config(ispif, &pcdata->params);
		msm_ispif_io_dump_reg(ispif);
		break;
	case ISPIF_START_FRAME_BOUNDARY:
		rc = msm_ispif_start_frame_boundary(ispif, &pcdata->params);
		msm_ispif_io_dump_reg(ispif);
		break;
	case ISPIF_RESTART_FRAME_BOUNDARY:
		rc = msm_ispif_restart_frame_boundary(ispif, &pcdata->params);
		msm_ispif_io_dump_reg(ispif);
		break;

	case ISPIF_STOP_FRAME_BOUNDARY:
		rc = msm_ispif_stop_frame_boundary(ispif, &pcdata->params);
		msm_ispif_io_dump_reg(ispif);
		break;
	case ISPIF_STOP_IMMEDIATELY:
		rc = msm_ispif_stop_immediately(ispif, &pcdata->params);
		msm_ispif_io_dump_reg(ispif);
		break;
	case ISPIF_RELEASE:
		msm_ispif_release(ispif);
		break;
	case ISPIF_SET_VFE_INFO:
		rc = msm_ispif_set_vfe_info(ispif, &pcdata->vfe_info);
		break;
	default:
		pr_err("%s: invalid cfg_type\n", __func__);
		rc = -EINVAL;
		break;
	}
	mutex_unlock(&ispif->mutex);
	return rc;
}
static struct v4l2_file_operations msm_ispif_v4l2_subdev_fops;

static long msm_ispif_subdev_ioctl(struct v4l2_subdev *sd,
	unsigned int cmd, void *arg)
{
	struct ispif_device *ispif =
		(struct ispif_device *)v4l2_get_subdevdata(sd);

	switch (cmd) {
	case VIDIOC_MSM_ISPIF_CFG:
		return msm_ispif_cmd(sd, arg);
	case MSM_SD_NOTIFY_FREEZE: {
		ispif->ispif_sof_debug = 0;
		ispif->ispif_rdi0_debug = 0;
		ispif->ispif_rdi1_debug = 0;
		ispif->ispif_rdi2_debug = 0;
		return 0;
	}
	case MSM_SD_SHUTDOWN: {
		if (ispif && ispif->base) {
			mutex_lock(&ispif->mutex);
			msm_ispif_release(ispif);
			mutex_unlock(&ispif->mutex);
		}
		return 0;
	}
	default:
		pr_err_ratelimited("%s: invalid cmd 0x%x received\n",
			__func__, cmd);
		return -ENOIOCTLCMD;
	}
}

static long msm_ispif_subdev_do_ioctl(
	struct file *file, unsigned int cmd, void *arg)
{
	struct video_device *vdev = video_devdata(file);
	struct v4l2_subdev *sd = vdev_to_v4l2_subdev(vdev);
	return msm_ispif_subdev_ioctl(sd, cmd, arg);
}

static long msm_ispif_subdev_fops_ioctl(struct file *file, unsigned int cmd,
	unsigned long arg)
{
	return video_usercopy(file, cmd, arg, msm_ispif_subdev_do_ioctl);
}

static int ispif_open_node(struct v4l2_subdev *sd, struct v4l2_subdev_fh *fh)
{
	struct ispif_device *ispif = v4l2_get_subdevdata(sd);
	mutex_lock(&ispif->mutex);
	/* mem remap is done in init when the clock is on */
	ispif->open_cnt++;
	mutex_unlock(&ispif->mutex);
	return 0;
}

static int ispif_close_node(struct v4l2_subdev *sd, struct v4l2_subdev_fh *fh)
{
	int rc = 0;
	struct ispif_device *ispif = v4l2_get_subdevdata(sd);
	if (!ispif) {
		pr_err("%s: invalid input\n", __func__);
		return -EINVAL;
	}

	mutex_lock(&ispif->mutex);
	if (ispif->open_cnt == 0) {
		pr_err("%s: Invalid close\n", __func__);
		rc = -ENODEV;
		goto end;
	}
	ispif->open_cnt--;
	if (ispif->open_cnt == 0)
		msm_ispif_release(ispif);
end:
	mutex_unlock(&ispif->mutex);
	return rc;
}

static struct v4l2_subdev_core_ops msm_ispif_subdev_core_ops = {
	.g_chip_ident = &msm_ispif_subdev_g_chip_ident,
	.ioctl = &msm_ispif_subdev_ioctl,
};

static const struct v4l2_subdev_ops msm_ispif_subdev_ops = {
	.core = &msm_ispif_subdev_core_ops,
};

static const struct v4l2_subdev_internal_ops msm_ispif_internal_ops = {
	.open = ispif_open_node,
	.close = ispif_close_node,
};

static int ispif_probe(struct platform_device *pdev)
{
	int rc;
	struct ispif_device *ispif;

	ispif = kzalloc(sizeof(struct ispif_device), GFP_KERNEL);
	if (!ispif) {
		pr_err("%s: no enough memory\n", __func__);
		return -ENOMEM;
	}

	if (pdev->dev.of_node) {
		of_property_read_u32((&pdev->dev)->of_node,
		"cell-index", &pdev->id);
		rc = of_property_read_u32((&pdev->dev)->of_node,
		"qcom,num-isps", &ispif->hw_num_isps);
		if (rc)
			/* backward compatibility */
			ispif->hw_num_isps = 1;
		/* not an error condition */
		rc = 0;
	}

	rc = msm_ispif_get_clk_info(ispif, pdev,
		ispif_ahb_clk_info, ispif_clk_info);
	if (rc < 0) {
		pr_err("%s: msm_isp_get_clk_info() failed", __func__);
			return -EFAULT;
	}

	mutex_init(&ispif->mutex);
	ispif->mem = platform_get_resource_byname(pdev,
		IORESOURCE_MEM, "ispif");
	if (!ispif->mem) {
		pr_err("%s: no mem resource?\n", __func__);
		rc = -ENODEV;
		goto error;
	}
	ispif->irq = platform_get_resource_byname(pdev,
		IORESOURCE_IRQ, "ispif");
	if (!ispif->irq) {
		pr_err("%s: no irq resource?\n", __func__);
		rc = -ENODEV;
		goto error;
	}
	ispif->io = request_mem_region(ispif->mem->start,
		resource_size(ispif->mem), pdev->name);
	if (!ispif->io) {
		pr_err("%s: no valid mem region\n", __func__);
		rc = -EBUSY;
		goto error;
	}
	ispif->clk_mux_mem = platform_get_resource_byname(pdev,
		IORESOURCE_MEM, "csi_clk_mux");
	if (ispif->clk_mux_mem) {
		ispif->clk_mux_io = request_mem_region(
			ispif->clk_mux_mem->start,
			resource_size(ispif->clk_mux_mem),
			ispif->clk_mux_mem->name);
		if (!ispif->clk_mux_io)
			pr_err("%s: no valid csi_mux region\n", __func__);
	}

	v4l2_subdev_init(&ispif->msm_sd.sd, &msm_ispif_subdev_ops);
	ispif->msm_sd.sd.internal_ops = &msm_ispif_internal_ops;
	ispif->msm_sd.sd.flags |= V4L2_SUBDEV_FL_HAS_DEVNODE;

	snprintf(ispif->msm_sd.sd.name,
		ARRAY_SIZE(ispif->msm_sd.sd.name), MSM_ISPIF_DRV_NAME);
	v4l2_set_subdevdata(&ispif->msm_sd.sd, ispif);

	platform_set_drvdata(pdev, &ispif->msm_sd.sd);

	media_entity_init(&ispif->msm_sd.sd.entity, 0, NULL, 0);
	ispif->msm_sd.sd.entity.type = MEDIA_ENT_T_V4L2_SUBDEV;
	ispif->msm_sd.sd.entity.group_id = MSM_CAMERA_SUBDEV_ISPIF;
	ispif->msm_sd.sd.entity.name = pdev->name;
	ispif->msm_sd.close_seq = MSM_SD_CLOSE_1ST_CATEGORY | 0x1;
	rc = msm_sd_register(&ispif->msm_sd);
	if (rc) {
		pr_err("%s: msm_sd_register error = %d\n", __func__, rc);
		goto error;
	}
	msm_ispif_v4l2_subdev_fops.owner = v4l2_subdev_fops.owner;
	msm_ispif_v4l2_subdev_fops.open = v4l2_subdev_fops.open;
	msm_ispif_v4l2_subdev_fops.unlocked_ioctl = msm_ispif_subdev_fops_ioctl;
	msm_ispif_v4l2_subdev_fops.release = v4l2_subdev_fops.release;
	msm_ispif_v4l2_subdev_fops.poll = v4l2_subdev_fops.poll;
#ifdef CONFIG_COMPAT
	msm_ispif_v4l2_subdev_fops.compat_ioctl32 = msm_ispif_subdev_fops_ioctl;
#endif
	ispif->msm_sd.sd.devnode->fops = &msm_ispif_v4l2_subdev_fops;
	ispif->pdev = pdev;
	ispif->ispif_state = ISPIF_POWER_DOWN;
	ispif->open_cnt = 0;
	return 0;

error:
	mutex_destroy(&ispif->mutex);
	kfree(ispif);
	return rc;
}

static const struct of_device_id msm_ispif_dt_match[] = {
	{.compatible = "qcom,ispif"},
	{}
};

MODULE_DEVICE_TABLE(of, msm_ispif_dt_match);

static struct platform_driver ispif_driver = {
	.probe = ispif_probe,
	.driver = {
		.name = MSM_ISPIF_DRV_NAME,
		.owner = THIS_MODULE,
		.of_match_table = msm_ispif_dt_match,
	},
};

static int __init msm_ispif_init_module(void)
{
	return platform_driver_register(&ispif_driver);
}

static void __exit msm_ispif_exit_module(void)
{
	platform_driver_unregister(&ispif_driver);
}

module_init(msm_ispif_init_module);
module_exit(msm_ispif_exit_module);
MODULE_DESCRIPTION("MSM ISP Interface driver");
MODULE_LICENSE("GPL v2");
