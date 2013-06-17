/*****************************************************************************
* Copyright 2006 - 2011 Broadcom Corporation.  All rights reserved.
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
#include <linux/timer.h>
#include <linux/jiffies.h>
#include <linux/uaccess.h>
#include <linux/proc_fs.h>
#include <linux/broadcom/cmp_battery_bq24616.h>

#if defined(CONFIG_MONITOR_BQ27541_I2C) || defined(CONFIG_MONITOR_BQ27541_I2C_MODULE)
#include <linux/broadcom/bq27541.h>
#endif

/************
 * CONSTANTS
 ************/

/* Wake lock flags */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 27)) && defined(CONFIG_HAS_WAKELOCK)
#include <linux/wakelock.h>
#define USE_WAKELOCK               1
#else
#define USE_WAKELOCK               0
#endif

/*
 * Time constants for scheduling when to poll the battery monitor device upon
 * system start up and interrupt detected
 */
#define DELAY_POLL_INTERVAL        (2 * HZ)
/* Interval to poll after system has stabilized */
#define POLL_INTERVAL              (60 * HZ)

/* Constant used letting the system to stabilize during initialization */
#define INT_SETUP_SLEEP_MSECS      50

/* Constants used for debouncing the AC connection input signal */
#define NUM_DEBOUNCE_TRIES         10
#define NUM_TRIES_TO_STABILIZE     5
#define DEBOUNCE_WAIT_MSECS        50

/* Constants for procfs */
#define MAX_PROC_BUF_SIZE         256
#define MAX_PROC_NAME_SIZE        15
#define PROC_GLOBAL_PARENT_DIR    "battery"  /* parent directory */
#define PROC_ENTRY_DEBUG          "debug"    /* procfs for enabling debug log.
                                                Writing "1" enables debug log,
                                                and "0" will disable it.*/
#define PROC_ENTRY_STATUS         "status"   /* procfs for querying battery.
                                                status.  Reading it will print
                                                all battery status the driver
                                                supports. */

#if defined(CONFIG_MONITOR_BQ27541_I2C) || defined(CONFIG_MONITOR_BQ27541_I2C_MODULE)
/* macros for constructing the bq27541_device_data elements (defined below */
#define BQ27541_DATA(_psp, _addr, _min_value, _max_value, _name) { \
        .psp = _psp, \
        .addr = _addr, \
        .min_value = _min_value, \
        .max_value = _max_value, \
        .name = _name, \
}
#endif

/********
 * ENUMS
 ********/

/* Index for referencing registers in the battery monitor device */
enum {
        REG_TEMPERATURE,
        REG_VOLTAGE,
        REG_FULL_CHARGE_AVAILABLE,
        REG_REMAINING_CAPACITY,
        REG_FULL_CHARGE_CAPACITY,
        REG_CURRENT,
        REG_TIME_TO_EMPTY,
        REG_TIME_TO_FULL,
        REG_AVAILABLE_ENERGY,
        REG_AVERAGE_POWER,
        REG_CYCLE_COUNT,
        REG_CAPACITY,
        REG_FLAGS,
};

/* power supply property supported by the battery driver */
static enum power_supply_property battery_main_props[] = {
        POWER_SUPPLY_PROP_TECHNOLOGY,
        POWER_SUPPLY_PROP_HEALTH,
        POWER_SUPPLY_PROP_TEMP,
        POWER_SUPPLY_PROP_VOLTAGE_NOW,
        POWER_SUPPLY_PROP_PRESENT,
        POWER_SUPPLY_PROP_STATUS,
        POWER_SUPPLY_PROP_CHARGE_FULL_DESIGN,
        POWER_SUPPLY_PROP_CHARGE_NOW,
        POWER_SUPPLY_PROP_CHARGE_FULL,
        POWER_SUPPLY_PROP_CURRENT_AVG,
        POWER_SUPPLY_PROP_TIME_TO_EMPTY_NOW,
        POWER_SUPPLY_PROP_TIME_TO_FULL_NOW,
        POWER_SUPPLY_PROP_ENERGY_NOW,
        POWER_SUPPLY_PROP_POWER_AVG,
        POWER_SUPPLY_PROP_CYCLE_COUNT,
        POWER_SUPPLY_PROP_CAPACITY,
};

