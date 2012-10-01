/*****************************************************************************
* Copyright 2010 - 2011 Broadcom Corporation.  All rights reserved.
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

#include <linux/init.h>
#include <linux/module.h>
#include <linux/err.h>
#include <linux/param.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/pm.h>
#include <linux/io.h>
#include <linux/notifier.h>

#include <linux/interrupt.h>

#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>
#include <linux/timer.h>

#include <linux/clk.h>
#include <linux/irq.h>
#include <linux/broadcom/bcm59055-adc.h>
#include <linux/mfd/bcm590xx/core.h>
#include <linux/debugfs.h>
#include <linux/seq_file.h>
#include <linux/broadcom/kona-thermal.h>

#ifdef CONFIG_MFD_BCMPMU
#include <linux/mfd/bcmpmu.h>
#include <linux/csapi_adc.h>
#endif

#define USE_INTERRUPT

#ifdef CONFIG_ARCH_RHEA
#define therm_debug(format...)              \
    do {                    \
        if (thermal_debug)  \
			pr_info(format);     \
    } while (0)

int therm_tbl[] = {
/*0x*/ 2302, 1631, 1326, 1137, 1038, 964, 905, 856, 815, 779, 747, 718,
	    693, 669, 647, 627,
/*1x*/ 609, 591, 575, 559, 545, 531, 518, 506, 494, 482, 471, 461, 451,
	    441, 432, 422,
/*2x*/ 414, 405, 397, 388, 381, 373, 366, 358, 351, 345, 338, 331, 325,
	    318, 312, 306,
/*3x*/ 300, 295, 289, 283, 278, 272, 267, 262, 257, 252, 247, 242, 237,
	    232, 227, 223,
/*4x*/ 218, 214, 209, 205, 201, 196, 192, 188, 184, 179, 175, 171, 167,
	    163, 159, 156,
/*5x*/ 152, 148, 144, 141, 137, 133, 129, 126, 122, 119, 115, 112, 108,
	    105, 101, 98,
/*6x*/ 95, 91, 88, 84, 81, 78, 75, 71, 68, 65, 62, 58, 55, 52, 49, 46,
/*7x*/ 43, 39, 36, 33, 30, 27, 24, 21, 18, 15, 12, 9, 6, 3, 0, -3,
/*8x*/ -6, -9, -12, -15, -18, -21, -24, -27, -30, -33, -36, -39, -41,
	    -44, -47, -50,
/*9x*/ -53, -56, -59, -62, -65, -68, -71, -74, -77, -80, -83, -86, -89,
	    -92, -95, -98,
/*Ax*/ -101, -104, -107, -110, -113, -116, -119, -122, -125, -128, -131,
	    -134, -137, -140, -144, -147,
/*Bx*/ -150, -153, -156, -160, -163, -166, -169, -173, -176, -179, -183,
	    -186, -189, -193, -196, -200,
/*Cx*/ -203, -207, -210, -214, -218, -221, -225, -229, -232, -236, -240,
	    -244, -248, -252, -256, -260,
/*Dx*/ -264, -269, -273, -277, -281, -286, -290, -295, -300, -304, -309,
	    -314, -319, -324, -330, -335,
/*Ex*/ -340, -346, -352, -358, -364, -370, -377, -383, -389, -397, -406,
	    -412, -418, -425, -431, -439,
/*Fx*/ -447, -455, -464, -473, -483, -495, -507, -520, -536, -554, -574,
	    -599, -630, -673, -742, -750
};

struct platform_drvdata {
	struct platform_device *pdev;
	int irq;
	int alarm_set;
	struct work_struct alarm_work;
	struct work_struct wakeup_work;
};

static int thermal_test_panic(struct notifier_block *this, unsigned long event,
			      void *ptr);

static struct notifier_block thermal_test_block = {
	.notifier_call = thermal_test_panic,
};

ATOMIC_NOTIFIER_HEAD(thermal_notifier_list_1);
ATOMIC_NOTIFIER_HEAD(thermal_notifier_list_2);
ATOMIC_NOTIFIER_HEAD(thermal_notifier_list_3);
ATOMIC_NOTIFIER_HEAD(thermal_notifier_list_4);

