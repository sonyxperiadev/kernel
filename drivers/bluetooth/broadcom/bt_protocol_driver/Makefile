# Makefile for Bluetooth protocol driver

obj-$(CONFIG_BT_PROTOCOL_DRIVER) := brcm_bt_drv.o

KBUILD_CFLAGS += -w 
EXTRA_CFLAGS := -I$(PWD)/../include/

#BT_S : [CONBT-2455] LGC_BT_COMMON_IMP_KO_MODULE_LOAD_FAIL_FIX ("Unknown symbol _GLOBAL_OFFSET_TABLE_" Error fix)
CFLAGS_MODULE = -fno-pic
#BT_E : [CONBT-2455] LGC_BT_COMMON_IMP_KO_MODULE_LOAD_FAIL_FIX ("Unknown symbol _GLOBAL_OFFSET_TABLE_" Error fix)

ccflags-y := -DVERSION="\"1.2\""
