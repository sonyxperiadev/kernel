/*******************************************************************************
Copyright 2010 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement
governing use of this software, this software is licensed to you under the
terms of the GNU General Public License version 2, available at
http://www.gnu.org/copyleft/gpl.html (the "GPL").

Notwithstanding the above, under no circumstances may you combine this software
in any way with any other Broadcom software provided under a license other than
the GPL, without Broadcom's express prior written consent.
*******************************************************************************/

#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/semaphore.h>
#include <linux/platform_device.h>
#include <plat/kona_unicam.h>
#include <linux/mutex.h>
#include <mach/irqs.h>
#include <linux/io.h>
#include <linux/clk.h>
#include <linux/uaccess.h>
#include <linux/proc_fs.h>
#include <linux/vmalloc.h>
#include <linux/mm.h>
#include <linux/gpio.h>
#include <linux/delay.h>

#include <linux/broadcom/unicam.h>
#include <mach/rdb/brcm_rdb_sysmap.h>
#include <mach/rdb/brcm_rdb_mm_cfg.h>
#include <mach/rdb/brcm_rdb_cam.h>

#include <mach/memory.h>
#include <plat/clock.h>
#include <plat/pi_mgr.h>
#include <plat/scu.h>
#include <linux/regulator/consumer.h>

/* TODO - define the major device ID */
#define UNICAM_DEV_MAJOR	0

#define RHEA_UNICAM_BASE_PERIPHERAL_ADDRESS	MM_CSI0_BASE_ADDR
#define RHEA_MM_CFG_BASE_ADDRESS		MM_CFG_BASE_ADDR

#define IRQ_UNICAM         (156+32)

#define CSI0_UNICAM_PORT     0
#define CSI0_UNICAM_CLK      0

#define CSI1_UNICAM_PORT     1
#define CSI1_UNICAM_CLK      0

#define UNICAM_MEM_POOL_SIZE   SZ_8M

static int unicam_major = UNICAM_DEV_MAJOR;
static struct class *unicam_class;
static void __iomem *unicam_base;
static void __iomem *mmcfg_base;
static struct pi_mgr_dfs_node unicam_dfs_node = {
	.valid = 0,
};
static struct pi_mgr_qos_node unicam_qos_node;

struct unicam {
	struct completion irq_sem;
	cam_isr_reg_status_st_t unicam_isr_reg_status;
	unsigned int irq_pending;
	unsigned int irq_start;
};

struct unicam_info {
	unsigned int csi0_unicam_gpio;
	unsigned int csi1_unicam_gpio;
	struct regulator *reg;
	struct device *dev;
};

static struct unicam_info unicam_info;

static int enable_unicam_clock(void);
static void disable_unicam_clock(void);
static void reset_unicam(void);
static void unicam_init_camera_intf(void);
static void unicam_open_csi(unsigned int port, unsigned int clk_src);
static void unicam_close_csi(unsigned int port, unsigned int clk_src);
static void unicam_sensor_control(unsigned int sensor_id, unsigned int enable);

static inline unsigned int reg_read(void __iomem *, unsigned int reg);
static inline void reg_write(void __iomem *, unsigned int reg,
			     unsigned int value);

static irqreturn_t unicam_isr(int irq, void *dev_id)
{
	struct unicam *dev;
	unsigned int rx_status, image_intr;
	dev = dev_id;

	rx_status = reg_read(unicam_base, CAM_STA_OFFSET);
	image_intr = reg_read(unicam_base, CAM_ISTA_OFFSET);
	/*  enable access */
	reg_write(unicam_base, CAM_ISTA_OFFSET, image_intr);
	reg_write(unicam_base, CAM_STA_OFFSET, rx_status);

	if (dev->irq_start == 1) {
		if (dev->irq_pending == 0) {
			dev->unicam_isr_reg_status.rx_status = rx_status;
			dev->unicam_isr_reg_status.image_intr = image_intr;
			complete(&dev->irq_sem);
		}
		dev->irq_pending++;
	}

	return IRQ_RETVAL(1);
}

