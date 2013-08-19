#include <linux/kobject.h>
#include <linux/kernel.h>
#include <linux/sysfs.h>
#include <linux/string.h>
#include <asm/setup.h>
#include <linux/moduleparam.h>
#include "board-sony_fusion3.h"

struct hw_rev_map {
	const char *pba_id;
	const char *pba_rev;
	int hw_rev;
	enum sony_hw hw;
	const char *hw_rev_str;
};

static const struct hw_rev_map hw_revisions[] = {
	{ "1261-6683", "1B", HW_REV_ODIN_DP1, HW_ODIN, "odin_dp1-a" },
	{ "1261-6683", "1C", HW_REV_ODIN_SP1, HW_ODIN, "odin_sp1-a" },
	{ "1261-6683", "1D", HW_REV_ODIN_SP11, HW_ODIN, "odin_sp1.1-a" },
	{ "1261-6683", "1E", HW_REV_ODIN_AP1, HW_ODIN, "odin_ap1-a" },
	{ "1265-7302", "1A", HW_REV_ODIN_SP1_PB, HW_ODIN, "odin_sp1-powerboard" },
	{ "1264-3414", "1A", HW_REV_ODIN_SP1, HW_ODIN, "odin_sp1-g" },
	{ "1264-3414", "1B", HW_REV_ODIN_SP11, HW_ODIN, "odin_sp1.1-g" },
	{ "1264-3414", "1C", HW_REV_ODIN_SP12, HW_ODIN, "odin_sp1.2-g" },
	{ "1264-3414", "1D", HW_REV_ODIN_AP1, HW_ODIN, "odin_ap1.0-g" },
	{ "1264-3414", "1E", HW_REV_ODIN_AP101, HW_ODIN, "odin_ap1.01-g" },
	{ "1264-3414", "1F", HW_REV_ODIN_AP105, HW_ODIN, "odin_ap1.05.g" },
	{ "1268-0967", "1A", HW_REV_ODIN_AP1, HW_ODIN, "odin_ap1-r" },
	{ "1263-6460", "1A", HW_REV_YUGA_MAKI_DP0, HW_YUGA_MAKI, "yuga_maki_dp0" },
	{ "1263-6460", "1B", HW_REV_YUGA_MAKI_DP1, HW_YUGA_MAKI, "yuga_maki_dp1" },
	{ "1263-6460", "1C", HW_REV_YUGA_MAKI_DP11, HW_YUGA_MAKI, "yuga_maki_dp1.1" },
	{ "1263-6460", "1D", HW_REV_YUGA_MAKI_SP1, HW_YUGA_MAKI, "yuga_maki_sp1" },
	{ "1263-6460", "1E", HW_REV_YUGA_MAKI_SP1, HW_YUGA_MAKI, "yuga_maki_sp1" },
	{ "1263-6460", "1F", HW_REV_YUGA_MAKI_SP1, HW_YUGA_MAKI, "yuga_maki_sp1" },
	{ "1263-6460", "1G", HW_REV_YUGA_MAKI_SP11, HW_YUGA_MAKI, "yuga_maki_sp1.1" },
	{ "1263-6460", "1H", HW_REV_YUGA_MAKI_SP11, HW_YUGA_MAKI, "yuga_maki_sp1.1" },
	{ "1263-6460", "1K", HW_REV_YUGA_MAKI_AP1, HW_YUGA_MAKI, "yuga_maki_ap1" },
	{ "1263-6460", "1M", HW_REV_YUGA_MAKI_AP1, HW_YUGA_MAKI, "yuga_maki_ap1" },
	{ "1263-6460", "1N", HW_REV_YUGA_MAKI_AP1, HW_YUGA_MAKI, "yuga_maki_ap1" },
	{ "1263-6460", "1P", HW_REV_YUGA_MAKI_AP11, HW_YUGA_MAKI, "yuga_maki_ap1.1" },
	{ "1263-6460", "1S", HW_REV_YUGA_MAKI_TP1, HW_YUGA_MAKI, "yuga_maki_tp1" },
	{ "1263-6460", "1T", HW_REV_YUGA_MAKI_TP1, HW_YUGA_MAKI, "yuga_maki_tp1" },
	{ "1263-6460", "1", HW_REV_YUGA_MAKI_TP1, HW_YUGA_MAKI, "yuga_maki_tp1" },
	{ "1263-6460", "1U", HW_REV_YUGA_MAKI_PQ, HW_YUGA_MAKI, "yuga_maki_pq" },
	{ "1263-6460", "1V", HW_REV_YUGA_MAKI_PQ, HW_YUGA_MAKI, "yuga_maki_pq" },
	{ "1263-6460", "2", HW_REV_YUGA_MAKI_PQ_HVM, HW_YUGA_MAKI, "yuga_maki_pq_hvm" },
	{ "1267-1691", "1A", HW_REV_YUGA_GINA_SP1, HW_YUGA, "yuga_gina_sp1" },
	{ "1267-1691", "1B", HW_REV_YUGA_GINA_SP11, HW_YUGA, "yuga_gina_sp1.1" },
	{ "1267-1691", "1D", HW_REV_YUGA_GINA_AP1, HW_YUGA, "yuga_gina_ap1" },
	{ "1267-1691", "1E", HW_REV_YUGA_GINA_AP1, HW_YUGA, "yuga_gina_ap1" },
	{ "1267-1691", "1G", HW_REV_YUGA_GINA_AP1, HW_YUGA, "yuga_gina_ap1" },
	{ "1267-1691", "1H", HW_REV_YUGA_GINA_AP12, HW_YUGA, "yuga_gina_ap1.2" },
	{ "1267-1691", "1J", HW_REV_YUGA_GINA_AP12, HW_YUGA, "yuga_gina_ap1.2" },
	{ "1267-1691", "1K", HW_REV_YUGA_GINA_AP12, HW_YUGA, "yuga_gina_ap1.2" },
	{ "1267-1691", "1L", HW_REV_YUGA_GINA_TP2, HW_YUGA, "yuga_gina_tp2" },
	{ "1267-1691", "1M", HW_REV_YUGA_GINA_TP2, HW_YUGA, "yuga_gina_tp2" },
	{ "1267-1691", "2", HW_REV_YUGA_GINA_TP2, HW_YUGA, "yuga_gina_tp2" },
	{ "1267-1691", "1N", HW_REV_YUGA_GINA_PQ2, HW_YUGA, "yuga_gina_pq2" },
	{ "1267-1691", "1P", HW_REV_YUGA_GINA_PQ2, HW_YUGA, "yuga_gina_pq2" },
	{ "1267-1691", "3", HW_REV_YUGA_GINA_PQ2_HVM, HW_YUGA, "yuga_gina_pq2_hvm" },
	{ "1267-4177", "1A", HW_REV_YUGA_RITA_SP1_SP11, HW_YUGA, "yuga_rita_sp1_sp1.1" },
	{ "1267-4177", "1B", HW_REV_YUGA_RITA_AP1, HW_YUGA, "yuga_rita_ap1" },
	{ "1267-4177", "1C", HW_REV_YUGA_RITA_AP1, HW_YUGA, "yuga_rita_ap1" },
	{ "1267-4177", "1E", HW_REV_YUGA_RITA_AP12, HW_YUGA, "yuga_rita_ap1.2" },
	{ "1267-4177", "1F", HW_REV_YUGA_RITA_AP12, HW_YUGA, "yuga_rita_ap1.2" },
	{ "1267-4177", "1", HW_REV_YUGA_RITA_AP12, HW_YUGA, "yuga_rita_ap1.2" },
	{ "1267-4177", "1G", HW_REV_YUGA_RITA_TP2, HW_YUGA, "yuga_rita_tp2" },
	{ "1267-4177", "2", HW_REV_YUGA_RITA_TP2, HW_YUGA, "yuga_rita_tp2" },
	{ "1267-4177", "1H", HW_REV_YUGA_RITA_PQ2, HW_YUGA, "yuga_rita_pq2" },
	{ "1267-4177", "3", HW_REV_YUGA_RITA_PQ2_HVM, HW_YUGA, "yuga_rita_pq2_hvm" },
	{ "1271-5995", "1A", HW_REV_YUGA_TINA_TP2, HW_YUGA, "yuga_tina_tp2" },
	{ "1271-5995", "1B", HW_REV_YUGA_TINA_TP3, HW_YUGA, "yuga_tina_tp3" },
	{ "1271-5995", "1C", HW_REV_YUGA_TINA_TP3, HW_YUGA, "yuga_tina_tp3" },
	{ "1271-5995", "1", HW_REV_YUGA_TINA_HVM, HW_YUGA, "yuga_tina_hvm" },
	{ "1266-1553", "1A", HW_REV_POLLUX_MAKI_DP1, HW_POLLUX_MAKI, "pollux_maki_dp1" },
	{ "1266-1553", "1C", HW_REV_POLLUX_MAKI_DP11, HW_POLLUX_MAKI, "pollux_maki_dp1.1" },
	{ "1266-1553", "1D", HW_REV_POLLUX_MAKI_DP11, HW_POLLUX_MAKI, "pollux_maki_dp1.1" },
	{ "1266-1553", "1F", HW_REV_POLLUX_MAKI_SP1, HW_POLLUX_MAKI, "pollux_maki_sp1" },
	{ "1266-1553", "1G", HW_REV_POLLUX_MAKI_SP1, HW_POLLUX_MAKI, "pollux_maki_sp1" },
	{ "1266-1553", "1H", HW_REV_POLLUX_MAKI_SP1, HW_POLLUX_MAKI, "pollux_maki_sp1" },
	{ "1266-1553", "1J", HW_REV_POLLUX_MAKI_AP1, HW_POLLUX_MAKI, "pollux_maki_ap1" },
	{ "1266-1553", "1K", HW_REV_POLLUX_MAKI_AP1, HW_POLLUX_MAKI, "pollux_maki_ap1" },
	{ "1266-1553", "1", HW_REV_POLLUX_MAKI_TP1, HW_POLLUX_MAKI, "pollux_maki_tp1" },
	{ "1266-1553", "2A", HW_REV_POLLUX_MAKI_PQ, HW_POLLUX_MAKI, "pollux_maki_pq" },
	{ "1266-1553", "2", HW_REV_POLLUX_MAKI_PQ_HV, HW_POLLUX_MAKI, "pollux_maki_pq_hv" },
	{ "1267-7244", "1A", HW_REV_POLLUX_GINA_DP11, HW_POLLUX, "pollux_gina_dp1.1" },
	{ "1267-7244", "1B", HW_REV_POLLUX_GINA_DP2, HW_POLLUX, "pollux_gina_dp2" },
	{ "1267-7244", "1C", HW_REV_POLLUX_GINA_DP2, HW_POLLUX, "pollux_gina_dp2" },
	{ "1267-7244", "1D", HW_REV_POLLUX_GINA_DP2, HW_POLLUX, "pollux_gina_dp2" },
	{ "1267-7244", "1E", HW_REV_POLLUX_GINA_DP2, HW_POLLUX, "pollux_gina_dp2" },
	{ "1267-7244", "1F", HW_REV_POLLUX_GINA_SP1, HW_POLLUX, "pollux_gina_sp1" },
	{ "1267-7244", "1G", HW_REV_POLLUX_GINA_AP1, HW_POLLUX, "pollux_gina_ap1" },
	{ "1267-7244", "1H", HW_REV_POLLUX_GINA_AP1, HW_POLLUX, "pollux_gina_ap1" },
	{ "1266-1660", "1A", HW_REV_POLLUX_REX_DP1, HW_POLLUX, "pollux_rex_dp1" },
	{ "1266-1660", "1B", HW_REV_POLLUX_REX_SP1, HW_POLLUX, "pollux_rex_sp1" },
	{ "1266-1660", "1C", HW_REV_POLLUX_REX_SP1, HW_POLLUX, "pollux_rex_sp1" },
	{ "1266-1660", "1D", HW_REV_POLLUX_REX_SP1, HW_POLLUX, "pollux_rex_sp1" },
	{ "1266-1675", "1A", HW_REV_POLLUX_WINDY_DP11, HW_POLLUX, "pollux_windy_dp1.1" },
	{ "1266-1675", "1B", HW_REV_POLLUX_WINDY_DP2_DP3, HW_POLLUX, "pollux_windy_dp2_dp3" },
	{ "1266-1675", "1C", HW_REV_POLLUX_WINDY_AP1, HW_POLLUX, "pollux_windy_ap1" },
	{ "1266-4533", "1A", HW_REV_POLLUX_WINDY_AP1, HW_POLLUX, "pollux_windy_ap1" },
	{ "1267-4466", "1A", HW_REV_DOGO_GINA_DP11, HW_DOGO, "dogo_gina_dp1.1" },
	{ "1267-4466", "1B", HW_REV_DOGO_GINA_DP11, HW_DOGO, "dogo_gina_dp1.1" },
	{ "1267-4466", "1C", HW_REV_DOGO_GINA_SP1, HW_DOGO, "dogo_gina_sp1" },
	{ "1267-4466", "1D", HW_REV_DOGO_GINA_SP1, HW_DOGO, "dogo_gina_sp1" },
	{ "1267-4466", "1E", HW_REV_DOGO_GINA_AP1, HW_DOGO, "dogo_gina_ap1" },
	{ "1267-4466", "1", HW_REV_DOGO_GINA_AP1, HW_DOGO, "dogo_gina_ap1" },
	{ "1267-4466", "2A", HW_REV_DOGO_GINA_TP1, HW_DOGO, "dogo_gina_tp1" },
	{ "1267-4466", "2", HW_REV_DOGO_GINA_TP1_PQ_HVM, HW_DOGO, "dogo_gina_tp1_pq_hvm" },
	{ "1267-4466", "3", HW_REV_DOGO_GINA_PQ_HVM, HW_DOGO, "dogo_gina_pq_hvm" },
	{ "1267-4473", "1A", HW_REV_DOGO_RITA_SP1, HW_DOGO, "dogo_rita_sp1" },
	{ "1267-4473", "1B", HW_REV_DOGO_RITA_AP1, HW_DOGO, "dogo_rita_ap1" },
	{ "1267-4473", "1", HW_REV_DOGO_RITA_AP1, HW_DOGO, "dogo_rita_ap1" },
	{ "1267-4473", "2", HW_REV_DOGO_RITA_TP1_PQ_HVM, HW_DOGO, "dogo_rita_tp1_pq_hvm" },
	{ "1267-4473", "3", HW_REV_DOGO_RITA_PQ_HVM, HW_DOGO, "dogo_rita_pq_hvm" },
	{ "1267-0594", "1A", HW_REV_DOGO_MAKI_DP1, HW_DOGO_MAKI, "dogo_maki_dp1" },
	{ "1267-0594", "1B", HW_REV_DOGO_MAKI_DP1, HW_DOGO_MAKI, "dogo_maki_dp1" },
	{ "1267-0594", "1C", HW_REV_DOGO_MAKI_DP1, HW_DOGO_MAKI, "dogo_maki_dp1" },
	{ "1267-0594", "1D", HW_REV_DOGO_MAKI_DP1, HW_DOGO_MAKI, "dogo_maki_dp1" },
	{ "1267-0594", "1E", HW_REV_DOGO_MAKI_SP1, HW_DOGO_MAKI, "dogo_maki_sp1" },
	{ "1267-0594", "1F", HW_REV_DOGO_MAKI_SP1, HW_DOGO_MAKI, "dogo_maki_sp1" },
	{ "1267-0594", "1G", HW_REV_DOGO_MAKI_SP1, HW_DOGO_MAKI, "dogo_maki_sp1" },
	{ "1267-0594", "1H", HW_REV_DOGO_MAKI_AP1, HW_DOGO_MAKI, "dogo_maki_ap1" },
	{ "1267-0594", "1", HW_REV_DOGO_MAKI_AP1, HW_DOGO_MAKI, "dogo_maki_ap1" },
	{ "1267-0594", "2A", HW_REV_DOGO_MAKI_TP1, HW_DOGO_MAKI, "dogo_maki_tp1" },
	{ "1267-0594", "2", HW_REV_DOGO_MAKI_TP1, HW_DOGO_MAKI, "dogo_maki_tp1" },
	{ "1267-0594", "3", HW_REV_DOGO_MAKI_PQ_HVM, HW_DOGO_MAKI, "dogo_maki_pq_hvm" },
};

