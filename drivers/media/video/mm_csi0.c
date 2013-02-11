
/*****************************************************************************
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

/*
 * Unicam camera host exports
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/vmalloc.h>
#include <linux/platform_device.h>
#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/delay.h>

#include <linux/fs.h>
#include <linux/syscalls.h>
#include <linux/videodev2.h>
#include <media/v4l2-common.h>
#include <media/v4l2-dev.h>
#include <media/videobuf2-dma-contig.h>
#include <media/soc_camera.h>
#include <media/soc_mediabus.h>
#include <asm/bitops.h>
#include <linux/spinlock.h>
#include <mach/rdb/brcm_rdb_util.h>
#include <mach/memory.h>
#include "mm_csi0.h"

struct mm_csi0_generic {
	int devbusy;
	enum host_mode mode;
	enum afe_num afe;
	enum csi2_lanes lanes;
	int db_en;
	int trigger;
	struct semaphore sem; /* Do a sema init on this at init time */
};

/* For now using local addresses */
/* This will be changed once the same is 
   available via platform driver */
#define V_BASE HW_IO_PHYS_TO_VIRT(MM_CSI0_BASE)
#define MM_CFG_BASE HW_IO_PHYS_TO_VIRT(0x3C004000)
#define MM_CLK_BASE HW_IO_PHYS_TO_VIRT(0x3C000000)

struct mm_csi0_generic cam_state = {
	0,
	INVALID,
};

static spinlock_t lock;
void reg_dump()
{
	u32 base = V_BASE;
	printk("  CAM_CTL 0x%x\n",BRCM_READ_REG(base, CAM_CTL));
	printk("  CAM_STA 0x%x\n",BRCM_READ_REG(base, CAM_STA));
	printk("  CAM_ANA 0x%x\n",BRCM_READ_REG(base, CAM_ANA));
	printk("  CAM_PRI 0x%x\n",BRCM_READ_REG(base, CAM_PRI));
	printk("  CAM_CLK 0x%x\n",BRCM_READ_REG(base, CAM_CLK));
	printk("  CAM_CLT 0x%x\n",BRCM_READ_REG(base, CAM_CLT));
	printk("  CAM_DAT0 0x%x\n",BRCM_READ_REG(base, CAM_DAT0));
	printk("  CAM_DAT1 0x%x\n",BRCM_READ_REG(base, CAM_DAT1));
	printk("  CAM_DLT 0x%x\n",BRCM_READ_REG(base, CAM_DLT));
	printk("  CAM_ICTL 0x%x\n",BRCM_READ_REG(base, CAM_ICTL));
	printk("  CAM_ISTA 0x%x\n",BRCM_READ_REG(base, CAM_ISTA));
	printk("  CAM_IDI 0x%x\n",BRCM_READ_REG(base, CAM_IDI));
	printk("  CAM_IPIPE 0x%x\n",BRCM_READ_REG(base, CAM_IPIPE));
	printk("  CAM_IBSA 0x%x\n",BRCM_READ_REG(base, CAM_IBSA));
	printk("  CAM_IBEA 0x%x\n",BRCM_READ_REG(base, CAM_IBEA));
	printk("  CAM_IBLS 0x%x\n",BRCM_READ_REG(base, CAM_IBLS));
	printk("  CAM_DCS 0x%x\n",BRCM_READ_REG(base, CAM_DCS));
	printk("  CAM_MISC 0x%x\n",BRCM_READ_REG(base, CAM_MISC));

	printk("MM CFG dump 0x%x*********\n", BRCM_READ_REG(MM_CFG_BASE, MM_CFG_CSI0_LDO_CTL));

	printk("MM CLK dump ************\n");
	base = MM_CLK_BASE;
	printk("   MM_CLK_MGR_REG_CSI0_PHY_DIV 0x%x\n",BRCM_READ_REG(base, MM_CLK_MGR_REG_CSI0_PHY_DIV));
	printk(" MM_CLK_MGR_REG_CSI0_DIV 0x%x\n",BRCM_READ_REG(base, MM_CLK_MGR_REG_CSI0_DIV));
	printk("  MM_CLK_MGR_REG_CSI0_LP_CLKGATE 0x%x\n",BRCM_READ_REG(base,MM_CLK_MGR_REG_CSI0_LP_CLKGATE ));
	printk("MM_CLK_MGR_REG_CSI0_AXI_CLKGATE  0x%x\n",BRCM_READ_REG(base, MM_CLK_MGR_REG_CSI0_AXI_CLKGATE));
	printk("MM_CLK_MGR_REG_DIV_TRIG  0x%x\n",BRCM_READ_REG(base, MM_CLK_MGR_REG_DIV_TRIG));

}

void * get_mm_csi0_handle (enum host_mode mode, enum afe_num afe, enum csi2_lanes lanes)
{
	int ret = 0;
	unsigned long flags;
	spin_lock_init(&lock);
	spin_lock_irqsave(&lock,flags);
	if((cam_state.devbusy == 1) || (cam_state.mode != INVALID) ){
		printk(KERN_ERR "dev busy or mode active\n");
		goto out;
	}
	if((mode != CSI2) && (mode != CSI1CCP2)){
		printk(KERN_ERR "Wrong mode specified \n");
		goto out;
	}
	if((afe != AFE0) && (afe != AFE1)){
		printk(KERN_ERR "Wrong AFE specified \n");
		goto out;
	}
	if(mode == CSI2){
		if((afe == AFE1) && (lanes == CSI2_DUAL_LANE)){
			printk("Cannot support dual lane CSI2 on AFE1\n");
			goto out;
		}
	}
	cam_state.devbusy = 1;
	cam_state.mode = mode;
	cam_state.afe = afe;
	if (mode == CSI2)
		cam_state.lanes = lanes;
	spin_unlock_irqrestore(&lock,flags);	
	return (void *)&cam_state;
out:    	
	spin_unlock_irqrestore(&lock,flags);	
	return NULL;
}

