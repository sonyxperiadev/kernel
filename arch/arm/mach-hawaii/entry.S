        @ Created by arm_to_gnu.pl from entry.s
        .syntax unified

        @  Copyright (c) 2009-11, ARM Limited. All rights reserved.
        @  
        @  Redistribution and use in source and binary forms, with or without
        @  modification, are permitted provided that the following conditions are met:
        
        @   * Redistributions of source code must retain the above copyright notice,
        @     this list of conditions and the following disclaimer.
        @   * Redistributions in binary form must reproduce the above copyright notice,
        @     this list of conditions and the following disclaimer in the documentation
        @     and/or other materials provided with the distribution.
        @   * Neither the name of ARM nor the names of its contributors may be used to
        @     endorse or promote products derived from this software without specific
        @     prior written permission.
        
        @   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
        @   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
        @   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
        @   ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
        @   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
        @   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
        @   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
        @   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
        @   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
        @   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
        @   POSSIBILITY OF SUCH DAMAGE.
        
        .section APPF_ENTRY_POINT_CODE,"ax"     
        
		@DORMANT_TBD
        @.eabi_attribute Tag_ABI_align_preserved, 1  
        
    .equ APPF_FUNCTION_INITIALIZE, 0
        
        .global appf_entry_point    
        .global appf_reset_entry_point  
        .global appf_return_to_os   
        .global appf_runtime_call_flat_mapped   
        .global appf_ttbr0  
        .global appf_ttbcr  
        
        
        
        
        
        
        
        
        
        
    .equ SCTLR_I, (1<<12)
    .equ SCTLR_Z, (1<<11)
        
        
appf_entry_point:               
        @ This is the entry point from the OS
        @ Save some stuff on the OS stack (preserve 8 byte alignment)
        push	{r4-r12, lr}   
        
        @ Preserve arguments
        mov	r5, r0  
        mov	r6, r1  
        mov	r7, r2  
        mov	r8, r3  
        
        @ Is this runtime initialization?
        cmp	r0, #APPF_FUNCTION_INITIALIZE   
        bne	1f  
        
        @ Runtime intialization (build translation tables, etc)
        bl	appf_runtime_init    
        adr	r1, appf_return_to_os_address   
        adr	r2, appf_return_to_os   
        str	r2, [r1]    
        
        @ Get new stack pointer
1:                  bl	appf_platform_get_stack_pointer  
        mov	r11, r0			  @ save SP value for later
        
        @ Restore arguments
        mov	r0, r5  
        mov	r1, r6  
        mov	r2, r7  
        mov	r3, r8  
        
        @ This function uses the following registers:
        @ r12 - OS stack pointer
        @ r11 - APPF stack pointer
        @ r10 - APPF TTBR0
        @  r9 - APPF TTBCR
        @  r5 - APPF DACR
        @  r8 - OS TTBR0
        @  r7 - OS TTBCR
        @  r6 - OS DACR
        @  r4 - OS CPSR
        
        @ Disable interrupts
        mrs	r4, CPSR    
        cpsid	if    
        
        @ Get APPF values for TTBR0, TTBCR, DACR
        adr	r10, appf_ttbr0     
        adr	r9, appf_ttbcr  
        ldr	r10, [r10]  
        ldr	r9, [r9]    
        mov	r5, #0xffffffff     
        mov	r12, #0     
        
        @ Get current TTBR0, TTBCR, DACR
        mrc	p15, 0, r8, c2, c0, 0	  @ read TTBR0
        mrc	p15, 0, r7, c2, c0, 2	  @ read TTBCR
        mrc	p15, 0, r6, c3, c0, 0	  @ read DACR
        
        @ Switch translation tables to APPF
        mcr	p15, 0, r10, c2, c0, 0	     @ write TTBR0
        mcr	p15, 0,  r9, c2, c0, 2	     @ write TTBCR
        mcr	p15, 0,  r5, c3, c0, 0	     @ write DACR
        dsb     
        mcr	p15, 0, r12, c8, c7, 0	     @ Inv both TLBs
        mcr	p15, 0, r12, c7, c5, 6	     @ Inv branch predictor array
        dsb		   @ Wait for the preceding operations to finish
        isb		   @ Now re-fetch following instructions
        
        @ Switch stacks, save OS stack pointer and TTBR0, TTBCR
        mov	r12, sp     
        mov	sp, r11     
        push	{r4-r8,r12}		  @ see comment in appf_reset_entry_point below
        
        adr	r7, appf_runtime_call_flat_mapped   
        ldr	r7, [r7]    
        
        blx	r7  
        
appf_return_to_os:              
        
        @ retrieve OS stack pointer, TTBR0, TTBCR, DACR
        pop	{r4-r8,r12}     
        
        @ Switch stacks
        mov	sp, r12     
        mov	r12, #0     
        
        @ Switch translation tables back to OS
        mcr	p15, 0, r8, c2, c0, 0	  @ write TTBR0
        mcr	p15, 0, r7, c2, c0, 2	  @ write TTBCR
        mcr	p15, 0, r6, c3, c0, 0	  @ write DACR
        dsb				     @ Wait for the preceding operations to finish
        mcr	p15, 0, r12, c8, c7, 0	     @ Inv both TLBs
        mcr	p15, 0, r12, c7, c5, 6	     @ Inv branch predictor array
        dsb				     @ Wait for the preceding operations to finish
        isb				     @ Now re-fetch following instructions
        
        @ Restore interrupts
        msr	CPSR_c, r4  
        
        @ Return to OS
        pop	{r4-r12, pc}    
        
        
appf_reset_entry_point:             
        @ This is the entry point from the platform warm start code
        @ Single argument: r0 is non-zero if caches need invalidating
        mov	r4, r0  
        
        @ Get stack pointer
        bl	appf_platform_get_stack_pointer  
        sub	r0, r0, #24		   @ The 6 registers PUSHed above are still valid!
        mov	sp, r0  
        
        cmp	r4, #0  
        beq	0f  
        
        @ Clear I cache levels to PoU (also clears BTBs)
        bl	invalidate_icache_v7_pou     
        
        @ Turn I cache and branch prediction on
0:                  bl	read_sctlr   
        orr	r0, r0, #(SCTLR_I  |  SCTLR_Z)  
        bl	write_sctlr  
        
        cmp	r4, #0  
        beq	1f  
        
        @ Clear all data cache levels visible to CPU
        bl	invalidate_dcache_v7_all     
        
        @ Caches have been invalidated, and any other init completed.
        @ Stack pointer is valid, and I cache on
        @ So we are ready to go to the C code
        
1:                  bl	appf_warm_reset  
        adr	r1, appf_return_to_os_address   
        ldr	r1, [r1]    
        bx	r1   
        
        
appf_runtime_call_flat_mapped:              
        .word 0     
        
appf_return_to_os_address:              
        .word 0     
        
appf_ttbr0:             
        .word 0     
        
appf_ttbcr:             
        .word 0     
        
        .end    
