#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/platform_device.h>
#include <mach/platform.h>
#include <asm/sizes.h>
#include <asm/io.h>
#include <mach/bcm2708_smi.h>

#include "bcm2708_smi_regs.h"

struct bcm2708_smi_info {
	struct resource *smi_res;
	void __iomem *smi_base;
};
static struct bcm2708_smi_info *smi_info = NULL;

int32_t smi_wait_transfer_complete(void)
{
	uint32_t smics;

	do {
		smics = readl((smi_info->smi_base + SMICS));
	}while( !(smics & (1 << SMICS_DONE)) ); 

	return 0;
}
EXPORT_SYMBOL(smi_wait_transfer_complete);

int32_t smi_wait_direct_complete(void)
{
	uint32_t smidcs;

	do {
		smidcs = readl((smi_info->smi_base + SMIDCS));
	}while( !(smidcs & (1 << SMIDCS_DONE)) ); 

	return 0;
}
EXPORT_SYMBOL(smi_wait_direct_complete);

int32_t smi_read (const uint32_t slot, const uint32_t address,
		const uint32_t data_size_in_bytes, const uint32_t blocking,
		const void *data)
{
	int ret = -1;
	uint32_t smidcs, smida;
	uint32_t read_data;

	bcm2708_smi_print("slot=%d, address=0x%x, size=%d, blocking=%d, data=%p\n",
			slot, address, data_size_in_bytes, blocking, data);

	smida = (address & 0x3F) | (slot << SMIDA_DEVICE);
	writel(smida, (smi_info->smi_base + SMIDA));

	//start the transfer
	smidcs = (1 << SMIDCS_ENABLE);
	writel(smidcs, (smi_info->smi_base + SMIDCS));

	smidcs |= (1 << SMIDCS_START);
	writel(smidcs, (smi_info->smi_base + SMIDCS));

	/* wait until data is transferred */
	smi_wait_direct_complete();
	//TODO
	read_data = readl((smi_info->smi_base + SMIDD));

	switch(data_size_in_bytes)
	{
		case 1:
			*((uint8_t *)data) = read_data;
			break;
		case 2:
			*((uint16_t *)data) = read_data;
			break;
		case 4:
			*((uint32_t *)data) = read_data;
			break;
		default:
			printk(KERN_ERR"%s: incorrect data size=%d\n", __func__,
					data_size_in_bytes);
			ret = -EINVAL;
			goto exit_err;
	}
	
	smidcs = 1 << SMIDCS_DONE;
	writel(smidcs, (smi_info->smi_base + SMIDCS));
	return 0;
exit_err:
	return ret;

}
EXPORT_SYMBOL(smi_read);

int32_t smi_write (const uint32_t slot, const uint32_t address,
		const uint32_t data_size_in_bytes, const uint32_t blocking,
		const void *data)
{
	int ret = -1;
	uint32_t smidcs, smida;
	uint32_t write_data = 0;

	bcm2708_smi_print("slot=%d, address=0x%x, size=%d, blocking=%d, data=%p\n",
			slot, address, data_size_in_bytes, blocking, data);

	smida = (address & 0x3F) | (slot << SMIDA_DEVICE);
	writel(smida, (smi_info->smi_base + SMIDA));

	switch(data_size_in_bytes)
	{
		case 1:
			write_data = *((uint8_t *) data);
			break;
		case 2:
			write_data = *((uint16_t *) data);
			break;
		case 4:
			write_data = *((uint32_t *) data);
			break;
		default:
			printk(KERN_ERR"%s: incorrect data size=%d\n", __func__,
					data_size_in_bytes);
			ret = -EINVAL;
			goto exit_err;
	}

	writel(write_data, (smi_info->smi_base + SMIDD));	

	//start the transfer
	smidcs = (1 << SMIDCS_ENABLE | 1 << SMIDCS_WRITE);
	writel(smidcs, (smi_info->smi_base + SMIDCS));

	smidcs |= (1 << SMIDCS_START);
	writel(smidcs, (smi_info->smi_base + SMIDCS));

	//wait ofr data complete
	smi_wait_direct_complete();
	
	smidcs = (1 << SMIDCS_DONE);
	writel(smidcs, (smi_info->smi_base + SMIDCS));

	return 0;
exit_err:
	return ret;
}
EXPORT_SYMBOL(smi_write);

