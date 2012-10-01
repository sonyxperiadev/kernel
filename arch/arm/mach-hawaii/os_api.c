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
#include <stdarg.h>

#ifdef CAPRI_DORMANT_CHANGE
#include <mach/io_map.h>
#include "mach/sec_api.h"
#endif


static int late_init(void);
static int power_down_cpu(unsigned cstate, unsigned rstate, unsigned flags);
static int power_up_cpus(unsigned cluster_index, unsigned cpus);

#define DBG_LOG(x) \
	*(volatile unsigned *)0x3404DF00= x;

#define DBG_LOG_0(x) \
	*(volatile unsigned *)0x3404DF04= x;

/**
 * This function handles the OS firmware calls. It is called from entry.S,
 * just after the OS translation tables and stack have been switched for the
 * firmware translation tables and stack. Returning from this function
 * returns to the OS via some code in entry.S.
 */
int appf_runtime_call(unsigned function, unsigned arg1, unsigned arg2, unsigned arg3)
{
	int x;
    switch (function)
    {
    case APPF_INITIALIZE:
        return late_init();
    case APPF_POWER_DOWN_CPU:
		x= power_down_cpu(arg1, arg2, arg3);
		return x;
    case APPF_POWER_UP_CPUS:
        return power_up_cpus(arg1, arg2);
    }
    return APPF_BAD_FUNCTION;
}


/** 
 * This function is called while running flat-mapped, on the boot loader's stack.
 */
int appf_boottime_init(void)
{
    unsigned ret;

    /* Set up stack pointers per CPU, per cluster */
    /* Discover devices, set up tables */
    ret = appf_platform_boottime_init();
    if (ret)
    {
        return ret;
    }

#ifdef CAPRI_DORMANT_CHANGE
    appf_runtime_call_flat_mapped = (unsigned)virt_to_phys(appf_runtime_call);
    appf_device_memory_flat_mapped = (unsigned)virt_to_phys(appf_device_memory);
#else
    appf_runtime_call_flat_mapped = (unsigned)appf_runtime_call;
    appf_device_memory_flat_mapped = (unsigned)appf_device_memory;
#endif

    return ret;
}


/*
 * The next function is called using OS stack, with just the entry point mapped
 * so we put it (and its globals) in the APPF_ENTRY_POINT_* sections
 */
int __attribute__ ((section ("APPF_ENTRY_POINT_DATA"))) is_smp = FALSE;

/**
 * This function is called when the OS makes a firmware call with the 
 * function code APPF_INITIALIZE
 *
 * It is called using the OS translation tables and stack, with only the 
 * entry point is mapped in, so it cannot call any functions that are 
 * not linked in the entry point.
 */
int appf_runtime_init(void) __attribute__ ((section ("APPF_ENTRY_POINT_CODE")));
int appf_runtime_init(void)
{
    int ret;
    
    ret = appf_setup_translation_tables();
    if (ret < 0)
    {
        return ret;
    }

    return appf_platform_runtime_init();
}


/**
 * This function is called when the OS makes a firmware call with the 
 * function code APPF_INTIALIZE.
 *
 * It is called using the APPF translation tables that were set up in
 * appf_runtime_init, above.
 */
static int late_init(void)
{
    struct appf_cluster *cluster;
    int cluster_index;

    cluster_index = appf_platform_get_cluster_index();
    cluster = main_table.cluster_table + cluster_index;

    /*
     * Clean the translation tables out of the L1 dcache
     * (see comments in disable_clean_inv_dcache_v7_l1)
     */
    dsb();
    clean_dcache_v7_l1();

    return appf_platform_late_init(cluster);
}

/* instead of using the regular secure api_call which
 * uses smp and logging, issue warnings etc, directly use the
 * hw_sec_rom_pub_bridge..
 */
static unsigned secure_api_call_local(unsigned appl_id, unsigned flags, ...)
{
	extern unsigned hw_sec_rom_pub_bridge(unsigned appl_id, unsigned flags,
				va_list);
	va_list var_args;
	va_start(var_args, flags);
	hw_sec_rom_pub_bridge(appl_id, flags, var_args);
	va_end(var_args);
}

/**
 * This function is called when the OS makes a firmware call with the 
 * function code APPF_POWER_DOWN_CPU
 */
