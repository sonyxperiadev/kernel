/*
 * arch/arm/mach-bcm2708/include/mach/entry-macro.S
 *
 * Low-level IRQ helper macros for BCM2708 platforms
 *
 * This file is licensed under  the terms of the GNU General Public
 * License version 2. This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */
#include <mach/hardware.h>

		.macro	disable_fiq
		.endm

		.macro  get_irqnr_preamble, base, tmp
		ldr	\base, =IO_ADDRESS(ARMCTRL_IC_BASE)
		.endm

		.macro  arch_ret_to_user, tmp1, tmp2
		.endm

		.macro	get_irqnr_and_base, irqnr, irqstat, base, tmp
		ldr	\irqstat, [\base, #(ARM_IRQ_PEND0 - ARMCTRL_IC_BASE)]	@ get masked status
		mov	\irqnr, #(ARM_IRQ0_BASE + 31)
		teq	\irqstat, #0
		beq	1010f

      @ For non-zero x, LSB(x) = 31 - CLZ(x^(x-1))
      @ N.B. CLZ is an ARM5 instruction.
      sub   \tmp, \irqstat, #1
      eor   \irqstat, \irqstat, \tmp
      clz   \tmp, \irqstat
      sub   \irqnr, \tmp

      cmp   \irqnr, #INTERRUPT_PENDING1
      blt   1010f
      cmp   \irqnr, #INTERRUPT_PENDING2
      bgt   1010f

      ldrne \irqstat, [\base, #(ARM_IRQ_PEND1 - ARMCTRL_IC_BASE)]	@ get masked status
      ldreq \irqstat, [\base, #(ARM_IRQ_PEND2 - ARMCTRL_IC_BASE)]	@ get masked status
      movne \irqnr, #(ARM_IRQ1_BASE + 31)
      moveq \irqnr, #(ARM_IRQ2_BASE + 31)
		teq	\irqstat, #0
		beq	1010f

      @ For non-zero x, LSB(x) = 31 - CLZ(x^(x-1))
      @ N.B. CLZ is an ARM5 instruction.
      sub   \tmp, \irqstat, #1
      eor   \irqstat, \irqstat, \tmp
      clz   \tmp, \irqstat
      sub   \irqnr, \tmp

1010:	@ EQ will be set if no irqs pending
		.endm
