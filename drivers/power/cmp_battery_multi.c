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
 * registered at a time.
 *
 * Platform information is passed into the driver via the platform_data pointer.
 *
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

/* ---- Public Variables ------------------------------------------------- */
static int    mod_debug = 0x0; 
module_param(mod_debug, int, 0644);

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

struct battery_data 
{
   int status;
   struct power_supply t_power_supply;
   int full_chrg;

   /* Power supply properties */
   int battery_technology;  /* POWER_SUPPLY_PROP_TECHNOLOGY */
   int battery_max;         /* POWER_SUPPLY_PROP_VOLTAGE_MAX */
   int battery_min;         /* POWER_SUPPLY_PROP_VOLTAGE_MIN */

   struct multi_data *pt_multi_data;
};

struct power_data 
{
   int online;       /* POWER_SUPPLY_PROP_ONLINE */
   struct power_supply t_power_supply;
   
   int prev_online;

   struct multi_data *pt_multi_data;
};

/* Main structure used in this driver. */
struct multi_data
{
   struct battery_data *pt_battery_data;
   struct power_data   *pt_power_data;
   
   /* Called when AC is plugged in or removed. */
   struct work_struct  t_ac_power_work;
   
   struct delayed_work t_battery_dwork;
   struct delayed_work t_power_dwork;

   /* battery monitor instance and data that is registered with the driver */
   struct battery_monitor *pt_battery_monitor;
   void *pt_battery_monitor_data;

   struct mutex work_lock;                 /* Protects data. */    
   struct wake_lock wakelock;              /* keeps device awake */
   
   int battery_milliVolts;                 /* save the monitor voltage */
   int battery_charge;                     /* save the monitor charge */
   int battery_status;
   
   /* Reference to platform_data passed in by probe. *
    * Needed for suspend() and resume().             */
   struct cbm_platform_data    *pt_platform_data;
};	

#define DEBOUNCE_TIME_USECS        128000
#define MULTI_WAIT_PERIOD          3000
#define MULTI_NO_WAIT              0

#define INT_SETUP_SLEEP_MSECS      50

static char *work_callers[] = 
{  
   "reset", 
   "ISR",
   "poll",
   "resume",
   "external power changed",
};

typedef enum
{
   enum_caller_reset,
   enum_caller_isr,
   enum_caller_poll,
   enum_caller_resume,
   enum_caller_external_power_changed
} work_caller_enum;  


/* ---- Private Variables ------------------------------------------------ */
static const __devinitconst char gBanner[] =
    KERN_INFO "Multi Battery Driver: 1.00\n";

/* workqueue and work task for debouncing power input signal */
static struct workqueue_struct *isr_wq;

const char   *wake_lock_name = "multi_battery_driver";
#define WAKE_LOCK_TIMEOUT_DURATION 6*HZ

static struct power_data gt_power_info;

static work_caller_enum battery_work_caller = 0; 
static work_caller_enum power_work_caller   = 0; 
static int              test_isr_count      = 0;

/* todo: may be used in the future to improve diagnostic */
static char *status_text[] = 
{
   [POWER_SUPPLY_STATUS_UNKNOWN]     = "Unknown",
   [POWER_SUPPLY_STATUS_CHARGING]    = "Charging",
   [POWER_SUPPLY_STATUS_DISCHARGING] = "Discharging",
   [POWER_SUPPLY_STATUS_FULL]        = "Full",
   [POWER_SUPPLY_STATUS_NOT_CHARGING]= "Not Charging",
};

static char *power_supplied_to[] = 
{
   "battery",
};

/* array of battery properties supported by this driver */
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

/* array of "power"  properties supported by this driver */
static enum power_supply_property power_data_props[] = {
   POWER_SUPPLY_PROP_ONLINE,             /* AC plugged in or not      */ 
};

/* array of "usb power"  properties supported by this driver */
static enum power_supply_property usbpower_data_props[] = {
   POWER_SUPPLY_PROP_ONLINE,             /* USB power plugged in or not      */ 
};


