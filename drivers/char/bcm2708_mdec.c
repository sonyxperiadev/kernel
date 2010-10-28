#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/uaccess.h>
#include <linux/proc_fs.h>
#include <linux/device.h>
#include <linux/sched.h>
#include <linux/miscdevice.h>
#include <linux/vmalloc.h>
#include <linux/string.h>

#include <asm/bug.h>

#include <mach/media_dec_regs.h>
#include <mach/ipc.h>

#include "bcm2708_mdec.h"


#define BCM2708_MDEC_DRIVER_NAME "bcm2708_mdec"

#define BCM2708MDEC_DEBUG 0

#define bcm2708mdec_error(format, arg...) \
	printk(KERN_ERR BCM2708_MDEC_DRIVER_NAME ": %s" format, __func__, ##arg) 

#if BCM2708MDEC_DEBUG
#define bcm2708mdec_dbg(format, arg...) bcm2708mdec_error(format, ##arg)
#else
#define bcm2708mdec_dbg(format, arg...) printk(KERN_DEBUG BCM2708_MDEC_DRIVER_NAME ": %s" format, __func__, ##arg)
#endif

#define VC_MFS_SD_PREFIX "/mfs/sd/"	/* the path for mdeia file on VC SD card. */

#define MEDIA_DEC_TEST_REGISTER_RW(offset)	\
	(*((volatile u32 *)((u32)g_mdec->reg_base + (offset))))

struct bcm2708_mdec {
        u32		irq;
        void __iomem	*reg_base;
	char 		ioctl_cmd_buf[MAX_BCM2708_MDEC_IOCTL_CMD_SIZE];
};

/* hacky here; needs to make a per thread buffer */
static struct bcm2708_mdec *g_mdec = NULL;

static int do_playback(bcm2708_mdec_play_t *play_cmd)
{
	int ret = 0;
	u32 time = 0, prev_time = 0;
	int count;
	char *temp_name;
	
	BUG_ON(MEDIA_DEC_DEBUG_FILENAME_LENGTH  <= play_cmd->filename_size);

	/* Set up the debug mode */
	MEDIA_DEC_TEST_REGISTER_RW( MEDIA_DEC_DEBUG_MASK ) = 0;

	/* Set up the src width as 0xFFFFFFFF (ignore) */
	MEDIA_DEC_TEST_REGISTER_RW( MEDIA_DEC_SOURCE_X_OFFSET ) = 0xFFFFFFFF;
	MEDIA_DEC_TEST_REGISTER_RW( MEDIA_DEC_SOURCE_Y_OFFSET ) = 0xFFFFFFFF;
	MEDIA_DEC_TEST_REGISTER_RW( MEDIA_DEC_SOURCE_WIDTH_OFFSET ) = 0xFFFFFFFF;
	MEDIA_DEC_TEST_REGISTER_RW( MEDIA_DEC_SOURCE_HEIGHT_OFFSET ) = 0xFFFFFFFF;
	
	/* Set up the target codec */
	MEDIA_DEC_TEST_REGISTER_RW( MEDIA_DEC_VID_TYPE ) = play_cmd->video_type;
	MEDIA_DEC_TEST_REGISTER_RW( MEDIA_DEC_AUD_TYPE ) = play_cmd->audio_type;

#if BCM2708MDEC_DEBUG
	play_cmd->filename[play_cmd->filename_size] = 0;
	bcm2708mdec_dbg("filename=%s size=%d\n", play_cmd->filename, play_cmd->filename_size);
#endif
	/* If user does not provide a full ppath filename, fix it. */
	if (strncmp(play_cmd->filename, VC_MFS_SD_PREFIX, strlen(VC_MFS_SD_PREFIX))) {
		temp_name = (char *)vmalloc(play_cmd->filename_size);
		if (NULL == temp_name) {
			bcm2708mdec_error("Unable to allocate name\n");
			return -ENOMEM;
		}
		strncpy(temp_name, play_cmd->filename, play_cmd->filename_size);
		strcpy(play_cmd->filename, VC_MFS_SD_PREFIX);
		strncat(play_cmd->filename, temp_name, play_cmd->filename_size);
		vfree(temp_name);
		play_cmd->filename_size += strlen(VC_MFS_SD_PREFIX);
	}

#if BCM2708MDEC_DEBUG
        play_cmd->filename[play_cmd->filename_size] = 0;
        bcm2708mdec_dbg("filename=%s size=%d\n", play_cmd->filename, play_cmd->filename_size);
#endif

	/* Write in the filename */	
	strncpy((char *)&MEDIA_DEC_TEST_REGISTER_RW( MEDIA_DEC_DEBUG_FILENAME), play_cmd->filename, play_cmd->filename_size);

	/* Enable the mode */
	MEDIA_DEC_TEST_REGISTER_RW( MEDIA_DEC_CONTROL_OFFSET ) = MEDIA_DEC_CONTROL_ENABLE_BIT | MEDIA_DEC_CONTROL_LOCAL_FILEMODE_BIT;

	ipc_notify_vc_event(g_mdec->irq);	

	/* Wait for it to get ready */
	while ((MEDIA_DEC_TEST_REGISTER_RW( MEDIA_DEC_STATUS_OFFSET) & 0x1) != 0x1) {
		schedule_timeout(100);
		bcm2708mdec_dbg("slept for 1 sec in enabling playback\n");	
	}

	/* start to play */
	MEDIA_DEC_TEST_REGISTER_RW(MEDIA_DEC_CONTROL_OFFSET) |= MEDIA_DEC_CONTROL_PLAY_BIT;

        ipc_notify_vc_event(g_mdec->irq);

	/* Wait for it to start */
	while ((MEDIA_DEC_TEST_REGISTER_RW( MEDIA_DEC_STATUS_OFFSET) & MEDIA_DEC_CONTROL_PLAY_BIT) != MEDIA_DEC_CONTROL_PLAY_BIT) {
		schedule_timeout(100);
		bcm2708mdec_dbg("slept for 1 sec in playback\n");
	}

	for( count = 0; count < 10; count++ ) {
		prev_time = time;
		schedule_timeout(100);
		time = MEDIA_DEC_TEST_REGISTER_RW( MEDIA_DEC_PLAYBACK_TIME );
		if (time == prev_time)
			bcm2708mdec_dbg("the playback ts is not moving\n");
	}

	return ret;
}


static int do_playback_teardown(void)
{
	BUG_ON(0x1 != (0x1 & MEDIA_DEC_TEST_REGISTER_RW( MEDIA_DEC_CONTROL_OFFSET )));

	MEDIA_DEC_TEST_REGISTER_RW( MEDIA_DEC_CONTROL_OFFSET ) = 0;

        ipc_notify_vc_event(g_mdec->irq);

#if 0
	while ((MEDIA_DEC_TEST_REGISTER_RW( MEDIA_DEC_STATUS_OFFSET) & 0x3) != 0x0) {
        	schedule_timeout(100);
		bcm2708mdec_dbg("slept for 1 sec in tear down\n");
	}

	BUG_ON(0x0 != MEDIA_DEC_TEST_REGISTER_RW( MEDIA_DEC_MAX_BUFFER_SIZE));
#endif

	return 0;
}

static int mdec_open( struct inode *inode, struct file *file_id)
{
	return 0;
}

static int mdec_release( struct inode *inode, struct file *file_id )
{
	int ret = 0;

	ret = do_playback_teardown();	
	
	return ret;
}

static ssize_t mdec_read( struct file *file, char *buffer, size_t count, loff_t *ppos )
{
	return -EINVAL;
}

static ssize_t mdec_write( struct file *file, const char *buffer, size_t count, loff_t *ppos )
{
        return -EINVAL;
}

static int mdec_ioctl( struct inode *inode, struct file *file_id, unsigned int cmd, unsigned long arg )
{
	int ret = 0;
	unsigned long uncopied;

	BUG_ON(MAX_BCM2708_MDEC_IOCTL_CMD_SIZE < _IOC_SIZE(cmd));
	uncopied = 
		copy_from_user(g_mdec->ioctl_cmd_buf, (void *)arg, _IOC_SIZE(cmd));
	if (uncopied != 0)
		return -EFAULT;

	switch (cmd) {
	case MDEC_IOCTL_PLAY:
		do_playback((bcm2708_mdec_play_t *)g_mdec->ioctl_cmd_buf);
		break; 

	default: 
		bcm2708mdec_error("Wrong IOCTL cmd\n");
		ret = -EFAULT;
		break;
	}

	return ret;
}

static int mdec_mmap(struct file *filp, struct vm_area_struct *vma)
{
	return 0;
}

static struct file_operations mdec_file_ops =
{
    owner:      THIS_MODULE,
    open:       mdec_open,
    release:    mdec_release,
    read:       mdec_read,
    write:      mdec_write,
    ioctl:      mdec_ioctl,
    mmap:       mdec_mmap,
};

static struct proc_dir_entry *mdec_create_proc_entry( const char * const name,
                                                     read_proc_t *read_proc,
                                                     write_proc_t *write_proc )
{
   struct proc_dir_entry *ret = NULL;

   ret = create_proc_entry( name, 0644, NULL);

   if (ret == NULL)
   {
      remove_proc_entry( name, NULL);
      printk(KERN_ALERT "could not initialize %s", name );
   }
   else
   {
      ret->read_proc  = read_proc;
      ret->write_proc = write_proc;
      ret->mode           = S_IFREG | S_IRUGO;
      ret->uid    = 0;
      ret->gid    = 0;
      ret->size           = 37;
   }
   return ret;
}

static int mdec_dummy_read(char *buf, char **start, off_t offset, int count, int *eof, void *data)
{
   int len = 0;

   if (offset > 0)
   {
      *eof = 1;
      return 0;
   }

   *eof = 1;

   return len;
}

#define INPUT_MAX_INPUT_STR_LENGTH   256

static int mdec_proc_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
        char *init_string = NULL;
	bcm2708_mdec_play_t cmd;
        int num;

        init_string = vmalloc(INPUT_MAX_INPUT_STR_LENGTH);

   if(NULL == init_string)
      return -EFAULT;

   memset(init_string, 0, INPUT_MAX_INPUT_STR_LENGTH);

   count = (count > INPUT_MAX_INPUT_STR_LENGTH) ? INPUT_MAX_INPUT_STR_LENGTH : count;

   if(copy_from_user(init_string, buffer, count))
   {
      return -EFAULT;
   }
   init_string[ INPUT_MAX_INPUT_STR_LENGTH  - 1 ] = 0;

	num = simple_strtol(init_string, 0, 0);
   bcm2708mdec_dbg("read from /proc is %d\n", num);

   if (8 == num) 
   {

   cmd.audio_type = MEDIA_DEC_AUDIO_CodingUnused;
   cmd.video_type = MEDIA_DEC_VIDEO_CodingAVC;
   strncpy(cmd.filename, "/mfs/sd/bond2.264", strlen("/mfs/sd/bond2.264"));
   cmd.filename_size = strlen("/mfs/sd/bond2.264");

   do_playback(&cmd);
   } else
   {
	do_playback_teardown();	
   }

   vfree(init_string);

   return count;
}