static int unicam_open(struct inode *inode, struct file *filp)
{
	int ret = 0;
	struct unicam *dev = kzalloc(sizeof(struct unicam), GFP_KERNEL);

	if (!dev)
		return -ENOMEM;

	filp->private_data = dev;

	init_completion(&dev->irq_sem);
	dev->irq_pending = 0;
	dev->irq_start = 0;

	if (pi_mgr_dfs_request_update(&unicam_dfs_node, PI_OPP_TURBO)) {
		dev_err(unicam_info.dev, "%s:failed to update dfs request for unicam\n",
		       __func__);
		return -EIO;
	}

	pi_mgr_qos_request_update(&unicam_qos_node, 0);
	scu_standby(0);

	enable_unicam_clock();
	reset_unicam();
	unicam_init_camera_intf();

	ret =
	    request_irq(IRQ_UNICAM, unicam_isr, IRQF_DISABLED | IRQF_SHARED,
			UNICAM_DEV_NAME, dev);
	if (ret) {
		dev_err(unicam_info.dev, "%s: request_irq failed ret = %d\n",
			__func__, ret);
		goto err;
	}

	if (unicam_info.reg) {
		ret = regulator_enable(unicam_info.reg);
		if (ret < 0) {
			dev_err(unicam_info.dev,
				"%s: cannot enable regulator: %d\n",
				__func__, ret);
			goto err;
		}
	}

	return 0;

err:
	kfree(dev);

	return ret;
}

static int unicam_release(struct inode *inode, struct file *filp)
{
	struct unicam *dev = filp->private_data;

	disable_irq(IRQ_UNICAM);
	free_irq(IRQ_UNICAM, dev);

	if (unicam_info.reg) {
		if (regulator_disable(unicam_info.reg) < 0)
			dev_err(unicam_info.dev, "%s: cannot disable regulator\n",
				__func__);
	}

	reset_unicam();
	disable_unicam_clock();
	pi_mgr_qos_request_update(&unicam_qos_node, PI_MGR_QOS_DEFAULT_VALUE);
	if (pi_mgr_dfs_request_update(&unicam_dfs_node, PI_MGR_DFS_MIN_VALUE))
		dev_err(unicam_info.dev, "%s: failed to update dfs request for unicam\n",
		       __func__);
	scu_standby(1);
	kfree(dev);

	return 0;
}

static int unicam_mmap(struct file *filp, struct vm_area_struct *vma)
{
	unsigned long vma_size = vma->vm_end - vma->vm_start;

	if (vma_size & (~PAGE_MASK)) {
		dev_err(unicam_info.dev, "%s: mmaps must be aligned to multiple of pages_size.\n",
			__func__);
		return -EINVAL;
	}

	if (!vma->vm_pgoff) {
		vma->vm_pgoff =
		    RHEA_UNICAM_BASE_PERIPHERAL_ADDRESS >> PAGE_SHIFT;
	}

	vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);

	/* Remap-pfn-range will mark the range VM_IO and VM_RESERVED */
	if (remap_pfn_range(vma,
			    vma->vm_start,
			    vma->vm_pgoff, vma_size, vma->vm_page_prot)) {
		dev_err(unicam_info.dev, "%s(): remap_pfn_range() failed\n",
			__func__);
		return -EINVAL;
	}

	return 0;
}

