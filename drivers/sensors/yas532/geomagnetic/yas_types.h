#ifndef __YAS_TYPES_H__
#define __YAS_TYPES_H__

/* macro */
#ifndef	NULL
#define	NULL			((void *)0)
#endif

#if defined(__KERNEL__)
#include <linux/types.h>
#else
#include <stdint.h>
/*typedef signed char	int8_t;*/
/*typedef unsigned char	uint8_t;*/
/*typedef signed short	int16_t;*/
/*typedef unsigned short	uint16_t;*/
/*typedef signed int	int32_t;*/
/*typedef unsigned int	uint32_t;*/
#endif

#endif	/* __YASTYPES_H__ */

/* end of file */
