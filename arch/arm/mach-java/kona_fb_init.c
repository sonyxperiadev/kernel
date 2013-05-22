/*******************************************************************************
* Copyright 2011 Broadcom Corporation.  All rights reserved.
*
*             @file     arch/arm/mach-kona/hawaii_fb_init.c
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

/* Kona Frame Buffer Display Platform Data Init Setup */
static int __init konafb_init(void)
{
	int	err;
	struct	platform_device *pdev;
	int	dev_count;
	int	i;

			dev_count = ARRAY_SIZE(konafb_devices);
	for (i = 0; i < dev_count; i++) {
		pdev = platform_device_alloc("kona_fb", i);
		if (!pdev) {
			err = -ENOMEM;
			printk(KERN_ERR "%s: Device allocation failed\n",
				__func__);
			goto exit;
		}

		pdev->dev.dma_mask	    = (u64 *) ~(u32)0;
		pdev->dev.coherent_dma_mask = ~(u32)0;

		err = platform_device_add_data(pdev,
			(const void *)&konafb_devices[i],
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
	return 0;

exit_put:
	platform_device_put(pdev);
	return 0;
}

#endif /* #ifdef CONFIG_FB_BRCM_KONA */
