ifeq ($(CONFIG_BUILD_ARM64_DT_OVERLAY),y)

ifeq ($(CONFIG_MACH_SONY_PDX223),y)
dtbo-$(CONFIG_ARCH_SONY_NAGARA) += waipio-nagara-pdx223_generic-overlay.dtbo

waipio-nagara-pdx223_generic-overlay.dtbo-base := waipio.dtb waipiop.dtb waipio-v2.dtb waipiop-v2.dtb waipio-lte.dtb
else
dtbo-$(CONFIG_ARCH_SONY_NAGARA) += waipio-nagara-pdx223_generic-crossflash-overlay.dtbo

waipio-nagara-pdx223_generic-crossflash-overlay.dtbo-base := waipio.dtb waipiop.dtb waipio-v2.dtb waipiop-v2.dtb waipio-lte.dtb
endif
else

ifeq ($(CONFIG_BUILD_ARM64_DT_OVERLAY),y)
dtbo-$(CONFIG_ARCH_LAHAINA) += lahaina-rumi-overlay.dtbo \
	lahaina-mtp-overlay.dtbo \
	lahaina-mtp-hsp-overlay.dtbo \
	lahaina-cdp-overlay.dtbo \
	lahaina-qrd-overlay.dtbo \
	lahaina-atp-overlay.dtbo \
	lahaina-hdk-overlay.dtbo \
	lahaina-qrd-hsp-overlay.dtbo \
	lahaina-qrd-module-overlay.dtbo \
	lahainap-mtp-overlay.dtbo \
	lahainap-cdp-overlay.dtbo \
	lahainap-atp-overlay.dtbo \
	lahainap-qrd-overlay.dtbo \
	lahaina-cdp-v2.2-overlay.dtbo \
	lahaina-mtp-v2-overlay.dtbo \
	lahaina-mtp-v2.1-overlay.dtbo

lahaina-rumi-overlay.dtbo-base := lahaina.dtb
lahaina-mtp-hsp-overlay.dtbo-base := lahaina.dtb
lahaina-mtp-overlay.dtbo-base := lahaina.dtb lahaina-v2.dtb lahaina-v2.1.dtb
lahaina-cdp-overlay.dtbo-base := lahaina.dtb lahaina-v2.dtb lahaina-v2.1.dtb
lahaina-atp-overlay.dtbo-base := lahaina.dtb lahaina-v2.dtb lahaina-v2.1.dtb
lahaina-hdk-overlay.dtbo-base := lahaina.dtb lahaina-v2.dtb lahaina-v2.1.dtb
lahaina-qrd-hsp-overlay.dtbo-base := lahaina.dtb
lahaina-qrd-overlay.dtbo-base := lahaina.dtb lahaina-v2.dtb lahaina-v2.1.dtb
lahaina-qrd-module-overlay.dtbo-base := lahaina.dtb lahaina-v2.dtb lahaina-v2.1.dtb
lahainap-mtp-overlay.dtbo-base := lahainap.dtb lahainap-v2.dtb lahainap-v2.1.dtb
lahainap-cdp-overlay.dtbo-base := lahainap.dtb lahainap-v2.dtb lahainap-v2.1.dtb
lahainap-atp-overlay.dtbo-base := lahainap.dtb lahainap-v2.dtb lahainap-v2.1.dtb
lahainap-qrd-overlay.dtbo-base := lahainap.dtb lahainap-v2.dtb lahainap-v2.1.dtb
lahaina-cdp-v2.2-overlay.dtbo-base := lahaina.dtb lahaina-v2.dtb \
					lahaina-v2.1.dtb
lahaina-mtp-v2-overlay.dtbo-base := lahaina.dtb lahaina-v2.dtb \
					lahaina-v2.1.dtb
lahaina-mtp-v2.1-overlay.dtbo-base := lahaina.dtb lahaina-v2.dtb \
					lahaina-v2.1.dtb