/* power supply property supported by the power driver */
static enum power_supply_property power_props[] = {
        POWER_SUPPLY_PROP_ONLINE,
};


/*************
 * STRUCTURES
 *************/

/* local structure for storing power related variables */
struct power {
        int online;
        struct power_supply psy;
        struct mutex work_lock;   /* protects data */
        int gpio_dok_b;
};

/* local structure for storing battery related variables */
struct battery {
        struct power_supply psy;
        struct mutex work_lock;   /* protects data */
        int technology;
};

/* structure and declaration for the GPIO pins used by this driver */
/* using standard kernel gpio struct and array handling */
static struct gpio gpios[] = {
        /* Output GPIO: controls battery power output. 
           HIGH = letting power through, LOW = cutting power off. */
        {-1, GPIOF_OUT_INIT_HIGH, "battery power output on/off"},
        /* Input GPIO: AC power present/shutdown. HIGH = AC power removed. */
        {-1, GPIOF_IN, "ac present/shutdown"},
        /* Until proper solution for docking station is found, cp docking
           station power is handled here */
        /* Output GPIO: controls docking station 3.3v power. 
           HIGH = letting power through, LOW = cutting power off. */
        {-1, GPIOF_OUT_INIT_HIGH, "docking station power 3.3v on/off"},
};

/* GPIO indexes. Allow access GPIO PIN values from "struct gpio
   gpios[]". Must be in sync with gpios initialization */ 
#define CTL_PW_INDEX 0
#define ACP_SHD_INDEX 1
#define DOCKING_STATION_POWER_INDEX 2

#if defined(CONFIG_MONITOR_BQ27541_I2C) || defined(CONFIG_MONITOR_BQ27541_I2C_MODULE)
/* structure and declaration for the mapping between the BQ27541 commands
 * and the power supply properties supported by this driver */
static const struct bq27541_device_data {
        enum power_supply_property psp;
        u8 addr;
        int min_value;
        int max_value;
        char *name;
} bq27541_data[] = {
        [REG_TEMPERATURE] =
                BQ27541_DATA(POWER_SUPPLY_PROP_TEMP,
                             BQ27541_CMD_TEMP,
                             0, 65535, "TEMP"),
        [REG_VOLTAGE] =
                BQ27541_DATA(POWER_SUPPLY_PROP_VOLTAGE_NOW,
                             BQ27541_CMD_VOLT,
                             0, 6000, "VOLT"),
        [REG_FULL_CHARGE_AVAILABLE] =
                BQ27541_DATA(POWER_SUPPLY_PROP_CHARGE_FULL_DESIGN,
                             BQ27541_CMD_FAC,
                             0, 65535, "FULL_CHARGE_AVAILABLE"),
        [REG_REMAINING_CAPACITY] =
                BQ27541_DATA(POWER_SUPPLY_PROP_CHARGE_NOW,
                             BQ27541_CMD_RM,
                             0, 65535, "REMAINING_CAPACITY"),
        [REG_FULL_CHARGE_CAPACITY] =
                BQ27541_DATA(POWER_SUPPLY_PROP_CHARGE_FULL,
                             BQ27541_CMD_FCC,
                             0, 65535, "FULL_CHARGE_CAPACITY"),
        [REG_CURRENT] =
                BQ27541_DATA(POWER_SUPPLY_PROP_CURRENT_AVG,
                             BQ27541_CMD_AI,
                             -32768, 32767, "CURRENT"),
        [REG_TIME_TO_EMPTY] =
                BQ27541_DATA(POWER_SUPPLY_PROP_TIME_TO_EMPTY_NOW,
                             BQ27541_CMD_TTE,
                             0, 65535, "TIME_TO_EMPTY"),
        [REG_TIME_TO_FULL] =
                BQ27541_DATA(POWER_SUPPLY_PROP_TIME_TO_FULL_NOW,
                             BQ27541_CMD_TTF,
                             0, 65535, "TIME_TO_FULL"),
        [REG_AVAILABLE_ENERGY] =
                BQ27541_DATA(POWER_SUPPLY_PROP_ENERGY_NOW,
                             BQ27541_CMD_AE,
                             0, 65535, "AVAILABLE_ENERGY"),
        [REG_AVERAGE_POWER] =
                BQ27541_DATA(POWER_SUPPLY_PROP_POWER_AVG,
                             BQ27541_CMD_AP,
                             0, 65535, "AVERAGE_POWER"),
        [REG_CYCLE_COUNT] =
                BQ27541_DATA(POWER_SUPPLY_PROP_CYCLE_COUNT,
                             BQ27541_CMD_CC,
                             0, 65535, "CYCLE_COUNT"),
        [REG_CAPACITY] =
                BQ27541_DATA(POWER_SUPPLY_PROP_CAPACITY,
                             BQ27541_CMD_SOC,
                             0, 100, "CAPACITY"),
        [REG_FLAGS] =
                BQ27541_DATA(POWER_SUPPLY_PROP_STATUS,
                             BQ27541_CMD_FLAGS,
                             0, 65535, "FLAGS"),
};
#endif