static int power_down_cpu(unsigned cstate, unsigned rstate, unsigned flags)
{
	extern void platform_reset_handler(void);
    struct appf_cpu *cpu;
    struct appf_cluster *cluster;
    int cpu_index, cluster_index;
    int i, rc, cluster_can_enter_cstate1;
	void (*core1_workaround)(unsigned ddr_address_for_work_around) = 0x3404D000;
#ifdef USE_REALVIEW_EB_RESETS
    int system_reset = FALSE, last_cpu = FALSE;
#endif

    cpu_index = appf_platform_get_cpu_index();
    cluster_index = appf_platform_get_cluster_index();

    cluster = main_table.cluster_table + cluster_index;
    cpu = cluster->cpu_table + cpu_index;
    
    /* Validate arguments */
    if (cstate > 3)
    {
        return APPF_BAD_CSTATE;
    }
    if (rstate > 3)
    {
        return APPF_BAD_RSTATE;
    }

    /* If we're just entering standby mode, we don't mark the CPU as inactive */
    if (cstate == 1)
    {
        get_spinlock(cpu_index, cluster->lock);
        cpu->power_state = 1;
        
        /* See if we can make the cluster standby too */
        if (rstate == 1)
        {
            cluster_can_enter_cstate1 = TRUE;
            for(i=0; i<cluster->num_cpus; ++i)
            {
                if (cluster->cpu_table[i].power_state != 1)
                {
                    cluster_can_enter_cstate1 = FALSE;
                    break;
                }
            }
            if (cluster_can_enter_cstate1)
            {
                *cluster->ppower_state = 1;
            }
        }
                
        rc = appf_platform_enter_cstate1(cpu_index, cpu, cluster);

        if (rc == 0)
        {
            release_spinlock(cpu_index, cluster->lock);
            dsb();
            wfi();
            get_spinlock(cpu_index, cluster->lock);
            rc = appf_platform_leave_cstate1(cpu_index, cpu, cluster);
        }
        
        cpu->power_state = 0;
        *cluster->ppower_state = 0;
        release_spinlock(cpu_index, cluster->lock);
        return rc;
    }

    /* Ok, we're not just entering standby, so we are going to lose the context on this CPU */
    
    get_spinlock(cpu_index, cluster->lock);
    --*cluster->pactive_cpus;

    cpu->power_state = cstate;
    if (*cluster->pactive_cpus == 0)
    {
        *cluster->ppower_state = rstate;
#ifdef USE_REALVIEW_EB_RESETS
        /* last CPU down must not issue WFI, or we get stuck! */
        last_cpu = TRUE;
        if (rstate > 1)
        {
            system_reset = TRUE;
        }
#endif
    }
	cpu->ns_context->secure_api_params[0] = (appf_u32)virt_to_phys(platform_reset_handler);

	cpu->ns_context->secure_api_params[1] = (appf_u32)virt_to_phys(platform_reset_handler);

	cpu->ns_context->secure_api_params[2] = (appf_u32)cpu->ns_context->workaround_buffer;
    
#if SECURE_ONLY
    appf_save_context(cluster, cpu, flags, TRUE);
#elif NONSECURE_ONLY
    appf_save_context(cluster, cpu, flags, FALSE);
#else
    /*
     * Note that in a real TrustZone system it is a VERY BAD IDEA to share code
     * between the Secure and Non-secure worlds. If the code was compromised in
     * the Non-secure world, this would lead to a security breach. A production
     * implementation of context save/restore would not return from
     * enter_secure_monitor_mode() and all the code to save the secure context
     * (and sanction the power down) would be held in Secure memory.
     */
    appf_save_context(cluster, cpu, flags, FALSE);
    enter_secure_monitor_mode();
    appf_save_context(cluster, cpu, flags, TRUE);
#endif

    /* Call the platform-specific shutdown code */
    rc = appf_platform_enter_cstate(cpu_index, cpu, cluster);
    
    /* Did the power down succeed? */
    if (rc == APPF_OK)
    {
        release_spinlock(cpu_index, cluster->lock);

#ifdef CAPRI_DORMANT_CHANGE
		/* We do not loop around infinitely.  We allow no power down
		 * situation even when we want to enter dormant
		 */
#else
        while (1) 
#endif
        {
#if defined(NO_PCU) || defined(USE_REALVIEW_EB_RESETS)
            extern void platform_reset_handler(unsigned, unsigned, unsigned, unsigned);
            void (*reset)(unsigned, unsigned, unsigned, unsigned) = platform_reset_handler;

#ifdef USE_REALVIEW_EB_RESETS
            /* Unlock system registers */
            *(volatile unsigned *)0x10000020 = 0xa05f;
            if (system_reset)
            {
                /* Tell the Realview EB to do a system reset */
                *(volatile unsigned *)0x10000040 = 6;
                /* goto reset vector! */
            }
            else
            {
                if (!last_cpu)
                {
                    /* Tell the Realview EB to put this CPU into reset */
                    *(volatile unsigned *)0x10000074 &= ~(1 << (6 + cpu_index));
                    /* goto reset vector! (when another CPU takes us out of reset) */
                }
            }
#endif
            /*
             * If we get here, either we are the last CPU, or the EB resets 
             * aren't present (e.g. Emulator). So, fake a reset: Turn off MMU, 
             * corrupt registers, wait for a while, jump to warm reset entry point
             */
            write_sctlr(read_sctlr() & ~0x10001807); /* clear TRE, I Z C M */
            dsb();
            for (i=0; i<10000; ++i)
            {
                __nop();
            }
            reset(0xdeadbeef, 0xdeadbeef, 0xdeadbeef, 0xdeadbeef);
#endif

            dsb();    
#ifdef CAPRI_DORMANT_CHANGE
			if (cpu_index == 0 )
			{

				do
				{
            		//wfi(); 
					secure_api_call_local(SSAPI_DORMANT_ENTRY_SERV, 0xB, cpu->ns_context->secure_api_params, cpu->ns_context->secure_api_params + 0x20 , 0x2);
				} while(0);
				//while ( ((*(volatile unsigned int *)(0x3FF00118))&0x3ff)  == 0x3ff );

				//*(volatile unsigned *)(0x3500417C) = ((*(volatile unsigned int *)(0x3ff00118))&0x3ff);

				/* Restore context asthough we have not entered 
				 * this if condition
				 */
    			get_spinlock(cpu_index, cluster->lock);

        		//appf_restore_context(cluster, cpu, FALSE);
				/* Restore control registers so that cache would be re-enabled
				 * and cache coherency would be turned on for this CORE
				 */
				restore_control_registers(cpu->ns_context->control_data, FALSE);


				/* Restore timers since they are stopped while saving */
        		restore_a9_timers(cpu->ns_context->timer_data, cluster->scu_address);

				/* Restore performance monitors since they are stopped while saving */
        		restore_performance_monitors(cpu->ns_context->pmu_data);

				cpu->power_state = 0;
				*cluster->ppower_state = 0;
				++*cluster->pactive_cpus;
				release_spinlock(cpu_index, cluster->lock);
			}
			else
			{
				extern void platform_reset_handler(void);
				/* For core-1 there is no secure state 
				 * just call WFI and indicate readyness for dormant
				 */

				/* Write the second boot register
				 */
				*(volatile unsigned *)(0x3500417C) = (appf_u32)virt_to_phys(platform_reset_handler);

				/* At this point, we are already out of SMP
				 * now allow core-1 to enter dormant 
				 */
				*(volatile unsigned char*)(0x3ff00009) = 0x02;

				DBG_LOG(0x1);
				/* execute simple WFI */
				// let us execute the WFI from sram
				//core1_workaround((appf_u32)cpu->ns_context->workaround_buffer);
				DBG_LOG(0x2);

				do	
				{
            		wfi(); 
				} while (0);
				//while ( ((*(volatile unsigned int *)(0x3FF00118))&0x3ff)  == 0x3ff );

				/* keep this code in case we fail to enter dormant */
				*(volatile unsigned char*)(0x3ff00009) = 0x0;

    			get_spinlock(cpu_index, cluster->lock);

        		//appf_restore_context(cluster, cpu, FALSE);


				/* Restore context asthough we have not entered 
				 * this if condition
				 */
    			//get_spinlock(cpu_index, cluster->lock);
				DBG_LOG(0x3);

				/* Restore control registers so that cache would be re-enabled
				 * and cache coherency would be turned on for this CORE
				 */
				restore_control_registers(cpu->ns_context->control_data, FALSE);
				DBG_LOG(0x4);


				/* Restore timers since they are stopped while saving */
        		restore_a9_timers(cpu->ns_context->timer_data, cluster->scu_address);
				DBG_LOG(0x5);

				/* Restore performance monitors since they are stopped while saving */
        		restore_performance_monitors(cpu->ns_context->pmu_data);
				DBG_LOG(0x6);

				cpu->power_state = 0;
				*cluster->ppower_state = 0;
				++*cluster->pactive_cpus;
				release_spinlock(cpu_index, cluster->lock);
				DBG_LOG(0x7);
			}
#else
            wfi(); /* This signals the power controller to cut the power */
#endif
            /* Next stop, reset vector! */
        }
    }
    else
    {
        /* Power down failed for some reason, return to the OS */
#if SECURE_ONLY
        appf_restore_context(cluster, cpu, TRUE);
#elif NONSECURE_ONLY
        appf_restore_context(cluster, cpu, FALSE);
#else
        appf_restore_context(cluster, cpu, TRUE);
        enter_nonsecure_svc_mode();
        appf_restore_context(cluster, cpu, FALSE);
#endif
        cpu->power_state = 0;
        *cluster->ppower_state = 0;
        ++*cluster->pactive_cpus;
        release_spinlock(cpu_index, cluster->lock);
    }

    return APPF_CPU_NOT_SHUTDOWN;
}

