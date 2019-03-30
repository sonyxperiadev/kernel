/*
 * Copyright(c) 2017 Hauke Mehrtens <hauke@hauke-m.de>
 *
 * Backport functionality introduced in Linux 4.10.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/ethtool.h>
#include <linux/mii.h>

#if LINUX_VERSION_IS_GEQ(4,6,0)
#if LINUX_VERSION_IS_LESS(4,7,0)
static bool ethtool_convert_link_mode_to_legacy_u32(u32 *legacy_u32,
						    const unsigned long *src)
{
	bool retval = true;

	/* TODO: following test will soon always be true */
	if (__ETHTOOL_LINK_MODE_MASK_NBITS > 32) {
		__ETHTOOL_DECLARE_LINK_MODE_MASK(ext);

		bitmap_zero(ext, __ETHTOOL_LINK_MODE_MASK_NBITS);
		bitmap_fill(ext, 32);
		bitmap_complement(ext, ext, __ETHTOOL_LINK_MODE_MASK_NBITS);
		if (bitmap_intersects(ext, src,
				      __ETHTOOL_LINK_MODE_MASK_NBITS)) {
			/* src mask goes beyond bit 31 */
			retval = false;
		}
	}
	*legacy_u32 = src[0];
	return retval;
}

static void ethtool_convert_legacy_u32_to_link_mode(unsigned long *dst,
						    u32 legacy_u32)
{
	bitmap_zero(dst, __ETHTOOL_LINK_MODE_MASK_NBITS);
	dst[0] = legacy_u32;
}
#endif

#endif /* LINUX_VERSION_IS_GEQ(4,6,0) */
