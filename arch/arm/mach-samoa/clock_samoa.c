/*****************************************************************************
*
* Samoa-specific clock framework
*
* Copyright 2011 Broadcom Corporation.  All rights reserved.
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2, available at
* http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a
* license other than the GPL, without Broadcom's express prior written
* consent.
*****************************************************************************/

#include <linux/math64.h>
#include <linux/delay.h>
#include <plat/clock.h>
#include <mach/io_map.h>
#include <asm/io.h>

#include <mach/rdb/brcm_rdb_sysmap.h>
#include <mach/rdb/brcm_rdb_khub_clk_mgr_reg.h>
#include <mach/rdb/brcm_rdb_kps_clk_mgr_reg.h>
#include <mach/rdb/brcm_rdb_kpm_clk_mgr_reg.h>
#include <mach/rdb/brcm_rdb_root_clk_mgr_reg.h>
#include <mach/rdb/brcm_rdb_khubaon_clk_mgr_reg.h>
#include <mach/rdb/brcm_rdb_mm_clk_mgr_reg.h>
#include <mach/rdb/brcm_rdb_kproc_clk_mgr_reg.h>
#include <mach/rdb/brcm_rdb_pwrmgr.h>

#include <linux/debugfs.h>
#include <linux/seq_file.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/string.h>
#include <linux/clk.h>
#include <linux/spinlock.h>
#include <asm/io.h>


#define	clksamoa_dbg(format...)			printk(KERN_ALERT format);

static int arm_reg_baseaddr = 0;
static int arm_reg_offsetaddr = 0;
static struct dentry *dent_pmtest_root_dir;
				 
#if 0   /* ssg, bringup */
/*****************************************************************************
	Proc clocks
*****************************************************************************/
static struct proc_clock arm_clk = {
	.clk	=	{
		.name	=	"arm_clk",
		.rate	=	700*CLOCK_1M,
		.div	=	1,
		.id	=	-1,
		.ops	=	&proc_clk_ops,
	},
	.proc_clk_mgr_base = PROC_CLK_BASE_ADDR,
};

/*****************************************************************************
	Reference clocks
*****************************************************************************/
DECLARE_REF_CLK (crystal,	CRYSTAL, 			26*CLOCK_1M,	1,	0);
DECLARE_REF_CLK (frac_1m,	FRAC_1M,			1*CLOCK_1M,		1,	0);
DECLARE_REF_CLK (ref_96m_varVDD,	REF_96M_VARVDD,		96*CLOCK_1M,	1,	0);
DECLARE_REF_CLK (var_96m,	VAR_96M,			96*CLOCK_1M,	1,	0);
DECLARE_REF_CLK (ref_96m,	REF_96M,			96*CLOCK_1M,	1,	0);
DECLARE_REF_CLK (var_500m,	VAR_500M,	 		500*CLOCK_1M,	1,	0);
DECLARE_REF_CLK (var_500m_varVDD,	VAR_500M_VARVDD,		500*CLOCK_1M,	1,	0);
DECLARE_REF_CLK (ref_1m,	REF_1M,		 		1*CLOCK_1M,	1,	0);
DECLARE_REF_CLK (ref_32k,	REF_32K, 			32*CLOCK_1K,	1,	0);
DECLARE_REF_CLK (bbl_32k,	BBL_32K, 			32*CLOCK_1K,	1,	0);
DECLARE_REF_CLK (misc_32k,	MISC_32K,			32*CLOCK_1K,	1,	0);

DECLARE_REF_CLK (ref_312m,	REF_312M,			312*CLOCK_1M,	0,	0);
DECLARE_REF_CLK (ref_208m,	REF_208M,			208*CLOCK_1M,	0,	name_to_clk(ref_312m));
DECLARE_REF_CLK (ref_156m,	REF_156M,			156*CLOCK_1M,	2,	name_to_clk(ref_312m));
DECLARE_REF_CLK (ref_104m,	REF_104M,			104*CLOCK_1M,	3,	name_to_clk(ref_312m));
DECLARE_REF_CLK (ref_52m,	REF_52M,			52*CLOCK_1M,	2,	name_to_clk(ref_104m));
DECLARE_REF_CLK (ref_13m,	REF_13M,			13*CLOCK_1M,	4,	name_to_clk(ref_52m));
DECLARE_REF_CLK (ref_26m,	REF_26M,			26*CLOCK_1M,	6,	name_to_clk(ref_156m));

DECLARE_REF_CLK (var_312m,	VAR_312M,			312*CLOCK_1M,	0,	0);
DECLARE_REF_CLK (var_208m,	VAR_208M,			208*CLOCK_1M,	0,	name_to_clk(var_312m));
DECLARE_REF_CLK (var_156m,	VAR_156M,			156*CLOCK_1M,	2,	name_to_clk(var_312m));
DECLARE_REF_CLK (var_104m,	VAR_104M,			104*CLOCK_1M,	3,	name_to_clk(var_312m));
DECLARE_REF_CLK (var_52m,	VAR_52M,			52*CLOCK_1M,	2,	name_to_clk(var_104m));
DECLARE_REF_CLK (var_13m,	VAR_13M,			13*CLOCK_1M,	4,	name_to_clk(var_52m));

DECLARE_REF_CLK (dft_19_5m,	DFT_19_5M,			19500*CLOCK_1K,	16, 	name_to_clk(ref_312m));
DECLARE_REF_CLK	(usbh_48m,	USBH_48M,			48*CLOCK_1M,	1,	0);
DECLARE_REF_CLK	(ref_cx40,	REF_CX40,			153600*CLOCK_1K,1,	0);	// FIXME

DECLARE_REF_CLK (csi0_pix_phy,		CSI0_PIX_PHY,		125*CLOCK_1M,	1,	0);
DECLARE_REF_CLK (csi0_byte0_phy,	CSI0_BYTE0_PHY,		125*CLOCK_1M,	1,	0);
DECLARE_REF_CLK (csi0_byte1_phy,	CSI0_BYTE1_PHY,		125*CLOCK_1M,	1,	0);
DECLARE_REF_CLK (csi1_pix_phy,		CSI1_PIX_PHY,		125*CLOCK_1M,	1,	0);
DECLARE_REF_CLK (csi1_byte0_phy,	CSI1_BYTE0_PHY,		125*CLOCK_1M,	1,	0);
DECLARE_REF_CLK (csi1_byte1_phy,	CSI1_BYTE1_PHY,		125*CLOCK_1M,	1,	0);
DECLARE_REF_CLK (dsi0_pix_phy,		DSI0_PIX_PHY,		125*CLOCK_1M,	1,	0);
DECLARE_REF_CLK (dsi1_pix_phy,		DSI1_PIX_PHY,		125*CLOCK_1M,	1,	0);
DECLARE_REF_CLK (testdebug,		TEST_DEBUG,		125*CLOCK_1M,	1,	0);



/* CCU clock */
/*****************************************************************************
	Reference clocks
*****************************************************************************/
static struct ccu_clock root_ccu_clk = {
		.clk	=	{
			.name	=	"root_ccu_clk",
			.ops	=	&ccu_clk_ops,
			.ccu_id =       BCM2165x_ROOT_CCU,
			.flags	=	BCM2165x_ROOT_CCU_FLAGS,
		},
		.ccu_clk_mgr_base	=	ROOT_CLK_BASE_ADDR,
		.wr_access_offset	=	ROOT_CLK_MGR_REG_WR_ACCESS_OFFSET,
};

DECLARE_CCU_CLK(kpm_ccu, 2, KONA_MST, KPM, MASK,
	26*CLOCK_1M,  52*CLOCK_1M, 104*CLOCK_1M, 156*CLOCK_1M,
	156*CLOCK_1M, 208*CLOCK_1M, 312*CLOCK_1M, 312*CLOCK_1M);

DECLARE_CCU_CLK(kps_ccu, 2, KONA_SLV, KPS, MASK,
	26*CLOCK_1M,  52*CLOCK_1M,  78*CLOCK_1M, 104*CLOCK_1M,
	156*CLOCK_1M, 156*CLOCK_1M);

DECLARE_CCU_CLK(mm_ccu, 5, MM, MM, MASK,
	 26*CLOCK_1M,  49920*CLOCK_1K,	83200*CLOCK_1K, 99840*CLOCK_1K,
	166400*CLOCK_1K, 249600*CLOCK_1K);

DECLARE_CCU_CLK(khubaon_ccu, 4, AON, KHUBAON, MASK1,
	 26*CLOCK_1M,  52*CLOCK_1M, 78*CLOCK_1M, 104*CLOCK_1M,
	156*CLOCK_1M, CLOCK_UNUSED);

static struct ccu_clock khub_ccu_clk = {
		.clk	=	{
			.name	=	__stringify(khub_ccu_clk),
			.ops	=	&ccu_clk_ops,
			.ccu_id =       BCM2165x_HUB_CCU,
			.flags	=	BCM2165x_HUB_CCU_FLAGS,
		},
		.ccu_clk_mgr_base	=	HUB_CLK_BASE_ADDR,
		.wr_access_offset	=	KHUB_CLK_MGR_REG_WR_ACCESS_OFFSET,
		.policy_freq_offset	=	KHUB_CLK_MGR_REG_POLICY_FREQ_OFFSET,
		.freq_bit_shift 	=	KHUB_CLK_MGR_REG_POLICY_FREQ_POLICY1_FREQ_SHIFT,
		.policy_ctl_offset	=	KHUB_CLK_MGR_REG_POLICY_CTL_OFFSET,
		.policy0_mask_offset	=	KHUB_CLK_MGR_REG_POLICY0_MASK1_OFFSET,
		.policy1_mask_offset	=	KHUB_CLK_MGR_REG_POLICY1_MASK1_OFFSET,
		.policy2_mask_offset	=	KHUB_CLK_MGR_REG_POLICY2_MASK1_OFFSET,
		.policy3_mask_offset	=	KHUB_CLK_MGR_REG_POLICY3_MASK1_OFFSET,
		.policy0_mask1_offset	=	KHUB_CLK_MGR_REG_POLICY0_MASK2_OFFSET,
		.policy1_mask1_offset	=	KHUB_CLK_MGR_REG_POLICY1_MASK2_OFFSET,
		.policy2_mask1_offset	=	KHUB_CLK_MGR_REG_POLICY2_MASK2_OFFSET,
		.policy3_mask1_offset	=	KHUB_CLK_MGR_REG_POLICY3_MASK2_OFFSET,
		.lvm_en_offset		=	KHUB_CLK_MGR_REG_LVM_EN_OFFSET,
		.freq_id	=	5,
		.freq_tbl	=	{26*CLOCK_1M,  52*CLOCK_1M, 104*CLOCK_1M, 156*CLOCK_1M,
					156*CLOCK_1M, 208*CLOCK_1M, 208*CLOCK_1M, CLOCK_UNUSED },
};