/**
 * This function is called when the OS makes a firmware call with the 
 * function code APPF_POWER_UP_CPUS
 *
 * It brings powered down CPUs/clusters back to running state.
 */
static int power_up_cpus(unsigned cluster_index, unsigned cpus)
{
    struct appf_cpu *cpu;
    struct appf_cluster *cluster;
    unsigned cpu_index, this_cpu_index;
    int ret = APPF_OK;

    if (cluster_index >= main_table.num_clusters)
    {
        return APPF_BAD_CLUSTER;
    }
    
    cluster = main_table.cluster_table + cluster_index;

    if (cpus >= (1<<cluster->num_cpus))
    {
        return APPF_BAD_CPU;
    }
    
    /* TODO: add cluster-waking code for multi-cluster systems */
    /* TODO: locks will have to be expanded once extra-cluster CPUs can contend for them */

    this_cpu_index = appf_platform_get_cpu_index();
    get_spinlock(this_cpu_index, cluster->lock);
    
    for (cpu_index=0; cpu_index < cluster->num_cpus; ++cpu_index)
    {
        cpu = cluster->cpu_table + cpu_index;
        if (cpu->power_state == 0)
        {
            continue;
        }
        ret = appf_platform_power_up_cpu(cpu, cluster);
        if (ret)
        {
            break;
        }
        cpu->power_state = 0;
    }

    release_spinlock(this_cpu_index, cluster->lock);
    return ret;
}


