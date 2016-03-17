 /*****************************************************************************
 *
 * Title: Linux Device Driver for Proximity Sensor GP2AP002S00F
 * COPYRIGHT(C) : Samsung Electronics Co.Ltd, 2006-2015 ALL RIGHTS RESERVED
 *
 *****************************************************************************/
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/irq.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/i2c.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/input.h>
#include <linux/workqueue.h>
#include <linux/delay.h>
#include <linux/wakelock.h>
#include <mach/hardware.h>
#include <asm/gpio.h>  
#include <linux/pm.h>
 
#include <linux/gp2ap002_dev.h> 
#include <linux/gp2ap002.h>
#include <linux/regulator/consumer.h>

#define GP2A_DEBUG 0
#define error(fmt, arg...) printk("--------" fmt "\n", ##arg)
#if GP2A_DEBUG
#define PROXDBG(fmt, args...) printk(KERN_INFO fmt, ## args)
#define debug(fmt, arg...) printk("--------" fmt "\n", ##arg)
#else
#define PROXDBG(fmt, args...)
#define debug(fmt,arg...)
#endif

#if defined(CONFIG_MACH_HAWAII_SS_LOGAN) || defined(CONFIG_MACH_HAWAII_SS_LOGANDS)

#define PROX_NONDETECT	0x2F
#define PROX_DETECT		0x0C

#else

#define PROX_NONDETECT	0x40
#define PROX_DETECT		0x20

#endif

#define PROX_NONDETECT_MODE1	0x43
#define PROX_DETECT_MODE1		0x28

#define PROX_NONDETECT_MODE2	0x48
#define PROX_DETECT_MODE2		0x42

#define OFFSET_FILE_PATH	"/efs/prox_cal"
#define CHIP_NAME	"GP2AP002"

/*Driver data */
struct gp2a_prox_data {
	void (*power_on) (bool);
	void (*led_on) (bool);
	unsigned int irq_gpio;
	int    irq;
	struct i2c_client* gp2a_prox_i2c_client;
	struct work_struct work_prox;  /* for proximity sensor */ 
	struct input_dev *prox_input_dev;
	struct mutex power_lock;    
	struct wake_lock prx_wake_lock;
	u8 reg[7];
    	char cal_mode;
};

static struct i2c_driver  gp2a_prox_i2c_driver;
static struct gp2a_prox_data *gp2a_data;
static struct workqueue_struct *gp2a_prox_wq;
static bool proximity_enable = OFF;
static short proximity_value = 0;
static int nondetect;
static int detect;

#if defined(CONFIG_SENSORS_CORE)
extern struct class *sensors_class;
#endif

/*Only One Read Only register, so word address need not be specified (from Data Sheet)*/
static int gp2a_i2c_read(u8 reg, u8 *value)
{
	int ret =0;
	int count=0;
	u8 buf[3];
	struct i2c_msg msg[1];

	buf[0] = reg;
	
	/*first byte read(buf[0]) is dummy read*/
	msg[0].addr = gp2a_data->gp2a_prox_i2c_client->addr;
	msg[0].flags = I2C_M_RD;	
	msg[0].len = 2;
	msg[0].buf = buf;
	count = i2c_transfer(gp2a_data->gp2a_prox_i2c_client->adapter, msg, 1);
	
	if(count < 0)
	{
		debug("%s %d i2c transfer error\n", __func__, __LINE__);
		ret =-1;
	}
	else
	{
		*value = buf[0] << 8 | buf[1];
		debug("value=%d", *value);
	}
	
	return ret;	
}

static int gp2a_i2c_write( u8 reg, u8 *value )
{
    int ret =0;
    int count=0;
    struct i2c_msg msg[1];
    u8 data[2];

    if( (gp2a_data->gp2a_prox_i2c_client == NULL) || (!gp2a_data->gp2a_prox_i2c_client->adapter) ){
        return -ENODEV;
    }	

    data[0] = reg;
    data[1] = *value;

    msg[0].addr = gp2a_data->gp2a_prox_i2c_client->addr;
    msg[0].flags = 0;
    msg[0].len = 2;
    msg[0].buf 	= data;
    count = i2c_transfer(gp2a_data->gp2a_prox_i2c_client->adapter,msg,1);
	
    if(count < 0)
        ret =-1;
	
    return ret;
}