int mm_csi0_init ()
{
	u32 base = MM_CFG_BASE;
	int ret = 0;
	/* LDO Enable */
	/* point base to mm_cfg base address */
	if(cam_state.afe == AFE0){
		BRCM_WRITE_REG_FIELD(base, MM_CFG_CSI0_PHY_CTRL, CSI_PHY_SEL, 0);
		BRCM_WRITE_REG(base, MM_CFG_CSI0_LDO_CTL, 0x5A00000F);
	} else if (cam_state.afe == AFE1){
		BRCM_WRITE_REG_FIELD(base, MM_CFG_CSI0_PHY_CTRL, CSI_PHY_SEL, 1);
		BRCM_WRITE_REG(base, MM_CFG_CSI1_LDO_CTL, 0x5A00000F);
	} else {
		printk(KERN_INFO "Wrong AFE specified in Enable\n");
		ret = -EINVAL;
	}
	/* To check Enable all memories within mm_csi0*/
	return ret;

}

int mm_csi0_teardown ()
{
	u32 base = MM_CFG_BASE;
	/* LDO Disable */
	/* point base to mm_cfg base address */
	unsigned long flags;
	spin_lock_irqsave(&lock,flags);
	if(cam_state.afe == AFE0){
		BRCM_WRITE_REG(base, MM_CFG_CSI0_LDO_CTL, 0x0);
	} else if (cam_state.afe == AFE1){
		BRCM_WRITE_REG(base, MM_CFG_CSI1_LDO_CTL, 0x0);
	} else {
		printk(KERN_INFO "Wrong AFE specified in Teardown\n");
	}
        BRCM_WRITE_REG(MM_CLK_BASE,MM_CLK_MGR_REG_WR_ACCESS,0xA5A501); // enable access
        BRCM_WRITE_REG(MM_CLK_BASE,MM_CLK_MGR_REG_CSI0_LP_CLKGATE, 0x00000302);  // default value
        BRCM_WRITE_REG(MM_CLK_BASE,MM_CLK_MGR_REG_CSI0_AXI_CLKGATE, 0x0000302);  // ...
	memset(&cam_state,0x0,sizeof(struct mm_csi0_generic));
	cam_state.devbusy = 0;
	cam_state.mode = INVALID;
	spin_unlock_irqrestore(&lock,flags);	
	printk("Teardown!!\n");
	/* To check Enable all memories within mm_csi0*/
	return 0;

}