/********************
 * PRIVATE VARIABLES
 ********************/

 /* driver's banner */
static const __devinitconst char gBanner[] =
        KERN_INFO "CMP BQ24616 Battery Driver: 1.00\n";

static struct battery battery_main;  /* private battery structure */
static struct power power_dc;        /* private power structure */

/* function for cutting power from the battery/AC */
static void bcmring_power_off(void);

/* workqueue and work task for polling battery status */
static struct workqueue_struct *battery_status_wq;
static void battery_work(struct work_struct *work);
DECLARE_DELAYED_WORK(battery_status_task, battery_work);

/* workqueue and work task for debouncing power input signal */
static struct workqueue_struct *power_status_wq;
static void power_work(struct work_struct *work);
DECLARE_DELAYED_WORK(power_status_task, power_work);

/* procfs directory and entries */
static struct proc_dir_entry *proc_parent;
static struct proc_dir_entry *proc_debug;
static struct proc_dir_entry *proc_status;

/* debug flag.  By default, debug log is disabled */
static int mod_debug = 0;

/* wake lock declaration.  Wake lock is used for keeping the system
 * from going into power saving mode */
#if USE_WAKELOCK
static struct wake_lock wakelock;
const char   *wake_lock_name = "bq24616_battery_driver";
#endif


/************
 * FUNCTIONS
 ************/

/*** Power related routines ***/

 /*
  * Function that returns all power related properties. Currently
  * only one property, POWER_SUPPLY_PROP_ONLINE which indicates if
  * AC power is supplied or not, is supported.
  */
static int power_get_property(struct power_supply *psy,
                              enum power_supply_property psp,
                              union power_supply_propval *val)
{
        struct power *pow = container_of(psy, struct power, psy);

        switch (psp) {
        case POWER_SUPPLY_PROP_ONLINE:
                val->intval = pow->online;
                break;
        default:
                return -EINVAL;
        }

        if (mod_debug) {
                printk("%s(%2d) = %d\n", __FUNCTION__, psp, val->intval);
        }

        return 0;
}

 /*
  * Interrupt service routine for responding to the power input
  * detection/removal signal.  Since the power detection signal bounces
  * when inserting/removing, the ISR queues a task into a specific
  * workqueue for handling the debouncing in a non-interrupt context.
  */
static irqreturn_t power_gpio_isr(int irq, void *data)
{
#if USE_WAKELOCK
        wake_lock(&wakelock);
#endif

        /* The GPIO signal requires debouncing, schedule a work task
           to respond to the interrupt in non-interrupt context */
        queue_delayed_work(power_status_wq, &power_status_task, 0);

        return IRQ_HANDLED;
}

 /*
  * Function for detecting if the power input signal has changed (after
  * being debounced).  If the state of the signal changed, it would schedule
  * the battery_work work task to notify upper layer that the signal has
  * changed. Note that the function takes in the raw GPIO value, which is
  * an active low signal.
  */
