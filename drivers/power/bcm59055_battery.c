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
#include <linux/kernel.h>
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

#define BYTE_COMBINE(msb,lsb)  (msb<<8 | lsb)
#define MV_FROM_REG(regval)  DIV_ROUND_CLOSEST(((regval) * 4800 ) , 1024)

struct bcm59055_battery_data {

    struct bcm590xx *bcm590xx;
    struct power_supply battery;
    struct power_supply wall;

    enum power_supply_type power_src;
    struct delayed_work charger_insert_wq;
    struct delayed_work batt_lvl_wq;

    struct mv_percent *vp_table ;
    unsigned int vp_table_cnt ;

    unsigned short batt_min_volt ;     
    unsigned short batt_max_volt ;     
    unsigned short batt_technology ;     

    // Display properties.
    unsigned int batt_capacity ; 
    unsigned int prev_batt_capacity ; 
    unsigned int batt_status ; 
    unsigned int batt_voltage_now ; 
};

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

static inline int bcm59055_linear_solve(int x, int x1, int x2, int y1, int y2)
{
   return DIV_ROUND_CLOSEST(((x-x1)*(y2-y1)),((x2-x1))) + y1;
}

static int bcm59055_mv_to_percent(struct mv_percent *vp_table, int voltage, unsigned int cnt )
{
    int i;

    i = cnt - 1;
    while ( i >= 0) { 
       if ( voltage > vp_table[i].mv ) {
           if( i == cnt-1 )
               return vp_table[i].percentage;
           else
               return bcm59055_linear_solve(voltage,
                      vp_table[i].mv, vp_table[i+1].mv,
                      vp_table[i].percentage, vp_table[i+1].percentage);
       }
   }
   return vp_table[0].percentage;
}

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


static int bcm59055_get_battery_mv(struct bcm590xx *bcm590xx, unsigned int *mv  ) 
{
    unsigned int regval = 0 ;
    unsigned int regval1 = 0 ;

    regval = bcm590xx_reg_read(SLAVE_ID0,BCM59055_REG_ADCCTRL3 ) ;
    regval1 = bcm590xx_reg_read(SLAVE_ID0,BCM59055_REG_ADCCTRL4 ) ;

    if ( !(regval & BCM59055_INVALID_ADCVAL ) ) {
        regval  &= BCM59055_REG_ADCCTRL3_VALID_BITS ;
        *mv = MV_FROM_REG(BYTE_COMBINE(regval,regval1)) ;
    }
    return 0 ;    
}

