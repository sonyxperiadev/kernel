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
#include <linux/mutex.h>
#include <mach/irqs.h>
#include <asm/io.h>
#include <linux/clk.h>
#include <asm/uaccess.h>
#include <linux/proc_fs.h>
#include <linux/vmalloc.h>
#include <linux/mm.h>
#include <linux/bootmem.h>

#include <linux/broadcom/isp.h>
#include <mach/rdb/brcm_rdb_sysmap.h>
#include <mach/rdb/brcm_rdb_pwrmgr.h>
#include <mach/rdb/brcm_rdb_mm_rst_mgr_reg.h>
#include <mach/rdb/brcm_rdb_isp.h>
#include <linux/delay.h>


//TODO - define the major device ID
#define ISP_DEV_MAJOR    0

#define RHEA_ISP_BASE_PERIPHERAL_ADDRESS    ISP_BASE_ADDR
#define RHEA_MM_CLK_BASE_ADDRESS            MM_CLK_BASE_ADDR
#define RHEA_MM_RST_OFFSET                 (MM_RST_BASE_ADDR - MM_CLK_BASE_ADDR)

#define IRQ_ISP         (153+32)

#define ISP_DEBUG
#ifdef ISP_DEBUG
    #define dbg_print(fmt, arg...) \
    printk(KERN_ALERT "%s():" fmt, __func__, ##arg)
#else
    #define dbg_print(fmt, arg...)   do { } while (0)
#endif

#define err_print(fmt, arg...) \
    printk(KERN_ERR "%s():" fmt, __func__, ##arg)

static int isp_major = ISP_DEV_MAJOR;
static struct class *isp_class;
static void __iomem *isp_base = NULL;
static void __iomem *mmclk_base = NULL;
static struct clk *isp_clk;

typedef struct {
    unsigned int  status;
} isp_status_t;

typedef struct {
    struct semaphore irq_sem;
    spinlock_t lock;
    isp_status_t isp_status;     
} isp_t;

static int enable_isp_clock(void);
static void disable_isp_clock(void);
static inline unsigned int reg_read(void __iomem *, unsigned int reg);
static inline void reg_write(void __iomem *, unsigned int reg, unsigned int value);

static irqreturn_t isp_isr(int irq, void *dev_id)
{
    isp_t *dev;
    unsigned long flags;

    dbg_print("Got ISP interrupt\n");
    dev = (isp_t *)dev_id;

    spin_lock_irqsave(&dev->lock, flags);
    dev->isp_status.status = reg_read(isp_base, ISP_STATUS_OFFSET);        
    spin_unlock_irqrestore(&dev->lock, flags);
    
    reg_write(isp_base,ISP_STATUS_OFFSET, dev->isp_status.status);
        
    up(&dev->irq_sem);

    return IRQ_RETVAL(1);
}

static int isp_open(struct inode *inode, struct file *filp)
{
    int ret = 0;

    isp_t *dev = kmalloc(sizeof(isp_t), GFP_KERNEL);
    if (!dev)
        return -ENOMEM;

    filp->private_data = dev;
    dev->lock = SPIN_LOCK_UNLOCKED;
    dev->isp_status.status = 0;
    
    sema_init(&dev->irq_sem, 0);
    
    enable_isp_clock();

    ret = request_irq(IRQ_ISP, isp_isr, IRQF_DISABLED | IRQF_SHARED, ISP_DEV_NAME, dev);
    if (ret){
        err_print("request_irq failed ret = %d\n", ret);
        goto err;
    }

    disable_irq(IRQ_ISP);
    return 0;

err:
    if (dev)
        kfree(dev);
    return ret;
}

static int isp_release(struct inode *inode, struct file *filp)
{
    isp_t *dev = (isp_t *)filp->private_data;

    disable_isp_clock();
    
    free_irq(IRQ_ISP, dev);
    if (dev)
        kfree(dev);

    return 0;
}

static int isp_mmap(struct file *filp, struct vm_area_struct *vma)
{
    unsigned long vma_size = vma->vm_end - vma->vm_start;

    if (vma_size & (~PAGE_MASK)) {
        pr_err(KERN_ERR "isp_mmap: mmaps must be aligned to a multiple of pages_size.\n");
        return -EINVAL;
    }

    if (!vma->vm_pgoff) {
        vma->vm_pgoff = RHEA_ISP_BASE_PERIPHERAL_ADDRESS >> PAGE_SHIFT;
    } 
    else {
        return -EINVAL;
    }

    vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);

    /* Remap-pfn-range will mark the range VM_IO and VM_RESERVED */
    if (remap_pfn_range(vma,
                       vma->vm_start,
                       vma->vm_pgoff,
                       vma_size,
                       vma->vm_page_prot)) {
        pr_err("%s(): remap_pfn_range() failed\n", __FUNCTION__);
        return -EINVAL;
    }

    return 0;
}

static int isp_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
    isp_t *dev;
    int ret = 0;

    if(_IOC_TYPE(cmd) != BCM_ISP_MAGIC)
        return -ENOTTY;

    if(_IOC_NR(cmd) > ISP_CMD_LAST)
        return -ENOTTY;

    if(_IOC_DIR(cmd) & _IOC_READ)
        ret = !access_ok(VERIFY_WRITE, (void *) arg, _IOC_SIZE(cmd));

    if(_IOC_DIR(cmd) & _IOC_WRITE)
        ret |= !access_ok(VERIFY_READ, (void *) arg, _IOC_SIZE(cmd));

    if(ret)
        return -EFAULT;

    dev = (isp_t *)(filp->private_data);

    switch (cmd)
    {
    case ISP_IOCTL_WAIT_IRQ:
    {        
        dbg_print("Enabling ISP interrupt\n");

        enable_irq(IRQ_ISP);
        dbg_print("Waiting for interrupt\n");
        if (down_interruptible(&dev->irq_sem))
        {
            disable_irq(IRQ_ISP);
            return -ERESTARTSYS;
        }
        
        if (copy_to_user((u32 *)arg, &dev->isp_status, sizeof(dev->isp_status))) {
            err_print("ISP_IOCTL_WAIT_IRQ copy_to_user failed\n");
            ret = -EFAULT;
        }        
        
        dbg_print("Disabling ISP interrupt\n");
        disable_irq(IRQ_ISP);
    }
    break;
	
    case ISP_IOCTL_CLK_RESET:
    {
        unsigned int reg_val;	
        dbg_print("reset ISP clock\n");
        
        reg_write( mmclk_base, 
		           (RHEA_MM_RST_OFFSET + MM_RST_MGR_REG_WR_ACCESS_OFFSET), 
				   0xA5A501 );
		
        reg_val = reg_read( mmclk_base, (RHEA_MM_RST_OFFSET + MM_RST_MGR_REG_SOFT_RSTN0_OFFSET) ) & 
                  (~MM_RST_MGR_REG_SOFT_RSTN0_ISP_SOFT_RSTN_MASK); 
				  
        reg_write( mmclk_base, 
		           (RHEA_MM_RST_OFFSET + MM_RST_MGR_REG_SOFT_RSTN0_OFFSET), 
				   reg_val );
				   
        reg_val = reg_read( mmclk_base, (RHEA_MM_RST_OFFSET + MM_RST_MGR_REG_SOFT_RSTN0_OFFSET) ) |
                  MM_RST_MGR_REG_SOFT_RSTN0_ISP_SOFT_RSTN_MASK;
				  
        reg_write( mmclk_base, 
		           (RHEA_MM_RST_OFFSET + MM_RST_MGR_REG_SOFT_RSTN0_OFFSET), 
				   reg_val );

        msleep(5);				    
				   
        				   
		reg_write(mmclk_base,ISP_STATUS_OFFSET, dev->isp_status.status);		
	}

    default:
    break;
   }
   
    return ret;
}

