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
 * A driver to monitor the MAX17085 or LTC4006, the AC power and charge state of
 * the battery. 
 * On A1
 * MAX17085 Charger              GPIO  Direction
 * BAT_CH_B                      180   SOC to MAX17085 ?
 * ACOK(Active Low) connected to 181   MAX17085 to SOC confirmed (lo-AC, hi-battery)
 * PGOOD                         18    MAX17085 to SOC ?
 *
 * POWER_HOLD                    85    Out of Host
 * PWR_BUTTON                    4     In to Host  
 *
 * Platform information is passed into the driver via the platform_data pointer.
 * It follows that all data to be accessible via the platfrom_data as well.
 * This is what is looks like:
 * dev->dev.platform_data = cbm_platform_data 
 * cbm_platform_data ->p_cbm_data <== is void * but is really type struct
 * multi_data *. 
 *
 */
 
 /* 
  * NOTE:
  * It is important that the charge level of the battery returned by 
  * POWER_SUPPLY_PROP_CAPACITY never be accidentally set to zero. This might
  * cause a reboot by the parent OS. It is also bad if the value is greater 
  * than 100.
  */

#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/power_supply.h>
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
static int battery_get_property(struct power_supply *psy,
                                enum power_supply_property psp,
                                union power_supply_propval *val);
static int power_get_property(struct power_supply *psy,
                              enum power_supply_property psp,
                              union power_supply_propval *val);
                              
static void battery_external_power_changed(struct power_supply *psy);

/* ---- Private Constants and Types -------------------------------------- */

static struct battery_data 
{
   int status;
   struct power_supply t_power_supply;
   int full_chrg;

   bool(*is_present) (struct battery_data *p_battery);
   int gpio_charging_b;

   /* Power supply properties */
   int battery_technology;  /* POWER_SUPPLY_PROP_TECHNOLOGY */
   int battery_max;         /* POWER_SUPPLY_PROP_VOLTAGE_MAX */
   int battery_min;         /* POWER_SUPPLY_PROP_VOLTAGE_MIN */
   
   int adc_bat;
   int adc_temp;
         
   struct multi_data *pt_multi_data;
}; /* battery_data */

struct power_data 
{
   int online;       /* POWER_SUPPLY_PROP_ONLINE */
   struct power_supply t_power_supply;
   
   int prev_online;

   struct multi_data *pt_multi_data;
}; /* power_data */


/* Main structure used in this driver. */
struct multi_data
{
   struct battery_data *pt_battery_data;
   struct power_data   *pt_power_data;
   
   struct work_struct  t_ac_power_work; /* Called when AC is plugged in or removed. */
   
   struct delayed_work t_battery_dwork;
   struct delayed_work t_power_dwork;
   
   struct mutex work_lock;                 /* Protects data. */    
   struct wake_lock wakelock;             /* keeps device awake */
   
   /* Monitor values set at runtime. */
   int (*get_battery_voltage_fn)(void *p_data); /* read the monitor voltage */
   int (*get_battery_charge_fn) (void *p_data); /* read the monitor charge */
   void **p_battery_monitor_data;               /* monitor specific data */
   int battery_milliVolts;                      /* save the monitor voltage */
   int battery_charge;                          /* save the monitor charge */
   int battery_status;
   
   struct battery_monitor_info *pt_monitor_info;
   
   /* For battery monitor registration. */
   //struct battery_monitor_calls monitor_calls;     
   
   /* Reference to platform_data passed in by probe. *
    * Needed for suspend() and resume().             */
   struct cbm_platform_data    *pt_platform_data;
};	

#define NUM_DEBOUNCE_TRIES         5
#define DEBOUNCE_WAIT_MSECS        50 
#define INITIAL_POLL_INTERVAL      5000
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

static bool battery_is_present(struct battery_data *p_battery);

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