/* initialize PSY defined battery data */
static struct battery_data gt_battery_info = 
{
   .status = POWER_SUPPLY_STATUS_DISCHARGING,
   .full_chrg = -1,
   .t_power_supply = 
   {
      .name                   = "battery",
      .type                   = POWER_SUPPLY_TYPE_BATTERY,
      .properties             = battery_data_props,
      .num_properties         = ARRAY_SIZE(battery_data_props),
      .get_property           = battery_get_property,
      .external_power_changed = battery_external_power_changed,
      .use_for_apm            = 1,
   },
   .battery_technology = POWER_SUPPLY_TECHNOLOGY_LION,
};

/* initialize PSY defined power data */
static struct power_data gt_power_info = 
{
   .online = -1,

   .t_power_supply = 
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

/* initialize PSY defined usb power data */
static struct power_supply usb_power_supply = 
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

static struct multi_data *gpt_multi_data = NULL;

/* flag inidicating if the driver is ready for battery monitor drivers 
   to register */ 
static bool               g_is_driver_ok = false;
            
/* ---- Private Function Prototypes -------------------------------------- */
static void cmp_battery_multi_power_off(void);
static bool is_ac_connected            (struct multi_data *pt_multi_data, 
                                        int ac_gpio_level);
static void multi_power_work           (struct work_struct *p_work);
static void multi_battery_work         (struct work_struct *p_work);
static int  setup_gpios                (struct battery_monitor *pt_battery_monitor);
static void close_gpios                (struct battery_monitor *pt_battery_monitor);
static void ac_power_isr_handler_work  (struct work_struct *p_work);

/* ---- Public Functions ------------------------------------------------- */

int register_battery_monitor(struct battery_monitor *p_monitor,
                             void   *p_data)
{
   struct power_data  *pt_power_data;
   int                 ac_power_gpio_level;
   int                 rc;
   
   printk("%s() called\n", __FUNCTION__);
   
   if (g_is_driver_ok == false)
   {  
      printk("%s() battery monitor %s cannot register. "
             "Battery driver is not ready.\n", __FUNCTION__, p_monitor->name);
      return -ENODATA;       
   }
   
   if (gpt_multi_data == NULL)
   {
      printk("%s() battery monitor %s cannot register. "
             "Called too early, multi data is not created\n",
             __FUNCTION__, p_monitor->name);
      return -ENODATA;
   }

   if ((rc = setup_gpios(p_monitor)) != 0)
   {
      printk("%s() setup_gpios() returned %d\n", __FUNCTION__, rc);
      return rc;
   }
   
   /* store monitor and monitor data in multi struct */
   gpt_multi_data->pt_battery_monitor = p_monitor;
   gpt_multi_data->pt_battery_monitor_data = p_data;

   if (mod_debug)
         printk("%s() battery monitor %s successfully registered "
                "with multi battery driver\n", __FUNCTION__, p_monitor->name); 

   pt_power_data = gpt_multi_data->pt_power_data;	
   
   /* Is the tablet plugged or not? Need to know for wakelock. */
   ac_power_gpio_level = 
      gpio_get_value(gpt_multi_data->pt_battery_monitor->gpio_ac_power);
   pt_power_data->online = is_ac_connected(gpt_multi_data, ac_power_gpio_level);
   pt_power_data->prev_online = pt_power_data->online;
   
   if (pt_power_data->online)
   {  /* The device is connected to an AC power supply. */
      wake_lock(&gpt_multi_data->wakelock);
      if (mod_debug)                       
      {
         printk("%s() adding wake lock\n", __FUNCTION__);
      }          
   }   
   
   INIT_WORK(&gpt_multi_data->t_ac_power_work, ac_power_isr_handler_work);
   schedule_delayed_work(&gpt_multi_data->t_power_dwork, MULTI_NO_WAIT);
   schedule_delayed_work(&gpt_multi_data->t_battery_dwork, MULTI_NO_WAIT);
   return 0;
}

/* ---- Functions -------------------------------------------------------- */

static bool is_monitor_registered(void)
{
   if (gpt_multi_data->pt_battery_monitor == NULL)
      return false;
   else
      return true;
}      

/* todo: properly implement in the future, if needed */
static unsigned long read_temp(struct battery_data *p_battery)
{
   unsigned long value = 0;
   return value;
}

/* AC power pin level differs between boards. */
static bool is_ac_connected(struct multi_data *pt_multi_data, 
                            int ac_gpio_level)
{
   bool ret;
   
   if (pt_multi_data->pt_battery_monitor->ac_power_on_level > 0)
   {  /* GPIO high indicates AC power is connected. */
      if (ac_gpio_level > 0)
      {
         ret = true;
      }   
      else
      {
         ret = false;
      }   
   }      
   else
   {   /* GPIO low indicates AC power is connected. */
      if (ac_gpio_level > 0)
      {
         ret = false;
      }   
      else
      {
         ret = true;
      }   
   }      
   
   return ret;
}

/* Battery charge pin differs between tablets. */
/* todo: review the logic of status assigning */
static int get_battery_status(struct multi_data *pt_multi_data)
{
   int status = POWER_SUPPLY_STATUS_UNKNOWN;

   if(pt_multi_data->pt_battery_monitor != NULL)
   {
      if (pt_multi_data->pt_power_data->online)
      {
         /* if battery monitor has charging pin */
         if (pt_multi_data->pt_battery_monitor->gpio_charger != -1)
         {
            if (gpio_get_value(pt_multi_data->pt_battery_monitor->gpio_charger))
            {
               status = POWER_SUPPLY_STATUS_NOT_CHARGING;
            } 
            else 
            {
               status = POWER_SUPPLY_STATUS_CHARGING;
            }
         }
      }

      if (pt_multi_data->pt_battery_monitor->gpio_charger == -1 && 
          pt_multi_data->pt_power_data->online)
      {
         status = POWER_SUPPLY_STATUS_CHARGING;
      }

      if (pt_multi_data->pt_battery_monitor->gpio_charger == -1 && 
          !pt_multi_data->pt_power_data->online)
      {
         status = POWER_SUPPLY_STATUS_DISCHARGING;
      }
   }
   return status;
}

static int cbm_get_battery_charge(struct multi_data *pt_multi_data)
{
   int charge_percent;   
   int current_voltage = pt_multi_data->battery_milliVolts;
   int battery_max     = pt_multi_data->pt_platform_data->battery_max_voltage;
   int battery_min     = pt_multi_data->pt_platform_data->battery_min_voltage;
      
   /* if battery monitor is not registered */
   if (pt_multi_data->pt_battery_monitor == NULL)
   {
      /* calculate charge */
      charge_percent = 
         (current_voltage - battery_min)*100/(battery_max-battery_min);
   }
   else
   {  
      /* 
       * todo: change this to retrieve it rather than use an out of
       * date value. 
       */
      charge_percent = pt_multi_data->battery_charge;  
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
      if (mod_debug)
         printk("%s() charge out of range %d, setting to 100\n",
                __FUNCTION__, charge_percent);
      charge_percent = 100;
   }         
      
   return charge_percent;
}

static int battery_get_property(struct power_supply *pt_power_supply,
                                enum power_supply_property psp,
                                union power_supply_propval *val)
{
   int ret = 0;
   struct battery_data *pt_battery = container_of(pt_power_supply, 
                                                  struct battery_data, 
                                                  t_power_supply);

