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
 * This function saves all the context that will be lost 
 * when a CPU and cluster enter a low power state.
 *
 * This function is called with cluster->lock held
 */
int appf_save_context(struct appf_cluster *cluster, struct appf_cpu *cpu,
		      unsigned flags, int is_secure)
{
	appf_u32 saved_items = 0;
	appf_u32 cluster_saved_items = 0;
	struct appf_cpu_context *context;
	struct appf_cluster_context *cluster_context;
	int cluster_down, l2_contents_lost;

	if (is_secure) {
		context = cpu->s_context;
		cluster_context = cluster->s_context;
	} else {
		context = cpu->ns_context;
		cluster_context = cluster->ns_context;
	}

	/* add flags as required by hardware (e.g. APPF_SAVE_L2 if L2 is on) */
	flags |= context->flags;

	/* Save perf. monitors first, so we don't interfere too much with counts */
	if (flags & APPF_SAVE_PMU) {
		save_performance_monitors(context->pmu_data);
		saved_items |= APPF_SAVE_PMU;
	}

	if (flags & APPF_SAVE_TIMERS) {
		save_a9_timers(context->timer_data, cluster->scu_address);
		saved_items |= APPF_SAVE_TIMERS;
	}

	if (flags & APPF_SAVE_GLOBAL_TIMER) {
		save_a9_global_timer(context->global_timer_data,
				     cluster->scu_address);
		saved_items |= APPF_SAVE_GLOBAL_TIMER;
	}

	if (flags & APPF_SAVE_VFP) {
		save_vfp(context->vfp_data);
		saved_items |= APPF_SAVE_VFP;
	}

	if (cpu->ic_address) {
		save_gic_interface(context->gic_interface_data, cpu->ic_address,
				   is_secure);
		save_gic_distributor_private(context->gic_dist_private_data,
					     cluster->ic_address, is_secure);
		/* TODO: check return value and quit if nonzero! */
	}

	save_banked_registers(context->banked_registers);
	save_cp15(context->cp15_data);

	if (flags & APPF_SAVE_OTHER) {
		if (cluster->cpu_type == CPU_A9) {
			save_a9_other(context->other_data, is_secure);
		} else {
			save_a5_other(context->other_data, is_secure);
		}
		saved_items |= APPF_SAVE_OTHER;
	}

	if (flags & APPF_SAVE_DEBUG) {
		save_v7_debug(context->debug_data);
		saved_items |= APPF_SAVE_DEBUG;
	}

	cluster_down = *cluster->ppower_state >= 2;
	l2_contents_lost = *cluster->ppower_state >= 3;

	if (cluster_down && cluster->ic_address) {
		save_gic_distributor_shared
		    (cluster_context->gic_dist_shared_data, cluster->ic_address,
		     is_secure);
	}

	save_control_registers(context->control_data, is_secure);
	save_mmu(context->mmu_data);
	context->saved_items = saved_items;

	if (cluster_down) {
		if (flags & APPF_SAVE_SCU) {
			save_a9_scu(cluster_context->scu_data,
				    cluster->scu_address);
			cluster_saved_items |= APPF_SAVE_SCU;
		}

		if (flags & APPF_SAVE_L2) {
			save_pl310(cluster_context->l2_data,
				   cluster->l2_address);
			cluster_saved_items |= APPF_SAVE_L2;
		}
		cluster_context->saved_items = cluster_saved_items;
	}

	/* 
	 * DISABLE DATA CACHES
	 *
	 * First, disable, then clean+invalidate the L1 cache.
	 *
	 * Note that if L1 was to be dormant and we were the last CPU down (and first back), we would not clean L1.
	 */
	disable_clean_inv_dcache_v7_l1();

	/* 
	 * Next, disable cache coherency
	 */
	if (cluster->scu_address) {
		/* let core-0 take care of this in the secure side */
		if (appf_platform_get_cpu_index() == 1) {
			write_actlr(read_actlr() & ~A9_SMP_BIT);
		}
	}

	/*
	 * If the L2 cache is in use, there is still more to do.
	 */
	if (cluster->l2_address && is_enabled_pl310(cluster->l2_address)
	    && l2_contents_lost) {
		/* Clean the whole thing */
		/* TODO: Could optimize this by doing it once in S (if we are saving context in S+NS) */
		clean_pl310(cluster->l2_address);
	}

	return APPF_OK;
}

