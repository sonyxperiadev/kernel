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
 
extern unsigned va_to_pa(unsigned virtual_address);

extern unsigned read_mpidr(void);
extern unsigned read_sctlr(void);
extern void write_sctlr(unsigned value);
extern unsigned read_actlr(void);
extern void write_actlr(unsigned value);
extern unsigned read_prrr(void);
extern unsigned read_nmrr(void);
extern unsigned read_l2ctlr(void);
extern unsigned read_mvbar(void);
extern unsigned read_cbar(void);
extern unsigned read_drar(void);
extern unsigned read_dsar(void);
extern void write_osdlr(unsigned value);
extern unsigned get_cpu_type(void);

extern void dsb(void);
extern void dmb(void);
extern void wfi(void);
extern void wfi_loop(void);
extern void wfe(void);
extern void sev(void);

extern unsigned * copy_words(volatile unsigned *destination, volatile unsigned *source, unsigned num_words);
extern void appf_memcpy(void *dst, const void *src, unsigned length);
extern void appf_memset(void *dst, unsigned value, unsigned length);

extern void initialize_spinlock(bakery_t *bakery);
extern void get_spinlock(unsigned cpuid, bakery_t *bakery);
extern void release_spinlock(unsigned cpuid, bakery_t *bakery);

/*
 * V7 functions
 */
extern void save_control_registers(appf_u32 *pointer, int is_secure);
extern void save_mmu(appf_u32 *pointer);
extern void save_performance_monitors(appf_u32 *pointer);
extern void save_banked_registers(appf_u32 *pointer);
extern void save_cp15(appf_u32 *pointer);
extern void save_vfp(appf_u32 *pointer);
extern void save_generic_timer(appf_u32 *pointer);
extern void save_v7_debug(appf_u32 *pointer);

extern void restore_control_registers(appf_u32 *pointer, int is_secure);
extern void restore_mmu(appf_u32 *pointer);
extern void restore_performance_monitors(appf_u32 *pointer);
extern void restore_banked_registers(appf_u32 *pointer);
extern void restore_cp15(appf_u32 *pointer);
extern void restore_vfp(appf_u32 *pointer);
extern void restore_generic_timer(appf_u32 *pointer);
extern void restore_v7_debug(appf_u32 *pointer);

extern void disable_clean_inv_dcache_v7_l1(void);
extern void disable_clean_inv_cache_pl310(unsigned pl310_address, unsigned stack_start, unsigned stack_size, int disable);
extern void disable_clean_inv_dcache_v7_all(void);
extern void clean_dcache_v7_l1(void);
extern void clean_mva_dcache_v7_l1(void *mva);
extern void invalidate_icache_v7_pou(void);
extern void invalidate_dcache_v7_all(void);

extern void appf_smc_handler(void);
extern void enter_secure_monitor_mode(void);
extern void enter_nonsecure_svc_mode(void);
extern void set_security_state(int ns);

/*
 * PL310 functions
 */
extern void clean_inv_range_pl310(void *start, unsigned size, unsigned pl310_address);  /* addresses are inclusive */
extern void clean_range_pl310(void *start, unsigned size, unsigned pl310_address);  /* addresses are inclusive */
extern void inv_range_pl310(void *start, unsigned size, unsigned pl310_address);
extern void clean_inv_pl310(unsigned pl310_address);
extern void clean_pl310(unsigned pl310_address);
extern void save_pl310(appf_u32 *pointer, unsigned pl310_address);
extern void restore_pl310(appf_u32 *pointer, unsigned pl310_address, int dormant);
extern void set_enabled_pl310(unsigned enabled, unsigned pl310_address);
extern void set_status_pl310(unsigned status, unsigned pl310_address);
extern int is_enabled_pl310(unsigned pl310_address);

/*
 * GIC functions
 */
extern int gic_distributor_set_enabled(int enabled, unsigned gic_distributor_address);
extern void save_gic_interface(appf_u32 *pointer, unsigned gic_interface_address, int is_secure);
extern int save_gic_distributor_private(appf_u32 *pointer, unsigned gic_distributor_address, int is_secure);
extern int save_gic_distributor_shared(appf_u32 *pointer, unsigned gic_distributor_address, int is_secure);
extern void restore_gic_interface(appf_u32 *pointer, unsigned gic_interface_address, int is_secure);
extern void restore_gic_distributor_private(appf_u32 *pointer, unsigned gic_distributor_address, int is_secure);
extern void restore_gic_distributor_shared(appf_u32 *pointer, unsigned gic_distributor_address, int is_secure);

/*
 * A8 functions
 */
extern void save_a8_other(appf_u32 *pointer);
extern void restore_a8_other(appf_u32 *pointer);

/*
 * A9/A5 functions
 */
extern void save_a9_timers(appf_u32 *pointer, unsigned scu_address);
extern void save_a9_global_timer(appf_u32 *pointer, unsigned scu_address);
extern void save_a9_other(appf_u32 *pointer, int is_secure);
extern void save_a5_other(appf_u32 *pointer, int is_secure);

extern void restore_a9_timers(appf_u32 *pointer, unsigned scu_address);
extern void restore_a9_global_timer(appf_u32 *pointer, unsigned scu_address);
extern void restore_a9_other(appf_u32 *pointer, int is_secure);
extern void restore_a5_other(appf_u32 *pointer, int is_secure);

/*
 * A15 functions
 */
extern void save_a15_l2(appf_u32 *pointer);
extern void restore_a15_l2(appf_u32 *pointer);

/*
 * A9 SCU functions
 */
extern void save_a9_scu(appf_u32 *pointer, unsigned scu_address);
extern void restore_a9_scu(appf_u32 *pointer, unsigned scu_address);
extern void set_status_a9_scu(unsigned cpu_index, unsigned status, unsigned scu_address);
extern int num_cpus_from_a9_scu(unsigned scu_address);

/*
 * GCC Compatibility
 */
#ifndef __ARMCC_VERSION
#define __nop() __asm__ __volatile__( "nop\n" )
#endif
