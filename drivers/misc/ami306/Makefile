
# Kernel makefile for ami306
#
#
obj-$(CONFIG_SENSORS_AMI306)	:= ami306.o

ami306-y := 	ami306_km_brcm.o	\
		ami306_cmd.o \
		ami306_self_test.o

#EXTRA_CFLAGS += -DDEBUG_LOG
#EXTRA_CFLAGS += -DUSE_DRDY_PIN
#EXTRA_CFLAGS += -DDELAY_MILLI_SEC
EXTRA_CFLAGS += -DUSER_MEMORY
#EXTRA_CFLAGS += -DAMI306L
EXTRA_CFLAGS += -Idrivers/misc/ami306/linux
