
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <asm/uaccess.h>
#include <linux/unistd.h>
#include <linux/delay.h>
#include <linux/miscdevice.h>

#include <linux/dac6571.h>


struct dac6571_data {
     unsigned speed;
     unsigned mode;
     unsigned defmode;
     unsigned smode;
     short value;
};

static const unsigned short dac6571_normal_i2c[] = 
{
     0x4C,0x4D,I2C_CLIENT_END
};


static struct i2c_client * dac6571_client=NULL;

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Brendan Moran");
MODULE_DESCRIPTION("DAC6571 device driver");

/*	read command for BMA150 device file	*/
static ssize_t dac6571_read(struct file *file, char __user *buf, size_t count, loff_t *offset)
{	
	if( dac6571_client == NULL )
	{
#ifdef DAC6571_DEBUG
		printk(KERN_INFO "I2C driver not install\n");
#endif
		return -1;
	}
   return 0;
}

/*	write command for dac6571 device file	*/
static ssize_t dac6571_write(struct file *file, const char __user *buf, size_t count, loff_t *offset)
{
	if( dac6571_client == NULL )
		return -1;
#ifdef DAC6571_DEBUG
	   printk(KERN_INFO "dac6571 should be accessed with ioctl command\n");
#endif
	return 0;
}

/*	open command for dac6571 device file	*/
static int dac6571_open(struct inode *inode, struct file *file)
{
#ifdef DAC6571_DEBUG
		printk(KERN_INFO "%s\n",__FUNCTION__); 
#endif

	if( dac6571_client == NULL)
	{
#ifdef DAC6571_DEBUG
		printk(KERN_INFO "I2C driver not install\n");
#endif
		return -1;
	}
	return 0;
}

/*	release command for dac6571 device file	*/
static int dac6571_close(struct inode *inode, struct file *file)
{
#ifdef DAC6571_DEBUG
	printk(KERN_INFO "%s\n",__FUNCTION__);	
#endif
	return 0;
}