int32_t smi_setup_timing( const uint32_t slot, 
		const struct smi_periph_setup *periph_setup, const uint32_t smi_freq_hz )
{
	int ret = -1;
	uint32_t mode68, format32, pixel_swap;
	uint32_t write_width, read_width, clock_period_in_ns = 1000000000/smi_freq_hz;
	uint32_t read_strobe_time, read_pace_time, read_hold_time, read_setup_time;
	uint32_t write_strobe_time, write_pace_time, write_hold_time, write_setup_time;
	uint32_t val;
	uint32_t smidsr, smidsw, smics;


	bcm2708_smi_print("slot=%d periph_setup=%p smi_freq_hz=%d\n",
			slot, periph_setup, smi_freq_hz);

	if(!smi_info) {
		printk(KERN_ERR "SMI driver not initialized\n");
		ret = -ENODEV;
		goto exit_err_init;
	}
	
	if(slot > 3) {
		printk(KERN_ERR "slot should be less than 3\n");
		ret = -ENOENT;
		goto exit_err_slot;
	}

	/* disable smi */
	val = readl(smi_info->smi_base + SMICS);
	val &= ~1;
	writel((smi_info->smi_base + SMICS), val);
	
	switch(periph_setup->read_timings.transfer_width) 
	{
		case SMI_TRANSFER_WIDTH_8BIT:
			bcm2708_smi_print("Read xfer width=8bit\n");
			read_width = 0;
			break;
		default:
			printk(KERN_ERR"[%s]:Invalid read transfer width\n", __func__);
			ret = -EINVAL;
			goto exit_err_xfer;
			break;
	}
	
	switch(periph_setup->write_timings.transfer_width)
	{
		case SMI_TRANSFER_WIDTH_8BIT:
			bcm2708_smi_print("write xfer width=8bit\n");
			write_width = 0;
			break;
		default:
			printk(KERN_ERR"[%s]:Invalid write transfer width\n", __func__);
			ret = -EINVAL;
			goto exit_err_xfer;
			break;
	}
	
	read_strobe_time = (periph_setup->read_timings.strobe_time_in_ns + 
			clock_period_in_ns - 1) / clock_period_in_ns;
	read_pace_time = (periph_setup->read_timings.pace_time_in_ns + 
			clock_period_in_ns - 1) / clock_period_in_ns;
	read_hold_time = (periph_setup->read_timings.hold_time_in_ns + 
			clock_period_in_ns - 1) / clock_period_in_ns;
	read_setup_time = (periph_setup->read_timings.setup_time_in_ns + 
			clock_period_in_ns - 1) / clock_period_in_ns;
	
	read_strobe_time = clamp_t(uint32_t, read_strobe_time, 127, 0);
	read_pace_time = clamp_t(uint32_t, read_pace_time, 127, 0);
	read_hold_time = clamp_t(uint32_t, read_hold_time, 63, 0);
	read_setup_time = clamp_t(uint32_t, read_setup_time, 63, 0);

	mode68 = (periph_setup->mode == SMI_MODE_68);

	bcm2708_smi_print("Read: stobe_time=%d pace_time=%d hold_time=%d"
			"setup_time=%d mode68=%d width=%d\n", read_strobe_time,
			read_pace_time, read_hold_time, read_setup_time, mode68,
			read_width);

	smidsr = (read_strobe_time << SMIDS_STROBE)
			| (read_pace_time << SMIDS_PACE)
			| (read_hold_time << SMIDS_HOLD)
			| (mode68  << SMIDS_MODE68)
			| (read_setup_time << SMIDS_SETUP)
			| (read_width << SMIDS_WIDTH);

	/* smidsw */
	write_strobe_time = (periph_setup->write_timings.strobe_time_in_ns + 
			clock_period_in_ns - 1) / clock_period_in_ns;
	write_pace_time = (periph_setup->write_timings.pace_time_in_ns + 
			clock_period_in_ns - 1) / clock_period_in_ns;
	write_hold_time = (periph_setup->write_timings.hold_time_in_ns + 
			clock_period_in_ns - 1) / clock_period_in_ns;
	write_setup_time = (periph_setup->write_timings.setup_time_in_ns + 
			clock_period_in_ns - 1) / clock_period_in_ns;

	write_strobe_time = clamp_t( uint32_t, write_strobe_time, 127, 0);
	write_pace_time = clamp_t(uint32_t, write_pace_time, 127, 0);
	write_hold_time = clamp_t(uint32_t, write_hold_time, 63, 0);
	write_setup_time = clamp_t(uint32_t, write_setup_time, 63, 0);
	
	pixel_swap = (periph_setup->pixel_bits_swapped && 
			(periph_setup->pixel_format != SMI_PIXEL_FORMAT_DONT_CARE)) ? 1 : 0;

	format32 = (periph_setup->pixel_format == SMI_PIXEL_FORMAT_32BIT_RGB888);

	bcm2708_smi_print("write: stobe_time=%d pace_time=%d hold_time=%d"
			"setup_time=%d pixel_swap=%d format32=%d width=%d\n", 
			write_strobe_time,write_pace_time, write_hold_time,
			write_setup_time, pixel_swap, format32, write_width);

	smidsw = (write_strobe_time << SMIDS_STROBE)
		| (write_pace_time << SMIDS_PACE)
		| (write_hold_time << SMIDS_HOLD)
		| (pixel_swap  << SMIDS_SWAP)
		| (format32 << SMIDS_FORMAT)
		| (write_setup_time << SMIDS_SETUP)
		| (write_width << SMIDS_WIDTH);


	/* sync settings */
	writel(smidsr, (smi_info->smi_base + SMIDSR0 + (slot * (SMIDSR1 - SMIDSR0))));
	writel(smidsw, (smi_info->smi_base + SMIDSW0 + (slot * (SMIDSW1 - SMIDSW0))));

	/*smics*/
	smics = readl(smi_info->smi_base + SMICS);
	if (periph_setup->tearing_effect)
		smics |= SMICS_TEEN;
	else
		smics &= ~SMICS_TEEN;

	if (periph_setup->hvs_input)
		smics |= SMICS_PVMODE;
	else
		smics &= ~SMICS_PVMODE;
	writel(smics, (smi_info->smi_base + SMICS));

	return 0;

exit_err_xfer:
exit_err_slot:
exit_err_init:
	return ret;
}
EXPORT_SYMBOL(smi_setup_timing);