static int gp2a_prox_offset(unsigned char vout)
{	
    	u8 reg_value;
	int ret=0;
	debug("[GP2A] %s called\n",__func__); 

	/* Write HYS Register */
	if(!vout)
		reg_value = nondetect;
	else
		reg_value = detect;
    
	if((ret=gp2a_i2c_write(GP2A_REG_HYS/*0x02*/,&reg_value))<0)
			error("gp2a_i2c_write 2 failed\n");
	
	return ret;
}

static int gp2a_prox_reset(void)
{
    	u8 reg_value;
	int ret=0;
	debug("[GP2A] %s called\n",__func__); 

	reg_value = 0x18;
	if((ret=gp2a_i2c_write(GP2A_REG_CON/*0x06*/,&reg_value))<0)
			error("gp2a_i2c_write 4 failed\n");	
    
	reg_value = 0x08;
	if((ret=gp2a_i2c_write(GP2A_REG_GAIN/*0x01*/,&reg_value))<0)
			error("gp2a_i2c_write 1 failed\n");

        gp2a_prox_offset(0);
        
	reg_value = 0x04;
	if((ret=gp2a_i2c_write(GP2A_REG_CYCLE/*0x03*/,&reg_value))<0)
			error("gp2a_i2c_write 3 failed\n");
	
	return ret;
}

static int gp2a_prox_mode(int enable)
{	
	u8 reg_value;
	int ret=0;
	debug("%s called\n",__func__); 
    
	if(1==enable)
	{
        	if (gp2a_data->led_on){
                    gp2a_data->led_on(1);
        	}
        
		reg_value = 0x18;
		if((ret=gp2a_i2c_write(GP2A_REG_CON,&reg_value))<0)
			error("gp2a_i2c_write 1 failed");
		
                gp2a_prox_offset(0);
        
		reg_value = 0x03;
		if((ret=gp2a_i2c_write(GP2A_REG_OPMOD,&reg_value))<0)
			error("gp2a_i2c_write 3 failed");

		enable_irq(gp2a_data->irq);
		
		reg_value = 0x00;
		if((ret=gp2a_i2c_write(GP2A_REG_CON,&reg_value))<0)
			error("gp2a_i2c_write 4 failed");
		
		proximity_enable=1;
	}
	else 
	{
		disable_irq_nosync(gp2a_data->irq);

		reg_value = 0x02;
		if((ret=gp2a_i2c_write(GP2A_REG_OPMOD,&reg_value))<0)
			error("gp2a_i2c_write 3 failed");
		
        	if (gp2a_data->led_on){
                    gp2a_data->led_on(0);
        	}
		
		proximity_enable=0;
                proximity_value = 0;
	}   
	
	return ret;
}


static void gp2a_chip_init(void)
{
	debug("%s called",__func__); 
}


static void gp2a_prox_work_func(struct work_struct *work)
{
	unsigned char value;
	unsigned char int_val = GP2A_REG_PROX;
	unsigned char vout = 0;
        int ret=0;

	/* Read VO & INT Clear */	
	debug("[PROXIMITY] %s : \n",__func__);
    
	if((ret=gp2a_i2c_read((u8)(int_val), &value))<0)
	{
            error("gp2a_i2c_read  failed\n");            
            gp2a_prox_reset();
            
            if(proximity_enable == 1)
                gp2a_prox_mode(1);
            else
                gp2a_prox_mode(0);
            
            return;
	}
    
	vout = value & 0x01;
	printk(KERN_INFO "[GP2A] vout = %d \n",vout);

	/* Report proximity information */ 
	proximity_value = vout;

        input_report_abs(gp2a_data->prox_input_dev, ABS_DISTANCE,((vout == 1)? 0:1));
        input_sync(gp2a_data->prox_input_dev);
        mdelay(1);

	/* Write HYS Register */
        gp2a_prox_offset(vout);

	/* Forcing vout terminal to go high */
	value = 0x18;
	gp2a_i2c_write((u8)(GP2A_REG_CON),&value);

	/* enable INT */
	enable_irq(gp2a_data->irq);
	printk(KERN_INFO "[GP2A] enable_irq IRQ_NO:%d\n",gp2a_data->irq);

	/* enabling VOUT terminal in nomal operation */
	value = 0x00;
	gp2a_i2c_write((u8)(GP2A_REG_CON),&value);
	
}