struct atomic_notifier_head therm_notifier[4];

static struct timer_list thermal_timer;
struct device *test_dev;
static int thermal_debug;

int convert_adc_to_temp(int raw)
{
	return therm_tbl[raw / 4] * 100;
}

int convert_temp_to_adc(int raw)
{
	int i;

	for (i = 0; i < sizeof(therm_tbl); i++) {
		if ((therm_tbl[i] >= raw / 10)
		    && (therm_tbl[i + 1] <= raw / 10)) {
			therm_debug("new table_offset = 0x%x\n", i + 1);
			return (i + 1) * 4;
		}
	}
	return 0;
}

static ssize_t kona_thermal_get_name(struct device *dev,
				     struct device_attribute *devattr,
				     char *buf)
{
	struct platform_drvdata *pdata = dev_get_drvdata(dev);
	struct sensor_device_attribute_2 *attr = to_sensor_dev_attr_2(devattr);
	struct therm_data *board;

	board = pdata->pdev->dev.platform_data;
	return sprintf(buf, "%s\n", board->sensors[attr->index].thermal_name);
}

static ssize_t kona_thermal_get_threshold(struct device *dev,
					  struct device_attribute *devattr,
					  char *buf)
{
	int long raw = 0;
	struct platform_drvdata *pdata = dev_get_drvdata(dev);
	struct sensor_device_attribute_2 *attr = to_sensor_dev_attr_2(devattr);
	struct therm_data *board;

	board = pdata->pdev->dev.platform_data;

	if (attr->nr == 1) {
		raw = board->sensors[attr->index].thermal_warning_lvl_1;
	} else if (attr->nr == 2) {
		raw = board->sensors[attr->index].thermal_fatal_lvl;
	} else {
		if (board->sensors[attr->index].thermal_read ==
		    SENSOR_READ_PMU_I2C) {
#ifdef CONFIG_MFD_BCM_PMU590XX
			raw = bcm59055_saradc_read_data(board->
							sensors[attr->index].
							thermal_sensor_param);
#else
#if 0
			csapi_adc_raw_read(NULL,
					   board->sensors[attr->index].
					   thermal_sensor_param, &adc_raw, NULL,
					   NULL);
			raw = adc_raw;
#endif
			raw = thermal_get_adc
			    (board->sensors[attr->index].thermal_sensor_param);
#endif
			therm_debug
			    ("temp (deg. C) = %d (raw = %ld (0x%lx) (0x%lx)\n",
			     convert_adc_to_temp(raw), raw, raw, raw / 4);
			return sprintf(buf, "%d\n", convert_adc_to_temp(raw));
		}
	}

	return sprintf(buf, "%ld\n", raw);

}

static ssize_t kona_thermal_set_threshold(struct device *dev,
					  struct device_attribute *devattr,
					  const char *buf, size_t count)
{
	unsigned long raw;
	struct platform_drvdata *pdata = dev_get_drvdata(dev);
	struct sensor_device_attribute_2 *attr = to_sensor_dev_attr_2(devattr);
	struct therm_data *board;

	board = pdata->pdev->dev.platform_data;

	if (strict_strtol(buf, 10, &raw))
		return -EINVAL;

	if (attr->nr == 1)
		board->sensors[attr->index].thermal_warning_lvl_1 = raw;
	else if (attr->nr == 2)
		board->sensors[attr->index].thermal_fatal_lvl = raw;
	therm_debug("new temp = %ld\n", raw);

	return count;
}

static ssize_t kona_thermal_get_alarm(struct device *dev,
				      struct device_attribute *devattr,
				      char *buf)
{
	struct platform_drvdata *pdata = dev_get_drvdata(dev);

	if (pdata->alarm_set) {
		pdata->alarm_set = 0;

		return sprintf(buf, "1\n");
	}

	return sprintf(buf, "0\n");
}

void thermal_interval_wake(unsigned long var)
{
	struct platform_drvdata *pdata = dev_get_drvdata(test_dev);
	struct therm_data *board;

	board = pdata->pdev->dev.platform_data;

	/* Schedule work to notify userspace */
	schedule_work(&pdata->wakeup_work);
	mod_timer(&thermal_timer,
		  jiffies + (board->thermal_update_interval) * HZ);
}

