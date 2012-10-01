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
/****************************************************************************
* The contents of the original source code is minimally modified to suit
* the needs of the environment in which this code is being used.
*****************************************************************************/
 
#include <mach/appf_types.h>
#include <mach/appf_internals.h>
#include <mach/appf_helpers.h>

/* This macro sets either the NS or S enable bit in the GIC distributor control register */
#define GIC_DIST_ENABLE      0x00000001

struct set_and_clear_regs
{
    volatile unsigned int set[32], clear[32];
};

typedef struct
{
    /* 0x000 */  volatile unsigned int control;
                 const unsigned int controller_type;
                 const unsigned int implementer;
                 const char padding1[116];
    /* 0x080 */  volatile unsigned int security[32];
    /* 0x100 */  struct set_and_clear_regs enable;
    /* 0x200 */  struct set_and_clear_regs pending;
    /* 0x300 */  volatile const unsigned int active[32];
                 const char padding2[128];
    /* 0x400 */  volatile unsigned int priority[256];
    /* 0x800 */  volatile unsigned int target[256];
    /* 0xC00 */  volatile unsigned int configuration[64];
    /* 0xD00 */  const char padding3[512];
    /* 0xF00 */  volatile unsigned int software_interrupt;
                 const char padding4[220];
    /* 0xFE0 */  unsigned const int peripheral_id[4];
    /* 0xFF0 */  unsigned const int primecell_id[4];
} interrupt_distributor;

typedef struct
{
    /* 0x00 */  volatile unsigned int control;
    /* 0x04 */  volatile unsigned int priority_mask;
    /* 0x08 */  volatile unsigned int binary_point;
    /* 0x0c */  volatile unsigned const int interrupt_ack;
    /* 0x10 */  volatile unsigned int end_of_interrupt;
    /* 0x14 */  volatile unsigned const int running_priority;
    /* 0x18 */  volatile unsigned const int highest_pending;
    /* 0x1c */  volatile unsigned int aliased_binary_point;
} cpu_interface;


/*
 * Saves the GIC CPU interface context
 * Requires 3 or 4 words of memory
 */
void save_gic_interface(appf_u32 *pointer, unsigned gic_interface_address, int is_secure)
{
    cpu_interface *ci = (cpu_interface *)gic_interface_address;

    pointer[0] = ci->control;
    pointer[1] = ci->priority_mask;
    pointer[2] = ci->binary_point;
    
    if (is_secure)
    {
        pointer[3] = ci->aliased_binary_point;
    }
}

/* 
 * Enables or disables the GIC distributor (for the current security state)
 * Parameter 'enabled' is boolean.
 * Return value is boolean, and reports whether GIC was previously enabled.
 */
int gic_distributor_set_enabled(int enabled, unsigned gic_distributor_address)
{
    unsigned tmp;
    interrupt_distributor *id = (interrupt_distributor *)gic_distributor_address;

    tmp = id->control;
    if (enabled) 
    {
        id->control = tmp | GIC_DIST_ENABLE;
    }
    else
    {
        id->control = tmp & ~GIC_DIST_ENABLE;
    }
    return (tmp & GIC_DIST_ENABLE) != 0;
}

/*
 * Saves this CPU's banked parts of the distributor
 * Returns non-zero if an SGI/PPI interrupt is pending (after saving all required context)
 * Requires 19 words of memory
 */
int save_gic_distributor_private(appf_u32 *pointer, unsigned gic_distributor_address, int is_secure)
{
    interrupt_distributor *id = (interrupt_distributor *)gic_distributor_address;

    *pointer = id->enable.set[0];
    ++pointer;
    pointer = copy_words(pointer, id->priority, 8);
    pointer = copy_words(pointer, id->target, 8);
    if (is_secure)
    {
        *pointer = id->security[0];
        ++pointer;
    }
    /* Save just the PPI configurations (SGIs are not configurable) */
    *pointer = id->configuration[1];
    ++pointer;
    *pointer = id->pending.set[0];
    if (*pointer)
    {
        return -1;
    }
    else
    {
        return 0;
    }        
}

