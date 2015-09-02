obj-$(CONFIG_CYPRESS_CYTTSP4_BUS)		+= cyttsp4_bus.o
obj-$(CONFIG_TOUCHSCREEN_CYPRESS_CYTTSP4)	+= cyttsp4_core.o
obj-$(CONFIG_TOUCHSCREEN_CYPRESS_CYTTSP4_I2C)	+= cyttsp4_i2c.o
obj-$(CONFIG_TOUCHSCREEN_CYPRESS_CYTTSP4_MT_B)  += cyttsp4_mt_b.o
# obj-$(CONFIG_TOUCHSCREEN_CYPRESS_CYTTSP4_BUTTON)	+= cyttsp4_btn.o
# obj-$(CONFIG_TOUCHSCREEN_CYPRESS_CYTTSP4_PROXIMITY)	+= cyttsp4_proximity.o
obj-$(CONFIG_TOUCHSCREEN_CYPRESS_CYTTSP4_DEVICE_ACCESS)	+= cyttsp4_device_access.o
obj-$(CONFIG_TOUCHSCREEN_CYPRESS_CYTTSP4_TEST_DEVICE_ACCESS_API)	+= cyttsp4_test_device_access_api.o
obj-$(CONFIG_TOUCHSCREEN_CYPRESS_CYTTSP4_LOADER)	+= cyttsp4_loader.o
obj-$(CONFIG_TOUCHSCREEN_CYPRESS_CYTTSP4_DEBUG_MODULE)	+= cyttsp4_debug.o
obj-$(CONFIG_TOUCHSCREEN_CYPRESS_CYTTSP4_DEVICETREE_SUPPORT)	+= cyttsp4_devtree.o

ifeq ($(CONFIG_TOUCHSCREEN_CYPRESS_CYTTSP4_DEBUG),y)
CFLAGS_cyttsp4_bus.o += -DDEBUG
CFLAGS_cyttsp4_core.o += -DDEBUG
CFLAGS_cyttsp4_i2c.o += -DDEBUG
CFLAGS_cyttsp4_mtb.o += -DDEBUG
CFLAGS_cyttsp4_mt_b.o += -DDEBUG
CFLAGS_cyttsp4_mt_common.o += -DDEBUG
# CFLAGS_cyttsp4_btn.o += -DDEBUG
# CFLAGS_cyttsp4_proximity.o += -DDEBUG
CFLAGS_cyttsp4_device_access.o += -DDEBUG
CFLAGS_cyttsp4_loader.o += -DDEBUG
CFLAGS_cyttsp4_debug.o += -DDEBUG
CFLAGS_cyttsp4_devtree.o += -DDEBUG
CFLAGS_cyttsp4_platform.o += -DDEBUG
endif

ifeq ($(CONFIG_TOUCHSCREEN_CYPRESS_CYTTSP4_VDEBUG),y)
CFLAGS_cyttsp4_bus.o += -DVERBOSE_DEBUG
CFLAGS_cyttsp4_core.o += -DVERBOSE_DEBUG
CFLAGS_cyttsp4_i2c.o += -DVERBOSE_DEBUG
CFLAGS_cyttsp4_mtb.o += -DVERBOSE_DEBUG
CFLAGS_cyttsp4_mt_b.o += -DVERBOSE_DEBUG
CFLAGS_cyttsp4_mt_common.o += -DVERBOSE_DEBUG
# CFLAGS_cyttsp4_btn.o += -DVERBOSE_DEBUG
# CFLAGS_cyttsp4_proximity.o += -DVERBOSE_DEBUG
CFLAGS_cyttsp4_device_access.o += -DVERBOSE_DEBUG
CFLAGS_cyttsp4_loader.o += -DVERBOSE_DEBUG
CFLAGS_cyttsp4_debug.o += -DVERBOSE_DEBUG
CFLAGS_cyttsp4_devtree.o += -DVERBOSE_DEBUG
CFLAGS_cyttsp4_platform.o += -DVERBOSE_DEBUG
endif

cyttsp4_mt_b-y := cyttsp4_mtb.o cyttsp4_mt_common.o

ifdef CONFIG_TOUCHSCREEN_CYPRESS_CYTTSP4
obj-y += cyttsp4_platform.o
endif