/*****************************************************************************
	Bus clocks
*****************************************************************************/
/* KPM bus clock */
DECLARE_BUS_CLK(usb_otg, USB_OTG, USB_OTG_AHB, kpm_ccu, KONA_MST, KPM,
	26*CLOCK_1M,  52*CLOCK_1M,  52*CLOCK_1M,  52*CLOCK_1M,
	78*CLOCK_1M, 104*CLOCK_1M, 104*CLOCK_1M, 156*CLOCK_1M);

DECLARE_BUS_CLK(sdio1_ahb, SDIO1, SDIO1_AHB, kpm_ccu, KONA_MST, KPM,
	26*CLOCK_1M,  52*CLOCK_1M,  52*CLOCK_1M,  52*CLOCK_1M,
	78*CLOCK_1M, 104*CLOCK_1M, 104*CLOCK_1M, 156*CLOCK_1M);

DECLARE_BUS_CLK(sdio2_ahb, SDIO2, SDIO2_AHB, kpm_ccu, KONA_MST, KPM,
	26*CLOCK_1M,  52*CLOCK_1M,  52*CLOCK_1M,  52*CLOCK_1M,
	78*CLOCK_1M, 104*CLOCK_1M, 104*CLOCK_1M, 156*CLOCK_1M);

DECLARE_BUS_CLK(sdio3_ahb, SDIO3, SDIO3_AHB, kpm_ccu, KONA_MST, KPM,
	26*CLOCK_1M,  52*CLOCK_1M,  52*CLOCK_1M,  52*CLOCK_1M,
	78*CLOCK_1M, 104*CLOCK_1M, 104*CLOCK_1M, 156*CLOCK_1M);

DECLARE_BUS_CLK(sdio4_ahb, SDIO4, SDIO4_AHB, kpm_ccu, KONA_MST, KPM,
	26*CLOCK_1M,  52*CLOCK_1M,  52*CLOCK_1M,  52*CLOCK_1M,
	78*CLOCK_1M, 104*CLOCK_1M, 104*CLOCK_1M, 156*CLOCK_1M);

DECLARE_BUS_CLK_NO_GATING_SEL(sdio1_sleep, SDIO1, SDIO1_SLEEP, kpm_ccu, KONA_MST, KPM,
	32*CLOCK_1K, 32*CLOCK_1K, 32*CLOCK_1K, 32*CLOCK_1K,
	32*CLOCK_1K, 32*CLOCK_1K, 32*CLOCK_1K, 32*CLOCK_1K);

DECLARE_BUS_CLK_NO_GATING_SEL(sdio2_sleep, SDIO2, SDIO2_SLEEP, kpm_ccu, KONA_MST, KPM,
	32*CLOCK_1K, 32*CLOCK_1K, 32*CLOCK_1K, 32*CLOCK_1K,
	32*CLOCK_1K, 32*CLOCK_1K, 32*CLOCK_1K, 32*CLOCK_1K);

DECLARE_BUS_CLK_NO_GATING_SEL(sdio3_sleep, SDIO3, SDIO3_SLEEP, kpm_ccu, KONA_MST, KPM,
	32*CLOCK_1K, 32*CLOCK_1K, 32*CLOCK_1K, 32*CLOCK_1K,
	32*CLOCK_1K, 32*CLOCK_1K, 32*CLOCK_1K, 32*CLOCK_1K);

DECLARE_BUS_CLK_NO_GATING_SEL(sdio4_sleep, SDIO4, SDIO4_SLEEP, kpm_ccu, KONA_MST, KPM,
	32*CLOCK_1K, 32*CLOCK_1K, 32*CLOCK_1K, 32*CLOCK_1K,
	32*CLOCK_1K, 32*CLOCK_1K, 32*CLOCK_1K, 32*CLOCK_1K);

/* KPS bus clock */
DECLARE_BUS_CLK(hsm_ahb, HSM, HSM_AHB, kps_ccu, KONA_SLV, KPS,
	 26*CLOCK_1M,  52*CLOCK_1M,  78*CLOCK_1M,  104*CLOCK_1M,
	 156*CLOCK_1M,  156*CLOCK_1M);

DECLARE_BUS_CLK(hsm_apb, HSM, HSM_APB, kps_ccu, KONA_SLV, KPS,
	 26*CLOCK_1M,  26*CLOCK_1M,  39*CLOCK_1M,  52*CLOCK_1M,
	 52*CLOCK_1M,  78*CLOCK_1M);

DECLARE_BUS_CLK(spum_open_apb, SPUM_OPEN_APB, SPUM_OPEN_APB, kps_ccu, KONA_SLV, KPS,
	 26*CLOCK_1M,  26*CLOCK_1M,  39*CLOCK_1M,  52*CLOCK_1M,
	 52*CLOCK_1M,  78*CLOCK_1M);

DECLARE_BUS_CLK(spum_sec_apb, SPUM_SEC_APB, SPUM_SEC_APB, kps_ccu, KONA_SLV, KPS,
	 26*CLOCK_1M,  26*CLOCK_1M,  39*CLOCK_1M,  52*CLOCK_1M,
	 52*CLOCK_1M,  78*CLOCK_1M);

DECLARE_BUS_CLK_NO_GATING_SEL(apb1, APB1, APB1, kps_ccu, KONA_SLV, KPS,
	 26*CLOCK_1M,  26*CLOCK_1M,  39*CLOCK_1M,  52*CLOCK_1M,
	 52*CLOCK_1M,  78*CLOCK_1M);

DECLARE_BUS_CLK(timers_apb, TIMERS, TIMERS_APB, kps_ccu, KONA_SLV, KPS,
	 26*CLOCK_1M,  26*CLOCK_1M,  39*CLOCK_1M,  52*CLOCK_1M,
	 52*CLOCK_1M,  78*CLOCK_1M);

DECLARE_BUS_CLK(ssp0_apb, SSP0, SSP0_APB, kps_ccu, KONA_SLV, KPS,
	 26*CLOCK_1M,  26*CLOCK_1M,  39*CLOCK_1M,  52*CLOCK_1M,
	 52*CLOCK_1M,  78*CLOCK_1M);

DECLARE_BUS_CLK(dmac_mux_apb, DMAC_MUX, DMAC_MUX_APB, kps_ccu, KONA_SLV, KPS,
	 26*CLOCK_1M,  26*CLOCK_1M,  39*CLOCK_1M,  52*CLOCK_1M,
	 52*CLOCK_1M,  78*CLOCK_1M);

DECLARE_BUS_CLK(uartb3_apb, UARTB3, UARTB3_APB, kps_ccu, KONA_SLV, KPS,
	 26*CLOCK_1M,  26*CLOCK_1M,  39*CLOCK_1M,  52*CLOCK_1M,
	 52*CLOCK_1M,  78*CLOCK_1M);

DECLARE_BUS_CLK(uartb2_apb, UARTB2, UARTB2_APB, kps_ccu, KONA_SLV, KPS,
	 26*CLOCK_1M,  26*CLOCK_1M,  39*CLOCK_1M,  52*CLOCK_1M,
	 52*CLOCK_1M,  78*CLOCK_1M);

DECLARE_BUS_CLK(uartb_apb, UARTB, UARTB_APB, kps_ccu, KONA_SLV, KPS,
	 26*CLOCK_1M,  26*CLOCK_1M,  39*CLOCK_1M,  52*CLOCK_1M,
	 52*CLOCK_1M,  78*CLOCK_1M);

DECLARE_BUS_CLK(pwm_apb, PWM, PWM_APB, kps_ccu, KONA_SLV, KPS,
	 26*CLOCK_1M,  26*CLOCK_1M,  39*CLOCK_1M,  52*CLOCK_1M,
	 52*CLOCK_1M,  78*CLOCK_1M);

/* PWM has no divider - looks more likely bus clock, somehow in the peripheral clock zone  */
DECLARE_BUS_CLK(pwm, PWM, PWM, kps_ccu, KONA_SLV, KPS,
	 26*CLOCK_1M,  26*CLOCK_1M,  26*CLOCK_1M,  26*CLOCK_1M,
	 26*CLOCK_1M,  26*CLOCK_1M);

DECLARE_BUS_CLK(bsc1_apb, BSC1, BSC1_APB, kps_ccu, KONA_SLV, KPS,
	 26*CLOCK_1M,  26*CLOCK_1M,  39*CLOCK_1M,  52*CLOCK_1M,
	 52*CLOCK_1M,  78*CLOCK_1M);

DECLARE_BUS_CLK(bsc2_apb, BSC2, BSC2_APB, kps_ccu, KONA_SLV, KPS,
	 26*CLOCK_1M,  26*CLOCK_1M,  39*CLOCK_1M,  52*CLOCK_1M,
	 52*CLOCK_1M,  78*CLOCK_1M);

DECLARE_BUS_CLK_NO_GATING_SEL(apb2, APB2_REG, APB2_REG, kps_ccu, KONA_SLV, KPS,
	 26*CLOCK_1M,  26*CLOCK_1M,  39*CLOCK_1M,  52*CLOCK_1M,
	 52*CLOCK_1M,  78*CLOCK_1M);


/* MM bus clock */
DECLARE_BUS_CLK(spi_apb, SPI_APB, SPI_APB, mm_ccu, MM, MM,
	 26*CLOCK_1M,  49920*CLOCK_1K,	83200*CLOCK_1K, 99840*CLOCK_1K,
	166400*CLOCK_1K, 249600*CLOCK_1K);