   if (mod_debug)                       
      printk("%s() requested property: %d\n", 
             __FUNCTION__, psp);
   
   switch (psp) 
   {
      case POWER_SUPPLY_PROP_STATUS:
         val->intval = pt_battery->status;
         break;
      case POWER_SUPPLY_PROP_HEALTH:
         val->intval = POWER_SUPPLY_HEALTH_GOOD;
         break;
      case POWER_SUPPLY_PROP_PRESENT:
          /* for now we are assuming that battery is always present */
          /* todo: came up with the way to determine its presence */
          val->intval = 1;
         break;
      case POWER_SUPPLY_PROP_TECHNOLOGY:
         val->intval = pt_battery->battery_technology;
         break;
      case POWER_SUPPLY_PROP_VOLTAGE_MAX:
         val->intval = pt_battery->full_chrg*1000;
         break;
      case POWER_SUPPLY_PROP_VOLTAGE_MIN:
         val->intval = pt_battery->battery_min*1000;
         break;
      case POWER_SUPPLY_PROP_VOLTAGE_MAX_DESIGN:
         val->intval = pt_battery->battery_max*1000;
         break;
      case POWER_SUPPLY_PROP_VOLTAGE_MIN_DESIGN:
         val->intval = pt_battery->battery_min*1000;
         break;
      case POWER_SUPPLY_PROP_VOLTAGE_NOW:
         /* if battery monitor is registered */
         if (pt_battery->pt_multi_data->pt_battery_monitor != NULL)
         {
            /* todo: retrieve this data from the monitor directly */ 
            val->intval = pt_battery->pt_multi_data->battery_milliVolts*1000; 
         }
         else
            val->intval = pt_battery->battery_max*1000;            
         break;
      case POWER_SUPPLY_PROP_CAPACITY:   
         val->intval = cbm_get_battery_charge(pt_battery->pt_multi_data);
         break;
      case POWER_SUPPLY_PROP_TEMP:
         val->intval = read_temp(pt_battery);
         break;
      default:
         ret = -EINVAL;
         break;
   }

