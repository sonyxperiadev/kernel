/* Copyright (c) 2012, Code Aurora Forum. All rights reserved.
 * Copyright (C) 2012 Sony Mobile Communications AB.
 *
 * Author: johan.olson@sonymobile.com
 * Author: joakim.wesslen@sonymobile.com
 * Author: perumal.jayamani@sonymobile.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/init.h>
#include <linux/ioport.h>
#include <linux/gpio.h>
#include <linux/platform_device.h>
#include <linux/bootmem.h>
#include <linux/msm_ion.h>
#include <asm/mach-types.h>
#include <mach/msm_memtypes.h>
#include <mach/board.h>
#include <mach/gpiomux.h>
#include <mach/ion.h>
#include <mach/msm_bus_board.h>
#include <mach/socinfo.h>
#include <video/mipi_dsi_panel.h>

#ifdef CONFIG_FB_MSM_MHL_SII8334
#include <linux/mhl_sii8334.h>
#endif

#include "devices.h"
#include "board-8064.h"

#ifdef CONFIG_FB_MSM_TRIPLE_BUFFER
/* prim = 1280 x 720 x 4(bpp) x 3(pages) */
#define MSM_FB_PRIM_BUF_SIZE roundup(1280 * 736 * 4 * 3, 0x10000)
#else
/* prim = 1280 x 720 x 4(bpp) x 2(pages) */
#define MSM_FB_PRIM_BUF_SIZE roundup(1280 * 736 * 4 * 2, 0x10000)
#endif

#define MSM_FB_SIZE roundup(MSM_FB_PRIM_BUF_SIZE, 4096)

#ifdef CONFIG_FB_MSM_OVERLAY0_WRITEBACK
#define MSM_FB_OVERLAY0_WRITEBACK_SIZE roundup((1280 * 720 * 3 * 2), 4096)
#else
#define MSM_FB_OVERLAY0_WRITEBACK_SIZE (0)
#endif  /* CONFIG_FB_MSM_OVERLAY0_WRITEBACK */

#ifdef CONFIG_FB_MSM_OVERLAY1_WRITEBACK
#define MSM_FB_OVERLAY1_WRITEBACK_SIZE roundup((1920 * 1088 * 3 * 2), 4096)
#else
#define MSM_FB_OVERLAY1_WRITEBACK_SIZE (0)
#endif  /* CONFIG_FB_MSM_OVERLAY1_WRITEBACK */


static struct resource msm_fb_resources[] = {
	{
		.flags = IORESOURCE_DMA,
	}
};

#define LVDS_CHIMEI_PANEL_NAME "lvds_chimei_wxga"
#define LVDS_FRC_PANEL_NAME "lvds_frc_fhd"
#define MIPI_VIDEO_TOSHIBA_WSVGA_PANEL_NAME "mipi_video_toshiba_wsvga"
#define MIPI_VIDEO_CHIMEI_WXGA_PANEL_NAME "mipi_video_chimei_wxga"
#define MIPI_VIDEO_RENESAS_FWVGA_PANEL_NAME "mipi_video_renesas_fwvga"
#define HDMI_PANEL_NAME "hdmi_msm"
#define TVOUT_PANEL_NAME "tvout_msm"

#define LVDS_PIXEL_MAP_PATTERN_1	1
#define LVDS_PIXEL_MAP_PATTERN_2	2

#ifdef CONFIG_FB_MSM_HDMI_AS_PRIMARY
static unsigned char hdmi_is_primary = 1;
#else
static unsigned char hdmi_is_primary;
#endif

unsigned char apq8064_hdmi_as_primary_selected(void)
{
	return hdmi_is_primary;
}

static void set_mdp_clocks_for_wuxga(void);

static int msm_fb_detect_panel(const char *name)
{
	pr_err("%s: comparing name=%s with %s\n", __func__, name,
			MIPI_VIDEO_RENESAS_FWVGA_PANEL_NAME);

	if (!strncmp(name, MIPI_VIDEO_RENESAS_FWVGA_PANEL_NAME,
			strnlen(MIPI_VIDEO_RENESAS_FWVGA_PANEL_NAME,
				PANEL_NAME_MAX_LEN))) {
		return 0;
	}

	if (!strncmp(name, HDMI_PANEL_NAME,
		strnlen(HDMI_PANEL_NAME,
			PANEL_NAME_MAX_LEN)))
		return 0;

	return -ENODEV;
}

