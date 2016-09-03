# Makefile for Line discipline driver

brcm_hci_ldisc-objs := brcm_hci.o brcm_sh_ldisc.o brcm_bluesleep.o
obj-$(CONFIG_LINE_DISCIPLINE_DRIVER) := brcm_hci_ldisc.o

KBUILD_CFLAGS += -w
EXTRA_CFLAGS := -I$(PWD)/../include/

ccflags-y += -w -DVERSION="\"1.2\""
#ccflags-y += -DLPM_BLUESLEEP
