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
 
/** \file platform.c
 * This file contains code to set up the APPF tables, context save/restore space,
 * reset handling and stack space for this platform.
 *
 * Some code and data in this file must be linked in the entry point sections. 
 * Functions that need to be in the entry point must be given the following attribute:
 *     __attribute__ ((section ("APPF_ENTRY_POINT_CODE")))
 * 
 * APPF tables must be given the following attribute:
 *     __attribute__ ((section ("APPF_TABLE_DATA")))
 *
 */

#include "appf_types.h"
#include "appf_platform_api.h"
#include "appf_internals.h"
#include "appf_helpers.h"

#ifdef CAPRI_DORMANT_CHANGE
#include <mach/io_map.h>
#include <mach/memory.h>
#include <mach/sec_api.h>
#endif

#ifndef MAX_CPUS
#define MAX_CPUS 4
#endif

extern unsigned platform_cpu_stacks[MAX_CPUS];

/*
 * This will be copied to the correct place in memory at boottime_init
 * so that the OS knows where to find it.
 */
#ifdef CAPRI_DORMANT_CHANGE
//static struct appf_main_table temp_main_table = {PHYS_OFFSET,0,1}; 
static struct appf_main_table temp_main_table = {CONFIG_BCM_RAM_BASE,0,1}; 
#else
static struct appf_main_table temp_main_table = {0x0,0x10000000,1};  /* TODO: Get ram_size from somewhere sensible? */
#endif

/*
 * These tables go in the APPF_TABLE_DATA section 
 */
struct appf_cluster aem_cluster[1] __attribute__ ((section ("APPF_TABLE_DATA")));
struct appf_cpu aem_cpu[MAX_CPUS]  __attribute__ ((section ("APPF_TABLE_DATA")));

/**
 * Simple Normal memory allocator function.
 * Returns start address of allocated region
 * Allocates region of size bytes, size will be rounded up to multiple of sizeof(long long)
 * Memory is zero-initialized.
 */
static long long normal_memory[CONTEXT_SPACE / sizeof(long long)];
#ifdef CAPRI_DORMANT_CHANGE

/* Macro to get back the VA */
#define get_normal_memory_va(x) \
	( ( (appf_u32)(x) - (appf_u32)virt_to_phys(normal_memory) ) + \
	 (appf_u32)normal_memory)

/* Rerutn a physical address not virtual */ 
#define get_normal_memory(x) \
	(virt_to_phys((const volatile void *)_get_normal_memory(x)))
static unsigned _get_normal_memory(unsigned size)
#else
static unsigned get_normal_memory(unsigned size)
#endif
{
    static unsigned watermark = 0;
    unsigned ret, chunks_required;
    
    ret = watermark;
    chunks_required = (size + sizeof(long long) - 1) / sizeof(long long);
    watermark += chunks_required;
    
    if (watermark >= CONTEXT_SPACE / sizeof(long long))
    {
        while (1);  /* No output possible, so loop */
    }
    
    return (unsigned) &normal_memory[ret];
}

/**
 * Simple Device memory allocator function.
 * Returns start address of allocated region
 * Allocates region of size bytes, size will be rounded up to multiple of sizeof(long long)
 * Memory is zero-initialized.
 */
static long long *device_memory = (void *)appf_device_memory;

#ifdef CAPRI_DORMANT_CHANGE

/* Macro to get back the VA */
#define get_device_memory_va(x) \
	( ( (appf_u32)(x) - (appf_u32)virt_to_phys(appf_device_memory) ) + \
	 (appf_u32)appf_device_memory )


/* Rerutn a physical address not virtual */ 
#define get_device_memory(x) \
	(virt_to_phys((const volatile void *)_get_device_memory(x)))
static unsigned _get_device_memory(unsigned size)
#else
static unsigned get_device_memory(unsigned size)
#endif
{
    static unsigned watermark = 0;
    unsigned ret, chunks_required;
    
    ret = watermark;
    chunks_required = (size + sizeof(long long) - 1) / sizeof(long long);
    watermark += chunks_required;
    
    if (watermark >= DEVICE_MEMORY / sizeof(long long))
    {
        while (1);  /* No output possible, so loop */
    }
    
    return (unsigned) &device_memory[ret];
}


/**
 * This is where the reset vector jumps to.
 *
 * It needs to restore the endianness bit in the CPSR, and 
 * decide whether this is a warm-or-cold start
 */
extern void platform_reset_handler(void);

/**
 * This function is called at boot time. Memory will be flat-mapped, and the code will use 
 * the bootloader's stack . The code must examine the hardware and set up the APPF tables.
 * The values could alternatively be hard-coded if, for example, the platform always provides
 * certain hardware features. 
 *
 * The values for the master table must be written to the global variable
 * main_table, which is a pointer to the start of the entry point section.
 * All the other tables must be in the APPF_TABLE_DATA section.
 */
