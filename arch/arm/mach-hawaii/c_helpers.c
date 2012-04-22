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
 
 
/**
 * Lamport's Bakery algorithm for spinlock handling
 *
 * Note that the algorithm requires the stack and the bakery struct
 * to be in Strongly-Ordered memory.
 */

#include "appf_types.h"
#include "appf_internals.h"
#include "appf_helpers.h"

/**
 * Initialize a bakery - only required if the bakery_t is
 * on the stack or heap, as static data is zeroed anyway.
 */
void initialize_spinlock(bakery_t *bakery)
{
    appf_memset(bakery, 0, sizeof(bakery_t));
}

/**
 * Claim a bakery lock. Function does not return until
 * lock has been obtained.
 */
void get_spinlock(unsigned cpuid, bakery_t *bakery)
{
    unsigned i, max=0, my_full_number, his_full_number;

    /* Get a ticket */
    bakery->entering[cpuid] = TRUE;
    for (i=0; i<MAX_CPUS; ++i)
    {
        if (bakery->number[i] > max)
        {
            max = bakery->number[i];
        }
    }
    ++max;
    bakery->number[cpuid] = max;
    bakery->entering[cpuid] = FALSE;

    /* Wait for our turn */
    my_full_number = (max << 8) + cpuid;
    for (i=0; i<MAX_CPUS; ++i)
    {
        while(bakery->entering[i]);  /* Wait */
        do
        {
            his_full_number = bakery->number[i];
            if (his_full_number)
            {
                his_full_number = (his_full_number << 8) + i;
            }
        }
        while(his_full_number && (his_full_number < my_full_number));
    }
    dmb();
}

/**
 * Release a bakery lock.
 */
void release_spinlock(unsigned cpuid, bakery_t *bakery)
{
    dmb();
    bakery->number[cpuid] = 0;
}