static irqreturn_t gp2a_irq_handler( int irq, void *unused )
{
  	printk(KERN_INFO "[GP2A] gp2a_irq_handler called\n");
	if(gp2a_data->irq !=-1)
	{
        	wake_lock_timeout(&gp2a_data->prx_wake_lock, 3 * HZ);
            
		disable_irq_nosync(gp2a_data->irq);
		printk(KERN_INFO "[GP2A] disable_irq : IRQ_NO:%d\n",gp2a_data->irq);
		queue_work(gp2a_prox_wq, &gp2a_data->work_prox);
	}
	else
	{
		error("PROX_IRQ not handled");
		return IRQ_NONE;
	}
	debug("PROX_IRQ handled");
	return IRQ_HANDLED;
}

static int gp2a_prox_cal_mode(char mode)
{
	int ret=0;
	u8 reg_value;

	if (mode == 1) 
        {
		nondetect = PROX_NONDETECT_MODE1;
		detect = PROX_DETECT_MODE1;
	} 
        else if (mode == 2) 
        {
		nondetect = PROX_NONDETECT_MODE2;
		detect = PROX_DETECT_MODE2;
	} 
        else
        {
		nondetect = PROX_NONDETECT;
		detect = PROX_DETECT;
	} 
    
	reg_value = 0x08;
	if((ret=gp2a_i2c_write(GP2A_REG_GAIN/*0x01*/,&reg_value))<0)
			error("gp2a_i2c_write 1 failed\n");

        gp2a_prox_offset(0);
        
	reg_value = 0x04;
	if((ret=gp2a_i2c_write(GP2A_REG_CYCLE/*0x03*/,&reg_value))<0)
			error("gp2a_i2c_write 3 failed\n");

	reg_value = 0x03;
	if((ret=gp2a_i2c_write(GP2A_REG_OPMOD,&reg_value))<0)
		error("gp2a_i2c_write 3 failed");

	reg_value = 0x00;
	if((ret=gp2a_i2c_write(GP2A_REG_CON,&reg_value))<0)
		error("gp2a_i2c_write 4 failed");

        return ret;

}

static int gp2a_cal_mode_read_file(char *mode)
{
	int err = 0;
	mm_segment_t old_fs;
	struct file *cal_mode_filp = NULL;

	old_fs = get_fs();
	set_fs(KERNEL_DS);

	cal_mode_filp = filp_open(OFFSET_FILE_PATH, O_RDONLY, 0666);
	if (IS_ERR(cal_mode_filp)) {
		printk(KERN_INFO "[GP2A] %s: no cal_mode file\n", __func__);        
		err = PTR_ERR(cal_mode_filp);
		if (err != -ENOENT)
			pr_err("[GP2A] %s: Can't open cal_mode file\n", __func__);
		set_fs(old_fs);
		return err;
	}
	err = cal_mode_filp->f_op->read(cal_mode_filp, mode, sizeof(u8), &cal_mode_filp->f_pos);

	if (err != sizeof(u8)) {
		pr_err("%s: Can't read the cal_mode from file\n",	__func__);
		filp_close(cal_mode_filp, current->files);
		set_fs(old_fs);
		return -EIO;
	}
               
	filp_close(cal_mode_filp, current->files);
	set_fs(old_fs);

	return err;
}

