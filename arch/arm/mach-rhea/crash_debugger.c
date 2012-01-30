/* arch/arm/mach-rhea/crash_debugger.c
 *
 * Derived from sec_debug.c by Samsung Electronics Co.Ltd.
 * Copyright (c) 2010 Samsung Electronics Co., Ltd.
 *      http://www.samsung.com/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <linux/errno.h>
#include <linux/ctype.h>
#include <linux/notifier.h>
#include <linux/reboot.h>
#include <linux/input.h>
#include <linux/delay.h>
#include <linux/sysrq.h>
#include <asm/cacheflush.h>
#include <asm/io.h>
#include <linux/sched.h>
#include <linux/smp.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/uaccess.h>
#include <linux/proc_fs.h>

#include <mach/hardware.h>
#include <mach/system.h>
#include <mach/cdebugger.h>
#include <linux/fs.h>

enum cdebugger_upload_cause_t {
    UPLOAD_CAUSE_INIT = 0xCAFEBABE,
    UPLOAD_CAUSE_KERNEL_PANIC = 0x000000C8,
    UPLOAD_CAUSE_FORCED_UPLOAD = 0x00000022,
    UPLOAD_CAUSE_CP_ERROR_FATAL = 0x000000CC,
    UPLOAD_CAUSE_USER_FAULT = 0x0000002F,
};

struct cdebugger_mmu_reg_t {
    int SCTLR;
    int TTBR0;
    int TTBR1;
    int TTBCR;
    int DACR;
    int DFSR;
    int FAR;
    int IFSR;
    int DAFSR;
    int IAFSR;
    int PMRRR;
    int NMRRR;
    int FCSEPID;
    int CONTEXT;
    int URWTPID;
    int UROTPID;
    int POTPIDR;
};

/* ARM CORE regs mapping structure */
struct cdebugger_core_t {
    /* COMMON */
    unsigned int r0;
    unsigned int r1;
    unsigned int r2;
    unsigned int r3;
    unsigned int r4;
    unsigned int r5;
    unsigned int r6;
    unsigned int r7;
    unsigned int r8;
    unsigned int r9;
    unsigned int r10;
    unsigned int r11;
    unsigned int r12;

    /* SVC */
    unsigned int r13_svc;
    unsigned int r14_svc;
    unsigned int spsr_svc;

    /* PC & CPSR */
    unsigned int pc;
    unsigned int cpsr;

    /* USR/SYS */
    unsigned int r13_usr;
    unsigned int r14_usr;

    /* FIQ */
    unsigned int r8_fiq;
    unsigned int r9_fiq;
    unsigned int r10_fiq;
    unsigned int r11_fiq;
    unsigned int r12_fiq;
    unsigned int r13_fiq;
    unsigned int r14_fiq;
    unsigned int spsr_fiq;

    /* IRQ */
    unsigned int r13_irq;
    unsigned int r14_irq;
    unsigned int spsr_irq;

    /* ABT */
    unsigned int r13_abt;
    unsigned int r14_abt;
    unsigned int spsr_abt;

    /* UNDEF */
    unsigned int r13_und;
    unsigned int r14_und;
    unsigned int spsr_und;

};

struct cdebugger_fault_status_t {
    /* COMMON */
    unsigned int r0;
    unsigned int r1;
    unsigned int r2;
    unsigned int r3;
    unsigned int r4;
    unsigned int r5;
    unsigned int r6;
    unsigned int r7;
    unsigned int r8;
    unsigned int r9;
    unsigned int r10;
    unsigned int r11;
    unsigned int r12;
    unsigned int r13;
    unsigned int r14;
    unsigned int r15;
    unsigned int cpsr;
    unsigned int cur_process_magic;
};

/* enable cdebugger feature */
static unsigned enable = 1;
/*SRAM base address*/
void __iomem *cdebugger_mem_base;
module_param_named(enable, enable, uint, 0644);

static const char *gkernel_cdebugger_build_info_date_time[] = {
    __DATE__,
    __TIME__
};

static char gkernel_cdebugger_build_info[100];

struct cdebugger_core_t cdebugger_core_reg;
struct cdebugger_mmu_reg_t cdebugger_mmu_reg;
enum cdebugger_upload_cause_t cdebugger_upload_cause;

struct cdebugger_fault_status_t cdebugger_fault_status;