static struct msm_fb_platform_data msm_fb_pdata = {
	.detect_client = msm_fb_detect_panel,
};

static struct platform_device msm_fb_device = {
	.name              = "msm_fb",
	.id                = 0,
	.num_resources     = ARRAY_SIZE(msm_fb_resources),
	.resource          = msm_fb_resources,
	.dev.platform_data = &msm_fb_pdata,
};

void __init apq8064_allocate_fb_region(void)
{
	void *addr;
	unsigned long size;

	size = MSM_FB_SIZE;
	addr = alloc_bootmem_align(size, 0x1000);
	msm_fb_resources[0].start = __pa(addr);
	msm_fb_resources[0].end = msm_fb_resources[0].start + size - 1;
	pr_info("allocating %lu bytes at %p (%lx physical) for fb\n",
			size, addr, __pa(addr));
}

#define MDP_VSYNC_GPIO 0

static struct msm_bus_vectors mdp_init_vectors[] = {
	{
		.src = MSM_BUS_MASTER_MDP_PORT0,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab = 0,
		.ib = 0,
	},
};

static struct msm_bus_vectors mdp_ui_vectors[] = {
	{
		.src = MSM_BUS_MASTER_MDP_PORT0,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab = 216000000 * 2,
		.ib = 270000000 * 2,
	},
};

static struct msm_bus_vectors mdp_vga_vectors[] = {
	/* VGA and less video */
	{
		.src = MSM_BUS_MASTER_MDP_PORT0,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab = 216000000 * 2,
		.ib = 270000000 * 2,
	},
};

static struct msm_bus_vectors mdp_720p_vectors[] = {
	/* 720p and less video */
	{
		.src = MSM_BUS_MASTER_MDP_PORT0,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab = 230400000 * 2,
		.ib = 288000000 * 2,
	},
};

static struct msm_bus_vectors mdp_1080p_vectors[] = {
	/* 1080p and less video */
	{
		.src = MSM_BUS_MASTER_MDP_PORT0,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab = 334080000 * 2,
		.ib = 417600000 * 2,
	},
};

static struct msm_bus_paths mdp_bus_scale_usecases[] = {
	{
		ARRAY_SIZE(mdp_init_vectors),
		mdp_init_vectors,
	},
	{
		ARRAY_SIZE(mdp_ui_vectors),
		mdp_ui_vectors,
	},
	{
		ARRAY_SIZE(mdp_ui_vectors),
		mdp_ui_vectors,
	},
	{
		ARRAY_SIZE(mdp_vga_vectors),
		mdp_vga_vectors,
	},
	{
		ARRAY_SIZE(mdp_720p_vectors),
		mdp_720p_vectors,
	},
	{
		ARRAY_SIZE(mdp_1080p_vectors),
		mdp_1080p_vectors,
	},
};

static struct msm_bus_scale_pdata mdp_bus_scale_pdata = {
	mdp_bus_scale_usecases,
	ARRAY_SIZE(mdp_bus_scale_usecases),
	.name = "mdp",
};

static struct msm_panel_common_pdata mdp_pdata = {
	.gpio = MDP_VSYNC_GPIO,
	.mdp_max_clk = 266667000,
	.mdp_max_bw = 2000000000,
	.mdp_bw_ab_factor = 115,
	.mdp_bw_ib_factor = 205,
	.mdp_bus_scale_table = &mdp_bus_scale_pdata,
	.mdp_rev = MDP_REV_44,
#ifdef CONFIG_MSM_MULTIMEDIA_USE_ION
	.mem_hid = BIT(ION_CP_MM_HEAP_ID),
#else
	.mem_hid = MEMTYPE_EBI1,
#endif
	.mdp_iommu_split_domain = 1,
};