/* Mostly AFE selection and clocks */
int mm_csi0_set_afe ()
{
	u32 base = MM_CLK_BASE;
	int term = 1;
	/* Enable access ... need to check how or why ?? */	
	 BRCM_WRITE_REG(base,MM_CLK_MGR_REG_WR_ACCESS,0xA5A501);
	if (cam_state.afe == AFE0){ 
		BRCM_WRITE_REG(MM_CFG_BASE, MM_CFG_CSI0_LDO_CTL, 0x5A00000F);
		BRCM_WRITE_REG(base, MM_CLK_MGR_REG_CSI0_PHY_DIV, 0x00000888);
		BRCM_WRITE_REG(base, MM_CLK_MGR_REG_CSI0_DIV, 0x00000040);
		BRCM_WRITE_REG(base, MM_CLK_MGR_REG_CSI0_LP_CLKGATE, 0x00000303);
		BRCM_WRITE_REG(base, MM_CLK_MGR_REG_CSI0_AXI_CLKGATE, 0x00000303);
		BRCM_WRITE_REG(base, MM_CLK_MGR_REG_DIV_TRIG, (1 << MM_CLK_MGR_REG_DIV_TRIG_CSI0_LP_TRIGGER_SHIFT));
	} else {
		BRCM_WRITE_REG(MM_CFG_BASE, MM_CFG_CSI1_LDO_CTL, 0x5A00000F);
		BRCM_WRITE_REG(base, MM_CLK_MGR_REG_CSI1_PHY_DIV, 0x00000888);
		BRCM_WRITE_REG(base, MM_CLK_MGR_REG_CSI1_DIV, 0x00000040);
		BRCM_WRITE_REG(base, MM_CLK_MGR_REG_CSI1_LP_CLKGATE, 0x00000303);
		BRCM_WRITE_REG(base, MM_CLK_MGR_REG_CSI1_AXI_CLKGATE, 0x00000303);
		BRCM_WRITE_REG(base, MM_CLK_MGR_REG_DIV_TRIG, (1 << MM_CLK_MGR_REG_DIV_TRIG_CSI1_LP_TRIGGER_SHIFT));
		BRCM_WRITE_REG(base, MM_CLK_MGR_REG_CSI0_DIV, 0x00000040);
		BRCM_WRITE_REG(base, MM_CLK_MGR_REG_CSI0_LP_CLKGATE, 0x00000303);
		BRCM_WRITE_REG(base, MM_CLK_MGR_REG_CSI0_AXI_CLKGATE, 0x00000303);
		BRCM_WRITE_REG(base, MM_CLK_MGR_REG_DIV_TRIG, (1 << MM_CLK_MGR_REG_DIV_TRIG_CSI0_LP_TRIGGER_SHIFT));
	}
	/* CSI1 uses CSI0 clocks and dividers */

	/* point base to mm_csi0 base */
	base = V_BASE;
	BRCM_WRITE_REG_FIELD(base, CAM_CTL, MEN, 0x1);

	BRCM_WRITE_REG(base, CAM_CLK, 0x00);
	BRCM_WRITE_REG(base, CAM_CLT, 0x00);
	BRCM_WRITE_REG(base, CAM_DAT0, 0x00);
	BRCM_WRITE_REG(base, CAM_DAT1, 0x00);
	BRCM_WRITE_REG(base, CAM_DLT, 0x00);

	BRCM_WRITE_REG_FIELD(base, CAM_CTL, CPR, 1);
	udelay(5);
	BRCM_WRITE_REG_FIELD(base, CAM_CTL, CPR, 0);
	BRCM_WRITE_REG_FIELD(base, CAM_ANA, PTATADJ, 0x7);
	BRCM_WRITE_REG_FIELD(base, CAM_ANA, CTATADJ, 0x7);

	BRCM_WRITE_REG_FIELD(base, CAM_ANA, APD, 0x0);
	BRCM_WRITE_REG_FIELD(base, CAM_ANA, BPD, 0x0);
	BRCM_WRITE_REG_FIELD(base, CAM_ANA, DDL, 0x0);
	/* Waiting for analog PHY LDO to settle */
	udelay(20);
	BRCM_WRITE_REG_FIELD(base, CAM_ANA, AR, 0x1);
	udelay(100);
	BRCM_WRITE_REG_FIELD(base, CAM_ANA, AR, 0x0);

	BRCM_WRITE_REG(base, CAM_IPIPE, 0x00);
	BRCM_WRITE_REG(base, CAM_CMP0, 0x00);
	BRCM_WRITE_REG(base, CAM_CMP1, 0x00);

	BRCM_WRITE_REG_FIELD(base, CAM_CLK, CLPD, 0x0);
	BRCM_WRITE_REG_FIELD(base, CAM_DAT0, DLPDN, 0x0);
	if(cam_state.lanes == CSI2_DUAL_LANE){
		BRCM_WRITE_REG_FIELD(base, CAM_DAT1, DLPDN, 0x0);
	} else {
		BRCM_WRITE_REG_FIELD(base, CAM_DAT1, DLPDN, 0x1);
	}
	if(cam_state.mode == CSI2)
		term = 0;
	/* HS */	
	BRCM_WRITE_REG_FIELD(base, CAM_CLK, CLHSE, 1);
	BRCM_WRITE_REG_FIELD(base, CAM_DAT0, DLHSEN, term);
	if(cam_state.lanes == CSI2_DUAL_LANE){
		BRCM_WRITE_REG_FIELD(base, CAM_DAT1, DLHSEN, term);
	}
	/* LP */
	if (cam_state.mode == CSI1CCP2){
		BRCM_WRITE_REG_FIELD(base, CAM_CLK, CLLPE, 0x0);
		BRCM_WRITE_REG_FIELD(base, CAM_DAT0, DLLPEN, 0x0);
		if(cam_state.lanes == CSI2_DUAL_LANE){
			BRCM_WRITE_REG_FIELD(base, CAM_DAT1, DLLPEN, 0x0);
		}
	} else { /* CSI-2 */
		BRCM_WRITE_REG_FIELD(base, CAM_CLK, CLLPE, 0x1);
		BRCM_WRITE_REG_FIELD(base, CAM_DAT0, DLLPEN, 0x1);
		if(cam_state.lanes == CSI2_DUAL_LANE){
			BRCM_WRITE_REG_FIELD(base, CAM_DAT1, DLLPEN, 0x1);
		}
	}
	/* Now enable only clock lane */
	BRCM_WRITE_REG_FIELD(base, CAM_CLK, CLE, 0x1);
	return 0;	
}


int mm_csi0_set_dig_phy (struct lane_timing *timing)
{
	u32 base = V_BASE;
	int term = 1;
	if(timing == NULL)
		return -EINVAL;
	if(cam_state.mode == CSI2)
		term = 0;
	/* Digital PHY setup */	
	if(cam_state.mode == CSI1CCP2){	
		BRCM_WRITE_REG_FIELD(base, CAM_CLT, CLT1, 0x6);		
		BRCM_WRITE_REG_FIELD(base, CAM_CLT, CLT2, 0x6);		

		BRCM_WRITE_REG_FIELD(base, CAM_DLT, DLT1, 0x0);		
		BRCM_WRITE_REG_FIELD(base, CAM_DLT, DLT2, 0x6);		
		BRCM_WRITE_REG_FIELD(base, CAM_DLT, DLT3, 0x0);	

	} else {
		BRCM_WRITE_REG_FIELD(base, CAM_CLT, CLT1, 0xA);
		BRCM_WRITE_REG_FIELD(base, CAM_CLT, CLT2,  0x3C);

		BRCM_WRITE_REG_FIELD(base, CAM_DLT, DLT1, 0x8);
		BRCM_WRITE_REG_FIELD(base, CAM_DLT, DLT2, 0x0);
		/* DLT3 has always been zero */
		BRCM_WRITE_REG_FIELD(base, CAM_DLT, DLT3, 0x0);
		/* New value from Venky */
		/* CLT1 0xA CLT2 0x3C*/
		/* DLT1 0x8 DLT2 and DLT3 = 0 */
	}
	/* The OV5640 driver to send the correct timings */
	BRCM_WRITE_REG_FIELD(base, CAM_CLK, CLTRE, term);
	BRCM_WRITE_REG_FIELD(base, CAM_DAT0, DLTREN, term);
	if(cam_state.lanes == CSI2_DUAL_LANE){
		BRCM_WRITE_REG_FIELD(base, CAM_DAT1, DLTREN, term);
	}
	BRCM_WRITE_REG_FIELD(base, CAM_DAT0, DLSMN, 0x0);
	if(cam_state.lanes == CSI2_DUAL_LANE)
		BRCM_WRITE_REG_FIELD(base, CAM_DAT1, DLSMN, 0x0);
	/* Packet framer timeout */
	BRCM_WRITE_REG_FIELD(base, CAM_CTL, PFT, 0xF);
	BRCM_WRITE_REG_FIELD(base, CAM_CTL, OET, 0xFF);
	return 0;	
}

