#ifndef _ISLAND_KEYPAD_H
#define _ISLAND_KEYPAD_H

#if defined(CONFIG_KEYBOARD_KONA)

/*
 * Keypad mapping
 */
struct KEYMAP
{
   /*
    * Scancode contains two hex numbers. The 1st hex denotes the row and
    * the 2nd hex denotes the column
    *
    * Syntax: 0x[Row][Column]
    */
   unsigned int scancode;
   unsigned int keycode;
};

/*
 * The keypad data structure. This is hardware dependent and is registered
 * as platform data into the platform device. It is passed into the driver
 * through the platform interface when the driver is probed
 */
struct KEYPAD_DATA
{
    unsigned int active_mode;

    struct KEYMAP *keymap;
    unsigned int keymap_cnt;

    /* key sets for powering off (reboot) Linux */
    unsigned int *pwroff;
    unsigned int pwroff_cnt;

    const char *clock;
};

#endif

#endif