void __init apq8064_mdp_writeback(struct memtype_reserve *reserve_table)
{
	mdp_pdata.ov0_wb_size = MSM_FB_OVERLAY0_WRITEBACK_SIZE;
	mdp_pdata.ov1_wb_size = MSM_FB_OVERLAY1_WRITEBACK_SIZE;
#if defined(CONFIG_ANDROID_PMEM) && !defined(CONFIG_MSM_MULTIMEDIA_USE_ION)
	reserve_table[mdp_pdata.mem_hid].size +=
		mdp_pdata.ov0_wb_size;
	reserve_table[mdp_pdata.mem_hid].size +=
		mdp_pdata.ov1_wb_size;
#endif
}

static struct resource hdmi_msm_resources[] = {
	{
		.name  = "hdmi_msm_qfprom_addr",
		.start = 0x00700000,
		.end   = 0x007060FF,
		.flags = IORESOURCE_MEM,
	},
	{
		.name  = "hdmi_msm_hdmi_addr",
		.start = 0x04A00000,
		.end   = 0x04A00FFF,
		.flags = IORESOURCE_MEM,
	},
	{
		.name  = "hdmi_msm_irq",
		.start = HDMI_IRQ,
		.end   = HDMI_IRQ,
		.flags = IORESOURCE_IRQ,
	},
};

static int hdmi_enable_5v(int on);
static int hdmi_core_power(int on, int show);
static int hdmi_cec_power(int on);
static int hdmi_gpio_config(int on);
static int hdmi_panel_power(int on);

static struct msm_hdmi_platform_data hdmi_msm_data = {
	.irq = HDMI_IRQ,
#ifdef CONFIG_FB_MSM_MHL_SII8334
	.coupled_mhl_device = SII_DEV_NAME,
#endif
	.enable_5v = hdmi_enable_5v,
	.core_power = hdmi_core_power,
	.cec_power = hdmi_cec_power,
	.panel_power = hdmi_panel_power,
	.gpio_config = hdmi_gpio_config,
};

static struct platform_device hdmi_msm_device = {
	.name = "hdmi_msm",
	.id = 0,
	.num_resources = ARRAY_SIZE(hdmi_msm_resources),
	.resource = hdmi_msm_resources,
	.dev.platform_data = &hdmi_msm_data,
};

static char wfd_check_mdp_iommu_split_domain(void)
{
	return mdp_pdata.mdp_iommu_split_domain;
}

#ifdef CONFIG_FB_MSM_WRITEBACK_MSM_PANEL
static struct msm_wfd_platform_data wfd_pdata = {
	.wfd_check_mdp_iommu_split = wfd_check_mdp_iommu_split_domain,
};

static struct platform_device wfd_panel_device = {
	.name = "wfd_panel",
	.id = 0,
	.dev.platform_data = NULL,
};

static struct platform_device wfd_device = {
	.name          = "msm_wfd",
	.id            = -1,
	.dev.platform_data = &wfd_pdata,
};
#endif

/* HDMI related GPIOs */
#define HDMI_CEC_VAR_GPIO	69
#define HDMI_DDC_CLK_GPIO	70
#define HDMI_DDC_DATA_GPIO	71
#define HDMI_HPD_GPIO		72

#define MLCD_RESET_N 25
#define LCD_DCDC_EN  24

static int lcd_reset_gpio;
static int lcd_dcdc_en_gpio;
static bool dsi_power_on;
static struct regulator *reg_l29, *reg_l2;
static struct regulator *reg_l11;

