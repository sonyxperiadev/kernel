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
#include "appf_platform_api.h"
#include "appf_internals.h"
#include "appf_helpers.h"

/**
 * This function powers up a CPU
 *
 * It is entered with cluster->lock held.
 */
int appf_platform_power_up_cpu(struct appf_cpu *cpu,
                               struct appf_cluster *cluster)
{
    if (cpu->power_state == 1)
    {
        dsb();
//        send_interrupt_gic(cluster->ic_address);
    }
    else
    {
        /* Wake another CPU from deeper sleep */
        
#ifdef USE_REALVIEW_EB_RESETS
        int cpu_index = cpu - cluster->cpu_table;
        dsb();
        /* Unlock system registers */
        *(volatile unsigned *)0x10000020 = 0xa05f;
        /* Cause the model to take the CPU out of reset */
        *(volatile unsigned *)0x10000074 |= 1 << (6 + cpu_index);
#endif
    }
    /* TODO: Add code to handle a cpu in another cluster */
    
    return APPF_OK;
}

/**
 * This function tells the PCU to power down the executing CPU, and possibly the whole cluster.
 *
 * It is entered with cluster->lock held.
 */
int appf_platform_enter_cstate(unsigned cpu_index, struct appf_cpu *cpu, struct appf_cluster *cluster)
{
    /*
     * This is where the code to talk to the PCU goes. We could check the value
     * of cpu->power_state, but we don't, since we can only do run or shutdown.
     */
    if (cluster->scu_address)
    {
//        set_status_a9_scu(cpu_index, STATUS_SHUTDOWN, cluster->scu_address);
    }

    return APPF_OK;
}

/**
 * This function tells the PCU this CPU has finished powering up.
 *
 * It is entered with cluster->lock held.
 */
int appf_platform_leave_cstate(unsigned cpu_index, struct appf_cpu *cpu, struct appf_cluster *cluster)
{
    /*
     * We could check the value in the SCU power status register as it is written at 
     * power up by the PCU. But we don't, since we know we're always powering up.
     */
    if (cluster->scu_address)
    {
//        set_status_a9_scu(cpu_index, STATUS_RUN, cluster->scu_address);
    }
    
    return APPF_OK;
}

/**
 * This function puts the executing CPU, and possibly the whole cluster, into STANDBY
 *
 * It is entered with cluster->lock held.
 */
int appf_platform_enter_cstate1(unsigned cpu_index, struct appf_cpu *cpu, struct appf_cluster *cluster)
{
//    if (cluster->power_state == 1 && cluster->l2_address)
//    {
//        set_status_pl310(STATUS_STANDBY, cluster->l2_address);
//    }

    return APPF_OK;
}
    
/**
 * This function removes the executing CPU, and possibly the whole cluster, from STANDBY
 *
 * It is entered with cluster->lock held.
 */
int appf_platform_leave_cstate1(unsigned cpu_index, struct appf_cpu *cpu, struct appf_cluster *cluster)
{
//    if (cluster->l2_address)
//    {
//        set_status_pl310(STATUS_RUN, cluster->l2_address);
//    }

    return APPF_OK;
}