static void power_update(int gpio_val)
{
        int old_power_online;

        mutex_lock(&power_dc.work_lock);

        old_power_online = power_dc.online;

        /* the gpio for detecting the presence of AC is an active low signal,
           hence, reverse its polarity here */
        if (gpio_val) {
                power_dc.online = 0;
        } else {
                power_dc.online = 1;
        }

        /* AC removal/insertion detected */
        if (old_power_online != power_dc.online) {
                if (power_dc.online) {
#if USE_WAKELOCK
                        /* The tablet has been applied with external power.*/
                        wake_lock(&wakelock);
#endif
                        if (mod_debug) {
                                printk("AC applied to tablet\n");
                        }
                } else {
#if USE_WAKELOCK
                        /* The tablet has been unplugged from external power.*/
                        wake_unlock(&wakelock);
#endif
                        if (mod_debug) {
                                printk("AC removed from tablet\n");
                        }
                }
                /* notify upper layers that the power state has changed */
                if (mod_debug) {
                        printk("NEW AC status is: %d\n", power_dc.online);
                }
                /* since it takes a bit of time for the battery monitoring
                   device to realize that AC is removed/inserted, hence
                   delay the notification by DELAY_POLL_INTERVAL */
                cancel_delayed_work(&battery_status_task);
                queue_delayed_work(battery_status_wq,
                                   &battery_status_task,
                                   DELAY_POLL_INTERVAL);
        }

        mutex_unlock(&power_dc.work_lock);
}

/*
 * Work task for debouncing the power insertion/removal input signal.  Note
 * that since this work task has to sleep for debouncing, it is preferrable
 * run this work task on its own work queue.  For this reason, a workqueue
 * called power_status_wq is specifically created for this work task.
 */
static void power_work(struct work_struct *work)
{
        int i = 0, j = 0;
        int prev_gpio_val = gpio_get_value(gpios[ACP_SHD_INDEX].gpio);
        int curr_gpio_val;

        do {
                msleep(DEBOUNCE_WAIT_MSECS);
                curr_gpio_val = gpio_get_value(gpios[ACP_SHD_INDEX].gpio);
                if (prev_gpio_val != curr_gpio_val) {
                        prev_gpio_val = curr_gpio_val;
                        j = 0;
                } else {
                        j++;
                        if (j == NUM_TRIES_TO_STABILIZE) {
                                /* signal appears to be stabilized.
                                   notify upper layer if the signal
                                   state has changed since last
                                   notification */
                                power_update(curr_gpio_val);
                                break;
                        }
                }
                i++;
        } while (i < NUM_DEBOUNCE_TRIES);

        if (i >= NUM_DEBOUNCE_TRIES) {
                printk(KERN_NOTICE "%s cannot detect a stabilized signal\n",
                       __FUNCTION__);
        }
}

/* structure declaration for the power driver */
static char *power_supplied_to[] = {
        "battery", /* name of the device that is dependent of
                      this power driver. */
};
static struct power power_dc = {
        .online = 0,
        .psy = {
                .name = "dc",
                .type = POWER_SUPPLY_TYPE_MAINS,
                .supplied_to = power_supplied_to,
                .num_supplicants = ARRAY_SIZE(power_supplied_to),
                .properties = power_props,
                .num_properties = ARRAY_SIZE(power_props),
                .get_property = power_get_property,
        },
};

/*** Battery related routines ***/

/*
 * Routine for cutting power to the platform
 */
static void bcmring_power_off(void)
{
        gpio_set_value(gpios[CTL_PW_INDEX].gpio, 0);
}

/*
 * Work task for notifying and polling battery status to upper layers
 */
static void battery_work(struct work_struct *work)
{
        /* periodically notify upper layers to query battery's status */
        power_supply_changed(&(power_dc.psy));

        /* schedule the next notification */
        queue_delayed_work(battery_status_wq,
                           &battery_status_task,
                           POLL_INTERVAL);

}

/*
 * Generic API for issuing i2c command to BQ27541 for the given
 * power supply property.
 */
static int get_data(enum power_supply_property psp,
                    int *value)
{
        int ret = 0;
#if defined(CONFIG_MONITOR_BQ27541_I2C) || defined(CONFIG_MONITOR_BQ27541_I2C_MODULE)
        int count;
        for (count = 0; count < ARRAY_SIZE(bq27541_data); count++) {
                if (psp == bq27541_data[count].psp)
                        break;
        }
        ret = i2c_bq27541_cmd(bq27541_data[count].addr, value);
        if (!ret) {
                if (*value > bq27541_data[count].max_value) {
                        *value = bq27541_data[count].max_value;
                } else if (*value < bq27541_data[count].min_value) {
                        *value = bq27541_data[count].min_value;
                }
                if (mod_debug) {
                        printk("%-25s: %5d (0x%04x)\n",
                               bq27541_data[count].name, *value, *value);
                }
        }
#endif
        return ret;
}