static enum power_supply_property battery_data_props[] = 
{
   POWER_SUPPLY_PROP_STATUS,             /* 0 charging, discharging ... */
   POWER_SUPPLY_PROP_HEALTH,             /* 2 good, overheat, dead ...  */
   POWER_SUPPLY_PROP_PRESENT,            /* 3                           */
   POWER_SUPPLY_PROP_TECHNOLOGY,         /* 5 NiMH|LION|...             */ 
   POWER_SUPPLY_PROP_VOLTAGE_MAX,        /* 7 battery_max               */
   POWER_SUPPLY_PROP_VOLTAGE_MIN,        /* 8 battery_min               */
   POWER_SUPPLY_PROP_VOLTAGE_MAX_DESIGN, /* 9                           */
   POWER_SUPPLY_PROP_VOLTAGE_MIN_DESIGN, /* 10                          */
   POWER_SUPPLY_PROP_VOLTAGE_NOW,        /* 11 battery voltage(mV)      */
   POWER_SUPPLY_PROP_CAPACITY,           /* 30 Capacity in percent      */
   POWER_SUPPLY_PROP_TEMP,               /* 32 Degrees celsius          */
};

static enum power_supply_property power_data_props[] = {
   POWER_SUPPLY_PROP_ONLINE,             /* 4 AC plugged in or not      */ 
};

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
   .is_present = battery_is_present,
   .gpio_charging_b = -1,

   .adc_bat = -1,
   .battery_technology = POWER_SUPPLY_TECHNOLOGY_LION,

   .adc_temp = -1,
};

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

static int g_gpio_power_control = -1;

/* Needed so battery monitor drivers can register with this driver. */
static struct multi_data *gpt_multi_data = NULL;
static bool               g_is_driver_ok = false;
            
//static struct cbm_platform_data *gpt_cbm_platform_data = NULL;

/* ---- Private Function Prototypes -------------------------------------- */
static void cmp_battery_multi_power_off(void);
static bool is_ac_connected            (struct multi_data *pt_multi_data, int ac_gpio_level);
static void multi_power_work           (struct work_struct *p_work);
static void multi_battery_work         (struct work_struct *p_work);
static int  setup_gpios                (struct cbm_platform_data *pt_platform_data);
static void ac_power_isr_handler_work  (struct work_struct *p_work);

/* ---- Public Functions ------------------------------------------------- */