int mm_csi0_set_mode (enum csi1ccp2_clock_mode clk_mode)
{
	u32 base = V_BASE;
	/* As per the recommended seq, we set the mode over here */
	BRCM_WRITE_REG_FIELD(base, CAM_CTL, CPM, cam_state.mode);
	if(cam_state.mode == CSI1CCP2){
		if((clk_mode != DATA_CLOCK) && (clk_mode != DATA_STROBE))
			clk_mode = DATA_CLOCK;
		BRCM_WRITE_REG_FIELD(base, CAM_CTL, DCM, clk_mode);
		printk("Setting CCP2 clock mode %d\n", clk_mode);
	}
	return 0;	
}

int mm_csi0_cfg_image_id (int vc, int id)
{
	u32 base = V_BASE;
	if( cam_state.mode == CSI1CCP2){
		if (id > 15){
			printk("Wrong CCP2 ID\n");
			return -EINVAL;
		}
		BRCM_WRITE_REG_FIELD(base, CAM_IDI, IDI0, (0x80 | id));

	} else { /* CSI2 mode */
		if (vc > 3){
			printk("Wrong CSI2 VC\n");
			return -EINVAL;
		}
		BRCM_WRITE_REG_FIELD(base, CAM_IDI, IDI0, ((vc << 6) | id));
	}
	return 0;
}

int mm_csi0_set_windowing_vertical (int v_line_start, int v_line_end)
{
	u32 base = V_BASE;
	BRCM_WRITE_REG(base, CAM_IVWIN, ((v_line_end << 16) | (v_line_start )));
	return 0;
}

int mm_csi0_set_windowing_horizontal (int h_pix_start, int h_pix_end)
{
	u32 base = V_BASE;
	BRCM_WRITE_REG(base, CAM_IHWIN, ((h_pix_end << 16) | (h_pix_start)));
	return 0;
}

int mm_csi0_enc_blk_length (int len)
{
	 u32 base = V_BASE;
	 u8 val;
	 if (len == 0)
		 BRCM_WRITE_REG_FIELD(base, CAM_IPIPE, DEBL, 0x0);
	else {
		if( ((len & (len - 1)) != 0) || (len > 512 ) || (len < 8)){
			printk(KERN_INFO "Wrong encode block length\n");
			return -EINVAL;
		}
		val = __ffs(len) - 2; /* Validate this change */
		BRCM_WRITE_REG_FIELD(base, CAM_IPIPE, DEBL, val);
	}
	return 0;
}

int mm_csi0_cfg_pipeline_dpcm_enc (enum dpcm_encode_mode enc)
{
	u32 base = V_BASE;
	if( (enc & ~0x3) != 0 ) {
		printk(KERN_INFO "Wrong DPCM encode mode\n");
		return -EINVAL;
	}
	BRCM_WRITE_REG_FIELD(base, CAM_IPIPE, DEM, enc);
	return 0;
}

int mm_csi0_cfg_pipeline_dpcm_dec (enum dpcm_decode_mode dec)
{
	u32 base = V_BASE;
	if( (dec < 0) || (dec > 11)){
		printk(KERN_INFO "Wrong DPCM decode mode\n");
		return -EINVAL;
	}
	BRCM_WRITE_REG_FIELD(base, CAM_IPIPE, DDM, dec);
	return 0;
}

int mm_csi0_cfg_pipeline_pack (enum pix_pack_mode pack)
{
	u32 base = V_BASE;
	if( (pack & ~0x7) != 0 ){
		printk("Wrong pix pack mode\n");
		return -EINVAL;
	}
	BRCM_WRITE_REG_FIELD(base, CAM_IPIPE, PPM, pack);
	return 0;
}

int mm_csi0_cfg_pipeline_unpack (enum pix_unpack_mode unpack)
{
	u32 base = V_BASE;
	if( (unpack & ~0x7) != 0 ){
		printk("Wrong pix unpack mode\n");
		return -EINVAL;
	}
	BRCM_WRITE_REG_FIELD(base, CAM_IPIPE, PUM, unpack);
	return 0;
}

int mm_csi0_enable_fsp_ccp2 (void)
{
	u32 base = V_BASE;
	if( cam_state.mode == CSI1CCP2 ){
		BRCM_WRITE_REG_FIELD(base, CAM_DCS, FDE, 0x1);
	} else {
		BRCM_WRITE_REG_FIELD(base, CAM_DCS, FDE, 0x0);
	}
	return 0;
}

