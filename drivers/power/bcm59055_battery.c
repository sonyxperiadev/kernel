/*
 *  linux/drivers/power/bcm59055_batter.c - Broadcom BCM59055 Battery driver
 *
 *  Copyright (C) 2010 Broadcom, All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/power_supply.h>
#include <linux/types.h>
#include <linux/pci.h>
#include <linux/interrupt.h>
#include <asm/io.h>
#include <linux/slab.h>

#include <linux/mfd/bcm590xx/bcm59055_A0.h>

#include <linux/mfd/bcm590xx/core.h>
#include <linux/mfd/bcm590xx/pmic.h>

#include <linux/workqueue.h>

#define ARRAYSIZE(a) (sizeof(a)/sizeof(a[0]))
#define BATT_LVL_WQ_DELAY  (60000)

struct bcm59055_battery_data {
	// struct power_supply battery;
	// struct power_supply ac;

	struct bcm590xx *bcm590xx;
	struct power_supply battery;
	struct power_supply wall;
	// struct power_supply usb;

	enum power_supply_type power_src;
	struct delayed_work charger_insert_wq;
	struct delayed_work batt_lvl_wq;
#ifdef CONFIG_HAS_WAKELOCK
	// struct wake_lock batt_monitor_wl;
	// struct wake_lock usb_charger_wl;
#endif
	// Display properties.
	unsigned int batt_capacity ; // Used for battery icon capacity. This is a percentage value.
	unsigned int batt_status ; 
	unsigned int batt_voltage_now ; 
};


struct voltage_percentage   
{
    unsigned int voltage ;
    unsigned int percentage ;
} ; 

struct voltage_percentage vp_table[] = 
{
    { 3800 , 5 },
    { 3850 , 25 },
    { 3900 , 50 },
    { 3950 , 70 },
    { 4000 , 90 },
    { 4100 , 100 },
} ;

static inline int bcm59055_divround(int numer, int denom)
{
    return (numer + denom/2)/denom;
}

static inline int bcm59055_linear_solve(int x, int x1, int x2, int y1, int y2)
{
   return bcm59055_divround( (x-x1)*(y2-y1), (x2-x1)) + y1;
}

static int bcm59055_mv_to_percent(int voltage)
{
    int i;

    for(i=ARRAYSIZE(vp_table) ; i > 0;  )
    {
       --i;
       if (voltage > vp_table[i].voltage )
       {
          if( i == ARRAYSIZE(vp_table)-1 )
                return vp_table[i].percentage;
          else
               return bcm59055_linear_solve(voltage,
                       vp_table[i].voltage, vp_table[i+1].voltage,
                       vp_table[i].percentage, vp_table[i+1].percentage);
       }
   }
   return vp_table[0].percentage;
}

/* AC power values exported in /sys/class/power_supply/ac */
#define AC_ONLINE	(1)

static enum power_supply_property bcm59055_battery_props[] = {
	POWER_SUPPLY_PROP_STATUS,
	POWER_SUPPLY_PROP_TECHNOLOGY,
	POWER_SUPPLY_PROP_CAPACITY,
	POWER_SUPPLY_PROP_VOLTAGE_NOW,
};

static enum power_supply_property bcm59055_wall_props[] = {
	POWER_SUPPLY_PROP_ONLINE,
};

static int bcm59055_wall_get_property(struct power_supply *psy,
			enum power_supply_property psp,
			union power_supply_propval *val)
{
	int ret = 0;

	switch (psp) {
	case POWER_SUPPLY_PROP_ONLINE:
		val->intval = AC_ONLINE;
		break;
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}

static int bcm59055_get_battery_voltage_per(struct bcm590xx *bcm590xx, unsigned int *percentage, unsigned int *voltage_now  ) 
{
	unsigned int regval = 0 ;
	unsigned int regval1 = 0 ;
	unsigned int millivolt_mul = 1000 ;

    regval = bcm590xx_reg_read(bcm590xx,BCM59055_REG_ADCCTRL3 ) ;
    regval1 = bcm590xx_reg_read(bcm590xx,BCM59055_REG_ADCCTRL4 ) ;

	if ( !(regval & BCM59055_INVALID_ADCVAL ) )
	{
		regval  = regval & BCM59055_REG_ADCCTRL3_VALID_BITS ;

        regval = ( regval << 8 ) | ( regval1 ) ;

    	regval = ( regval * millivolt_mul * 48) / ( 1024 * 10 ) ;

        *voltage_now = regval ;

        *percentage = bcm59055_mv_to_percent(*voltage_now) ;
    }
	return 0 ;    
}


static void bcm59055_batt_lvl_wq(struct work_struct *work)
{
    struct bcm59055_battery_data *battery_data = container_of(work, struct bcm59055_battery_data, batt_lvl_wq.work);

    bcm59055_get_battery_voltage_per(battery_data->bcm590xx, &(battery_data->batt_capacity) , &(battery_data->batt_voltage_now)) ;

    power_supply_changed(&battery_data->battery) ;

	schedule_delayed_work(&battery_data->batt_lvl_wq, msecs_to_jiffies(BATT_LVL_WQ_DELAY));
}


static int bcm59055_battery_get_property(struct power_supply *psy,
				 enum power_supply_property psp,
				 union power_supply_propval *val)
{
	int ret = 0;