int register_battery_monitor(struct battery_monitor_calls *p_calls,
                             void   *p_data)
{
   struct power_data  *pt_power_data;
   int                 ac_power_gpio_level;
   int                 rc;
   
   printk("%s() called\n", __FUNCTION__);
   
   if (g_is_driver_ok == false)
   {  
      printk("%s() cannot register, problem encountered in battery_probe()\n",
             __FUNCTION__);
      return -1;       
   }
   
   if (gpt_multi_data == NULL)
   {
      printk("%s() called too early, pointer not created\n", __FUNCTION__);
      return -1;
   }

   gpt_multi_data->get_battery_voltage_fn = p_calls->get_voltage_fn;
   gpt_multi_data->get_battery_charge_fn  = p_calls->get_charge_fn;
   
   /* Need to set this pointer pt_multi_data->p_battery_monitor_data. */
   gpt_multi_data->p_battery_monitor_data = p_data;
   
   if (p_calls->type == enum_adc121)
   {
      gpt_multi_data->pt_monitor_info = &gpt_multi_data->pt_platform_data->adc121_info;
      if (mod_debug)
         printk("%s() adc121 battery monitor registered with cmp_battery_multi\n",
                __FUNCTION__); 
   }   
   else 
   if (p_calls->type == enum_max17040)
   {
      gpt_multi_data->pt_monitor_info = &gpt_multi_data->pt_platform_data->max17040_info;
      if (mod_debug)
         printk("%s() max17040 battery monitor registered with cmp_battery_multi\n",
                __FUNCTION__); 
   }   

   if ((rc = setup_gpios(gpt_multi_data->pt_platform_data)) != 0)
   {
      printk("%s() setup_gpios() returned %d\n", __FUNCTION__, rc);
      return -1;
   }
   
   pt_power_data = gpt_multi_data->pt_power_data;	
   
   /* Is the tablet plugged or not? Need to know for wakelock. */
   ac_power_gpio_level        = gpio_get_value(gpt_multi_data->pt_monitor_info->gpio_ac_power);
   pt_power_data->online      = is_ac_connected(gpt_multi_data, ac_power_gpio_level);
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

bool is_monitor_registered(void)
{
   if (gpt_multi_data->pt_monitor_info == NULL)
      return false;
   else
      return true;
}      
                             
static unsigned long read_temp(struct battery_data *p_battery)
{
   unsigned long value = 0;

   //if (bat->adc_temp < 0)
   //   return 20;

   return value;
}

static bool battery_is_present(struct battery_data *p_battery)
{
   (void)p_battery;
   return 1;
}

/* AC power pin level differs between boards. */
static bool is_ac_connected(struct multi_data *pt_multi_data, 
                            int ac_gpio_level)
{
   bool ret;
   
   if (pt_multi_data->pt_monitor_info->ac_power_on_level > 0)
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
static int get_battery_status(struct multi_data *pt_multi_data)
{
   int status;
   struct battery_data *pt_battery = pt_multi_data->pt_battery_data;

   if (pt_battery->is_present && !pt_battery->is_present(pt_battery)) 
   {
      printk(KERN_NOTICE "%s not present\n", pt_battery->t_power_supply.name);
      return POWER_SUPPLY_STATUS_UNKNOWN;
   } 
   
   switch(pt_multi_data->pt_monitor_info->type)
   {
      case enum_adc121:      
         /* if (power_supply_am_i_supplied(pt_battery_psy)) *
          * checks for POWER_SUPPLY_PROP_ONLINE             */
         if (pt_multi_data->pt_power_data->online)
         {
            if (gpio_get_value(pt_battery->gpio_charging_b)) 
            {
               status = POWER_SUPPLY_STATUS_NOT_CHARGING;
            } 
            else 
            {
               status = POWER_SUPPLY_STATUS_CHARGING;
            }
         } 
         else 
         {
            status = POWER_SUPPLY_STATUS_DISCHARGING;
         }
         break;

      case enum_max17040:
         if (pt_multi_data->pt_power_data->online)
         {
            status = POWER_SUPPLY_STATUS_CHARGING;
         } 
         else 
         {
            status = POWER_SUPPLY_STATUS_DISCHARGING;
         }
         break;
         
         
      default:
         status = POWER_SUPPLY_STATUS_UNKNOWN;
   }   
   
   return status;
}

static int cbm_get_battery_charge(struct multi_data *pt_multi_data)
{
   int charge_percent;   
   int current_voltage = pt_multi_data->battery_milliVolts;
   int battery_max     = pt_multi_data->pt_platform_data->battery_max_voltage;
   int battery_min     = pt_multi_data->pt_platform_data->battery_min_voltage;
      
   if (pt_multi_data->get_battery_charge_fn == NULL)
   {  /* No charge API provided, have to calculate it. */
      charge_percent = (current_voltage - battery_min)*100/(battery_max-battery_min);  
   }
   else
   {  /* 
       * Temporary, should be able to retrieve it now rather than use an out of
       *  date value. 
       */
      charge_percent = pt_multi_data->battery_charge;  
   }

   if (charge_percent <= 0 || charge_percent > 100)
   {  /* Need to keep this value in range. */
      if (mod_debug)
         printk("%s() charge out of range %d, setting to 100\n",
                __FUNCTION__, charge_percent);
      charge_percent = 100;
   }         
      
   return charge_percent;
}

/* POWER_SUPPLY_PROP_STATUS,              0 charging, discharging ... */
/* POWER_SUPPLY_PROP_HEALTH,              2 good, overheat, dead ...  */
/* POWER_SUPPLY_PROP_PRESENT,             3                           */
/* POWER_SUPPLY_PROP_TECHNOLOGY,          5 NiMH|LION|...             */ 
/* POWER_SUPPLY_PROP_VOLTAGE_MAX,         7 battery_max               */
/* POWER_SUPPLY_PROP_VOLTAGE_MIN,         8 battery_min               */
/* POWER_SUPPLY_PROP_VOLTAGE_MAX_DESIGN,  9                           */
/* POWER_SUPPLY_PROP_VOLTAGE_MIN_DESIGN,  10                          */
/* POWER_SUPPLY_PROP_VOLTAGE_NOW,         11 battery voltage(mV)      */
/* POWER_SUPPLY_PROP_CAPACITY,            30 Capacity in percent      */
/* POWER_SUPPLY_PROP_TEMP,                32 Degrees celsius          */

static int battery_get_property(struct power_supply *t_power_supply,
                                enum power_supply_property psp,
                                union power_supply_propval *val)
{
   int ret = 0;
   struct battery_data *pt_battery = container_of(t_power_supply, 
                                                   struct battery_data, 
                                                   t_power_supply);

   if (pt_battery->is_present && !pt_battery->is_present(pt_battery)
       && psp != POWER_SUPPLY_PROP_PRESENT) 
   {
      return -ENODEV;
   }
   
   switch (psp) 
   {
      case POWER_SUPPLY_PROP_STATUS:
         val->intval = pt_battery->status;
         break;
      case POWER_SUPPLY_PROP_HEALTH:
         val->intval = POWER_SUPPLY_HEALTH_GOOD;
         break;
      case POWER_SUPPLY_PROP_PRESENT:
         val->intval = pt_battery->is_present ? pt_battery->is_present(pt_battery) : 1;
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
         if (pt_battery->pt_multi_data->get_battery_voltage_fn != NULL)
         {
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
   {
      printk("%s() property: %d val: %d\n", 
             __FUNCTION__, psp, val->intval);
   }
   
   return ret;
}  /* battery_get_property(..) */

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
   
   /* Flush the delayed work. */
   cancel_delayed_work_sync(&pt_multi_data->t_battery_dwork);
   cancel_delayed_work_sync(&pt_multi_data->t_power_dwork);
         
   /* Handle the bounce in the the AC power GPIO value. ac_power_isr_handler_work()
    * will call battery_update() and power_update() once the signal has been
    * debounced.
    */
   queue_work(isr_wq, &pt_multi_data->t_ac_power_work);
   
   return IRQ_HANDLED;
}  /* ac_power_gpio_isr() */

static void battery_update(struct multi_data *pt_multi_data)
{
   int old_battery_status;
   struct battery_data *pt_battery     = pt_multi_data->pt_battery_data;
   struct power_supply  *pt_battery_psy = &pt_battery->t_power_supply;
   
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
      //printk(KERN_INFO "%s %s -> %s\n", pt_battery_psy->name,
      //    status_text[old_battery_status], status_text[pt_battery->status]);
      printk("%s() battery status changed: %d\n", __FUNCTION__, pt_battery->status);
   }

   /* Retrieve the battery voltage from the monitor. */   
   if (pt_multi_data->get_battery_voltage_fn != NULL)
   {   
      battery_value = pt_multi_data->get_battery_voltage_fn(pt_multi_data->p_battery_monitor_data);      
      
      if (pt_multi_data->pt_monitor_info->type == enum_max17040)
      {         
         if (battery_value > 0)
         {
            battery_value = 2*battery_value + 1700;
         }
         else
         {  /* At startup takes a bit of time to get the battery monitor voltage.*/
            battery_value = pt_multi_data->pt_platform_data->battery_max_voltage;
            if (mod_debug)
               printk("%s() monitor not ready, setting voltage to %d\n",
                     __FUNCTION__, battery_value);
         }         
      }   
      
      if (battery_value == 0)
      {  /* Sometimes occurs and may cause tablet to restart. */
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
         printk("%s() battery monitor get_battery_voltage_fn() not configured\n",
                __FUNCTION__);
   }
   
   

   /* Retrieve the battery charge from the monitor. */      
   if (pt_multi_data->get_battery_charge_fn != NULL)
   {      
      if (pt_multi_data->pt_monitor_info->type == enum_max17040)
      {
         battery_value = pt_multi_data->get_battery_charge_fn(pt_multi_data->p_battery_monitor_data);
         pt_multi_data->battery_charge = battery_value;      
      }   
      if (mod_debug)
         printk("%s() battery_charge: %d battery_charge: %d\n", 
                __FUNCTION__, battery_value, pt_multi_data->battery_charge);      
   }            
   else
   {
      if (mod_debug)
         printk("%s() battery monitor get_battery_charge_fn() not configured\n",
                __FUNCTION__);
   }

   power_supply_changed(pt_battery_psy); 
   mutex_unlock(&pt_multi_data->work_lock);
   
}  /* battery_update() */

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
      printk("%s() running, gpio_ac_power: %d\n", __FUNCTION__, pt_multi_data->pt_monitor_info->gpio_ac_power);
   }
   
   gpio_power_level = gpio_get_value(pt_multi_data->pt_monitor_info->gpio_ac_power);
   
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
             __FUNCTION__, pt_multi_data->pt_monitor_info->gpio_ac_power, 
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
   pt_multi_data = container_of(p_work, struct multi_data, t_battery_dwork.work);
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
             pt_multi_data->pt_monitor_info->gpio_ac_power, 
             pt_multi_data->pt_platform_data->gpio_power_control);
   } 
   
   /* Let power_update() call power_supply_changed(..) */      
   power_update(pt_multi_data);    
   schedule_delayed_work(&pt_multi_data->t_power_dwork, MULTI_WAIT_PERIOD);
}

