/* arch/arm/kernel/crash_notes.c
 *
 * Copyright (C) 2012 Sony Mobile Communications AB.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include <linux/capability.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/percpu.h>
#include <linux/elf.h>
#include <linux/elfcore.h>
#include <linux/slab.h>
#include <linux/nmi.h>
#include <linux/reboot.h>
#include <linux/delay.h>

#include <asm/cacheflush.h>
#include <asm/crash_notes.h>

#ifdef CONFIG_RAMDUMP_TAGS
#include <linux/rdtags.h>
#endif

#define CRASH_NOTE_NAME "CORE"

#define CRASH_NOTE_MAGIC1 0xCAFEBABE
#define CRASH_NOTE_MAGIC2 0xC001BABE

/* If not set, flatmem is assumed */
#define CRASH_NOTE_VM_FLAG_SPARSEMEM	0x01
#define CRASH_NOTE_VM_FLAG_13SPLIT	0x02	/* 1GB/3GB user/kernel split */
#define CRASH_NOTE_VM_FLAG_22SPLIT	0x04	/* 2GB/2GB user/kernel split */
#define CRASH_NOTE_VM_FLAG_31SPLIT	0x08	/* 3GB/1GB user/kernel split */

/* Has VM information */
#define CRASH_NOTE_MAGIC_FLAG_VM		0x01
/* This is the crashing cpu */
#define CRASH_NOTE_MAGIC_FLAG_CRASHING_CPU	0x02
/* The build_id, product and variant members of crash_extras is present.
 * Process_name is also present,
 * and completed if CRASH_NOTE_MAGIC_FLAG_REGISTERS is set */
#define CRASH_NOTE_MAGIC_FLAG_BUILDINFO		0x04

#define CRASH_NOTE_SIZE (2*ALIGN(sizeof(struct elf_note), 4) + \
			 ALIGN(sizeof(CRASH_NOTE_NAME), 4) + \
			 ALIGN(sizeof(struct elf_prstatus), 4))

#define CRASH_NOTE_BYTES sizeof(struct crash_extras_t)

typedef u32 note_buf_t[CRASH_NOTE_SIZE / 4];
note_buf_t *crash_notes;

static inline void dump_regs(struct pt_regs *regs)
{
	/* dump critical general registers first */
	__asm__ __volatile__("str fp, %0" : "=m"(regs->ARM_fp));
	__asm__ __volatile__("str sp, %0" : "=m"(regs->ARM_sp));
	__asm__ __volatile__("str pc, %0" : "=m"(regs->ARM_pc));
	__asm__ __volatile__("str lr, %0" : "=m"(regs->ARM_lr));
	/* dump general registers that will be used later */
	__asm__ __volatile__("str r0, %0" : "=m"(regs->ARM_r0));
	__asm__ __volatile__("str r1, %0" : "=m"(regs->ARM_r1));
	__asm__ __volatile__("str r2, %0" : "=m"(regs->ARM_r2));
	__asm__ __volatile__("str r3, %0" : "=m"(regs->ARM_r3));
	__asm__ __volatile__("str r4, %0" : "=m"(regs->ARM_r4));
	__asm__ __volatile__("str r5, %0" : "=m"(regs->ARM_r5));
	__asm__ __volatile__("str r6, %0" : "=m"(regs->ARM_r6));
	__asm__ __volatile__("str r7, %0" : "=m"(regs->ARM_r7));
	__asm__ __volatile__("str r8, %0" : "=m"(regs->ARM_r8));
	__asm__ __volatile__("str r9, %0" : "=m"(regs->ARM_r9));
	__asm__ __volatile__("str r10, %0" : "=m"(regs->ARM_r10));
	__asm__ __volatile__("str ip, %0" : "=m"(regs->ARM_ip));
	/* The registers involved with processor states and cp states
	 * will not be changed in the above operation, so it is safe
	 * to dump them at last
	 */
	/* dump cpsr register */
	__asm__ __volatile__("mrs %0, cpsr" : "=r"(regs->ARM_cpsr));
}