#define SONY_HW_PBA_ID_MAXLEN 15
#define SONY_HW_PBA_REV_MAXLEN 10

static char sony_hw_pba_id[SONY_HW_PBA_ID_MAXLEN] = "not_set";
static char sony_hw_pba_rev[SONY_HW_PBA_REV_MAXLEN] = "not_set";

/* The fallback is set to 'future', which would indicate a newer build */
static int _sony_hw_rev = HW_REV_FUTURE;
static int _sony_hw = HW_UNKNOWN;
static const char *_sony_hw_rev_str = "future";

int sony_hw_rev(void)
{
	return _sony_hw_rev;
}

int sony_hw(void)
{
	return _sony_hw;
}

static void __init b16decode(char *src, char *dst, int dstlen)
{
	char *p;
	long c;
	char temp[3] = {0, 0, 0};
	int srclen;
	int i = 0;
	srclen = strnlen(src, dstlen * 2);

	for (p = src; p < src + srclen; p = p + 2) {
		temp[0] = *p;
		temp[1] = *(p + 1);
		if (kstrtol(temp, 16, &c) < 0)
			break;
		dst[i++] = (char)c;
	}
	dst[i] = 0;
}

static int __init do_very_early_param(char *param, char *val)
{
	if (strncmp(param, "oemandroidboot.babe1325",
		    sizeof("oemandroidboot.babe1325")) == 0) {
		b16decode(val, sony_hw_pba_id, SONY_HW_PBA_ID_MAXLEN);
	} else if (strncmp(param, "oemandroidboot.babe1326",
		    sizeof("oemandroidboot.babe1326")) == 0) {
		b16decode(val, sony_hw_pba_rev, SONY_HW_PBA_REV_MAXLEN);
	}
	return 0;
}

