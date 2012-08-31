/*******************************************************************************
*  Copyright 2003 - 2007 Broadcom Corporation.  All rights reserved.
*
*  Unless you and Broadcom execute a separate written software license
*  agreement governing use of this software, this software is licensed to you
*  under the terms of the GNU General Public License version 2, available at
*  http://www.gnu.org/copyleft/gpl.html (the "GPL").
*
*  Notwithstanding the above, under no circumstances may you combine this
*  software in any way with any other Broadcom software provided under a
*  license other than the GPL, without Broadcom's express prior written
*  consent.
*
*******************************************************************************/

#include <linux/preempt.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <asm/io.h>
#include <asm/memory.h>

#include "lnx_support.h"
#include "ipc_debug.h"

extern void intc_trigger_softirq(unsigned int irq);
struct tasklet_struct *intr_tasklet_ptr_g = NULL;

/**
 *  @fn void bcm_lnx_init(struct tasklet_struct *tsklet_ptr);
 *
 *  Note: this api is to avoid cyclic dependency between this file and
 *        the ipc_proxy_server code.
**/
void bcm_lnx_init(struct tasklet_struct *tsklet_ptr)
{
	intr_tasklet_ptr_g = tsklet_ptr;
}

/**
 * From FualDualProcessorInterfaceSpec.doc  [point #13 under section 2]
 *
 * "All virtual to physical address mappings are done by the IPC module assuming
 * that the entire shared memory area occupies a contiguous memory range. They
 * are calculated by adding offsets to the shared memory start address supplied
 * to IPC as a startup parameter. The address conversion callbacks supplied
 * during IPC initialization are not currently used."
 *
 * So, the following two APIs for address conversion is never invoked from the
 * the IPC lib. But these are provided here for completion.
 */

/**
 * unsigned int bcm_map_virt_to_phys(void *virt_addr);
 */
unsigned int bcm_map_virt_to_phys(void *virt_addr)
{
	/**
	 * Note: since shared-mem is io-mem, these kernel call to map
	 * virt to phys does not work
	 *
	 * return((unsigned int)virt_to_phys(virt_addr));
	 */
	IPC_DEBUG(DBG_TRACE, "should not be used on this address\n");

	return 0;
}

/**
 * void *bcm_map_phys_to_virt(unsigned int physical_addr);
 */
void *bcm_map_phys_to_virt(unsigned int physical_addr)
{
	/**
	 * Note: since shared-mem is io-mem, these kernel call to map
	 * phys to virt does not work
	 *
	 * return((void *)phys_to_virt((unsigned long)physical_addr));
	 */
	IPC_DEBUG(DBG_TRACE, "should not be used on this address\n");

	return NULL;
}

/**
 * void bcm_raise_cp_int(void);
 */
void bcm_raise_cp_int(void)
{
	/* Write into software interrupt register to interrupt cp */
	intc_trigger_softirq(IRQ_IPC_A2C);
}

void *bcm_create_lock(void)
{
	void *lock;
	lock = kmalloc(sizeof(spinlock_t), GFP_ATOMIC);
	if (!lock)
	{
		IPC_DEBUG(DBG_ERROR, "IPC: failed to create spinlock\n");
	}
	else
	{
		spin_lock_init((spinlock_t *)lock);
		IPC_DEBUG(DBG_TRACE, "IPC: spinlock initialized\n");
	}
	return lock;
}

/**
   void bcm_release_lock(void *);
*/
void bcm_release_lock(void * lock)
{ 
	if (lock)
	{
		spin_unlock_bh((spinlock_t *)lock);
	}
	else
	{	
		preempt_enable();   

		local_irq_enable();
		if (intr_tasklet_ptr_g)
		{
			tasklet_enable(intr_tasklet_ptr_g);
		}
	}
}

/**
   void bcm_aquire_lock(void *);
*/
void bcm_aquire_lock(void * lock)
{
	if (lock)
	{
		spin_lock_bh((spinlock_t *)lock);
	}
	else
	{
		preempt_disable();
		local_irq_disable();

		if (intr_tasklet_ptr_g)
		{
			tasklet_disable(intr_tasklet_ptr_g);
		}
	}
}

/**
  void bcm_delete_lock(void *);
  */
void bcm_delete_lock(void *lock)
{
	if (lock) {
		IPC_DEBUG(DBG_TRACE, "IPC: bcm_delete_lock %p\n", lock);
		kfree(lock);
	}
}