int appf_platform_boottime_init(void)
{
    int i;
    unsigned cbar = 0;

#ifdef CAPRI_DORMANT_CHANGE
	struct appf_cluster_context *cluster_context;
	struct appf_cpu_context *cpu_context;
#else
	/* Ensure that the CPUs enter APPF code at reset */
    /* TODO: Consider high vectors */
    *(unsigned *)0x20 = (unsigned)&platform_reset_handler;
#endif

#if !SECURE_ONLY && !NONSECURE_ONLY
    /* Also need to ensure that the SMC instruction reaches APPF code */
    *(unsigned *)(read_mvbar() + 0x20) = (unsigned)&appf_smc_handler;
#endif
    
    /* Setup tables - Note that pointers are flat-mapped/physical addresses */

    aem_cluster[0].cpu_type = get_cpu_type();

    if ((aem_cluster[0].cpu_type != CPU_A9) && (aem_cluster[0].cpu_type != CPU_A5))
    {
        return APPF_BAD_CPU;
    }
   
    if (read_mpidr() & MPIDR_U_BIT)
    {
        is_smp = FALSE;
        aem_cluster[0].num_cpus = 1;
    }
    else
    {
        is_smp = TRUE;
        cbar = read_cbar();
        aem_cluster[0].scu_address = cbar;
        aem_cluster[0].ic_address = cbar + 0x1000;
#ifdef CAPRI_DORMANT_CHANGE
		/* Read from virtual address not the physical */
        aem_cluster[0].num_cpus = num_cpus_from_a9_scu( 
				HW_IO_PHYS_TO_VIRT(aem_cluster[0].scu_address));
#if 0
		aem_cluster[0].num_cpus = 1;
#endif /*DORMANT_TBD */
#else
        aem_cluster[0].num_cpus = num_cpus_from_a9_scu(aem_cluster[0].scu_address);
#endif
    }

#ifdef CAPRI_DORMANT_CHANGE
    aem_cluster[0].l2_address         = L2C_BASE_ADDR;
#else
    aem_cluster[0].l2_address         = 0x1f002000;
#endif
    aem_cluster[0].pactive_cpus       = (void *)get_device_memory(sizeof(appf_i32));
#ifdef CAPRI_DORMANT_CHANGE
	*(appf_u32 *)(get_device_memory_va(aem_cluster[0].pactive_cpus)) = 
		aem_cluster[0].num_cpus;
    aem_cluster[0].cpu_table          = ( struct appf_cpu *) 
		virt_to_phys(&aem_cpu[0]);
#else
    *aem_cluster[0].pactive_cpus      = aem_cluster[0].num_cpus;
    aem_cluster[0].cpu_table          = &aem_cpu[0];
#endif
    aem_cluster[0].lock               = (void *)get_device_memory(sizeof(bakery_t));
    aem_cluster[0].ppower_state       = (void *)get_device_memory(sizeof(appf_i32));
#ifdef CAPRI_DORMANT_CHANGE
	*(appf_u32 *)(get_device_memory_va(aem_cluster[0].ppower_state)) = 0;
#else
    *aem_cluster[0].ppower_state      = 0;
#endif
    initialize_spinlock((bakery_t *)get_device_memory_va(aem_cluster[0].lock));

    /*
     * Note that we wastefully allocate memory for every item in both the S and NS worlds.
     * This could be done more efficiently, but we are keeping it simple here.
     *
     * Also, the Secure context should be allocated in Secure memory!
     */
#ifdef CAPRI_DORMANT_CHANGE 
	/*
	 * We do not want to have any secure date stored
	 * Just save memory and time
	 */
#else
    aem_cluster[0].s_context = (void *)get_normal_memory(sizeof(struct appf_cluster_context));
    aem_cluster[0].s_context->gic_dist_shared_data = (void *)get_normal_memory(GIC_DIST_SHARED_DATA_SIZE);
    aem_cluster[0].s_context->l2_data              = (void *)get_device_memory(L2_DATA_SIZE);
    aem_cluster[0].s_context->scu_data             = (void *)get_device_memory(SCU_DATA_SIZE);
#endif

    aem_cluster[0].ns_context = (void *)get_normal_memory(sizeof(struct appf_cluster_context));
#ifdef CAPRI_DORMANT_CHANGE
	cluster_context = (struct appf_cluster_context *) 
		get_normal_memory_va( aem_cluster[0].ns_context );
    cluster_context->gic_dist_shared_data = (void *)get_normal_memory(GIC_DIST_SHARED_DATA_SIZE);
    cluster_context->l2_data              = (void *)get_device_memory(L2_DATA_SIZE);
    cluster_context->scu_data             = (void *)get_device_memory(SCU_DATA_SIZE);
#else
    aem_cluster[0].ns_context->gic_dist_shared_data = (void *)get_normal_memory(GIC_DIST_SHARED_DATA_SIZE);
    aem_cluster[0].ns_context->l2_data              = (void *)get_device_memory(L2_DATA_SIZE);
    aem_cluster[0].ns_context->scu_data             = (void *)get_device_memory(SCU_DATA_SIZE);
#endif

    for (i=0; i<aem_cluster[0].num_cpus; ++i)
    {
        if (is_smp)
        {
            aem_cpu[i].ic_address = cbar + 0x100;
        }

#ifdef CAPRI_DORMANT_CHANGE 
	/*
	 * We do not want to have any secure date stored
	 * Just save memory and time
	 */
#else
        aem_cpu[i].s_context = (void *)get_normal_memory(sizeof(struct appf_cpu_context));
        aem_cpu[i].s_context->mmu_data              = (void *)get_device_memory(MMU_DATA_SIZE);
        aem_cpu[i].s_context->control_data          = (void *)get_device_memory(CONTROL_DATA_SIZE);
        aem_cpu[i].s_context->pmu_data              = (void *)get_normal_memory(PMU_DATA_SIZE);
        aem_cpu[i].s_context->timer_data            = (void *)get_normal_memory(TIMER_DATA_SIZE);
        aem_cpu[i].s_context->global_timer_data     = (void *)get_normal_memory(GLOBAL_TIMER_DATA_SIZE);
        aem_cpu[i].s_context->vfp_data              = (void *)get_normal_memory(VFP_DATA_SIZE);
        aem_cpu[i].s_context->gic_interface_data    = (void *)get_normal_memory(GIC_INTERFACE_DATA_SIZE);
        aem_cpu[i].s_context->gic_dist_private_data = (void *)get_normal_memory(GIC_DIST_PRIVATE_DATA_SIZE);
        aem_cpu[i].s_context->banked_registers      = (void *)get_normal_memory(BANKED_REGISTERS_SIZE);
        aem_cpu[i].s_context->cp15_data             = (void *)get_normal_memory(CP15_DATA_SIZE);
        aem_cpu[i].s_context->debug_data            = (void *)get_normal_memory(DEBUG_DATA_SIZE);
        aem_cpu[i].s_context->other_data            = (void *)get_normal_memory(OTHER_DATA_SIZE);
#endif

        aem_cpu[i].ns_context = (void *)get_normal_memory(sizeof(struct appf_cpu_context));
#ifdef CAPRI_DORMANT_CHANGE
		cpu_context = (struct appf_cpu_context *) 
			get_normal_memory_va( aem_cpu[i].ns_context );
        cpu_context->control_data          = (void *)get_device_memory(CONTROL_DATA_SIZE);
        cpu_context->mmu_data              = (void *)get_device_memory(MMU_DATA_SIZE);
        cpu_context->pmu_data              = (void *)get_normal_memory(PMU_DATA_SIZE);
        cpu_context->timer_data            = (void *)get_normal_memory(TIMER_DATA_SIZE);
        cpu_context->global_timer_data     = (void *)get_normal_memory(GLOBAL_TIMER_DATA_SIZE);
        cpu_context->vfp_data              = (void *)get_normal_memory(VFP_DATA_SIZE);
        cpu_context->gic_interface_data    = (void *)get_normal_memory(GIC_INTERFACE_DATA_SIZE);
        cpu_context->gic_dist_private_data = (void *)get_normal_memory(GIC_DIST_PRIVATE_DATA_SIZE);
	cpu_context->secure_api_params     =
		(void *)get_secure_buffer() + get_secure_buffer_size() / 2;
        cpu_context->banked_registers      = (void *)get_normal_memory(BANKED_REGISTERS_SIZE);
        cpu_context->cp15_data             = (void *)get_normal_memory(CP15_DATA_SIZE);
        cpu_context->debug_data            = (void *)get_normal_memory(DEBUG_DATA_SIZE);
        cpu_context->other_data            = (void *)get_normal_memory(OTHER_DATA_SIZE);
		cpu_context->workaround_buffer     = (appf_u32)get_normal_memory(WORKAROUND_BUFFER_SIZE);
#endif
        /*
         * Here we are setting up what context gets saved in each world. It is of course
         * possible that another system could use, say, the local timers in S rather than
         * NS, in which case the APPF_SAVE_TIMERS bit should be moved to s_context->flags.
         */
#if SECURE_ONLY
        aem_cpu[i].s_context->flags       = APPF_SAVE_PMU    |
                                            APPF_SAVE_OTHER  |
                                            APPF_SAVE_DEBUG  |
                                            APPF_SAVE_L2;
        if (is_smp)
        {
            aem_cpu[i].s_context->flags  |= APPF_SAVE_TIMERS |
                                            APPF_SAVE_SCU;
        }
#elif NONSECURE_ONLY
        cpu_context->flags      = 			APPF_SAVE_PMU    |
                                       		APPF_SAVE_OTHER  |
                                            APPF_SAVE_DEBUG;
        if (is_smp)
        {
            cpu_context->flags |= 			APPF_SAVE_TIMERS |
                                            APPF_SAVE_SCU;
        }
#ifdef CAPRI_DORMANT_CHANGE
		cpu_context->flags |= 				APPF_SAVE_GLOBAL_TIMER;
#endif
#else
        aem_cpu[i].s_context->flags       = APPF_SAVE_L2     |
                                            APPF_SAVE_OTHER;
        aem_cpu[i].ns_context->flags      = APPF_SAVE_PMU    |
                                            APPF_SAVE_DEBUG;
        if (is_smp)
        {
            aem_cpu[i].s_context->flags  |= APPF_SAVE_SCU;
            aem_cpu[i].ns_context->flags |= APPF_SAVE_TIMERS;
        }
#endif
        /*
         * We put the stack for each CPU in Device memory. This is because we write to
         * the stack while cacheing is disabled. At any point CPU A can allocate
         * a cache line containing some of CPU B's stack, so when CPU B turns cacheing on
         * and reads that line the data will come from CPU A's copy, which is out of date.
         *
         *  CPU A                               CPU B
         *  normal operation                    warm start: cacheing & coherency disabled
         *  speculatively allocates X in cache
         *                                      writes to X (goes to main memory)
         *                                      enables cacheing & coherency
         *                                      reads from X (Error: comes from CPU A's cache!)
         * 
         * This applies to any data that could be written while cacheing is disabled, but in
         * practise we only write to the stack.
         */
        platform_cpu_stacks[i] = get_device_memory(STACK_SIZE) + STACK_SIZE;
    }

#ifdef CAPRI_DORMANT_CHANGE
    temp_main_table.cluster_table           = (struct appf_cluster *)
		virt_to_phys(&aem_cluster[0]);
#else
    temp_main_table.cluster_table           = &aem_cluster[0];
#endif
    temp_main_table.num_clusters            = sizeof(aem_cluster) / sizeof(aem_cluster[0]);
    temp_main_table.entry_point             = appf_entry_point;

    /* Copy our temp table to the right place in memory */
    appf_memcpy(&main_table, &temp_main_table, sizeof(struct appf_main_table));
    
    return APPF_OK;
}