DECLARE_BUS_CLK(mm_apb, MM_APB, MM_APB, mm_ccu, MM, MM,
	 26*CLOCK_1M,  49920*CLOCK_1K,	83200*CLOCK_1K, 99840*CLOCK_1K,
	166400*CLOCK_1K, 249600*CLOCK_1K);

DECLARE_BUS_CLK(csi0_axi, CSI0_AXI, CSI0_AXI, mm_ccu, MM, MM,
	 26*CLOCK_1M,  49920*CLOCK_1K,	83200*CLOCK_1K, 99840*CLOCK_1K,
	166400*CLOCK_1K, 249600*CLOCK_1K);

DECLARE_BUS_CLK(csi1_axi, CSI1_AXI, CSI1_AXI, mm_ccu, MM, MM,
	 26*CLOCK_1M,  49920*CLOCK_1K,	83200*CLOCK_1K, 99840*CLOCK_1K,
	166400*CLOCK_1K, 249600*CLOCK_1K);

DECLARE_BUS_CLK(dsi0_axi, DSI0_AXI, DSI0_AXI, mm_ccu, MM, MM,
	 26*CLOCK_1M,  49920*CLOCK_1K,	83200*CLOCK_1K, 99840*CLOCK_1K,
	166400*CLOCK_1K, 249600*CLOCK_1K);

DECLARE_BUS_CLK(dsi1_axi, DSI1_AXI, DSI1_AXI, mm_ccu, MM, MM,
	 26*CLOCK_1M,  49920*CLOCK_1K,	83200*CLOCK_1K, 99840*CLOCK_1K,
	166400*CLOCK_1K, 249600*CLOCK_1K);

DECLARE_BUS_CLK(smi_axi, SMI_AXI, SMI_AXI, mm_ccu, MM, MM,
	 26*CLOCK_1M,  49920*CLOCK_1K,	83200*CLOCK_1K, 99840*CLOCK_1K,
	166400*CLOCK_1K, 249600*CLOCK_1K);

DECLARE_BUS_CLK(v3d_axi, V3D, V3D_AXI, mm_ccu, MM, MM,
	 26*CLOCK_1M,  49920*CLOCK_1K,	83200*CLOCK_1K, 99840*CLOCK_1K,
	166400*CLOCK_1K, 249600*CLOCK_1K);

DECLARE_BUS_CLK(vce_axi, VCE, VCE_AXI, mm_ccu, MM, MM,
	 26*CLOCK_1M,  49920*CLOCK_1K,	83200*CLOCK_1K, 99840*CLOCK_1K,
	166400*CLOCK_1K, 249600*CLOCK_1K);

DECLARE_BUS_CLK(isp_axi, ISP, ISP_AXI, mm_ccu, MM, MM,
	 26*CLOCK_1M,  49920*CLOCK_1K,	83200*CLOCK_1K, 99840*CLOCK_1K,
	166400*CLOCK_1K, 249600*CLOCK_1K);

DECLARE_BUS_CLK(mm_dma_axi, MM_DMA, MM_DMA_AXI, mm_ccu, MM, MM,
	 26*CLOCK_1M,  49920*CLOCK_1K,	83200*CLOCK_1K, 99840*CLOCK_1K,
	166400*CLOCK_1K, 249600*CLOCK_1K);

/* KHUBAON bus clocks  */
DECLARE_BUS_CLK(pwrmgr_axi, PWRMGR, PWRMGR_AXI, khubaon_ccu, AON, KHUBAON,
	 26*CLOCK_1M,  52*CLOCK_1M, 78*CLOCK_1M, 104*CLOCK_1M,
	156*CLOCK_1M, CLOCK_UNUSED);

DECLARE_BUS_CLK(gpiokp_apb, GPIOKP, GPIOKP_APB, khubaon_ccu, AON, KHUBAON,
	 26*CLOCK_1M,  52*CLOCK_1M, 78*CLOCK_1M, 104*CLOCK_1M,
	156*CLOCK_1M, CLOCK_UNUSED);

DECLARE_BUS_CLK(hub_timer_apb, HUB_TIMER, HUB_TIMER_APB, khubaon_ccu, AON, KHUBAON,
	 26*CLOCK_1M,  52*CLOCK_1M, 78*CLOCK_1M, 104*CLOCK_1M,
	156*CLOCK_1M, CLOCK_UNUSED);

DECLARE_BUS_CLK(pmu_bsc_apb, PMU_BSC, PMU_BSC_APB, khubaon_ccu, AON, KHUBAON,
	 26*CLOCK_1M,  52*CLOCK_1M, 78*CLOCK_1M, 104*CLOCK_1M,
	156*CLOCK_1M, CLOCK_UNUSED);

DECLARE_BUS_CLK(chipreg_apb, CHIPREG, CHIPREG_APB, khubaon_ccu, AON, KHUBAON,
	 26*CLOCK_1M,  52*CLOCK_1M, 78*CLOCK_1M, 104*CLOCK_1M,
	156*CLOCK_1M, CLOCK_UNUSED);

DECLARE_BUS_CLK(fmon_apb, FMON, FMON_APB, khubaon_ccu, AON, KHUBAON,
	 26*CLOCK_1M,  52*CLOCK_1M, 78*CLOCK_1M, 104*CLOCK_1M,
	156*CLOCK_1M, CLOCK_UNUSED);

DECLARE_BUS_CLK(hub_tzcfg_apb, HUB_TZCFG, HUB_TZCFG_APB, khubaon_ccu, AON, KHUBAON,
	 26*CLOCK_1M,  52*CLOCK_1M, 78*CLOCK_1M, 104*CLOCK_1M,
	156*CLOCK_1M, CLOCK_UNUSED);

DECLARE_BUS_CLK(sec_wd_apb, SEC_WD, SEC_WD_APB, khubaon_ccu, AON, KHUBAON,
	 26*CLOCK_1M,  52*CLOCK_1M, 78*CLOCK_1M, 104*CLOCK_1M,
	156*CLOCK_1M, CLOCK_UNUSED);

DECLARE_BUS_CLK(sysemi_sec_apb, SYSEMI, SYSEMI_SEC_APB, khubaon_ccu, AON, KHUBAON,
	 26*CLOCK_1M,  52*CLOCK_1M, 78*CLOCK_1M, 104*CLOCK_1M,
	156*CLOCK_1M, CLOCK_UNUSED);

DECLARE_BUS_CLK(sysemi_open_apb, SYSEMI, SYSEMI_OPEN_APB, khubaon_ccu, AON, KHUBAON,
	 26*CLOCK_1M,  52*CLOCK_1M, 78*CLOCK_1M, 104*CLOCK_1M,
	156*CLOCK_1M, CLOCK_UNUSED);

DECLARE_BUS_CLK(vcemi_sec_apb, VCEMI, VCEMI_SEC_APB, khubaon_ccu, AON, KHUBAON,
	 26*CLOCK_1M,  52*CLOCK_1M, 78*CLOCK_1M, 104*CLOCK_1M,
	156*CLOCK_1M, CLOCK_UNUSED);

DECLARE_BUS_CLK(vcemi_open_apb, VCEMI, VCEMI_OPEN_APB, khubaon_ccu, AON, KHUBAON,
	 26*CLOCK_1M,  52*CLOCK_1M, 78*CLOCK_1M, 104*CLOCK_1M,
	156*CLOCK_1M, CLOCK_UNUSED);

DECLARE_BUS_CLK(aci_apb, ACI, ACI_APB, khubaon_ccu, AON, KHUBAON,
	 26*CLOCK_1M,  52*CLOCK_1M, 78*CLOCK_1M, 104*CLOCK_1M,
	156*CLOCK_1M, CLOCK_UNUSED);

DECLARE_BUS_CLK(sim_apb, SIM, SIM_APB, khubaon_ccu, AON, KHUBAON,
	 26*CLOCK_1M,  52*CLOCK_1M, 78*CLOCK_1M, 104*CLOCK_1M,
	156*CLOCK_1M, CLOCK_UNUSED);

DECLARE_BUS_CLK(spm_apb, SPM, SPM_APB, khubaon_ccu, AON, KHUBAON,
	 26*CLOCK_1M,  52*CLOCK_1M, 78*CLOCK_1M, 104*CLOCK_1M,
	156*CLOCK_1M, CLOCK_UNUSED);

DECLARE_BUS_CLK(sim2_apb, SIM2, SIM2_APB, khubaon_ccu, AON, KHUBAON,
	 26*CLOCK_1M,  52*CLOCK_1M, 78*CLOCK_1M, 104*CLOCK_1M,
	156*CLOCK_1M, CLOCK_UNUSED);
/* DAP clock is similar to a BUS clock */
DECLARE_BUS_CLK(dap, DAP, DAP, khubaon_ccu, AON, KHUBAON,
	 26*CLOCK_1M,  52*CLOCK_1M, 78*CLOCK_1M, 104*CLOCK_1M,
	156*CLOCK_1M, CLOCK_UNUSED);

/* KHUB bus clocks  */
/* DAP_SWITCH clock is similar to a BUS clock */
DECLARE_BUS_CLK(dap_switch, DAP_SWITCH, DAP_SWITCH, khub_ccu, HUB, KHUB,
	 26*CLOCK_1M,  52*CLOCK_1M, 52*CLOCK_1M, 78*CLOCK_1M,
	78*CLOCK_1M, 104*CLOCK_1M, 104*CLOCK_1M, CLOCK_UNUSED);

DECLARE_BUS_CLK(brom, BROM, BROM, khub_ccu, HUB, KHUB,
	 26*CLOCK_1M,  52*CLOCK_1M, 104*CLOCK_1M, 156*CLOCK_1M,
	156*CLOCK_1M, 208*CLOCK_1M, 208*CLOCK_1M, CLOCK_UNUSED);

DECLARE_BUS_CLK(nor_apb, NOR, NOR_APB, khub_ccu, HUB, KHUB,
	 26*CLOCK_1M,  52*CLOCK_1M, 52*CLOCK_1M, 78*CLOCK_1M,
	78*CLOCK_1M, 104*CLOCK_1M, 104*CLOCK_1M, CLOCK_UNUSED);

