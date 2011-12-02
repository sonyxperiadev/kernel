/*************************************************************************
*
* Copyright 2010 --2011 Broadcom Corporation.
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2, available at
* http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
*
*****************************************************************************/

#include <linux/sched.h>
#include <linux/cpuidle.h>
#include <linux/pm.h>
#include <linux/delay.h>
#include <linux/suspend.h>
#include <linux/module.h>
#include <linux/smp.h>
#include <linux/cpumask.h>
#include <plat/kona_pm.h>
#include <plat/pwr_mgr.h>
#include <plat/pi_mgr.h>
#include <linux/clk.h>
#include <asm/io.h>
#include <mach/io_map.h>
#include <plat/clock.h>
#include <linux/err.h>
#include <linux/debugfs.h>
#include <mach/rdb/brcm_rdb_scu.h>
#include <mach/rdb/brcm_rdb_csr.h>
#include <mach/rdb/brcm_rdb_chipreg.h>
#include <mach/rdb/brcm_rdb_iroot_clk_mgr_reg.h>
#include <mach/rdb/brcm_rdb_gicdist.h>
#include <mach/rdb/brcm_rdb_pwrmgr.h>
#include <linux/workqueue.h>
#include <mach/pwr_mgr.h>
#include <asm/gpio.h>
#include <linux/irq.h>
#include <linux/interrupt.h>

#include <linux/kernel.h>
#include <linux/init.h>
#include <mach/pinmux.h>

#ifdef CONFIG_DEBUG_FS

static struct __init pin_config board_test_pin_config[] = {

	PIN_CFG(SIM2_RESETN, GPIO, 0, ON, OFF, 0, 0, 8MA),
};

static void gpio143Callback(u32 event_id, void *param)
{
	pr_info("%s:%s", __func__, (char *)param);
}


static irqreturn_t gpioPmTest_isr(int irq, void *param)
{
	pr_info("%s:%s", __func__, (char *)param);
	return IRQ_HANDLED;
}

char *test_string = "gpio 143";

static int set_pm_gpio_event(void *data, u64 val)
{
	int ret;
	u32 event_id;
	int irq;
	struct pm_policy_cfg cfg;
	cfg.ac = 1;
	cfg.atl = 0;

	pr_info("%s: GPIO143\n", __func__);

	ret = gpio_request(143, "pmgpio-irq");
	if (ret < 0)
		pr_info("%s failed at gpio 143 request.\n", __func__);

	ret = gpio_direction_input(143);

	if (ret < 0)
		pr_info("%s failed at gpio 143 direction_input\n", __func__);

	irq = gpio_to_irq(143);

	if (irq < 0)
		pr_info("%s failed to set gpio 143 irq\n", __func__);

	ret = request_irq(irq, gpioPmTest_isr,
		IRQF_DISABLED | IRQF_TRIGGER_FALLING | IRQF_NO_SUSPEND,
		"gpioTestPM-irq", test_string);
	if (ret)
		pr_info("%s, failed request irq.\n", __func__);


	/* enable gpio 143 event  */

	event_id = GPIO143_A_EVENT;

	if (event_id >= GPIO29_A_EVENT && event_id <= SPARE10_A_EVENT)
		event_id = GPIO29_A_EVENT;
	if (event_id >= GPIO29_B_EVENT && event_id <= SPARE10_B_EVENT)
		event_id = GPIO29_B_EVENT;


	pwr_mgr_event_trg_enable(GPIO143_A_EVENT, PM_TRIG_POS_EDGE);

	cfg.policy = 1;
	pwr_mgr_event_set_pi_policy(event_id, PI_MGR_PI_ID_MODEM, &cfg);

	cfg.policy = 5;
	pwr_mgr_event_set_pi_policy(event_id, PI_MGR_PI_ID_ARM_CORE, &cfg);

	cfg.policy = 5;
	pwr_mgr_event_set_pi_policy(event_id, PI_MGR_PI_ID_ARM_SUB_SYSTEM, &cfg);

	cfg.policy = 5;
	pwr_mgr_event_set_pi_policy(event_id, PI_MGR_PI_ID_HUB_AON, &cfg);

	cfg.policy = 5;
	pwr_mgr_event_set_pi_policy(event_id, PI_MGR_PI_ID_HUB_SWITCHABLE, &cfg);

	pwr_mgr_register_event_handler(GPIO143_A_EVENT, gpio143Callback, test_string);
	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(pm_set_gpio_event, NULL, set_pm_gpio_event, "%llu\n");



static struct dentry *dent_island_pm_test_dir;

int __init island_pm_test_init(void)
{

	int i;
	for (i = 0; i < ARRAY_SIZE(board_test_pin_config); i++)
		pinmux_set_pin_config(&board_test_pin_config[i]);


	/* create island_pm_test */
	dent_island_pm_test_dir = debugfs_create_dir("island_pm_test", 0);

	if (!dent_island_pm_test_dir)
		return -ENOMEM;

	if (!debugfs_create_file("set_gpio_event", S_IWUSR|S_IRUSR, dent_island_pm_test_dir, NULL, &pm_set_gpio_event))
		return -ENOMEM;


	    return 0;
}
late_initcall(island_pm_test_init);

#endif


