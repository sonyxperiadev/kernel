/* drivers/video/msm/mipi_dsi_panel_driver.h
 *
 * Copyright (C) 2012-2013 Sony Mobile Communications AB.
 *
 * Author: Yosuke Hatanaka <yosuke.hatanaka@sonyericsson.com>
 * Author: Johan Olson <johan.olson@sonymobile.com>
 * Author: Joakim Wesslen <joakim.wesslen@sonymobile.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */


#ifndef MIPI_DSI_PANEL_DRIVER_H
#define MIPI_DSI_PANEL_DRIVER_H

#include <linux/types.h>
#include "mipi_dsi.h"
#include <video/mipi_dsi_panel.h>

#define ONE_FRAME_TRANSMIT_WAIT_MS 20
#define CLR_REG_DATA_NUM    2
#define CLR_RESOLUTION     60
#define CLR_NUM_PART        6
#define NVRW_NUM_E6_PARAM	8
#define NVRW_NUM_E7_PARAM	4
#define NVRW_NUM_DE_PARAM	12
#define PANEL_SKIP_ID		0xff

enum power_state {
	PANEL_OFF,
	DEBUGFS_POWER_OFF,
	DEBUGFS_POWER_ON,
	PANEL_INITIALIZED,
	PANEL_SLEEP_OUT,
	PANEL_ON
};

enum color_type {
	CLR01_GRN,
	CLR02_GRN,
	CLR03_LG,	/* light green */
	CLR04_LG,
	CLR05_YEL,
	CLR06_YEL,
	CLR07_GRN,
	CLR08_GRN,
	CLR09_LG,
	CLR10_LG,
	CLR11_YEL,
	CLR12_YEL,
	CLR13_LB,	/* light blue */
	CLR14_LB,
	CLR15_WHT,
	CLR16_ORG,
	CLR17_ORG,
	CLR18_BLE,
	CLR19_BLE,
	CLR20_PUR,
	CLR21_PUR,
	CLR22_RED,
	CLR23_RED,
	CLR24_RED,
	CLR25_RED,
};

static const enum color_type panel_color_type[CLR_NUM_PART][CLR_NUM_PART] = {
	{CLR18_BLE, CLR19_BLE, CLR21_PUR, CLR21_PUR, CLR24_RED, CLR25_RED},
	{CLR19_BLE, CLR19_BLE, CLR20_PUR, CLR20_PUR, CLR22_RED, CLR23_RED},
	{CLR13_LB,  CLR14_LB,  CLR15_WHT, CLR15_WHT, CLR16_ORG, CLR17_ORG},
	{CLR13_LB,  CLR14_LB,  CLR15_WHT, CLR15_WHT, CLR16_ORG, CLR17_ORG},
	{CLR07_GRN, CLR08_GRN, CLR09_LG,  CLR10_LG,  CLR11_YEL, CLR12_YEL},
	{CLR01_GRN, CLR02_GRN, CLR03_LG,  CLR04_LG,  CLR05_YEL, CLR06_YEL},
};

#define CLR_SUB_AREA_V_START	4
#define CLR_SUB_V_BLOCK_HEIGHT	4
#define CLR_SUB_COL_MAX		6
static const struct {
	uint8 u_min;
	uint8 u_max;
	uint8 sub_area;
}  clr_sub_tbl[][CLR_SUB_COL_MAX] = {
	{{22, 25, 50}, },			/* Area50 */
	{{22, 25, 48}, {26, 29, 49},  },	/* Area48/49 */
	{{22, 25, 45}, {26, 29, 46}, {30, 33, 47}, },
	{{22, 25, 41}, {26, 29, 42}, {30, 33, 43}, {34, 37, 44}, },
	{{22, 25, 37}, {26, 29, 38}, {30, 33, 39}, {34, 37, 40}, },
	{{22, 23, 31}, {24, 27, 32}, {28, 31, 33}, {32, 35, 34}, {36, 39, 35},
	 {40, 41, 36}, },
	{{22, 23, 26}, {24, 27, 27}, {32, 35, 28}, {36, 39, 29}, {40, 41, 30},},
	{{22, 23, 20}, {24, 27, 21}, {28, 31, 22}, {32, 35, 23}, {36, 39, 24},
	 {40, 41, 25}, },
	{{24, 27, 15}, {28, 31, 16}, {32, 35, 17}, {36, 39, 18}, {40, 41, 19},},
	{{26, 29, 11}, {30, 33, 12}, {34, 37, 13}, {38, 41, 14}, },
	{{29, 32,  7}, {33, 36,  8}, {37, 40,  9}, {41, 44, 10}, },
	{{32, 35,  4}, {36, 39,  5}, {40, 43,  6}, },
	{{35, 38,  2}, {39, 42,  3}, },
	{{39, 42,  1}, },			/* Area1 */
};

struct mipi_dsi_data {
	struct dsi_buf tx_buf;
	struct dsi_buf rx_buf;
	struct msm_fb_panel_data panel_data;
	const struct panel *panel;
	const struct panel **panels;
	int (*lcd_power)(bool on);
	int (*lcd_reset)(bool on);
#ifdef CONFIG_DEBUG_FS
	struct dentry *dir;
#endif
	bool panel_detected;
	enum power_state panel_state;
	struct mutex lock;
	struct mdp_pcc_cfg_data *pcc_config;

	bool	nvrw_panel_detective;
	int	nvrw_result;
	int	nvrw_retry_cnt;
	void	*nvrw_private;

	int	(*override_nvm_data)(struct msm_fb_data_type *mfd,
					const char *buf, int count);
	int	(*seq_nvm_read)(struct msm_fb_data_type *mfd, char *buf);
	int	(*seq_nvm_erase)(struct msm_fb_data_type *mfd);
	int	(*seq_nvm_rsp_write)(struct msm_fb_data_type *mfd);
	int	(*seq_nvm_user_write)(struct msm_fb_data_type *mfd);
	int	(*dsi_power_save) (int on);
};
extern struct mdp_pcc_cfg_data *pcc_cfg_ptr;

void mipi_dsi_panel_fps_data_update(struct msm_fb_data_type *mfd);
#ifdef CONFIG_DEBUG_FS
extern void mipi_dsi_panel_create_debugfs(struct platform_device *pdev);
extern void mipi_dsi_panel_remove_debugfs(struct platform_device *pdev);
#endif
int prepare_for_reg_access(struct msm_fb_data_type *mfd,
				enum power_state *old_state);
void post_reg_access(struct msm_fb_data_type *mfd, enum power_state old_state);
#endif /* MIPI_DSI_PANEL_DRIVER_H */