	struct bcm59055_battery_data *battery_data = dev_get_drvdata(psy->dev->parent);

	switch (psp) {
	case POWER_SUPPLY_PROP_STATUS:
		val->intval = battery_data->batt_status ;
		break;
	case POWER_SUPPLY_PROP_TECHNOLOGY:
		val->intval = POWER_SUPPLY_TECHNOLOGY_LION;
		break;
	case POWER_SUPPLY_PROP_CAPACITY:
		val->intval = battery_data->batt_capacity ;
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_MIN:
		val->intval = 3200 * 1000 ;
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_MAX:
		val->intval = 4200 * 1000 ;
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_NOW:
		val->intval = battery_data->batt_voltage_now * 1000 ;
		break;
	default:
		ret = -EINVAL;
		break;
	}


	return ret;
}

void bcm59055_initialize_charging( struct bcm590xx *bcm59055 )
{
    bcm590xx_reg_write(bcm59055, BCM59055_REG_MBCCTRL6, 0x08 ) ;
    bcm590xx_reg_write(bcm59055, BCM59055_REG_MBCCTRL8, 0x07 ) ;
    bcm590xx_reg_write(bcm59055, BCM59055_REG_MBCCTRL9, 0x01 ) ;
}

void bcm59055_start_charging(struct bcm590xx *bcm59055 )
{
    bcm590xx_reg_write(bcm59055, BCM59055_REG_MBCCTRL3, 5 ) ;
    bcm590xx_reg_write_slave1(0, 0x44) ;
    return ;
}

static void bcm59055_power_isr(int intr, void *data)
{
	struct bcm59055_battery_data *battery_data = data;
	struct bcm590xx *bcm59055 = battery_data->bcm590xx;
	struct bcm590xx_battery_pdata *pdata = bcm59055->pdata->battery_pdata;

	switch (intr) {
	case BCM59055_IRQID_INT2_CHGINS:
		printk("%s Wall Charger inserted interrupt \n", __func__);
		if (pdata && pdata->can_start_charging && !pdata->can_start_charging(NULL))
		{
			printk ("charging not started\n");
			battery_data->batt_status = POWER_SUPPLY_STATUS_DISCHARGING ;
            power_supply_changed(&battery_data->battery) ;
		}
		else 
		{
			printk ("charging started\n");
			bcm59055_start_charging(bcm59055);
			battery_data->batt_status = POWER_SUPPLY_STATUS_CHARGING ; 
            power_supply_changed(&battery_data->battery) ;
		}
		break;

	case BCM59055_IRQID_INT2_CHGRM:
		{
    		printk("%s Wall Charger REMOVED interrupt \n", __func__);
            bcm590xx_reg_write_slave1(0, 0x46) ;
            battery_data->batt_status = POWER_SUPPLY_STATUS_DISCHARGING ;
            power_supply_changed(&battery_data->battery) ;
    		break;
		}
	}
}

int bcm59055_init_charger(struct bcm59055_battery_data *battery_data)
{
	struct bcm590xx *bcm59055 = battery_data->bcm590xx;

	printk("######## Init charging called \n" ) ;

	bcm59055_initialize_charging(bcm59055 ) ;

    bcm590xx_request_irq(bcm59055, BCM59055_IRQID_INT2_CHGINS, true, bcm59055_power_isr, battery_data);	/*EOC charge interrupt */
	bcm590xx_request_irq(bcm59055, BCM59055_IRQID_INT2_CHGRM, true, bcm59055_power_isr, battery_data);	/*WAC connected interrupt */

    return 0 ;
}

static int bcm59055_battery_probe(struct platform_device *pdev)
{
	int ret;
	int regval = 0 ;
	struct bcm59055_battery_data *battery_data;
	struct bcm590xx *bcm59055 = dev_get_drvdata(pdev->dev.parent);  // From debugger make sure we get this information correctly.
	struct bcm590xx_battery_pdata *battery_pdata;
   
	battery_pdata = bcm59055->pdata->battery_pdata;

	battery_data = kzalloc(sizeof(struct bcm59055_battery_data), GFP_KERNEL);
	if (battery_data == NULL) {
		printk(KERN_ERR"%s : Failed to allocate memory for bcm59055_battery_data\n", __func__);
		ret = -ENOMEM;
		goto err_data_alloc;
	}

	battery_data->bcm590xx = bcm59055;

	// INIT_DELAYED_WORK(&battery_data->charger_insert_wq, bcm59055_charger_wq); Implement bcm59055_charger_wq
	INIT_DELAYED_WORK(&battery_data->batt_lvl_wq, bcm59055_batt_lvl_wq); 

	platform_set_drvdata(pdev, battery_data);

	bcm59055_init_charger(battery_data) ;

	battery_data->battery.name = "bcm59055-battery";
	battery_data->battery.type = POWER_SUPPLY_TYPE_BATTERY;
	battery_data->battery.properties = bcm59055_battery_props;
	battery_data->battery.num_properties = ARRAY_SIZE(bcm59055_battery_props);
	battery_data->battery.get_property = bcm59055_battery_get_property;
    battery_data->batt_status = POWER_SUPPLY_STATUS_NOT_CHARGING ;

	battery_data->wall.name = "bcm59055-wall";
	battery_data->wall.type = POWER_SUPPLY_TYPE_MAINS;
	battery_data->wall.properties = bcm59055_wall_props;
	battery_data->wall.num_properties = ARRAY_SIZE(bcm59055_wall_props);
	battery_data->wall.get_property = bcm59055_wall_get_property;

	/* If it were a real device, get platform device resources here
	 * and ioremap the register space + register an interrupt if needed
	 */

	/* Register AC power supply */
	ret = power_supply_register(&pdev->dev, &battery_data->wall);
	if (ret) {
		printk(KERN_ERR"%s : Failed to register WALL power supply\n", __func__); 
		goto err_ac_register;
	}

	/* Register battery power supply */
	ret = power_supply_register(&pdev->dev, &battery_data->battery);
	if (ret) {
		printk(KERN_ERR"%s : Failed to register battery power supply\n", __func__); 
		goto err_battery_register;
	}

	// Findout if charging is already enabled in uboot.
    regval = bcm590xx_reg_read(bcm59055, BCM59055_REG_MBCCTRL3 ) ;

	if ( ( ( regval & BCM59055_REG_MBCCTRL3_WAC_HOSTEN ) == BCM59055_REG_MBCCTRL3_WAC_HOSTEN ) && 
         (  ( regval & BCM59055_REG_MBCCTRL3_USB_HOSTEN ) == 0 ) )
	{
        // This means charging was enabled in uboot. 
        battery_data->batt_status = POWER_SUPPLY_STATUS_CHARGING ; 
        power_supply_changed(&battery_data->battery) ;
	}

	schedule_delayed_work(&battery_data->batt_lvl_wq, 0 );
	return 0;

err_battery_register:
	power_supply_unregister(&battery_data->wall);
err_ac_register:
	kfree(battery_data);
err_data_alloc:
	return ret;
}

static int bcm59055_battery_remove(struct platform_device *pdev)
{
	struct bcm59055_battery_data *data = platform_get_drvdata(pdev);

	power_supply_unregister(&data->battery);
	power_supply_unregister(&data->wall);
	kfree(data);

	return 0;
}

static struct platform_driver bcm59055_battery_driver = {
	.probe		= bcm59055_battery_probe,
	.remove		= bcm59055_battery_remove,
	.driver = {
		.name = "bcm59055-battery"
	}
};

static int __init bcm59055_battery_init(void)
{
	return platform_driver_register(&bcm59055_battery_driver);
}

static void __exit bcm59055_battery_exit(void)
{
	platform_driver_unregister(&bcm59055_battery_driver);
}

module_init(bcm59055_battery_init);
module_exit(bcm59055_battery_exit);
