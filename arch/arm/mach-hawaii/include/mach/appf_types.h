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
 
#define APPF_ENTRY_POINT_SIZE (32 * 1024)

#define CPU_A5             0x4100c050
#define CPU_A8             0x4100c080
#define CPU_A9             0x4100c090
#define CPU_A15            0x4100c0f0

#define APPF_INITIALIZE             0
#define APPF_POWER_DOWN_CPU         1
#define APPF_POWER_UP_CPUS          2

/*
 * Return codes
 */
#define APPF_EARLY_RETURN        1 
#define APPF_OK                  0
#define APPF_BAD_FUNCTION      (-1)
#define APPF_BAD_CSTATE        (-2)
#define APPF_BAD_RSTATE        (-3)
#define APPF_NO_MEMORY_MAPPING (-4)
#define APPF_BAD_CLUSTER       (-5)
#define APPF_BAD_CPU           (-6)
#define APPF_CPU_NOT_SHUTDOWN  (-7)

/*
 * Values for flags parameter
 */
#define APPF_SAVE_PMU          (1U<<0)
#define APPF_SAVE_TIMERS       (1U<<1)
#define APPF_SAVE_GLOBAL_TIMER (1U<<2)
#define APPF_SAVE_VFP          (1U<<3)
#define APPF_SAVE_DEBUG        (1U<<4)


typedef unsigned int appf_u32;
typedef signed int appf_i32;


typedef appf_i32 (appf_entry_point_t)(appf_u32, appf_u32, appf_u32, appf_u32);

struct appf_main_table
{
    appf_u32 ram_start;
    appf_u32 ram_size;
    appf_u32 num_clusters;
    struct appf_cluster            *cluster_table;
    appf_entry_point_t *entry_point;
};

struct appf_cluster_context;
struct appf_cpu_context;

/*
 * A cluster is a container for CPUs, typically either a single CPU or a 
 * coherent cluster.
 * We assume the CPUs in the cluster can be switched off independently.
 */
struct appf_cluster
{
    appf_u32 cpu_type;                /* A9mpcore, A5mpcore, etc                  */
    appf_i32 num_cpus;
    volatile appf_i32 *pactive_cpus;  /* Initialized to number of cpus present    */
    appf_u32 scu_address;             /*  0 => no SCU                             */
    appf_u32 ic_address;              /*  0 => no Interrupt Controller            */
    appf_u32 l2_address;              /*  0 => no L2CC                            */
    struct appf_cluster_context *s_context;
    struct appf_cluster_context *ns_context;
    struct appf_cpu *cpu_table;
    volatile appf_i32 *ppower_state;
    void *lock;
};

struct appf_cpu
{
    appf_u32 ic_address;              /*  0 => no Interrupt Controller            */
    struct appf_cpu_context *s_context;
    struct appf_cpu_context *ns_context;
    volatile appf_u32 power_state;
};