/*
 * This function reads the specified temperature related power supply property
 * from BQ27541 and convert the returned value from Kelvin to Celsius.
 */
static int get_temp(enum power_supply_property psp,
                    int *value)
{
        int ret = 0;
        *value = 20;
#if defined(CONFIG_MONITOR_BQ27541_I2C) || defined(CONFIG_MONITOR_BQ27541_I2C_MODULE)
        ret = get_data(psp, value);
        if (!ret) {
                /* convert Kelvin to Celsius */
                *value = ((*value * 10) - 27315) / 10;
        }
#endif
        return ret;
}

/*
 * This function reads the requested time related power supply property
 * from BQ27541 and convert the returned value from minute to 0.1 second.
 */
static int get_times(enum power_supply_property psp,
                     int *value)
{
        int ret = 0;
        *value = 65535;
#if defined(CONFIG_MONITOR_BQ27541_I2C) || defined(CONFIG_MONITOR_BQ27541_I2C_MODULE)
        ret = get_data(psp, value);
        if (!ret) {
                /* convert minute to 0.1 second */
                *value *= 600;
        }
#endif
        return ret;
}

/*
 * This function reads the given power supply property from
 * BQ27541 and scale the returned value by the given "multipler" argument.
 */
static int get_properties(enum power_supply_property psp,
                          int *value, int multiplier)
{
        int ret = 0;
        *value = 65535;
#if defined(CONFIG_MONITOR_BQ27541_I2C) || defined(CONFIG_MONITOR_BQ27541_I2C_MODULE)
        ret = get_data(psp, value);
        if (!ret) {
                *value *= multiplier;
        }
#endif
        return ret;
}

/*
 * This function reads the requested % related power supply property
 * from BQ27541.
 */
static int get_percents(enum power_supply_property psp,
                        int *value)
{
        int ret = 0;
        *value = 100;
#if defined(CONFIG_MONITOR_BQ27541_I2C) || defined(CONFIG_MONITOR_BQ27541_I2C_MODULE)
        ret = get_data(psp, value);
#endif
        return ret;
}

/*
 * This function issues the corresponding i2c commands to BQ27541 for
 * intepreting the health or status of the battery or power supply, respectively.
 */
static int get_health_and_status(enum power_supply_property psp,
                                 int *value)
{
        int ret = 0;
#if defined(CONFIG_MONITOR_BQ27541_I2C) || defined(CONFIG_MONITOR_BQ27541_I2C_MODULE)
        int flags, ttf;
        ret = i2c_bq27541_cmd(BQ27541_CMD_FLAGS, &flags);
        if (!ret) {
                if (psp == POWER_SUPPLY_PROP_HEALTH) {
                        if (flags & 0xC000) {
                                *value = POWER_SUPPLY_HEALTH_OVERHEAT;
                        } else if (flags & 0x0C00) {
                                *value = POWER_SUPPLY_HEALTH_UNSPEC_FAILURE;
                        } else {
                                *value = POWER_SUPPLY_HEALTH_GOOD;
                        }
                } else if (psp == POWER_SUPPLY_PROP_STATUS) {
                        if (flags & 0x0200) {
                                *value = POWER_SUPPLY_STATUS_FULL;
                        } else if ((flags & 0x0001) && !power_dc.online) {
                                *value = POWER_SUPPLY_STATUS_DISCHARGING;
                        } else {
                                ret = i2c_bq27541_cmd(BQ27541_CMD_TTF, &ttf);
                                if (!ret) {
                                        if (ttf == 65535) {
                                                *value = POWER_SUPPLY_STATUS_NOT_CHARGING;
                                        } else {
                                                *value = POWER_SUPPLY_STATUS_CHARGING;
                                        }
                                }
                        }
                }
        }
#else
        if (psp == POWER_SUPPLY_PROP_HEALTH) {
                *value = POWER_SUPPLY_HEALTH_GOOD;
        } else if (psp == POWER_SUPPLY_PROP_STATUS) {
                *value = POWER_SUPPLY_STATUS_FULL;
        }
#endif
        return ret;
}

 /*
  * Function that returns all battery related properties.
  */