else
dtb-$(CONFIG_ARCH_LAHAINA) += lahaina-rumi.dtb \
	lahaina-mtp-hsp.dtb \
	lahaina-mtp.dtb \
	lahaina-cdp.dtb \
	lahaina-atp.dtb \
	lahaina-hdk.dtb \
	lahaina-qrd-hsp.dtb \
	lahaina-qrd.dtb \
	lahaina-qrd-module.dtb \
	lahainap-mtp.dtb \
	lahainap-cdp.dtb \
	lahainap-atp.dtb \
	lahainap-qrd.dtb \
	lahaina-v2-mtp.dtb \
	lahaina-v2-cdp.dtb \
	lahaina-v2-atp.dtb \
	lahaina-v2-hdk.dtb \
	lahaina-v2-qrd.dtb \
	lahaina-v2-qrd-module.dtb \
	lahainap-v2-mtp.dtb \
	lahainap-v2-cdp.dtb \
	lahainap-v2-atp.dtb \
	lahainap-v2-qrd.dtb \
	lahaina-cdp-v2.2.dtb \
	lahaina-v2.1-mtp.dtb \
	lahaina-v2.1-cdp.dtb \
	lahaina-v2.1-atp.dtb \
	lahaina-v2.1-hdk.dtb \
	lahaina-v2.1-qrd.dtb \
	lahaina-v2.1-qrd-module.dtb \
	lahainap-v2.1-mtp.dtb \
	lahainap-v2.1-cdp.dtb \
	lahainap-v2.1-atp.dtb \
	lahainap-v2.1-qrd.dtb \
	lahaina-mtp-v2.dtb \
	lahaina-mtp-v2.1.dtb
endif

ifneq ($(SOMC_PLATFORM),nagara)
ifeq ($(CONFIG_BUILD_ARM64_DT_OVERLAY),y)
dtbo-$(CONFIG_ARCH_WAIPIO) += waipio-rumi-overlay.dtbo \
	waipio-mtp-pm8008-overlay.dtbo \
	waipio-cdp-pm8008-overlay.dtbo \
	waipio-qrd-pm8008-overlay.dtbo \
	waipio-atp-pm8008-overlay.dtbo \
	waipio-mtp-pm8010-overlay.dtbo \
	waipio-cdp-pm8010-overlay.dtbo \
	waipio-qrd-pm8010-overlay.dtbo \
	waipio-qrd-pm8010-2s-overlay.dtbo \
	waipio-atp-pm8010-overlay.dtbo \
	waipiop-hdk-pm8010-overlay.dtbo \
	waipio-lemur-mtp-pm8008-overlay.dtbo \
	waipio-lemur-mtp-pm8010-overlay.dtbo \
	waipio-lemur-cdp-pm8008-overlay.dtbo \
	waipio-lemur-cdp-pm8010-overlay.dtbo \
	waipio-kiwi-mtp-pm8008-overlay.dtbo \
	waipio-kiwi-mtp-pm8010-overlay.dtbo \
	waipio-kiwi-cdp-pm8008-overlay.dtbo \
	waipio-kiwi-cdp-pm8010-overlay.dtbo

waipio-rumi-overlay.dtbo-base := waipio.dtb waipiop.dtb waipio-v2.dtb waipiop-v2.dtb
waipio-mtp-pm8008-overlay.dtbo-base := waipio.dtb waipiop.dtb waipio-v2.dtb waipiop-v2.dtb waipio-lte.dtb
waipio-cdp-pm8008-overlay.dtbo-base := waipio.dtb waipiop.dtb waipio-v2.dtb waipiop-v2.dtb waipio-lte.dtb
waipio-qrd-pm8008-overlay.dtbo-base := waipio.dtb waipiop.dtb waipio-v2.dtb waipiop-v2.dtb waipio-lte.dtb
waipio-atp-pm8008-overlay.dtbo-base := waipio.dtb waipiop.dtb waipio-v2.dtb waipiop-v2.dtb waipio-lte.dtb
waipio-mtp-pm8010-overlay.dtbo-base := waipio.dtb waipiop.dtb waipio-v2.dtb waipiop-v2.dtb waipio-lte.dtb
waipio-cdp-pm8010-overlay.dtbo-base := waipio.dtb waipiop.dtb waipio-v2.dtb waipiop-v2.dtb waipio-lte.dtb
waipio-qrd-pm8010-overlay.dtbo-base := waipio.dtb waipiop.dtb waipio-v2.dtb waipiop-v2.dtb waipio-lte.dtb
waipio-qrd-pm8010-2s-overlay.dtbo-base := waipio.dtb waipiop.dtb waipio-v2.dtb waipiop-v2.dtb
waipio-atp-pm8010-overlay.dtbo-base := waipio.dtb waipiop.dtb waipio-v2.dtb waipiop-v2.dtb waipio-lte.dtb
waipiop-hdk-pm8010-overlay.dtbo-base := waipiop.dtb waipiop-v2.dtb
waipio-lemur-mtp-pm8008-overlay.dtbo-base := waipio.dtb waipiop.dtb waipio-v2.dtb waipiop-v2.dtb waipio-lte.dtb
waipio-lemur-mtp-pm8010-overlay.dtbo-base := waipio.dtb waipiop.dtb waipio-v2.dtb waipiop-v2.dtb waipio-lte.dtb
waipio-lemur-cdp-pm8008-overlay.dtbo-base := waipio.dtb waipiop.dtb waipio-v2.dtb waipiop-v2.dtb waipio-lte.dtb
waipio-lemur-cdp-pm8010-overlay.dtbo-base := waipio.dtb waipiop.dtb waipio-v2.dtb waipiop-v2.dtb waipio-lte.dtb
waipio-kiwi-mtp-pm8008-overlay.dtbo-base := waipio.dtb waipio-v2.dtb waipio-lte.dtb
waipio-kiwi-mtp-pm8010-overlay.dtbo-base := waipio.dtb waipio-v2.dtb waipio-lte.dtb
waipio-kiwi-cdp-pm8008-overlay.dtbo-base := waipio.dtb waipio-v2.dtb waipio-lte.dtb
waipio-kiwi-cdp-pm8010-overlay.dtbo-base := waipio.dtb waipio-v2.dtb waipio-lte.dtb
else
dtb-$(CONFIG_ARCH_WAIPIO) += waipio-rumi.dtb \
	waipio-mtp-pm8008.dtb \
	waipio-cdp-pm8008.dtb \
	waipio-qrd-pm8008.dtb \
	waipio-atp-pm8008.dtb \
	waipio-mtp-pm8010.dtb \
	waipio-cdp-pm8010.dtb \
	waipio-qrd-pm8010.dtb \
	waipio-qrd-pm8010-2s.dtb \
	waipio-atp-pm8010.dtb \
	waipiop-mtp-pm8008.dtb \
	waipiop-cdp-pm8008.dtb \
	waipiop-qrd-pm8008.dtb \
	waipiop-mtp-pm8010.dtb \
	waipiop-cdp-pm8010.dtb \
	waipiop-qrd-pm8010.dtb \
	waipiop-hdk-pm8010.dtb \
	waipiop-qrd-pm8010-2s.dtb
