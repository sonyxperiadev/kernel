#ifndef __RHEA_PM_H__
#define __RHEA_PM_H__

#ifndef __ASSEMBLY__
#include <linux/suspend.h>
#endif

#define KONA_MACH_MAX_IDLE_STATE 1

/* Set this to 0 to disable dormant mode tracing code */
#define DORMANT_TRACE_ENABLE        1
#define DORMANT_ENTRY               0xF0F0F0F0
#define DORMANT_EXIT                0xE0E0E0E0
/* Following macro values should be loadable via a single
 * mov instruction.
 */
#define DORMANT_RESTORE1_START      0xF1
#define DORMANT_RESTORE1_END        0xF2
#define DORMANT_RESTORE2_START      0xF3
#define DORMANT_CTRL_PROG_START     0xE0
#define DORMANT_CTRL_PROG_END       0xE1

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

extern int rhea_force_sleep(suspend_state_t state);
extern void request_suspend_state(suspend_state_t state);
extern void instrument_dormant_entry(void);
extern void instrument_dormant_exit(void);
extern int get_force_sleep_state(void);
#endif /* __ASSEMBLY__ */

#endif /*__RHEA_PM_H__*/