int mm_csi0_config_int (struct int_desc *desc, enum buffer_type type)
{
	u32 base = V_BASE;
	if (type == IMAGE_BUFFER) {
		if(desc->fsi)
			 BRCM_WRITE_REG_FIELD(base, CAM_ICTL, FSIE, 0x1);
		else	 
			 BRCM_WRITE_REG_FIELD(base, CAM_ICTL, FSIE, 0x0);
		if(desc->fei)
			 BRCM_WRITE_REG_FIELD(base, CAM_ICTL, FEIE, 0x1);
		else	 
			 BRCM_WRITE_REG_FIELD(base, CAM_ICTL, FEIE, 0x0);
		if(desc->lci)	 
			 BRCM_WRITE_REG_FIELD(base, CAM_ICTL, LCIE, desc->lci);
		else	 
			 BRCM_WRITE_REG_FIELD(base, CAM_ICTL, LCIE, 0);
		/* The CSI2 on UNICAM cannot have EDL=0 due to a limitation 
		   Not sure where to address this                        */	 
	} else if (type == DATA_BUFFER) {
		/* For data buffer there is one interrupt */
		/* The interrupt can be triggered on an FE or embedded data line end */
		/* fei is used to say if interrupt is on an FE else interrupt on EDL 
		   from desc->dataline */
		/* For data if FE is not required, pl provide the number of lines of data 
		   properly */   
		   printk("Data buffer set\n");
		if(desc->die){
			 /*BRCM_WRITE_REG_FIELD(base, CAM_DCS, DIE, 0x1);
			 if( desc->fei) {
				 BRCM_WRITE_REG_FIELD(base, CAM_DCS, DIM, 0x0);
			 }
			 else {
				 BRCM_WRITE_REG_FIELD(base, CAM_DCS, DIM, 0x0);
				 BRCM_WRITE_REG_FIELD(base, CAM_DCS, EDL, desc->dataline);
			 }*/
			 BRCM_WRITE_REG_FIELD(base, CAM_DCS, EDL, desc->dataline);
		} else {
			 BRCM_WRITE_REG_FIELD(base, CAM_DCS, DIE, 0x0);
		}
	} else {
		printk(KERN_INFO "Invalid buffer type\n");
		return -EINVAL;
	}
	return 0;
}

int mm_csi0_get_int_stat (struct int_desc *desc, int ack)
{
	u32 base = V_BASE;
	u32 reg;
	if(desc == NULL){
		printk(KERN_INFO "Null descriptor\n");
		return -EINVAL;
	}
	reg = BRCM_READ_REG(base, CAM_ISTA);
	if(ack)
		BRCM_WRITE_REG (base, CAM_ISTA, reg);
	if (reg & CAM_ISTA_FSI_MASK)
		desc->fsi = 1;
	if (reg & CAM_ISTA_FEI_MASK)	
		desc->fei = 1;
	if (reg & CAM_ISTA_LCI_MASK)	
		desc->lci = 1;
	return 0;	
}

int mm_csi0_get_data_stat (struct int_desc *desc, int ack)
{
	u32 base = V_BASE;
	if(desc == NULL){
		printk(KERN_INFO "Null descriptor data\n");
		return -EINVAL;
	}
	if (BRCM_READ_REG_FIELD(base, CAM_DCS, DI) & CAM_DCS_DI_MASK ){
		if(ack)
			BRCM_WRITE_REG_FIELD(base, CAM_DCS, DI, 0x1);
		desc->die = 1;
		/* The caller knows whether the interrupt was FE or EDL */
	}
	return 0;
}

u32 mm_csi0_get_rx_stat (struct rx_stat_list *list, int ack)
{
	u32 base = V_BASE;
	u32 reg;
	if (list == NULL){
		printk(KERN_INFO "Null descriptor for rx ack\n");
		return 0;
	}
	reg = BRCM_READ_REG(base, CAM_STA);
	if (ack)
		BRCM_WRITE_REG(base, CAM_STA, reg);
	if (reg & CAM_STA_SYN_MASK)
		list->syn = 1;
	if (reg & CAM_STA_CS_MASK)
		list->clk_hs_present = 1;
	if (cam_state.mode == CSI2){	
		if (reg & CAM_STA_SBE_MASK)	
			list->sbe = 1;
		if (reg & CAM_STA_PBE_MASK)	
			list->pbe = 1;
		if (reg & CAM_STA_HOE_MASK)	
			list->hoe = 1;
		if (reg & CAM_STA_PLE_MASK)	
			list->ple = 1;
	}
	if (cam_state.mode == CSI1CCP2)
		if (reg & CAM_STA_SSC_MASK)	
			list->ple = 1;
	if (reg & CAM_STA_CRCE_MASK)	
		list->crce = 1;
	if (reg & CAM_STA_OES_MASK)	
		list->oes = 1;
	if (reg & CAM_STA_IFO_MASK)	
		list->ifo = 1;
	if (reg & CAM_STA_OFO_MASK)	
		list->ofo = 1;
	if (reg & CAM_STA_BFO_MASK)	
		list->bfo = 1;
	if (reg & CAM_STA_DL_MASK)	
		list->dl = 1;
	if (reg & CAM_STA_PS_MASK)	
		list->ps = 1;
	if (reg & CAM_STA_IS_MASK)	
		list->is = 1;
	return reg;
}

int mm_csi0_buffering_mode (enum buffer_mode bmode)
{
	if(BUFFER_DOUBLE == bmode){
		cam_state.db_en = 1;
		cam_state.trigger = 0;
	} else {	
		cam_state.db_en = 0;
		if (bmode == BUFFER_TRIGGER)
			cam_state.trigger = 1;
		else	
			cam_state.trigger = 0;
	}
	return 0;	
}

