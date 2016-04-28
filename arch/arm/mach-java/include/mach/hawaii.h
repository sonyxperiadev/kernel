#ifndef __MACH_HAWAII_H
#define __MACH_HAWAII_H

extern enum reboot_mode reboot_mode;

void __init hawaii_map_io(void);
void hawaii_restart(enum reboot_mode mode, const char *cmd);

#endif /* __MACH_HAWAII_H */
