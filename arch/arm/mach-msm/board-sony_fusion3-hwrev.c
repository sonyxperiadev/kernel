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