int mm_csi0_update_addr (struct buffer_desc *im0, struct buffer_desc *im1, struct buffer_desc *dat0, struct buffer_desc *dat1)
{
	u32 base = V_BASE;
	int data = 1;
	if(im0 == NULL){
		printk("IMO is NULL\n");
		return -EINVAL;
	}
	if(cam_state.db_en && (im1 == NULL)){
		printk("IM1 is NULL but DB_EN is set\n");
		return -EINVAL;
	}
	/* Image buffer updates */
	if ((im0->start & 0xF) || ((im0->start + im0->size) & 0xF) || (im0->ls & 0xF)){
		printk("IM0 is not properly alligned\n");
		return -EINVAL;
	}
	if ( cam_state.db_en && ((im1->start & 0xF) || ((im1->start + im1->size) & 0xF) || (im1->ls & 0xF))){
		printk("IM1 is not properly alligned\n");
		return -EINVAL;
	}
	if (dat0 == NULL)
		data = 0;
	else {	
		if (cam_state.db_en && (dat1 == NULL)){
			printk("Data DB set, DAT0 set but no DAT1\n");
			/* Not sure if this is an error or we can ignore data 
			   For now disabling data */
			   data = 0;
		}
		if (cam_state.db_en && dat1)
			data = 2;
	}
	if (data){
		if ((dat0->start & 0xF) || ((dat0->start + dat0->size) & 0xF) || (dat0->ls & 0xF)){
			data = 0;
			printk("Disabling data as buffer not alligned\n");
		}
		if ((data == 2) && ((dat1->start & 0xF) || ((dat1->start + dat1->size) & 0xF) || (dat1->ls & 0xF))){
			data = 0;
			printk("Disabling data 2 as buffer not alligned\n");
		}
	}
	BRCM_WRITE_REG_FIELD(base, CAM_DBCTL, DB_EN, 0); 
	BRCM_WRITE_REG_FIELD(base, CAM_DBCTL, BUF0_IE, 0); 
	BRCM_WRITE_REG_FIELD(base, CAM_DBCTL, BUF1_IE, 0); 
	BRCM_WRITE_REG_FIELD(base, CAM_MISC,  DIS_DB_IE, 1);
	if(cam_state.trigger)
		BRCM_WRITE_REG_FIELD (base, CAM_ICTL, FCM, 0x1);
	else	
		BRCM_WRITE_REG_FIELD (base, CAM_ICTL, FCM, 0x0);
	/* Check the logic above again */
	/* register prog start */
	if (cam_state.db_en)
		BRCM_WRITE_REG_FIELD(base, CAM_DBCTL, DB_EN, 1); 
	if(im0){
		BRCM_WRITE_REG(base, CAM_IBSA, im0->start);
		BRCM_WRITE_REG(base, CAM_IBEA, (im0->start + im0->size));
		BRCM_WRITE_REG_FIELD(base, CAM_IBLS, IBLS, im0->ls);
	}
	if(im1){
		BRCM_WRITE_REG(base, CAM_IBSA1, im1->start);
		BRCM_WRITE_REG(base, CAM_IBEA1, (im1->start + im1->size));
	}
	BRCM_WRITE_REG_FIELD(base, CAM_ICTL, LIP, 1); 
	if (data){
		if (data == 1) {
			BRCM_WRITE_REG(base, CAM_DBSA, dat0->start);
			BRCM_WRITE_REG(base, CAM_DBEA, (dat0->start + dat0->size));
		}
		if (data == 2) {
			BRCM_WRITE_REG(base, CAM_DBSA1, dat1->start);
			BRCM_WRITE_REG(base, CAM_DBEA1, (dat1->start + dat1->size));
		}
		BRCM_WRITE_REG_FIELD(base, CAM_DCS, LDP, 1); 
	}
	if(cam_state.db_en){
		BRCM_WRITE_REG_FIELD(base, CAM_DBCTL, DB_EN, 1); 
		BRCM_WRITE_REG_FIELD(base, CAM_DBCTL, BUF0_IE, 1); 
		BRCM_WRITE_REG_FIELD(base, CAM_DBCTL, BUF1_IE, 1); 
		BRCM_WRITE_REG_FIELD(base, CAM_MISC,  DIS_DB_IE, 0); 
	} else {
		BRCM_WRITE_REG_FIELD(base, CAM_DBCTL, DB_EN, 0); 
		BRCM_WRITE_REG_FIELD(base, CAM_DBCTL, BUF0_IE, 0); 
		BRCM_WRITE_REG_FIELD(base, CAM_DBCTL, BUF1_IE, 0); 
		BRCM_WRITE_REG_FIELD(base, CAM_MISC,  DIS_DB_IE, 1); 
	}
	return 0;
}

int mm_csi0_update_one(struct buffer_desc *im, int buf_num, enum buffer_type type)
{
	u32 base = V_BASE;
	if (im == NULL){
		printk(KERN_INFO "Null passed\n");
		return -EINVAL;
	}
	if ((im->start & 0xF) || ((im->start + im->size) & 0xF) || (im->ls & 0xF)){
		printk(KERN_INFO "Wrong allignment\n");
		return -EINVAL;
	}
	if (type == IMAGE_BUFFER) {
		if (buf_num == 0){
			BRCM_WRITE_REG(base, CAM_IBSA, im->start);
			BRCM_WRITE_REG(base, CAM_IBEA, (im->start + im->size));
			BRCM_WRITE_REG(base, CAM_IBLS, im->ls);
			BRCM_WRITE_REG_FIELD(base, CAM_STA, BUF1_RDY, 1);
			BRCM_WRITE_REG_FIELD(base, CAM_STA, BUF0_RDY, 0);
		} else {
			BRCM_WRITE_REG(base, CAM_IBSA1, im->start);
			BRCM_WRITE_REG(base, CAM_IBEA1, (im->start + im->size));
			BRCM_WRITE_REG(base, CAM_IBLS, im->ls);
			BRCM_WRITE_REG_FIELD(base, CAM_STA, BUF0_RDY, 1);
			BRCM_WRITE_REG_FIELD(base, CAM_STA, BUF1_RDY, 0);
		}
	} else {
		/* Data buffer */
		if (buf_num == 0){
			BRCM_WRITE_REG(base, CAM_DBSA, im->start);
			BRCM_WRITE_REG(base, CAM_DBEA, (im->start + im->size));
		} else {
			BRCM_WRITE_REG(base, CAM_DBSA1, im->start);
			BRCM_WRITE_REG(base, CAM_DBEA1, (im->start + im->size));
		}
	}
	BRCM_WRITE_REG_FIELD(base, CAM_ICTL, LIP, 1); 
	if(cam_state.db_en){
		BRCM_WRITE_REG_FIELD(base, CAM_DBCTL, DB_EN, 1); 
		BRCM_WRITE_REG_FIELD(base, CAM_DBCTL, BUF0_IE, 1); 
		BRCM_WRITE_REG_FIELD(base, CAM_DBCTL, BUF1_IE, 1); 
		BRCM_WRITE_REG_FIELD(base, CAM_MISC,  DIS_DB_IE, 0); 
	} else {
		BRCM_WRITE_REG_FIELD(base, CAM_DBCTL, DB_EN, 0); 
		BRCM_WRITE_REG_FIELD(base, CAM_DBCTL, BUF0_IE, 0); 
		BRCM_WRITE_REG_FIELD(base, CAM_DBCTL, BUF1_IE, 0); 
		BRCM_WRITE_REG_FIELD(base, CAM_MISC,  DIS_DB_IE, 1); 
	}
	return 0;
}