/* 
 * The next three functions may be called using OS stack, with just the entry point mapped
 * so we put them in the APPF_ENTRY_POINT_CODE section
 */
int appf_platform_runtime_init(void)      __attribute__ ((section ("APPF_ENTRY_POINT_CODE")));
int appf_platform_get_cpu_index(void)     __attribute__ ((section ("APPF_ENTRY_POINT_CODE")));
int appf_platform_get_cluster_index(void) __attribute__ ((section ("APPF_ENTRY_POINT_CODE")));

/**
 * This function is called when the OS initializes the firmware at run time.
 * This function uses the OS stack and translation tables, and must be in the entry point section.
 * It is responsible for setting up the reset vector so it enters the platform-specific reset
 * code (in reset.S) and may update the cpu and cluster flags field based on the state
 * of the system (e.g. setting APPF_SAVE_L2 if the L2 cache controller is enabled)
 */
int appf_platform_runtime_init(void)
{
    /* TODO: Add flags to each CPU struct, as required by hardware (e.g. APPF_SAVE_L2 if L2 is on) */
    return APPF_OK;
}

/**
 * This function returns the index in the CPU table for the currently executing CPU.
 * Normally, reading the MPIDR is sufficient.
 *
 * This function must be in the entry point section.
 */
int appf_platform_get_cpu_index(void)
{
    return read_mpidr() & 0xff;
}

/**
 * This function returns the index in the cluster table for the currently executing CPU.
 * Normally, either returning zero or reading the MPIDR is sufficient.
 *
 * This function must be in the entry point section.
 */
int appf_platform_get_cluster_index(void)
{
    return 0;
/*    return (read_mpidr() >> 8) & 0xff; */
}


/**
 * This function is called at the end of runtime initialization.
 *
 * It is called using APPF's translation tables and stack, by the same CPU that
 * did the early initialization.
 */
int appf_platform_late_init(struct appf_cluster *cluster)
{
    /*
     * Clean the APPF code and translation tables from L2 cache, if it's enabled
     * - this matters as we will disable the L2 during power down.
     */
    if (cluster->l2_address && is_enabled_pl310(cluster->l2_address))
    {
        clean_pl310(cluster->l2_address);
    }
    
    return APPF_OK;
}

