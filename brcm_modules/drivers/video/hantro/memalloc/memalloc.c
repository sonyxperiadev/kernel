/*------------------------------------------------------------------------------
--                                                                            --
--       This software is confidential and proprietary and may be used        --
--        only as expressly authorized by a licensing agreement from          --
--                                                                            --
--                            Hantro Products Oy.                             --
--                                                                            --
--                   (C) COPYRIGHT 2006 HANTRO PRODUCTS OY                    --
--                            ALL RIGHTS RESERVED                             --
--                                                                            --
--                 The entire notice above must be reproduced                 --
--                  on all copies and should not be removed.                  --
--                                                                            --
--------------------------------------------------------------------------------
--
--  Abstract : Allocate memory blocks
--
--------------------------------------------------------------------------------
--
--  Version control information, please leave untouched.
--
--  $RCSfile: memalloc.c,v $
--  $Date: 2006/10/10 12:27:31 $
--  $Revision: 1.16 $
--
------------------------------------------------------------------------------*/

#include <linux/broadcom/bcm_memalloc_wrapper.h>
#include <linux/module.h>

#include <cfg_global.h>
/* Our header */
#include "memalloc.h"

#define MEMALLOC_SW_MINOR 4
#define MEMALLOC_SW_MAJOR 0
#define MEMALLOC_SW_BUILD ((MEMALLOC_SW_MAJOR * 1000) + MEMALLOC_SW_MINOR)

#define PAGE_SHIFT 12
#define MAX_OPEN 32
#define ID_UNUSED 0xFF
#define MEMALLOC_BASIC 0
#define MEMALLOC_MAX_OUTPUT 1
#define MEMALLOC_SIZE   (SZ_4M)

MODULE_LICENSE("Proprietary");
MODULE_AUTHOR("Hantro Products Oy");
MODULE_DESCRIPTION("RAM allocation");

static int id[MAX_OPEN] = { ID_UNUSED };

struct list_head heap_list;

typedef struct hlinc {
	unsigned int bus_address;
	unsigned int used;
	unsigned int size;
	void *file_id;
} hlina_chunk;

static int memalloc_logic_init(unsigned int memory_size, unsigned int phy_start_address);

#if defined(CONFIG_ARCH_BCM215XX) || defined(CONFIG_ARCH_BCM116X)
/* BCM21553  or BCM2157 */
unsigned int size_table_0[] = {
#if 0
	/*1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1,
	4, 4, 4, 4, 4, 4, 4, 4,
	5, 5, 5, 5, 5, 5, 5, 5, 5,
	6, 6, 6, 6, 6, 6, 6,6, */
	10, 10, 10,
	22, 22, 22,
	38, 38, 38,
	38, 38,	38, //38, 38, 38, 38,
	54, 54,	54,
	128, 128, 128, 154	/* 128 */ /* 256 */
#else
	10, 10, 10, 			// QCIF 176*144*1.5
	24, 24, 			// for input buffer
	57, 57, 57, 57,			// HVGA 480*320*1.5
	113, 113, 114, 114, 114,	// VGA 640*480*1.5
	150 				// VGA 640*480*2
#endif
};
#else
/*4MB in total*/
unsigned int size_table_0[] = {
	1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1,
	4, 4, 4, 4, 4, 4, 4, 4,
	5, 5, 5, 5, 5, 5, 5, 5,
	6, 6, 6, 6, 6, 6, 6, 6,
	10, 10, 10, 10,
	22, 22, 22, 22,
	38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38,
	309
};
#endif

#define HLINA_NUMBER_OF_CHUNKS (sizeof(size_table_0)/sizeof(unsigned int))

hlina_chunk hlina_chunks[HLINA_NUMBER_OF_CHUNKS];

static int AllocMemory(MEMALLOC_HDL hdl, unsigned long *busaddr,
			unsigned int size);
static int FreeMemory(MEMALLOC_HDL hdl, unsigned long *busaddr);

static int memalloc_open(MEMALLOC_HDL *hdlp)
{
	int i=0;

	PDEBUG(KERN_DEBUG "memalloc_open\n");

	for (i = 0; i < MAX_OPEN + 1; i++) {

		if (i == MAX_OPEN)
			return -1;
		if (id[i] == ID_UNUSED) {
			id[i] = i;
			*hdlp = id + i;
			break;
		}
	}

	return 0;
}

static int memalloc_release(MEMALLOC_HDL hdl)
{

	int i = 0;
	for (i = 0; i < HLINA_NUMBER_OF_CHUNKS; i++) {
		if (hlina_chunks[i].file_id == hdl) {
			hlina_chunks[i].used = 0;
			hlina_chunks[i].file_id = NULL;
			printk("Freed some memory in release for %d\n", hdl);
		}
	}

	*((int*)hdl) = ID_UNUSED;
	PDEBUG(KERN_DEBUG "memalloc_release\n");

	return 0;
}