static int battery_get_property(struct power_supply *psy,
                                enum power_supply_property psp,
                                union power_supply_propval *val)
{
        int ret = 0;
        struct battery *bat = container_of(psy, struct battery, psy);

        mutex_lock(&bat->work_lock);

        switch (psp) {
        case POWER_SUPPLY_PROP_TECHNOLOGY:
                val->intval = bat->technology;
                break;
        case POWER_SUPPLY_PROP_HEALTH:
        case POWER_SUPPLY_PROP_STATUS:
                ret = get_health_and_status(psp, &(val->intval));
                break;
        case POWER_SUPPLY_PROP_TEMP:
                ret = get_temp(psp, &(val->intval));
                break;
        case POWER_SUPPLY_PROP_TIME_TO_EMPTY_NOW:
        case POWER_SUPPLY_PROP_TIME_TO_FULL_NOW:
                ret = get_times(psp, &(val->intval));
                break;
        case POWER_SUPPLY_PROP_CAPACITY:
                ret = get_percents(psp, &(val->intval));
                break;
        case POWER_SUPPLY_PROP_CHARGE_FULL_DESIGN:
        case POWER_SUPPLY_PROP_CHARGE_NOW:
        case POWER_SUPPLY_PROP_CHARGE_FULL:
        case POWER_SUPPLY_PROP_VOLTAGE_NOW:
        case POWER_SUPPLY_PROP_CURRENT_AVG:
        case POWER_SUPPLY_PROP_POWER_AVG:
        case POWER_SUPPLY_PROP_ENERGY_NOW:
                ret = get_properties(psp, &(val->intval), 1000);
                break;
        case POWER_SUPPLY_PROP_CYCLE_COUNT:
                ret = get_properties(psp, &(val->intval), 1);
                break;
        case POWER_SUPPLY_PROP_PRESENT:
                val->intval = 1; /* Assume that the battery is always there
                                  * This assumption is valid for tablet */
                break;
        default:
                ret = -EINVAL;
                break;
        }
        mutex_unlock(&bat->work_lock);

        if (mod_debug && !ret) {
                printk("%s(%2d) = %d\n", __FUNCTION__, psp, val->intval);
        }

        return ret;
}

/* structure declaration for the battery driver */
static struct battery battery_main = {
        .psy = {
                .name = "battery",
                .type = POWER_SUPPLY_TYPE_BATTERY,
                .properties = battery_main_props,
                .num_properties = ARRAY_SIZE(battery_main_props),
                .get_property = battery_get_property,
                .use_for_apm = 1,
        },
        .technology = POWER_SUPPLY_TECHNOLOGY_LION,
};

/*** Kernel registration routines ***/

/*
 * Functions for the "/proc/battery/debug" procfs.
 * Writing "1" to this procfs enables kernel debug log.
 * Writing "0" to this procfs disables kernel debug log.
 * Reading from this procfs returns the current state of the debug flag.
 */
static int proc_debug_read(char *buffer, char **start, off_t off, int count,
                           int *eof, void *data)
{
        unsigned int len = 0;

        if (off > 0)
                return 0;

        len += sprintf(buffer + len, "Debug print is %s\n",
                       mod_debug ? "enabled" : "disabled");

        return len;
}
static int proc_debug_write(struct file *file, const char __user *buffer,
                            unsigned long count, void *data)
{
        int rc;
        unsigned int debug;
        unsigned char kbuf[MAX_PROC_BUF_SIZE];

        if (count > MAX_PROC_BUF_SIZE) {
                count = MAX_PROC_BUF_SIZE;
        }

        rc = copy_from_user(kbuf, buffer, count);
        if (rc) {
                printk(KERN_ERR "copy_from_user failed status=%d", rc);
                return -EFAULT;
        }

        if (sscanf(kbuf, "%u", &debug) != 1) {
                printk(KERN_ERR "echo <debug> > %s\n", PROC_ENTRY_DEBUG);
                return count;
        }

        if (debug)
                mod_debug = 1;
        else
                mod_debug = 0;

        return count;
}

