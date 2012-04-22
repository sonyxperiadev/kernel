/*
  Copyright (c) 2009-11, ARM Limited. All rights reserved.
  
  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

   * Redistributions of source code must retain the above copyright notice,
     this list of conditions and the following disclaimer.
   * Redistributions in binary form must reproduce the above copyright notice,
     this list of conditions and the following disclaimer in the documentation
     and/or other materials provided with the distribution.
   * Neither the name of ARM nor the names of its contributors may be used to
     endorse or promote products derived from this software without specific
     prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
   POSSIBILITY OF SUCH DAMAGE.
*/
 
#include "appf_types.h"
#include "appf_internals.h"
#include "appf_helpers.h"

typedef struct
{
    /* 0x00 */  volatile unsigned int control;
    /* 0x04 */  const unsigned int configuration;
    /* 0x08 */  union
                {
                    volatile unsigned int w;
                    volatile unsigned char b[4];
                } power_status;
    /* 0x0c */  volatile unsigned int invalidate_all;
                char padding1[48];
    /* 0x40 */  volatile unsigned int filtering_start;
    /* 0x44 */  volatile unsigned int filtering_end;
                char padding2[8];
    /* 0x50 */  volatile unsigned int access_control;
    /* 0x54 */  volatile unsigned int ns_access_control;
} a9_scu_registers;

/* 
 * TODO: we need to use the power status register, not save it!
 */

void save_a9_scu(appf_u32 *pointer, unsigned scu_address)
{
    a9_scu_registers *scu = (a9_scu_registers *)scu_address;
    
    pointer[0] = scu->control;
    //pointer[1] = scu->power_status.w;
    pointer[2] = scu->filtering_start;
    pointer[3] = scu->filtering_end;
    pointer[4] = scu->access_control;
    pointer[5] = scu->ns_access_control;
}

void restore_a9_scu(appf_u32 *pointer, unsigned scu_address)
{
    a9_scu_registers *scu = (a9_scu_registers *)scu_address;
    
    scu->invalidate_all = 0xffff;
    scu->filtering_start = pointer[2];
    scu->filtering_end = pointer[3];
    scu->access_control = pointer[4];
    scu->ns_access_control = pointer[5];
    //scu->power_status.w = pointer[1];
    scu->control = pointer[0];
}

void set_status_a9_scu(unsigned cpu_index, unsigned status, unsigned scu_address)
{
    a9_scu_registers *scu = (a9_scu_registers *)scu_address;
    unsigned power_status;

    switch(status)
    {
        case STATUS_STANDBY:
        case STATUS_DORMANT:
            power_status = 2;
            break;
        case STATUS_SHUTDOWN:
            power_status = 3;
            break;
        default:
            power_status = 0;
    }
        
    scu->power_status.b[cpu_index] = power_status;
    dsb();
}

int num_cpus_from_a9_scu(unsigned scu_address)
{
    a9_scu_registers *scu = (a9_scu_registers *)scu_address;

    return ((scu->configuration) & 0x3) + 1;
}
