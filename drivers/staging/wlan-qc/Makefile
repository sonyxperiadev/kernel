obj-$(CONFIG_QCA_CLD_WLAN)	+= qcacld-3.0/

ccflags_no-error_array_parameter := $(call cc-option,-Wno-error=array-parameter)
ccflags_no-error_maybe_uninitialized := $(call cc-option,-Wno-error=maybe-uninitialized)
ccflags_no-error_misleading_indentation := $(call cc-option,-Wno-error=misleading-indentation)

subdir-ccflags-y := $(ccflags_no-error_array_parameter) $(ccflags_no-error_maybe_uninitialized) $(ccflags_no-error_misleading_indentation)