void __init sony_set_hw_revision(void)
{
	int i;
	char *slash;
	static __initdata char tmp_cmdline[COMMAND_LINE_SIZE];

	strlcpy(tmp_cmdline, boot_command_line, COMMAND_LINE_SIZE);
	parse_args("very early options", tmp_cmdline, NULL, 0, 0, 0,
		   do_very_early_param);

	printk(KERN_INFO "SONY hardware build PBA ID: '%s'\n",
	       sony_hw_pba_id);
	printk(KERN_INFO "SONY hardware build PBA rev(1): '%s'\n",
	       sony_hw_pba_rev);

	/* Clean the PBA revision ID */
	slash = strnchr(sony_hw_pba_rev, SONY_HW_PBA_REV_MAXLEN, '/');
	if (slash)
		*slash = 0;

	printk(KERN_INFO "SONY hardware build PBA rev(2): '%s'\n",
	       sony_hw_pba_rev);

	/* Map the build subversion to a numerical value */
	for (i = 0; i < ARRAY_SIZE(hw_revisions); i++) {
		if (!strncmp(hw_revisions[i].pba_id, sony_hw_pba_id,
			     SONY_HW_PBA_ID_MAXLEN)) {

			_sony_hw = hw_revisions[i].hw;

			if (!strncmp(hw_revisions[i].pba_rev, sony_hw_pba_rev,
				 SONY_HW_PBA_REV_MAXLEN)) {
				_sony_hw_rev = hw_revisions[i].hw_rev;
				_sony_hw_rev_str = hw_revisions[i].hw_rev_str;
				break;
			}
		}
	}

	printk(KERN_INFO "SONY hardware build HW rev: %s (%d)\n",
	       _sony_hw_rev_str, _sony_hw_rev);
}

static ssize_t sony_hwrev_show(struct kobject *kobj,
			       struct kobj_attribute *attr, char *buf)
{
	return sprintf(buf, "%s\n", _sony_hw_rev_str);
}

static struct kobj_attribute hwrev_attr =
	__ATTR(sony_hwrev, 0444, sony_hwrev_show, NULL);

static int __init hwrev_init_sysdev(void)
{
	return sysfs_create_file(kernel_kobj, &hwrev_attr.attr);
}

arch_initcall(hwrev_init_sysdev);