static int memalloc_mmap(unsigned long size, unsigned long pgoff)
{
	int i=0;

	for (i = 0; i < HLINA_NUMBER_OF_CHUNKS; i++) {
		if (hlina_chunks[i].used) {
			if (((hlina_chunks[i].bus_address >> PAGE_SHIFT) ==
			     pgoff)
			    && (hlina_chunks[i].size >= size))
				break;
		}
	}

	if (i >= HLINA_NUMBER_OF_CHUNKS) {
		printk(KERN_ERR "%s(): invalid mmap parameters\n", __FUNCTION__);
		return -1;
	}

	return 0;
}

static struct memalloc_logic memalloc_fops = {
	.AllocMemory = AllocMemory,
	.FreeMemory = FreeMemory,
	.open = memalloc_open,
	.release = memalloc_release,
	.mmap = memalloc_mmap,
	.init = memalloc_logic_init,
	.cleanup = NULL,
};

int memalloc_init(void)
{
	int result, i = 0;

	PDEBUG(KERN_DEBUG "memalloc_init\n");
	printk(KERN_INFO "memalloc x170 sw build: %d \n", MEMALLOC_SW_BUILD);

	result = register_memalloc_wrapper(&memalloc_fops);
	if (result < 0) {
		printk(KERN_ERR "memalloc: module not inserted\n");
		return result;
	}

	/* We keep a register of out customers, reset it */
	for (i = 0; i < MAX_OPEN; i++)
		id[i] = ID_UNUSED;

	return 0;
}

static int memalloc_logic_init(unsigned int memory_size,
				unsigned int phy_start_address)
{
	int i;
	unsigned int ba;

	ba = phy_start_address;
	for (i = 0; i < HLINA_NUMBER_OF_CHUNKS; i++) {
		hlina_chunks[i].bus_address = ba;
		hlina_chunks[i].used = 0;
		hlina_chunks[i].file_id = NULL;
		hlina_chunks[i].size = 4096 * size_table_0[i];
		ba += hlina_chunks[i].size;
	}
	PDEBUG(KERN_DEBUG "MEMALLOC: START=0x%x, LENGTH=0x%x\n",
	       phy_start_address, ba - phy_start_address);
	if (ba - phy_start_address > MEMALLOC_SIZE) {
		PDEBUG(KERN_ERR "MEMALLOC ERROR: MEMORY ALLOC BUG\n");
		return -1;
	}

	return 0;
}

void memalloc_cleanup(void)
{

	deregister_memalloc_wrapper();

	PDEBUG(KERN_DEBUG "memalloc_cleanup\n");
	printk(KERN_NOTICE "memalloc: module removed\n");

	return;
}

module_init(memalloc_init);
module_exit(memalloc_cleanup);

/* Cycle through the buffers we have, give the first free one */
static int AllocMemory(MEMALLOC_HDL hdl, unsigned long *busaddr,
			unsigned int size)
{

	int i = 0;

	*busaddr = 0;

	for (i = 0; i < HLINA_NUMBER_OF_CHUNKS; i++) {

		if (!hlina_chunks[i].used && (hlina_chunks[i].size >= size)) {
			*busaddr = (unsigned long) hlina_chunks[i].bus_address;
			hlina_chunks[i].used = 1;
			hlina_chunks[i].file_id = hdl;
			break;
		}
	}

	if (*busaddr == 0) {
		printk(KERN_ERR "MEMALLOC FAILED: size = %d\n", size);
	} else {
		printk("MEMALLOC OK: size: %d, size reserved: %d, bus_address: 0x%08x\n", size, hlina_chunks[i].size, *busaddr);
	}

	return 0;
}

/* Free a buffer based on bus address */
static int FreeMemory(MEMALLOC_HDL hdl, unsigned long *busaddr)
{
	int i = 0;

	for (i = 0; i < HLINA_NUMBER_OF_CHUNKS; i++) {
		if (hlina_chunks[i].bus_address == *busaddr && hlina_chunks[i].file_id == hdl) {
			hlina_chunks[i].used = 0;
			hlina_chunks[i].file_id = NULL;
			break;
		}
	}

	if (i >= HLINA_NUMBER_OF_CHUNKS)
		printk(KERN_ERR "MEMFREE fail: addr: 0x%08x\n", (unsigned int)*busaddr);
	else
		printk("MEMFREE OK: size: 0x%08x, addr: 0x%08x\n", hlina_chunks[i].size, hlina_chunks[i].bus_address);

	return 0;
}