   if (mod_debug)                       
      printk("%s() property: %d val: %d\n", 
             __FUNCTION__, psp, val->intval);
   
   return ret;
}

static int power_get_property(struct power_supply *pt_power_supply,
                              enum power_supply_property psp,
                              union power_supply_propval *val)
{
   struct power_data *p_pow = container_of(pt_power_supply, 
                                           struct power_data, 
                                           t_power_supply);
   
   switch (psp) 
   {
      case POWER_SUPPLY_PROP_ONLINE:         
         val->intval = p_pow->online;
         break;
      
      default:
         return -EINVAL;
   }
   
   if (mod_debug)                       
   {
      printk("%s() property: %d val: %d\n", 
             __FUNCTION__, psp, val->intval);
   }

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
   
   if (mod_debug)                       
   {
      printk("%s() property: %d val: %d\n", 
             __FUNCTION__, psp, val->intval);
   }

   return 0;
}

/* Called when the device is plugged into or removed from an AC supply. */
static irqreturn_t ac_power_gpio_isr(int irq, void *data)
{
   struct multi_data *pt_multi_data = (struct multi_data *)data;
   
   if (pt_multi_data == NULL)
   {
      printk("%s() ERROR, pt_multi_data == NULL!\n", __FUNCTION__);
      return IRQ_HANDLED;      
   }
   
   test_isr_count++;

   power_work_caller = enum_caller_isr;
   
   queue_work(isr_wq, &pt_multi_data->t_ac_power_work);
   
   return IRQ_HANDLED;
}  /* ac_power_gpio_isr() */

static void battery_update(struct multi_data *pt_multi_data)
{
   int old_battery_status;
   struct battery_data *pt_battery     = pt_multi_data->pt_battery_data;
   struct power_supply  *pt_battery_psy = &pt_battery->t_power_supply;
   struct battery_monitor *pt_battery_monitor = 
      pt_multi_data->pt_battery_monitor;

   int battery_value;
   
   if (is_monitor_registered() == false)
   {
      if (mod_debug)
         printk("%s() battery monitor not registered\n", __FUNCTION__);
      return;   
   }   

   mutex_lock(&pt_multi_data->work_lock);
   
   if (mod_debug)
      printk("%s() entering\n", __FUNCTION__);

   old_battery_status = pt_battery->status;
   pt_battery->status = get_battery_status(pt_multi_data); 

   if (old_battery_status != pt_battery->status) 
   {
      printk("%s() battery status changed: %d\n",
             __FUNCTION__, pt_battery->status);
   }

   /* Retrieve the battery voltage from the monitor. */   
   if (pt_battery_monitor->get_voltage_fn != NULL)
   {   
      battery_value = 
         pt_battery_monitor->get_voltage_fn(pt_multi_data->pt_battery_monitor_data);
      
      if (battery_value == 0)
      {  
         /* Sometimes occurs and may cause tablet to restart. */
         battery_value = pt_multi_data->pt_platform_data->battery_min_voltage;
      }
      
      pt_multi_data->battery_milliVolts = battery_value;      
      if (mod_debug)
         printk("%s() battery_voltage: %d battery_milliVolts: %d\n", 
                __FUNCTION__, battery_value, pt_multi_data->battery_milliVolts);
   }
   else
   {
      if (mod_debug)
         printk("%s() battery monitor get_battery_voltage_fn() "
                "not configured\n", __FUNCTION__);
   }

   /* Retrieve the battery charge from the monitor. */      
   if (pt_battery_monitor->get_charge_fn != NULL)
   {      
      battery_value = 
         pt_battery_monitor->get_charge_fn(pt_multi_data->pt_battery_monitor_data);
      pt_multi_data->battery_charge = battery_value;      

      if (mod_debug)
         printk("%s() battery_charge: %d battery_charge: %d\n", 
                __FUNCTION__, battery_value, pt_multi_data->battery_charge);      
   }            
   else
   {
      if (mod_debug)
         printk("%s() battery monitor get_charge_fn() not configured\n",
                __FUNCTION__);
   }

   power_supply_changed(pt_battery_psy); 
   mutex_unlock(&pt_multi_data->work_lock);
   
}

static void power_update(struct multi_data *pt_multi_data)
{  	
   int gpio_power_level;
   struct power_data  *pt_power_data = pt_multi_data->pt_power_data;	
   struct power_supply *pt_power_supply = &(pt_power_data->t_power_supply);

   if (is_monitor_registered() == false)
   {
      if (mod_debug)
         printk("%s() battery monitor not registered\n", __FUNCTION__);
      return;   
   }   
   
   mutex_lock(&pt_multi_data->work_lock);

   if (mod_debug)                       
   {
      printk("%s() running, gpio_ac_power: %d\n",
             __FUNCTION__, pt_multi_data->pt_battery_monitor->gpio_ac_power);
   }
   
   gpio_power_level = 
      gpio_get_value(pt_multi_data->pt_battery_monitor->gpio_ac_power);
   
   if (is_ac_connected(pt_multi_data, gpio_power_level))
   {
      pt_power_data->online = 1;
   }
   else
   {
      pt_power_data->online = 0;
   }
   
   if (mod_debug > 1)                       
   {
      printk("cmp_battery_multi.c %s() gpio %d prev_online: %d online: %d\n", 
             __FUNCTION__, pt_multi_data->pt_battery_monitor->gpio_ac_power, 
             pt_power_data->prev_online, 
             pt_power_data->online);
   }          
   
   if (pt_power_data->prev_online != pt_power_data->online)
   {  /* AC power status has changed. */
      if (pt_power_data->online)  
      {      
         if (mod_debug)                       
            printk("AC applied to tablet\n");
      }
      else
      {      
         if (mod_debug)                       
            printk("AC removed from tablet\n");
      }   
   }
   
   pt_power_data->prev_online = pt_power_data->online;

   if (mod_debug > 2)                       
   {
      printk("cmp_battery_multi.c %s() calling power_supply_changed()\n", 
             __FUNCTION__);                          
   }          
   power_supply_changed(pt_power_supply);
   
   mutex_unlock(&pt_multi_data->work_lock);
}  /* power_update() */

/* Uses delayed work. */
static void multi_battery_work(struct work_struct *p_work)
{
   struct multi_data    *pt_multi_data;
   struct battery_data *pt_battery_data;
   pt_multi_data = container_of(p_work, 
                                struct multi_data, 
                                t_battery_dwork.work);
   pt_battery_data = pt_multi_data->pt_battery_data;
   
   if (pt_battery_data == NULL)
   {
      printk("%s() error: pt_battery_data == NULL\n", __FUNCTION__);
      return;
   }	   

   /* Let battery_update() call power_supply_changed(..) that informs the    *
    * the system that a value changed.                                       */
   battery_update(pt_multi_data); 
   schedule_delayed_work(&pt_multi_data->t_battery_dwork, MULTI_WAIT_PERIOD);
}

static void multi_power_work(struct work_struct *p_work)
{
   struct multi_data *pt_multi_data;
   struct power_data *pt_power_data;
   
   pt_multi_data = container_of(p_work, struct multi_data, t_power_dwork.work);
   pt_power_data = pt_multi_data->pt_power_data;

   if (pt_power_data == NULL)
   {
      printk("%s() error: pt_power_data == NULL\n", __FUNCTION__);
      return;
   }	   

   if (mod_debug)
   {
      printk("%s() running gpio_ac_power: %d gpio_power_button: %d\n", 
             __FUNCTION__, 
             pt_multi_data->pt_battery_monitor->gpio_ac_power, 
             pt_multi_data->pt_platform_data->gpio_power_control);
   } 
   
   /* Let power_update() call power_supply_changed(..) */      
   power_update(pt_multi_data);    
   schedule_delayed_work(&pt_multi_data->t_power_dwork, MULTI_WAIT_PERIOD);
}

static void ac_power_isr_handler_work(struct work_struct *p_work)
{
   struct multi_data *pt_multi_data;
   int gpio_val;
      
   if (mod_debug)
   {
      printk("running %s() name: %s\n", __FUNCTION__, 
             pt_multi_data->pt_power_data->t_power_supply.name);
   }          
   
   pt_multi_data = container_of(p_work, struct multi_data, t_ac_power_work);   
                                
   gpio_val = gpio_get_value(pt_multi_data->pt_battery_monitor->gpio_ac_power);
      
   if (is_ac_connected(pt_multi_data, gpio_val))
   {  /* The tablet is plugged into an external power supply. */
      wake_lock(&pt_multi_data->wakelock);
      if (mod_debug)                       
         printk("%s() adding wake lock\n", __FUNCTION__);
   }
   else
   {  /* The tablet has been unplugged from external power supply. */
      wake_unlock(&pt_multi_data->wakelock);
      if (mod_debug)                       
         printk("%s() removing wake lock\n", __FUNCTION__);
   }
      
   if (mod_debug > 2)                       
   {
      printk("running %s() run by: %s isr count: %d\n", 
             __FUNCTION__, work_callers[power_work_caller], test_isr_count);
   }          
   
   /* These two methods call power_supply_changed(). */
   power_update(pt_multi_data);
   battery_update(pt_multi_data);
   power_work_caller = enum_caller_reset;
   
   /* Need to update power info ahead of battery to get ac power state. */
   schedule_delayed_work(&pt_multi_data->t_power_dwork, MULTI_WAIT_PERIOD);
   schedule_delayed_work(&pt_multi_data->t_battery_dwork, MULTI_WAIT_PERIOD);
}  /* ac_power_isr_handler_work() */

static void battery_external_power_changed(struct power_supply *t_power_supply)
{   
   battery_work_caller = enum_caller_external_power_changed;
   
   if (mod_debug)
      printk("%s() called\n", __FUNCTION__);
}

static void cmp_battery_multi_power_off(void)
{
   gpio_set_value(g_gpio_power_control, 0);
}

static int setup_gpios(struct battery_monitor *pt_battery_monitor)
{
   int rc;
   
   if (mod_debug)
   {
      printk("%s() power down gpio: %d\n", __FUNCTION__, g_gpio_power_control);
      printk("%s() ac power gpio  : %d\n",
             __FUNCTION__, pt_battery_monitor->gpio_ac_power); 
      printk("%s() charger gpio   : %d\n",
             __FUNCTION__, pt_battery_monitor->gpio_charger); 
   }

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

   if (pt_battery_monitor->gpio_ac_power >= 0)
   {
      rc = gpio_request_one(pt_battery_monitor->gpio_ac_power,
                            GPIOF_IN, "ac power");
      if (rc)
      {
         close_gpios(pt_battery_monitor);
         return rc;
      }   
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
                     "ac detect", gpt_multi_data);
   if (rc)
   {
      printk("%s():  request_irq(gpio_to_irq(%d)) returned %d\n",
             __FUNCTION__, pt_battery_monitor->gpio_ac_power, rc);
      close_gpios(pt_battery_monitor);
      return rc;
   }

   return 0;
}

static void close_gpios(struct battery_monitor *pt_battery_monitor)
{
   /* release AC power switch IRQ */
   free_irq(gpio_to_irq(pt_battery_monitor->gpio_ac_power), &gt_power_info);

   if (g_gpio_power_control >= 0)
      gpio_free(g_gpio_power_control);
      
   if (pt_battery_monitor->gpio_ac_power >= 0)  
      gpio_free(pt_battery_monitor->gpio_ac_power);
      
   if (pt_battery_monitor->gpio_charger >= 0)   
      gpio_free(pt_battery_monitor->gpio_charger);
}

#ifdef CONFIG_PM
static int battery_suspend(struct platform_device *p_dev, pm_message_t state)
{
   struct cbm_platform_data *pt_data = 
      (struct cbm_platform_data *)p_dev->dev.platform_data;
   struct multi_data *pt_multi_data  = (struct multi_data *)pt_data->p_cbm_data;
   /* flush all pending status updates */
   flush_scheduled_work();  
   flush_workqueue(isr_wq);
   /* Flush the delayed work. */
   cancel_delayed_work_sync(&pt_multi_data->t_power_dwork);
   cancel_delayed_work_sync(&pt_multi_data->t_battery_dwork);
   return 0;
}

static int battery_resume(struct platform_device *p_dev)
{
   struct cbm_platform_data *pt_data = 
      (struct cbm_platform_data *)p_dev->dev.platform_data;
   struct multi_data *pt_multi_data  = (struct multi_data *)pt_data->p_cbm_data;
   /* Things may have changed while we were away */
   power_work_caller = enum_caller_resume;

   /* How to do in a SMP centric safe manner? */
   schedule_delayed_work(&pt_multi_data->t_power_dwork, MULTI_WAIT_PERIOD);
   schedule_delayed_work(&pt_multi_data->t_battery_dwork, MULTI_WAIT_PERIOD);
   battery_work_caller = enum_caller_resume;
   return 0;
}
#endif

static int __devinit battery_probe(struct platform_device *p_dev)
{
   int ret;
   struct multi_data    *pt_multi_data;
   struct battery_data  *pt_battery_data;
   struct power_data    *pt_power_data;
   /* CBM (C)mp (B)attery (M)ulti. */
   struct cbm_platform_data *pt_cbm_platform_data;

   printk(gBanner);

   if (p_dev->dev.platform_data == NULL)
   {  /* Need this information. */
      printk("%s() error p_dev->dev.platform_data == NULL\n", __FUNCTION__);
      return -ENODATA;
   }
   
   pt_cbm_platform_data = (struct cbm_platform_data *)p_dev->dev.platform_data;
   
   if (mod_debug)
   {
      printk("battery max voltage: %d\n", 
             pt_cbm_platform_data->battery_max_voltage);
      printk("battery min voltage: %d\n", 
             pt_cbm_platform_data->battery_min_voltage);
   }  

   /* store platform data in battery info structure */
   gt_battery_info.battery_max = pt_cbm_platform_data->battery_max_voltage;   
   gt_battery_info.full_chrg = pt_cbm_platform_data->battery_max_voltage;   
   gt_battery_info.battery_min = pt_cbm_platform_data->battery_min_voltage;   
   gt_battery_info.battery_technology = pt_cbm_platform_data->battery_technology;

   /* Needed to turn off the device. */   
   g_gpio_power_control = pt_cbm_platform_data->gpio_power_control;
   
   pt_multi_data = kzalloc(sizeof(struct multi_data), GFP_KERNEL);
   if (pt_multi_data == NULL)
      return -ENOMEM;

   gpt_multi_data = pt_multi_data;
      
   pt_battery_data = kzalloc(sizeof(struct battery_data), GFP_KERNEL);
   if (pt_battery_data == NULL)
   {
      kfree(pt_multi_data);
      return -ENOMEM;
   }
   
   pt_power_data = kzalloc(sizeof(struct power_data), GFP_KERNEL);
   if (pt_power_data == NULL)
   {
      kfree(pt_multi_data);   
      kfree(pt_battery_data);   
      return -ENOMEM;
   }

   mutex_init(&pt_multi_data->work_lock);

   memcpy(pt_battery_data, (void *)&gt_battery_info, sizeof(struct battery_data));
   memcpy(pt_power_data, (void *)&gt_power_info, sizeof(struct power_data));

   /* Set the battery monitor specific values. */
   pt_multi_data->pt_battery_data = pt_battery_data;
   pt_multi_data->pt_power_data   = pt_power_data;

   /* Need a reference to the parent structure. */
   pt_battery_data->pt_multi_data = pt_multi_data;
   pt_power_data->pt_multi_data   = pt_multi_data;

   /* Needed when doing suspend() or resume(). */
   pt_cbm_platform_data->p_cbm_data = pt_multi_data;
   
   /* So the platform data can be retrieved. */
   pt_multi_data->pt_platform_data = pt_cbm_platform_data;

   if (mod_debug)
   {
      printk("%s() name: %s\n", __FUNCTION__, pt_power_data->t_power_supply.name);
   } 
   
   /* Tell the system how to turn off the power. */
   /* pm_power_off is a global function pointer. It is declared as external in
      linux/pm.h. I am not sure where it is declared and how user space uses it */
   pm_power_off = cmp_battery_multi_power_off;

   isr_wq = create_workqueue("multi_isr_wq");
   if (isr_wq == NULL)
   {
      printk("%s() Can not create ISR workqueue\n", __FUNCTION__);
      ret = -1;
      goto err_isr_wq;
   }

   /* Use a wake lock to keep the tablet awake. */
   wake_lock_init(&pt_multi_data->wakelock, WAKE_LOCK_SUSPEND, wake_lock_name);

   /* Prepare to start the delayed work. */
   INIT_DELAYED_WORK_DEFERRABLE(&pt_multi_data->t_power_dwork, multi_power_work);
   INIT_DELAYED_WORK_DEFERRABLE(&pt_multi_data->t_battery_dwork, multi_battery_work);

   ret = power_supply_register(&p_dev->dev, &pt_battery_data->t_power_supply);
   if (ret)
      goto err_psy_reg_battery;

   ret = power_supply_register(&p_dev->dev, &pt_power_data->t_power_supply);
   if (ret)
      goto err_psy_reg_power;

   ret = power_supply_register(&p_dev->dev, &usb_power_supply);
   if (ret)
      goto err_psy_reg_usbpower;

   /* return success */
   g_is_driver_ok = true;
   return 0;

  err_psy_reg_usbpower:
   power_supply_unregister(&pt_multi_data->pt_power_data->t_power_supply);

  err_psy_reg_power:
   power_supply_unregister(&pt_multi_data->pt_battery_data->t_power_supply);

  err_psy_reg_battery:
   wake_lock_destroy(&pt_multi_data->wakelock);

   /* see comment in battery_remove */
   flush_scheduled_work();
   flush_workqueue(isr_wq);
   destroy_workqueue(isr_wq);
  err_isr_wq:
   kfree(pt_multi_data);
   kfree(pt_battery_data);
   kfree(pt_power_data);
   pm_power_off = NULL;
   printk("%s(): return %d\n", __FUNCTION__, ret);
   return ret;
}

static int __devexit battery_remove(struct platform_device *p_dev)
{
   struct cbm_platform_data *pt_data = 
      (struct cbm_platform_data *)p_dev->dev.platform_data;
   struct multi_data *pt_multi_data  = (struct multi_data *)pt_data->p_cbm_data;

   printk("%s() entering ...\n", __FUNCTION__);

   /* unregister power supplies from PSY framework */
   power_supply_unregister(&pt_multi_data->pt_battery_data->t_power_supply);
   power_supply_unregister(&pt_multi_data->pt_power_data->t_power_supply);

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
   cancel_delayed_work_sync(&pt_multi_data->t_battery_dwork);
   cancel_delayed_work_sync(&pt_multi_data->t_power_dwork);
   
   pm_power_off = NULL;
   g_is_driver_ok = false;

   if (pt_multi_data->pt_battery_monitor != NULL)
      close_gpios(pt_multi_data->pt_battery_monitor);

   wake_lock_destroy(&pt_multi_data->wakelock);
   mutex_destroy(&pt_multi_data->work_lock);

   /* free all the data structures allocated in probe */
   kfree(pt_multi_data->pt_battery_data);
   kfree(pt_multi_data->pt_power_data);
   kfree(pt_multi_data);

   printk("%s() exiting ...\n", __FUNCTION__);   
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
      printk("%s(): platform_driver_register failed %d\n", __FUNCTION__,
             ret);
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
MODULE_VERSION("1.01");
