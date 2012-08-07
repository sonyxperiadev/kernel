/*******************************************************************************
* Copyright 2011 Broadcom Corporation.  All rights reserved.
*
*             @file     arch/arm/mach-rhea/rhea_fb_init.c
*
* Unless you and Broadcom execute a separate written software license agreement
* governing use of this software, this software is licensed to you under the
* terms of the GNU General Public License version 2, available at
* http://www.gnu.org/copyleft/gpl.html (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a license
* other than the GPL, without Broadcom's express prior written consent.
*******************************************************************************/

#ifdef CONFIG_FB_BRCM_KONA

void* (*konafb_dispdrv_entry) (void);
int konafb_parms[KONA_LCD_BOOT_PARM_COUNT];
int konafb_bootcfg = -1;  /* 0 if valid boot config found */

/* Kona Frame Buffer Display Platform Data Init Setup */
static int __init konafb_init(void)
{
	struct 	kona_fb_platform_data pdata;
	int    	err;
	struct 	platform_device *pdev;
	int	dev_count;
	int     i;

	if (konafb_bootcfg==0) {
		/* check API revs match */
		pdata.parms.w1.w32 = konafb_parms[1];
		if ((u8)pdata.parms.w1.bits.api_rev == KONA_LCD_BOOT_API_REV) {
			dev_count = 1;
		} else {
			konafb_bootcfg=-1;
			dev_count = ARRAY_SIZE(konafb_devices);
			printk(KERN_ERR "%s: LCD Init Boot API Rev mismatch, "
				"%u vs %u\n", __func__,
				(u8)pdata.parms.w1.bits.api_rev, 
				KONA_LCD_BOOT_API_REV);
		}	
	} else {
		dev_count = ARRAY_SIZE(konafb_devices);
	}
	
	pr_info("rheafb_devices count=%d\n", dev_count);
	for (i=0;i<dev_count;i++) {
		pdev = platform_device_alloc("rhea_fb", i);
		if (!pdev) {
			err = -ENOMEM;
			printk(KERN_ERR "%s: Device allocation failed\n",
				__func__);
			goto exit;
		}

		pdev->dev.dma_mask	    = (u64 *) ~(u32)0;
		pdev->dev.coherent_dma_mask = ~(u32)0;

		if(konafb_bootcfg==0) {
			pdata.dispdrv_entry = konafb_dispdrv_entry;
			pdata.parms.w0.w32 = (u32)konafb_parms[0];
			pdata.parms.w0.bits.boot_mode = 1;
			pdata.parms.w1.w32 = (u32)konafb_parms[1];
		} else {
			pdata.dispdrv_entry = konafb_devices[i].dispdrv_entry;
			pdata.parms.w0.w32 = konafb_devices[i].parms.w0.w32;
			pdata.parms.w0.bits.boot_mode = 0;
			pdata.parms.w1.w32 = konafb_devices[i].parms.w1.w32;
		}

		pr_info("board supports %s\n", konafb_devices[i].dispdrv_name);
		err = platform_device_add_data(pdev, (const void*)&pdata, 
			sizeof(struct kona_fb_platform_data));
		if (err) {
			printk(KERN_ERR "%s: Device Add Data failed\n",
				__func__);
			goto exit_put;
		}

		err = platform_device_add(pdev);
		if (err) {
			printk(KERN_ERR "%s: Device Add failed (%d)\n",
				__func__, err);
			goto exit_put;
		}
	}
	
exit:
	return (0);
	
exit_put:
	platform_device_put(pdev);
	return (0);
}

/* Parse RHEA Display Driver Boot Setup */
static int __init konafb_setup(char* options)
{
	char* this_opt;
	char konafb_drv_name[KONA_DISP_DRV_NAME_MAX];
	int retval;
	int i = 0;	

	if (options) {
		this_opt = strsep(&options,",");
		if (this_opt) {
			strncpy(konafb_drv_name, this_opt,
				KONA_DISP_DRV_NAME_MAX);

			for(i=0;i<ARRAY_SIZE(konafb_devices);i++) {
				if (!strncmp(konafb_drv_name,
					     konafb_devices[i].dispdrv_name, 
					     KONA_DISP_DRV_NAME_MAX))
					break;	
			}
			
			if (i>=ARRAY_SIZE(konafb_devices))
				goto no_match;

			konafb_dispdrv_entry = 
				konafb_devices[i].dispdrv_entry;
			
			i=0;
			while ((retval=get_option(&options,&konafb_parms[i])) 
				&& (i < KONA_LCD_BOOT_PARM_COUNT)) {
				i++;
			}
			
			if (i == KONA_LCD_BOOT_PARM_COUNT) {
				konafb_bootcfg = 0;
				printk(KERN_ERR "%s: Matched Display [%s]\n",
					__func__, konafb_drv_name);
			}
		}
       	} 
no_match:	
	return(1);
}
__setup("konafb=",konafb_setup); 

#endif /* #ifdef CONFIG_FB_BRCM_KONA */
