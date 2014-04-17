/*****************************************************************************
* Copyright 2011 Broadcom Corporation.  All rights reserved.
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
 * A driver to monitor the MAX17040 or ADC121C021, the AC power and charge state
 * of the battery. 
 *
 * Driver works in conjunction with frameworks defined in linux/power_supply.h
 * and linux/pm.h.
 *
 * 2 power_supply devices - power and battery are registered/unregistered with
 * power supply(PSY) framework and report properties to PSY. The list of
 * properties supported by this driver is defined in battery_data_props and
 * power_data_props arrays.
 *
 * Another PSY - USB is added to comply with Android's expectations. For now
 * this is a mock PSY, that provides one propery - "online" that is always set
 * to false. In the future boards (when the USB supply/charger is available)
 * this will be changed.
 * 
 * Driver works in cooperation with battery monitor that registers with the
 * driver via exported register_battery_monitor function. Driver relies on
 * battery monitor for methods to determine volatage and charge and for supply
 * of some essential data such as GPIO values. Only one battery monitor may be
 * registered at a time. Battery driver will be nonfunctional if there is no
 * battery monitor registered.
 *
 * Platform information is passed into the driver via the platform_data pointer.
 */
 
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/power_supply.h>
#include <linux/pm.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/gpio.h>
#include <linux/wakelock.h>
#include <linux/slab.h>
#include <linux/workqueue.h>
#include <linux/i2c.h>

#include <linux/broadcom/cmp_battery_multi.h>

EXPORT_SYMBOL(register_battery_monitor);

/* ---- Public Functions ------------------------------------------------- */
/* after power_supply devices are registered, these functions are used to get
 * power and battery properies */
static int battery_get_property(struct power_supply *psy,
                                enum power_supply_property psp,
                                union power_supply_propval *val);
static int power_get_property(struct power_supply *psy,
                              enum power_supply_property psp,
                              union power_supply_propval *val);

static int usbpower_get_property(struct power_supply *psy,
                              enum power_supply_property psp,
                              union power_supply_propval *val);
                              
static void battery_external_power_changed(struct power_supply *psy);

/* ---- Private Constants and Types -------------------------------------- */

/* Main structure used in this driver. */
struct multi_data
{
   /* Battery PSY */
   struct power_supply battery_psy;
   
   /* AC power PSY */
   struct power_supply ac_power_psy;

   /* task to handle AC power plug / remove. */
   struct work_struct  t_ac_power_work;
   
   /* task to periodically send battery uEvent */
   struct delayed_work t_battery_dwork;

   /* battery monitor instance and data that is registered with the driver */
   struct battery_monitor *pt_battery_monitor;
   void *pt_battery_monitor_data;

   /* wake lock. When taken keeps device awake */
   struct wake_lock wakelock;
   
   /* Reference to platform_data passed in by probe. */
   struct cbm_platform_data    *pt_platform_data;
};	

#define DEBOUNCE_TIME_USECS        128000
#define MULTI_WAIT_PERIOD          (30*HZ)
#define MULTI_NO_WAIT              0

#define INT_SETUP_SLEEP_MSECS      50

/* ---- Private Variables ------------------------------------------------ */

