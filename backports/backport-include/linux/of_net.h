#ifndef _BP_OF_NET_H
#define _BP_OF_NET_H
#include_next <linux/of_net.h>
#include <linux/version.h>

#ifndef CONFIG_OF
#if LINUX_VERSION_IS_LESS(3,10,0)
static inline const void *of_get_mac_address(struct device_node *np)
{
	return NULL;
}
#endif
#endif

#endif /* _BP_OF_NET_H */