static ssize_t kona_thermal_set_interval(struct device *dev,
					 struct device_attribute *devattr,
					 const char *buf, size_t count)
{
	unsigned long raw;
	struct platform_drvdata *pdata = dev_get_drvdata(dev);
	struct therm_data *board;

	board = pdata->pdev->dev.platform_data;

	if (strict_strtol(buf, 10, &raw))
		return -EINVAL;

	if (board->thermal_update_interval == 0) {
		init_timer(&thermal_timer);
		thermal_timer.function = thermal_interval_wake;
		thermal_timer.expires = jiffies + raw * HZ;
		add_timer(&thermal_timer);
	} else {
		mod_timer(&thermal_timer, jiffies + raw * HZ);
	}
	if (raw == 0)
		del_timer(&thermal_timer);
	board->thermal_update_interval = raw;
	test_dev = dev;

	return count;
}

static ssize_t kona_thermal_get_interval(struct device *dev,
					 struct device_attribute *devattr,
					 char *buf)
{
	unsigned raw;
	struct platform_drvdata *pdata = dev_get_drvdata(dev);
	struct therm_data *board;

	board = pdata->pdev->dev.platform_data;

	raw = board->thermal_update_interval;
	return sprintf(buf, "%d\n", raw);
}

#define SENSOR_TEMP(ix) \
	SENSOR_ATTR_2(temp##ix##_name, S_IRUGO, \
		kona_thermal_get_name, NULL, 0, ix-1), \
	SENSOR_ATTR_2(temp##ix##_input, S_IRUGO, \
		kona_thermal_get_threshold, NULL, 0, ix-1), \
	SENSOR_ATTR_2(temp##ix##_max, S_IRUGO | S_IWUSR, \
		kona_thermal_get_threshold, \
		kona_thermal_set_threshold, 1, ix-1), \
	SENSOR_ATTR_2(temp##ix##_crit, S_IRUGO | S_IWUSR, \
		kona_thermal_get_threshold, \
		kona_thermal_set_threshold, 2, ix-1), \
	SENSOR_ATTR_2(temp##ix##_alarm, S_IRUGO, \
		kona_thermal_get_alarm, NULL, 0, ix-1)

static struct sensor_device_attribute_2 kona_attrs[] = {
	SENSOR_ATTR_2(update_interval, S_IWUSR | S_IRUGO,
		      kona_thermal_get_interval, kona_thermal_set_interval, 0,
		      0),
	SENSOR_TEMP(1),
	SENSOR_TEMP(2),
	SENSOR_TEMP(3),
	SENSOR_TEMP(4)
};

void kona_thermal_register_notify(unsigned therm_id,
				  struct notifier_block *thermal_block)
{
	therm_debug("thermal_register_notify (therm_id=%d)\n", therm_id);
	atomic_notifier_chain_register(&therm_notifier[therm_id - 1],
				       thermal_block);
}
EXPORT_SYMBOL(kona_thermal_register_notify);

void kona_thermal_unregister_notify(unsigned therm_id,
				    struct notifier_block *thermal_block)
{
	therm_debug("thermal_unregister_notify (therm_id=%d)\n", therm_id);
	atomic_notifier_chain_unregister(&therm_notifier[therm_id - 1],
					 thermal_block);
}
EXPORT_SYMBOL(kona_thermal_unregister_notify);

static void kona_thermal_notify_alarm(struct work_struct *ws)
{
	struct platform_drvdata *pdata =
	    container_of(ws, struct platform_drvdata, alarm_work);

	pdata->alarm_set = 1;

	sysfs_notify(&pdata->pdev->dev.kobj, NULL, "therm1_alarm");
	kobject_uevent(&pdata->pdev->dev.kobj, KOBJ_CHANGE);
}