struct T_RAMDUMP_BLOCK {
	unsigned int mem_start;
	unsigned int mem_size;
	/* 0xFFFFFFFF means stand-alone ramdump block */
	unsigned int buffer_in_main;
	char name[8];		/* one of names must be "MAIN" */
};

struct BCMLOG_Fifo_t {
	unsigned char		*buf_ptr ;			///< the buffer
	unsigned long		 buf_sz ;			///< buffer size
	unsigned long		 idx_read ;			///< index of first data byte
	unsigned long		 idx_write ;			///< index of first free byte
};

extern struct BCMLOG_Fifo_t g_fifo;

const char linkSignature[120] = "Link Signature:  LINK_SIGNATURE";
const char decoder_version[] = {"!!! SDL decoder: hspa_11_11_22.zip"};
const unsigned int num_of_ramdumps = 1;
const unsigned int mmu_unit_size = (1<< PAGE_SHIFT);
struct T_RAMDUMP_BLOCK ramdump_list[16] = {
{PHYS_OFFSET, 0x20000000, 0xFFFFFFFF, "MAIN"},
};

static struct TLV_android {
	unsigned char type;
	unsigned char name[30];
	unsigned int length;
	unsigned char *buf;
};

extern unsigned char _buf_log_main[512*1024];
extern unsigned char _buf_log_radio[256*1024];
extern unsigned char _buf_log_events[256*1024];
extern unsigned char _buf_log_system[256*1024];
extern char *log_buf;
extern int log_buf_len;
#define LOG_BUFFER	1

static struct TLV_android main_log = {
	.type	=	LOG_BUFFER,
	.name	=	"android_main_log",
	.length	=	512*1024,
};

static struct TLV_android radio_log = {
	.type	=	LOG_BUFFER,
	.name	=	"android_radio_log",
	.length	=	256*1024,
};

static struct TLV_android events_log = {
	.type	=	LOG_BUFFER,
	.name	=	"android_events_log",
	.length	=	256*1024,
};

static struct TLV_android system_log = {
	.type	=	LOG_BUFFER,
	.name	=	"android_system_log",
	.length	=	256*1024,
};