static void ac_power_isr_handler_work(struct work_struct *p_work)
{
   struct multi_data *pt_multi_data;
   int i;
   int real_gpio_val; 
   int gpio_val;
      
   pt_multi_data = container_of(p_work, 
                                struct multi_data, 
                                t_ac_power_work);   
                                
   real_gpio_val = gpio_get_value(pt_multi_data->pt_monitor_info->gpio_ac_power);                                

   if (mod_debug)                       
   {
      printk("running %s() name: %s\n", __FUNCTION__, 
             pt_multi_data->pt_power_data->t_power_supply.name);
   }          
   
   /* While the ISR is triggered on the rising edge the signal has to be
    * debounced before it be known for sure what really happened.
    */  
      
   for (i = 0; i < NUM_DEBOUNCE_TRIES; i++)
   {
      msleep(DEBOUNCE_WAIT_MSECS);
      gpio_val = gpio_get_value(pt_multi_data->pt_monitor_info->gpio_ac_power);
      
      if (gpio_val != real_gpio_val)
      {
         if (mod_debug)                       
         {
            printk("AC power changed, orig %d new: %d \n", 
                real_gpio_val, gpio_val); 
         }       
         real_gpio_val = gpio_val;      
      }
   }
   
   if (is_ac_connected(pt_multi_data, real_gpio_val))
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
   //schedule_work(&bat_work);
}