static long unicam_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	struct unicam *dev;
	int ret = 0;
	static int interrupt_irq;
	sensor_ctrl_t sensor_ctrl;

	if (_IOC_TYPE(cmd) != BCM_UNICAM_MAGIC)
		return -ENOTTY;

	if (_IOC_NR(cmd) > UNICAM_CMD_LAST)
		return -ENOTTY;

	if (_IOC_DIR(cmd) & _IOC_READ)
		ret = !access_ok(VERIFY_WRITE, (void *)arg, _IOC_SIZE(cmd));

	if (_IOC_DIR(cmd) & _IOC_WRITE)
		ret |= !access_ok(VERIFY_READ, (void *)arg, _IOC_SIZE(cmd));

	if (ret)
		return -EFAULT;

	dev = filp->private_data;

	switch (cmd) {
	case UNICAM_IOCTL_WAIT_IRQ:
		interrupt_irq = 0;
		dev->irq_start = 1;
		dev_dbg(unicam_info.dev, "UNICAM: Waiting for interrupt\n");
		if (wait_for_completion_interruptible(&dev->irq_sem)) {
			disable_irq(IRQ_UNICAM);
			return -ERESTARTSYS;
		}
		if (interrupt_irq) {
			dev_dbg(unicam_info.dev, "interrupted irq ioctl\n");
			return -EIO;
		}
		dev->unicam_isr_reg_status.dropped_frames =
		    dev->irq_pending - 1;
		if (copy_to_user
		    ((cam_isr_reg_status_st_t *) arg,
		     &dev->unicam_isr_reg_status,
		     sizeof(cam_isr_reg_status_st_t)))
			ret = -EPERM;
		dev_dbg(unicam_info.dev, "UNICAM: Frame Received: dropped=%d\n",
			  dev->unicam_isr_reg_status.dropped_frames);
		dev->irq_pending = 0;	/*  allow a new frame */
		break;
	case UNICAM_IOCTL_RETURN_IRQ:
		interrupt_irq = 1;
		dev_dbg(unicam_info.dev, "Interrupting irq ioctl\n");
		if (dev->irq_pending == 0) {
			complete(&dev->irq_sem);
			dev->irq_pending = 1;
		}
		break;

	case UNICAM_IOCTL_OPEN_CSI0:
		dev_dbg(unicam_info.dev, "Open unicam CSI0 port\n");
		unicam_open_csi(CSI0_UNICAM_PORT, CSI0_UNICAM_CLK);
		break;

	case UNICAM_IOCTL_CLOSE_CSI0:
		dev_dbg(unicam_info.dev, "Close unicam CSI0 port\n");
		unicam_close_csi(CSI0_UNICAM_PORT, CSI0_UNICAM_CLK);
		break;

	case UNICAM_IOCTL_OPEN_CSI1:
		dev_dbg(unicam_info.dev, "Open unicam CSI1 port\n");
		unicam_open_csi(CSI1_UNICAM_PORT, CSI1_UNICAM_CLK);
		break;

	case UNICAM_IOCTL_CLOSE_CSI1:
		dev_dbg(unicam_info.dev, "close unicam CSI1 port\n");
		unicam_close_csi(CSI1_UNICAM_PORT, CSI1_UNICAM_CLK);
		break;

	case UNICAM_IOCTL_CONFIG_SENSOR:
		dev_dbg(unicam_info.dev, "Config Sensor\n");
		if (copy_from_user
		    (&sensor_ctrl, (sensor_ctrl_t *) arg,
		     sizeof(sensor_ctrl_t)))
			ret = -EPERM;

		unicam_sensor_control(sensor_ctrl.sensor_id,
				      sensor_ctrl.enable);
		break;
	default:
		break;
	}
	return ret;
}

static const struct file_operations unicam_fops = {
	.open = unicam_open,
	.release = unicam_release,
	.mmap = unicam_mmap,
	.unlocked_ioctl = unicam_ioctl,
};

static inline unsigned int reg_read(void __iomem *base_addr, unsigned int reg)
{
	unsigned int flags;

	flags = ioread32(base_addr + reg);
	return flags;
}

static inline void reg_write(void __iomem *base_addr, unsigned int reg,
			     unsigned int value)
{
	iowrite32(value, base_addr + reg);
}

static void unicam_init_camera_intf(void)
{
	/*  Init GPIO's to off */
	if (unicam_info.csi0_unicam_gpio != 0xffffffff) {
		gpio_request(unicam_info.csi0_unicam_gpio, "CAM_STNDBY0");
		gpio_direction_output(unicam_info.csi0_unicam_gpio, 0);
		gpio_set_value(unicam_info.csi0_unicam_gpio, 0);
	}

	if (unicam_info.csi1_unicam_gpio != 0xffffffff) {
		gpio_request(unicam_info.csi1_unicam_gpio, "CAM_STNDBY1");
		gpio_direction_output(unicam_info.csi1_unicam_gpio, 0);
		gpio_set_value(unicam_info.csi1_unicam_gpio, 0);
	}
	usleep_range(10000, 12000);
}

