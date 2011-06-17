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

//TODO - remove most of these!
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/semaphore.h>
#include <linux/mutex.h>
#include <mach/irqs.h>
#include <asm/io.h>
#include <linux/clk.h>
#include <asm/uaccess.h>
#include <linux/proc_fs.h>
#include <linux/vmalloc.h>
#include <linux/mm.h>
#include <linux/bootmem.h>

#include <linux/broadcom/v3d.h>
#include <mach/rdb/brcm_rdb_sysmap.h>
#include <mach/rdb/brcm_rdb_pwrmgr.h>
#include <mach/rdb/brcm_rdb_v3d.h>
#include <mach/rdb/brcm_rdb_mm_rst_mgr_reg.h>
#include <mach/gpio.h>

#define V3D_DEV_NAME	"v3d"

//TODO - define the major device ID
#define V3D_DEV_MAJOR	0

#define RHEA_V3D_BASE_PERIPHERAL_ADDRESS	MM_V3D_BASE_ADDR

#define IRQ_GRAPHICS	BCM_INT_ID_RESERVED148

//#define V3D_DEBUG
#ifdef V3D_DEBUG
	#define dbg_print(fmt, arg...) \
			printk(KERN_ALERT "%s():" fmt, __func__, ##arg)
#else
	#define dbg_print(fmt, arg...)	do { } while (0)
#endif

#define err_print(fmt, arg...) \
		printk(KERN_ERR "%s():" fmt, __func__, ##arg)

static int v3d_major = V3D_DEV_MAJOR;
static struct class *v3d_class;
static void __iomem *v3d_base = NULL;
static void __iomem *mm_rst_base = NULL;
static struct clk *v3d_clk;

//external pointer to the V3D memory
//This is the relocatable heap
void *v3d_mempool_base = NULL;
int v3d_mempool_size = V3D_MEMPOOL_SIZE;

typedef struct {
	mem_t mempool;
	struct semaphore irq_sem;
} v3d_t;

static int setup_v3d_clock(void);

static irqreturn_t v3d_isr(int irq, void *dev_id)
{
	v3d_t *dev;

	dbg_print("Got v3d interrupt\n");
	dev = (v3d_t *)dev_id;

	up(&dev->irq_sem);

	return IRQ_HANDLED;
}

static int v3d_open(struct inode *inode, struct file *filp)
{
	int ret = 0;

	v3d_t *dev = kmalloc(sizeof(v3d_t), GFP_KERNEL);
	if (!dev)
		return -ENOMEM;

	filp->private_data = dev;

	dev->mempool.ptr = v3d_mempool_base;
	dev->mempool.addr = virt_to_phys(dev->mempool.ptr);
	dev->mempool.size = v3d_mempool_size;

	sema_init(&dev->irq_sem, 0);

	ret = request_irq(IRQ_GRAPHICS, v3d_isr,
			IRQF_ONESHOT | IRQF_DISABLED | IRQF_TRIGGER_RISING, V3D_DEV_NAME, dev);
	if (ret){
		err_print("request_irq failed ret = %d\n", ret);
		goto err;
	}

	disable_irq(IRQ_GRAPHICS);
	return 0;

err:
	if (dev)
		kfree(dev);
	return ret;
}

static int v3d_release(struct inode *inode, struct file *filp)
{
	v3d_t *dev = (v3d_t *)filp->private_data;

	free_irq(IRQ_GRAPHICS, dev);
	if (dev)
		kfree(dev);

	return 0;
}

static int v3d_mmap(struct file *filp, struct vm_area_struct *vma)
{
	unsigned long vma_size = vma->vm_end - vma->vm_start;
	v3d_t *dev = (v3d_t *)(filp->private_data);

	if (vma_size & (~PAGE_MASK)) {
		err_print(KERN_ERR "v3d_mmap: mmaps must be aligned to a multiple of pages_size.\n");
		return -EINVAL;
	}

	if (!vma->vm_pgoff) {
		vma->vm_pgoff = RHEA_V3D_BASE_PERIPHERAL_ADDRESS >> PAGE_SHIFT;
	} else if (vma->vm_pgoff != (dev->mempool.addr >> PAGE_SHIFT)) {
		err_print("v3d_mmap failed\n");
		return -EINVAL;
	}

	vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);

	/* Remap-pfn-range will mark the range VM_IO and VM_RESERVED */
	if (remap_pfn_range(vma,
			vma->vm_start,
			vma->vm_pgoff,
			vma_size,
			vma->vm_page_prot)) {
		err_print("%s(): remap_pfn_range() failed\n", __FUNCTION__);
		return -EINVAL;
	}

	return 0;
}

static int v3d_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
	v3d_t *dev;
	int ret = 0;

	if(_IOC_TYPE(cmd) != BCM_V3D_MAGIC)
		return -ENOTTY;

	if(_IOC_NR(cmd) > V3D_CMD_LAST)
		return -ENOTTY;

	if(_IOC_DIR(cmd) & _IOC_READ)
		ret = !access_ok(VERIFY_WRITE, (void *) arg, _IOC_SIZE(cmd));

	if(_IOC_DIR(cmd) & _IOC_WRITE)
		ret |= !access_ok(VERIFY_READ, (void *) arg, _IOC_SIZE(cmd));

	if(ret)
		return -EFAULT;

	dev = (v3d_t *)(filp->private_data);

	switch (cmd)
   {
	   case V3D_IOCTL_GET_MEMPOOL:
		 //This is used to give userspace the pointer to the relocatable heap memory
		   if (copy_to_user((mem_t *)arg, &(dev->mempool), sizeof(mem_t)))
			   ret = -EPERM;
		break;

	   case V3D_IOCTL_WAIT_IRQ:
		{
			//Enable V3D block to generate interrupt
			dbg_print("Enabling v3d interrupt\n");
			enable_irq(IRQ_GRAPHICS);

			dbg_print("Waiting for interrupt\n");
		   if (down_interruptible(&dev->irq_sem))
			{
				disable_irq(IRQ_GRAPHICS);
				err_print("Wait for IRQ failed\n");
				return -ERESTARTSYS;
			}
			dbg_print("Disabling v3d interrupt\n");
			disable_irq(IRQ_GRAPHICS);
		}
	  break;

	   case V3D_IOCTL_EXIT_IRQ_WAIT:
				up(&dev->irq_sem);
		break;

	   case V3D_IOCTL_RESET:
	   {
				uint32_t value;

				clk_disable(v3d_clk);
				//Write the password to enable accessing other registers
				writel ( (0xA5A5 << MM_RST_MGR_REG_WR_ACCESS_PASSWORD_SHIFT) | 
					( 0x1 << MM_RST_MGR_REG_WR_ACCESS_RSTMGR_ACC_SHIFT), mm_rst_base + MM_RST_MGR_REG_WR_ACCESS_OFFSET);
	   
				// Put V3D in reset state
				value = readl( mm_rst_base + MM_RST_MGR_REG_SOFT_RSTN0_OFFSET );
				value = value & ~( 0x1 << MM_RST_MGR_REG_SOFT_RSTN0_V3D_SOFT_RSTN_SHIFT);
				writel ( value , mm_rst_base + MM_RST_MGR_REG_SOFT_RSTN0_OFFSET);
				
				// Enable V3D 
				value = value | ( 0x1 << MM_RST_MGR_REG_SOFT_RSTN0_V3D_SOFT_RSTN_SHIFT);
				writel ( value , mm_rst_base + MM_RST_MGR_REG_SOFT_RSTN0_OFFSET);
				
				//Write the password to disable accessing other registers
				writel ( (0xA5A5 << MM_RST_MGR_REG_WR_ACCESS_PASSWORD_SHIFT), mm_rst_base + MM_RST_MGR_REG_WR_ACCESS_OFFSET);
				
				clk_enable(v3d_clk);

	   }
		break;

	default:
		break;
	}

	return ret;
}