void *log_tx_param[] __aligned(8) =
{
	(void *)0x4150FEFF,
	(void *)0x7F576656,	/* begin flag */
	0,	/* (void *)&log_sio_tx_buffer[0], */	// log_tx_buf, hard-coded by compiler, no SW initialization needed
	0,	/* (void *)LOG_TX_SIZE, */			// log_tx_buf_size, hard-coded by compiler, no SW initialization needed
	0,					// log_wr_index
	0,					// log_rd_index
	0,	/* (void *)&sdltrace_q, */
	(void *)linkSignature,
	0,
	0,					// sl1r_versionString
	0,					// DSP_version,
	(void *)decoder_version,
	0,					// ts_assert_begin, filled by asserting()
	0,					// expr passed to asserting()
	0,					// file passed to asserting()
	0,					// line passed to asserting()
	0,					// value passed to asserting()
	0,					// his_log_switch filled by asserting()
	0,	/* (void *)&taskmempool, */
	(void *)&mmu_unit_size,
	0,	/* (void *)&task_his[0], */
	0,	/* (void *)&int_his[0], */
	0,	/* (void *)&task_int_total, */
	0,	/* (void *)&osheap_history[0], */
	0,	/* (void *)&osheap_history_size, */
	0,	/* &wtt_log_buffer[0], */
	0,	/* WTT_LOG_BUFFER_SIZE, */
	0,	/* &wtt_log_read_idx, */
	0,	/* &wtt_log_write_idx, */
	0,	/*(void *)&sl1r_buf, */         // Legacy code commented  H.Luo
	0,	/*(void *)sizeof(sl1r_buf), */  // Legacy code commented  H.Luo
	0,	/* (void *)&spinner_coredump_buffer[0], */
	0,	/* (void *)&spinner_coredump_length, */
	// Debug information version 0 ends here
	(void *)5,	// Debug information version number
	(void *)&cdebugger_fault_status,	// for backward compatibility//
	0, //(void *)&TCD_Current_Thread,
	0, //(void *)&TCD_System_Stack,
	0, //(void *)&TCD_Created_Tasks_List,
	0, //(void *)&TCD_Created_HISRs_List,
	0, //(void *)&TMD_Created_Timers_List,
	0, //(void *)&TMD_Created_List_Protect,
	0, //(void *)&IOD_Created_Drivers_List,
	0, //(void *)&DMD_Created_Pools_List,
	0, //(void *)&PMD_Created_Pools_List,
	0, //(void *)&QUD_Created_Queues_List,
	0, //(void *)&EVD_Created_Event_Groups_List,
	0, //(void *)&MBD_Created_Mailboxes_List,
	0, //(void *)&PID_Created_Pipes_List,
	0, //(void *)&SMD_Created_Semaphores_List,
	0,  //(void *)procmap_table,         // Legacy code commented  H.Luo
	0,  //(void *)&procmap_table_size,   // Legacy code commented  H.Luo
	0,  //(void *)sigmap_table,          // Legacy code commented  H.Luo
	0,  //(void *)&sigmap_table_size,    // Legacy code commented  H.Luo
	0,  //(void *)statemap_table,        // Legacy code commented  H.Luo
	0,  //(void *)&statemap_table_size,  // Legacy code commented  H.Luo
	// Debug information version 1 ends here
	(void *)2, // MMU-based virtual memory data structure version
	(void *)4, // memory debug data structure version
	(void *)&ramdump_list[0],
	(void *)&num_of_ramdumps,
	// Debug information version 2 ends here, Hui Luo, 10/12/07
	0,	/* (void *)&ossemaphore_history[0], */
	0,	/* (void *)&ossemaphore_history_size, */
	0,	/* (void *)&osqueue_history[0], */
	0,	/* (void *)&osqueue_history_size, */
	0,	/* (void *)&oseventgroup_history[0], */
	0,	/* (void *)&oseventgroup_history_size, */
	0,	/* (void *)&ostimer_history[0], */
	0,	/* (void *)&ostimer_history_size, */
	0,	/* (void *)dump_assert_log, */
	0,	/* (void *)SIM_LOG_ADDR, */
	0,	/* (void *)SIM_AP_DEBUG_DATA, */
	0,	/* (void *)&sim_flag, */
	0,	/* (void *)&ITCM_buffer[0], */
	0,	/* (void *)ITCM_SIZE, */
	0,	/* (void *)ITCM_MVA_BASE, */
	0,	/* (void *)ITCM_PAGE_SIZE, */
	0,	/* (void *)ITCM_VPAGE_SIZE, */
	0,	/* (void *)&DTCM_buffer[0], */
	0,	/* (void *)DTCM_SIZE, */
	0,	/* (void *)DTCM_BASE, */
	0,
	&cdebugger_mmu_reg,//
	// Debug information version 3 ends here, Hui Luo, 1/2/08
	0,	/* (void *)&REG_addr_buffer[0], ends with zero */
	0,	/* (void *)&REG_value_buffer[0], ends with zero */
	0,	/* (void *)&REG_buffer_size, */
	0,	/* (void *)&MMUL1Entry[0], always 4096 */
	/* Debug information version 4 ends here, Hui Luo, 9/24/09 */
	(void *)0x7A9,		/* 0x7A9 = CortexA9 */
	(void *)2,		/* 0=Nucleus, 1=ThreadX 2=Linux*/
	/* Debug information version 5 ends here, Hui Luo, 12/13/10 */
	&main_log,
	&radio_log,
	&events_log,
	&system_log,
	(void *)0x656675F7,
	(void *)0xFFEF0514	/* end flag */
};

EXPORT_SYMBOL(log_tx_param);