static void bcm59055_batt_lvl_wq(struct work_struct *work)
{
    struct bcm59055_battery_data *battery_data = container_of(work, struct bcm59055_battery_data, batt_lvl_wq.work);

    bcm59055_get_battery_mv(battery_data->bcm590xx, &(battery_data->batt_voltage_now)) ;

    battery_data->batt_capacity = bcm59055_mv_to_percent(battery_data->vp_table, battery_data->batt_voltage_now, battery_data->vp_table_cnt ) ;

    if ( battery_data->batt_capacity != battery_data->prev_batt_capacity ) { 
        power_supply_changed(&battery_data->battery) ;
    }

    battery_data->prev_batt_capacity = battery_data->batt_capacity ;

    schedule_delayed_work(&battery_data->batt_lvl_wq, msecs_to_jiffies(60000));
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
            val->intval = battery_data->batt_technology ;
            break;
        case POWER_SUPPLY_PROP_CAPACITY:
            val->intval = battery_data->batt_capacity ;
            break;
        case POWER_SUPPLY_PROP_VOLTAGE_MIN:
            val->intval = battery_data->batt_min_volt ;
            break;
        case POWER_SUPPLY_PROP_VOLTAGE_MAX:
            val->intval = battery_data->batt_max_volt ;
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

int bcm59055_initialize_charging( struct bcm590xx *bcm59055 )
{
    int ret = 0 ;

    ret = bcm590xx_reg_write(SLAVE_ID0, BCM59055_REG_MBCCTRL6, BCM59055_REG_MBCCTRL6_VFLOATMAX_4_2V ) ;
    if ( ret < 0 ) {
        printk(KERN_ERR "%s Configuring MBCCTRL6 failed \n", __func__ ) ;
        return ret ;
    }

    ret = bcm590xx_reg_write(SLAVE_ID0, BCM59055_REG_MBCCTRL8, BCM59055_REG_MBCCTRL8_ICCMAX_940MA ) ;
    if ( ret < 0 ) {
        printk(KERN_ERR "%s Configuring MBCCTRL8 failed \n", __func__ ) ;
        return ret ;
    }

    ret = bcm590xx_reg_write(SLAVE_ID0, BCM59055_REG_MBCCTRL9, BCM59055_REG_MBCCTRL9_SWUP_ON ) ;
    if ( ret < 0 ) {
        printk(KERN_ERR "%s Configuring MBCCTRL8 failed \n", __func__ ) ;
        return ret ;
    }
    return ret ;
}

int bcm59055_start_charging(struct bcm590xx *bcm59055 )
{
    int ret = 0 ;
    int val = 0 ;

    ret = bcm590xx_reg_write(SLAVE_ID0, BCM59055_REG_MBCCTRL3, (BCM59055_REG_MBCCTRL3_WAC_HOSTEN | BCM59055_REG_MBCCTRL3_WAC_HAS_PRIORITY ) ) ;
    if ( ret < 0 ) {
        printk(KERN_ERR "%s Configuring MBCCTRL3 failed \n", __func__ ) ;
        return ret ;
    }

    val = ( BCM59055_REG_PWMLEDCTRL1_PWM_FREQ_256 << BCM59055_REG_PWMLEDCTRL1_PWM_FREQ_256_SHIFT) | ( BCM59055_REG_PWMLEDCTRL1_BREATHING_REPEAT << BCM59055_REG_PWMLEDCTRL1_BREATHING_REPEAT_SHIFT ) ; 
    ret = bcm590xx_reg_write(SLAVE_ID1, BCM59055_REG_PWMLEDCTRL1_HWSW_CNTRL_SW, val) ;
    if ( ret < 0 ) {
        printk(KERN_ERR "%s Configuring PMU LED failed \n", __func__ ) ;
        return ret ;
    }
    return ret ;
}

static void bcm59055_power_isr(int intr, void *data)
{
    int val = 0 ;
    struct bcm59055_battery_data *battery_data = data;
    struct bcm590xx *bcm59055 = battery_data->bcm590xx;
    struct bcm590xx_battery_pdata *pdata = bcm59055->pdata->battery_pdata;

    switch (intr) {
        case BCM59055_IRQID_INT2_CHGINS:
            if (pdata && pdata->can_start_charging && !pdata->can_start_charging(NULL))
            {
                battery_data->batt_status = POWER_SUPPLY_STATUS_DISCHARGING ;
                power_supply_changed(&battery_data->battery) ;
            }
            else 
            {
                bcm59055_start_charging(bcm59055);
                battery_data->batt_status = POWER_SUPPLY_STATUS_CHARGING ; 
                power_supply_changed(&battery_data->battery) ;
            }
            break;

        case BCM59055_IRQID_INT2_CHGRM:
        {
            val = ( BCM59055_REG_PWMLEDCTRL1_PWM_FREQ_256 << BCM59055_REG_PWMLEDCTRL1_PWM_FREQ_256_SHIFT) | 
                  ( BCM59055_REG_PWMLEDCTRL1_PWMLED_PD_DISABLE << BCM59055_REG_PWMLEDCTRL1_PWMLED_PD_DISABLE_SHIFT ) |
                  ( BCM59055_REG_PWMLEDCTRL1_BREATHING_REPEAT << BCM59055_REG_PWMLEDCTRL1_BREATHING_REPEAT_SHIFT ) ; 
            bcm590xx_reg_write( SLAVE_ID1, BCM59055_REG_PWMLEDCTRL1, val) ;
            battery_data->batt_status = POWER_SUPPLY_STATUS_DISCHARGING ;
            power_supply_changed(&battery_data->battery) ;
            break;
        }
    }
}

int bcm59055_init_charger(struct bcm59055_battery_data *battery_data)
{
    int ret = 0 ;
    struct bcm590xx *bcm59055 = battery_data->bcm590xx;

    ret = bcm59055_initialize_charging(bcm59055 ) ;
    if ( ret < 0 ) 
    {
        printk(KERN_ERR "%s bcm59055_initialize_charging failed.\n", __func__ ) ;
        goto err ;
    }

    ret = bcm590xx_request_irq(bcm59055, BCM59055_IRQID_INT2_CHGINS, true, bcm59055_power_isr, battery_data);	/*EOC charge interrupt */
    if ( ret < 0 ) 
    {
        printk(KERN_ERR "%s request_irq for charger inserted condition failed \n", __func__ ) ;
        goto err ;
    }

    ret = bcm590xx_request_irq(bcm59055, BCM59055_IRQID_INT2_CHGRM, true, bcm59055_power_isr, battery_data);	/*WAC connected interrupt */
    if ( ret < 0 ) 
    {
        printk(KERN_ERR "%s request_irq for charger removed condition failed \n", __func__ ) ;
        goto err ;
    }

err :    
    return ret ;
}

static int bcm59055_battery_probe(struct platform_device *pdev)
{
    int ret;
    int regval = 0 ;
    struct bcm59055_battery_data *battery_data;
    struct bcm590xx *bcm59055 = dev_get_drvdata(pdev->dev.parent);
    struct bcm590xx_battery_pdata *battery_pdata;
   
    battery_pdata = bcm59055->pdata->battery_pdata;

    battery_data = kzalloc(sizeof(struct bcm59055_battery_data), GFP_KERNEL);
    if (battery_data == NULL) {
        printk(KERN_ERR"%s : Failed to allocate memory for bcm59055_battery_data\n", __func__);
        ret = -ENOMEM;
        goto err_data_alloc;
    }

    battery_data->bcm590xx = bcm59055;
    battery_data->vp_table = battery_pdata->vp_table ;
    battery_data->vp_table_cnt = battery_pdata->vp_table_cnt ;
    battery_data->batt_min_volt = battery_pdata->batt_min_volt ;
    battery_data->batt_max_volt = battery_pdata->batt_max_volt ;
    battery_data->batt_technology = battery_pdata->batt_technology ;

    INIT_DELAYED_WORK(&battery_data->batt_lvl_wq, bcm59055_batt_lvl_wq); 

    platform_set_drvdata(pdev, battery_data);

    ret = bcm59055_init_charger(battery_data) ;
    if ( ret < 0 ) 
    {
        printk(KERN_ERR "%s Charger initialization failed. \n", __func__) ;
        goto err_ac_register;
    }

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
    regval = bcm590xx_reg_read(SLAVE_ID0, BCM59055_REG_MBCCTRL3 ) ;

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
    struct bcm590xx *bcm59055 = data->bcm590xx;

    power_supply_unregister(&data->battery);
    power_supply_unregister(&data->wall);

    cancel_delayed_work(&data->batt_lvl_wq);
	
    bcm590xx_free_irq(bcm59055, BCM59055_IRQID_INT2_CHGINS);	/*EOC charge interrupt */
    bcm590xx_free_irq(bcm59055, BCM59055_IRQID_INT2_CHGRM);	/*WAC connected interrupt */
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
