        @ Created by arm_to_gnu.pl from a8.s
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
        
        
        .global save_a8_other   
        .global restore_a8_other    
        
save_a8_other:          .func   
        @ First wait for any running Preloads to finish
0:                  mrc	p15,0,r3,c11,c0,2	  @ Read PLE Identification and Status Register running
        cmp	r3, #0  
        bne	0b  
        
        @ Now check which PLE channels are present
        mrc	p15,0,r12,c11,c0,0	     @ Read PLE IDR
        cmp	r12, #0     
        beq	20f			  @ No PLE present
        
        @ Read the channel number register to store later
        mrc	p15,0,r3,c11,c2,0	  @ Read PLE Channel Number Register
        
        @ Save channel 0 (if present)
        tst	r12, #1     
        beq	10f     
        
        mov	r1, #0  
        mcr	p15,0,r1,c11,c2,0	  @ Write PLE Channel Number Register
        mrc	p15,0,r1,c11,c4,0	  @ Read PLE Control Register
        mrc	p15,0,r2,c11,c5,0	  @ Read PLE Internal Start Address Register
        stm	r0!, {r1, r2}   
        mrc	p15,0,r1,c11,c7,0	  @ Read PLE Internal End Address Register
        mrc	p15,0,r2,c11,c15,0	     @ Read PLE Context ID Register
        stm	r0!, {r1, r2}   
        
        @ Save channel 1 (if present)
10:                 tst	r12, #2     
        beq	15f     
        
        mov	r1, #2  
        mcr	p15,0,r1,c11,c2,0	  @ Write PLE Channel Number Register
        mrc	p15,0,r1,c11,c4,0	  @ Read PLE Control Register
        mrc	p15,0,r2,c11,c5,0	  @ Read PLE Internal Start Address Register
        stm	r0!, {r1, r2}   
        mrc	p15,0,r1,c11,c7,0	  @ Read PLE Internal End Address Register
        mrc	p15,0,r2,c11,c15,0	     @ Read PLE Context ID Register
        stm	r0!, {r1, r2}   
        
        @ Save channel number register
15:                 str	r3, [r0], #4    
        
20:                 bx	lr	  
        .endfunc    
        
restore_a8_other:           .func   
        @ First check which PLE channels are present
        mrc	p15,0,r12,c11,c0,0	     @ Read PLE IDR
        cmp	r12, #0     
        beq	20f			  @ No PLE present
        
        @ Restore channel 0 (if present)
        tst	r12, #1     
        beq	10f     
        
        mov	r1, #0  
        mcr	p15,0,r1,c11,c2,0	  @ Write PLE Channel Number Register
        ldm	r0!, {r1, r2}   
        mcr	p15,0,r1,c11,c4,0	  @ Write PLE Control Register
        mcr	p15,0,r2,c11,c5,0	  @ Write PLE Internal Start Address Register
        ldm	r0!, {r1, r2}   
        mcr	p15,0,r1,c11,c7,0	  @ Write PLE Internal End Address Register
        mcr	p15,0,r2,c11,c15,0	     @ Write PLE Context ID Register
        
10:                     @ Restore channel 1 (if present)
        tst	r12, #2     
        beq	15f     
        
        mov	r1, #1  
        mcr	p15,0,r1,c11,c2,0	  @ Write PLE Channel Number Register
        ldm	r0!, {r1, r2}   
        mcr	p15,0,r1,c11,c4,0	  @ Write PLE Control Register
        mcr	p15,0,r2,c11,c5,0	  @ Write PLE Internal Start Address Register
        ldm	r0!, {r1, r2}   
        mcr	p15,0,r1,c11,c7,0	  @ Write PLE Internal End Address Register
        mcr	p15,0,r2,c11,c15,0	     @ Write PLE Context ID Register
        
        @ Restore channel number register
15:                 ldr	r3, [r0], #4    
        mcr	p15,0,r3,c11,c2,0	  @ Write PLE Channel Number Register
        
20:                 bx	lr	  
        .endfunc    
        
        
        .end    
        
        
        