static struct file_operations v3d_fops =
{
	.open		= v3d_open,
	.release	= v3d_release,
	.mmap		= v3d_mmap,
	.ioctl		= v3d_ioctl,
};

static int __init setup_v3d_mempool(char *str)
{
	if(str){
		get_option(&str, &v3d_mempool_size);
	}
	dbg_print("Allocating relocatable heap of size = %d\n", v3d_mempool_size);
	v3d_mempool_base = alloc_bootmem_pages( v3d_mempool_size );
	if( !v3d_mempool_base )
		err_print("Failed to allocate relocatable heap memory\n");
	return 0;
}

__setup("v3d_mem=", setup_v3d_mempool);

static int setup_v3d_clock(void)
{
	unsigned long rate;
	int rc;

	v3d_clk = clk_get(NULL, "v3d_axi_clk");
	if (!v3d_clk) {
		err_print("%s: error get clock\n", __func__);
		return -EIO;
	}

	rc = clk_set_rate( v3d_clk, 249600000);
	if (rc) {
		err_print("%s: error changing clock rate\n", __func__);
		//return -EIO;
	}

	rc = clk_enable(v3d_clk);
	if (rc) {
		err_print("%s: error enable clock\n", __func__);
		return -EIO;
	}

	rate = clk_get_rate(v3d_clk);
	err_print("v3d_clk_clk rate %lu\n", rate);

	return (rc);
}

