/*****************************************************************************
*  Copyright 2006 - 2007 Broadcom Corporation.  All rights reserved.
*
*  Unless you and Broadcom execute a separate written software license
*  agreement governing use of this software, this software is licensed to you
*  under the terms of the GNU General Public License version 2, available at
*  http://www.gnu.org/copyleft/gpl.html (the "GPL").
*
*  Notwithstanding the above, under no circumstances may you combine this
*  software in any way with any other Broadcom software provided under a
*  license other than the GPL, without Broadcom's express prior written
*  consent.
*
*****************************************************************************/

#ifndef KEYMAP_9MC2003SD_H
#define KEYMAP_9MC2003SD_H

/*  Keymap for BCM9MC2003SD platform */
/*  0 - 63 = functions */

/* Use BCM_NO_COLUMN as gpio_col is the key is direct gpio key */
#define BCM_NO_COLUMN   -1

/* The following defines BCM_GPIO_KEY_ROW and BCM_GPIO_KEY_COL
   specifies the actual gpio pin to which the keys are connected */
#define BCM_GPIO_KEY_ROW_0  0
#define BCM_GPIO_KEY_ROW_1  1
#define BCM_GPIO_KEY_ROW_2  2
#define BCM_GPIO_KEY_ROW_3  3
#define BCM_GPIO_KEY_ROW_4  4
#define BCM_GPIO_KEY_ROW_5  5
#define BCM_GPIO_KEY_ROW_6  6
#define BCM_GPIO_KEY_ROW_7  7

#define BCM_GPIO_KEY_COL_0  8
#define BCM_GPIO_KEY_COL_1  9
#define BCM_GPIO_KEY_COL_2  10
#define BCM_GPIO_KEY_COL_3  11
#define BCM_GPIO_KEY_COL_4  12
#define BCM_GPIO_KEY_COL_5  13
#define BCM_GPIO_KEY_COL_6  14
#define BCM_GPIO_KEY_COL_7  15