static void kona_thermal_periodic_query(struct platform_drvdata *pdata)
{
	int temp;
	int i;
	static int panic_buf[1024];
	struct therm_data *board;

	board = pdata->pdev->dev.platform_data;

	for (i = 0; i < board->num_sensors; i++) {
		if (board->sensors[i].thermal_control == SENSOR_PERIODIC_READ) {
			if (board->sensors[i].thermal_read ==
			    SENSOR_READ_PMU_I2C) {
#ifdef CONFIG_MFD_BCM_PMU590XX
				temp =
				    convert_adc_to_temp
				    (bcm59055_saradc_read_data
				     (board->sensors[i].thermal_sensor_param));
#else
				temp =
				    convert_adc_to_temp(thermal_get_adc
							(board->sensors[i].
							 thermal_sensor_param));
#if 0
				csapi_adc_raw_read(NULL,
						   board->sensors[i].
						   thermal_sensor_param,
						   &adc_raw, NULL, NULL);
				temp = convert_adc_to_temp(adc_raw);
#endif
#endif
				if (temp >
				    board->sensors[i].thermal_warning_lvl_1) {
					/* Perform WARNING (maximum) action */
					therm_debug
					    ("sensor%d max exceeded %d > %ld\n",
					     i + 1, temp,
					     board->sensors[i].
					     thermal_warning_lvl_1);
					atomic_notifier_call_chain
					    (&therm_notifier[i], 0, panic_buf);

					pdata->alarm_set = 1;

					sysfs_notify(&pdata->pdev->dev.kobj,
						     NULL, "therm_alarm");
					kobject_uevent(&pdata->pdev->dev.kobj,
						       KOBJ_CHANGE);
					if (temp >
					    board->sensors[i].
					    thermal_fatal_lvl) {
						/* Perform FATAL (critical) action */
						therm_debug
						    ("sensor%d exceeded %d > %ld\n",
						     i + 1, temp,
						     board->sensors[i].
						     thermal_fatal_lvl);
						atomic_notifier_call_chain
						    (&therm_notifier[i], 0,
						     panic_buf);
						if ((board->sensors[i].
						     thermal_fatal_action ==
						     THERM_ACTION_NOTIFY_SHUTDOWN)
						    || (board->sensors[i].
							thermal_fatal_action ==
							THERM_ACTION_SHUTDOWN)) {
#ifdef CONFIG_MFD_BCM_PMU590XX
							bcm590xx_shutdown();
#else
							bcmpmu_client_power_off
							    ();
#endif
						}
					}
				}
			}
		}
	}
}

static void kona_thermal_wakeup_event(struct work_struct *ws)
{
	struct platform_drvdata *pdata =
	    container_of(ws, struct platform_drvdata, wakeup_work);

	kona_thermal_periodic_query(pdata);
}

struct dentry *dent_thermal_root_dir;
static int thermal_test_panic(struct notifier_block *this, unsigned long event,
			      void *ptr)
{
	pr_info("thermal_panic test occurred\n");

	return NOTIFY_DONE;
}