static int bcm2708_smi_probe(struct platform_device *pdev)
{
	struct bcm2708_smi_info *info;
	int ret;
	struct resource *res;
	
	bcm2708_smi_print("probe=%p\n", pdev);	

	info = kmalloc(sizeof(*info), GFP_KERNEL);
	if (info == NULL) {
		dev_err(&pdev->dev, "no memory for smi info\n");
		ret = -ENOMEM;
		goto exit_error;
	}

	/* get the smi  memory region */
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (res == NULL) {
		dev_err(&pdev->dev, "failed to get memory region resource\n");
		ret = -ENODEV;
		goto exit_rsrc_error;
	}

	bcm2708_smi_print("SMI start=%p end=%p\n", (void *)res->start,
			(void *)res->end);

	/* request memory region */
	info->smi_res = request_mem_region_exclusive(res->start,
			resource_size(res), "bcm2708smi_regs");

	if (info->smi_res == NULL) {
		dev_err(&pdev->dev, "Unable to request smi memory region\n");
		ret = -ENOENT;
		goto exit_mem_error;
	}

	info->smi_base = ioremap(res->start, SZ_4K);
	if( info->smi_base == NULL) { 
		dev_err(&pdev->dev, "Unable to request gencmd memory region\n");
		ret = -ENOMEM;
		goto exit_ioremap_error;
	}

	smi_info = info; 
	return 0;

exit_ioremap_error:
	release_region(info->smi_res->start, resource_size(info->smi_res));
exit_mem_error:
exit_rsrc_error:
	kfree(info);
exit_error:
	return ret;

}

static int __devexit bcm2708_smi_remove(struct platform_device *pdev)
{
	struct bcm2708_smi_info *info = smi_info;
	iounmap(info->smi_base);
	release_region(info->smi_res->start, resource_size(info->smi_res));
	kfree(info);
	smi_info = NULL;
	return 0;
}

#define bcm2708_smi_suspend    NULL
#define bcm2708_smi_resume     NULL

static struct platform_driver bcm2708_smi_driver = {
	.probe      = bcm2708_smi_probe,
	.remove     = __devexit_p(bcm2708_smi_remove),
	.suspend    = bcm2708_smi_suspend,
	.resume     = bcm2708_smi_resume,
	.driver     = {
		.name   = "bcm2708_smi",
		.owner  = THIS_MODULE,
	},

};

static int __init bcm2708_smi_init(void)
{
	printk(KERN_INFO"BCM2708 SMI Driver,(C) 2010 Braodcom Corp\n");
	return platform_driver_register(&bcm2708_smi_driver);

}

static void __exit bcm2708_smi_exit(void)
{
	platform_driver_unregister(&bcm2708_smi_driver);
}

module_init(bcm2708_smi_init);
module_exit(bcm2708_smi_exit);