/* Debug macro */
#define PRINT_DEBUG(format, args...) \
    do { if (mod_debug) printk(KERN_WARNING "[cmp_battery_multi]: " format, ## args); } while (0)

/* define mod_debug module parameter to enable/disable debug messages */
static int mod_debug = 0x0;
module_param(mod_debug, int, 0644);

static const __devinitconst char gBanner[] =
    KERN_INFO "Multi Battery Driver: 1.02\n";

/* workqueue and work task for debouncing power input signal */
static struct workqueue_struct *isr_wq;

static const char *wake_lock_name = "multi_battery_driver";

static char *power_supplied_to[] = 
{
   "battery",
};

/* array of properties supported by battery PSY */
static enum power_supply_property battery_data_props[] = 
{
   POWER_SUPPLY_PROP_STATUS,             /* charging, discharging ... */
   POWER_SUPPLY_PROP_HEALTH,             /* good, overheat, dead ...  */
   POWER_SUPPLY_PROP_PRESENT,            /*                           */
   POWER_SUPPLY_PROP_TECHNOLOGY,         /* NiMH|LION|...             */ 
   POWER_SUPPLY_PROP_VOLTAGE_MAX,        /* battery_max               */
   POWER_SUPPLY_PROP_VOLTAGE_MIN,        /* battery_min               */
   POWER_SUPPLY_PROP_VOLTAGE_MAX_DESIGN, /*                           */
   POWER_SUPPLY_PROP_VOLTAGE_MIN_DESIGN, /*                          */
   POWER_SUPPLY_PROP_VOLTAGE_NOW,        /* battery voltage(mV)      */
   POWER_SUPPLY_PROP_CAPACITY,           /* Capacity in percent      */
   POWER_SUPPLY_PROP_TEMP,               /* Degrees celsius          */
};

/* array of properties supported by AC power PSY */
static enum power_supply_property power_data_props[] = {
   POWER_SUPPLY_PROP_ONLINE,             /* AC plugged in or not      */ 
};

/* array of properties supported by USB power PSY */
static enum power_supply_property usbpower_data_props[] = {
   POWER_SUPPLY_PROP_ONLINE,             /* USB power plugged in or not      */ 
};

/* initialize PSY in multi_data struct */
static struct multi_data g_multi_data = 
{
   .battery_psy = 
   {
      .name                   = "battery",
      .type                   = POWER_SUPPLY_TYPE_BATTERY,
      .properties             = battery_data_props,
      .num_properties         = ARRAY_SIZE(battery_data_props),
      .get_property           = battery_get_property,
      .external_power_changed = battery_external_power_changed,
      .use_for_apm            = 1,
   },

   .ac_power_psy = 
   {
      .name            = "power",
      .type            = POWER_SUPPLY_TYPE_MAINS,
      .supplied_to     = power_supplied_to,
      .num_supplicants = ARRAY_SIZE(power_supplied_to),
      .properties      = power_data_props,
      .num_properties  = ARRAY_SIZE(power_data_props),
      .get_property    = power_get_property,
   },
};

/* initialize usb power PSY  */
static struct power_supply usb_power_psy = 
{
   .name            = "usbpower",
   .type            = POWER_SUPPLY_TYPE_USB,
   .supplied_to     = power_supplied_to,
   .num_supplicants = ARRAY_SIZE(power_supplied_to),
   .properties      = usbpower_data_props,
   .num_properties  = ARRAY_SIZE(power_data_props),
   .get_property    = usbpower_get_property,
};

static int g_gpio_power_control = -1;

/* flag inidicating if the driver is ready for battery monitor drivers 
   to register */ 
static bool               g_is_driver_ok = false;
            
/* ---- Private Function Prototypes -------------------------------------- */
static void cmp_battery_multi_power_off(void);
static bool is_ac_connected            (void);
static void multi_battery_work         (struct work_struct *p_work);
static int  setup_gpios                (struct battery_monitor *pt_battery_monitor);
static void close_gpios                (struct battery_monitor *pt_battery_monitor);
static void ac_power_isr_handler_work  (struct work_struct *p_work);
static int get_voltage(void);
static int get_charge(void);

/* ---- Public Functions ------------------------------------------------- */

/* this is exported function that is called by battery monitor to register with
   the battery driver */
int register_battery_monitor(struct battery_monitor *p_monitor, void *p_data)
{
   int rc;
   
   if (g_is_driver_ok == false)
   {  
      printk(KERN_ERR "%s() battery monitor %s cannot register. "
             "Battery driver is not ready.\n", __FUNCTION__, p_monitor->name);
      return -ENODATA;       
   }
   
   /* Check the fundamental assumption that the AC power GPIO must be supplied
      by the battery monitor. Note that after this check we do not need to
      check AC power gpio presence again */
   if (p_monitor->gpio_ac_power < 0)
   {
      printk(KERN_ERR "%s() battery monitor %s did not supply "
             "valid AC power GPIO. The value is %d\n",
             __FUNCTION__, p_monitor->name, p_monitor->gpio_ac_power);
      return -ENODATA;
   }

   printk(KERN_INFO "%s() battery monitor %s successfully registered "
          "with multi battery driver\n", __FUNCTION__, p_monitor->name); 

   /* store monitor and monitor data in multi struct */
   g_multi_data.pt_battery_monitor = p_monitor;
   g_multi_data.pt_battery_monitor_data = p_data;

   /* initialize ISR work */
   INIT_WORK(&g_multi_data.t_ac_power_work, ac_power_isr_handler_work);

   /* setup GPIOs. Note that setup_gpios also sets up AC power interrupt so this
      should be done after t_ac_power_work is initialized */
   if ((rc = setup_gpios(p_monitor)) != 0)
   {
      printk(KERN_ERR "%s() setup_gpios() returned %d\n", __FUNCTION__, rc);
      return rc;
   }

   /* take wakelock if the AC power is connected */
   if (is_ac_connected())
   {
      wake_lock(&g_multi_data.wakelock);
      printk(KERN_INFO "%s() taking wake lock\n", __FUNCTION__);
   }   
   
   /*  schedule delayed work for battery PSY */
   schedule_delayed_work(&g_multi_data.t_battery_dwork, MULTI_NO_WAIT);
   return 0;
}

/* ---- Private Functions ------------------------------------------------ */

static bool is_ac_connected(void)
{
   /* Note that this function may be called before battery monitor is
      registered */
   if (g_multi_data.pt_battery_monitor == NULL)
      return false;

   int ac_gpio_level = 
      gpio_get_value(g_multi_data.pt_battery_monitor->gpio_ac_power);

   /* AC power GPIO level differs between boards. */

   /* if GPIO high indicates AC power is connected. */
   if (g_multi_data.pt_battery_monitor->ac_power_on_level > 0)
      return (ac_gpio_level > 0) ? true : false;
   else
      /* GPIO low indicates AC power is connected. */   
      return (ac_gpio_level > 0) ? false : true;
}

/* get battery charging status */
/* Note that the P11 tablet uses MAX 17040 monitor that does not have charge
   indicator pin */
static int get_battery_status(void)
{
   /* Note that this function may be called before battery monitor is
      registered */
   if (g_multi_data.pt_battery_monitor == NULL)
      return POWER_SUPPLY_STATUS_UNKNOWN;
   
   /* if AC power is disconnected */
   if (!is_ac_connected())
      return POWER_SUPPLY_STATUS_DISCHARGING;

   /* else if there is charging pin that does not indicate charging */
   /* note that we are assuming that charging GPIO set to high indicates full
      charge (no charging) */
   else if (g_multi_data.pt_battery_monitor->gpio_charger != -1 &&
            gpio_get_value(g_multi_data.pt_battery_monitor->gpio_charger))
      return POWER_SUPPLY_STATUS_NOT_CHARGING;
   else
      return POWER_SUPPLY_STATUS_CHARGING;
}

static int get_voltage(void)
{
   int voltage;
   struct battery_monitor *pt_battery_monitor = g_multi_data.pt_battery_monitor;

   /* if monitor provides function to access voltage */   
   if (pt_battery_monitor != NULL && pt_battery_monitor->get_voltage_fn != NULL)
   {
      /* get value from the monitor */
      voltage = pt_battery_monitor->get_voltage_fn(
         g_multi_data.pt_battery_monitor_data)*1000;
      
      /* do not want to return 0, may cause tablet to restart */
      if (voltage == 0)
      {  
         voltage = g_multi_data.pt_platform_data->battery_min_voltage*1000;
      }
      
      PRINT_DEBUG("%s() battery_voltage: %d\n", __FUNCTION__, voltage);
   }
   else
   {
      PRINT_DEBUG("%s() battery monitor get_battery_voltage_fn() "
             "not configured\n", __FUNCTION__);
      voltage = g_multi_data.pt_platform_data->battery_max_voltage*1000;
   }
   return voltage;
}

static int get_charge()
{
   int charge_percent;   
   struct battery_monitor *pt_battery_monitor = g_multi_data.pt_battery_monitor;
      
   /* if monitor provides function to access charge */   
   if (pt_battery_monitor != NULL && pt_battery_monitor->get_charge_fn != NULL)
   {      
      charge_percent = 
         pt_battery_monitor->get_charge_fn(g_multi_data.pt_battery_monitor_data);
      PRINT_DEBUG("%s() battery_charge: %d\n", __FUNCTION__, charge_percent);
   }
   else
   {
      PRINT_DEBUG("%s() battery monitor get_charge_fn() not configured\n",
             __FUNCTION__);

      /* calculate charge */
      charge_percent = 
         (get_voltage() - g_multi_data.pt_platform_data->battery_min_voltage)*100/
         (g_multi_data.pt_platform_data->battery_max_voltage - 
          g_multi_data.pt_platform_data->battery_min_voltage);
   }

   /* 
    * It is important that the charge level of the battery returned by 
    * POWER_SUPPLY_PROP_CAPACITY never be accidentally set to zero. This might
    * cause a reboot by the parent OS. It is also bad if the value is greater 
    * than 100.
    */
   if (charge_percent <= 0 || charge_percent > 100)
   {  
      /* Need to keep this value in range. */
      charge_percent = 100;
      PRINT_DEBUG("%s() charge out of range %d, setting to 100\n",
                __FUNCTION__, charge_percent);
   }         
   return charge_percent;
}

static int battery_get_property(struct power_supply *pt_power_supply,
                                enum power_supply_property psp,
                                union power_supply_propval *val)
{
   int ret = 0;
   PRINT_DEBUG("%s() requested property: %d\n", __FUNCTION__, psp);
   
   switch (psp) 
   {
      case POWER_SUPPLY_PROP_STATUS:
         val->intval = get_battery_status();
         break;
      case POWER_SUPPLY_PROP_HEALTH:
         val->intval = POWER_SUPPLY_HEALTH_GOOD;
         break;
      case POWER_SUPPLY_PROP_PRESENT:
          /* assume that the battery is always present */
          /* this assumption is valid for tablet */
          val->intval = 1;
         break;
      case POWER_SUPPLY_PROP_TECHNOLOGY:
         val->intval = g_multi_data.pt_platform_data->battery_technology;
         break;
      case POWER_SUPPLY_PROP_VOLTAGE_MAX:
         val->intval = g_multi_data.pt_platform_data->battery_max_voltage*1000;
         break;
      case POWER_SUPPLY_PROP_VOLTAGE_MIN:
         val->intval = g_multi_data.pt_platform_data->battery_min_voltage*1000;
         break;
      case POWER_SUPPLY_PROP_VOLTAGE_MAX_DESIGN:
         val->intval = g_multi_data.pt_platform_data->battery_max_voltage*1000;
         break;
      case POWER_SUPPLY_PROP_VOLTAGE_MIN_DESIGN:
         val->intval = g_multi_data.pt_platform_data->battery_min_voltage*1000;
         break;
      case POWER_SUPPLY_PROP_VOLTAGE_NOW:
         val->intval = get_voltage();
         break;
      case POWER_SUPPLY_PROP_CAPACITY:
         val->intval = get_charge();
         break;
      case POWER_SUPPLY_PROP_TEMP:
         /* current battery monitors do not supply this data */
         val->intval = 0;
         break;
      default:
         ret = -EINVAL;
         break;
   }

   PRINT_DEBUG("%s() property: %d value: %d\n", __FUNCTION__, psp, val->intval);
   return ret;
}

static int power_get_property(struct power_supply *pt_power_supply,
                              enum power_supply_property psp,
                              union power_supply_propval *val)
{
   switch (psp) 
   {
      case POWER_SUPPLY_PROP_ONLINE:
         val->intval = is_ac_connected();
         break;
      
      default:
         return -EINVAL;
   }
   
   PRINT_DEBUG("%s() property: %d value: %d\n", 
          __FUNCTION__, psp, val->intval);
   return 0;
}

static int usbpower_get_property(struct power_supply *pt_power_supply,
                                 enum power_supply_property psp,
                                 union power_supply_propval *val)
{
   switch (psp)
   {
      case POWER_SUPPLY_PROP_ONLINE:         
         val->intval = 0;
         break;
      
      default:
         return -EINVAL;
   }
   
   PRINT_DEBUG("%s() property: %d value: %d\n",
          __FUNCTION__, psp, val->intval);
   return 0;
}

/* Called when the device is plugged into or removed from an AC supply. */
static irqreturn_t ac_power_gpio_isr(int irq, void *data)
{
   struct multi_data *pt_multi_data = (struct multi_data *)data;
   
   if (pt_multi_data == NULL)
   {
      printk(KERN_ERR "%s() pt_multi_data == NULL!\n", __FUNCTION__);
      return IRQ_HANDLED;      
   }
   
   queue_work(isr_wq, &pt_multi_data->t_ac_power_work);
   return IRQ_HANDLED;
}

/* battery PSY delayed work. */
static void multi_battery_work(struct work_struct *p_work)
{
   /* send uEvent indicating that battery PSY has been changed. Note that we
      are not attempting (at the driver level) to check if any of the battery
      properties changed, leaving it to the uEvent handler to do all the analysis */
   power_supply_changed(&g_multi_data.battery_psy); 

   /* reschedule delayed work to keep periodic uEvent sending */
   schedule_delayed_work(&g_multi_data.t_battery_dwork, MULTI_WAIT_PERIOD);
}

/* AC power change ISR work */
static void ac_power_isr_handler_work(struct work_struct *p_work)
{
   /* if AC power is connected */
   if (is_ac_connected())
   {  
      wake_lock(&g_multi_data.wakelock);
      PRINT_DEBUG("%s() taking wake lock\n", __FUNCTION__);
   }
   else
   {  
      wake_unlock(&g_multi_data.wakelock);
      PRINT_DEBUG("%s() releasing wake lock\n", __FUNCTION__);
   }

   /* send uEvent indicating that power PSY has been changed */
   power_supply_changed(&g_multi_data.ac_power_psy);
}

static void battery_external_power_changed(struct power_supply *t_power_supply)
{   
   PRINT_DEBUG("%s() called\n", __FUNCTION__);
}

static void cmp_battery_multi_power_off(void)
{
   printk(KERN_INFO "%s() power off called\n",  __FUNCTION__);
   gpio_set_value(g_gpio_power_control, 0);
}

static int setup_gpios(struct battery_monitor *pt_battery_monitor)
{
   int rc;
   
   printk(KERN_INFO "%s() power on/off gpio: %d ac power gpio: %d "
	  "charger gpio: %d\n", 
	  __FUNCTION__,
	  g_gpio_power_control,
	  pt_battery_monitor->gpio_ac_power,
	  pt_battery_monitor->gpio_charger);

   if (g_gpio_power_control >= 0)
   {   
      rc = gpio_request_one(g_gpio_power_control,
                            GPIOF_OUT_INIT_HIGH, "switch off");
      if (rc)
      {
         close_gpios(pt_battery_monitor);
         return rc;
      }
   }

   if (pt_battery_monitor->gpio_charger >= 0)
   {
      rc = gpio_request_one(pt_battery_monitor->gpio_charger, 
                            GPIOF_IN, "charger");
      if (rc)
      {
         close_gpios(pt_battery_monitor);
         return rc;
      }   
   }

   /* handle AC power gpio. We are assuming that its presence was already 
      checked */
   rc = gpio_request_one(pt_battery_monitor->gpio_ac_power,
                         GPIOF_IN, "ac power");
   if (rc)
   {
      close_gpios(pt_battery_monitor);
      return rc;
   }   

   /*
    * power on/off gpio signal bounces up and down before settling in either
    * case, so we need to debounce it.
    */
   rc = gpio_set_debounce(pt_battery_monitor->gpio_ac_power, 
                          DEBOUNCE_TIME_USECS);
   if (rc < 0) {
      pr_err("%s: gpio_ac_power set debounce failed. gpio: %d, return: %d\n",
             __FUNCTION__, pt_battery_monitor->gpio_ac_power, rc);
      close_gpios(pt_battery_monitor);
      return rc;
   }

   /*
    * Setup the interrupt to detect if external power is plugged in or not.
    * IRQF_TRIGGER_FALLING  - external power is connected
    * IRQF_TRIGGER_RISING   - external power is disconnected
    */
   rc = request_irq(gpio_to_irq(pt_battery_monitor->gpio_ac_power),
                     ac_power_gpio_isr,
                     IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
                     "ac detect", &g_multi_data);
   if (rc)
   {
      printk(KERN_ERR "%s():  request_irq(gpio_to_irq(%d)) returned %d\n",
             __FUNCTION__, pt_battery_monitor->gpio_ac_power, rc);
      close_gpios(pt_battery_monitor);
      return rc;
   }

   return 0;
}

static void close_gpios(struct battery_monitor *pt_battery_monitor)
{
   if (pt_battery_monitor->gpio_ac_power >= 0) {
      free_irq(gpio_to_irq(pt_battery_monitor->gpio_ac_power), &g_multi_data);
      gpio_free(pt_battery_monitor->gpio_ac_power);
   }
      
   if (g_gpio_power_control >= 0)
      gpio_free(g_gpio_power_control);
      
   if (pt_battery_monitor->gpio_charger >= 0)   
      gpio_free(pt_battery_monitor->gpio_charger);
}

#ifdef CONFIG_PM
static int battery_suspend(struct platform_device *p_dev, pm_message_t state)
{
   /* flush pending ISR work */
   flush_scheduled_work();  
   flush_workqueue(isr_wq);

   /* flush delayed work. */
   cancel_delayed_work_sync(&g_multi_data.t_battery_dwork);
   return 0;
}

static int battery_resume(struct platform_device *p_dev)
{
   /* take wakelock if AC is connected. This is needed as we do not know what
      was happening when we were suspended */
   if (is_ac_connected())
   {
      wake_lock(&g_multi_data.wakelock);
      printk(KERN_INFO "%s() taking wake lock\n", __FUNCTION__);
   }   

   /* How to do in a SMP centric safe manner? */
   schedule_delayed_work(&g_multi_data.t_battery_dwork, MULTI_WAIT_PERIOD);
   return 0;
}
#endif

static int __devinit battery_probe(struct platform_device *p_dev)
{
   int ret;
   /* CBM (C)mp (B)attery (M)ulti. */
   struct cbm_platform_data *pt_cbm_platform_data;

   printk(gBanner);

   if (p_dev->dev.platform_data == NULL)
   {  /* Need this information. */
      printk(KERN_ERR "%s() error p_dev->dev.platform_data == NULL\n",
	     __FUNCTION__);
      return -ENODATA;
   }
   
   pt_cbm_platform_data = (struct cbm_platform_data *)p_dev->dev.platform_data;

   printk(KERN_INFO "%s() battery max voltage: %d battery min voltage: %d\n", 
	  __FUNCTION__, 
	  pt_cbm_platform_data->battery_max_voltage,
	  pt_cbm_platform_data->battery_min_voltage);

   /* Needed to turn off the device. */   
   g_gpio_power_control = pt_cbm_platform_data->gpio_power_control;
   
   /* store platform data */
   g_multi_data.pt_platform_data = pt_cbm_platform_data;

   /* Tell the system how to turn off the power. */
   /* pm_power_off is a global function pointer. It is declared as external in
      linux/pm.h. Somehow this is called when Tablet is powered off */
   pm_power_off = cmp_battery_multi_power_off;

   isr_wq = create_workqueue("multi_isr_wq");
   if (isr_wq == NULL)
   {
      printk(KERN_ERR "%s() Can not create ISR workqueue\n", __FUNCTION__);
      ret = -1;
      goto err_isr_wq;
   }

   /* Use a wake lock to keep the tablet awake. */
   wake_lock_init(&g_multi_data.wakelock, WAKE_LOCK_SUSPEND, wake_lock_name);

   /* Prepare to start the delayed work. */
   INIT_DELAYED_WORK_DEFERRABLE(&g_multi_data.t_battery_dwork, 
				multi_battery_work);

   /* register battery PSY with power supply framework */
   ret = power_supply_register(&p_dev->dev, &g_multi_data.battery_psy);
   if (ret)
      goto err_psy_reg_battery;

   /* register AC power PSY with power supply framework */
   ret = power_supply_register(&p_dev->dev, &g_multi_data.ac_power_psy);
   if (ret)
      goto err_psy_reg_power;

   /* register USB power PSY with power supply framework */
   ret = power_supply_register(&p_dev->dev, &usb_power_psy);
   if (ret)
      goto err_psy_reg_usbpower;

   /* return success */
   g_is_driver_ok = true;
   return 0;

  err_psy_reg_usbpower:
   power_supply_unregister(&g_multi_data.ac_power_psy);

  err_psy_reg_power:
   power_supply_unregister(&g_multi_data.battery_psy);

  err_psy_reg_battery:
   wake_lock_destroy(&g_multi_data.wakelock);

   /* see comment in battery_remove */
   flush_scheduled_work();
   flush_workqueue(isr_wq);
   destroy_workqueue(isr_wq);
  err_isr_wq:
   pm_power_off = NULL;
   return ret;
}

static int __devexit battery_remove(struct platform_device *p_dev)
{
   /* unregister power supplies from PSY framework */
   power_supply_unregister(&g_multi_data.battery_psy);
   power_supply_unregister(&g_multi_data.ac_power_psy);
   power_supply_unregister(&usb_power_psy);

   /*
    * now flush all pending work.
    * we won't get any more schedules, since all
    * sources (isr and external_power_changed)
    * are unregistered now.
    */
   flush_scheduled_work();
   flush_workqueue(isr_wq);
   destroy_workqueue(isr_wq);

   /* flush the delayed work. */
   cancel_delayed_work_sync(&g_multi_data.t_battery_dwork);
   
   pm_power_off = NULL;
   g_is_driver_ok = false;

   if (g_multi_data.pt_battery_monitor != NULL)
      close_gpios(g_multi_data.pt_battery_monitor);

   wake_lock_destroy(&g_multi_data.wakelock);
   return 0;
}

static struct platform_driver battery_driver = 
{
   .driver.name = HW_CMP_MULTI_DRIVER_NAME,
   .driver.owner = THIS_MODULE,
   .probe = battery_probe,
   .remove = __devexit_p(battery_remove),
#ifdef CONFIG_PM
   .suspend = battery_suspend,
   .resume = battery_resume,
#endif
};

static int __init battery_init(void)
{
   int ret;

   ret = platform_driver_register(&battery_driver);
   if (ret) 
   {
      printk(KERN_ERR "%s(): platform_driver_register failed %d\n",
             __FUNCTION__, ret);
      return ret;
   }
   return 0;
}

static void __exit battery_exit(void)
{
   platform_driver_unregister(&battery_driver);
}

module_init(battery_init);
module_exit(battery_exit);

MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("Broadcom CMP Battery Multi Driver");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.02");
