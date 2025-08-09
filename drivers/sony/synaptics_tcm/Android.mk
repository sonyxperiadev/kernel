TCM_LOCAL_PATH := $(call my-dir)

include $(TCM_LOCAL_PATH)/synaptics_tcm_i2c/Android.mk
include $(TCM_LOCAL_PATH)/synaptics_tcm_core/Android.mk
include $(TCM_LOCAL_PATH)/synaptics_tcm_touch/Android.mk
include $(TCM_LOCAL_PATH)/synaptics_tcm_device/Android.mk
include $(TCM_LOCAL_PATH)/synaptics_tcm_testing/Android.mk
include $(TCM_LOCAL_PATH)/synaptics_tcm_reflash/Android.mk
include $(TCM_LOCAL_PATH)/synaptics_tcm_recovery/Android.mk
include $(TCM_LOCAL_PATH)/synaptics_tcm_diagnostics/Android.mk