static int gp2a_cal_mode_save_file(char mode)
{
	struct file *cal_mode_filp = NULL;
	int err = 0;
	mm_segment_t old_fs;

        gp2a_prox_cal_mode(mode);

	old_fs = get_fs();
	set_fs(KERNEL_DS);

	cal_mode_filp = filp_open(OFFSET_FILE_PATH, O_CREAT | O_TRUNC | O_WRONLY | O_SYNC, 0666);
	if (IS_ERR(cal_mode_filp)) 
        {
		pr_err("%s: Can't open cal_mode file\n", __func__);
		set_fs(old_fs);
		err = PTR_ERR(cal_mode_filp);
		pr_err("%s: err = %d\n", __func__, err);
		return err;
	}

	err = cal_mode_filp->f_op->write(cal_mode_filp, (char *)&mode, sizeof(u8), &cal_mode_filp->f_pos);
	if (err != sizeof(u8)) {
		pr_err("%s: Can't read the cal_mode from file\n", __func__);
		err = -EIO;
	}
               
	filp_close(cal_mode_filp, current->files);
	set_fs(old_fs);

	return err;
}


static ssize_t proximity_cal_show(struct device *dev, struct device_attribute *attr, char *buf)
{
        int result = 0;    
	result = gp2a_data->cal_mode;
	printk(KERN_INFO "[GP2A] prox_cal_read = %d\n", result);        

	return sprintf(buf, "%d\n", result);
}

static ssize_t proximity_cal_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
       
	if (sysfs_streq(buf, "1")) 
		gp2a_data->cal_mode = 1;
        else if (sysfs_streq(buf, "2")) 
		gp2a_data->cal_mode = 2;
        else if (sysfs_streq(buf, "0")) 
		gp2a_data->cal_mode = 0;
        else  {
		pr_err("[GP2A] %s: invalid value %d\n", __func__, *buf);
		return -EINVAL;
	}      

        printk(KERN_INFO "[GP2A] prox_cal_write =%d\n", gp2a_data->cal_mode); 
        
        gp2a_cal_mode_save_file(gp2a_data->cal_mode);
        
	return size;
}

static ssize_t proximity_name_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%s\n", CHIP_NAME);
}

static DEVICE_ATTR(prox_cal, S_IRUGO | S_IWUSR, proximity_cal_show, proximity_cal_store);
static DEVICE_ATTR(name, S_IRUGO, proximity_name_show, NULL);

static ssize_t proximity_enable_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", proximity_enable);
}

static ssize_t proximity_enable_store(struct device *dev, struct device_attribute *attr,  const char *buf, size_t size)
{
    bool new_value;

    if (sysfs_streq(buf, "1"))
        new_value = true;
    else if (sysfs_streq(buf, "0"))
        new_value = false;
    else {
        pr_err("%s: invalid value %d\n", __func__, *buf);
        return -EINVAL;
    }

    printk(KERN_INFO "[GP2A] proximity_enable_store: new_value=%d mode=%d\n", new_value, gp2a_data->cal_mode);   
   
    mutex_lock(&gp2a_data->power_lock);   
   
    if (new_value ){          
        input_report_abs(gp2a_data->prox_input_dev, ABS_DISTANCE, 1);
        input_sync(gp2a_data->prox_input_dev);        
        gp2a_prox_mode(1);        
    }
    else if (!new_value ) 
    {
        gp2a_prox_mode(0);
    }

    mutex_unlock(&gp2a_data->power_lock);
    return size;
}

static struct device_attribute dev_attr_prox_enable =
	__ATTR(enable, S_IRUGO | S_IWUSR | S_IWGRP,
	       proximity_enable_show, proximity_enable_store);

static struct attribute *gp2a_prox_attributes[] = {
	&dev_attr_prox_enable.attr,
	NULL
};

static struct attribute_group gp2a_prox_attr_group = {
	.attrs = gp2a_prox_attributes,
};


static int gp2a_prox_open(struct inode *ip, struct file *fp)
{
	debug("%s called",__func__);
	return nonseekable_open(ip, fp);	
}

static int gp2a_prox_release(struct inode *ip, struct file *fp)
{	
	debug("%s called",__func__);
	return 0;
}

