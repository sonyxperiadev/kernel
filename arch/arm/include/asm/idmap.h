#ifndef _ARM_IDMAP_H
#define _ARM_IDMAP_H

#include <asm/page.h>

void identity_mapping_add(pgd_t *pgd, unsigned long addr, unsigned long end);
void identity_mapping_del(pgd_t *pgd, unsigned long addr, unsigned long end);

void setup_mm_for_reboot(char mode);

#endif	/* _ARM_IDMAP_H */