static void unicam_sensor_control(unsigned int sensor_id, unsigned int enable)
{
	/*  primary sensor */
	if ((sensor_id == 0) && (unicam_info.csi0_unicam_gpio != 0xffffffff))
		gpio_set_value(unicam_info.csi0_unicam_gpio, enable);

	/*  secondary sensor */
	else if ((sensor_id == 1)
		 && (unicam_info.csi1_unicam_gpio != 0xffffffff))
		gpio_set_value(unicam_info.csi1_unicam_gpio, enable);

	usleep_range(10000, 12000);
}

static void unicam_open_csi(unsigned int port, unsigned int clk_src)
{
	unsigned int value, ret;
	struct clk *csi_clk;

	/*  Set Camera CSI0 Phy & Clock Registers, CSI1 use these clocks also*/
	csi_clk = clk_get(NULL, "csi0_lp_clk");
	if (IS_ERR_OR_NULL(csi_clk))
		dev_err(unicam_info.dev, "%s: error get lp clock\n",
			__func__);
	ret = clk_enable(csi_clk);
	if (ret)
		dev_err(unicam_info.dev, "%s: error enable csi lp clock\n",
			__func__);
	ret = clk_set_rate(csi_clk, 100000000);
	if (ret)
		dev_err(unicam_info.dev, "%s: error set lp clock rate\n",
			__func__);

	if (port == 0) {
		mm_ccu_set_pll_select(CSI0_CAMPIX_PLL, CSI_RX0_BCLKHS);
		mm_ccu_set_pll_select(CSI0_BYTE1_PLL, CSI_RX0_BCLKHS);
		mm_ccu_set_pll_select(CSI0_BYTE0_PLL, CSI_RX0_BCLKHS);
		/* Enable CSI0 LDO */
		reg_write(mmcfg_base, MM_CFG_CSI0_LDO_CTL_OFFSET, 0x5A00000F);
		/*  AFE 0 Select:  CSI0 has PHY selection. */
		value = reg_read(mmcfg_base,
				MM_CFG_CSI0_PHY_CTRL_OFFSET) & 0x7fffffff;
		reg_write(mmcfg_base, MM_CFG_CSI0_PHY_CTRL_OFFSET, value);
	} else {
		mm_ccu_set_pll_select(CSI1_CAMPIX_PLL, CSI_RX0_BCLKHS);
		mm_ccu_set_pll_select(CSI1_BYTE1_PLL, CSI_RX0_BCLKHS);
		mm_ccu_set_pll_select(CSI1_BYTE0_PLL, CSI_RX0_BCLKHS);
		/* Enable CSI1 LDO */
		reg_write(mmcfg_base, MM_CFG_CSI1_LDO_CTL_OFFSET, 0x5A00000F);
		/*  AFE 1 Select:  CSI0 has PHY selection. */
		value = reg_read(mmcfg_base,
				MM_CFG_CSI0_PHY_CTRL_OFFSET) | 0x80000000;
		reg_write(mmcfg_base, MM_CFG_CSI0_PHY_CTRL_OFFSET, value);
	}

	if (clk_src == 0) {
		csi_clk = clk_get(NULL, "dig_ch0_clk");
		if (IS_ERR_OR_NULL(csi_clk))
			dev_err(unicam_info.dev, "%s: error get clock\n",
				__func__);

		ret = clk_enable(csi_clk);
		if (ret)
			dev_err(unicam_info.dev, "%s: error enable unicam clock\n",
				__func__);

		ret = clk_set_rate(csi_clk, 13000000);
		if (ret)
			dev_err(unicam_info.dev, "%s: error changing clock rate\n",
				__func__);

		dev_dbg(unicam_info.dev, "dig_chan_clk rate %lu\n",
			  clk_get_rate(csi_clk));
	} else {		/* if (clk_src == 1) { */
		csi_clk = clk_get(NULL, "dig_ch1_clk");
		if (IS_ERR_OR_NULL(csi_clk))
			dev_err(unicam_info.dev, "%s: error get clock\n",
				__func__);

		ret = clk_enable(csi_clk);
		if (ret)
			dev_err(unicam_info.dev, "%s: error enable unicam clock\n",
				__func__);

		ret = clk_set_rate(csi_clk, 13000000);
		if (ret)
			dev_err(unicam_info.dev, "%s: error changing clock rate\n",
				__func__);

		dev_dbg(unicam_info.dev, "dig_chan_clk rate %lu\n",
			  clk_get_rate(csi_clk));
	}
}

