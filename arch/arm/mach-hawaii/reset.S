        @ Created by arm_to_gnu.pl from reset.s
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
        
        .section APPF,"ax"  
        
        
        
        
        .global platform_cpu_stacks     
        .global platform_reset_handler  
        .global appf_platform_get_stack_pointer     
        
    .equ CPU_A5, 0x4100c050
        
		@DORMANT_TBD
        @.eabi_attribute Tag_ABI_align_preserved, 1  
        
platform_reset_handler:             
        @ TODO: Restore Endianness bit(s) in CPSR
        @ TODO: Check for a warm start - either here or in boot monitor
        
        @ Work out whether caches need to be invalidated: A9 - yes, A6 - no
        @ TODO: If the cpu has been in an L1-retention state, don't invalidate
        bl	get_cpu_type     
        ldr	r1, =CPU_A5     
        cmp	r0, r1  
        moveq	r0, #0    
        movne	r0, #1    
        
        b	appf_reset_entry_point    
        
        
        .section APPF_ENTRY_POINT_CODE,"ax"     
        
        
appf_platform_get_stack_pointer:                
        adr	r1, platform_cpu_stacks     
        mrc	p15, 0, r0, c0, c0, 5	  @ Read MPIDR
        and	r0, r0, #0xff		     @ extract CPU number
        ldr	r0, [r1, r0, lsl #2]    
        bx	lr           
        
platform_cpu_stacks:                
        .space 16,0     
        
        .end    
