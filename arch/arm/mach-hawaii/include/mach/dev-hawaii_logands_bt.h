#ifndef __ARM_ARCH_DEV_HAWAII_LOGANDS_BT_H
#define __ARM_ARCH_DEV_HAWAII_LOGANDS_BT_H

#if (defined(CONFIG_BCM_RFKILL) || defined(CONFIG_BCM_RFKILL_MODULE))
extern struct platform_device hawaii_bcmbt_rfkill_device;
#endif

#ifdef CONFIG_BCM_BZHW
extern struct platform_device hawaii_bcm_bzhw_device;
#endif

#ifdef CONFIG_BCM_BT_LPM
extern struct platform_device board_bcmbt_lpm_device;
#endif

void __init hawaii_bt_init(void);


#endif