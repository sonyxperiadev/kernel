#ifndef _LINUX_BUG_H
#define _LINUX_BUG_H

#include <asm/bug.h>
#include <linux/compiler.h>
#include <linux/build_bug.h>

enum bug_trap_type {
	BUG_TRAP_TYPE_NONE = 0,
	BUG_TRAP_TYPE_WARN = 1,
	BUG_TRAP_TYPE_BUG = 2,
};

struct pt_regs;

#ifdef __CHECKER__
#else /* __CHECKER__ */

#define BUILD_BUG() BUILD_BUG_ON_MSG(1, "BUILD_BUG failed")

#endif	/* __CHECKER__ */

#ifdef CONFIG_GENERIC_BUG
#include <asm-generic/bug.h>

static inline int is_warning_bug(const struct bug_entry *bug)
{
	return bug->flags & BUGFLAG_WARNING;
}

const struct bug_entry *find_bug(unsigned long bugaddr);

enum bug_trap_type report_bug(unsigned long bug_addr, struct pt_regs *regs);

/* These are defined by the architecture */
int is_valid_bugaddr(unsigned long addr);

#else	/* !CONFIG_GENERIC_BUG */

static inline enum bug_trap_type report_bug(unsigned long bug_addr,
					    struct pt_regs *regs)
{
	return BUG_TRAP_TYPE_BUG;
}

#endif	/* CONFIG_GENERIC_BUG */

#ifdef CONFIG_PANIC_ON_DATA_CORRUPTION
#define PANIC_CORRUPTION 1
#else
#define PANIC_CORRUPTION 0
#endif  /* CONFIG_PANIC_ON_DATA_CORRUPTION */
#endif	/* _LINUX_BUG_H */