/* core reg dump function*/
static void cdebugger_save_core_reg(struct cdebugger_core_t *core_reg)
{

	/* we will be in SVC mode when we enter this function. Collect
	   SVC registers along with cmn registers. */
	asm("str r0, [%0,#0]\n\t"	/* R0 is pushed first to core_reg */
	    "mov r0, %0\n\t"		/* R0 will be alias for core_reg */
	    "str r1, [r0,#4]\n\t"	/* R1 */
	    "str r2, [r0,#8]\n\t"	/* R2 */
	    "str r3, [r0,#12]\n\t"	/* R3 */
	    "str r4, [r0,#16]\n\t"	/* R4 */
	    "str r5, [r0,#20]\n\t"	/* R5 */
	    "str r6, [r0,#24]\n\t"	/* R6 */
	    "str r7, [r0,#28]\n\t"	/* R7 */
	    "str r8, [r0,#32]\n\t"	/* R8 */
	    "str r9, [r0,#36]\n\t"	/* R9 */
	    "str r10, [r0,#40]\n\t"	/* R10 */
	    "str r11, [r0,#44]\n\t"	/* R11 */
	    "str r12, [r0,#48]\n\t"	/* R12 */
	    /* SVC */
	    "str r13, [r0,#52]\n\t"	/* R13_SVC */
	    "str r14, [r0,#56]\n\t"	/* R14_SVC */
	    "mrs r1, spsr\n\t"		/* SPSR_SVC */
	    "str r1, [r0,#60]\n\t"
	    /* PC and CPSR */
	    "sub r1, r15, #0x4\n\t"	/* PC */
	    "str r1, [r0,#64]\n\t"
	    "mrs r1, cpsr\n\t"		/* CPSR */
	    "str r1, [r0,#68]\n\t"
	    /* SYS/USR */
	    "mrs r1, cpsr\n\t"		/* switch to SYS mode */
	    "and r1, r1, #0xFFFFFFE0\n\t"
	    "orr r1, r1, #0x1f\n\t"
	    "msr cpsr,r1\n\t"
	    "str r13, [r0,#72]\n\t"	/* R13_USR */
	    "str r14, [r0,#76]\n\t"	/* R14_USR */
	    /* FIQ */
	    "mrs r1, cpsr\n\t"		/* switch to FIQ mode */
	    "and r1,r1,#0xFFFFFFE0\n\t"
	    "orr r1,r1,#0x11\n\t"
	    "msr cpsr,r1\n\t"
	    "str r8, [r0,#80]\n\t"	/* R8_FIQ */
	    "str r9, [r0,#84]\n\t"	/* R9_FIQ */
	    "str r10, [r0,#88]\n\t"	/* R10_FIQ */
	    "str r11, [r0,#92]\n\t"	/* R11_FIQ */
	    "str r12, [r0,#96]\n\t"	/* R12_FIQ */
	    "str r13, [r0,#100]\n\t"	/* R13_FIQ */
	    "str r14, [r0,#104]\n\t"	/* R14_FIQ */
	    "mrs r1, spsr\n\t"		/* SPSR_FIQ */
	    "str r1, [r0,#108]\n\t"
	    /* IRQ */
	    "mrs r1, cpsr\n\t"		/* switch to IRQ mode */
	    "and r1, r1, #0xFFFFFFE0\n\t"
	    "orr r1, r1, #0x12\n\t"
	    "msr cpsr,r1\n\t"
	    "str r13, [r0,#112]\n\t"	/* R13_IRQ */
	    "str r14, [r0,#116]\n\t"	/* R14_IRQ */
	    "mrs r1, spsr\n\t"		/* SPSR_IRQ */
	    "str r1, [r0,#120]\n\t"
	    /* ABT */
	    "mrs r1, cpsr\n\t"		/* switch to Abort mode */
	    "and r1, r1, #0xFFFFFFE0\n\t"
	    "orr r1, r1, #0x17\n\t"
	    "msr cpsr,r1\n\t"
	    "str r13, [r0,#136]\n\t"	/* R13_ABT */
	    "str r14, [r0,#140]\n\t"	/* R14_ABT */
	    "mrs r1, spsr\n\t"		/* SPSR_ABT */
	    "str r1, [r0,#144]\n\t"
	    /* UND */
	    "mrs r1, cpsr\n\t"		/* switch to undef mode */
	    "and r1, r1, #0xFFFFFFE0\n\t"
	    "orr r1, r1, #0x1B\n\t"
	    "msr cpsr,r1\n\t"
	    "str r13, [r0,#148]\n\t"	/* R13_UND */
	    "str r14, [r0,#152]\n\t"	/* R14_UND */
	    "mrs r1, spsr\n\t"		/* SPSR_UND */
	    "str r1, [r0,#156]\n\t"
	    /* restore to SVC mode */
	    "mrs r1, cpsr\n\t"		/* switch to SVC mode */
	    "and r1, r1, #0xFFFFFFE0\n\t"
	    "orr r1, r1, #0x13\n\t"
	    "msr cpsr,r1\n\t" :		/* output */
	    : "r"(core_reg)		/* input */
	    : "%r0", "%r1"		/* clobbered registers */
	);
	return;
}