DECLARE_BUS_CLK(nor, NOR, NOR, khub_ccu, HUB, KHUB,
	 26*CLOCK_1M,  52*CLOCK_1M, 104*CLOCK_1M, 156*CLOCK_1M,
	156*CLOCK_1M, 208*CLOCK_1M, 208*CLOCK_1M, CLOCK_UNUSED);

DECLARE_BUS_CLK(mdiomaster, MDIO, MDIOMASTER, khub_ccu, HUB, KHUB,
	 26*CLOCK_1M,  CLOCK_UNUSED, CLOCK_UNUSED, CLOCK_UNUSED,
	CLOCK_UNUSED, CLOCK_UNUSED, CLOCK_UNUSED, CLOCK_UNUSED);

DECLARE_BUS_CLK(apb5, APB5, APB5, khub_ccu, HUB, KHUB,
	 26*CLOCK_1M,  52*CLOCK_1M, 52*CLOCK_1M, 78*CLOCK_1M,
	78*CLOCK_1M, 104*CLOCK_1M, 104*CLOCK_1M, CLOCK_UNUSED);

DECLARE_BUS_CLK(cti_apb, CTI, CTI_APB, khub_ccu, HUB, KHUB,
	 26*CLOCK_1M,  52*CLOCK_1M, 52*CLOCK_1M, 78*CLOCK_1M,
	78*CLOCK_1M, 104*CLOCK_1M, 104*CLOCK_1M, CLOCK_UNUSED);

DECLARE_BUS_CLK(funnel_apb, FUNNEL, FUNNEL_APB, khub_ccu, HUB, KHUB,
	 26*CLOCK_1M,  52*CLOCK_1M, 52*CLOCK_1M, 78*CLOCK_1M,
	78*CLOCK_1M, 104*CLOCK_1M, 104*CLOCK_1M, CLOCK_UNUSED);

DECLARE_BUS_CLK(tpiu_apb, TPIU, TPIU_APB, khub_ccu, HUB, KHUB,
	 26*CLOCK_1M,  52*CLOCK_1M, 52*CLOCK_1M, 78*CLOCK_1M,
	78*CLOCK_1M, 104*CLOCK_1M, 104*CLOCK_1M, CLOCK_UNUSED);

DECLARE_BUS_CLK(vc_itm_apb, VC_ITM, VC_ITM_APB, khub_ccu, HUB, KHUB,
	 26*CLOCK_1M,  52*CLOCK_1M, 52*CLOCK_1M, 78*CLOCK_1M,
	78*CLOCK_1M, 104*CLOCK_1M, 104*CLOCK_1M, CLOCK_UNUSED);

DECLARE_BUS_CLK(sec_viol_trap_4_apb, SECTRAP4, SEC_VIOL_TRAP_4_APB, khub_ccu, HUB, KHUB,
	 26*CLOCK_1M,  52*CLOCK_1M, 52*CLOCK_1M, 78*CLOCK_1M,
	78*CLOCK_1M, 104*CLOCK_1M, 104*CLOCK_1M, CLOCK_UNUSED);

DECLARE_BUS_CLK(sec_viol_trap_5_apb, SECTRAP5, SEC_VIOL_TRAP_5_APB, khub_ccu, HUB, KHUB,
	 26*CLOCK_1M,  52*CLOCK_1M, 52*CLOCK_1M, 78*CLOCK_1M,
	78*CLOCK_1M, 104*CLOCK_1M, 104*CLOCK_1M, CLOCK_UNUSED);

DECLARE_BUS_CLK(sec_viol_trap_7_apb, SECTRAP7, SEC_VIOL_TRAP_7_APB, khub_ccu, HUB, KHUB,
	 26*CLOCK_1M,  52*CLOCK_1M, 52*CLOCK_1M, 78*CLOCK_1M,
	78*CLOCK_1M, 104*CLOCK_1M, 104*CLOCK_1M, CLOCK_UNUSED);

DECLARE_BUS_CLK(hsi_apb, HSI, HSI_APB, khub_ccu, HUB, KHUB,
	 26*CLOCK_1M,  52*CLOCK_1M, 52*CLOCK_1M, 78*CLOCK_1M,
	78*CLOCK_1M, 104*CLOCK_1M, 104*CLOCK_1M, CLOCK_UNUSED);

DECLARE_BUS_CLK(axi_trace_19_apb, AXI_TRACE19, AXI_TRACE_19_APB, khub_ccu, HUB, KHUB,
	 26*CLOCK_1M,  52*CLOCK_1M, 52*CLOCK_1M, 78*CLOCK_1M,
	78*CLOCK_1M, 104*CLOCK_1M, 104*CLOCK_1M, CLOCK_UNUSED);

DECLARE_BUS_CLK(axi_trace_11_apb, AXI_TRACE11, AXI_TRACE_11_APB, khub_ccu, HUB, KHUB,
	 26*CLOCK_1M,  52*CLOCK_1M, 52*CLOCK_1M, 78*CLOCK_1M,
	78*CLOCK_1M, 104*CLOCK_1M, 104*CLOCK_1M, CLOCK_UNUSED);

DECLARE_BUS_CLK(axi_trace_12_apb, AXI_TRACE12, AXI_TRACE_12_APB, khub_ccu, HUB, KHUB,
	 26*CLOCK_1M,  52*CLOCK_1M, 52*CLOCK_1M, 78*CLOCK_1M,
	78*CLOCK_1M, 104*CLOCK_1M, 104*CLOCK_1M, CLOCK_UNUSED);

DECLARE_BUS_CLK(axi_trace_13_apb, AXI_TRACE13, AXI_TRACE_13_APB, khub_ccu, HUB, KHUB,
	 26*CLOCK_1M,  52*CLOCK_1M, 52*CLOCK_1M, 78*CLOCK_1M,
	78*CLOCK_1M, 104*CLOCK_1M, 104*CLOCK_1M, CLOCK_UNUSED);

DECLARE_BUS_CLK(etb_apb, ETB, ETB_APB, khub_ccu, HUB, KHUB,
	 26*CLOCK_1M,  52*CLOCK_1M, 52*CLOCK_1M, 78*CLOCK_1M,
	78*CLOCK_1M, 104*CLOCK_1M, 104*CLOCK_1M, CLOCK_UNUSED);

DECLARE_BUS_CLK(final_funnel_apb, FINAL_FUNNEL, FINAL_FUNNEL_APB, khub_ccu, HUB, KHUB,
	 26*CLOCK_1M,  52*CLOCK_1M, 52*CLOCK_1M, 78*CLOCK_1M,
	78*CLOCK_1M, 104*CLOCK_1M, 104*CLOCK_1M, CLOCK_UNUSED);

DECLARE_BUS_CLK_NO_GATING_SEL(apb10, APB10, APB10, khub_ccu, HUB, KHUB,
	26*CLOCK_1K, 52*CLOCK_1K, 52*CLOCK_1K, 78*CLOCK_1K,
	78*CLOCK_1K, 104*CLOCK_1K, 104*CLOCK_1K, CLOCK_UNUSED);

DECLARE_BUS_CLK_NO_GATING_SEL(apb9, APB9, APB9, khub_ccu, HUB, KHUB,
	26*CLOCK_1K, 52*CLOCK_1K, 52*CLOCK_1K, 78*CLOCK_1K,
	78*CLOCK_1K, 104*CLOCK_1K, 104*CLOCK_1K, CLOCK_UNUSED);

DECLARE_BUS_CLK(atb_filter_apb, ATB, ATB_FILTER_APB, khub_ccu, HUB, KHUB,
	 26*CLOCK_1M,  52*CLOCK_1M, 52*CLOCK_1M, 78*CLOCK_1M,
	78*CLOCK_1M, 104*CLOCK_1M, 104*CLOCK_1M, CLOCK_UNUSED);

DECLARE_BUS_CLK(bt_slim_ahb_apb, BT_SLIM, BT_SLIM_AHB_APB, khub_ccu, HUB, KHUB,
	 26*CLOCK_1M,  26*CLOCK_1M, 26*CLOCK_1M, 26*CLOCK_1M,
	26*CLOCK_1M, 26*CLOCK_1M, 26*CLOCK_1M, CLOCK_UNUSED);

DECLARE_BUS_CLK(etb2axi_apb, ETB2AXI, ETB2AXI_APB, khub_ccu, HUB, KHUB,
	26*CLOCK_1M,  52*CLOCK_1M, 52*CLOCK_1M, 78*CLOCK_1M,
	78*CLOCK_1M, 104*CLOCK_1M, 104*CLOCK_1M, CLOCK_UNUSED);

DECLARE_BUS_CLK(ssp3_apb, SSP3, SSP3_APB, khub_ccu, HUB, KHUB,
	26*CLOCK_1M,  26*CLOCK_1M, 26*CLOCK_1M, 26*CLOCK_1M,
	26*CLOCK_1M, 26*CLOCK_1M, 26*CLOCK_1M, CLOCK_UNUSED);

DECLARE_BUS_CLK(audioh_apb, AUDIOH, AUDIOH_APB, khub_ccu, HUB, KHUB,
	26*CLOCK_1M,  26*CLOCK_1M, 26*CLOCK_1M, 26*CLOCK_1M,
	26*CLOCK_1M, 26*CLOCK_1M, 26*CLOCK_1M, CLOCK_UNUSED);
/* audioh_156m and audioh_2p4m clocks resemple BUS clock, hence declaring as
 * bus clokc. */
DECLARE_BUS_CLK(audioh_156m, AUDIOH, AUDIOH_156M, khub_ccu, HUB, KHUB,
	156*CLOCK_1M,  156*CLOCK_1M, 156*CLOCK_1M, 156*CLOCK_1M,
	156*CLOCK_1M, 156*CLOCK_1M, 156*CLOCK_1M, CLOCK_UNUSED);

DECLARE_BUS_CLK(audioh_2p4m, AUDIOH, AUDIOH_2P4M, khub_ccu, HUB, KHUB,
	2400*CLOCK_1K,  2400*CLOCK_1K, 2400*CLOCK_1K, 24*CLOCK_1K,
	2400*CLOCK_1K, 24*CLOCK_1K, 2400*CLOCK_1K, CLOCK_UNUSED);