static void cmp_battery_multi_power_off(void)
{
   gpio_set_value(g_gpio_power_control, 0);
}


static int setup_gpios(struct cbm_platform_data *pt_platform_data)
{
   int rc_req, rc_dir;
   struct multi_data *pt_multi_data  = (struct multi_data *)pt_platform_data->p_cbm_data;
   
   if (mod_debug)
   {
      printk("%s() power down gpio: %d\n", __FUNCTION__, pt_platform_data->gpio_power_control);
      printk("%s() ac power gpio  : %d\n", __FUNCTION__, pt_multi_data->pt_monitor_info->gpio_ac_power); 
      printk("%s() charger gpio   : %d\n", __FUNCTION__, pt_multi_data->pt_monitor_info->gpio_charger); 
   }

   if (pt_platform_data->gpio_power_control >= 0)
   {   
      rc_req = gpio_request(pt_platform_data->gpio_power_control, "switch off");
      rc_dir = gpio_direction_output(pt_platform_data->gpio_power_control, 1);
      
      if (rc_req || rc_dir)
      {
         gpio_free(pt_platform_data->gpio_power_control);
         return -1;
      }
   }      

   if (pt_multi_data->pt_monitor_info->gpio_ac_power >= 0)
   {
      rc_req = gpio_request(pt_multi_data->pt_monitor_info->gpio_ac_power, "ac power");
      rc_dir = gpio_direction_input(pt_multi_data->pt_monitor_info->gpio_ac_power);
      
      if (rc_req || rc_dir)
      {
         gpio_free(pt_multi_data->pt_monitor_info->gpio_ac_power);
         return -1;
      }   
   }   
   
   if (pt_multi_data->pt_monitor_info->gpio_charger >= 0)
   {
      rc_req = gpio_request(pt_multi_data->pt_monitor_info->gpio_charger , "charger");
      rc_dir = gpio_direction_input(pt_multi_data->pt_monitor_info->gpio_charger);
      
      if (rc_req || rc_dir)
      {
         gpio_free(pt_multi_data->pt_monitor_info->gpio_charger);
         return -1;
      }
   }
   
   /* Needed to turn off the device. */   
   g_gpio_power_control = pt_platform_data->gpio_power_control;
   
   return 0;
}

