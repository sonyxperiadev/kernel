#ifndef __PLAT_NAND_H_
#define __PLAT_NAND_H_

#include <linux/mtd/partitions.h>

struct bcm2708_platform_nand {
   struct mtd_partition *partitions;
   int nr_partitions;
};

#endif /* __PLAT_NAND_H_ */