/*	ioctl command for dac6571 device file	*/
static int dac6571_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
{
	int rc = 0;
	unsigned short data;
	unsigned short* adata;
	unsigned char buf[2];
	struct dac6571_data* ldata;
   int argsize=0;
	int i;
#ifdef DAC6571_DEBUG
	//printk(KERN_INFO "%s\n",__FUNCTION__);	
#endif

	/* check cmd */
	if(_IOC_TYPE(cmd) != DAC6571_IOC_MAGIC)	
	{
#ifdef DAC6571_DEBUG
		printk(KERN_INFO "cmd magic type error\n");
#endif
		return -ENOTTY;
	}
	if(_IOC_NR(cmd) > DAC6571_IOC_MAXNR)
	{
#ifdef DAC6571_DEBUG
		printk(KERN_INFO "cmd number error\n");
#endif
		return -ENOTTY;
	}

	if(_IOC_DIR(cmd) & _IOC_READ)
		rc = !access_ok(VERIFY_WRITE,(void __user*)arg, _IOC_SIZE(cmd));
	else if(_IOC_DIR(cmd) & _IOC_WRITE)
		rc = !access_ok(VERIFY_READ, (void __user*)arg, _IOC_SIZE(cmd));
	if(rc)
	{
#ifdef DAC6571_DEBUG
		printk(KERN_INFO "cmd access_ok error\n");
#endif
		return -EFAULT;
	}

	if( dac6571_client == NULL)
	{
#ifdef DAC6571_DEBUG
		printk(KERN_INFO "I2C driver not install\n");
#endif
		return -EFAULT;
	}

   ldata = dev_get_drvdata(&dac6571_client->dev);
   argsize=_IOC_SIZE(cmd);
	switch(cmd)
	{
   /* set power state */
   case DAC6571_SET_PWR:
      if(0 != (rc = copy_from_user((unsigned char*)&data,(unsigned char*)arg,sizeof(unsigned short))))
      {
         rc=-EFAULT;
         goto copy_from_err;
      }
      ldata->mode=data;
      ldata->smode=data<<12;      
      break;
   /* set output value */
   case DAC6571_SET_VAL:
      if(0 != (rc = copy_from_user((unsigned char*)&data,(unsigned char*)arg,sizeof(unsigned short))))
      {
         rc=-EFAULT;
         goto copy_from_err;
      }
      ldata->value=data;
      data <<= 2;
      data &= 0xfff;
      data |= ldata->smode;
      buf[0]=data >> 8;
      buf[1]=data & 0xff;
      rc = i2c_master_send(dac6571_client, buf, 2);
      break;
   /* set many output values */
   case DAC6571_SET_ARRAY:
      adata=kmalloc(argsize, GFP_KERNEL);
      if (adata == NULL) {
#ifdef DAC6571_DEBUG
         printk(KERN_INFO "kmalloc failed\n");
#endif
   		return -ENOMEM;
   	}
      if(0 != (rc = copy_from_user(adata,(unsigned char*)arg,argsize)))
      {
         kfree(adata);
         goto copy_from_err;
      }
   	for (i=0;i<(argsize>>1);i++){
   	   adata[i]<<=2;
   	   adata[i] &= 0xfff;
   	   adata[i] |= ldata->smode;
   	   buf[0] = adata[i] >> 8;
   	   buf[1] = adata[i] & 0xff;
   	   adata[i] = *(unsigned short*)buf;
   	}
      rc = i2c_master_send(dac6571_client, (unsigned char*)adata, argsize);
      printk(KERN_INFO "DAC6571: %i bytes transfered\n", rc);
   	kfree(adata);
   	break;
   /* Set communication speed */
   case DAC6571_SET_SPD:
      if(0 != (rc = copy_from_user((unsigned char*)&ldata->speed,(unsigned char*)arg,sizeof(unsigned))))
      {
         rc=-EFAULT;
         goto copy_from_err;
      }
      break;
   /* read power state */
   case DAC6571_GET_PWR:
      data = ldata->mode;
		if(copy_to_user((unsigned char*)arg,&data,sizeof(unsigned short))!=0)
		{
         rc=-EFAULT;
         goto copy_to_err;		  
      }
      break;      
   /* read last output value */
   case DAC6571_GET_VAL:
      data = ldata->value;
		if(copy_to_user((unsigned char*)arg,&data,sizeof(unsigned short))!=0)
		{
         rc=-EFAULT;
         goto copy_to_err;		  
      }
      break;
   /* read current speed setting */
   case DAC6571_GET_SPD:
		if(copy_to_user((unsigned char*)arg,&ldata->speed,sizeof(unsigned))!=0)
		{
         rc=-EFAULT;
         goto copy_to_err;		  
      }
      break;      
      
   }
   return rc;
   
copy_to_err:
#ifdef DAC6571_DEBUG
   printk(KERN_INFO "copy_from_user error\n");
#endif
   return rc;
copy_from_err:
#ifdef DAC6571_DEBUG
   printk(KERN_INFO "copy_from_user error\n");
#endif
   return rc;
}

static const struct file_operations dac6571_fops = {
	.owner = THIS_MODULE,
	.read = dac6571_read,
	.write = dac6571_write,
	.open = dac6571_open,
	.release = dac6571_close,
	.ioctl = dac6571_ioctl,
};

static struct miscdevice dac6571_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "dac6571",
	.fops = &dac6571_fops,
};


/* Standard driver model interfaces */



int dac6571_i2c_probe(struct i2c_client * client, const struct i2c_device_id * id){
	struct dac6571_data *data;
	int rc = 0;

#ifdef DAC6571_DEBUG
	printk(KERN_INFO "%s\n",__FUNCTION__);
#endif

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C))
	{
		printk(KERN_INFO "i2c_check_functionality error\n");
		rc = -ENODEV;
		goto exit;
	}
	
	/* OK. For now, we presume we have a valid client. We now create the
	   client structure, even though we cannot fill it completely yet. */
	if (!(data = kmalloc(sizeof(struct dac6571_data), GFP_KERNEL)))
	{
		rc = -ENOMEM;
		printk(KERN_INFO "kmalloc error\n");
		goto exit;
	}
	memset(data, 0, sizeof(struct dac6571_data));
	
	i2c_set_clientdata(client, data);
	data->mode=DAC6571_MODE_HIZ;
	data->smode=data->mode<<12;
	data->defmode=DAC6571_MODE_HIZ;
	data->value=0x1FF;
	data->speed=100000;

	printk(KERN_INFO "dac6571: slave address 0x%x\n", client->addr);
	
	/* no read is possible, so do a partial transaction to test for an ack */
	rc = i2c_master_send(client, (char*)"\0", 1);
	if (rc != 1){
	   printk(KERN_ERR "dac6571 did not respond\n");
	   goto exit_kfree;
   }
	//Register the device node
	rc = misc_register(&dac6571_device);
	if (rc) {
	   printk(KERN_ERR "dac6571 device register failed\n");
	   goto exit_kfree;
	}
	return 0;