/*
 * Saves the shared parts of the distributor.
 * Requires 1 word of memory, plus 20 words for each block of 32 SPIs (max 641 words)
 * Returns non-zero if an SPI interrupt is pending (after saving all required context)
 */
int save_gic_distributor_shared(appf_u32 *pointer, unsigned gic_distributor_address, int is_secure)
{
    interrupt_distributor *id = (interrupt_distributor *)gic_distributor_address;
    unsigned num_spis, *saved_pending;
    int i, retval = 0;
    
    
    /* Calculate how many SPIs the GIC supports */
    num_spis = 32 * (id->controller_type & 0x1f);

    /* TODO: add nonsecure stuff */

    /* Save rest of GIC configuration */
    if (num_spis)
    {
        pointer = copy_words(pointer, id->enable.set + 1,    num_spis / 32);
        pointer = copy_words(pointer, id->priority + 8,      num_spis / 4);
        pointer = copy_words(pointer, id->target + 8,        num_spis / 4);
        pointer = copy_words(pointer, id->configuration + 2, num_spis / 16);
        if (is_secure)
        {
            pointer = copy_words(pointer, id->security + 1,  num_spis / 32);
        }
        saved_pending = pointer;
        pointer = copy_words(pointer, id->pending.set + 1,   num_spis / 32);
    
        /* Check interrupt pending bits */
        for (i=0; i<num_spis/32; ++i)
        {
            if (saved_pending[i])
            {
                retval = -1;
                break;
            }
        }
    }
    
    /* Save control register */
    *pointer = id->control;
    
    return retval;
}

void restore_gic_interface(appf_u32 *pointer, unsigned gic_interface_address, int is_secure)
{
    cpu_interface *ci = (cpu_interface *)gic_interface_address;

    ci->priority_mask = pointer[1];
    ci->binary_point = pointer[2];

    if (is_secure)
    {
        ci->aliased_binary_point = pointer[3];
    }

    /* Restore control register last */
    ci->control = pointer[0];
}

void restore_gic_distributor_private(appf_u32 *pointer, unsigned gic_distributor_address, int is_secure)
{
    interrupt_distributor *id = (interrupt_distributor *)gic_distributor_address;
    
    /* We assume the distributor is disabled so we can write to its config registers */

    id->enable.set[0] = *pointer;
    ++pointer;
    copy_words(id->priority, pointer, 8);
    pointer += 8;
    copy_words(id->target, pointer, 8);
    pointer += 8;
    if (is_secure)
    {
        id->security[0] = *pointer;
        ++pointer;
    }
    /* Restore just the PPI configurations (SGIs are not configurable) */
    id->configuration[1] = *pointer;
    ++pointer;
    id->pending.set[0] = *pointer;
}

void restore_gic_distributor_shared(appf_u32 *pointer, unsigned gic_distributor_address, int is_secure)
{
    interrupt_distributor *id = (interrupt_distributor *)gic_distributor_address;
    unsigned num_spis;
    
    /* Make sure the distributor is disabled */
    gic_distributor_set_enabled(FALSE, gic_distributor_address);

    /* Calculate how many SPIs the GIC supports */
    num_spis = 32 * ((id->controller_type) & 0x1f);

    /* TODO: add nonsecure stuff */

    /* Restore rest of GIC configuration */
    if (num_spis)
    {
        copy_words(id->enable.set + 1, pointer, num_spis / 32);
        pointer += num_spis / 32;
        copy_words(id->priority + 8, pointer, num_spis / 4);
        pointer += num_spis / 4;
        copy_words(id->target + 8, pointer, num_spis / 4);
        pointer += num_spis / 4;
        copy_words(id->configuration + 2, pointer, num_spis / 16);
        pointer += num_spis / 16;
        if (is_secure)
        {
            copy_words(id->security + 1, pointer, num_spis / 32);
            pointer += num_spis / 32;
        }
        copy_words(id->pending.set + 1, pointer, num_spis / 32);
        pointer += num_spis / 32;
    }
        
    /* Restore control register - if the GIC was disabled during save, it will be restored as disabled. */
    id->control = *pointer;
    
    return;
}

