/*******************************************************************************************
Copyright 2010 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement 
governing use of this software, this software is licensed to you under the 
terms of the GNU General Public License version 2, available at 
http://www.gnu.org/copyleft/gpl.html (the "GPL"). 

Notwithstanding the above, under no circumstances may you combine this software 
in any way with any other Broadcom software provided under a license other than 
the GPL, without Broadcom's express prior written consent.
*******************************************************************************************/

#include <linux/broadcom/bcm_gememalloc_wrapper.h>
#include <linux/module.h>

#include <cfg_global.h>
/* Our header */
#include "gememalloc.h"

#define GEMEMALLOC_SW_MINOR 0
#define GEMEMALLOC_SW_MAJOR 1
#define GEMEMALLOC_SW_BUILD ((GEMEMALLOC_SW_MAJOR * 1000) + GEMEMALLOC_SW_MINOR)

#define PAGE_SHIFT 12
#define GEMEMALLOC_SIZE   (1024*1024*2)

MODULE_LICENSE("Proprietary");
MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("Memory allocation");

typedef struct _chunk{
    unsigned int address;
    unsigned int size;
    bool used;
    struct _chunk* prev;
    struct _chunk* next;
} chunk;

static chunk* memhead = NULL;
static int gememalloc_logic_init(unsigned int memory_size, unsigned int phy_start_address);


static int AllocateMemory(GEMEMALLOC_HDL hdl, unsigned long *busaddr,
			unsigned int size);
static int FreeMemory(GEMEMALLOC_HDL hdl, unsigned long *busaddr);

static int gememalloc_open(GEMEMALLOC_HDL *hdlp)
{

	PDEBUG(KERN_DEBUG "gememalloc_open\n");

	return 0;
}

static int gememalloc_release(GEMEMALLOC_HDL hdl)
{

	PDEBUG(KERN_DEBUG "gememalloc_release\n");

	return 0;
}

static int gememalloc_mmap(unsigned long size, unsigned long pgoff)
{

	chunk* head = memhead;
    while(head != NULL)
    {
        if (((head->address>> PAGE_SHIFT) == pgoff ) && (size == head->size))
        {
            break;
        }
        head = head->next;
    }
    if(head == NULL){
		printk(KERN_ERR "%s(): invalid mmap parameters\n", __FUNCTION__);
		return -1;
	}

	return 0;
}

static struct gememalloc_logic gememalloc_fops = {
	.AllocMemory = AllocateMemory,
	.FreeMemory = FreeMemory,
	.open = gememalloc_open,
	.release = gememalloc_release,
	.mmap = gememalloc_mmap,
	.init = gememalloc_logic_init,
	.cleanup = NULL,
};

int gememalloc_init(void)
{
	int result = 0;

	PDEBUG(KERN_DEBUG "gememalloc_init\n");
	printk(KERN_INFO "gememalloc  sw build: %d \n", GEMEMALLOC_SW_BUILD);

	result = register_gememalloc_wrapper(&gememalloc_fops);
	if (result < 0) {
		printk(KERN_ERR "gememalloc: module not inserted\n");
		return result;
	}

	return 0;
}

static int gememalloc_logic_init(unsigned int memory_size,
				unsigned int phy_start_address)
{
    memhead = kmalloc(sizeof(chunk),GFP_KERNEL);

    memhead->address = phy_start_address;
    memhead->size = memory_size;
    memhead->used = false;
    memhead->prev = NULL;
    memhead->next = NULL;

	return 0;
}

void gememalloc_cleanup(void)
{

	kfree(memhead);
	memhead = NULL;
	deregister_gememalloc_wrapper();

	PDEBUG(KERN_DEBUG "gememalloc_cleanup\n");
	printk(KERN_NOTICE "gememalloc: module removed\n");

	return;
}

module_init(gememalloc_init);
module_exit(gememalloc_cleanup);

/* Cycle through the buffers we have, give the first free one */
static int AllocateMemory(GEMEMALLOC_HDL hdl, unsigned long *busaddr,
			unsigned int size)
{

    chunk* curr = memhead;

	*busaddr = 0;

    do
    {
        if (curr->used == false)
        {
            if(curr->size > size)
            {
                chunk* temp = kmalloc(sizeof(chunk),GFP_KERNEL);
                temp->next = curr->next;
                temp->prev = curr;
                curr->next = temp;
                temp->size = curr->size - size;
                temp->address = curr->address + size;
                curr->size = size;
                temp->used = false;
                curr->used = true;
                if(temp->next)
                {
                    temp->next->prev = temp;
                }
				*busaddr = (unsigned long) curr->address;
				break;
            }
            else if(curr->size == size)
            {
                curr->used = true;
				*busaddr = (unsigned long) curr->address;
				break;
            }
        }
        curr = curr->next;
    }while(curr != 0);

	if (*busaddr == 0) {
		printk(KERN_ERR "GEMEMALLOC FAILED: size = %d\n", size);
	} else {
		printk("GEMEMALLOC OK: size: %d, size reserved: %d, bus_address: 0x%08x\n", size, curr->size, *busaddr);
	}

	return 0;
}

/* Free a buffer based on bus address */
static int FreeMemory(GEMEMALLOC_HDL hdl, unsigned long *busaddr)
{
	chunk* head = memhead;
	int address = *busaddr;
    while(head != NULL)
    {
        if ( head->address == address)
        {
            break;
        }
        head = head->next;
    }
    if(head != NULL)
		{
		printk("MEMFREE OK: size: 0x%08x, addr: 0x%08x\n", head->size, head->address);
		head->used = false;
		if(head->prev != NULL )
			{
			if(head->prev->used == false)
				{
				chunk* prev = head->prev;
				prev->next = head->next;
				if(head->next)
					{
					head->next->prev = prev;
					}
				prev->size += head->size;
				kfree(head);
				head = prev;
				}
			}
		if(head->next != NULL )
			{
			if(head->next->used == false)
				{
				chunk* next = head->next;
				head->next = next->next;
				head->size += next->size;
				if(next->next)
					{
					next->next->prev = head;
					}
				kfree(next);
				}
			}
		}
	else
		{
		printk(KERN_ERR "MEMFREE fail: addr: 0x%08x\n", (unsigned int)*busaddr);
		}


	return 0;
}