static void unicam_close_csi(unsigned int port, unsigned int clk_src)
{
	struct  clk *csi_clk;

	/*  Disable  Camera CSI0 Clock Registers, CSI1 use these clocks also*/
	csi_clk = clk_get(NULL, "csi0_lp_clk");
	clk_disable(csi_clk);

	/* Disable CSIx LDO */
	if (port == 0)
		reg_write(mmcfg_base, MM_CFG_CSI0_LDO_CTL_OFFSET, 0x5A000000);
	else
		reg_write(mmcfg_base, MM_CFG_CSI1_LDO_CTL_OFFSET, 0x5A000000);

	if (clk_src == 0) {
		/*  Disable Dig Clk0 */
		csi_clk = clk_get(NULL, "dig_ch0_clk");
		if (IS_ERR_OR_NULL(csi_clk))
			return;

		clk_disable(csi_clk);
	} else {
		/*  Disable Dig Clk1 */
		csi_clk = clk_get(NULL, "dig_ch1_clk");
		if (IS_ERR_OR_NULL(csi_clk))
			return;

		clk_disable(csi_clk);
	}
}

static int enable_unicam_clock(void)
{
	unsigned long rate;
	int ret;
	struct clk *unicam_clk;

	unicam_clk = clk_get(NULL, "csi0_axi_clk");
	if (IS_ERR_OR_NULL(unicam_clk)) {
		dev_err(unicam_info.dev, "%s: error get clock\n", __func__);
		return -EIO;
	}

	ret = clk_enable(unicam_clk);
	if (ret) {
		dev_err(unicam_info.dev, "%s: error enable unicam clock\n",
			__func__);
		return -EIO;
	}
#if 0
	ret = clk_set_rate(unicam_clk, 250000000);
	if (ret) {
		dev_err(unicam_info.dev, "%s: error changing clock rate\n",
			__func__);
		/* return -EIO; */
	}
#endif
	rate = clk_get_rate(unicam_clk);
	dev_dbg(unicam_info.dev, "unicam_clk_clk rate %lu\n", rate);

	return 0;
}

static void disable_unicam_clock(void)
{
	struct clk *unicam_clk;

	unicam_clk = clk_get(NULL, "csi0_axi_clk");
	if (IS_ERR_OR_NULL(unicam_clk))
		return;
	clk_disable(unicam_clk);
}

static void reset_unicam(void)
{
	struct clk *unicam_clk;

	/* Reset UNICAM interface */
	unicam_clk = clk_get(NULL, "csi0_axi_clk");
	if (IS_ERR_OR_NULL(unicam_clk))
		return;
	/* Should clear and set CSI0_SOFT_RSTN_MASK */
	clk_reset(unicam_clk);
}

static int __devexit unicam_drv_remove(struct platform_device *pdev)
{
	if (unicam_info.reg)
		regulator_put(unicam_info.reg);

	pi_mgr_dfs_request_remove(&unicam_dfs_node);
	unicam_dfs_node.name = NULL;
	pi_mgr_qos_request_remove(&unicam_qos_node);
	unicam_qos_node.name = NULL;
	return 0;
}