static long gp2a_prox_ioctl(struct file *filp, unsigned int ioctl_cmd,  unsigned long arg)
{	
    int ret = 0;
    short data = 0;
    char cal_mode=0;

    if( _IOC_TYPE(ioctl_cmd) != PROX_IOC_MAGIC )
    {
        error("Wrong _IOC_TYPE 0x%x",ioctl_cmd);
        return -ENOTTY;
    }

    switch (ioctl_cmd)
    {
        case PROX_IOC_NORMAL_MODE:
        {
            printk(KERN_INFO "[GP2A] PROX_IOC_NORMAL_MODE\n");
            if(0==proximity_enable)
            {
                if( (ret = gp2a_prox_mode(1)) < 0 )        
                    error("PROX_IOC_NORMAL_MODE failed"); 
            }
            else
                debug("Proximity Sensor is already Normal Mode");
            break;
        }
        case PROX_IOC_SHUTDOWN_MODE:			
        {
            printk(KERN_INFO "[GP2A] PROX_IOC_SHUTDOWN_MODE\n");				
            if(1==proximity_enable)
            {
            	if( (ret = gp2a_prox_mode(0)) < 0 )        
            		error("PROX_IOC_SHUTDOWN_MODE failed"); 
            }
            else
            	debug("Proximity Sensor is already set in Shutdown mode");
            break;
        }
	case PROX_IOC_SET_CALIBRATION:
        {
		printk(KERN_INFO "[GP2A] PROX_IOC_SET_CALIBRATION\n");                
		if (copy_from_user(&data, (void __user *)arg, sizeof(data)))
			return -EFAULT;

                ret = gp2a_cal_mode_read_file(&cal_mode);
		if (ret < 0 && ret != -ENOENT)
		{
			printk(KERN_INFO "[GP2A] gp2a_cal_mode_read_file() failed\n");
		}else {
		    if(cal_mode >=0 && cal_mode <=2){
                        gp2a_prox_cal_mode(cal_mode);
                        gp2a_data->cal_mode = cal_mode;                    
                        printk(KERN_INFO "[GP2A] cal mode (%d)\n", gp2a_data->cal_mode);
		    } else {
                        gp2a_data->cal_mode = 0;     
		    }
		}
		break;
	}
	case PROX_IOC_GET_CALIBRATION:
        {
		printk(KERN_INFO "[GP2A] PROX_IOC_GET_CALIBRATION\n");      
                data = gp2a_data->cal_mode;
		if (copy_to_user((void __user *)arg, &data, sizeof(data)))
			return -EFAULT;
		break;
	}
	default:
            error("Unknown IOCTL command");
            ret = -ENOTTY;
            break;
    }
    return ret;
}

static struct file_operations gp2a_prox_fops = {
	.owner  	= THIS_MODULE,
	.open   	= gp2a_prox_open,
	.release 	= gp2a_prox_release,    
	.unlocked_ioctl 	= gp2a_prox_ioctl,
};

static struct miscdevice gp2a_prox_misc_device = {
    .minor  = MISC_DYNAMIC_MINOR,
    .name   = "proximity",
    .fops   = &gp2a_prox_fops,
};