static irqreturn_t bcm2708_mdec_isr(int irq, void *dev_id)
{
	bcm2708mdec_dbg("The MDEC device rxed one interrupt");

	return IRQ_HANDLED;
}

struct miscdevice mdec_misc_dev = {
    .minor =    MISC_DYNAMIC_MINOR,
    .name =     BCM2708_MDEC_DRIVER_NAME,
    .fops =     &mdec_file_ops
};

static int bcm2708_mdec_probe(struct platform_device *pdev)
{
        int ret = -ENXIO;
        struct resource *r;
	struct bcm2708_mdec *bcm_mdec = NULL;

        bcm_mdec = kzalloc(sizeof(struct bcm2708_mdec), GFP_KERNEL);
        if (bcm_mdec == NULL) {
                bcm2708mdec_error("Unable to allocate mdec structure\n");
                ret = -ENOMEM;
                goto err_mdec_alloc_failed;
        }
	g_mdec = bcm_mdec;
        platform_set_drvdata(pdev, bcm_mdec);

	r = platform_get_resource(pdev, IORESOURCE_MEM, 0);
        if(r == NULL) {
                bcm2708mdec_error("Unable to get mdec memory resource\n");
                ret = -ENODEV;
                goto err_no_io_base;
        }
        bcm2708mdec_dbg("MDEC registers start-end (0x%08x)-(0x%08x)\n", r->start, r->end);
	bcm_mdec->reg_base = (void __iomem *)r->start;

	bcm_mdec->irq = platform_get_irq(pdev, 0);
        if(bcm_mdec->irq < 0) {
                bcm2708mdec_error("Unable to get mdec irq resource\n");
                ret = -ENODEV;
                goto err_no_irq;
        }

        ret = request_irq(bcm_mdec->irq, bcm2708_mdec_isr, IRQF_DISABLED,
                                "bcm2708 mdec interrupt", (void *)bcm_mdec);
        if (ret < 0) {
                bcm2708mdec_error("Unable to register Interrupt for bcm2708 MDEC\n");
                goto err_no_irq;
        }


	ret = misc_register(&mdec_misc_dev);
	if (ret < 0) {
		bcm2708mdec_error("failed to register char device\n");
		goto err_reg_chrdev;
	}

       mdec_create_proc_entry("bcm2835_mdec", mdec_dummy_read, mdec_proc_write);

       bcm2708mdec_dbg("The MDEC device is probed successfully");
	
	return 0;

err_reg_chrdev:
	free_irq(bcm_mdec->irq, NULL);
err_no_irq:
err_no_io_base:
	kfree(bcm_mdec);
err_mdec_alloc_failed:
	return ret;

}

