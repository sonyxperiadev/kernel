# MSM8974
   zreladdr-$(CONFIG_ARCH_MSM8974)	:= 0x00008000

# APQ8084
   zreladdr-$(CONFIG_ARCH_APQ8084)	:= 0x00008000

# MDM9630
   zreladdr-$(CONFIG_ARCH_MDM9630)	:= 0x00008000

# MSMZIRC
   zreladdr-$(CONFIG_ARCH_MSMZIRC)	:= 0x80008000

# MSM8226
   zreladdr-$(CONFIG_ARCH_MSM8226)	:= 0x00008000
	dtb-$(CONFIG_MACH_SONY_EAGLE)	+= msm8926-yukon_eagle-720p-mtp.dtb
	dtb-$(CONFIG_MACH_SONY_FLAMINGO)+= msm8926-yukon_flamingo-8926ss_ap.dtb
        dtb-$(CONFIG_MACH_SONY_TIANCHI) += msm8226-v1-yukon_tianchi_dsds.dtb
        dtb-$(CONFIG_MACH_SONY_TIANCHI) += msm8226-v2-yukon_tianchi_dsds.dtb
	dtb-$(CONFIG_MACH_SONY_TIANCHI) += msm8926-yukon_tianchi.dtb
	dtb-$(CONFIG_MACH_SONY_SEAGULL) += msm8926-yukon_seagull-720p-mtp.dtb

# FSM9900
   zreladdr-$(CONFIG_ARCH_FSM9900)	:= 0x0b608000

# FSM9010
   zreladdr-$(CONFIG_ARCH_FSM9010)	:= 0x0b608000

# MSM8610
   zreladdr-$(CONFIG_ARCH_MSM8610)	:= 0x00008000

