/*****************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
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
 * ADC121C021 I2C Battery Monitor Driver
 *
 * The ADC121C021 is a six pin IC that monitors the battery voltage. It is a
 * I2C slave device found at 0x54.
 */

/* ---- Include Files ---------------------------------------------------- */

#include <linux/version.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <asm/io.h>
#include <linux/i2c.h>
#include <linux/timer.h>
#include <linux/delay.h>
#include <asm/gpio.h>
#include <linux/wait.h>
#include <linux/signal.h>
#include <linux/kthread.h>
#include <linux/syscalls.h>
#include <linux/slab.h>

#include <linux/hrtimer.h>
#include <asm/io.h>

#include <linux/broadcom/adc121c021_driver.h>

#if defined(CONFIG_BCM_CMP_BATTERY_MULTI) || defined(CONFIG_BCM_CMP_BATTERY_MULTI_MODULE)
#include <linux/broadcom/cmp_battery_multi.h>
#endif  

/* ---- Public Variables ------------------------------------------------- */
static int mod_debug = 0;
module_param(mod_debug, int, 0644);

/* ---- Private Constants and Types -------------------------------------- */
struct i2c_priv_data 
{
   struct i2c_client *p_i2c_client;
};   

/* Driver upgrade changes ... */
struct i2c_state 
{
   struct i2c_client *p_i2c_client;
};

static const char *reg_names[] = 
{  "output", 
   "status", 
   "config", 
   "under alert", 
   "over alert", 
   "hysteresis", 
   "lowest", 
   "highest",
};

#define GPIO_I2C_RESET_DELAY_MSECS      10
#define GPIO_RESET_PIN                  16
#define MAX_NUMBER_READ_ERRORS          5
#define MILLISECS_BETWEEN_READS         20000

#define USE_ALERT_IRQ                   0


/* ---- Private Variables ------------------------------------------------ */
static        int                    g_num_read_errors    = 0;
static        int                    g_num_driver_errors  = 0;
static        int                    g_found_slave_addr   = 0;
static struct i2c_priv_data          *gp_i2c_driver_priv  = NULL;
static        char                   *gp_buffer           = NULL; 
const  struct I2C_ADC121C021_t       *gp_i2c_adc121c021   = NULL;
static        int                    g_battery_millivolts = 0;
static struct task_struct            *gp_task_struct      = NULL;

static        int                    g_adc121c021_registers[ADC121C021_NUM_REGISTERS];

static struct ADC121C021_REGISTER adc121c021_registers[] = 
{ /* Reg (0-7)                  Length R/W  Default */ 
   { ADC121C021_ADC_REG,        2,     0,   0,        },
   { ADC121C021_STATUS_REG,     1,     1,   0,        },
   { ADC121C021_CONFIG_REG,     1,     1,   0,        },
   { ADC121C021_UNDER_ALERT_REG,2,     1,   0,        },
   { ADC121C021_OVER_ALERT_REG, 2,     1,   0xfff,    },
   { ADC121C021_HSYT_ALERT_REG, 2,     1,   0,        },
   { ADC121C021_LOWEST_REG,     2,     1,   0xfff,    },
   { ADC121C021_HIGHEST_REG,    2,     1,   0,        },
};   
static DECLARE_WAIT_QUEUE_HEAD(g_event_waitqueue);
atomic_t g_atomic_irqs_rxd = ATOMIC_INIT(0);

/* ---- Private Function Prototypes -------------------------------------- */
int  i2c_adc121_driver_read            (int *millivolts);
int  i2c_adc121_driver_write           (int length);
void i2c_adc121_driver_handle_i2c_error(int rc);
void i2c_adc121_read_slave             (void);
int  i2c_adc121_get_battery_voltage    (int *battery_millivolts);
#if USE_ALERT_IRQ 
int  i2c_adc121_driver_setup_gpio      (void);
#endif
/* ---- Public Functions ------------------------------------------------- */

int adc121_get_battery_voltage(void *p_data)
{
   int battery_millivolts;
   i2c_adc121_get_battery_voltage(&battery_millivolts);
   if (mod_debug)
      printk("%s() retreiving battery voltage %d\n", __FUNCTION__, battery_millivolts);
   return battery_millivolts;   
}

/* ---- Functions -------------------------------------------------------- */

/* Battery voltage in millivolts. */
int i2c_adc121_get_battery_voltage(int *p_battery_millivolts)
{
   *p_battery_millivolts = g_battery_millivolts;      
   
   if (g_battery_millivolts > gp_i2c_adc121c021->battery_min_voltage && 
       g_battery_millivolts < gp_i2c_adc121c021->battery_max_voltage)
   {    
      return 0; 
   }
   else
   {
      return -1;
   }   
}   
 