DECLARE_BUS_CLK(tmon_apb, TMON, TMON_APB, khub_ccu, HUB, KHUB,
	26*CLOCK_1M,  52*CLOCK_1M, 52*CLOCK_1M, 78*CLOCK_1M,
	78*CLOCK_1M, 104*CLOCK_1M, 104*CLOCK_1M, CLOCK_UNUSED);

DECLARE_BUS_CLK_NO_GATING_SEL(var_spm_apb, VAR_SPM, VAR_SPM_APB, khub_ccu, HUB, KHUB,
	26*CLOCK_1K, 52*CLOCK_1K, 52*CLOCK_1K, 78*CLOCK_1K,
	78*CLOCK_1K, 104*CLOCK_1K, 104*CLOCK_1K, CLOCK_UNUSED);

DECLARE_BUS_CLK(ssp4_apb, SSP4, SSP4_APB, khub_ccu, HUB, KHUB,
	26*CLOCK_1M,  26*CLOCK_1M, 26*CLOCK_1M, 26*CLOCK_1M,
	26*CLOCK_1M, 26*CLOCK_1M, 26*CLOCK_1M, CLOCK_UNUSED);

/*****************************************************************************
	Peripheral clocks
*****************************************************************************/
/* KPM peripheral */
static struct clk *sdio_clk_src_tbl[] =
{
	name_to_clk(crystal),
	name_to_clk(var_52m),
	name_to_clk(ref_52m),
	name_to_clk(var_96m),
	name_to_clk(ref_96m),
};

static struct clk_src sdio1_clk_src = {
	.total		=	ARRAY_SIZE(sdio_clk_src_tbl),
	.sel		=	2,
	.parents	=	sdio_clk_src_tbl,
};

static struct clk_src sdio2_clk_src = {
	.total		=	ARRAY_SIZE(sdio_clk_src_tbl),
	.sel		=	2,
	.parents	=	sdio_clk_src_tbl,
};

static struct clk_src sdio3_clk_src = {
	.total		=	ARRAY_SIZE(sdio_clk_src_tbl),
	.sel		=	2,
	.parents	=	sdio_clk_src_tbl,
};

static struct clk_src sdio4_clk_src = {
	.total		=	ARRAY_SIZE(sdio_clk_src_tbl),
	.sel		=	2,
	.parents	=	sdio_clk_src_tbl,
};

DECLARE_PERI_CLK(sdio1, SDIO1, SDIO1, ref_52m, 26*CLOCK_1M, 2, DIV_TRIG, KONA_MST, KPM, 0);
DECLARE_PERI_CLK(sdio2, SDIO2, SDIO2, ref_52m, 26*CLOCK_1M, 2, DIV_TRIG, KONA_MST, KPM, 0);
DECLARE_PERI_CLK(sdio3, SDIO3, SDIO3, ref_52m, 26*CLOCK_1M, 2, DIV_TRIG, KONA_MST, KPM, 0);
DECLARE_PERI_CLK(sdio4, SDIO4, SDIO4, ref_52m, 26*CLOCK_1M, 2, DIV_TRIG, KONA_MST, KPM, 0);

/* KPS peripheral clock */
static struct clk *bsc_clk_src_tbl[] =
{
	name_to_clk(crystal),
	name_to_clk(var_104m),
	name_to_clk(ref_104m),
	name_to_clk(var_13m),
	name_to_clk(ref_13m),
};

static struct clk_src bsc1_clk_src = {
	.total		=	ARRAY_SIZE(bsc_clk_src_tbl),
	.sel		=	3,
	.parents	=	bsc_clk_src_tbl,
};

static struct clk_src bsc2_clk_src = {
	.total		=	ARRAY_SIZE(bsc_clk_src_tbl),
	.sel		=	3,
	.parents	=	bsc_clk_src_tbl,
};

DECLARE_PERI_CLK_NO_DIV_COUNT(bsc1, BSC1, ref_13m, 13*CLOCK_1M, DIV_TRIG, KONA_SLV, KPS);
DECLARE_PERI_CLK_NO_DIV_COUNT(bsc2, BSC2, ref_13m, 13*CLOCK_1M, DIV_TRIG, KONA_SLV, KPS);

static struct clk *ssp_clk_src_tbl[] =
{
	name_to_clk(crystal),
	name_to_clk(var_104m),
	name_to_clk(ref_104m),
	name_to_clk(var_96m),
	name_to_clk(ref_96m),
};

static struct clk_src ssp0_clk_src = {
	.total		=	ARRAY_SIZE(ssp_clk_src_tbl),
	.sel		=	1,
	.parents	=	ssp_clk_src_tbl,
};
DECLARE_PERI_CLK(ssp0, SSP0, SSP0, var_104m, 52*CLOCK_1M, 2, DIV_TRIG, KONA_SLV, KPS, 0);

static struct clk *ssp0_audio_clk_src_tbl[] =
{
	name_to_clk(crystal),
	name_to_clk(ref_312m),
	name_to_clk(ref_cx40),
};

static struct clk_src ssp0_audio_clk_src = {
	.total		=	ARRAY_SIZE(ssp0_audio_clk_src_tbl),
	.sel		=	1,
	.parents	=	ssp0_audio_clk_src_tbl,
};

DECLARE_PERI_CLK_PRE_DIV(ssp0_audio, SSP0_AUDIO, SSP0_AUDIO, crystal, 26*CLOCK_1M, 1, DIV_TRIG, KONA_SLV, KPS, 0);



static struct clk *uart_clk_src_tbl[] =
{
	name_to_clk(crystal),
	name_to_clk(var_156m),
	name_to_clk(ref_156m),
};

static struct clk_src uartb3_clk_src = {
	.total		=	ARRAY_SIZE(uart_clk_src_tbl),
	.sel		=	1,
	.parents	=	uart_clk_src_tbl,
};
static struct clk_src uartb2_clk_src = {
	.total		=	ARRAY_SIZE(uart_clk_src_tbl),
	.sel		=	1,
	.parents	=	uart_clk_src_tbl,
};
static struct clk_src uartb_clk_src = {
	.total		=	ARRAY_SIZE(uart_clk_src_tbl),
	.sel		=	1,
	.parents	=	uart_clk_src_tbl,
};
DECLARE_PERI_CLK(uartb3, UARTB3, UARTB3, var_156m, 13*CLOCK_1M, 12, DIV_TRIG, KONA_SLV, KPS, 8);
DECLARE_PERI_CLK(uartb2, UARTB2, UARTB2, var_156m, 13*CLOCK_1M, 12, DIV_TRIG, KONA_SLV, KPS, 8);
DECLARE_PERI_CLK(uartb, UARTB, UARTB, var_156m, 13*CLOCK_1M, 12, DIV_TRIG, KONA_SLV, KPS, 8);

static struct clk *timers_clk_src_tbl[] =
{
	name_to_clk(ref_1m),
	name_to_clk(ref_32k),
};

static struct clk_src timers_clk_src = {
	.total		=	ARRAY_SIZE(timers_clk_src_tbl),
	.sel		=	1,
	.parents	=	timers_clk_src_tbl,
};
DECLARE_PERI_CLK_NO_DIV_COUNT(timers, TIMERS, ref_32k, 32*CLOCK_1K, DIV_TRIG, KONA_SLV, KPS);

static struct clk *spum_clk_src_tbl[] =
{
	name_to_clk(var_312m),
	name_to_clk(ref_312m),
};

static struct clk_src spum_open_clk_src = {
	.total		=	ARRAY_SIZE(spum_clk_src_tbl),
	.sel		=	0,
	.parents	=	spum_clk_src_tbl,
};

static struct clk_src spum_sec_clk_src = {
	.total		=	ARRAY_SIZE(spum_clk_src_tbl),
	.sel		=	0,
	.parents	=	spum_clk_src_tbl,
};

DECLARE_PERI_CLK(spum_open, SPUM_OPEN, SPUM_OPEN, var_312m, 28*CLOCK_1M, 11, DIV_TRIG, KONA_SLV, KPS, 0);
DECLARE_PERI_CLK(spum_sec, SPUM_SEC, SPUM_SEC, var_312m, 28*CLOCK_1M, 11, DIV_TRIG, KONA_SLV, KPS, 0);

/* MM peripheral clock */
static struct clk *mm_src_tbl[] =
{
	name_to_clk(var_500m),
	name_to_clk(var_312m),
};

static struct clk_src smi_clk_src = {
	.total		=	ARRAY_SIZE(mm_src_tbl),
	.sel		=	0,
	.parents	=	mm_src_tbl,
};
DECLARE_PERI_CLK(smi, SMI, SMI, var_500m, 500*CLOCK_1M, 1, DIV_TRIG, MM, MM, 1);

static struct clk *csi0_lp_clk_src_tbl[] =
{
	name_to_clk(var_500m),
	name_to_clk(var_312m),
};

static struct clk_src csi0_lp_clk_src = {
	.total		=	ARRAY_SIZE(csi0_lp_clk_src_tbl),
	.sel		=	0,
	.parents	=	csi0_lp_clk_src_tbl,
};

DECLARE_PERI_CLK(csi0_lp, CSI0_LP, CSI0_LP, var_500m, 500*CLOCK_1M, 1, DIV_TRIG, MM, MM, 0);

static struct clk *csi1_lp_clk_src_tbl[] =
{
	name_to_clk(var_500m),
	name_to_clk(var_312m),
};

static struct clk_src csi1_lp_clk_src = {
	.total		=	ARRAY_SIZE(csi1_lp_clk_src_tbl),
	.sel		=	0,
	.parents	=	csi1_lp_clk_src_tbl,
};

DECLARE_PERI_CLK(csi1_lp, CSI1_LP, CSI1_LP, var_500m, 500*CLOCK_1M, 1, DIV_TRIG, MM, MM, 0);

static struct clk *dsi0_esc_clk_src_tbl[] =
{
	name_to_clk(var_500m),
	name_to_clk(var_312m),
};