/*
 * Functions for the "/proc/battery/status" procfs.
 * Reading from this procfs triggers the driver to query all battery properties
 * supported by this driver.
 */
static int proc_status_read(char *buffer, char **start, off_t off, int count,
                           int *eof, void *data)
{
        unsigned int len = 0, i;
        int value;

        if (off > 0)
                return 0;

#if defined(CONFIG_MONITOR_BQ27541_I2C) || defined(CONFIG_MONITOR_BQ27541_I2C_MODULE)
        mutex_lock(&battery_main.work_lock);

        for (i = 0; i < ARRAY_SIZE(bq27541_data); i++) {
                get_data(bq27541_data[i].psp, &value);
                len += sprintf(buffer + len, "%-25s: %5d (0x%04x)\n",
                               bq27541_data[i].name, value, value);
        }

        mutex_unlock(&battery_main.work_lock);
#endif

        return len;
}

#ifdef CONFIG_PM
/*
 * Power management functions that get invoked when suspending to and resuming
 * from power saving mode.
 */
static int battery_suspend(struct platform_device *dev,
                           pm_message_t state)
{
        /* cancel and flush all pending status updates */
        cancel_delayed_work(&battery_status_task);
        cancel_delayed_work(&power_status_task);
        flush_workqueue(battery_status_wq);
        flush_workqueue(power_status_wq);
        return 0;
}

static int battery_resume(struct platform_device *dev)
{
        /* reschedule battery monitoring and notify upper layers to
           query power state upon resume */
        queue_delayed_work(battery_status_wq, &battery_status_task, 0);
        queue_delayed_work(power_status_wq, &power_status_task, 0);
        return 0;
}
#endif

