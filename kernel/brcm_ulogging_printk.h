/* ----------------------------------------------------------- */
/* Unified Logging */

extern void BCMLOG_LogString(const char *inLogString,
				unsigned short inSender);

int bcmlog_mtt_on;
unsigned short bcmlog_log_ulogging_id;

/* ------------------------------------------------------------ */
int brcm_retrive_early_printk()
{
	/* int printed_len = length; */
	unsigned long flags;
	int this_cpu;
	/* char *p = data; */

	preempt_disable();
	/* This stops the holder of brcm_console_sem just where we want him */
	raw_local_irq_save(flags);
	this_cpu = smp_processor_id();

	/*
	 * Ouch, printk recursed into itself!
	 */
	if (unlikely(printk_cpu == this_cpu)) {
		/*
		 * If a crash is occurring during printk() on this CPU,
		 * then try to get the crash message out but make sure
		 * we can't deadlock. Otherwise just return to avoid the
		 * recursion and return - but flag the recursion so that
		 * it can be printed at the next appropriate moment:
		 */
		if (!oops_in_progress) {
			recursion_bug = 1;
			goto end_restore_irqs;
		}
		zap_locks();
	}

	lockdep_off();
	spin_lock(&logbuf_lock);
	printk_cpu = this_cpu;

	if (bcmlog_log_ulogging_id > 0)
		BCMLOG_LogString(log_buf, bcmlog_log_ulogging_id);

	/*
	 * Try to acquire and then immediately release the
	 * brcm_console semaphore. The release will do all the
	 * actual magic (print out buffers, wake up klogd,
	 * etc).
	 *
	 * The acquire_brcm_console_semaphore_for_printk() function
	 * will release 'logbuf_lock' regardless of whether it
	 * actually gets the semaphore or not.
	 */
	if (acquire_console_semaphore_for_printk(this_cpu))
		release_console_sem();

	lockdep_on();

end_restore_irqs:
	raw_local_irq_restore(flags);

	preempt_enable();
	return 0;
}