static int gp2a_prox_probe(struct i2c_client *client,const struct i2c_device_id *id)
{
	int ret =0;
	u8 reg_value;
	struct gp2ap002_platform_data *platform_data;

	printk(KERN_INFO "[GP2A] %s start \n", __func__);	
	
	/* Allocate driver_data */
	gp2a_data = kzalloc(sizeof(struct gp2a_prox_data),GFP_KERNEL);
	if(!gp2a_data)
	{
		error("kzalloc:allocating driver_data error");
		return -ENOMEM;		
	} 
	
    	platform_data = client->dev.platform_data;

	if (platform_data->power_on){
		gp2a_data->power_on = platform_data->power_on;
                gp2a_data->power_on(1);
	}

    	if (platform_data->led_on){
		gp2a_data->led_on = platform_data->led_on;            
		gp2a_data->led_on(1);
    	}
        
	gp2a_data->gp2a_prox_i2c_client = client;
	i2c_set_clientdata(client, gp2a_data);
	
	/*misc device registration*/
	if( (ret = misc_register(&gp2a_prox_misc_device)) < 0 )
	{
		error("gp2a_prox driver misc_register failed");
		goto FREE_GP2A_DATA;
	}
	
	gp2a_data->irq_gpio = platform_data->irq_gpio;		
	/*Initialisation of GPIO_PS_OUT of proximity sensor*/
	if (gpio_request(gp2a_data->irq_gpio, "Proximity Out")) {
		printk(KERN_ERR "[GP2A] Proximity Request GPIO_%d failed!\n", gp2a_data->irq_gpio);
	} 
        else {
    		printk(KERN_ERR "[GP2A] Proximity Request GPIO_%d Sucess!\n", gp2a_data->irq_gpio);
	}
	
	gpio_direction_input(gp2a_data->irq_gpio);

	mutex_init(&gp2a_data->power_lock);
	
	/*Input Device Settings*/
	gp2a_data->prox_input_dev = input_allocate_device();
	if (!gp2a_data->prox_input_dev) 
	{
		error("Not enough memory for gp2a_data->prox_input_dev");
		ret = -ENOMEM;
		goto MISC_DREG;
	}
	gp2a_data->prox_input_dev->name = "proximity_sensor";
	set_bit(EV_SYN,gp2a_data->prox_input_dev->evbit);
	set_bit(EV_ABS,gp2a_data->prox_input_dev->evbit);	

	input_set_capability(gp2a_data->prox_input_dev, EV_ABS, ABS_DISTANCE);    
	input_set_abs_params(gp2a_data->prox_input_dev, ABS_DISTANCE, 0, 1, 0, 0);
	ret = input_register_device(gp2a_data->prox_input_dev);
	if (ret) 
	{
		error("Failed to register input device");
		input_free_device(gp2a_data->prox_input_dev);
		goto MISC_DREG;
	}
	debug("Input device settings complete");
    
	/*create sysfs attributes*/
	ret = sysfs_create_group(&gp2a_data->prox_input_dev->dev.kobj, &gp2a_prox_attr_group);
	if (ret)
	{
		error("Failed to create sysfs attributes");
		goto MISC_DREG;
	}
    
	/* Workqueue Settings */
	gp2a_prox_wq = create_singlethread_workqueue("gp2a_prox_wq");
	if (!gp2a_prox_wq)
	{
		error("Not enough memory for gp2a_prox_wq");
		ret = -ENOMEM;
		goto INPUT_DEV_DREG;
	}	     
	INIT_WORK(&gp2a_data->work_prox, gp2a_prox_work_func);
	debug("Workqueue settings complete");	
            
	/* wake lock init */
	wake_lock_init(&gp2a_data->prx_wake_lock, WAKE_LOCK_SUSPEND,
		"prx_wake_lock");
            
	gp2a_data->irq = platform_data->irq;    
    
        irq_set_irq_type(gp2a_data->irq, IRQ_TYPE_EDGE_FALLING);	
	if( (ret = request_irq(gp2a_data->irq, gp2a_irq_handler,IRQF_DISABLED | IRQF_NO_SUSPEND , "proximity_int", NULL )) )
	{
		error("GP2A request_irq failed IRQ_NO:%d", gp2a_data->irq);
		goto DESTROY_WORK_QUEUE;
	} 
	else {
		printk(KERN_INFO "[GP2A] request_irq success IRQ_NO:%d", gp2a_data->irq);
	}
	
	/*Device Initialisation with recommended register values from datasheet*/
	
	reg_value = 0x18;
	if((ret=gp2a_i2c_write(GP2A_REG_CON,&reg_value))<0)
			error("gp2a_i2c_write 1 failed");	
		
	reg_value = 0x08;
	if((ret=gp2a_i2c_write(GP2A_REG_GAIN,&reg_value))<0)
			error("gp2a_i2c_write 2 failed");
	
	reg_value = PROX_NONDETECT;
	if((ret=gp2a_i2c_write(GP2A_REG_HYS,&reg_value))<0)
			error("gp2a_i2c_write 3 failed");
	
	reg_value = 0x04;
	if((ret=gp2a_i2c_write(GP2A_REG_CYCLE,&reg_value))<0)
			error("gp2a_i2c_write 4 failed");

	gp2a_data->cal_mode = 0;
        nondetect = PROX_NONDETECT;
        detect = PROX_DETECT;
    
	/*Pulling the GPIO_PS_OUT Pin High*/
	printk(KERN_INFO "[GP2A] gpio_get_value of %d is %d\n",gp2a_data->irq_gpio,gpio_get_value(gp2a_data->irq_gpio));

	/*Setting the device into shutdown mode*/
	gp2a_prox_mode(0);

	/* set initial proximity value as 1 */
	input_report_abs(gp2a_data->prox_input_dev, ABS_DISTANCE, 1);
	input_sync(gp2a_data->prox_input_dev);

	printk(KERN_INFO "[GP2A] %s end\n", __func__);	
	return ret;
    
DESTROY_WORK_QUEUE:
	wake_lock_destroy(&gp2a_data->prx_wake_lock);
	destroy_workqueue(gp2a_prox_wq);
INPUT_DEV_DREG:
	input_unregister_device(gp2a_data->prox_input_dev);	
MISC_DREG:
	misc_deregister(&gp2a_prox_misc_device);
FREE_GP2A_DATA:
	kfree(gp2a_data);
	return ret;
}	