static int __devinit battery_probe(struct platform_device *dev)
{
        int ret;
        int i;
        struct battery_bq24616_cfg *bq24616_cfg;

        printk(gBanner);

        if (dev->dev.platform_data == NULL) { 
            /* platform data must be present for the driver to operate correctly */
            printk("%s() error dev->dev.platform_data == NULL\n", __FUNCTION__);
            return -1;
        }

        bq24616_cfg = (struct battery_bq24616_cfg *)dev->dev.platform_data;

        /* set gpio pin values in gpios array */
        gpios[CTL_PW_INDEX].gpio = bq24616_cfg->gpio_ctl_pwr;
        gpios[ACP_SHD_INDEX].gpio = bq24616_cfg->gpio_acp_shd;
        gpios[DOCKING_STATION_POWER_INDEX].gpio = bq24616_cfg->gpio_docking_station_power_3_3v;

        ret = gpio_request_array(gpios, ARRAY_SIZE(gpios));
        if (ret) {
                goto err_gpio;
        }

        pm_power_off = bcmring_power_off;

        mutex_init(&battery_main.work_lock);
        mutex_init(&power_dc.work_lock);

        battery_status_wq = create_workqueue("bq24616_battery_wq");
        power_status_wq = create_workqueue("bq24616_power_wq");

        ret = power_supply_register(&dev->dev, &power_dc.psy);
        if (ret) {
                goto err_psy_reg_dc;
        }
        ret = power_supply_register(&dev->dev, &battery_main.psy);
        if (ret) {
                goto err_psy_reg_main;
        }

        /*
         * Setup the interrupt to detect if external power is plugged in or not.
         * IRQF_TRIGGER_FALLING  - external power is connected
         * IRQF_TRIGGER_RISING   - external power is disconnected
         */
        msleep(INT_SETUP_SLEEP_MSECS);
        /* LOW = AC power applied, HIGH = AC power removed. */
        ret = request_irq(gpio_to_irq(gpios[ACP_SHD_INDEX].gpio),
                          power_gpio_isr,
                          IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
                          "dc detect", &power_dc);
        if (ret) {
                printk("%s():  request_irq failed and returned %d\n",
                       __FUNCTION__, ret);
                goto err_reverse_all_regs;
        }

        proc_parent = proc_mkdir(PROC_GLOBAL_PARENT_DIR, NULL);
        if (proc_parent == NULL) {
                printk(KERN_ERR "battery driver procfs failed\n");
                ret = -ENOMEM;
                goto err_reverse_all_regs;
        }

        proc_debug = create_proc_entry(PROC_ENTRY_DEBUG, 0644, proc_parent);
        if (proc_debug == NULL) {
                printk(KERN_ERR "battery/debug driver procfs failed\n");
                ret = -ENOMEM;
                goto err_remove_proc_parent;
        }
        proc_debug->read_proc = proc_debug_read;
        proc_debug->write_proc = proc_debug_write;

        proc_status = create_proc_entry(PROC_ENTRY_STATUS, 0644, proc_parent);
        if (proc_status == NULL) {
                printk(KERN_ERR "battery/status driver procfs failed\n");
                ret = -ENOMEM;
                goto err_remove_proc_debug;
        }
        proc_status->read_proc = proc_status_read;
        
#if USE_WAKELOCK
        /* Create a wake lock for keeping the tablet awake
           when AC is plugged in */
        wake_lock_init(&wakelock, WAKE_LOCK_SUSPEND, wake_lock_name);
#endif

        /* retrieve initial battery and power status and signal upper layers
           that the driver is ready for querying battery status */
        if (!ret) {
                queue_delayed_work(battery_status_wq,
                                   &battery_status_task,
                                   DELAY_POLL_INTERVAL);
                queue_delayed_work(power_status_wq,
                                   &power_status_task,
                                   DELAY_POLL_INTERVAL);
                return ret;
        }

        /* Error handling logic starts here: */
#if USE_WAKELOCK
        wake_lock_destroy(&wakelock);
#endif
err_remove_proc_debug:
        remove_proc_entry(PROC_ENTRY_DEBUG, proc_parent);
err_remove_proc_parent:
        remove_proc_entry(PROC_GLOBAL_PARENT_DIR, NULL);
err_reverse_all_regs:
        power_supply_unregister(&battery_main.psy);
err_psy_reg_main:
        power_supply_unregister(&power_dc.psy);
err_psy_reg_dc:
        cancel_delayed_work(&battery_status_task);
        cancel_delayed_work(&power_status_task);
        flush_workqueue(battery_status_wq);
        flush_workqueue(power_status_wq);
        destroy_workqueue(battery_status_wq);
        destroy_workqueue(power_status_wq);
err_gpio:
        pm_power_off = NULL;
        gpio_free_array(gpios, ARRAY_SIZE(gpios));

        printk("%s(): return %d\n", __FUNCTION__, ret);
        return ret;
}

static int __devexit battery_remove(struct platform_device *dev)
{
        int i;

        free_irq(gpio_to_irq(gpios[ACP_SHD_INDEX].gpio), &power_dc);

        power_supply_unregister(&battery_main.psy);
        power_supply_unregister(&power_dc.psy);

        /*
        * now flush all pending works.
        */
        cancel_delayed_work(&battery_status_task);
        cancel_delayed_work(&power_status_task);
        flush_workqueue(battery_status_wq);
        flush_workqueue(power_status_wq);
        destroy_workqueue(battery_status_wq);
        destroy_workqueue(power_status_wq);

        pm_power_off = NULL;

        for (i = 0; i < ARRAY_SIZE(gpios); i++) {
                gpio_free(gpios[i].gpio);
        }

#if USE_WAKELOCK
        wake_lock_destroy(&wakelock);
#endif

        remove_proc_entry(PROC_ENTRY_STATUS, proc_parent);
        remove_proc_entry(PROC_ENTRY_DEBUG, proc_parent);
        remove_proc_entry(PROC_GLOBAL_PARENT_DIR, NULL);
        
        return 0;
}

static struct platform_driver battery_driver = {
        .driver.name = "cmp-battery",
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
        printk(KERN_ERR "%s: called...\n", __FUNCTION__);
        return platform_driver_register(&battery_driver);
}

static void __exit battery_exit(void)
{
        printk(KERN_ERR "%s: called...\n", __FUNCTION__);
        platform_driver_unregister(&battery_driver);
}

module_init(battery_init);
module_exit(battery_exit);

MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("Broadcom CMP BQ24616 Battery Driver");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.01");