static int mipi_dsi_panel_plf_init(void)
{
	int rc = 0;

	pr_debug("%s\n", __func__);

	if (!dsi_power_on) {
		reg_l29 = regulator_get(&msm_mipi_dsi1_device.dev,
								"dsi1_vddio");
		if (IS_ERR_OR_NULL(reg_l29)) {
			pr_err("%s: could not get l29 dsi1_vddio, rc = %ld\n",
						__func__, PTR_ERR(reg_l29));
			return -ENODEV;
		}
		reg_l11 = regulator_get(&msm_mipi_dsi1_device.dev, "dsi1_avdd");
		if (IS_ERR_OR_NULL(reg_l11)) {
			pr_err("%s: could not get l11 dsi1_avdd, rc = %ld\n",
						__func__, PTR_ERR(reg_l11));
			rc = -ENODEV;
			goto put_l29;
		}
		rc = regulator_set_voltage(reg_l11, 2850000, 2850000);
		if (rc) {
			pr_err("%s: set_voltage l11 dsi1_avdd failed, rc=%d\n",
								__func__, rc);
			goto put_l29_l11;
		}

		reg_l2 = regulator_get(&msm_mipi_dsi1_device.dev,
							"dsi1_pll_vdda");
		if (IS_ERR_OR_NULL(reg_l2)) {
			pr_err("%s:could not get l2 dsi1_pll_vdda, rc = %ld\n",
						__func__, PTR_ERR(reg_l2));
			rc = -ENODEV;
			goto put_l29_l11;
		}

		rc = regulator_set_voltage(reg_l29, 1800000, 1800000);
		if (rc) {
			pr_err("%s: set_voltage l29 dsi1_vddio failed, rc=%d\n",
								__func__, rc);
			goto put_l29_l11_l2;
		}
		rc = regulator_set_voltage(reg_l2, 1200000, 1200000);
		if (rc) {
			pr_err("%s: set_voltage l2 dsi1_pll_vdda failed, rc=%d\n",
					__func__, rc);
			goto put_l29_l11_l2;
		}

		lcd_dcdc_en_gpio = PM8921_GPIO_PM_TO_SYS(LCD_DCDC_EN);
		rc = gpio_request(lcd_dcdc_en_gpio, "lcd_dcdc_en");
		if (rc) {
			pr_err("%s: request LCD_DCDC_EN (gpio %d) failed, rc=%d\n",
					__func__, LCD_DCDC_EN, rc);
			goto put_l29_l11_l2;
		}

		lcd_reset_gpio = PM8921_GPIO_PM_TO_SYS(MLCD_RESET_N);
		rc = gpio_request(lcd_reset_gpio, "mlcd_reset_n");
		if (rc) {
			pr_err("%s:request MLCD_RESET_N (gpio %d) failed, rc=%d\n",
					__func__, MLCD_RESET_N, rc);
			goto put_l29_l11_l2;
		}
		dsi_power_on = true;
	}

	return 0;
put_l29_l11_l2:
	regulator_put(reg_l2);
put_l29_l11:
	regulator_put(reg_l11);
put_l29:
	regulator_put(reg_l29);
	return rc;
}

static int mipi_dsi_panel_plf_reset(bool high)
{
	pr_debug("%s: high = %d\n", __func__, high);
	gpio_set_value_cansleep(lcd_reset_gpio, high);

	return 0;
}

