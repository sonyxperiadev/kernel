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
#include "appf_platform_api.h"
#include "appf_helpers.h"

#define SECTION    (1<<1)
#define PAGE_TABLE (1<<0)
#define SMALL_PAGE (1<<1)
#define CACHE      (1<<3)
#define BUFFER     (1<<2)

/* Section bits */
#define S_NG       (1<<17)
#define S_S        (1<<16)
#define S_APX      (1<<15)
#define S_TEX0     (1<<12)
#define S_TEX1     (1<<13)
#define S_TEX2     (1<<14)
#define S_AP0      (1<<10)
#define S_AP1      (1<<11)
#define S_PARITY   (1<<9)
#define S_XN       (1<<4)
#define S_NS       (1<<19)

/* Small Page bits */
#define SP_NG      (1<<11)
#define SP_S       (1<<10)
#define SP_AP2     (1<<9)
#define SP_TEX2    (1<<8)
#define SP_TEX1    (1<<7)
#define SP_TEX0    (1<<6)
#define SP_AP1     (1<<5)
#define SP_AP0     (1<<4)
#define SP_XN      (1<<0)

/* Page table bits */
#define PT_NS      (1<<3)


#define PAGE_SIZE     4096
#define PAGE_SHIFT    12
#define PAGE_MASK     0xfffff000
#define SECTION_MASK  0xfff00000
#define SECTION_SHIFT 20
#define TRE_MASK      (1<<28)


unsigned appf_device_memory_flat_mapped;

/*
 * find_mapping: Searches the PRRR and NMRR and returns memory attribute bits:
 *   bit 2: TEX0
 *   bit 1: C
 *   bit 0: B
 */

#define M_TEX0 (1<<2)
#define M_C    (1<<1)
#define M_B    (1<<0)
#define REMAPPING_DEVICE 1
#define REMAPPING_NORMAL 2
#define CACHE_WBWA 0x1

int find_remapping(unsigned prrr, unsigned nmrr, unsigned desired_type)
{
    int i;
    unsigned type, outer_cacheable, inner_cacheable;
    
    /*
     * Search through the PRRR for the desired memory type 
     */
    for (i = 0; i <= 7; ++i)
    {
        if (i == 6)
        {
            continue; /* Remapping 6 is IMPLEMENTATION DEFINED, ignore */
        }
        type = (prrr >> (2*i)) & 0x3;
        if (type == desired_type)
        {
            /* for Normal memory: check inner/outer cache attributes of this remapping */
            if (desired_type == REMAPPING_NORMAL) 
            {
                inner_cacheable = (nmrr >> (2*i)) & 0x3;
                outer_cacheable = (nmrr >> (16 + 2*i)) & 0x3;
                if ((inner_cacheable == CACHE_WBWA) && (outer_cacheable == CACHE_WBWA))
                {
                    break;  /* Found a suitable remapping */
                }
            }
            else /* for other memory types, we don't care about cache attributes */
            {
                break;  /* Found a suitable remapping */
            }
        }
    }
    
    if (i<=7)
    {
        return i;
    }
    else
    {
        return APPF_NO_MEMORY_MAPPING;
    }
}

int find_shareable_bit(unsigned prrr, unsigned type, unsigned desired_shareable)
{
    unsigned s0, s1;
    
    if (type == REMAPPING_DEVICE)
    {
        s0 = (prrr >> 16) & 0x1;
        s1 = (prrr >> 17) & 0x1;
    }
    else /* Normal memory */
    {
        s0 = (prrr >> 18) & 0x1;
        s1 = (prrr >> 19) & 0x1;
    }

    /* Deal with special case: both values of S bit encode the same shareability */
    if (s0 == s1)
    {
        if (s0 != desired_shareable)
        {
            return APPF_NO_MEMORY_MAPPING;
        }
        else
        {
            return s0;  /* Doesn't matter which value of S bit is chosen! */
        }
    }
    
    /* Otherwise: one value of S means shareable, the other means non-shareable */
    return s0 ^ desired_shareable;
}

