#ifndef __RHEA_PM_H__
#define __RHEA_PM_H__

#define KONA_MACH_MAX_IDLE_STATE 1

#ifndef __ASSEMBLY__
/*
 * Any change in this structure should reflect in the definition
 * in the asm file (arch/arm/mach-rhea/dm_pwr_policy_top.S).
 */
struct dormant_gpio_data {
	s32 enable;
	u32 gpio_set_p;
	u32 gpio_set_v;
	u32 gpio_clr_p;
	u32 gpio_clr_v;
	u32 gpio_bit;
};

extern u32 dormant_start(void);

/* Variables exported by asm code */
extern struct dormant_gpio_data dormant_gpio_data;
#endif /* __ASSEMBLY__ */

#endif /*__RHEA_PM_H__*/