int i2c_adc121c021_find_voltage(void)
{ 
   int rc = 0;
   int adc_millivolts;
   int fudged_millivolts;
   
   rc = i2c_adc121_driver_read(&adc_millivolts);
      
   if (rc == 0)
   {  /* Some adjustment needed to measurement to obtain accurate value. */
      fudged_millivolts = adc_millivolts - 700;
      g_battery_millivolts = (((gp_i2c_adc121c021->resistor_1 + 
                                gp_i2c_adc121c021->resistor_2)*1000 /
                               gp_i2c_adc121c021->resistor_2) * 
                              fudged_millivolts)/1000;      
      
      if (mod_debug)
      {
         printk("%s() raw(mV): %d fudged(mv): %d battery(mv): %d\n", 
                __FUNCTION__, adc_millivolts, fudged_millivolts, g_battery_millivolts);            
      }          
   }    
   else
   {
      printk("%s() error reading slave: %d\n", __FUNCTION__, rc);         
   }
   return rc;   
}   

int i2c_adc121_driver_read(int *p_measured_millivoltage)
{
   int rc = 0;
   int i;
   int length;
   
   if (gp_i2c_driver_priv == NULL ||
       gp_i2c_driver_priv->p_i2c_client == NULL)
   {
      printk("%s() gp_i2c_driver_priv->p_i2c_client == NULL\n", __FUNCTION__);
      return -1;
   }
   
   for (i = 0; i < ADC121C021_NUM_REGISTERS; i++)
   {  /* Have to set the address to read from each register. */
      memset(gp_buffer, 0, gp_i2c_adc121c021->num_bytes_to_read);

      /* Have to do a write to set the register index. */
      length       = ADC121C021_WRITE_REG_LENGTH;
      gp_buffer[0] = i;
      rc = i2c_master_send(gp_i2c_driver_priv->p_i2c_client,
                           gp_buffer,
                           length);
      if (rc < length)
      {
         printk("%s %s() i2c_master_send() failed %d\n", 
                I2C_ADC121C021_DRIVER_NAME, __FUNCTION__, rc);
         g_num_read_errors++;
         return rc;
      }      
      
      if (mod_debug > 1)
      {         
         printk("%s() i2c_master_send() rc: %d\n", __FUNCTION__, rc);
      }   
      
      memset(gp_buffer, 0, gp_i2c_adc121c021->num_bytes_to_read);           
      length = adc121c021_registers[i].num_bytes;
      
      rc = i2c_master_recv(gp_i2c_driver_priv->p_i2c_client, 
                           gp_buffer, 
                           length);

      if (mod_debug > 1)
      {         
         printk("%s() i2c_master_recv() length %d rc: %d,  "
                "reg: %11s i: %d rcvd: 0x%x 0x%x\n", 
                __FUNCTION__, length, rc, reg_names[i], 
                i, gp_buffer[0], gp_buffer[1]);
      }          
                              
      if (rc < adc121c021_registers[i].num_bytes)                           
      {
         printk("%s %s() failed %d\n", I2C_ADC121C021_DRIVER_NAME, __FUNCTION__, rc);
         g_num_read_errors++;
         i2c_adc121_driver_handle_i2c_error(rc);   
         return rc;
      }
      
      g_adc121c021_registers[i] = ((0x0f & gp_buffer[0]) << 8)  + gp_buffer[1];
   }   
      
   *p_measured_millivoltage = g_adc121c021_registers[ADC121C021_ADC_REG]; 
   g_num_read_errors = 0;          
   return 0;
}   

/*
 * Periodically wake up and read the battery voltage. 
 */
static int i2c_adc121_driver_kthread(void *unused)
{
   int rc = 0;
   long unsigned int my_jiffies, timeout_jiffies;
   wait_queue_head_t wait_queue;
   init_waitqueue_head (&wait_queue);

   daemonize("i2c-adc121-driver");
    
   /* Request delivery of SIGKILL */
   allow_signal(SIGKILL);

   timeout_jiffies = msecs_to_jiffies(MILLISECS_BETWEEN_READS);

   for (;;) 
   {
      /* Relinquish the processor until the event occurs */      
      set_current_state(TASK_INTERRUPTIBLE);
      
      if (atomic_read(&g_atomic_irqs_rxd) == 0)
      {  /* Nothing to read, wait a while ... */
         my_jiffies = wait_event_timeout(g_event_waitqueue,       /* the waitqueue to wait on */
                                         atomic_read(&g_atomic_irqs_rxd), /* condition to check */
                                         timeout_jiffies);        /* timeout in jiffies */

         if (my_jiffies < 0)
         {         
            printk("i2c-driver kernel thread ended!\n");
            break;
         }
         else 
         {  /* Timed out, read voltage. */       
            i2c_adc121c021_find_voltage();
         }
      }
      else       
      {  /* Perform a read immediately. */
         rc = i2c_adc121c021_find_voltage();
         
         if (mod_debug)
         {
            printk("%s() i2c_adc121c021_find_voltage() returned: %d", __FUNCTION__, rc);         
         }   

         if (atomic_read(&g_atomic_irqs_rxd) > 0)
         {   
            atomic_dec(&g_atomic_irqs_rxd);            
         }   
      }
       
    }

    return rc;    
}

