#include <linux/linkage.h>
#include <asm/assembler.h>
#include <asm/asm-offsets.h>
#include <mach/io_map.h>
#include <mach/rdb/brcm_rdb_csr.h>

ENTRY(enter_wfi)

	stmfd   sp!,{r0-r5, lr}

	mov	r0, r0
	mov	r0, r0
	mov	r0, r0
	mov	r0, r0
	mov	r0, r0
	mov	r0, r0
	mov	r0, r0
	mov	r0, r0
	mov	r0, r0
	mov	r0, r0
	mov	r0, r0
	mov	r0, r0
	mov	r0, r0
	mov	r0, r0
	mov	r0, r0
	mov	r0, r0
	mov	r0, r0
	mov	r0, r0
	mov	r0, r0
	mov	r0, r0
	mov	r0, r0
	mov	r0, r0
	mov	r0, r0
	mov	r0, r0
	mov	r0, r0
	mov	r0, r0
	mov	r0, r0
	mov	r0, r0
	mov	r0, r0
	mov	r0, r0
	dmb
	wfi
	dmb
	mov	r0, r0
	mov	r0, r0
	mov	r0, r0
	mov	r0, r0
	mov	r0, r0
	mov	r0, r0
	mov	r0, r0
	mov	r0, r0
	mov	r0, r0
	mov	r0, r0
	mov	r0, r0
	mov	r0, r0
	mov	r0, r0
	mov	r0, r0
	mov	r0, r0
	mov	r0, r0
	mov	r0, r0
	mov	r0, r0
	mov	r0, r0
	mov	r0, r0
	mov	r0, r0
	mov	r0, r0
	mov	r0, r0
	mov	r0, r0
	mov	r0, r0
	mov	r0, r0
	mov	r0, r0
	mov	r0, r0
	mov	r0, r0
	mov	r0, r0
	mov	r0, r0
	mov	r0, r0
	mov	r0, r0
	mov	r0, r0
	ldmfd   sp!,{r0-r5, pc}