int appf_setup_translation_tables(void)
{
     unsigned firmware_start_pa, firmware_start_va, pa, device_page_pa, pages_offset;
    int remapping, i;
    appf_u32 section_attributes_normal, 
        section_attributes_device, 
        page_attributes_normal, 
        page_attributes_device, 
        page_table_security,
        attributes,
        nmrr, prrr, attr;
    unsigned tex_remap, ram_start_mb, ram_end_mb, shareable;
#ifdef CAPRI_DORMANT_CHANGE
	extern unsigned num_physpages;
#endif
    
#ifdef CAPRI_DORMANT_CHANGE
    firmware_start_va = ((unsigned)&main_table) & PAGE_MASK;
#else
    firmware_start_va = ((unsigned)&main_table) & PAGE_MASK;
#endif

    firmware_start_pa = va_to_pa(firmware_start_va);
   
    tex_remap = read_sctlr() & TRE_MASK;
    
    if (tex_remap)
    {
        /*
         * Find some correct attributes for the types of translation
         * table entries we need.
         */
        nmrr = read_nmrr();
        prrr = read_prrr();

        /* First, find a Normal remapping */
        remapping = find_remapping(prrr, nmrr, REMAPPING_NORMAL);
        if (remapping < 0)
        {
            return remapping; /* Could not find appropriate mapping: return error */
        }
        section_attributes_normal = SECTION | S_AP0 | S_AP1;
        if (remapping & M_B)    section_attributes_normal |= BUFFER;
        if (remapping & M_C)    section_attributes_normal |= CACHE;
        if (remapping & M_TEX0) section_attributes_normal |= S_TEX0;

        page_attributes_normal = SMALL_PAGE | SP_AP0 | SP_AP1;
        if (remapping & M_B)    page_attributes_normal |= BUFFER;
        if (remapping & M_C)    page_attributes_normal |= CACHE;
        if (remapping & M_TEX0) page_attributes_normal |= SP_TEX0;

        shareable = find_shareable_bit(prrr, REMAPPING_NORMAL, is_smp);
        if (shareable)
        {
            section_attributes_normal |= S_S;
            page_attributes_normal    |= SP_S;
        }

        /* Second, find a Device remapping */
        remapping = find_remapping(prrr, nmrr, REMAPPING_DEVICE);
        if (remapping < 0)
        {
            return remapping; /* Could not find appropriate mapping: return error */
        }
        section_attributes_device = SECTION | S_AP0 | S_AP1 | S_XN;
        if (remapping & M_B)    section_attributes_device |= BUFFER;
        if (remapping & M_C)    section_attributes_device |= CACHE;
        if (remapping & M_TEX0) section_attributes_device |= S_TEX0;

        page_attributes_device = SMALL_PAGE | SP_AP0 | SP_AP1 | SP_XN;
        if (remapping & M_B)    page_attributes_device |= BUFFER;
        if (remapping & M_C)    page_attributes_device |= CACHE;
        if (remapping & M_TEX0) page_attributes_device |= SP_TEX0;

        shareable = find_shareable_bit(prrr, REMAPPING_DEVICE, is_smp);
        if (shareable)
        {
            section_attributes_device |= S_S;
            page_attributes_device    |= SP_S;
        }

    }
    else
    {
        /*
         * Without TEX remap, we can simply use the standard translation table attributes
         */
        section_attributes_normal = SECTION | CACHE | BUFFER | S_AP0 | S_AP1 | S_TEX0;
        section_attributes_device = SECTION | BUFFER | S_AP0 | S_AP1 | S_XN;
        page_attributes_normal = SMALL_PAGE | CACHE | BUFFER | SP_AP0 | SP_AP1 | SP_TEX0;
        page_attributes_device = SMALL_PAGE | BUFFER | SP_AP0 | SP_AP1 | SP_XN;
        if (is_smp)
        {
            section_attributes_normal |= S_S;
            section_attributes_device |= S_S;
            page_attributes_normal    |= SP_S;
        }
    }

#if NONSECURE_ONLY
    /* All Non-Secure - don't need to set any NS bits, all accesses will be NS anyway. */
    page_table_security = 0;
#elif SECURE_ONLY
    /* All Secure - don't set any NS bits, all accesses will be S. */
    page_table_security = 0;
#else
    /* 
     * APPF is operating both NS and S, so we mark all memory accesses NS, but leave 
     * devices accesses unmarked. That way, APPF always sees the same memory, but 
     * device access security depends on the current security state.
     */
    section_attributes_normal |= S_NS;
    page_table_security = PT_NS;
#endif
    
    /*
     * Create first level translation table
     *
     * These entries are all flat-mapped
     * One will be changed later to point to the second level table.
     */
         
    ram_start_mb = main_table.ram_start >> 20;
#ifdef CAPRI_DORMANT_CHANGE
    ram_end_mb = ram_start_mb + ( (num_physpages << PAGE_SHIFT) >> 20);
#else
    ram_end_mb = ram_start_mb + (main_table.ram_size >> 20);
#endif

    for (i = 0; i < 4096; ++i)
    {
        if (i >= ram_start_mb && i < ram_end_mb)
        {
            attr = section_attributes_normal;
        }
        else
        {
            attr = section_attributes_device;
        }
        appf_translation_table1[i] = attr | (i<<20);
    }

    /*
     * Add pointers to second level translation tables.
     * When called from a flat-mapped application, these tables will actually be in the same place,
     * so we take care to make them exactly the same!
     */
    appf_translation_table1[firmware_start_va >> SECTION_SHIFT]
        = va_to_pa((unsigned)appf_translation_table2a) | PAGE_TABLE | page_table_security;

    appf_translation_table1[firmware_start_pa >> SECTION_SHIFT]
        = va_to_pa((unsigned)appf_translation_table2b) | PAGE_TABLE | page_table_security;
    
    /*
     * Create second level translation tables
     * The first is not flat-mapped - it maps the firmware in the OS space.
     * The second is flat-mapped - its purpose is to create a DEVICE page
     * for the locking algorithm to use.
     */
#ifdef CAPRI_DORMANT_CHANGE
	pages_offset = 0;
    pa = firmware_start_pa & SECTION_MASK;
#else
    pages_offset = (firmware_start_va & ~SECTION_MASK) >> PAGE_SHIFT;
    pa = firmware_start_pa;
#endif
    device_page_pa = appf_device_memory_flat_mapped & PAGE_MASK;

    for (i = pages_offset; i < 256; ++i)
    {
        if (pa == device_page_pa)
        {
            attributes = page_attributes_device;
        }
        else
        {
            attributes = page_attributes_normal;
        }
        appf_translation_table2a[i] = attributes | pa;
        pa += PAGE_SIZE;
    }

#ifdef CAPRI_DORMANT_CHANGE
	pages_offset = 0;
    pa = firmware_start_pa & SECTION_MASK;
#else
    pages_offset = (firmware_start_pa & ~SECTION_MASK) >> PAGE_SHIFT;
    pa = firmware_start_pa;
#endif
    for (i = pages_offset; i < 256; ++i)
    {
        if (pa == device_page_pa)
        {
            attributes = page_attributes_device;
        }
        else
        {
            attributes = page_attributes_normal;
        }
        appf_translation_table2b[i] = attributes | pa;
        pa += PAGE_SIZE;
    }
    
    /* Setup values for TTBR0 and TTBCR */
    /* TODO: Add suitable values for non-A9 CPUs */

    /* Calculate physical address of translation tables */
    appf_ttbr0 = va_to_pa((unsigned)appf_translation_table1);
    
    /* Add appropriate cacheable/shareable attributes */
    if (is_smp)
    {
        appf_ttbr0 |= 0x4a; /* Table walks are Inner/Outer cacheable WBWA, Inner/Outer Shareable */
    }
    else
    {
        appf_ttbr0 |= 0x09; /* Table walks are Inner/Outer cacheable WBWA */
    }
    
    appf_ttbcr = 0x0;  /* Always use TTBR0 */
    
    return APPF_OK;
}