/** 
 * This function restores all the context that was lost 
 * when a CPU and cluster entered a low power state. It is called shortly after
 * reset, with the MMU and data cache off.
 *
 * This function is called with cluster->lock held
 */
int appf_restore_context(struct appf_cluster *cluster, struct appf_cpu *cpu,
			 int is_secure)
{
	struct appf_cpu_context *context;
	struct appf_cluster_context *cluster_context;
	int cluster_init;
	appf_u32 saved_items, cluster_saved_items = 0;

	/*
	 * At this point we may not write to any static data, and we may
	 * only read the data that is uncached (which includes the stack).
	 */
	if (is_secure) {
		context = cpu->s_context;
		cluster_context = cluster->s_context;
	} else {
		context = cpu->ns_context;
		cluster_context = cluster->ns_context;
	}

	/* Should we initialize the cluster: are we the first CPU back on, and has the cluster been off? */
	cluster_init = (*cluster->pactive_cpus == 0
			&& *cluster->ppower_state >= 2);

	/* First set up the SCU & L2, if necessary */
	if (cluster_init) {
		cluster_saved_items = cluster_context->saved_items;
		if (cluster_saved_items & APPF_SAVE_SCU) {
			restore_a9_scu(cluster_context->scu_data,
				       cluster->scu_address);
		}
		if (cluster_saved_items & APPF_SAVE_L2) {
			//restore_pl310(cluster_context->l2_data, cluster->l2_address, *cluster->ppower_state == 2);
		}
	}

	/* Next get the MMU back on */
	restore_mmu(context->mmu_data);
	restore_control_registers(context->control_data, is_secure);

	saved_items = context->saved_items;

	/* 
	 * MMU and L1 and L2 caches are on, we may now read/write any data.
	 * Now we need to restore the rest of this CPU's context 
	 */

	/* Get the debug registers restored, so we can debug most of the APPF code sensibly! */
	if (saved_items & APPF_SAVE_DEBUG) {
		restore_v7_debug(context->debug_data);
	}

	/* Restore shared items if necessary */
	if (cluster->ic_address) {
		if (cluster_init) {
			gic_distributor_set_enabled(FALSE, cluster->ic_address);
			restore_gic_distributor_shared
			    (cluster_context->gic_dist_shared_data,
			     cluster->ic_address, is_secure);
			gic_distributor_set_enabled(TRUE, cluster->ic_address);
		}
		restore_gic_distributor_private(context->gic_dist_private_data,
						cluster->ic_address, is_secure);
		restore_gic_interface(context->gic_interface_data,
				      cpu->ic_address, is_secure);
	}

	if (saved_items & APPF_SAVE_OTHER) {
		if (cluster->cpu_type == CPU_A9) {
			restore_a9_other(context->other_data, is_secure);
		} else {
			restore_a5_other(context->other_data, is_secure);
		}
	}

	restore_cp15(context->cp15_data);
	restore_banked_registers(context->banked_registers);

	if (saved_items & APPF_SAVE_VFP) {
		restore_vfp(context->vfp_data);
	}

	if (saved_items & APPF_SAVE_TIMERS) {
		restore_a9_timers(context->timer_data, cluster->scu_address);
	}

	if (saved_items & APPF_SAVE_GLOBAL_TIMER) {
		restore_a9_global_timer(context->global_timer_data,
					cluster->scu_address);
	}

	if (saved_items & APPF_SAVE_PMU) {
		restore_performance_monitors(context->pmu_data);
	}

	/* Return to OS */
	return APPF_OK;
}
