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

typedef struct
{
    /* 0x00 */ volatile unsigned timer_load;
    /* 0x04 */ volatile unsigned timer_counter;
    /* 0x08 */ volatile unsigned timer_control;
    /* 0x0c */ volatile unsigned timer_interrupt_status;
                char padding1[0x10];
    /* 0x20 */ volatile unsigned watchdog_load;
    /* 0x24 */ volatile unsigned watchdog_counter;
    /* 0x28 */ volatile unsigned watchdog_control;
    /* 0x2c */ volatile unsigned watchdog_interrupt_status;
    /* 0x30 */ volatile unsigned watchdog_reset_status;
    /* 0x34 */ volatile unsigned watchdog_disable;
} a9_timer_registers;

typedef struct
{
    unsigned timer_load;
    unsigned timer_counter;
    unsigned timer_control;
    unsigned timer_interrupt_status;
    unsigned watchdog_load;
    unsigned watchdog_counter;
    unsigned watchdog_control;
    unsigned watchdog_interrupt_status;
} a9_timer_context;

#define A9_TIMERS_OFFSET 0x600

void save_a9_timers(appf_u32 *pointer, unsigned scu_address)
{
    a9_timer_context *context = (a9_timer_context *)pointer;
    a9_timer_registers *timers = (a9_timer_registers *)(scu_address + A9_TIMERS_OFFSET);

    /* 
     * First, stop the timers
     */

    context->timer_control    = timers->timer_control;
    timers->timer_control     = 0;
    context->watchdog_control = timers->watchdog_control;
	timers->watchdog_control  = 0;

    context->timer_load                = timers->timer_load;
    context->timer_counter             = timers->timer_counter;
    context->timer_interrupt_status    = timers->timer_interrupt_status;
    context->watchdog_load             = timers->watchdog_load;
    context->watchdog_counter          = timers->watchdog_counter;
    context->watchdog_interrupt_status = timers->watchdog_interrupt_status;
    /* 
     * We ignore watchdog_reset_status, since it can only clear the status bit.
     * If the watchdog has reset the system, the OS will want to know about it.
     * Similarly, we have no use for watchdog_disable - this is only used for
     * returning to timer mode, which is the default mode after reset.
     */
}

void restore_a9_timers(appf_u32 *pointer, unsigned scu_address)
{
    a9_timer_context *context = (a9_timer_context *)pointer;
    a9_timer_registers *timers = (a9_timer_registers *)(scu_address + A9_TIMERS_OFFSET);

    timers->timer_control = 0;
    timers->watchdog_control = 0;

    /*
     * We restore the load register first, because it also sets the counter register.
     */
    timers->timer_load    = context->timer_load;
    timers->watchdog_load = context->watchdog_load;

    /*
     * If a timer has reached zero (presumably during the context save) and triggered
     * an interrupt, then we set it to the shortest possible expiry time, to make it
     * trigger again real soon.
     * We could fake this up properly, but we would have to wait around until the timer 
     * ticked, which could be some time if PERIPHCLK is slow. This approach should be 
     * good enough in most cases.
     */
    if (context->timer_interrupt_status)
    {
        timers->timer_counter = 1;
    }
    else
    {
        timers->timer_counter = context->timer_counter;
    }

    if (context->watchdog_interrupt_status)
    {
        timers->watchdog_counter = 1;
    }
    else
    {
        timers->watchdog_counter = context->watchdog_counter;
    }

    timers->timer_control = context->timer_control;
    timers->watchdog_control = context->watchdog_control;
}


typedef struct
{
    /* 0x00 */ volatile unsigned counter_lo;
    /* 0x04 */ volatile unsigned counter_hi;
    /* 0x08 */ volatile unsigned control;
    /* 0x0c */ volatile unsigned status;
    /* 0x10 */ volatile unsigned comparator_lo;
    /* 0x14 */ volatile unsigned comparator_hi;
    /* 0x18 */ volatile unsigned auto_increment;
} a9_global_timer_registers;