/**
 * This function is called after a warm reset, from entry.S.
 */
int appf_warm_reset(void)
{
    int ret;
#ifdef CAPRI_DORMANT_CHANGE
    struct appf_main_table *main_table_ptr;
#endif
    struct appf_cpu *cpu;
    struct appf_cluster *cluster;
    int cpu_index, cluster_index;

    cpu_index = appf_platform_get_cpu_index();
    cluster_index = appf_platform_get_cluster_index();

#ifdef CAPRI_DORMANT_CHANGE
	main_table_ptr = (struct appf_main_table *)(
			virt_to_phys(&main_table));
    cluster = main_table_ptr->cluster_table + cluster_index;
#else
    cluster = main_table.cluster_table + cluster_index;
#endif
    cpu = cluster->cpu_table + cpu_index;

    get_spinlock(cpu_index, cluster->lock);


#if SECURE_ONLY
    appf_restore_context(cluster, cpu, TRUE);
#elif NONSECURE_ONLY
    appf_restore_context(cluster, cpu, FALSE);
#else
    appf_restore_context(cluster, cpu, TRUE);
    enter_nonsecure_svc_mode();
    appf_restore_context(cluster, cpu, FALSE);
#endif

    ret = appf_platform_leave_cstate(cpu_index, cpu, cluster);

    ++*cluster->pactive_cpus;
    *cluster->ppower_state = 0;
    cpu->power_state = 0;
    
	/* Any cluster related stuff is now restored, if this is CORE-0 that is restoring,
	 * let CORE-1 continue its work and restoring it's context now.
	 */
	if ( cpu_index == 0 )
	{
		//*(volatile unsigned *)(0x3500417C) |= 1;
	}

    release_spinlock(cpu_index, cluster->lock);

    return ret;
}
