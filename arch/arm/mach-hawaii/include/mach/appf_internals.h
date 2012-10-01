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

 
#define TRUE  1
#define FALSE 0

#define A9_SMP_BIT     (1<<6)
#define A15_SMP_BIT    (1<<6)
#define MPIDR_U_BIT    (1<<30)

/*
 * Global variables 
 */
extern int is_smp;
extern struct appf_main_table main_table;
extern appf_u32 appf_translation_table1[];
extern appf_u32 appf_translation_table2[];
extern unsigned appf_runtime_call_flat_mapped;
extern unsigned appf_device_memory_flat_mapped;

/*
 * Entry points
 */
extern appf_entry_point_t appf_entry_point;
extern int appf_warm_reset(void);

/*
 * Context save/restore
 */
extern int save_cpu_context(struct appf_cluster *cluster, struct appf_cpu *cpu, unsigned flags);


int appf_setup_translation_tables(void);
int appf_setup_secure_translation_tables(void);

/* 
 * Power statuses
 */
#define STATUS_RUN      0
#define STATUS_STANDBY  1
#define STATUS_DORMANT  2
#define STATUS_SHUTDOWN 3
 
/*
 * Internal values for flags parameter
 */
#define APPF_SAVE_L2           (1U<<31)
#define APPF_SAVE_SCU          (1U<<30)
#define APPF_SAVE_OTHER        (1U<<29)

/* TODO: check these sizes */
#define CONTEXT_SPACE 32768
#define DEVICE_MEMORY 4096
#define STACK_SIZE 256

/*
 * Maximum size of each item of context, in bytes
 * We round these up to 32 bytes to preserve cache-line alignment
 */

#define PMU_DATA_SIZE               128
#define TIMER_DATA_SIZE             128
#define VFP_DATA_SIZE               288
#define GIC_INTERFACE_DATA_SIZE      64
#define GIC_DIST_PRIVATE_DATA_SIZE   96
#define BANKED_REGISTERS_SIZE       128
#define CP15_DATA_SIZE               64
#define DEBUG_DATA_SIZE             352
#define MMU_DATA_SIZE                64
#define OTHER_DATA_SIZE              32
#define CONTROL_DATA_SIZE            64

#ifdef CONFIG_CAPRI_DORMANT_MODE
/* cpu0, cpu1, work-around ddr address */
#define SECURE_PARAMETERS_SIZE       12
#define WORKAROUND_BUFFER_SIZE       512
#endif

#define GIC_DIST_SHARED_DATA_SIZE  2592
#define SCU_DATA_SIZE                32
#define L2_DATA_SIZE                 96
#define GLOBAL_TIMER_DATA_SIZE      128

/* 
 * Functions in context.c
 */
extern int appf_restore_context(struct appf_cluster *cluster, struct appf_cpu *cpu, int is_secure);
extern int appf_save_context(struct appf_cluster *cluster, struct appf_cpu *cpu, unsigned flags, int is_secure);


/*
 * Lamport's Bakery algorithm for spinlock handling
 *
 * Note that the algorithm requires the bakery struct
 * to be in Strongly-Ordered memory.
 */

#define MAX_CPUS 4

/** 
 * Bakery structure - declare/allocate one of these for each lock.
 * A pointer to this struct is passed to the lock/unlock functions.
 */
typedef struct 
{
    volatile char entering[MAX_CPUS];
    volatile unsigned number[MAX_CPUS];
} bakery_t;

extern void init_bakery_spinlock(bakery_t *bakery);
extern void get_bakery_spinlock(unsigned cpuid, bakery_t *bakery);
extern void release_bakery_spinlock(unsigned cpuid, bakery_t *bakery);


/*
 * Structures we hide from the OS API
 */

struct appf_cpu_context
{
    appf_u32 flags;
    appf_u32 saved_items;
    appf_u32 *control_data;
    appf_u32 *pmu_data;
    appf_u32 *timer_data;
    appf_u32 *global_timer_data;
    appf_u32 *vfp_data;
    appf_u32 *gic_interface_data;
    appf_u32 *gic_dist_private_data;
#ifdef CONFIG_CAPRI_DORMANT_MODE
	appf_u32 *secure_api_params;
#endif
    appf_u32 *banked_registers;
    appf_u32 *cp15_data;
    appf_u32 *debug_data;
    appf_u32 *mmu_data;
    appf_u32 *other_data;
#ifdef CONFIG_CAPRI_DORMANT_MODE
	appf_u32 workaround_buffer;
#endif
};

struct appf_cluster_context
{
    appf_u32 saved_items;
    appf_u32 *gic_dist_shared_data;
    appf_u32 *l2_data;
    appf_u32 *scu_data;
};