static struct clk_src dsi0_esc_clk_src = {
	.total		=	ARRAY_SIZE(dsi0_esc_clk_src_tbl),
	.sel		=	1,
	.parents	=	dsi0_esc_clk_src_tbl,
};

DECLARE_PERI_CLK(dsi0_esc, DSI0_ESC, DSI0_ESC, var_500m, 500*CLOCK_1M, 1, DIV_TRIG, MM, MM, 0);

static struct clk *dsi1_esc_clk_src_tbl[] =
{
	name_to_clk(var_500m),
	name_to_clk(var_312m),
};

static struct clk_src dsi1_esc_clk_src = {
	.total		=	ARRAY_SIZE(dsi1_esc_clk_src_tbl),
	.sel		=	1,
	.parents	=	dsi1_esc_clk_src_tbl,
};

DECLARE_PERI_CLK(dsi1_esc, DSI1_ESC, DSI1_ESC, var_500m, 500*CLOCK_1M, 1, DIV_TRIG, MM, MM, 0);

static struct clk *dsi_pll_o_dsi_pll_clk_src_tbl[] =
{
	name_to_clk(var_500m),
};

static struct clk_src dsi_pll_o_dsi_pll_clk_src = {
	.total		=	ARRAY_SIZE(dsi_pll_o_dsi_pll_clk_src_tbl),
	.sel		=	1,
	.parents	=	dsi_pll_o_dsi_pll_clk_src_tbl,
};

DECLARE_PERI_CLK_NO_DIV_COUNT(dsi_pll_o_dsi_pll, DSI_PLL, var_500m, 500*CLOCK_1M, DIV_TRIG, MM, MM);

/* KHUBAON peripheral clock */
/* HUBAON clock -- Needs work around as the enable bit is in HUB_DIV register
 * instead of HUB_CLKGATE*/
/* APB6 clock -- Needs work around as the enable bit is in HUB_DIV register
 * instead of APB6_CLKGATE*/

static struct clk *hub_timer_clk_src_tbl[] =
{
	name_to_clk(bbl_32k),
	name_to_clk(frac_1m),
	name_to_clk(dft_19_5m),
};

static struct clk_src hub_timer_clk_src = {
	.total		=	ARRAY_SIZE(hub_timer_clk_src_tbl),
	.sel		=	0,
	.parents	=	hub_timer_clk_src_tbl,
};

DECLARE_PERI_CLK_NO_DIV_COUNT(hub_timer, HUB_TIMER, bbl_32k, 32*CLOCK_1K, PERIPH_SEG_TRG, AON, KHUBAON);

static struct clk *pmu_bsc_clk_src_tbl[] =
{
	name_to_clk(crystal),
	/*pmu_bsc_var clock is not defined in the clock tree..!! this need to
	 * be confirmed and then enabled */
	/*name_to_clk(pmu_bsc_var), */
	name_to_clk(bbl_32k),
};

static struct clk_src pmu_bsc_clk_src = {
	.total		=	ARRAY_SIZE(pmu_bsc_clk_src_tbl),
	.sel		=	0,
	.parents	=	pmu_bsc_clk_src_tbl,
};

DECLARE_PERI_CLK(pmu_bsc, PMU_BSC, PMU_BSC, crystal, 32*CLOCK_1K, 1, PERIPH_SEG_TRG, AON, KHUBAON, 0);

static struct clk *sim_clk_src_tbl[] =
{
	name_to_clk(crystal),
	name_to_clk(var_312m),
	name_to_clk(ref_312m),
	name_to_clk(ref_96m),
	name_to_clk(var_96m),
};

static struct clk_src sim_clk_src = {
	.total		=	ARRAY_SIZE(sim_clk_src_tbl),
	.sel		=	0,
	.parents	=	sim_clk_src_tbl,
};

DECLARE_PERI_CLK(sim, SIM, SIM, crystal, 32*CLOCK_1K, 1, PERIPH_SEG_TRG, AON, KHUBAON, 0);

static struct clk *sim2_clk_src_tbl[] =
{
	name_to_clk(crystal),
	name_to_clk(var_312m),
	name_to_clk(ref_312m),
	name_to_clk(ref_96m),
	name_to_clk(var_96m),
};

static struct clk_src sim2_clk_src = {
	.total		=	ARRAY_SIZE(sim2_clk_src_tbl),
	.sel		=	0,
	.parents	=	sim2_clk_src_tbl,
};

DECLARE_PERI_CLK(sim2, SIM2, SIM2, crystal, 32*CLOCK_1K, 1, PERIPH_SEG_TRG, AON, KHUBAON, 0);

/* KHUB peripheral clock */
static struct clk *hub_clk_src_tbl[] =
{
	name_to_clk(crystal),
	name_to_clk(var_312m),
};

static struct clk_src hub_clk_src = {
	.total		=	ARRAY_SIZE(hub_clk_src_tbl),
	.sel		=	1,
	.parents	=	hub_clk_src_tbl,
};

DECLARE_PERI_CLK(hub, HUB, HUB, var_312m, 312*CLOCK_1M, 1, HUB_SEG_TRG, HUB, KHUB, 1);

static struct clk *audioh_26m_clk_src_tbl[] =
{
	name_to_clk(crystal),
	name_to_clk(ref_26m),
};

static struct clk_src audioh_26m_clk_src = {
	.total		=	ARRAY_SIZE(audioh_26m_clk_src_tbl),
	.sel		=	0,
	.parents	=	audioh_26m_clk_src_tbl,
};

DECLARE_PERI_CLK_NO_DIV_COUNT(audioh_26m, AUDIOH_26M, crystal, 26*CLOCK_1M, PERIPH_SEG_TRG, HUB, KHUB);

static struct clk *tmon_1m_clk_src_tbl[] =
{
	name_to_clk(frac_1m),
	name_to_clk(dft_19_5m),
};

static struct clk_src tmon_1m_clk_src = {
	.total		=	ARRAY_SIZE(tmon_1m_clk_src_tbl),
	.sel		=	1,
	.parents	=	tmon_1m_clk_src_tbl,
};

DECLARE_PERI_CLK_NO_DIV_COUNT(tmon_1m, TMON_1M, frac_1m, 1*CLOCK_1M, TMON_TRG_DBG, HUB, KHUB);

static struct clk *caph_srcmixer_clk_src_tbl[] =
{
	name_to_clk(crystal),
	name_to_clk(ref_312m),
};

static struct clk_src caph_srcmixer_clk_src = {
	.total		=	ARRAY_SIZE(caph_srcmixer_clk_src_tbl),
	.sel		=	1,
	.parents	=	caph_srcmixer_clk_src_tbl,
};

DECLARE_PERI_CLK(caph_srcmixer, CAPH, CAPH_SRCMIXER, crystal, 26*CLOCK_1M, 1, PERIPH_SEG_TRG, HUB, KHUB, 0);

static struct clk *ssp3_clk_src_tbl[] =
{
	name_to_clk(crystal),
	name_to_clk(var_312m),
	name_to_clk(ref_312m),
	name_to_clk(ref_96m),
	name_to_clk(var_96m),
};

static struct clk_src ssp3_clk_src = {
	.total		=	ARRAY_SIZE(ssp3_clk_src_tbl),
	.sel		=	1,
	.parents	=	ssp3_clk_src_tbl,
};

DECLARE_PERI_CLK_PRE_DIV(ssp3, SSP3, SSP3, crystal, 26*CLOCK_1M, 1, PERIPH_SEG_TRG, HUB, KHUB, 0);

static struct clk *ssp3_audio_clk_src_tbl[] =
{
	name_to_clk(crystal),
	name_to_clk(ref_312m),
	name_to_clk(ref_cx40),
};

static struct clk_src ssp3_audio_clk_src = {
	.total		=	ARRAY_SIZE(ssp3_audio_clk_src_tbl),
	.sel		=	1,
	.parents	=	ssp3_audio_clk_src_tbl,
};

DECLARE_PERI_CLK_PRE_DIV(ssp3_audio, SSP3_AUDIO, SSP3_AUDIO, crystal, 26*CLOCK_1M, 1, PERIPH_SEG_TRG, HUB, KHUB, 0);

static struct clk *ssp4_clk_src_tbl[] =
{
	name_to_clk(crystal),
	name_to_clk(var_312m),
	name_to_clk(ref_312m),
	name_to_clk(ref_96m),
	name_to_clk(var_96m),
};

static struct clk_src ssp4_clk_src = {
	.total		=	ARRAY_SIZE(ssp4_clk_src_tbl),
	.sel		=	1,
	.parents	=	ssp4_clk_src_tbl,
};

DECLARE_PERI_CLK_PRE_DIV(ssp4, SSP4, SSP4, crystal, 26*CLOCK_1M, 1, PERIPH_SEG_TRG, HUB, KHUB, 0);

static struct clk *ssp4_audio_clk_src_tbl[] =
{
	name_to_clk(crystal),
	name_to_clk(ref_312m),
	name_to_clk(ref_cx40),
};

static struct clk_src ssp4_audio_clk_src = {
	.total		=	ARRAY_SIZE(ssp4_audio_clk_src_tbl),
	.sel		=	1,
	.parents	=	ssp4_audio_clk_src_tbl,
};

DECLARE_PERI_CLK_PRE_DIV(ssp4_audio, SSP4_AUDIO, SSP4_AUDIO, crystal, 26*CLOCK_1M, 1, PERIPH_SEG_TRG, HUB, KHUB, 0);

/* table for registering clock */
static struct __init clk_lookup rhea_clk_tbl[] =
{
	CLK_LK(arm),
/*Reference clocks */
	CLK_LK(crystal),
	CLK_LK(frac_1m),
	CLK_LK(ref_96m_varVDD),
	CLK_LK(var_96m),
	CLK_LK(ref_96m),
	CLK_LK(var_500m),
	CLK_LK(var_500m_varVDD),

	CLK_LK(ref_32k),
	CLK_LK(bbl_32k),
	CLK_LK(misc_32k),

	CLK_LK(dft_19_5m),

	CLK_LK(ref_312m),
	CLK_LK(ref_208m),
	CLK_LK(ref_104m),
	CLK_LK(ref_52m),
	CLK_LK(ref_13m),
	CLK_LK(ref_26m),

