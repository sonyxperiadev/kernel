
@  Copyright (c) 2009-11, ARM Limited. All rights reserved.
@
@  Redistribution and use in source and binary forms, with or without
@  modification, are permitted provided that the following conditions are met:
@
@   * Redistributions of source code must retain the above copyright notice,
@     this list of conditions and the following disclaimer.
@   * Redistributions in binary form must reproduce the above copyright notice,
@     this list of conditions and the following disclaimer in the documentation
@     and/or other materials provided with the distribution.
@   * Neither the name of ARM nor the names of its contributors may be used to
@     endorse or promote products derived from this software without specific
@     prior written permission.
@
@   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
@   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
@   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
@   ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
@   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
@   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
@   SUBSTITUTE GOODS OR SERVICES
@ 	LOSS OF USE, DATA, OR PROFITS	@ OR BUSINESS
@   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
@   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
@   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
@   POSSIBILITY OF SUCH DAMAGE.

@****************************************************************************
@	* The contents of the original source code is minimally modified to suit
@ 	* the needs of the environment in which this code is being used.
@*****************************************************************************/


	.section APPF,"ax"

	.global save_a15_l2
	.global restore_a15_l2

save_a15_l2:          .func
	mrc	p15, 1, r1, c9, c0, 2	@ L2CTLR
	mrc	p15, 1, r2, c9, c0, 3	@ L2ECTLR
	mrc	p15, 1, r3, c15, c0, 0	@ L2ACTLR
	mrc	p15, 1, r12, c15, c0, 3	@ L2PFR
	stm	r0!, {r1,r2,r3,r12}
	mrrc	p15, 1, r2, r3, c15	@ L2MERRSR
	stm	r0!, {r2,r3}
	bx	lr
	.endfunc

	@ Restoring L2  MUST be done while Secure, MMU MUST be off
restore_a15_l2:          .func
	ldm	r0!, {r1,r2,r3,r12}
	mcr	p15, 1, r1, c9, c0, 2	@ L2CTLR
	mcr	p15, 1, r2, c9, c0, 3	@ L2ECTLR
	mcr	p15, 1, r3, c15, c0, 0	@ L2ACTLR
	mcr	p15, 1, r12, c15, c0, 3	@ L2PFR

	ldm	r0!, {r2,r3}
	mcrr	p15, 1, r2, r3, c15	@ L2MERRSR

	bx	lr
	.endfunc


	.end