static void cdebugger_save_mmu_reg(struct cdebugger_mmu_reg_t *mmu_reg)
{

	asm("mrc    p15, 0, r1, c1, c0, 0\n\t"	/* SCTLR */
	    "str r1, [%0]\n\t"
	    "mrc    p15, 0, r1, c2, c0, 0\n\t"	/* TTBR0 */
	    "str r1, [%0,#4]\n\t"
	    "mrc    p15, 0, r1, c2, c0,1\n\t"	/* TTBR1 */
	    "str r1, [%0,#8]\n\t"
	    "mrc    p15, 0, r1, c2, c0,2\n\t"	/* TTBCR */
	    "str r1, [%0,#12]\n\t"
	    "mrc    p15, 0, r1, c3, c0,0\n\t"	/* DACR */
	    "str r1, [%0,#16]\n\t"
	    "mrc    p15, 0, r1, c5, c0,0\n\t"	/* DFSR */
	    "str r1, [%0,#20]\n\t"
	    "mrc    p15, 0, r1, c6, c0,0\n\t"	/* FAR */
	    "str r1, [%0,#24]\n\t"
	    "mrc    p15, 0, r1, c5, c0,1\n\t"	/* IFSR */
	    "str r1, [%0,#28]\n\t"
	    "mrc    p15, 0, r1, c10, c2,0\n\t"	/* PMRRR */
	    "str r1, [%0,#44]\n\t"
	    "mrc    p15, 0, r1, c10, c2,1\n\t"	/* NMRRR */
	    "str r1, [%0,#48]\n\t"
	    "mrc    p15, 0, r1, c13, c0,0\n\t"	/* FCSEPID */
	    "str r1, [%0,#52]\n\t"
	    "mrc    p15, 0, r1, c13, c0,1\n\t"	/* CONTEXT */
	    "str r1, [%0,#56]\n\t"
	    "mrc    p15, 0, r1, c13, c0,2\n\t"	/* URWTPID */
	    "str r1, [%0,#60]\n\t"
	    "mrc    p15, 0, r1, c13, c0,3\n\t"	/* UROTPID */
	    "str r1, [%0,#64]\n\t"
	    "mrc    p15, 0, r1, c13, c0,4\n\t"	/* POTPIDR */
	    "str r1, [%0,#68]\n\t" :		/* output */
	    : "r"(mmu_reg)			/* input */
	    : "%r1", "memory"			/* clobbered register */
	);
}

static void cdebugger_save_context(void)
{
	unsigned long flags;
	local_irq_save(flags);
	cdebugger_save_mmu_reg(&cdebugger_mmu_reg);
	cdebugger_save_core_reg(&cdebugger_core_reg);
	local_irq_restore(flags);
}

void cdebugger_save_pte(void *pte, int task_addr) 
{

	memcpy(&cdebugger_fault_status, pte,sizeof(cdebugger_fault_status));
	cdebugger_fault_status.cur_process_magic =task_addr;

}

static void cdebugger_set_upload_magic(unsigned magic)
{

	iowrite32(magic,cdebugger_mem_base);

	flush_cache_all();

	outer_flush_all();
}

static int cdebugger_normal_reboot_handler(struct notifier_block *nb,
					   unsigned long l, void *p)
{
	cdebugger_set_upload_magic(0x0);

	return 0;
}

static void cdebugger_set_upload_cause(enum cdebugger_upload_cause_t type)
{
	cdebugger_upload_cause = type;

	/*
	 * Store the cause of crash.
	 * This can be read from the bootloader later on
	 */
	iowrite32(type,cdebugger_mem_base + 0x4);
}

static void cdebugger_hw_reset(void)
{
		arm_machine_restart('h', "upload");;
}

static void setup_log_buffer_address(void)
{
	main_log.buf 	= (void *) virt_to_phys((void *)_buf_log_main);
	radio_log.buf 	= (void *) virt_to_phys((void *)_buf_log_radio);
	events_log.buf 	= (void *) virt_to_phys((void *)_buf_log_events);
	system_log.buf 	= (void *) virt_to_phys((void *)_buf_log_system);
}