	CLK_LK(var_312m),
	CLK_LK(var_208m),
	CLK_LK(var_156m),
	CLK_LK(var_52m),
	CLK_LK(var_13m),

	CLK_LK(usbh_48m),
	CLK_LK(ref_cx40),
/* CCUs */
	CLK_LK(root_ccu),
	CLK_LK(kpm_ccu),
	CLK_LK(kps_ccu),
	CLK_LK(mm_ccu),
	CLK_LK(khubaon_ccu),
	CLK_LK(khub_ccu),
/* Bus clocks */
	CLK_LK(sdio1_ahb),
	CLK_LK(sdio2_ahb),
	CLK_LK(sdio3_ahb),
	CLK_LK(sdio4_ahb),
	CLK_LK(sdio1_sleep),
	CLK_LK(sdio2_sleep),
	CLK_LK(sdio3_sleep),
	CLK_LK(sdio4_sleep),
	CLK_LK(bsc1_apb),
	CLK_LK(bsc2_apb),
	CLK_LK(hsm_ahb),
	CLK_LK(hsm_apb),
	CLK_LK(spum_open_apb),
	CLK_LK(spum_sec_apb),
	CLK_LK(apb1),
	CLK_LK(timers_apb),
	CLK_LK(ssp0_apb),
	CLK_LK(dmac_mux_apb),
	CLK_LK(uartb3_apb),
	CLK_LK(uartb2_apb),
	CLK_LK(uartb_apb),
	CLK_LK(pwm_apb),
	CLK_LK(spi_apb),
	CLK_LK(mm_apb),
	CLK_LK(csi0_axi),
	CLK_LK(csi1_axi),
	CLK_LK(dsi0_axi),
	CLK_LK(dsi1_axi),
	CLK_LK(v3d_axi),
	CLK_LK(vce_axi),
	CLK_LK(isp_axi),
	CLK_LK(mm_dma_axi),
	CLK_LK(smi_axi),
	CLK_LK(pwrmgr_axi),
	CLK_LK(gpiokp_apb),
	CLK_LK(hub_timer_apb),
	CLK_LK(pmu_bsc_apb),
	CLK_LK(chipreg_apb),
	CLK_LK(fmon_apb),
	CLK_LK(hub_tzcfg_apb),
	CLK_LK(sec_wd_apb),
	CLK_LK(sysemi_sec_apb),
	CLK_LK(sysemi_open_apb),
	CLK_LK(vcemi_sec_apb),
	CLK_LK(vcemi_open_apb),
	CLK_LK(aci_apb),
	CLK_LK(sim_apb),
	CLK_LK(spm_apb),
	CLK_LK(sim2_apb),
	CLK_LK(dap),
	/* HUB CCU */
	CLK_LK(dap_switch),
	CLK_LK(brom),
	CLK_LK(nor_apb),
	CLK_LK(nor),
	CLK_LK(mdiomaster),
	CLK_LK(apb5),
	CLK_LK(cti_apb),
	CLK_LK(funnel_apb),
	CLK_LK(tpiu_apb),
	CLK_LK(vc_itm_apb),
	CLK_LK(sec_viol_trap_4_apb),
	CLK_LK(sec_viol_trap_5_apb),
	CLK_LK(sec_viol_trap_7_apb),
	CLK_LK(hsi_apb),
	CLK_LK(axi_trace_19_apb),
	CLK_LK(axi_trace_11_apb),
	CLK_LK(axi_trace_12_apb),
	CLK_LK(axi_trace_13_apb),
	CLK_LK(etb_apb),
	CLK_LK(final_funnel_apb),
	CLK_LK(apb10),
	CLK_LK(apb9),
	CLK_LK(atb_filter_apb),
	CLK_LK(bt_slim_ahb_apb),
	CLK_LK(etb2axi_apb),
	CLK_LK(ssp3_apb),
	CLK_LK(audioh_apb),
	CLK_LK(audioh_156m),
	CLK_LK(audioh_2p4m),
	CLK_LK(tmon_apb),
	CLK_LK(var_spm_apb),
	CLK_LK(ssp4_apb),

/* Perepheral clocks */
	/* HUB CCU */
	CLK_LK(hub),


	CLK_LK(sdio1),
	CLK_LK(sdio2),
	CLK_LK(sdio3),
	CLK_LK(sdio4),

	CLK_LK(bsc1),
	CLK_LK(bsc2),

	CLK_LK(smi),

	CLK_LK(usb_otg),
	CLK_LK(pwm),
	CLK_LK(apb2),
	CLK_LK(ssp0),
	CLK_LK(ssp0_audio),
	CLK_LK(uartb3),
	CLK_LK(uartb2),
	CLK_LK(uartb),
	CLK_LK(timers),
	CLK_LK(spum_open),
	CLK_LK(spum_sec),

	CLK_LK(hub_timer),
	CLK_LK(pmu_bsc),
	CLK_LK(sim),
	CLK_LK(sim2),
	CLK_LK(audioh_26m),
	CLK_LK(tmon_1m),
	CLK_LK(caph_srcmixer),
	CLK_LK(ssp3),
	CLK_LK(ssp4),
	CLK_LK(ssp3_audio),
	CLK_LK(ssp4_audio),

	CLK_LK(dsi_pll_o_dsi_pll),
	CLK_LK(dsi0_esc),
	CLK_LK(dsi1_esc),
	CLK_LK(csi0_lp),
	CLK_LK(csi1_lp),
	CLK_LK(csi0_pix_phy),
	CLK_LK(csi0_byte0_phy),
	CLK_LK(csi0_byte1_phy),
	CLK_LK(csi1_pix_phy),
	CLK_LK(csi1_byte0_phy),
	CLK_LK(csi1_byte1_phy),
	CLK_LK(dsi0_pix_phy),
	CLK_LK(dsi1_pix_phy),
	CLK_LK(testdebug),
};
#endif // #if 0   /* ssg, bringup */

#if 0   /* ssg, bringup */
static void pwr_on_mm_subsystem(void)
{
	writel(0x1500, KONA_PWRMGR_VA + PWRMGR_SOFTWARE_1_VI_MM_POLICY_OFFSET); 
	writel(0x1, KONA_PWRMGR_VA + PWRMGR_SOFTWARE_1_EVENT_OFFSET);	
	mdelay(2);
}

