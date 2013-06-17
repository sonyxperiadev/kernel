        @ Created by arm_to_gnu.pl from a9.s
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
        
        
        .global save_a9_other   
        .global restore_a9_other    
        
        .global save_a5_other   
        .global restore_a5_other    
        
        
save_a9_other:          .func   
        mrc	p15,0,r12,c15,c0,0	     @ Read Power Control Register
        str	r12, [r0], #4   
        mrc     p15, 4, r12, c15, c0, 0     @ Read Configuration Base Address Register
        str	r12, [r0], #4   
        
        mrc	p15,0,r3,c0,c0,0	   @ Read Main ID Register
        ubfx	r3, r3, #20, #4		  @ Extract major version number
        cmp	r3, #2  
        blt	1f			   @ PLE only possible in r2p0 onwards
        mrc	p15,0,r3,c11,c0,0	  @ Read PLE IDR
        cmp	r3, #0  
        beq	1f			   @ No PLE present
        
        mrc	p15,0,r3,c11,c1,0	  @ Read PLE UAR
        mrc	p15,0,r12,c11,c1,1	     @ Read PLE PCR
        stm	r0!, {r3, r12}  
        
1:                  bx	lr	  
        .endfunc    
        
restore_a9_other:           .func   
        cmp	r1, #0			   @ Check we are secure
        ldr	r12, [r0], #4   
        andne	r12, r12, #0x01		     @ We only restore the Dynamic Clock gating bit
        mcrne	p15,0,r12,c15,c0,0	   @ Write Power Control Register (if secure)
        ldr	r12, [r0], #4   
        mcrne   p15, 4, r12, c15, c0, 0	    @ Write Configuration Base Address Register (if Secure)
        
        mrc	p15,0,r3,c0,c0,0	   @ Read Main ID Register
        ubfx	r3, r3, #20, #4		  @ Extract major version number
        cmp	r3, #2  
        blt	1f			   @ PLE only possible in r2p0 onwards
        mrc	p15,0,r3,c11,c0,0	  @ Read PLE IDR
        cmp	r3, #0  
        beq	1f			   @ No PLE present
        
        ldm	r0!, {r3, r12}  
        mcr	p15,0,r3,c11,c1,0	  @ Write PLE UAR
        mcr	p15,0,r12,c11,c1,1	     @ Write PLE PCR
        
1:                  bx	lr	  
        .endfunc    
        
save_a5_other:          .func   
        cmp	r1, #0			   @ Check we are secure
        mrcne	p15, 5, r12, c15, c0, 0	  @ Read TLB Hitmap Register
        strne	r12, [r0], #4     
        bx	lr   
        .endfunc    
        
restore_a5_other:           .func   
        cmp	r1, #0			   @ Check we are secure
        ldrne	r12, [r0], #4     
        mcrne	p15, 5, r12, c15, c0, 0	  @ Write TLB Hitmap Register (Secure only!)
        bx	lr   
        .endfunc    
        
        
        
        
        .end    
        
        
        
