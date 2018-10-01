#ifndef __BACKPORT_LINUX_GFP_H
#define __BACKPORT_LINUX_GFP_H
#include_next <linux/gfp.h>

#ifndef ___GFP_KSWAPD_RECLAIM
#define ___GFP_KSWAPD_RECLAIM	0x0u
#endif

#ifndef __GFP_KSWAPD_RECLAIM
#define __GFP_KSWAPD_RECLAIM	((__force gfp_t)___GFP_KSWAPD_RECLAIM) /* kswapd can wake */
#endif

#endif /* __BACKPORT_LINUX_GFP_H */