int mm_csi0_trigger_cap(void)
{
	u32 base = V_BASE;
	if (cam_state.trigger){
		BRCM_WRITE_REG_FIELD (base, CAM_ICTL, FCM, 0x1);
		BRCM_WRITE_REG_FIELD (base, CAM_ICTL, TFC, 0x1);
		return 0;
	} else {
		return -EINVAL;
	}
}

int mm_csi0_rx_burst()
{
	u32 base = MM_CFG_BASE;
	u32 val_sw;
	val_sw = readl((base + 0x444));
	val_sw = val_sw | 1;
	writel(val_sw, (base + 0x444));
	/* For QoS enable */
	base = V_BASE;
	BRCM_WRITE_REG_FIELD(base, CAM_PRI, BL, 0);
	BRCM_WRITE_REG_FIELD(base, CAM_PRI, BS, 0);
	BRCM_WRITE_REG_FIELD(base, CAM_PRI, PT, 2);
	BRCM_WRITE_REG_FIELD(base, CAM_PRI, PP, 0xf);
	BRCM_WRITE_REG_FIELD(base, CAM_PRI, NP, 0);
	/* Enabling panic enable */
	BRCM_WRITE_REG_FIELD(base, CAM_PRI, PE, 1);
	return 0;
}

int mm_csi0_start_rx (void)
{
	u32 base = V_BASE;
	if(cam_state.trigger)
		BRCM_WRITE_REG_FIELD (base, CAM_ICTL, FCM, 0x1);
	else	
		BRCM_WRITE_REG_FIELD (base, CAM_ICTL, FCM, 0x0);
	BRCM_WRITE_REG_FIELD (base,CAM_ANA, DDL, 1);

	BRCM_WRITE_REG_FIELD (base,CAM_CLK, CLPD, 0);
	BRCM_WRITE_REG_FIELD (base,CAM_CLK, CLE, 1);
	/* analog reset */
	BRCM_WRITE_REG_FIELD(base,CAM_ANA,AR,1);
	BRCM_WRITE_REG_FIELD (base,CAM_CTL, CPE, 1);

	BRCM_WRITE_REG_FIELD(base,CAM_CTL,CPR,1);
	udelay(1);
	BRCM_WRITE_REG_FIELD(base,CAM_CTL,CPR,0);
	if(cam_state.db_en){
		BRCM_WRITE_REG_FIELD(base, CAM_DBCTL, DB_EN, 1); 
		BRCM_WRITE_REG_FIELD(base, CAM_DBCTL, BUF0_IE, 1); 
		BRCM_WRITE_REG_FIELD(base, CAM_DBCTL, BUF1_IE, 1); 
		BRCM_WRITE_REG_FIELD(base, CAM_MISC,  DIS_DB_IE, 0); 
	}
	mm_csi0_rx_burst();
	BRCM_WRITE_REG_FIELD(base, CAM_ICTL, LIP, 1); 
	BRCM_WRITE_REG_FIELD(base, CAM_DCS, LDP, 1); 
	BRCM_WRITE_REG_FIELD (base,CAM_DAT0, DLEN, 1);
	if(cam_state.lanes == CSI2_DUAL_LANE){
		BRCM_WRITE_REG_FIELD(base, CAM_DAT1, DLEN, 0x1);
	}
	BRCM_WRITE_REG_FIELD(base,CAM_ANA,AR,0);
	BRCM_WRITE_REG_FIELD (base,CAM_CTL, SOE, 0);
	BRCM_WRITE_REG_FIELD (base,CAM_ANA, DDL, 0);
	udelay(5);
	return 0;
}

int mm_csi0_stop_rx (void)
{
	u32 base = V_BASE;
	BRCM_WRITE_REG_FIELD(base,CAM_ANA,AR,1);

	BRCM_WRITE_REG_FIELD(base,CAM_CTL,CPR,1);
	udelay(1);
	BRCM_WRITE_REG_FIELD(base,CAM_CTL,CPR,0);

	BRCM_WRITE_REG_FIELD (base,CAM_ANA, DDL, 1);
	BRCM_WRITE_REG_FIELD (base,CAM_DAT0, DLEN, 0);
	BRCM_WRITE_REG_FIELD (base, CAM_DAT1, DLEN, 0);
	BRCM_WRITE_REG (base, CAM_DCS, 0x0);
	BRCM_WRITE_REG (base, CAM_IBSA, 0x0);
	BRCM_WRITE_REG (base, CAM_IBEA, 0x0);
	BRCM_WRITE_REG (base, CAM_IBLS, 0x0);
	BRCM_WRITE_REG_FIELD (base,CAM_CTL, CPE, 0);
	return 0;
}