int i2c_adc121_driver_write(int length)
{
   int rc;
   
   rc = i2c_master_send(gp_i2c_driver_priv->p_i2c_client,
                        gp_buffer,
                        length);
   return rc;
}

static irqreturn_t i2c_adc121_driver_isr(int irq, void *dev_id)
{
   /* This is called if USE_ALERT_IRQ is set to 1 and either a under or over
    * alert occurred. 
    * An under alert is raised if the battery voltage is detected to be less
    * than the minimum HW_BATTERY_MIN_VOLTAGE.
    * An over alert is raised if the battery voltage is detected to be greater
    * than the maximum HW_BATTERY_MAX_VOLTAGE.
    */              
   atomic_inc(&g_atomic_irqs_rxd);
   
   if (atomic_read(&g_atomic_irqs_rxd) == 1)
   {
      wake_up(&g_event_waitqueue);
   }   
   return IRQ_HANDLED;
}

void i2c_adc121_driver_handle_i2c_error(int rc)
{
   if (mod_debug > 0)
   {
      printk("%s I2C error, rc %d # read errors %d # known driver errors %d\n",
             I2C_ADC121C021_DRIVER_NAME, rc, 
             g_num_read_errors,
             g_num_driver_errors);
   }
   
   if (rc != 0)
   {  /* Was called by i2c_adc121_driver_read(). */
      if (g_num_read_errors < MAX_NUMBER_READ_ERRORS)
      {
         printk("%s I2C read error %d, error %d\n", 
                I2C_ADC121C021_DRIVER_NAME, g_num_read_errors, rc);
      }
      else if (g_num_read_errors == MAX_NUMBER_READ_ERRORS)
      {
         printk("%s maximum # I2C read errors reached %d, error %d\n", 
                I2C_ADC121C021_DRIVER_NAME, g_num_read_errors, rc);
      }
      else
      {
         return;
      }
   }
   else
   {
      g_num_driver_errors++;
   }   
         
   printk("%s I2C bus has problems but cannot reset slave at 0x%x\n", 
          I2C_ADC121C021_DRIVER_NAME, g_found_slave_addr);
      
   if (rc == -EREMOTEIO)
   {  /* Indicates a problem with the bus. Reset the I2C master controller. */   
      printk("%s detected remote IO problem but cannot reset I2C bus master\n", 
             I2C_ADC121C021_DRIVER_NAME);            
   }
}

/*
 * Setup the interrupt handling if it is going to be used.
 */
int i2c_adc121_driver_setup_gpio(void)
{
   int rc;
   int ret = 0;
   
   if ((rc = gpio_request(gp_i2c_adc121c021->gpio_irq_pin, "adc121c021 alert")) != 0)
   {
      printk("%s() gpio_request(%d) failed, rc = %d\n", __FUNCTION__,
             gp_i2c_adc121c021->gpio_irq_pin, rc);      
      ret = rc;
   }     
   
   if ((rc = request_irq(gpio_to_irq(gp_i2c_adc121c021->gpio_irq_pin), 
                         i2c_adc121_driver_isr, 
                         (IRQF_TRIGGER_FALLING),
                         "GPIO adc121c021 irq", 
                         gp_i2c_driver_priv)) < 0)
   {
      printk("%s() request_irq(%d) failed, rc = %d\n", __FUNCTION__,
             gp_i2c_adc121c021->gpio_irq_pin, rc);            
      ret = rc;
   }                             

   return ret;
}

#ifdef CONFIG_PM
static int i2c_adc121_suspend_driver(struct i2c_client *p_client, pm_message_t mesg)
{
   /* Internal thread is stopped. */
   return 0;
}

static int i2c_adc121_resume_driver(struct i2c_client *p_client)
{
   /* Internal thread is started. */
   return 0;
}
#endif

