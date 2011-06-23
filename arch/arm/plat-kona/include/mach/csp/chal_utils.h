/*****************************************************************************
* Copyright 2008 Broadcom Corporation.  All rights reserved.
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2, available at
* http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a
* license other than the GPL, without Broadcom's express prior written
* consent.
*****************************************************************************/

#ifndef CHAL_UTILS_H__
#define CHAL_UTILS_H__

#if defined( IGNORE_CHAL_UTILS )
#else

#include "chal_types.h"
#include "chal_std.h"

#ifdef __cplusplus
extern "C" {
#endif

#define isBetween(c, v1, v2) (((c) >= (v1)) && ((c) <= (v2)))
#define isHex(c,c1)     (((c) == '0') && (((c1) == 'x') || ((c1) == 'X')))
#define isDigit(c)      isBetween((c), '0','9')
#define isUpper(c)      isBetween((c), 'A','Z')
#define toLower(c)      (isUpper(c) ? ((c) - 'A' + 'a') : (c))
uint32_t strtonum(char* str);

#ifndef MAX
#define	MAX(a,b)	((a)>(b)?(a):(b))
#endif
#ifndef MIN
#define	MIN(a,b)	((a)<(b)?(a):(b))
#endif

#define MOD(a,b)    ((a)%(b))
#define DIV(a,b)    ((a)/(b))
    
#define	chal_pa(a)	(a)
#define	chal_va(a)	(a)

extern int get_cache_line_size (void);

#ifdef __KERNEL__
/* Avoid cache function naming collisions with Linux */
#else
#ifdef CONFIG_ENABLE_MMU
extern void dma_inv_range_L1(const void *, const void *);
extern void dma_clean_range_L1(const void *, const void *);
extern void dma_flush_range_L1(const void *, const void *);	/* clean & invalidate*/

#ifdef CONFIG_ENABLE_L2_CACHE
#include "chal_regmap.h"
static inline void dma_inv_range_L2 (const void *start, const void *end) 
{
	uint32_t p0;

	for (p0 = (uint32_t) start&(~31); p0 < (uint32_t) end; p0 += 32) {
		*((volatile uint32_t*) L2_CAHCE_INV_BY_PA) = p0;
	}

}
static inline void dma_clean_range_L2 (const void *start, const void *end)
{
	uint32_t p0;

	for (p0 = (uint32_t) start&(~31); p0 < (uint32_t) end; p0 += 32) {
		*((volatile uint32_t*) L2_CAHCE_CLEAN_BY_PA) = p0;
	}
}
static inline void dma_flush_range_L2 (const void *start, const void *end)
{
	uint32_t p0;

	for (p0 = (uint32_t) start&(~31); p0 < (uint32_t) end; p0 += 32) {
		*((volatile uint32_t*) L2_CAHCE_CLEAN_INV_BY_PA) = p0;
	}
}
#else
static inline void dma_inv_range_L2 (const void *start, const void *end) 
{
	CHAL_UNUSED (start);
	CHAL_UNUSED (end);
}
static inline void dma_clean_range_L2 (const void *start, const void *end)
{
	CHAL_UNUSED (start);
	CHAL_UNUSED (end);
}
static inline void dma_flush_range_L2 (const void *start, const void *end)
{
	CHAL_UNUSED (start);
	CHAL_UNUSED (end);
}
#endif

#if 0
#define	check_alignment(vir, phy) do {\
	if (((uint32_t)(vir) & 31) || ((uint32_t)(phy) &31)) {\
		BCM_DBG_ERR (("unaligned, %s %d: %x %x", __FUNCTION__, __LINE__, (vir), (phy)));\
		while (1); \
	}\
} while (0)
#else
#define	check_alignment(vir, phy) {}
#endif
		
static inline void dma_inv_range (void *vir, void *phy, uint32_t len) 
{
	check_alignment (vir, phy);
	dma_inv_range_L1(vir, vir + len);
	dma_inv_range_L2(phy, phy + len);
}

static inline void dma_clean_range (void *vir, void *phy, uint32_t len) 
{
	check_alignment (vir, phy);
	dma_clean_range_L1(vir, vir + len);
	dma_clean_range_L2(phy, phy + len);
}

static inline void dma_flush_range (void *vir, void *phy, uint32_t len) 
{
	check_alignment (vir, phy);
	dma_flush_range_L1(vir, vir + len);
	dma_flush_range_L2(phy, phy + len);
}

#else /*CONFIG_ENABLE_MMU*/
#define dma_inv_range(vir, phy, len) do{} while(0)
#define dma_clean_range(vir, phy, len) do {} while(0)
#define dma_flush_range(vir, phy, len) do {} while(0)
#endif /*CONFIG_ENABLE_MMU*/

/* for ARMv7 only*/
#define isb() __asm__ __volatile__ ("isb" : : : "memory")
#define dsb() __asm__ __volatile__ ("dsb" : : : "memory")
#define dmb() __asm__ __volatile__ ("dmb" : : : "memory")
#endif /* __KENREL__ */

#ifdef __cplusplus
}
#endif

#endif /* defined( IGNORE_CHAL_UTILS ) */

#endif /* CHAL_UTILS_H__ */