int __init v3d_init(void)
{
	int ret;

	dbg_print("V3D driver Init\n");

	if( !v3d_mempool_base ){
		err_print("Failed: Required relocatable heap memory is not present\n");
		return -EINVAL;
	}

	ret = register_chrdev(0, V3D_DEV_NAME, &v3d_fops);
	if (ret < 0)
		return -EINVAL;
	else
		v3d_major = ret;

	v3d_class = class_create(THIS_MODULE, V3D_DEV_NAME);
	if (IS_ERR(v3d_class)) {
		err_print("Failed to create V3D class\n");
		unregister_chrdev(v3d_major, V3D_DEV_NAME);
		return PTR_ERR(v3d_class);
	}

	device_create(v3d_class, NULL, MKDEV(v3d_major, 0), NULL, V3D_DEV_NAME);

	setup_v3d_clock();

	/* Map the V3D registers */
	v3d_base = (void __iomem *)ioremap_nocache(RHEA_V3D_BASE_PERIPHERAL_ADDRESS, SZ_64K);
	if (v3d_base == NULL)
		goto err;

	
	/* Map the V3D registers */
	mm_rst_base = (void __iomem *)ioremap_nocache(MM_RST_BASE_ADDR, SZ_4K);
	if (mm_rst_base == NULL)
		goto err;

	/* Print out the V3D identification registers */
	dbg_print("V3D Identification 0 = 0X%x\n", readl(v3d_base + V3D_IDENT0_OFFSET));
	dbg_print("V3D Identification 1 = 0X%x\n", readl(v3d_base + V3D_IDENT1_OFFSET));
	dbg_print("V3D Identification 2 = 0X%x\n", readl(v3d_base + V3D_IDENT2_OFFSET));
	dbg_print("V3D register base address (remaped) = 0X%p\n", v3d_base);

	return 0;

err:
	err_print("Failed to MAP the V3D IO space\n");
	unregister_chrdev(v3d_major, V3D_DEV_NAME);
	return ret;
}

void __exit v3d_exit(void)
{
	dbg_print("V3D driver Exit\n");
	if (v3d_base)
		iounmap(v3d_base);
	device_destroy(v3d_class, MKDEV(v3d_major, 0));
	class_destroy(v3d_class);
	unregister_chrdev(v3d_major, V3D_DEV_NAME);

	/* Free up the relocatable memory allocated */
	if (v3d_mempool_base)
		free_bootmem((long unsigned int)v3d_mempool_base, v3d_mempool_size);
}

module_init(v3d_init);
module_exit(v3d_exit);

MODULE_AUTHOR("Broadcom Corporation");
MODULE_DESCRIPTION("V3D device driver");
MODULE_LICENSE("GPL");