typedef struct
{
    unsigned counter_lo;
    unsigned counter_hi;
    unsigned control;
    unsigned status;
    unsigned comparator_lo;
    unsigned comparator_hi;
    unsigned auto_increment;
} a9_global_timer_context;

#define A9_GLOBAL_TIMER_OFFSET      0x200
#define A9_GT_TIMER_ENABLE          (1<<0)
#define A9_GT_COMPARE_ENABLE        (1<<1)
#define A9_GT_AUTO_INCREMENT_ENABLE (1<<3)
#define A9_GT_EVENT_FLAG            (1<<0)

void save_a9_global_timer(appf_u32 *pointer, unsigned scu_address)
{
    a9_global_timer_registers *timer = (void*)(scu_address + A9_GLOBAL_TIMER_OFFSET);
    a9_global_timer_context *context = (void*)pointer;

    unsigned tmp_lo, tmp_hi, tmp2_hi;

    do 
    {
        tmp_hi  = timer->counter_hi;
        tmp_lo  = timer->counter_lo;
        tmp2_hi = timer->counter_hi;
    } while (tmp_hi != tmp2_hi);

    context->counter_lo     = tmp_lo;
    context->counter_hi     = tmp_hi;
    context->control        = timer->control;
    context->status         = timer->status;
    context->comparator_lo  = timer->comparator_lo;
    context->comparator_hi  = timer->comparator_hi;
    context->auto_increment = timer->auto_increment;
}

void restore_a9_global_timer(appf_u32 *pointer, unsigned scu_address)
{
    a9_global_timer_registers *timer = (void*)(scu_address + A9_GLOBAL_TIMER_OFFSET);
    a9_global_timer_context *context = (void*)pointer;

    unsigned long long comparator_ull, current_ull;
    unsigned current_hi, current_lo;

    /* Is the timer currently enabled? */
    if (timer->control & A9_GT_TIMER_ENABLE)
    {
        /* Temporarily stop the timer so we can mess with it */
        timer->control &= ~A9_GT_TIMER_ENABLE;
    }
    else /* We must be the first CPU back up, or the timer is not in use */
    {
        /*
         * Restore the counter value - possibly we should update it to
         * reflect the length of time the power has been off
         */
        timer->counter_lo    = context->counter_lo;
        timer->counter_hi    = context->counter_hi;
    }

    /*
     * Timer is now stopped - do we need to set its event flag? 
     * We set it if the event flag was set when the context was saved,
     * or the timer has expired and the Compare control bit was set.
     */
    current_hi     = timer->counter_hi;
    current_lo     = timer->counter_lo;
    current_ull    = ((unsigned long long)current_hi << 32) + current_lo;
    comparator_ull = ((unsigned long long)context->comparator_hi << 32) + context->comparator_lo;

    if ((context->status & A9_GT_EVENT_FLAG) ||
	    ((current_ull > comparator_ull) &&
	    ((context->control & (A9_GT_COMPARE_ENABLE | A9_GT_TIMER_ENABLE))
		== ((A9_GT_COMPARE_ENABLE | A9_GT_TIMER_ENABLE))))) {
        /* Set the comparator to the current counter value */
        timer->comparator_hi = current_hi;
        timer->comparator_lo = current_lo;

        /* Start the timer */
        timer->control = context->control;

        /* Wait for the timer event */
        while (timer->status == 0)
        {
                /* Do nothing */
        }

        /* Set the comparator to the original value */
        timer->comparator_lo = context->comparator_lo;
        timer->comparator_hi = context->comparator_hi;
    }
    else /* Event flag does not need to be set */
    {
        timer->comparator_lo = context->comparator_lo;
        timer->comparator_hi = context->comparator_hi;
        /* Start the timer */
        timer->control = context->control;
    }

    timer->auto_increment = context->auto_increment;
}