static struct file_operations isp_fops =
{
    .open      = isp_open,
    .release   = isp_release,
    .mmap      = isp_mmap,
    .ioctl     = isp_ioctl,
};

static int enable_isp_clock(void)
{
    unsigned long rate;
    int ret;

    isp_clk = clk_get(NULL, "isp_axi_clk");
    if (!isp_clk) {
        err_print("%s: error get clock\n", __func__);
        return -EIO;
    }
    
    ret = clk_enable(isp_clk);
    if (ret) {
        err_print("%s: error enable ISP clock\n", __func__);
        return -EIO;
    }

    ret = clk_set_rate(isp_clk, 249600000);
    if (ret) {
        err_print("%s: error changing clock rate\n", __func__);
        //return -EIO;
    }
    
    rate = clk_get_rate(isp_clk);
    dbg_print("isp_clk_clk rate %lu\n", rate);
    
    return (ret);    
}

static void disable_isp_clock(void)
{
    isp_clk = clk_get(NULL, "isp_axi_clk");
    if (!isp_clk) return;
    
    clk_disable(isp_clk);       
}

static inline unsigned int reg_read(void __iomem * base_addr, unsigned int reg)
{
    unsigned int flags;

    flags = ioread32(base_addr + reg);
    return flags;
}

static inline void reg_write(void __iomem * base_addr, unsigned int reg, unsigned int value)
{
    iowrite32(value, base_addr + reg);
}

int __init isp_init(void)
{
    int ret;

    dbg_print("ISP driver Init\n");

    ret = register_chrdev(0, ISP_DEV_NAME, &isp_fops);
    if (ret < 0)
        return -EINVAL;
    else
        isp_major = ret;

    isp_class = class_create(THIS_MODULE, ISP_DEV_NAME);
    if (IS_ERR(isp_class)) {
        err_print("Failed to create ISP class\n");
        unregister_chrdev(isp_major, ISP_DEV_NAME);
        return PTR_ERR(isp_class);
    }

    device_create(isp_class, NULL, MKDEV(isp_major, 0), NULL, ISP_DEV_NAME);
    
    /* Map the ISP registers */
    isp_base = (void __iomem *)ioremap_nocache(RHEA_ISP_BASE_PERIPHERAL_ADDRESS, SZ_512K);
    if (isp_base == NULL)
        goto err;
		
    /* Map the MM CLK registers */
    mmclk_base = (void __iomem *)ioremap_nocache(RHEA_MM_CLK_BASE_ADDRESS, SZ_4K);
    if (mmclk_base == NULL)
        goto err;		

    return 0;

err:
    err_print("Failed to MAP the ISP IO space\n");
    unregister_chrdev(isp_major, ISP_DEV_NAME);
    return ret;
}

void __exit isp_exit(void)
{
    dbg_print("ISP driver Exit\n");
    if (isp_base)
        iounmap(isp_base);
		
    if (mmclk_base)
        iounmap(mmclk_base);		
   
    device_destroy(isp_class, MKDEV(isp_major, 0));
    class_destroy(isp_class);
    unregister_chrdev(isp_major, ISP_DEV_NAME);
}

module_init(isp_init);
module_exit(isp_exit);

MODULE_AUTHOR("Broadcom Corporation");
MODULE_DESCRIPTION("ISP device driver");
MODULE_LICENSE("GPL");
