/* Copyright (c) 2012-2013, The Linux Foundation. All rights reserved.
 * Copyright (c) 2014, Alok Nandan Nikhil (nikhil.jan93@gmail.com)
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/msm_tsens.h>
#include <linux/workqueue.h>
#include <linux/cpu.h>
#include <linux/cpufreq.h>
#include <linux/msm_tsens.h>
#include <linux/msm_thermal.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <mach/cpufreq.h>

unsigned int temp_threshold = 70;
module_param(temp_threshold, int, 0755);

unsigned int user_freq_hell = 729600;
module_param(user_freq_hell, int, 0755);

unsigned int user_freq_very_hot = 1036800;
module_param(user_freq_very_hot, int, 0755);

unsigned int user_freq_hot = 1267200;
module_param(user_freq_hot, int, 0755);

unsigned int user_freq_warm = 1497600;
module_param(user_freq_warm, int, 0755);

unsigned int user_reschedule = 250;
module_param(user_reschedule, int, 0755);

unsigned int aggressiveness = 5;
module_param(aggressiveness, int, 0755);

unsigned int smart_monitoring = 1;
module_param(smart_monitoring, int, 0755);

int avg_temp = 0;
int last_n_temp[10];
int pos = 0;

static struct thermal_info {
	uint32_t limited_max_freq;
	unsigned int safe_diff;
	uint32_t sensor_id[3];
	bool busy;
} info = {
	.limited_max_freq = LONG_MAX,
	.safe_diff = 5,
	.sensor_id = {1,5,7},
	.busy = false,
};

enum threshold_levels {
	LEVEL_HELL 	   = 12,
	LEVEL_VERY_HOT = 9,
	LEVEL_HOT 	   = 5,
};

static struct msm_thermal_data msm_thermal_info;

static struct delayed_work check_temp_work;
static struct delayed_work verify_freq_work;

unsigned short get_threshold(void)
{
	return temp_threshold;
}

unsigned int get_average_temp(int sensors[])	{

	struct tsens_device tsens_dev;
	int curr_avg = 0.0;
	long curr_temp = 0.0;
	int i;

	for (i = 0; i < 3; i++)	{
		tsens_dev.sensor_num = sensors[i];
		tsens_get_temp(&tsens_dev, &curr_temp);
		printk("Current temperature from Sensor %d is %lu \n",sensors[i], curr_temp);
		curr_avg+=curr_temp;			
	}
	
	return (curr_avg/3);
}

unsigned int get_moving_average_temp(int new_avg)	{

	int i;
	int overall_avg_temp = 0;

	if(aggressiveness > 5)
		aggressiveness = 5;

	if(pos > aggressiveness)	{
		pos = 0;
	}
	
	last_n_temp[pos] = new_avg;
	
	for(i = 0; i < aggressiveness; i++)	{
		overall_avg_temp += last_n_temp[i];
	}

	pos++;
	return (overall_avg_temp/aggressiveness);	

}

static int msm_thermal_cpufreq_callback(struct notifier_block *nfb,
		unsigned long event, void *data)
{
	struct cpufreq_policy *policy = data;

	if (event != CPUFREQ_ADJUST)
		return 0;

	cpufreq_verify_within_limits(policy, policy->cpuinfo.min_freq,
		info.limited_max_freq);

	return 0;
}

static struct notifier_block msm_thermal_cpufreq_notifier = {
	.notifier_call = msm_thermal_cpufreq_callback,
};

static void verify_freq(struct work_struct *work)
{

	unsigned int cpu;

	/* Update new limits */
	get_online_cpus();
	for_each_online_cpu(cpu)
	{
		user_changed = false;
		cpufreq_update_policy(cpu);
		user_changed = true;
		pr_info("%s: Setting cpu%d max frequency to %d\n",
				KBUILD_MODNAME, cpu, info.limited_max_freq);
	}
	put_online_cpus();
	info.busy = false;
}

static void check_temp(struct work_struct *work)
{
	uint32_t freq = 0;

	avg_temp = get_average_temp(info.sensor_id);
	avg_temp = get_moving_average_temp(avg_temp);

	printk("Average temperature is %d \n",avg_temp);

	if ((avg_temp < (temp_threshold - info.safe_diff)) && !info.busy)
	{
		info.limited_max_freq = ref_cpufreq_max;
		schedule_delayed_work_on(0, &verify_freq_work, msecs_to_jiffies(user_reschedule*4));
		info.busy = true;
		goto reschedule;
	}

	if (avg_temp >= (temp_threshold + LEVEL_HELL))
		freq = user_freq_hell;
	else if (avg_temp >= (temp_threshold + LEVEL_VERY_HOT))
		freq = user_freq_very_hot;
	else if (avg_temp >= (temp_threshold + LEVEL_HOT))
		freq = user_freq_hot;
	else if (avg_temp > temp_threshold)
		freq = user_freq_warm;

	if (freq)
	{
		info.limited_max_freq = freq;
		schedule_delayed_work_on(0, &verify_freq_work, msecs_to_jiffies(user_reschedule*4));

	}

	if(user_reschedule < 100)
		user_reschedule = 100;

	//aggressiveness = 100/(user_reschedule/250);

reschedule:
	if(smart_monitoring == 1)	{
		if(avg_temp < temp_threshold)
			schedule_delayed_work_on(0, &check_temp_work, msecs_to_jiffies((temp_threshold - avg_temp) * user_reschedule * 2));
		else	
			schedule_delayed_work_on(0, &check_temp_work, msecs_to_jiffies(user_reschedule));

	}
	else	
		schedule_delayed_work_on(0, &check_temp_work, msecs_to_jiffies(user_reschedule));
}

int __devinit msm_thermal_init(struct msm_thermal_data *pdata)
{
	int ret = 0;

	BUG_ON(!pdata);
	BUG_ON(pdata->sensor_id >= TSENS_MAX_SENSORS);
	memcpy(&msm_thermal_info, pdata, sizeof(struct msm_thermal_data));

	cpufreq_register_notifier(&msm_thermal_cpufreq_notifier,
			CPUFREQ_POLICY_NOTIFIER);

	INIT_DELAYED_WORK(&check_temp_work, check_temp);
	schedule_delayed_work_on(0, &check_temp_work, 0);

	INIT_DELAYED_WORK(&verify_freq_work, verify_freq);

	return ret;
}

static int __devinit msm_thermal_dev_probe(struct platform_device *pdev)
{
	int ret = 0;
	char *key = NULL;
	struct device_node *node = pdev->dev.of_node;
	struct msm_thermal_data data;

	memset(&data, 0, sizeof(struct msm_thermal_data));
	key = "qcom,sensor-id";
	ret = of_property_read_u32(node, key, &data.sensor_id);
	if (ret)
		goto fail;
	WARN_ON(data.sensor_id >= TSENS_MAX_SENSORS);

fail:
	if (ret)
		pr_err("%s: Failed reading node=%s, key=%s\n",
		       __func__, node->full_name, key);
	else
		ret = msm_thermal_init(&data);

	return ret;
}

static struct of_device_id msm_thermal_match_table[] = {
	{.compatible = "qcom,msm-thermal"},
	{},
};

static struct platform_driver msm_thermal_device_driver = {
	.probe = msm_thermal_dev_probe,
	.driver = {
		.name = "msm-thermal",
		.owner = THIS_MODULE,
		.of_match_table = msm_thermal_match_table,
	},
};

int __init msm_thermal_device_init(void)
{
	return platform_driver_register(&msm_thermal_device_driver);
}