static int mipi_dsi_panel_plf_power(int on)
{
	int rc = 0;

	pr_debug("%s: power_on=%d\n", __func__, on);

	rc = mipi_dsi_panel_plf_init();

	if (on) {
		pr_debug("%s: Power On\n", __func__);

		rc = regulator_enable(reg_l11);	/* Enable dsi1_avdd */
		if (rc) {
			pr_err("%s: enable reg_l11 failed, rc=%d\n",
								__func__, rc);
			goto disable_none;
		}
		rc = regulator_set_optimum_mode(reg_l29, 1000);
		if (rc < 0) {
			pr_err("%s: set_optimum_mode l29 failed, rc=%d\n",
								__func__, rc);
			goto disable_l11;
		}
		rc = regulator_enable(reg_l29);		/* Enable dsi1_vddio */
		if (rc) {
			pr_err("%s: enable reg_l29 failed, rc=%d\n",
								__func__, rc);
			goto disable_l11;
		}
		usleep_range(11000, 12000);		/* Spec says > 10 ms */

		rc = regulator_set_optimum_mode(reg_l2, 135000);
		if (rc < 0) {
			pr_err("%s: set_optimum_mode l2 failed, rc=%d\n",
								__func__, rc);
			goto disable_l29_l11;
		}
		rc = regulator_enable(reg_l2);		/* Enable dsi-vdda */
		if (rc) {
			pr_err("%s: enable l2 failed, rc=%d\n", __func__, rc);
			goto disable_l29_l11;
		}
		mipi_dsi_panel_plf_reset(0);		/* Reset LOW */
		usleep_range(6000, 7000);		/* Spec says > 5 ms */
		mipi_dsi_panel_plf_reset(1);		/* Reset HIGH */
		usleep_range(11000, 12000);		/* Spec says > 10 ms */
							/* Enable VSP/VSN */
		gpio_set_value_cansleep(lcd_dcdc_en_gpio, 1);
		usleep_range(11000, 12000);		/* Spec says > 10 ms */
	} else {
		pr_debug("%s: Power Off\n", __func__);
							/* Disable VSP/VSN */
		gpio_set_value_cansleep(lcd_dcdc_en_gpio, 0);
		msleep(20);				/* Spec says > 20 ms) */
		gpio_set_value_cansleep(lcd_reset_gpio, 0); /* Pull RESET Low */
		rc = regulator_disable(reg_l2);		/* Disable dsi-vdda */
		if (rc)
			pr_err("%s: disable l2 failed, rc=%d\n", __func__, rc);

		rc = regulator_disable(reg_l29);	/* Disable dsi1_vddio */
		if (rc)
			pr_err("%s: disable l29 failed, rc=%d\n", __func__, rc);

		rc = regulator_disable(reg_l11); /* Disable dsi1_avdd */
		if (rc)
			pr_err("%s: disable l11 failed, rc=%d\n",
								__func__, rc);
	}
	return 0;

disable_l29_l11:
	rc = regulator_disable(reg_l29);
	if (rc)
		pr_err("%s: disable l29 failed, rc=%d\n", __func__, rc);
disable_l11:
	rc = regulator_disable(reg_l11);
	if (rc)
		pr_err("disable reg_l11 failed, rc=%d\n", rc);
disable_none:
	return rc;
}

static struct platform_device mipi_dsi_panel_device = {
	.name = MIPI_DSI_PANEL_NAME,
	.id = 0,
};

static const struct panel *panels[] = {
#ifdef CONFIG_FB_MSM_MIPI_R63311_SHARP_LS050T3SX02
	&sharp_ls050t3sx02_r63311_rev_1a,
	&sharp_ls050t3sx02_r63311_rev_1b,
	&sharp_ls050t3sx02_r63311_rev_1c,
	&sharp_ls050t3sx02_r63311_no_rev,
	&sharp_ls050t3sx02_r63311_12624791_rev_1a,
	&sharp_ls050t3sx02_r63311_12624791_no_rev,
	/* TODO: Remove this Yuga panel when it doesn't need to be supported
	   in the Odin project anymore */
	&sharp_ls050t3sx01_r63311,
#endif /* CONFIG_FB_MSM_MIPI_R63311_SHARP_LS050T3SX02 */
#ifdef CONFIG_FB_MSM_MIPI_R63306_SHARP_LS043K3SX04
	&sharp_ls043k3sx04_panel_id_1a,
	&sharp_ls043k3sx04_panel_id,
#endif /* CONFIG_FB_MSM_MIPI_R63306_SHARP_LS043K3SX04 */
#ifdef CONFIG_FB_MSM_MIPI_R63306_SHARP_LS046K3SX01
	&sharp_ls046k3sx01_panel_tovis_id,
	&sharp_ls046k3sx01_panel_id_1a,
	&sharp_ls046k3sx01_panel_id,
#endif /* CONFIG_FB_MSM_MIPI_R63306_SHARP_LS046K3SX01 */
#ifdef CONFIG_FB_MSM_MIPI_R63306_TMD_MDW30
	&tmd_mdw30_panel_id_old,
	&tmd_mdw30_panel_id_1a,
	&tmd_mdw30_panel_id_1c,
	&tmd_mdw30_panel_id_1e,
	&tmd_mdw30_panel_id,
#endif /* CONFIG_FB_MSM_MIPI_R63306_TMD_MDW30 */
#ifdef CONFIG_FB_MSM_MIPI_R63311_JDC_MDY70
	&jdc_mdy70_panel_id_1a,
	&jdc_mdy70_panel_id,
#endif /* CONFIG_FB_MSM_MIPI_R63311_JDC_MDY70 */
#ifdef CONFIG_FB_MSM_MIPI_R63311_JDC_MDY80
	&jdc_mdy80_panel_id_1a,
	&jdc_mdy80_panel_id,
#endif /* CONFIG_FB_MSM_MIPI_R63311_JDC_MDY80 */
#ifdef CONFIG_FB_MSM_MIPI_R63311_SHARP_LS050T3SX01
	&sharp_ls050t3sx01_panel_id_1a,
	&sharp_ls050t3sx01_panel_id,
#endif /* CONFIG_FB_MSM_MIPI_R63311_SHARP_LS050T3SX01 */
#ifdef CONFIG_FB_MSM_MIPI_R63306_JDC_MDZ50
	&jdc_mdz50_panel_id_1a,
	&jdc_mdz50_panel_id_1b,
	&jdc_mdz50_panel_id,
	&jdc_mdz50_panel_id_nvm,
#endif /* CONFIG_FB_MSM_MIPI_R63306_JDC_MDZ50 */
	NULL,
};