/* The following define is casted to BCM_GPIO_KEYMAP specified in /drivers/keyboard/bcm_gpio_keypad.c */
#define HW_DEFAULT_KEYMAP \
{ \
   { BCM_GPIO_KEY_ROW_0,    BCM_GPIO_KEY_COL_0,  "CURSOR DOWN",      80 }, \
   { BCM_GPIO_KEY_ROW_0,    BCM_GPIO_KEY_COL_1,  "F1 (prev)",        59 }, \
   { BCM_GPIO_KEY_ROW_0,    BCM_GPIO_KEY_COL_2,  "CURSOR UP",        72 }, \
   { BCM_GPIO_KEY_ROW_0,    BCM_GPIO_KEY_COL_3,  "F2 (next)",        60 }, \
   { BCM_GPIO_KEY_ROW_0,    BCM_GPIO_KEY_COL_4,  "HOME (menu)",      71 }, \
   { BCM_GPIO_KEY_ROW_0,    BCM_GPIO_KEY_COL_5,  "unused",           0  }, \
   { BCM_GPIO_KEY_ROW_0,    BCM_GPIO_KEY_COL_6,  "unused",           0  }, \
   { BCM_GPIO_KEY_ROW_0,    BCM_GPIO_KEY_COL_7,  "unused",           0  }, \
   { BCM_GPIO_KEY_ROW_1,    BCM_GPIO_KEY_COL_0,  "CURSOR LEFT",      75 }, \
   { BCM_GPIO_KEY_ROW_1,    BCM_GPIO_KEY_COL_1,  "1 button",          2 }, \
   { BCM_GPIO_KEY_ROW_1,    BCM_GPIO_KEY_COL_2,  "2 button",          3 }, \
   { BCM_GPIO_KEY_ROW_1,    BCM_GPIO_KEY_COL_3,  "3 button",          4 }, \
   { BCM_GPIO_KEY_ROW_1,    BCM_GPIO_KEY_COL_4,  "unused",           12 }, \
   { BCM_GPIO_KEY_ROW_1,    BCM_GPIO_KEY_COL_5,  "unused",           13 }, \
   { BCM_GPIO_KEY_ROW_1,    BCM_GPIO_KEY_COL_6,  "unused",           14 }, \
   { BCM_GPIO_KEY_ROW_1,    BCM_GPIO_KEY_COL_7,  "unused",           15 }, \
   { BCM_GPIO_KEY_ROW_2,    BCM_GPIO_KEY_COL_0,  "CURSOR RIGHT",     77 }, \
   { BCM_GPIO_KEY_ROW_2,    BCM_GPIO_KEY_COL_1,  "4 button",          5 }, \
   { BCM_GPIO_KEY_ROW_2,    BCM_GPIO_KEY_COL_2,  "5 button",          6 }, \
   { BCM_GPIO_KEY_ROW_2,    BCM_GPIO_KEY_COL_3,  "6 button",          7 }, \
   { BCM_GPIO_KEY_ROW_2,    BCM_GPIO_KEY_COL_4,  "unused",           20 }, \
   { BCM_GPIO_KEY_ROW_2,    BCM_GPIO_KEY_COL_5,  "unused",           21 }, \
   { BCM_GPIO_KEY_ROW_2,    BCM_GPIO_KEY_COL_6,  "unused",           22 }, \
   { BCM_GPIO_KEY_ROW_2,    BCM_GPIO_KEY_COL_7,  "unused",           23 }, \
   { BCM_GPIO_KEY_ROW_3,    BCM_GPIO_KEY_COL_0,  "ESCAPE",           1 }, \
   { BCM_GPIO_KEY_ROW_3,    BCM_GPIO_KEY_COL_1,  "7 button",          8 }, \
   { BCM_GPIO_KEY_ROW_3,    BCM_GPIO_KEY_COL_2,  "8 button",          9 }, \
   { BCM_GPIO_KEY_ROW_3,    BCM_GPIO_KEY_COL_3,  "9 button",         10 }, \
   { BCM_GPIO_KEY_ROW_3,    BCM_GPIO_KEY_COL_4,  "unused",           28 }, \
   { BCM_GPIO_KEY_ROW_3,    BCM_GPIO_KEY_COL_5,  "unused",           29 }, \
   { BCM_GPIO_KEY_ROW_3,    BCM_GPIO_KEY_COL_6,  "unused",           30 }, \
   { BCM_GPIO_KEY_ROW_3,    BCM_GPIO_KEY_COL_7,  "unused",           31 }, \
   { BCM_GPIO_KEY_ROW_4,    BCM_GPIO_KEY_COL_0,  "RETURN (OK)",      28 }, \
   { BCM_GPIO_KEY_ROW_4,    BCM_GPIO_KEY_COL_1,  "ASTERISK",         55 }, \
   { BCM_GPIO_KEY_ROW_4,    BCM_GPIO_KEY_COL_2,  "0 button",         11 }, \
   { BCM_GPIO_KEY_ROW_4,    BCM_GPIO_KEY_COL_3,  "SPACE",            57 }, \
   { BCM_GPIO_KEY_ROW_4,    BCM_GPIO_KEY_COL_4,  "F3 (play|pause)",  61 }, \
   { BCM_GPIO_KEY_ROW_4,    BCM_GPIO_KEY_COL_5,  "unused",           37 }, \
   { BCM_GPIO_KEY_ROW_4,    BCM_GPIO_KEY_COL_6,  "unused",           38 }, \
   { BCM_GPIO_KEY_ROW_4,    BCM_GPIO_KEY_COL_7,  "unused",           39 }, \
   { BCM_GPIO_KEY_ROW_5,    BCM_GPIO_KEY_COL_0,  "unused",           40 }, \
   { BCM_GPIO_KEY_ROW_5,    BCM_GPIO_KEY_COL_1,  "unused",           41 }, \
   { BCM_GPIO_KEY_ROW_5,    BCM_GPIO_KEY_COL_2,  "unused",           42 }, \
   { BCM_GPIO_KEY_ROW_5,    BCM_GPIO_KEY_COL_3,  "unused",           43 }, \
   { BCM_GPIO_KEY_ROW_5,    BCM_GPIO_KEY_COL_4,  "unused",           44 }, \
   { BCM_GPIO_KEY_ROW_5,    BCM_GPIO_KEY_COL_5,  "unused",           45 }, \
   { BCM_GPIO_KEY_ROW_5,    BCM_GPIO_KEY_COL_6,  "unused",           46 }, \
   { BCM_GPIO_KEY_ROW_5,    BCM_GPIO_KEY_COL_7,  "unused",           47 }, \
   { BCM_GPIO_KEY_ROW_6,    BCM_GPIO_KEY_COL_0,  "unused",           48 }, \
   { BCM_GPIO_KEY_ROW_6,    BCM_GPIO_KEY_COL_1,  "unused",           49 }, \
   { BCM_GPIO_KEY_ROW_6,    BCM_GPIO_KEY_COL_2,  "unused",           50 }, \
   { BCM_GPIO_KEY_ROW_6,    BCM_GPIO_KEY_COL_3,  "unused",           51 }, \
   { BCM_GPIO_KEY_ROW_6,    BCM_GPIO_KEY_COL_4,  "unused",           52 }, \
   { BCM_GPIO_KEY_ROW_6,    BCM_GPIO_KEY_COL_5,  "unused",           53 }, \
   { BCM_GPIO_KEY_ROW_6,    BCM_GPIO_KEY_COL_6,  "unused",           54 }, \
   { BCM_GPIO_KEY_ROW_6,    BCM_GPIO_KEY_COL_7,  "unused",           55 }, \
   { BCM_GPIO_KEY_ROW_7,    BCM_GPIO_KEY_COL_0,  "unused",           56 }, \
   { BCM_GPIO_KEY_ROW_7,    BCM_GPIO_KEY_COL_1,  "unused",           57 }, \
   { BCM_GPIO_KEY_ROW_7,    BCM_GPIO_KEY_COL_2,  "unused",           58 }, \
   { BCM_GPIO_KEY_ROW_7,    BCM_GPIO_KEY_COL_3,  "unused",           59 }, \
   { BCM_GPIO_KEY_ROW_7,    BCM_GPIO_KEY_COL_4,  "unused",           60 }, \
   { BCM_GPIO_KEY_ROW_7,    BCM_GPIO_KEY_COL_5,  "unused",           61 }, \
   { BCM_GPIO_KEY_ROW_7,    BCM_GPIO_KEY_COL_6,  "unused",           62 }, \
   { BCM_GPIO_KEY_ROW_7,    BCM_GPIO_KEY_COL_7,  "unused",           63 }, \
}

#endif
