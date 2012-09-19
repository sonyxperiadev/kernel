#include <linux/linkage.h>
#include <asm/assembler.h>

	.text

ENTRY(invalidate_tlb_btac)
	stmfd	sp!, { lr }

	/* Invalidate the TLBs & BTAC */
	mov r0, #0
	mcr p15, 0, r0, c8, c3, 0   @ invalidate shared TLBs
	mcr p15, 0, r0, c7, c1, 6   @ invalidate shared BTAC
	dsb
	isb

	ldmfd	sp!, { pc }
ENDPROC(invalidate_tlb_btac)
