        @ Created by arm_to_gnu.pl from table_data.s
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
        
		@DORMANT_TBD
        .section APPF_ENTRY_POINT_CODE,"ax"     
        @.section APPF_TABLE_DATA,"aw"   
        @.eabi_attribute Tag_ABI_align_preserved, 1
        
		.align 12
        .global main_table  
        
main_table:             
        .space 256,0    
        
        
		@DORMANT_TBD
        .section APPF_ENTRY_POINT_CODE,"ax"     
        @.section APPF_TRANSLATION_TABLE2,"aw"   
        @.eabi_attribute Tag_ABI_align_preserved, 1  
        
		.align 10
        .global appf_translation_table2a    
        .global appf_translation_table2b    
        
appf_translation_table2a:               
        .space 1024,0   
appf_translation_table2b:               
        .space 1024,0   
        
        
		@DORMANT_TBD
        .section APPF_ENTRY_POINT_CODE,"ax"     
        @.section APPF_TRANSLATION_TABLE1,"aw"   
        @.eabi_attribute Tag_ABI_align_preserved, 1  
        
		.align 14
        .global appf_translation_table1     
        
appf_translation_table1:                
        .space 16384,0  
        
		@DORMANT_TBD
        .section APPF_ENTRY_POINT_CODE,"ax"     
        @.section APPF_DEVICE_MEMORY,"aw"    
        @.eabi_attribute Tag_ABI_align_preserved, 1  
        
		.align 12
        .global appf_device_memory  
        
appf_device_memory:             
        .space 4096,0   
        
        
        .end    