static void __init mipi_dsi_panel_add_device(void)
{
	int rc;
	struct panel_platform_data *pdata;

	pdata = kmalloc(sizeof(struct panel_platform_data), GFP_KERNEL);

	pdata->platform_power = NULL;
	pdata->platform_reset = mipi_dsi_panel_plf_reset;
	pdata->panels = panels;

	mipi_dsi_panel_device.dev.platform_data = pdata;
	rc = platform_device_register(&mipi_dsi_panel_device);
	if (rc)
		dev_err(&mipi_dsi_panel_device.dev,
			"%s: platform_device_register() failed = %d\n",
			__func__, rc);
}

static int dogo_dsi_power_save(int on)
{
	static int display_power_on;
	static int skip_first_off = 1;
	int ret = 0;

	/* To speed up the first image shown on screen after startup,
	 * we do not shut off the display at first "off" call */
	if (on) {
		if (!display_power_on) {
			ret = mipi_dsi_panel_plf_power(1);
			if (ret)
				goto exit;
			display_power_on = 1;
		}
	} else {
		if (skip_first_off) {
			skip_first_off = 0;
		} else {
			ret = mipi_dsi_panel_plf_power(0);
			if (ret)
				goto exit;
			display_power_on = 0;
		}
	}
exit:
	return ret;
}

static struct mipi_dsi_platform_data mipi_dsi_pdata = {
	.dsi_power_save = dogo_dsi_power_save,
	};

static struct msm_bus_vectors dtv_bus_init_vectors[] = {
	{
		.src = MSM_BUS_MASTER_MDP_PORT0,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab = 0,
		.ib = 0,
	},
};

static struct msm_bus_vectors dtv_bus_def_vectors[] = {
	{
		.src = MSM_BUS_MASTER_MDP_PORT0,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab = 566092800 * 2,
		.ib = 707616000 * 2,
	},
};

static struct msm_bus_paths dtv_bus_scale_usecases[] = {
	{
		ARRAY_SIZE(dtv_bus_init_vectors),
		dtv_bus_init_vectors,
	},
	{
		ARRAY_SIZE(dtv_bus_def_vectors),
		dtv_bus_def_vectors,
	},
};
static struct msm_bus_scale_pdata dtv_bus_scale_pdata = {
	dtv_bus_scale_usecases,
	ARRAY_SIZE(dtv_bus_scale_usecases),
	.name = "dtv",
};

static struct lcdc_platform_data dtv_pdata = {
	.bus_scale_table = &dtv_bus_scale_pdata,
	.lcdc_power_save = hdmi_panel_power,
};

static int hdmi_panel_power(int on)
{
	int rc;

	pr_debug("%s: HDMI Core: %s\n", __func__, (on ? "ON" : "OFF"));
	rc = hdmi_core_power(on, 1);
	if (rc)
		rc = hdmi_cec_power(on);

	pr_debug("%s: HDMI Core: %s Success\n", __func__, (on ? "ON" : "OFF"));
	return rc;
}