static int clock_module_temp_fixes(void)
{
   struct clk *temp_clk;
   /***************************************************
    * Temp fixes for clocks in HUBAON CCU.
    **************************************************/
    /* Enable ACI APB clock */
    temp_clk = clk_get(NULL, "aci_apb_clk");
    clk_enable(temp_clk);
    clk_put(temp_clk);

    /*Set SIM clock rate to 26Mhz and enabale it */
    temp_clk = clk_get(NULL, "sim_clk");
    clk_set_rate(temp_clk, 26*CLOCK_1M);
    clk_enable(temp_clk);
    clk_put(temp_clk);
     /*Set SIM2 clock rate to 26Mhz and enabale it */
    temp_clk = clk_get(NULL, "sim2_clk");
    clk_set_rate(temp_clk, 26*CLOCK_1M);
    clk_enable(temp_clk);
    clk_put(temp_clk);
    /***************************************************
    * Temp fixes for clocks in HUB CCU.
    **************************************************/
    /*enable and then disable. so that HUB CCU enable gets called and masks
     * gating policy bits*/
    temp_clk = clk_get(NULL, "nor_apb_clk");
    clk_enable(temp_clk);
    clk_disable(temp_clk);
    clk_put(temp_clk);

    temp_clk = clk_get(NULL, "nor_clk");
    clk_disable(temp_clk);
    clk_put(temp_clk);

    temp_clk = clk_get(NULL, "audioh_apb_clk");
    clk_enable(temp_clk);
    clk_put(temp_clk);

    temp_clk = clk_get(NULL, "audioh_26m_clk");
    clk_enable(temp_clk);
    clk_put(temp_clk);

    temp_clk = clk_get(NULL, "audioh_2p4m_clk");
    clk_enable(temp_clk);
    clk_put(temp_clk);

    temp_clk = clk_get(NULL, "audioh_156m_clk");
    clk_enable(temp_clk);
    clk_put(temp_clk);

    temp_clk = clk_get(NULL, "ssp3_audio_clk");
    clk_set_rate(temp_clk, 52*CLOCK_1M);
    clk_enable(temp_clk);
    clk_put(temp_clk);

    temp_clk = clk_get(NULL, "ssp3_clk");
    clk_set_rate(temp_clk, 52*CLOCK_1M);
    clk_enable(temp_clk);
    clk_put(temp_clk);

    temp_clk = clk_get(NULL, "ssp3_apb_clk");
    clk_enable(temp_clk);
    clk_put(temp_clk);

    temp_clk = clk_get(NULL, "ssp4_audio_clk");
    clk_set_rate(temp_clk, 52*CLOCK_1M);
    clk_enable(temp_clk);
    clk_put(temp_clk);

    temp_clk = clk_get(NULL, "ssp4_clk");
    clk_set_rate(temp_clk, 52*CLOCK_1M);
    clk_enable(temp_clk);
    clk_put(temp_clk);

    temp_clk = clk_get(NULL, "ssp4_apb_clk");
    clk_enable(temp_clk);
    clk_put(temp_clk);
    /***************************************************
    * Temp fixes for clocks in Kona master CCU.
    **************************************************/
    temp_clk = clk_get(NULL, "usb_otg_clk");
    clk_enable(temp_clk);
    clk_put(temp_clk);

   /***************************************************
    * Temp fixes for clocks in Kona slave CCU.
    **************************************************/
    temp_clk = clk_get(NULL, "uartb_apb_clk");
    clk_enable(temp_clk);
    clk_put(temp_clk);

    temp_clk = clk_get(NULL, "uartb_clk");
    clk_enable(temp_clk);
    clk_put(temp_clk);

    temp_clk = clk_get(NULL, "uartb2_clk");
    clk_enable(temp_clk);
    clk_put(temp_clk);

    temp_clk = clk_get(NULL, "uartb2_apb_clk");
    clk_enable(temp_clk);
    clk_put(temp_clk);

    temp_clk = clk_get(NULL, "uartb3_clk");
    clk_enable(temp_clk);
    clk_put(temp_clk);

    temp_clk = clk_get(NULL, "uartb3_apb_clk");
    clk_enable(temp_clk);
    clk_put(temp_clk);

    temp_clk = clk_get(NULL, "ssp0_audio_clk");
    clk_enable(temp_clk);
    clk_put(temp_clk);

    temp_clk = clk_get(NULL, "dmac_mux_apb_clk");
    clk_enable(temp_clk);
    clk_put(temp_clk);

    temp_clk = clk_get(NULL, "bsc1_clk");
    clk_enable(temp_clk);
    clk_put(temp_clk);

    temp_clk = clk_get(NULL, "bsc1_apb_clk");
    clk_enable(temp_clk);
    clk_put(temp_clk);

    temp_clk = clk_get(NULL, "bsc2_clk");
    clk_enable(temp_clk);
    clk_put(temp_clk);

    temp_clk = clk_get(NULL, "bsc2_apb_clk");
    clk_enable(temp_clk);
    clk_put(temp_clk);

    temp_clk = clk_get(NULL, "pwm_clk");
    clk_enable(temp_clk);
    clk_put(temp_clk);

    temp_clk = clk_get(NULL, "pwm_apb_clk");
    clk_enable(temp_clk);
    clk_put(temp_clk);
   /***************************************************
    * Temp fixes for clocks in Kona slave CCU.
    **************************************************/
    temp_clk = clk_get(NULL, "mm_dma_axi");
    clk_enable(temp_clk);
    clk_put(temp_clk);

    temp_clk = clk_get(NULL, "csi0_axi");
    clk_enable(temp_clk);
    clk_put(temp_clk);

    temp_clk = clk_get(NULL, "csi0_lp");
    clk_set_rate(temp_clk, 100*CLOCK_1M);
    clk_enable(temp_clk);
    clk_put(temp_clk);

    temp_clk = clk_get(NULL, "csi1_axi");
    clk_enable(temp_clk);
    clk_put(temp_clk);

    temp_clk = clk_get(NULL, "csi1_lp");
    clk_set_rate(temp_clk, 100*CLOCK_1M);
    clk_enable(temp_clk);
    clk_put(temp_clk);

    temp_clk = clk_get(NULL, "isp_axi");
    clk_enable(temp_clk);
    clk_put(temp_clk);

    temp_clk = clk_get(NULL, "smi_axi");
    clk_enable(temp_clk);
    clk_put(temp_clk);

    temp_clk = clk_get(NULL, "smi");
    clk_enable(temp_clk);
    clk_put(temp_clk);

    temp_clk = clk_get(NULL, "v3d_axi");
    clk_enable(temp_clk);
    clk_put(temp_clk);

    temp_clk = clk_get(NULL, "vce_axi");
    clk_enable(temp_clk);
    clk_put(temp_clk);

    temp_clk = clk_get(NULL, "dsi0_axi");
    clk_enable(temp_clk);
    clk_put(temp_clk);

    temp_clk = clk_get(NULL, "dsi0_esc");
    clk_enable(temp_clk);
    clk_put(temp_clk);

    temp_clk = clk_get(NULL, "dsi1_axi");
    clk_enable(temp_clk);
    clk_put(temp_clk);

    temp_clk = clk_get(NULL, "dsi1_esc");
    clk_enable(temp_clk);
    clk_put(temp_clk);

    temp_clk = clk_get(NULL, "mm_apb");
    clk_enable(temp_clk);
    clk_put(temp_clk);

    temp_clk = clk_get(NULL, "spi_apb");
    clk_enable(temp_clk);
    clk_put(temp_clk);

    temp_clk = clk_get(NULL, "dsi_pll_o_dsi_pll");
    clk_enable(temp_clk);
    clk_put(temp_clk);


    return 0;
}
#endif // #if 0   /* ssg, bringup */


int __init clock_init(void)
{
#if 0   /* ssg, bringup */
	int i;

	pwr_on_mm_subsystem();

	for (i=0; i<ARRAY_SIZE(rhea_clk_tbl); i++)
	clk_register(&rhea_clk_tbl[i]);

     /*********************  TEMPORARY *************************************
     * Work arounds for clock module . this could be because of ASIC
     * errata or other limitations or special requirements.
     * -- To be revised based on future fixes.
     *********************************************************************/
    clock_module_temp_fixes();

#endif // #if 0   /* ssg, bringup */
	return 0;
}

int __init clock_late_init(void)
{
#if 0   /* ssg, bringup */
#ifdef CONFIG_DEBUG_FS
	int i;
	clock_debug_init();
	for (i=0; i<ARRAY_SIZE(rhea_clk_tbl); i++)
		clock_debug_add_clock (rhea_clk_tbl[i].clk);
#endif

#endif //#if 0   /* ssg, bringup */
	return 0;
}

late_initcall(clock_late_init);

unsigned long clock_get_xtal(void)
{
	return 26*CLOCK_1M;
}



/** 
 *	get_arm_register - get arm register using static arm_reg set by arm_reg_to_rw file 
 */
static int get_arm_register(void *data, u64 *val)
{
    int val2;
    void __iomem *base ;

	base = ioremap (arm_reg_baseaddr, SZ_4K);		   // phys base address and size

	if (!base)
	    return -ENOMEM;

	if( arm_reg_baseaddr < HUB_CLK_BASE_ADDR )	{			 
		clksamoa_dbg("%s: arm_reg_baseaddr is NO good = 0x%8X \n", __func__, arm_reg_baseaddr);
		iounmap(base);
		return -ENOMEM;
	}

	val2 = readl(base + arm_reg_offsetaddr);

	clksamoa_dbg("arm_reg_baseaddr = 0x%X, arm_reg_offsetaddr = 0x%8X \n", arm_reg_baseaddr, arm_reg_offsetaddr);
	clksamoa_dbg("%s: readout reg at addr 0x%8X value = 0x%8X \n", __func__, (arm_reg_baseaddr + arm_reg_offsetaddr), val2);

	*val = val2;
    iounmap (base);

    return 0;
}

/** 
 *	set_arm_register - set arm register using static arm_reg set by arm_reg_to_rw file 
 */
static int set_arm_register(void *data, u64 val)
{
    void __iomem *base ;

	/* if 0 addr, no good */
	if (!arm_reg_baseaddr)	{
		clksamoa_dbg("%s: ERROR: arm_reg_addr is 0 \n",__func__);
	    return -EACCES;
	}

	base = ioremap (arm_reg_baseaddr, SZ_4K);		   // phys base address and size

    if (val > 0) {
		clksamoa_dbg("%s: Setting arm register at address 0x%8X= 0x%8X \n",__func__, (arm_reg_baseaddr + arm_reg_offsetaddr), (int)val);
		writel(val, base + arm_reg_offsetaddr);									   
    }
    else
		clksamoa_dbg("Invalid value \n");

    iounmap (base);

    return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(arm_register_fops, get_arm_register, set_arm_register, "%llu\n");


/**
 * set_arm_reg_baseaddress - Setting arm reg OFFSET address with  arm_reg_baseaddress_fops
 */
static int set_arm_reg_baseaddress (void *data, u64 val)
{
    if (val >= HUB_CLK_BASE_ADDR) {
		clksamoa_dbg("Setting arm BASE register address at = 0x%8X  to be written or read\n", (int)val);
		arm_reg_baseaddr = val;
    }
    else
		clksamoa_dbg("Invalid value \n");

    return 0;
}

/**
 * get_arm_reg_baseaddress - Getting arm reg OFFSET address with  arm_reg_baseaddress_fops
 */
static int get_arm_reg_baseaddress (void *data, u64 *val)
{
	clksamoa_dbg("ARM register BASE address currently set at = 0x%8X  to be written or read\n", arm_reg_baseaddr);

    return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(arm_reg_baseaddress_fops, get_arm_reg_baseaddress, set_arm_reg_baseaddress, "%llu\n");


/**
 * set_arm_reg_offsetaddress - Setting arm reg OFFSET address
 */
static int set_arm_reg_offsetaddress (void *data, u64 val)
{
    if (val >= 0) {
		arm_reg_offsetaddr = val;
		clksamoa_dbg("Setting arm register OFFSET address to rw at = 0x%8X  \n", (int)val);
		clksamoa_dbg("Current arm register address to rw is = 0x%8X  \n", (int)(arm_reg_baseaddr + arm_reg_offsetaddr));
    }
    else
		clksamoa_dbg("Invalid value \n");

    return 0;
}

/**
 * get_arm_reg_offsetaddress - Setting arm reg OFFSET address with arm_reg_offsetaddress_fops
 */
static int get_arm_reg_offsetaddress (void *data, u64 *val)
{
	clksamoa_dbg("ARM register OFFSET address currently set at = 0x%8X  to be written or read\n", arm_reg_offsetaddr);

    return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(arm_reg_offsetaddress_fops, get_arm_reg_offsetaddress, set_arm_reg_offsetaddress, "%llu\n");



/**
 * clksamoa_debug_init - Init func for pmtest.
 */
int __init clksamoa_debug_init(void)
{
	clksamoa_dbg("%s  \n", __func__);

	/* create root clock dir /pmtest */
	dent_pmtest_root_dir = debugfs_create_dir("clksamoa_test", 0);
	if (!dent_pmtest_root_dir)
		return -ENOMEM;

	/* create read/write register capability */
	if (!debugfs_create_file("rw_armreg", 0644, dent_pmtest_root_dir, NULL, &arm_register_fops))
		return -ENOMEM;

	/* create arm reg address to read and write to */
	if (!debugfs_create_file("rw_base_addr_armreg", 0644, dent_pmtest_root_dir, NULL, &arm_reg_baseaddress_fops))
		return -ENOMEM;

	/* create arm reg address to read and write to */
	if (!debugfs_create_file("rw_offset_addr_armreg", 0644, dent_pmtest_root_dir, NULL, &arm_reg_offsetaddress_fops))
		return -ENOMEM;

	return 0;
}


late_initcall(clksamoa_debug_init);