static int set_thermal_notification(void *data, u64 val)
{

	therm_debug("%s: val: %lld\n", __func__, val);
	if ((val != 0) && (val != 10)) {
		if (val < 10) {
			kona_thermal_register_notify(val, &thermal_test_block);
			therm_debug("thermal notify register test (%lld)\n",
				    val - 1);
		} else {
			kona_thermal_unregister_notify(val - 10,
						       &thermal_test_block);
			therm_debug("thermal notify unregister test (%lld)\n",
				    val - 11);
		}
	} else {
		therm_debug("invalid value\n");
	}

	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(set_thermal_test_notify, NULL, set_thermal_notification,
			"%llu\n");

int kona_thermal_debug_init(void)
{

	dent_thermal_root_dir = debugfs_create_dir("thermal", 0);
	if (!dent_thermal_root_dir)
		return -ENOMEM;

	if (!debugfs_create_u32
	    ("debug", S_IWUSR | S_IRUSR, dent_thermal_root_dir,
	     (int *)&thermal_debug))
		return -ENOMEM;

	if (!debugfs_create_file
	    ("test_notify", S_IWUSR | S_IRUSR, dent_thermal_root_dir, NULL,
	     &set_thermal_test_notify))
		return -ENOMEM;
	therm_debug("%s:Thermal DEBUGFS initialized\n", __func__);

	return 0;
}

static int kona_thermal_probe(struct platform_device *pdev)
{
	int rc = 0;
	struct platform_drvdata *pdata;
	int i;
	struct therm_data *board;

	/* Create structure to hold register base address
	   and pointer to thermal zone */
	pdata = kzalloc(sizeof(struct platform_drvdata), GFP_KERNEL);
	if (!pdata) {
		dev_err(&pdev->dev, "unable to allocate memory for data\n");
		rc = -ENOMEM;
		goto err_release_mem_region;
	}

	pdata->pdev = pdev;
	pdata->alarm_set = 0;

	/* Create sysfs files */
	for (i = 0; i < ARRAY_SIZE(kona_attrs); i++)
		if (device_create_file(&pdev->dev, &kona_attrs[i].dev_attr))
			goto err_remove_files;

	/* Register hwmon device */
	hwmon_device_register(&pdev->dev);

	/* Init alarm work */
	INIT_WORK(&pdata->alarm_work, kona_thermal_notify_alarm);

	/* Init thermal wakeup work */
	INIT_WORK(&pdata->wakeup_work, kona_thermal_wakeup_event);

	/* Store pointer to driver specific data structure
	   in platform driver structure */
	platform_set_drvdata(pdev, pdata);

	kona_thermal_debug_init();

	board = pdata->pdev->dev.platform_data;

#if 0
	if (board->thermal_update_interval != 0) {
		dev_err(&pdev->dev, "thermal init update YES\n");
		init_timer(&thermal_timer);
		thermal_timer.function = thermal_interval_wake;
		thermal_timer.expires =
		    jiffies + (board->thermal_update_interval) * HZ;
		add_timer(&thermal_timer);
	}
#endif
	therm_notifier[0] = thermal_notifier_list_1;
	therm_notifier[1] = thermal_notifier_list_2;
	therm_notifier[2] = thermal_notifier_list_3;
	therm_notifier[3] = thermal_notifier_list_4;

	return 0;

	platform_set_drvdata(pdev, NULL);

	hwmon_device_unregister(&pdev->dev);

err_remove_files:
	dev_err(&pdev->dev, "err_remove_files\n");
	for (i = i - 1; i >= 0; i--)
		device_remove_file(&pdev->dev, &kona_attrs[i].dev_attr);

err_release_mem_region:
	kfree(pdata);

	return rc;

}

static int kona_thermal_remove(struct platform_device *pdev)
{
	struct platform_drvdata *pdata = platform_get_drvdata(pdev);
	struct resource *iomem;
	int i;

	disable_irq(pdata->irq);
	free_irq(pdata->irq, pdata);

	platform_set_drvdata(pdev, NULL);

	hwmon_device_unregister(&pdev->dev);

	for (i = ARRAY_SIZE(kona_attrs) - 1; i >= 0; i--)
		device_remove_file(&pdev->dev, &kona_attrs[i].dev_attr);

	kfree(pdata);

	iomem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	release_mem_region(iomem->start, resource_size(iomem));

	return 0;
}

#ifdef CONFIG_PM
static int kona_thermal_resume(struct platform_device *pdev)
{
	struct platform_drvdata *pdata = dev_get_drvdata(&pdev->dev);

	kona_thermal_periodic_query(pdata);
	return 0;
}

static int kona_thermal_suspend(struct platform_device *pdev, pm_message_t mesg)
{
	/* TODO: add suspend support */
	return 0;
}
#else
#define kona_thermal_suspend    NULL
#define kona_thermal_resume     NULL
#endif

/*********************************************************************
 *             Driver initialisation and finalization
 *********************************************************************/

static struct platform_driver kona_thermal_driver = {
	.driver = {
		   .name = "kona-thermal",
		   .owner = THIS_MODULE,
		   },
	.probe = kona_thermal_probe,
	.remove = kona_thermal_remove,
	.suspend = kona_thermal_suspend,
	.resume = kona_thermal_resume,
};

static int __init kona_thermal_init(void)
{
	return platform_driver_register(&kona_thermal_driver);
}

static void __exit kona_thermal_exit(void)
{
	platform_driver_unregister(&kona_thermal_driver);
}

module_init(kona_thermal_init);
module_exit(kona_thermal_exit);

MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("Broadcom Thermal Management Monitor");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");
#endif
