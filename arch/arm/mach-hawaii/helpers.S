        @ Created by arm_to_gnu.pl from helpers.s
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
        
        .global dmb     
        .global wfi_loop
        .global wfi     
        .global wfe     
        .global sev     
        .global copy_words  
        .global appf_memcpy     
        .global appf_memset     
        .global read_mvbar  
        .global read_cbar   
        .global read_drar   
        .global read_dsar   
        .global write_osdlr     
        .global get_cpu_type    
        
        .global dsb     
        .global read_mpidr      
        .global va_to_pa    
        .global read_sctlr  
        .global write_sctlr     
        .global read_actlr  
        .global write_actlr     
        .global read_prrr   
        .global read_nmrr   
        .global read_l2ctlr     
        
    .equ MIDR_CPU_MASK, 0xff00fff0
        
        .section APPF,"ax"  
        
        
        
dmb:                .func   
        dmb     
        bx	lr   
        .endfunc    
        
wfi_loop:                .func   
        wfi     
        b wfi_loop
        .endfunc    

wfi:                .func   
        wfi     
        bx	lr   
        .endfunc    
        
wfe:                .func   
        wfe     
        bx	lr   
        .endfunc    
        
sev:                .func   
        sev     
        bx	lr   
        .endfunc    
        
        @ This function takes three arguments
        @ r0: Destination start address (must be word aligned)
        @ r1: Source start address (must be word aligned)
        @ r2: Number of words to copy
        @ Return value is updated destination pointer (first unwritten word)
copy_words:         .func   
        cmp	r2, #0  
        beq	1f  
0:                  ldr	r3, [r1], #4    
        str	r3, [r0], #4    
        subs	r2, r2, #1     
        bne	0b  
1:                  bx	lr   
        .endfunc    
        
        
appf_memcpy:            .func   
        cmp	r2, #0  
        bxeq	lr     
0:                  ldrb	r3, [r1], #1   
        strb	r3, [r0], #1   
        subs	r2, #1     
        bne	0b  
        bx	lr   
        .endfunc    
        
appf_memset:            .func   
        cmp	r2, #0  
        bxeq	lr     
0:                  strb	r1, [r0], #1   
        subs	r2, #1     
        bne	0b  
        bx	lr   
        .endfunc    
        
read_mvbar:         .func   
        mrc	p15, 0, r0, c12, c0, 1  
        bx	lr   
        .endfunc    
        
read_cbar:          .func   
        mrc	p15, 4, r0, c15, c0, 0	     @ Read Configuration Base Address Register
        bx	lr   
        .endfunc    
        
read_drar:          .func   
        mrc	p14, 0, r0, c1, c0, 0	  @ Read Debug ROM Address Register
        bx	lr   
        .endfunc    
        
read_dsar:          .func   
        mrc	p14, 0, r0, c2, c0, 0	  @ Read Debug Self Address Offset Register
        bx	lr   
        .endfunc    
        
write_osdlr:            .func   
        mcr	p14, 0, r0, c1, c3, 4	  @ Write OS Double Lock Register
        bx	lr   
        .endfunc    
        
get_cpu_type:           .func   
        mrc	p15, 0, r0, c0, c0, 0   @ read MIDR
        ldr	r1, =MIDR_CPU_MASK  
        ands	r0, r1     
        bx	lr   
        .endfunc    
        
        .section APPF_ENTRY_POINT_CODE,"ax"     
        
        
        @ Functions we need in the runtime entry point, i.e. before we switch pagetables,
        @ are placed in this area.
        
dsb:                .func   
        dsb     
        bx	lr   
        .endfunc    
        
read_mpidr:         .func   
        mrc	p15, 0, r0, c0, c0, 5   
        bx	lr   
        .endfunc    
        
va_to_pa:           .func	  @ Note: assumes conversion will be successful!
        mov	r1, r0  
        mcr	p15, 0, r0, c7, c8, 1	  @ Priv Write Current World VA-PA
        mrc	p15, 0, r0, c7, c4, 0	  @ Get PA
        bfc	r0, #0, #12		   @ We want top bits of translated addr
        bfc	r1, #12, #20		  @ plus bottom bits of input addr
        orr	r0, r0, r1  
        bx	lr   
        .endfunc    
        
read_sctlr:         .func           
        mrc	p15, 0, r0, c1, c0, 0   
        bx	lr   
        .endfunc    
        
write_sctlr:            .func   
        mcr	p15, 0, r0, c1, c0, 0   
        isb     
        bx	lr   
        .endfunc    
        
read_actlr:         .func           
        mrc	p15, 0, r0, c1, c0, 1   
        bx	lr   
        .endfunc    
        
write_actlr:            .func   
        mcr	p15, 0, r0, c1, c0, 1   
        isb     
        bx	lr   
        .endfunc    
        
read_prrr:          .func           
        mrc	p15, 0, r0, c10, c2, 0  
        bx	lr   
        .endfunc    
        
read_nmrr:          .func           
        mrc	p15, 0, r0, c10, c2, 1  
        bx	lr   
        .endfunc    
        
read_l2ctlr:            .func   
        mrc	p15, 1, r0, c9, c0, 2   
        bx	lr   
        .endfunc    
        
        .end    
