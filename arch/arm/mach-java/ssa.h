        @ Created by arm_to_gnu.pl from ssa.inc
        .syntax unified

        @/*************************************************************************
        @ **     Copyright (C) 2010 Nokia Corporation. All rights reserved.      **
        @ **                                                                     **
        @ ** Permission is hereby granted, free of charge, to any person         **
        @ ** obtaining a copy of this software and associated documentation      **
        @ ** files (the "Software"), to deal in the Software without             **
        @ ** restriction, including without limitation the rights to use, copy,  **
        @ ** modify, merge, publish, distribute, sublicense, and/or sell copies  **
        @ ** of the Software, and to permit persons to whom the Software is      **
        @ ** furnished to do so, subject to the following conditions:            **
        @ **                                                                     **
        @ ** The above copyright notice and this permission notice shall be      **
        @ ** included in all copies or substantial portions of the Software.     **
        @ ** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,     **
        @ ** EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF  **
        @ ** MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND               **
        @ ** NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS **
        @ ** BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN  **
        @ ** ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN   **
        @ ** CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE    **
        @ ** SOFTWARE.                                                           **
        @ **                                                                     **
        @ *************************************************************************
        @ **                                                                     **
        @ ** File:  ssa.inc                                                      **
        @ **                                                                     **
        @ ** Desc:  Secure ROM internal Interface for Secure Service API         **
        @ **                                                                     **
        @ *************************************************************************/
        
        @ Following definitions are used to tell public side what kind of security
        @ mode exit shall be done. These are a copy of definitions in ssa.h
        
    .equ SEC_EXIT_NORMAL, 1
    .equ SEC_EXIT_START_EXT_CODE, 2
    .equ SEC_EXIT_INT, 3
    .equ SEC_EXIT_RPC_CALL, 4
        
        
        @ SSA service IDs. These are a copy of definitions in ssa.h
        
    .equ SSAPI_PRE_INIT_SERV, 1
    .equ SSAPI_POST_SPEEDUP_INIT_SERV, 2
    .equ SSAPI_ISSW_IMPORT_SERV, 3
    .equ SSAPI_RET_FROM_INT_SERV, 4
    .equ SSAPI_RET_FROM_RPC_SERV, 5
    .equ SSAPI_ISSW_EXECUTE_SERV, 6
        
        
        
        @ Following definitions are a copy of definitions in ssa.h
        
    .equ SEC_ROM_ICACHE_ENABLE_MASK, 1
    .equ SEC_ROM_DCACHE_ENABLE_MASK, 2
    .equ SEC_ROM_IRQ_ENABLE_MASK, 4
    .equ SEC_ROM_FIQ_ENABLE_MASK, 8
    .equ SEC_ROM_UL2_CACHE_ENABLE_MASK, 16
        
        
        @ Following definitions are a copy of a part of definitions in patch_id.h
        
    .equ SEC_PATCH_BOOT_FAILURE, 1
    .equ SEC_PATCH_SLA_RESET_REQUEST, 2
        
        @ Following definitions are a copy of a part of definitions in sla_types.h
    .equ SEC_RESET_DORMANT_CHECK_FAILURE, 14
        
        @ Following definitions are a copy of a part of definitions in ssa.h
    .equ SEC_ROM_RET_ENTRY_FAILURE, 0x000E
        
        
        @ Following is PROM trace address 
    .equ ROM_TIME_STAMP_ADDR, 0x34052000
    .equ ASIC_TIMER_REG, 0x3E00D004
    .equ TIME_STAMP_ID, 0x454D4954
        
    .equ CHIPREG_BOOT_2ND_ADDR, 0x3500417C
        
        @/* vim: set autoindent shiftwidth=8 smarttab expandtab : */
        @/* -*- mode: C; c-basic-indent: 8; indent-tabs-mode: nil -*- */
