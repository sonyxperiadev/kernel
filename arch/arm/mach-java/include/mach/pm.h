#ifndef __HAWAII_PM_H__
#define __HAWAII_PM_H__

#ifndef __ASSEMBLY__
#include <linux/suspend.h>
#endif

#define KONA_MACH_MAX_IDLE_STATE 1

/* Uncomment this to profile time taken by the dormant
 * mode code. Use the following commands to control
 * dormant mode profiling:
 * 1. Enable profiling:
 *    echo d p 1 <ns_gpio> <sec_gpio> <ref_gpio>
 * 2. Disable profiling:
 *    echo d p 0 0 0 0
 * Three GPIOs are used in profiling the time taken by
 * dormant mode code (in both Linux side and ABI-EXT).
 *    a. ref_gpio: This GPIO is turned ON at dormant sequence
 *       start and turned OFF at dormant sequence end. This GPIO
 *       is used to create a reference on the oscilloscope to
 *       correctly determine the start and end of one dormant
 *       entry.
 *    b. ns_gpio: This GPIO is used to time the dormant sequence
 *       in the Linux side (till SMC in entry and from reset handler
 *       to the end in the exit path). Two pulses are seen on ns_gpio
 *       under one pulse of ref_gpio.
 *    c. sec_gpio: Set this value to the GPIO used in ABI-EXT. If ABI-EXT
 *       does not include the GPIO toggle code for timing, then give
 *       a number which does not conflict with either of the above(this
 *       is only a piece of test code and hence does not do any fancy
 *       sanity check!).
 */
/* #define DORMANT_PROFILE */


/* LPM trace values */
#define LPM_TRACE_ENTER_WFI		0x10
#define LPM_TRACE_EXIT_WFI		0x20

#define LPM_TRACE_ENTER_SYSPLL_WFI		0x40
#define LPM_TRACE_SYSPLL_WFI_SET_FREQ	0x42
#define LPM_TRACE_SYSPLL_WFI_RES_FREQ	0x44
#define LPM_TRACE_EXIT_SYSPLL_WFI		0x46

#define LPM_TRACE_ENTER_DRMNT		0x60
#define LPM_TRACE_DRMNT_SAVE_REG	0x61
#define LPM_TRACE_DRMNT_L2_OFF		0x62
#define LPM_TRACE_DRMNT_SAVE_SHRD	0x63
#define LPM_TRACE_DRMNT_SAVE_PROC	0x64
#define LPM_TRACE_DRMNT_SAVE_ADDNL	0x65
#define LPM_TRACE_DRMNT_CORE_CNT	0x66
#define LPM_TRACE_DRMNT_CPU_SUSPEND	0x67
#define LPM_TRACE_DRMNT_CNTNUE		0x68
#define LPM_TRACE_DRMNT_DIS_SMP		0x69
#define LPM_TRACE_DRMNT_SMC		0x6A
#define LPM_TRACE_DRMNT_WFI		0x6B
#define LPM_TRACE_DRMNT_WFI_EXIT	0x6C
#define LPM_TRACE_DRMNT_WAKEUP		0x6D
#define LPM_TRACE_DRMNT_L2_ON		0x6E
#define LPM_TRACE_DRMNT_RS1		0x6F
#define LPM_TRACE_DRMNT_RS2		0x70
#define LPM_TRACE_DRMNT_RS_PROC		0x71
#define LPM_TRACE_DRMNT_RS_ADDNL	0x72
#define LPM_TRACE_DRMNT_RS3		0x73
#define LPM_TRACE_DRMNT_RS4		0x74
#define LPM_TRACE_DRMNT_CPU_WAIT1	0x75
#define LPM_TRACE_DRMNT_CPU_WAIT2	0x76
#define LPM_TRACE_EXIT_DRMNT	0x77

/*Below traces are logged only by core-0*/
#define LPM_TRACE_DRMNT_ATMPT_CNT	0x78
#define LPM_TRACE_DRMNT_FAIL_CNT	0x79
#define LPM_TRACE_DRMNT_SUCSS_CNT	0x7A

#define LPM_TRACE_PARAM_SHIFT		16
#define LPM_TRACE_PARAM_MASK		0xFFFF
#define LPM_TRACE_VAL_MASK		0xFFFF

#define DRMT_FAILURE_PARAM		0x1000

#define DEEP_SLEEP_LATENCY     8000 /*latency due to xtal warm up delay*/

#ifndef __ASSEMBLY__

/**
 * C State definitions and latencies
 */
enum {
	CSTATE_SIMPLE_WFI, /* Simple WFI */
	CSTATE_SYSPLL_WFI, /*26MHz WFI*/
	CSTATE_CORE_DRMT, /*Core dormant*/
	CSTATE_SUSPEND_DRMT, /* suspend dormant */
	CSTATE_DS_DRMT, /* deep sleep dormant */
};

/*Cstate Exit latency*/
enum {
	EXIT_LAT_SIMPLE_WFI = 0,
	EXIT_LAT_SYSPLL_WFI = 100,
	EXIT_LAT_CORE_DRMT = 600,
	/* Worst case dormant sequence delay */
	EXIT_LAT_SUSPEND_DRMT = 2000,
	/*dormant latency + xtal warmup delay*/
	EXIT_LAT_DS_DRMT = DEEP_SLEEP_LATENCY + 2000,
};

/*CState target residency values*/
enum {
	TRGT_RESI_SIMPLE_WFI = 0,
	TRGT_RESI_SYSPLL_WFI = 100,
	TRGT_RESI_CORE_DRMT = 600,
	TRGT_RESI_SUSPEND_DRMT = EXIT_LAT_SUSPEND_DRMT + 2000,
	TRGT_RESI_DS_DRMT = EXIT_LAT_DS_DRMT + 5000,
};
#endif

#ifndef __ASSEMBLY__
extern u32 dormant_start(void);
extern int dormant_attempt;

/* Set this to 0 to disable retention  mode tracing code */
#define RETENTION_TRACE_ENABLE        1
#define RETENTION_ENTRY               0xACACACAC
#define RETENTION_EXIT                0xABABABAB

/* Set this to 0 to disable wfi mode tracing code */
#define WFI_TRACE_ENABLE        1
#define WFI_ENTRY               0xBABABABA
#define WFI_EXIT                0xBCBCBCBC

enum {
	TRACE_ENTRY,
	TRACE_EXIT
};


#ifdef DORMANT_PROFILE
/* Vars exported by dm_pwr_policy_top.S for dormant profiling */
extern u32 dormant_profile_on;
extern u32 ns_gpio_set_p;
extern u32 ns_gpio_clr_p;
extern u32 ns_gpio_set_v;
extern u32 ns_gpio_clr_v;
extern u32 ns_gpio_bit;

extern void clear_ns_gpio(void);
#endif /* DORMANT_PROFILE */

extern void enter_wfi(void);
extern void dbg_gpio_set(u32 gpio);
extern void dbg_gpio_clr(u32 gpio);
extern int force_sleep(suspend_state_t state);

#ifdef CONFIG_EARLYSUSPEND
extern void request_suspend_state(suspend_state_t state);
#else
static inline void request_suspend_state(suspend_state_t state) { }
#endif
extern void instrument_lpm(u16 trace, u16 param);
extern int get_force_sleep_state(void);
extern int pm_is_forced_sleep(void);

#endif /* __ASSEMBLY__ */

#endif /*__HAWAII_PM_H__*/