# Waipio V2
dtb-$(CONFIG_ARCH_WAIPIO) += waipio-v2-mtp-pm8008.dtb \
	waipio-v2-cdp-pm8008.dtb \
	waipio-v2-qrd-pm8008.dtb \
	waipio-v2-atp-pm8008.dtb \
	waipio-v2-mtp-pm8010.dtb \
	waipio-v2-cdp-pm8010.dtb \
	waipio-v2-qrd-pm8010.dtb \
	waipio-v2-qrd-pm8010-2s.dtb \
	waipio-v2-atp-pm8010.dtb \
	waipiop-v2-mtp-pm8008.dtb \
	waipiop-v2-cdp-pm8008.dtb \
	waipiop-v2-qrd-pm8008.dtb \
	waipiop-v2-mtp-pm8010.dtb \
	waipiop-v2-cdp-pm8010.dtb \
	waipiop-v2-qrd-pm8010.dtb \
	waipiop-v2-hdk-pm8010.dtb \
	waipiop-v2-qrd-pm8010-2s.dtb

# Waipio LTE
dtb-$(CONFIG_ARCH_WAIPIO) += waipio-lte-mtp-pm8008.dtb \
	waipio-lte-cdp-pm8008.dtb \
	waipio-lte-qrd-pm8008.dtb \
	waipio-lte-atp-pm8008.dtb \
	waipio-lte-mtp-pm8010.dtb \
	waipio-lte-cdp-pm8010.dtb \
	waipio-lte-qrd-pm8010.dtb \
	waipio-lte-atp-pm8010.dtb
endif
endif
endif

ifeq ($(CONFIG_ARCH_LAHAINA), y)
ifeq ($(CONFIG_ARCH_QTI_VM), y)
dtb-$(CONFIG_ARCH_QTI_VM) += lahaina-vm-mtp.dtb \
	lahaina-vm-cdp.dtb \
	lahaina-vm-qrd.dtb
endif
endif

ifeq ($(CONFIG_ARCH_WAIPIO), y)
ifeq ($(CONFIG_ARCH_QTI_VM), y)
dtb-$(CONFIG_ARCH_QTI_VM) += waipio-vm-mtp.dtb \
	waipio-vm-cdp.dtb \
	waipio-vm-qrd.dtb \
        waipio-vm-rumi.dtb \
	waipio-lte-vm-mtp.dtb \
	waipio-lte-vm-cdp.dtb \
	waipio-lte-vm-qrd.dtb
endif
endif

always-y	:= $(dtb-y)
subdir-y	:= $(dts-dirs)
clean-files	:= *.dtb *.dtbo