static void close_gpios(struct cbm_platform_data *pt_platform_data)
{
   struct multi_data *pt_multi_data  = (struct multi_data *)pt_platform_data->p_cbm_data;
   
   if (pt_platform_data->gpio_power_control >= 0)
      gpio_free(pt_platform_data->gpio_power_control);
      
   if (pt_multi_data->pt_monitor_info->gpio_ac_power >= 0)  
      gpio_free(pt_multi_data->pt_monitor_info->gpio_ac_power);
      
   if (pt_multi_data->pt_monitor_info->gpio_charger >= 0)   
      gpio_free(pt_multi_data->pt_monitor_info->gpio_charger);
}

#ifdef CONFIG_PM
static int battery_suspend(struct platform_device *p_dev, pm_message_t state)
{
   struct cbm_platform_data *pt_data = (struct cbm_platform_data *)p_dev->dev.platform_data;
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
   struct cbm_platform_data *pt_data = (struct cbm_platform_data *)p_dev->dev.platform_data;
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
   int i;
   int ac_power_gpio_level = 0;
   struct multi_data    *pt_multi_data;
   struct battery_data  *pt_battery_data;
   struct power_data    *pt_power_data;
   /* CBM (C)mp (B)attery (M)ulti. */
   struct cbm_platform_data *pt_cbm_platform_data;
   long data_location = 0;

   printk(gBanner);

   if (p_dev->dev.platform_data == NULL)
   {  /* Need this information. */
      printk("%s() error p_dev->dev.platform_data == NULL\n", __FUNCTION__);
      return -1;
   }
   
   pt_cbm_platform_data = (struct cbm_platform_data *)p_dev->dev.platform_data;
   
   if (mod_debug)
   {
      printk("battery max voltage: %d\n", pt_cbm_platform_data->battery_max_voltage);    
      printk("battery min voltage: %d\n", pt_cbm_platform_data->battery_min_voltage);    
      printk("pt_cbm_platform_data->type 0: %d\n", pt_cbm_platform_data->adc121_info.type);    
      printk("pt_cbm_platform_data->type 1: %d\n", pt_cbm_platform_data->max17040_info.type);    
      printk("pt_cbm_platform_data->gpio_ac_power: %d\n", pt_cbm_platform_data->gpio_ac_power);    
   }  

   /* Retrieve and assign the platform data. */
   gt_battery_info.battery_max = pt_cbm_platform_data->battery_max_voltage;   
   gt_battery_info.full_chrg = pt_cbm_platform_data->battery_max_voltage;   
   gt_battery_info.battery_min = pt_cbm_platform_data->battery_min_voltage;   
   gt_battery_info.battery_technology = pt_cbm_platform_data->battery_technology;
   gt_battery_info.gpio_charging_b = pt_cbm_platform_data->gpio_charging;

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
   pm_power_off = cmp_battery_multi_power_off;

   isr_wq = create_workqueue("multi_isr_wq");
      
   /* Setup the interrupt to detect if external power is plugged in or not. 
    * IRQF_TRIGGER_RISING  - external power is connected or disconnected
    * IRQF_TRIGGER_FALLING - external power is disconnected or connected
    * Unfortunately the signal bounces up and down before settling in either
    * case.
    */
   msleep(INT_SETUP_SLEEP_MSECS);

   /* Detect when AC is plugged in or removed. */
   ret = request_irq(gpio_to_irq(pt_cbm_platform_data->gpio_ac_power),
                     ac_power_gpio_isr,
                     IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
                     "ac detect", pt_multi_data);
   if (ret)
   {
      printk("%s():  request_irq(gpio_to_irq(%d)) returned %d\n",
             __FUNCTION__, pt_multi_data->pt_monitor_info->gpio_ac_power, ret);  
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

   if (ret == 0) 
   {   
      g_is_driver_ok = true;
      return 0;
   }

   wake_lock_destroy(&pt_multi_data->wakelock);
      
   power_supply_unregister(&pt_multi_data->pt_battery_data->t_power_supply);
      err_psy_reg_power:
   power_supply_unregister(&pt_multi_data->pt_power_data->t_power_supply);
      err_psy_reg_battery:

   /* see comment in battery_remove */
   flush_scheduled_work();
   flush_workqueue(isr_wq);
   destroy_workqueue(isr_wq);
   
   i--;
      err_gpio:
   pm_power_off = NULL;

   close_gpios(pt_cbm_platform_data);   

   printk("%s(): return %d\n", __FUNCTION__, ret);
   return ret;
}  /* battery_probe() */

static int __devexit battery_remove(struct platform_device *p_dev)
{
   struct cbm_platform_data *pt_data = (struct cbm_platform_data *)p_dev->dev.platform_data;
   struct multi_data *pt_multi_data  = (struct multi_data *)pt_data->p_cbm_data;

   printk("%s() entering ...\n", __FUNCTION__);   
   
   //free_irq(gpio_to_irq(pt_multi_data->pt_monitor_info->gpio_ac_power), &gt_power_info); 

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
   /* Flush the delayed work. */
   cancel_delayed_work_sync(&pt_multi_data->t_battery_dwork);
   cancel_delayed_work_sync(&pt_multi_data->t_power_dwork);
   
   pm_power_off = NULL;

   close_gpios(pt_data);

   wake_lock_destroy(&pt_multi_data->wakelock);
   mutex_destroy(&pt_multi_data->work_lock);

   printk("%s() exiting ...\n", __FUNCTION__);   
   msleep(INT_SETUP_SLEEP_MSECS);
   
   return 0;
}

static struct platform_driver battery_driver = 
{
   .driver.name = "cmp-battery",
   .driver.owner = THIS_MODULE,
   .probe = battery_probe,
   .remove = __devexit_p(battery_remove),
#ifdef CONFIG_PM
   .suspend = battery_suspend,
   .resume = battery_resume,
#endif
};

static struct platform_device *battery_platform_device = NULL;

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
   if (battery_platform_device) 
   {
      platform_device_del(battery_platform_device);
      platform_device_put(battery_platform_device);
   }
   platform_driver_unregister(&battery_driver);
}

module_init(battery_init);
module_exit(battery_exit);

MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("Broadcom CMP Battery Multi Driver");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.01");