static int cdebugger_panic_handler(struct notifier_block *nb,
				   unsigned long l, void *buf)
{
	if (!enable)
		return -1;

	cdebugger_set_upload_magic(0x66262564);

	if (!strcmp(buf, "Forced Ramdump !!\n"))
		cdebugger_set_upload_cause(UPLOAD_CAUSE_FORCED_UPLOAD);
	else if (!strcmp(buf, "CP Crash"))
		cdebugger_set_upload_cause(UPLOAD_CAUSE_CP_ERROR_FATAL);
	else
		cdebugger_set_upload_cause(UPLOAD_CAUSE_KERNEL_PANIC);


	handle_sysrq('t');

	ramdump_list[0].mem_size = (num_physpages << PAGE_SHIFT);
	setup_log_buffer_address();

	log_tx_param[2] = (void *)virt_to_phys((void *)log_buf);
	log_tx_param[3] = (void *)log_buf_len;
	log_tx_param[4] = (void *)0;//wr index
	log_tx_param[5] = (void *)0;//rd index
	log_tx_param[7] = (void *)virt_to_phys((void *)linkSignature);
	log_tx_param[11] = (void *)virt_to_phys((void *)decoder_version);
	log_tx_param[19] = (void *)virt_to_phys((void *)&mmu_unit_size);
	log_tx_param[34] = (void *)virt_to_phys((void *)&cdebugger_fault_status);
	log_tx_param[57] = (void *)virt_to_phys((void *)&ramdump_list[0]);
	log_tx_param[58] = (void *)virt_to_phys((void *)&num_of_ramdumps);
	log_tx_param[80] = (void *)virt_to_phys((void *)&cdebugger_mmu_reg);
	log_tx_param[84] = (void *)(cdebugger_mmu_reg.TTBR0 & 0xFFFFC000);

	log_tx_param[87] = (void *) virt_to_phys((void *)&main_log);
	log_tx_param[88] = (void *) virt_to_phys((void *)&radio_log);
	log_tx_param[89] = (void *) virt_to_phys((void *)&events_log);
	log_tx_param[90] = (void *) virt_to_phys((void *) &system_log);

	cdebugger_dump_stack();

	flush_cache_all();
	outer_flush_all();

	cdebugger_hw_reset();

	return 0;
}

/*
 * Called from dump_stack()
 * This function call does not necessarily mean that a fatal error
 * had occurred. It may be just a warning.
 */
int cdebugger_dump_stack(void)
{
	if (!enable)
		return -1;

	cdebugger_save_context();

	return 0;
}

static struct notifier_block nb_reboot_block = {
	.notifier_call = cdebugger_normal_reboot_handler
};

static struct notifier_block nb_panic_block = {
	.notifier_call = cdebugger_panic_handler,
};

static void cdebugger_set_build_info(void)
{
	char *p = gkernel_cdebugger_build_info;
	sprintf(p, "Kernel Build Info : ");
	strcat(p, " Date:");
	strncat(p, gkernel_cdebugger_build_info_date_time[0], 12);
	strcat(p, " Time:");
	strncat(p, gkernel_cdebugger_build_info_date_time[1], 9);
}

__init int cdebugger_init(void)
{
	if (!enable)
		return -1;

	cdebugger_set_build_info();

	cdebugger_set_upload_magic(0xDEAFABCD);
	cdebugger_set_upload_cause(UPLOAD_CAUSE_INIT);

	register_reboot_notifier(&nb_reboot_block);

	atomic_notifier_chain_register(&panic_notifier_list, &nb_panic_block);

	return 0;
}

#define CDEBUGGER_SRAM_MEM_OFFSET 	0xBF9C
#define INTERNAL_SRAM_BASE_ADDR         0x34040000
#define SCRATCHRAM_BASE 		INTERNAL_SRAM_BASE_ADDR

unsigned int cdebugger_memory_init(void)
{
	printk("cdebugger_magic_init > \n");

	cdebugger_mem_base = ioremap_nocache(SCRATCHRAM_BASE +
					CDEBUGGER_SRAM_MEM_OFFSET, SZ_1K);

	if(cdebugger_mem_base == NULL){
		printk("cdebugger ioremap failed!\n");
		return (int)cdebugger_mem_base;
	}

	return 0;
}

static int ramdump_panic(struct notifier_block *this, unsigned long event, void *ptr)
{
	panic_timeout = 2;
	return NOTIFY_DONE;
}

static struct notifier_block panic_block = {
	.notifier_call = ramdump_panic,
};

static int __init crash_debugger_init(void)
{
	cdebugger_memory_init();
	/* ready to run */
	cdebugger_init();
	/*Initialize MAGIC*/
	iowrite32(UPLOAD_CAUSE_INIT,cdebugger_mem_base);
	atomic_notifier_chain_register(&panic_notifier_list, &panic_block);
	/* Any other relevant init */
	return 0;
}

static void crash_debugger_exit(void)
{

}

device_initcall(crash_debugger_init);
module_exit(crash_debugger_exit);