static int unicam_drv_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct kona_unicam_platform_data *pdata = pdev->dev.platform_data;

	unicam_info.dev = &pdev->dev;
	unicam_info.csi0_unicam_gpio = pdata->csi0_gpio;
	unicam_info.csi1_unicam_gpio = pdata->csi1_gpio;

	dev_dbg(unicam_info.dev, "%s\n", __func__);

	if (!pdata) {
		dev_err(unicam_info.dev, "%s : invalid platform data !!\n",
			__func__);
		ret = -EPERM;
		goto error;
	}

	unicam_info.reg = regulator_get(unicam_info.dev, "vcc");
	if (IS_ERR_OR_NULL(unicam_info.reg)) {
		dev_err(unicam_info.dev, "%s: cannot get regulator: %ld\n",
			__func__, PTR_ERR(unicam_info.reg));
		unicam_info.reg = NULL;
	}

	ret =
	    pi_mgr_dfs_add_request(&unicam_dfs_node, "unicam", PI_MGR_PI_ID_MM,
				   PI_MGR_DFS_MIN_VALUE);
	if (ret) {
		dev_err(unicam_info.dev, "%s: failed to register PI DFS request\n",
		       __func__);
		ret = -EIO;
		goto dfs_request_fail;
	}
	ret = pi_mgr_qos_add_request(&unicam_qos_node, "unicam",
		PI_MGR_PI_ID_ARM_CORE, PI_MGR_QOS_DEFAULT_VALUE);
	if (ret) {
		dev_err(unicam_info.dev, "%s: failed to register PI QOS request\n",
			__func__);
		ret = -EIO;
		goto qos_request_fail;
	}

	return ret;

qos_request_fail:
	pi_mgr_dfs_request_remove(&unicam_dfs_node);
dfs_request_fail:
	if (unicam_info.reg)
		regulator_put(unicam_info.reg);
error:
	return ret;
}

static struct platform_driver __refdata unicam_drv = {
	.probe = unicam_drv_probe,
	.remove = __devexit_p(unicam_drv_remove),
	.driver = {.name = "kona-unicam",},
};

int __init unicam_init(void)
{
	int ret;

	dev_dbg(unicam_info.dev, "unicam driver Init\n");

	ret = register_chrdev(0, UNICAM_DEV_NAME, &unicam_fops);
	if (ret < 0)
		return -EINVAL;
	else
		unicam_major = ret;

	unicam_class = class_create(THIS_MODULE, UNICAM_DEV_NAME);
	if (IS_ERR(unicam_class)) {
		dev_err(unicam_info.dev, "%s Failed to create unicam class\n",
			__func__);
		unregister_chrdev(unicam_major, UNICAM_DEV_NAME);
		return PTR_ERR(unicam_class);
	}

	device_create(unicam_class, NULL, MKDEV(unicam_major, 0), NULL,
		      UNICAM_DEV_NAME);

	/*  Map the unicam registers */
	unicam_base =
	    (void __iomem *)ioremap_nocache(RHEA_UNICAM_BASE_PERIPHERAL_ADDRESS,
					    SZ_4K);
	if (unicam_base == NULL)
		goto err;

	mmcfg_base =
	    (void __iomem *)ioremap_nocache(RHEA_MM_CFG_BASE_ADDRESS, SZ_4K);
	if (mmcfg_base == NULL)
		goto err;

	ret = platform_driver_register(&unicam_drv);

	return ret;

err:
	dev_err(unicam_info.dev, "%s: Failed to MAP the unicam IO space\n",
		__func__);
	unregister_chrdev(unicam_major, UNICAM_DEV_NAME);
	return ret;
}

void __exit unicam_exit(void)
{
	dev_dbg(unicam_info.dev, "unicam driver Exit\n");
	if (unicam_base)
		iounmap(unicam_base);

	if (mmcfg_base)
		iounmap(mmcfg_base);

	device_destroy(unicam_class, MKDEV(unicam_major, 0));
	class_destroy(unicam_class);
	unregister_chrdev(unicam_major, UNICAM_DEV_NAME);

	platform_driver_unregister(&unicam_drv);
}

module_init(unicam_init);
module_exit(unicam_exit);

MODULE_AUTHOR("Broadcom Corporation");
MODULE_DESCRIPTION("unicam device driver");
MODULE_LICENSE("GPL");