void crash_notes_save_this_cpu(enum crash_note_save_type type,
			       unsigned int cpu)
{
	struct elf_prstatus prstatus;
	struct elf_note *note;
	u32 *buf;
	u32 *start;
	char process_name[TASK_COMM_LEN];

	buf = (u32 *)per_cpu_ptr(crash_notes, cpu);
	if (!buf)
		return;

	start = buf;
	memset(&prstatus, 0, sizeof(prstatus));
	prstatus.pr_pid = current->pid;

	BUG_ON(sizeof(prstatus.pr_reg) != sizeof(struct pt_regs));
	if (type != CRASH_NOTE_INIT) {
		dump_regs((struct pt_regs *)&prstatus.pr_reg);
		get_task_comm(process_name, current);
		/* These log prints may not be changed.
		 * Used by a number of dump analysis tools,
		 * ex. ramdump, crashtool, tl2elf...
		 */
		if (type == CRASH_NOTE_CRASHING)
			printk(KERN_ERR "&@panic_name@:*%s*\n", process_name);
		else if (type == CRASH_NOTE_STOPPING)
			printk(KERN_ERR "&@running_process@cpu%d:*%s*\n",
			       cpu, process_name);
	}

	note = (struct elf_note *)buf;
	note->n_namesz = sizeof(CRASH_NOTE_NAME);
	note->n_descsz = sizeof(prstatus);
	note->n_type = NT_PRSTATUS;
	buf += (sizeof(struct elf_note) + 3) / 4;
	memcpy(buf, CRASH_NOTE_NAME, note->n_namesz);
	buf += (note->n_namesz + 3) / 4;
	memcpy(buf, &prstatus, sizeof(prstatus));
	buf += (note->n_descsz + 3) / 4;

	/* Set the final note */
	note = (struct elf_note *)buf;
	note->n_namesz = 0;
	note->n_descsz = 0;
	note->n_type   = 0;

#ifdef CONFIG_RAMDUMP_TAGS
	if (type == CRASH_NOTE_CRASHING ||
		type == CRASH_NOTE_STOPPING) {
		char tmp_buf[24];
		snprintf(tmp_buf, sizeof(tmp_buf),
			"%s_proc_name_cpu%u",
			type == CRASH_NOTE_CRASHING ? "crash" : "halted",
			cpu);
		rdtags_add_tag_string(
				tmp_buf,
				process_name);
	}
#endif

	/* Make sure we have crash_notes in ram before reset */
	flush_cache_all();	/* L1 to L2 */
	outer_flush_all();	/* L2 to Ram */
}
EXPORT_SYMBOL(crash_notes_save_this_cpu);

static int update_crash_notes(struct notifier_block *this,
			      unsigned long event, void *ptr)
{
	long i;
	crash_notes_save_this_cpu(CRASH_NOTE_CRASHING, smp_processor_id());

	bust_spinlocks(0);

	if (panic_timeout > 0) {
		/*
		 * Delay timeout seconds before rebooting the machine.
		 * We can't use the "normal" timers since we just panicked.
		 */
		printk(KERN_EMERG "Rebooting in %d seconds..", panic_timeout);

		for (i = 0; i < panic_timeout; i++) {
			touch_nmi_watchdog();
			mdelay(MSEC_PER_SEC);
		}
		/*
		 * This will not be a clean reboot, with everything
		 * shutting down.  But if there is a chance of
		 * rebooting the system it will be rebooted.
		 */
		emergency_restart();
	}
	local_irq_enable();
	while (1) {
		touch_softlockup_watchdog();
		mdelay(MSEC_PER_SEC);
	}

	return NOTIFY_DONE;
}

static struct notifier_block panic_block = {
	.notifier_call = update_crash_notes,
	.priority = INT_MIN /* will not return; must be done last */
};

static int __init crash_notes_init(void)
{
	int i;

	/* Allocate memory for saving cpu registers. */
	crash_notes = alloc_percpu(note_buf_t);
	if (!crash_notes) {
		printk(KERN_ERR "crash: Memory allocation for saving cpu "\
		       "register states failed\n");
		return -ENOMEM;
	}

	/* Initialize memory with something that the tools pick up on */
	/* It will NOT be useful register info, but it's something at least */
	for_each_possible_cpu(i) {
		crash_notes_save_this_cpu(CRASH_NOTE_INIT, i);
	}

	atomic_notifier_chain_register(&panic_notifier_list, &panic_block);

	return 0;
}
module_init(crash_notes_init)