static int hdmi_enable_5v(int on)
{
	/* Powering external dongle is not supported at the moment,
	 * "hdmi_mvs" regulator is not used/enabled.
	 */

	return 0;
}

static int hdmi_core_power(int on, int show)
{
	static struct regulator *reg_8921_lvs7, *reg_8921_s4;
	static int prev_on;
	int rc;

	if (on == prev_on)
		return 0;

	if (!reg_8921_lvs7) {
		reg_8921_lvs7 = regulator_get(&hdmi_msm_device.dev,
					      "hdmi_vdda");
		if (IS_ERR(reg_8921_lvs7)) {
			pr_err("could not get 8921_lvs7, rc = %ld\n",
				PTR_ERR(reg_8921_lvs7));
			reg_8921_lvs7 = NULL;
			return -ENODEV;
		}
	}
	if (!reg_8921_s4) {
		reg_8921_s4 = regulator_get(&hdmi_msm_device.dev,
					    "hdmi_lvl_tsl");
		if (IS_ERR(reg_8921_s4)) {
			pr_err("could not get 8921_s4, rc = %ld\n",
				PTR_ERR(reg_8921_s4));
			reg_8921_s4 = NULL;
			return -ENODEV;
		}
		rc = regulator_set_voltage(reg_8921_s4, 1800000, 1800000);
		if (rc) {
			pr_err("set_voltage failed for 8921_s4, rc=%d\n", rc);
			return -EINVAL;
		}
	}

	if (on) {
		rc = regulator_enable(reg_8921_lvs7);
		if (rc) {
			pr_err("8921_lvs7 regulator enable failed, rc=%d\n",
				rc);
			return -ENODEV;
		}
		rc = regulator_enable(reg_8921_s4);
		if (rc) {
			pr_err("8921_s4 regulator enable failed, rc=%d\n",
				rc);
			goto error1;
		}
		pr_debug("%s(on): success\n", __func__);
	} else {
		rc = regulator_disable(reg_8921_lvs7);
		if (rc) {
			pr_err("disable 8921_lvs7 failed, rc=%d\n", rc);
			return -ENODEV;
		}
		rc = regulator_disable(reg_8921_s4);
		if (rc) {
			pr_err("disable 8921_s4 failed, rc=%d\n", rc);
			return -ENODEV;
		}
		pr_debug("%s(off): success\n", __func__);
	}

	prev_on = on;

	return 0;

error1:
	regulator_disable(reg_8921_lvs7);
	return rc;
}

static int hdmi_gpio_config(int on)
{
	int rc = 0;
	static int prev_on;
	int pmic_gpio14 = PM8921_GPIO_PM_TO_SYS(14);

	if (on == prev_on)
		return 0;

	if (on) {
		rc = gpio_request(HDMI_DDC_CLK_GPIO, "HDMI_DDC_CLK");
		if (rc) {
			pr_err("'%s'(%d) gpio_request failed, rc=%d\n",
				"HDMI_DDC_CLK", HDMI_DDC_CLK_GPIO, rc);
			goto error1;
		}
		rc = gpio_request(HDMI_DDC_DATA_GPIO, "HDMI_DDC_DATA");
		if (rc) {
			pr_err("'%s'(%d) gpio_request failed, rc=%d\n",
				"HDMI_DDC_DATA", HDMI_DDC_DATA_GPIO, rc);
			goto error2;
		}
		rc = gpio_request(HDMI_HPD_GPIO, "HDMI_HPD");
		if (rc) {
			pr_err("'%s'(%d) gpio_request failed, rc=%d\n",
				"HDMI_HPD", HDMI_HPD_GPIO, rc);
			goto error3;
		}
		if (machine_is_apq8064_liquid()) {
			rc = gpio_request(pmic_gpio14, "PMIC_HDMI_MUX_SEL");
			if (rc) {
				pr_err("'%s'(%d) gpio_request failed, rc=%d\n",
					"PMIC_HDMI_MUX_SEL", 14, rc);
				goto error4;
			}
			gpio_set_value_cansleep(pmic_gpio14, 0);
		}
		pr_debug("%s(on): success\n", __func__);
	} else {
		gpio_free(HDMI_DDC_CLK_GPIO);
		gpio_free(HDMI_DDC_DATA_GPIO);
		gpio_free(HDMI_HPD_GPIO);

		if (machine_is_apq8064_liquid()) {
			gpio_set_value_cansleep(pmic_gpio14, 1);
			gpio_free(pmic_gpio14);
		}
		pr_debug("%s(off): success\n", __func__);
	}

	prev_on = on;
	return 0;

error4:
	gpio_free(HDMI_HPD_GPIO);
error3:
	gpio_free(HDMI_DDC_DATA_GPIO);
error2:
	gpio_free(HDMI_DDC_CLK_GPIO);
error1:
	return rc;
}

