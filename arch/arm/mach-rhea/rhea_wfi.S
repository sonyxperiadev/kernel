#include <linux/linkage.h>
#include <asm/assembler.h>
#include <asm/asm-offsets.h>
#include <mach/io_map.h>
#include <mach/rdb/brcm_rdb_csr.h>

ENTRY(enter_wfi)

	stmfd   sp!,{r0-r5, lr}

	wait_for_self_refresh:
	ldr	r4, = (KONA_MEMC0_NS_VA + CSR_DDR_POWER_DOWN_STATUS_OFFSET)
	ldr	r0, [r4]
	cmp	r0, #03
	@bne	wait_for_self_refresh

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