static int __devexit gp2a_prox_remove(struct i2c_client *client)
{	
  	debug("%s called",__func__); 
	gp2a_prox_mode(0);
	gp2a_data->gp2a_prox_i2c_client = NULL;
	free_irq(gp2a_data->irq,NULL);
	sysfs_remove_group(&client->dev.kobj, &gp2a_prox_attr_group);
	wake_lock_destroy(&gp2a_data->prx_wake_lock);    
	destroy_workqueue(gp2a_prox_wq);
	input_unregister_device(gp2a_data->prox_input_dev);	
	misc_deregister(&gp2a_prox_misc_device);
	mutex_destroy(&gp2a_data->power_lock);
	kfree(gp2a_data);
	return 0;
}


#ifdef CONFIG_PM
static int gp2a_prox_suspend(struct device *dev)
{   	   
    debug("%s called",__func__); 
    return 0;
}
static int gp2a_prox_resume(struct device *dev)
{  	   
    debug("%s called",__func__); 
    return 0;
}
#else
#define gp2a_prox_suspend NULL
#define gp2a_prox_resume NULL
#endif

static const struct i2c_device_id gp2a_prox_id[] = {
	{"gp2ap002", 0},
	{}
};

MODULE_DEVICE_TABLE(i2c, gp2a_prox_id);

static const struct dev_pm_ops gp2a_prox_pm_ops = {
	.suspend = gp2a_prox_suspend,
	.resume = gp2a_prox_resume,
};

static struct i2c_driver gp2a_prox_i2c_driver = {
	.driver = {
                .name = "gp2ap002",
                .owner = THIS_MODULE,
                .pm = &gp2a_prox_pm_ops,
        },
	.probe 		= gp2a_prox_probe,
	.remove 	= __exit_p(gp2a_prox_remove),
	.id_table 	= gp2a_prox_id,
};

static int __init gp2a_prox_init(void)
{
#if defined(CONFIG_SENSORS_CORE)
	struct device *dev_t;    
#endif
    
	debug("%s called",__func__); 
	
	gp2a_chip_init();

#if defined(CONFIG_SENSORS_CORE)
	dev_t = device_create( sensors_class, NULL, 0, NULL, "proximity_sensor");

	if (device_create_file(dev_t, &dev_attr_prox_cal) < 0)
		printk(KERN_ERR "Failed to create device file(%s)!\n", dev_attr_prox_cal.attr.name);
	if (device_create_file(dev_t, &dev_attr_name) < 0)
		printk(KERN_ERR "Failed to create device file(%s)!\n", dev_attr_name.attr.name);

	if (IS_ERR(dev_t)) 
	{
            return PTR_ERR(dev_t);
	}
#endif  
	
	return i2c_add_driver(&gp2a_prox_i2c_driver);

}
static void __exit gp2a_prox_exit(void)
{	
	debug("%s called",__func__);

#if defined(CONFIG_SENSORS_CORE)    
    	device_destroy(sensors_class, 0);
#endif    	
	i2c_del_driver(&gp2a_prox_i2c_driver);	
}

module_init(gp2a_prox_init);
module_exit(gp2a_prox_exit);

MODULE_DESCRIPTION("Proximity Sensor driver for GP2AP002S00F");
MODULE_AUTHOR("SHARP Electronic");
MODULE_LICENSE("GPL"); 