static int hdmi_cec_power(int on)
{
	static int prev_on;
	int rc;

	if (on == prev_on)
		return 0;

	if (on) {
		rc = gpio_request(HDMI_CEC_VAR_GPIO, "HDMI_CEC_VAR");
		if (rc) {
			pr_err("'%s'(%d) gpio_request failed, rc=%d\n",
				"HDMI_CEC_VAR", HDMI_CEC_VAR_GPIO, rc);
			goto error;
		}
		pr_debug("%s(on): success\n", __func__);
	} else {
		gpio_free(HDMI_CEC_VAR_GPIO);
		pr_debug("%s(off): success\n", __func__);
	}

	prev_on = on;

	return 0;
error:
	return rc;
}

void __init apq8064_init_fb(void)
{
	mipi_dsi_panel_add_device();
	platform_device_register(&msm_fb_device);

#ifdef CONFIG_FB_MSM_WRITEBACK_MSM_PANEL
	platform_device_register(&wfd_panel_device);
	platform_device_register(&wfd_device);
#endif

	msm_fb_register_device("mdp", &mdp_pdata);
	msm_fb_register_device("mipi_dsi", &mipi_dsi_pdata);
	platform_device_register(&hdmi_msm_device);
	msm_fb_register_device("dtv", &dtv_pdata);
}

/**
 * Set MDP clocks to high frequency to avoid DSI underflow
 * when using high resolution 1200x1920 WUXGA panels
 */
static void set_mdp_clocks_for_wuxga(void)
{
	mdp_ui_vectors[0].ab = 2000000000;
	mdp_ui_vectors[0].ib = 2000000000;
	mdp_vga_vectors[0].ab = 2000000000;
	mdp_vga_vectors[0].ib = 2000000000;
	mdp_720p_vectors[0].ab = 2000000000;
	mdp_720p_vectors[0].ib = 2000000000;
	mdp_1080p_vectors[0].ab = 2000000000;
	mdp_1080p_vectors[0].ib = 2000000000;

	if (apq8064_hdmi_as_primary_selected()) {
		dtv_bus_def_vectors[0].ab = 2000000000;
		dtv_bus_def_vectors[0].ib = 2000000000;
	}
}

void __init apq8064_set_display_params(char *prim_panel, char *ext_panel,
		unsigned char resolution)
{
	if (strnlen(prim_panel, PANEL_NAME_MAX_LEN)) {
		strlcpy(msm_fb_pdata.prim_panel_name, prim_panel,
			PANEL_NAME_MAX_LEN);
		pr_debug("msm_fb_pdata.prim_panel_name %s\n",
			msm_fb_pdata.prim_panel_name);

		if (!strncmp((char *)msm_fb_pdata.prim_panel_name,
			HDMI_PANEL_NAME, strnlen(HDMI_PANEL_NAME,
				PANEL_NAME_MAX_LEN))) {
			pr_debug("HDMI is the primary display by boot parameter\n");
			hdmi_is_primary = 1;
			set_mdp_clocks_for_wuxga();
		}
	}
	if (strnlen(ext_panel, PANEL_NAME_MAX_LEN)) {
		strlcpy(msm_fb_pdata.ext_panel_name, ext_panel,
			PANEL_NAME_MAX_LEN);
		pr_debug("msm_fb_pdata.ext_panel_name %s\n",
			msm_fb_pdata.ext_panel_name);
	}

	msm_fb_pdata.ext_resolution = resolution;
}