static int __devexit bcm2708_mdec_remove(struct platform_device *pdev)
{
        struct bcm2708_mdec *bcm_mdec = platform_get_drvdata(pdev);

	free_irq(bcm_mdec->irq, NULL);
	misc_deregister(&mdec_misc_dev);
        kfree(bcm_mdec);
        bcm2708mdec_dbg("BCM2708 MDEC device removed!!\n");

        return 0;
}


static struct platform_driver bcm2708_mdec_driver = {
        .probe          = bcm2708_mdec_probe,
        .remove         = __devexit_p(bcm2708_mdec_remove),
        .driver = {
                .name = "bcm2835_MEDD"
        }
};

static int __init bcm2708_mdec_init(void)
{
        int ret;

        ret = platform_driver_register(&bcm2708_mdec_driver);
        if (ret)
                printk(KERN_ERR BCM2708_MDEC_DRIVER_NAME "%s : Unable to register BCM2708 MDEC driver\n", __func__);

        printk(KERN_INFO BCM2708_MDEC_DRIVER_NAME "Init %s !\n", ret ? "FAILED" : "OK");

        return ret;
}

static void __exit bcm2708_mdec_exit(void)
{
        /* Clean up .. */
        platform_driver_unregister(&bcm2708_mdec_driver);

        printk(KERN_INFO BCM2708_MDEC_DRIVER_NAME "BRCM MDEC driver exit OK\n");
}

module_init(bcm2708_mdec_init);
module_exit(bcm2708_mdec_exit);