static int i2c_adc121_driver_probe(struct i2c_client *p_i2c_client,
                                   const struct i2c_device_id *id)
{
   int rc = 0;
   struct i2c_state *p_state;
   struct device *dev = &p_i2c_client->dev;
   int battery_data;
#if defined(CONFIG_BCM_CMP_BATTERY_MULTI) || defined(CONFIG_BCM_CMP_BATTERY_MULTI_MODULE)
   struct battery_monitor *p_monitor;   
#endif   
   
   if (p_i2c_client == NULL)
   {
      printk(KERN_ERR "%s i2c_adc121_driver_probe() p_i2c_client == NULL\n", 
             I2C_ADC121C021_DRIVER_NAME);      
      return -1;
   }
   
   if (p_i2c_client->dev.platform_data == NULL)
   {
      printk(KERN_ERR "%s i2c_adc121_driver_probe() "
             "p_i2c_client->dev.platform_data == NULL\n",
             I2C_ADC121C021_DRIVER_NAME);      
      return -1;
   }

   if (g_found_slave_addr > 0)
   {  /* Needed when more than one I2C slave had the same address.  */
      printk(KERN_ERR "%s i2c_adc121_driver_probe() i2c slave already "
             "found at 0x%x\n",
             I2C_ADC121C021_DRIVER_NAME, g_found_slave_addr);      
      return -1;
   }
   
   /* get platform data */   
   gp_i2c_adc121c021 = 
       (struct I2C_ADC121C021_t *)p_i2c_client->dev.platform_data;   
   
   if (gp_i2c_adc121c021 == NULL)
   {  /* Cannot access platform data. */   
      printk("%s:%s Cannot access platform data for I2C slave address %d\n", 
             I2C_ADC121C021_DRIVER_NAME, __FUNCTION__, p_i2c_client->addr);
      return -1;
   }

   /* todo: clean up memory allocation failure handlings */
   p_state = kzalloc(sizeof(struct i2c_state), GFP_KERNEL);
   if (p_state == NULL) 
   {
      dev_err(dev, "failed to create our state\n");
      return -ENOMEM;
   }
   p_state->p_i2c_client = p_i2c_client;


   gp_i2c_driver_priv = kzalloc(sizeof(struct i2c_priv_data), GFP_KERNEL);
   if (gp_i2c_driver_priv == NULL)
   {
      dev_err(dev, "failed to create gp_i2c_driver_priv\n");
      return -ENOMEM;
   }       
   gp_i2c_driver_priv->p_i2c_client = p_i2c_client;
   
   i2c_set_clientdata(p_i2c_client, p_state);
 
   /* Rest of the initialisation goes here. */
   
   /* Create some space to store the I2C bytes read from the slave. */
   gp_buffer = kzalloc(gp_i2c_adc121c021->num_bytes_to_read + 10, GFP_KERNEL);
   if (!gp_buffer)
   {
      printk("i2c_adc121_driver_probe() kzalloc() returned NULL\n");
      return -ENOMEM;
   }       

   rc = i2c_adc121_driver_read(&battery_data);
   
   if (rc < 0)
   {  /* Do not free anything otherwise I2C bus goes kaput and system will
       * grind to a halt!
       */  
      printk("%s() leaving, I2C slave not detected\n", __FUNCTION__);
      return -ENODEV;   
   }   
      
#if defined(CONFIG_BCM_CMP_BATTERY_MULTI) || defined(CONFIG_BCM_CMP_BATTERY_MULTI_MODULE)
   p_monitor = kzalloc(sizeof(struct battery_monitor), GFP_KERNEL); 
   
	if (p_monitor == NULL)
   {
		return -ENOMEM;
   }

   p_monitor->name           = I2C_ADC121C021_DRIVER_NAME;
   p_monitor->get_voltage_fn = adc121_get_battery_voltage;     
   p_monitor->gpio_ac_power = gp_i2c_adc121c021->gpio_ac_power;
   p_monitor->ac_power_on_level = gp_i2c_adc121c021->ac_power_on_level;
   p_monitor->gpio_charger = gp_i2c_adc121c021->gpio_charger;
   rc = register_battery_monitor(p_monitor, p_i2c_client);
   if (rc < 0) {
       kfree(p_monitor);
       kfree(gp_buffer);             
       return rc;
   }
#endif   
      
   /* 
    *  Setup the gpio for handling interrupt requests and the reset pin if used
    *  based on platform_data. 
    */
#if USE_ALERT_IRQ    
   if (i2c_adc121_driver_setup_gpio() != 0)
   {
#if defined(CONFIG_BCM_CMP_BATTERY_MULTI) || defined(CONFIG_BCM_CMP_BATTERY_MULTI_MODULE)
      kfree(p_monitor);
#endif   

      kfree(gp_buffer);             
      return -1;       
   }
#endif
   
   /* This thread wakes periodically to read the battery voltage. */
   gp_task_struct = kthread_run(i2c_adc121_driver_kthread,  /* pointer to function   */
                                NULL,                       /* data pointer argument */
                                "adc121c021 thread");       /* thread name string    */
   
   if (gp_task_struct == NULL)
   {
      printk("%s i2c_adc121_driver_probe() kernel thread not created\n", 
             I2C_ADC121C021_DRIVER_NAME);
#if defined(CONFIG_BCM_CMP_BATTERY_MULTI) || defined(CONFIG_BCM_CMP_BATTERY_MULTI_MODULE)
      kfree(p_monitor);
#endif   
      kfree(gp_buffer);             
#if USE_ALERT_IRQ    
      free_irq(gp_i2c_adc121c021->gpio_irq_pin, gp_i2c_driver_priv);
#endif
      return -1;       
   }
             
   g_found_slave_addr = p_i2c_client->addr;
   
   printk("%s() found i2c slave at 0x%x\n", __FUNCTION__, p_i2c_client->addr);      
   
   if (mod_debug)
   {
      printk("%s() gp_i2c_adc121c021->i2c_slave_address : 0x%x\n", 
             __FUNCTION__, gp_i2c_adc121c021->i2c_slave_address);
      printk("%s() gp_i2c_adc121c021->gpio_irq_pin      : %d\n", 
             __FUNCTION__, gp_i2c_adc121c021->gpio_irq_pin);
      printk("%s() gp_i2c_adc121c021->num_bytes_to_read : %d\n",
             __FUNCTION__, gp_i2c_adc121c021->num_bytes_to_read);
   }   
   
   /*
    * The adc121c021 is being configured to run in manual conversion mode. 
    * Register 0 contains the output of the ADC of the voltage on pin 3, Vin.
    * This is the simplest mode and no over voltage or under voltage alerts
    * will be generated and detected on the GPIO.
    */
   rc = i2c_adc121c021_find_voltage();
   return rc;
}
 