void mm_csi0_ibwp()
{
	u32 base = V_BASE;
	u32 val,baseval;
	val = BRCM_READ_REG(base, CAM_IBWP);
	baseval = BRCM_READ_REG(base, CAM_IBSA);
	printk("IBWP 0x%x\n",(val - baseval));
	BRCM_WRITE_REG(base, CAM_IBWP, 0x00);
}

/* Packet compare and capture */

/* This feature applies only to CSI2 */
/* Will scan all incoming packets and captures a packet with
   matching credentials. Used to debug and recieve if sensor/ISP
   sends generic short packets */

enum packet_comp_cap mm_csi0_enable_packet_compare(struct packet_compare *compare)
{
	u32 base = V_BASE;
	enum packet_comp_cap cur = COMP_CAP_INVALID;
	if((compare == NULL) || (compare->enable == 0)){
		printk("Wrong parameters for compare and capture\n");
	} else {
		if(BRCM_READ_REG_FIELD(base, CAM_CMP0, PCEN)){
			printk("CMP0 occupied trying CMP1\n");
		} else {
			BRCM_WRITE_REG_FIELD(base, CAM_CMP0, PCEN, 1);
			BRCM_WRITE_REG_FIELD(base, CAM_CMP0, GIN, compare->gen_int);
			BRCM_WRITE_REG_FIELD(base, CAM_CMP0, CPHN, compare->capture_header);
			BRCM_WRITE_REG_FIELD(base, CAM_CMP0, PCVCN, compare->vc);
			BRCM_WRITE_REG_FIELD(base, CAM_CMP0, PCDTN, compare->dt);
			BRCM_WRITE_REG_FIELD(base, CAM_CAP0, CPHV, 1);
			cur = COMP_CAP_0;
		}
		if(BRCM_READ_REG_FIELD(base, CAM_CMP1, PCEN)){
			printk("CMP1 occupied trying CMP1\n");
		} else {
			BRCM_WRITE_REG_FIELD(base, CAM_CMP1, PCEN, 1);
			BRCM_WRITE_REG_FIELD(base, CAM_CMP1, GIN, compare->gen_int);
			BRCM_WRITE_REG_FIELD(base, CAM_CMP1, CPHN, compare->capture_header);
			BRCM_WRITE_REG_FIELD(base, CAM_CMP1, PCVCN, compare->vc);
			BRCM_WRITE_REG_FIELD(base, CAM_CMP1, PCDTN, compare->dt);
			BRCM_WRITE_REG_FIELD(base, CAM_CAP1, CPHV, 1);
			cur = COMP_CAP_1;
		}
	}
	return cur;
}

enum packet_comp_cap mm_csi0_get_captured_packet (enum packet_comp_cap num, struct packet_capture *cap)
{
	u32 base = V_BASE;
	enum packet_comp_cap cur = COMP_CAP_INVALID;
	if(cap == NULL){
		return COMP_CAP_INVALID;
	}
	if (num == COMP_CAP_0){
		if(BRCM_READ_REG_FIELD(base, CAM_CAP0, CPHV)){
			cap->valid = 1;
			cap->ecc = BRCM_READ_REG_FIELD(base, CAM_CAP0, CECCN);
			cap->word_count = BRCM_READ_REG_FIELD(base, CAM_CAP0, CWCN);
			cap->vc = BRCM_READ_REG_FIELD(base, CAM_CAP0, CVCN);
			cap->dt = BRCM_READ_REG_FIELD(base, CAM_CAP0, CDTN);
			cur = COMP_CAP_0;
		} else {
			printk("Invalid frame in CAP0\n");
		}
	} else if (num == COMP_CAP_1){
		if(BRCM_READ_REG_FIELD(base, CAM_CAP1, CPHV)){
			cap->valid = 1;
			cap->ecc = BRCM_READ_REG_FIELD(base, CAM_CAP1, CECCN);
			cap->word_count = BRCM_READ_REG_FIELD(base, CAM_CAP1, CWCN);
			cap->vc = BRCM_READ_REG_FIELD(base, CAM_CAP1, CVCN);
			cap->dt = BRCM_READ_REG_FIELD(base, CAM_CAP1, CDTN);
			cur = COMP_CAP_1;
		} else {
			printk("Invalid frame in CAP0\n");
		}
	} else {
		printk("Wrong COMP CAP ID\n");
	}
	return cur;
}

/* Get lane trans states */
int mm_csi0_get_trans(void)
{
	u32 base = V_BASE;
	u32 val;
	if(cam_state.mode == CSI1CCP2){
		printk("Lane transitions not valid for CSI1CCP2\n");
		return 0;
	}
	val = BRCM_READ_REG_FIELD(base, CAM_CLK, CLS);
	if((val == ULPS) || (val == ULPS_REQ) || (val == LANE_ERR)){
		printk("clock lane error \n");
		return val;
	}
	val = BRCM_READ_REG_FIELD(base, CAM_DAT0, DLSN);
	if((val == ULPS) || (val == ULPS_REQ) || (val == LANE_ERR)){
		printk("Dat0 lane error \n");
		return val;
	}
	if(cam_state.lanes == CSI2_DUAL_LANE){
		val = BRCM_READ_REG_FIELD(base, CAM_DAT1, DLSN);
		if((val == ULPS) || (val == ULPS_REQ) || (val == LANE_ERR)){
			printk("Dat0 lane error \n");
			return val;
		}
	}
	return 0;
}

/* STA register bit PS will be high to denote a panic was signalled */
bool mm_csi0_get_panic_state()
{
	u32 base = V_BASE;
	if(BRCM_READ_REG_FIELD(base, CAM_STA,PS)){
		BRCM_WRITE_REG_FIELD(base, CAM_STA, PS, 1);
		return 1;
	} 
	return 0;
}

