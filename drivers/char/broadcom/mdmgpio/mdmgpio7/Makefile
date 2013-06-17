#Default compile as module
CONFIG_BCM_MDMGPIO	?= y

#Build flags
EXTRA_CFLAGS  += -Wall -Wstrict-prototypes -Wno-trigraphs -O2

EXTRA_CFLAGS  += -D__KERNEL__
EXTRA_CFLAGS  += -D__linux__


obj-$(CONFIG_BCM_MDMGPIO) += mdmgpio7.o 