exit_kfree:
	kfree(data);
exit:
   return rc;
}
int dac6571_remove(struct i2c_client *client){
	struct dac6571_data *data;
#ifdef DAC6571_DEBUG
	printk(KERN_INFO "%s\n",__FUNCTION__);
#endif	
	data = i2c_get_clientdata(client);
	dac6571_client = NULL;
	misc_deregister(&dac6571_device);
	kfree(data);
	return 0;
}

/* driver model interfaces that don't relate to enumeration  */
int dac6571_suspend(struct i2c_client *client, pm_message_t mesg){
   int rc;
   char buf[2];
   struct dac6571_data* ldata = dev_get_drvdata(&client->dev);
#ifdef DAC6571_DEBUG
	printk(KERN_INFO "%s\n", __FUNCTION__);
#endif
   ldata->defmode=ldata->mode;
   ldata->mode=DAC6571_MODE_HIZ;
   ldata->smode=DAC6571_MODE_HIZ<<12;
   buf[0]=(ldata->mode)<<4;
   buf[1]=0;
	rc = i2c_master_send(client, buf, 2);
   return rc<0?rc:0;
}

int dac6571_resume(struct i2c_client *client){
   struct dac6571_data* ldata = dev_get_drvdata(&client->dev);
#ifdef DAC6571_DEBUG
	printk(KERN_INFO "%s\n", __FUNCTION__);
#endif
   ldata->mode=ldata->defmode;
   ldata->smode=(ldata->mode)<<12;
   return 0;
}


/* Device detection callback for automatic device creation */
int dac6571_detect(struct i2c_client *client, struct i2c_board_info *info){
	struct i2c_adapter *adapter = client->adapter;
#ifdef DAC6571_DEBUG
	printk(KERN_INFO "%s\n", __FUNCTION__);
#endif
	if (!i2c_check_functionality(adapter, I2C_FUNC_I2C))
		return -ENODEV;

    strlcpy(info->type, DAC6571_DRIVER_NAME, I2C_NAME_SIZE);
    
    return 0;
}


/* End of if using .probe in i2c_driver. */ 
//static unsigned short normal_i2c[] = { I2C_CLIENT_END};

static struct i2c_device_id dac6571_idtable[] = {
	{ "dac6571",0 },
	{ }
};


MODULE_DEVICE_TABLE(i2c, dac6571_idtable);

static struct i2c_driver dac6571_driver = {
	.driver = {
		.name	= DAC6571_DRIVER_NAME,
	},

	.id_table	= dac6571_idtable,
	.probe		= dac6571_i2c_probe,
	.remove		= dac6571_remove,
	/* if device autodetection is needed: */
	.class		= 1<<11,
	.detect		= dac6571_detect,
	//.address_data = &dac6571_addr_data,

	.suspend	= dac6571_suspend,	/* optional */
	.resume		= dac6571_resume,	/* optional */
	.command	= 0,	/* optional, deprecated */
};

static struct i2c_board_info __initdata i2c_dac6571_info[] = 
{
   {
      I2C_BOARD_INFO(DAC6571_DRIVER_NAME, 0x4C),
      I2C_BOARD_INFO(DAC6571_DRIVER_NAME, 0x4D),
   },  
};


int __init dac6571_init(void) {
   int rc;
   int i;
   struct i2c_adapter *adap;
#ifdef DAC6571_DEBUG
	printk(KERN_INFO "%s\n",__FUNCTION__);
#endif
   rc = i2c_add_driver(&dac6571_driver);
   if (rc){
      printk(KERN_ERR "Failed to add dac6571 I2C driver\n");
      return rc;
   }
   for (i=0; i<3; i++){
      adap = i2c_get_adapter(i);
      if (adap == NULL) {
         printk(KERN_ERR "i2c-%i does not exist\n", i);
         continue;
      }
      dac6571_client = i2c_new_probed_device(adap, i2c_dac6571_info, dac6571_normal_i2c);
      if (dac6571_client == NULL){
         printk(KERN_ERR "Failed to probe dac6571 on i2c-%i\n",i);
         continue;
      }
      printk(KERN_INFO "Found a dac6571 on i2c-%i\n",i);
      return 0;
   }
   printk(KERN_ERR "Failed to probe dac6571 on any i2c bus!\n");
   return -ENODEV;
}

static void __exit dac6571_exit(void) {
   i2c_del_driver(&dac6571_driver);
}


module_init(dac6571_init);
module_exit(dac6571_exit);