static int __devexit i2c_adc121_driver_remove(struct i2c_client *client)
{
   struct i2c_state *state = i2c_get_clientdata(client);
   kfree(state);

   if (gp_task_struct != NULL)
   {
      kthread_stop(gp_task_struct);
   }
   
#if USE_ALERT_IRQ
   free_irq(gp_i2c_adc121c021->gpio_irq_pin, gp_i2c_driver_priv);
#endif      
   
   /* Free all the memory that was allocated. */   
   if (gp_i2c_driver_priv->p_i2c_client != NULL)
   {
      kfree(gp_i2c_driver_priv->p_i2c_client);
   }
   
   if (gp_i2c_driver_priv != NULL)
   {
      kfree(gp_i2c_driver_priv);
   }

   if (gp_buffer != NULL)
   {
      kfree(gp_buffer);
   }
   
   return 0;
}

/* End of if using .probe in i2c_driver. */ 

static struct i2c_device_id adc121c021_i2c_idtable[] = {
   { I2C_ADC121C021_DRIVER_NAME, 0 },
   { }
};

static struct i2c_driver adc121c021_i2c_driver = {
   .driver = {
      .name  = I2C_ADC121C021_DRIVER_NAME,
   },   
   .id_table       = adc121c021_i2c_idtable,
   .class          = I2C_CLASS_HWMON,
   .probe          = i2c_adc121_driver_probe,
   .remove         = __devexit_p(i2c_adc121_driver_remove),
#ifdef CONFIG_PM  
   .suspend        = i2c_adc121_suspend_driver,
   .resume         = i2c_adc121_resume_driver,
#endif      
};

int __init i2c_adc121_driver_init(void)
{
   int rc;
   
   rc = i2c_add_driver(&adc121c021_i2c_driver);
   if (rc != 0) 
   {
      printk("%s i2c_adc121_driver_init(): i2c_add_driver() failed, errno is %d\n", 
             I2C_ADC121C021_DRIVER_NAME, rc);
      return rc;
   }

   return rc;
}

static void __exit i2c_adc121_driver_exit(void)
{
	i2c_del_driver(&adc121c021_i2c_driver);
}

MODULE_DESCRIPTION("I2C adc121c021 driver");
MODULE_AUTHOR("Broadcom");
MODULE_LICENSE("GPL");

module_init(i2c_adc121_driver_init);
module_exit(i2c_adc121_driver_exit);
