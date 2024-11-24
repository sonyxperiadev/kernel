// SPDX-License-Identifier: GPL-2.0
#include <linux/bug.h>
#include <linux/kernel.h>
#include <linux/bitops.h>
#include <linux/fixp-arith.h>
#include <linux/iio/adc/qcom-vadc-common.h>
#include <linux/math64.h>
#include <linux/log2.h>
#include <linux/err.h>
#include <linux/module.h>
#include <linux/units.h>

/**
 * struct vadc_map_pt - Map the graph representation for ADC channel
 * @x: Represent the ADC digitized code.
 * @y: Represent the physical data which can be temperature, voltage,
 *     resistance.
 */
struct vadc_map_pt {
	s32 x;
	s32 y;
};

/* Voltage to temperature */
static const struct vadc_map_pt adcmap_100k_104ef_104fb[] = {
	{1758,	-40000 },
	{1742,	-35000 },
	{1719,	-30000 },
	{1691,	-25000 },
	{1654,	-20000 },
	{1608,	-15000 },
	{1551,	-10000 },
	{1483,	-5000 },
	{1404,	0 },
	{1315,	5000 },
	{1218,	10000 },
	{1114,	15000 },
	{1007,	20000 },
	{900,	25000 },
	{795,	30000 },
	{696,	35000 },
	{605,	40000 },
	{522,	45000 },
	{448,	50000 },
	{383,	55000 },
	{327,	60000 },
	{278,	65000 },
	{237,	70000 },
	{202,	75000 },
	{172,	80000 },
	{146,	85000 },
	{125,	90000 },
	{107,	95000 },
	{92,	100000 },
	{79,	105000 },
	{68,	110000 },
	{59,	115000 },
	{51,	120000 },
	{44,	125000 }
};

/*
 * Voltage to temperature table for 100k pull up for NTCG104EF104 with
 * 1.875V reference.
 */
static const struct vadc_map_pt adcmap_100k_104ef_104fb_1875_vref[] = {
	{ 1831,	-40000 },
	{ 1814,	-35000 },
	{ 1791,	-30000 },
	{ 1761,	-25000 },
	{ 1723,	-20000 },
	{ 1675,	-15000 },
	{ 1616,	-10000 },
	{ 1545,	-5000 },
	{ 1463,	0 },
	{ 1370,	5000 },
	{ 1268,	10000 },
	{ 1160,	15000 },
	{ 1049,	20000 },
	{ 937,	25000 },
	{ 828,	30000 },
	{ 726,	35000 },
	{ 630,	40000 },
	{ 544,	45000 },
	{ 467,	50000 },
	{ 399,	55000 },
	{ 340,	60000 },
	{ 290,	65000 },
	{ 247,	70000 },
	{ 209,	75000 },
	{ 179,	80000 },
	{ 153,	85000 },
	{ 130,	90000 },
	{ 112,	95000 },
	{ 96,	100000 },
	{ 82,	105000 },
	{ 71,	110000 },
	{ 62,	115000 },
	{ 53,	120000 },
	{ 46,	125000 },
};

#if IS_ENABLED(CONFIG_ARCH_SONY_COLUMBIA)
/*
 * Voltage to temperature table for SomC Columbia with

 * 1.875V reference.
 */
static const struct vadc_map_pt adcmap_100k_1875_vref_columbia[] = {
	{ 1832,	-40000 },
	{ 1829,	-39000 },
	{ 1826,	-38000 },
	{ 1822,	-37000 },
	{ 1818,	-36000 },
	{ 1815,	-35000 },
	{ 1811,	-34000 },
	{ 1806,	-33000 },
	{ 1802,	-32000 },
	{ 1797,	-31000 },
	{ 1792,	-30000 },
	{ 1786,	-29000 },
	{ 1781,	-28000 },
	{ 1775,	-27000 },
	{ 1768,	-26000 },
	{ 1762,	-25000 },
	{ 1755,	-24000 },
	{ 1747,	-23000 },
	{ 1740,	-22000 },
	{ 1732,	-21000 },
	{ 1723,	-20000 },
	{ 1714,	-19000 },
	{ 1705,	-18000 },
	{ 1696,	-17000 },
	{ 1686,	-16000 },
	{ 1675,	-15000 },
	{ 1664,	-14000 },
	{ 1653,	-13000 },
	{ 1641,	-12000 },
	{ 1629,	-11000 },
	{ 1616,	-10000 },
	{ 1603,	-9000 },
	{ 1589,	-8000 },
	{ 1575,	-7000 },
	{ 1561,	-6000 },
	{ 1546,	-5000 },
	{ 1530,	-4000 },
	{ 1514,	-3000 },
	{ 1498,	-2000 },
	{ 1481,	-1000 },
	{ 1463,	0 },
	{ 1446,	1000 },
	{ 1428,	2000 },
	{ 1409,	3000 },
	{ 1390,	4000 },
	{ 1371,	5000 },
	{ 1351,	6000 },
	{ 1331,	7000 },
	{ 1311,	8000 },
	{ 1290,	9000 },
	{ 1269,	10000 },
	{ 1248,	11000 },
	{ 1226,	12000 },
	{ 1205,	13000 },
	{ 1183,	14000 },
	{ 1161,	15000 },
	{ 1139,	16000 },
	{ 1117,	17000 },
	{ 1094,	18000 },
	{ 1072,	19000 },
	{ 1049,	20000 },
	{ 1027,	21000 },
	{ 1005,	22000 },
	{ 982,	23000 },
	{ 959,	24000 },
	{ 938,	25000 },
	{ 915,	26000 },
	{ 894,	27000 },
	{ 872,	28000 },
	{ 850,	29000 },
	{ 829,	30000 },
	{ 808,	31000 },
	{ 787,	32000 },
	{ 766,	33000 },
	{ 746,	34000 },
	{ 726,	35000 },
	{ 706,	36000 },
	{ 687,	37000 },
	{ 668,	38000 },
	{ 649,	39000 },
	{ 631,	40000 },
	{ 613,	41000 },
	{ 595,	42000 },
	{ 578,	43000 },
	{ 561,	44000 },
	{ 544,	45000 },
	{ 528,	46000 },
	{ 512,	47000 },
	{ 497,	48000 },
	{ 482,	49000 },
	{ 467,	50000 },
	{ 453,	51000 },
	{ 439,	52000 },
	{ 426,	53000 },
	{ 412,	54000 },
	{ 400,	55000 },
	{ 387,	56000 },
	{ 375,	57000 },
	{ 363,	58000 },
	{ 352,	59000 },
	{ 341,	60000 },
	{ 330,	61000 },
	{ 320,	62000 },
	{ 310,	63000 },
	{ 300,	64000 },
	{ 290,	65000 },
	{ 281,	66000 },
	{ 272,	67000 },
	{ 264,	68000 },
	{ 255,	69000 },
	{ 247,	70000 },
	{ 239,	71000 },
	{ 232,	72000 },
	{ 224,	73000 },
	{ 217,	74000 },
	{ 210,	75000 },
	{ 204,	76000 },
	{ 197,	77000 },
	{ 191,	78000 },
	{ 185,	79000 },
	{ 179,	80000 },
	{ 174,	81000 },
	{ 168,	82000 },
	{ 163,	83000 },
	{ 158,	84000 },
	{ 153,	85000 },
	{ 148,	86000 },
	{ 144,	87000 },
	{ 139,	88000 },
	{ 135,	89000 },
	{ 131,	90000 },
	{ 127,	91000 },
	{ 123,	92000 },
	{ 119,	93000 },
	{ 116,	94000 },
	{ 112,	95000 },
	{ 109,	96000 },
	{ 105,	97000 },
	{ 102,	98000 },
	{ 99,	99000 },
	{ 96,	100000 },
	{ 93,	101000 },
	{ 91,	102000 },
	{ 88,	103000 },
	{ 85,	104000 },
	{ 83,	105000 },
	{ 80,	106000 },
	{ 78,	107000 },
	{ 76,	108000 },
	{ 74,	109000 },
	{ 72,	110000 },
	{ 69,	111000 },
	{ 67,	112000 },
	{ 66,	113000 },
	{ 64,	114000 },
	{ 62,	115000 },
	{ 60,	116000 },
	{ 58,	117000 },
	{ 57,	118000 },
	{ 55,	119000 },
	{ 54,	120000 },
	{ 52,	121000 },
	{ 51,	122000 },
	{ 49,	123000 },
	{ 48,	124000 },
	{ 47,	125000 },
};
#endif

/*
 * Voltage to temperature table for 100k pull up for bat_therm with
 * Alium.
 */
static const struct vadc_map_pt adcmap_batt_therm_100k[] = {
	{1840,	-400},
	{1835,	-380},
	{1828,	-360},
	{1821,	-340},
	{1813,	-320},
	{1803,	-300},
	{1793,	-280},
	{1781,	-260},
	{1768,	-240},
	{1753,	-220},
	{1737,	-200},
	{1719,	-180},
	{1700,	-160},
	{1679,	-140},
	{1655,	-120},
	{1630,	-100},
	{1603,	-80},
	{1574,	-60},
	{1543,	-40},
	{1510,	-20},
	{1475,	0},
	{1438,	20},
	{1400,	40},
	{1360,	60},
	{1318,	80},
	{1276,	100},
	{1232,	120},
	{1187,	140},
	{1142,	160},
	{1097,	180},
	{1051,	200},
	{1005,	220},
	{960,	240},
	{915,	260},
	{871,	280},
	{828,	300},
	{786,	320},
	{745,	340},
	{705,	360},
	{666,	380},
	{629,	400},
	{594,	420},
	{560,	440},
	{527,	460},
	{497,	480},
	{467,	500},
	{439,	520},
	{413,	540},
	{388,	560},
	{365,	580},
	{343,	600},
	{322,	620},
	{302,	640},
	{284,	660},
	{267,	680},
	{251,	700},
	{235,	720},
	{221,	740},
	{208,	760},
	{195,	780},
	{184,	800},
	{173,	820},
	{163,	840},
	{153,	860},
	{144,	880},
	{136,	900},
	{128,	920},
	{120,	940},
	{114,	960},
	{107,	980}
};

/*
 * Voltage to temperature table for 30k pull up for bat_therm with
 * Alium.
 */
static const struct vadc_map_pt adcmap_batt_therm_30k[] = {
	{1864,	-400},
	{1863,	-380},
	{1861,	-360},
	{1858,	-340},
	{1856,	-320},
	{1853,	-300},
	{1850,	-280},
	{1846,	-260},
	{1842,	-240},
	{1837,	-220},
	{1831,	-200},
	{1825,	-180},
	{1819,	-160},
	{1811,	-140},
	{1803,	-120},
	{1794,	-100},
	{1784,	-80},
	{1773,	-60},
	{1761,	-40},
	{1748,	-20},
	{1734,	0},
	{1718,	20},
	{1702,	40},
	{1684,	60},
	{1664,	80},
	{1643,	100},
	{1621,	120},
	{1597,	140},
	{1572,	160},
	{1546,	180},
	{1518,	200},
	{1489,	220},
	{1458,	240},
	{1426,	260},
	{1393,	280},
	{1359,	300},
	{1324,	320},
	{1288,	340},
	{1252,	360},
	{1214,	380},
	{1176,	400},
	{1138,	420},
	{1100,	440},
	{1061,	460},
	{1023,	480},
	{985,	500},
	{947,	520},
	{910,	540},
	{873,	560},
	{836,	580},
	{801,	600},
	{766,	620},
	{732,	640},
	{699,	660},
	{668,	680},
	{637,	700},
	{607,	720},
	{578,	740},
	{550,	760},
	{524,	780},
	{498,	800},
	{474,	820},
	{451,	840},
	{428,	860},
	{407,	880},
	{387,	900},
	{367,	920},
	{349,	940},
	{332,	960},
	{315,	980}
};

/*
 * Voltage to temperature table for 400k pull up for bat_therm with
 * Alium.
 */
static const struct vadc_map_pt adcmap_batt_therm_400k[] = {
	{1744,	-400},
	{1724,	-380},
	{1701,	-360},
	{1676,	-340},
	{1648,	-320},
	{1618,	-300},
	{1584,	-280},
	{1548,	-260},
	{1509,	-240},
	{1468,	-220},
	{1423,	-200},
	{1377,	-180},
	{1328,	-160},
	{1277,	-140},
	{1225,	-120},
	{1171,	-100},
	{1117,	-80},
	{1062,	-60},
	{1007,	-40},
	{953,	-20},
	{899,	0},
	{847,	20},
	{795,	40},
	{745,	60},
	{697,	80},
	{651,	100},
	{607,	120},
	{565,	140},
	{526,	160},
	{488,	180},
	{453,	200},
	{420,	220},
	{390,	240},
	{361,	260},
	{334,	280},
	{309,	300},
	{286,	320},
	{265,	340},
	{245,	360},
	{227,	380},
	{210,	400},
	{195,	420},
	{180,	440},
	{167,	460},
	{155,	480},
	{144,	500},
	{133,	520},
	{124,	540},
	{115,	560},
	{107,	580},
	{99,	600},
	{92,	620},
	{86,	640},
	{80,	660},
	{75,	680},
	{70,	700},
	{65,	720},
	{61,	740},
	{57,	760},
	{53,	780},
	{50,	800},
	{46,	820},
	{43,	840},
	{41,	860},
	{38,	880},
	{36,	900},
	{34,	920},
	{32,	940},
	{30,	960},
	{28,	980}
};

static const struct vadc_map_pt adcmap7_die_temp[] = {
	{ 857300, 160000 },
	{ 820100, 140000 },
	{ 782500, 120000 },
	{ 744600, 100000 },
	{ 706400, 80000 },
	{ 667900, 60000 },
	{ 629300, 40000 },
	{ 590500, 20000 },
	{ 551500, 0 },
	{ 512400, -20000 },
	{ 473100, -40000 },
	{ 433700, -60000 },
};

/*
 * Resistance to temperature table for 100k pull up for NTCG104EF104.
 */
static const struct vadc_map_pt adcmap7_100k[] = {
	{ 4250657, -40960 },
	{ 3962085, -39936 },
	{ 3694875, -38912 },
	{ 3447322, -37888 },
	{ 3217867, -36864 },
	{ 3005082, -35840 },
	{ 2807660, -34816 },
	{ 2624405, -33792 },
	{ 2454218, -32768 },
	{ 2296094, -31744 },
	{ 2149108, -30720 },
	{ 2012414, -29696 },
	{ 1885232, -28672 },
	{ 1766846, -27648 },
	{ 1656598, -26624 },
	{ 1553884, -25600 },
	{ 1458147, -24576 },
	{ 1368873, -23552 },
	{ 1285590, -22528 },
	{ 1207863, -21504 },
	{ 1135290, -20480 },
	{ 1067501, -19456 },
	{ 1004155, -18432 },
	{ 944935, -17408 },
	{ 889550, -16384 },
	{ 837731, -15360 },
	{ 789229, -14336 },
	{ 743813, -13312 },
	{ 701271, -12288 },
	{ 661405, -11264 },
	{ 624032, -10240 },
	{ 588982, -9216 },
	{ 556100, -8192 },
	{ 525239, -7168 },
	{ 496264, -6144 },
	{ 469050, -5120 },
	{ 443480, -4096 },
	{ 419448, -3072 },
	{ 396851, -2048 },
	{ 375597, -1024 },
	{ 355598, 0 },
	{ 336775, 1024 },
	{ 319052, 2048 },
	{ 302359, 3072 },
	{ 286630, 4096 },
	{ 271806, 5120 },
	{ 257829, 6144 },
	{ 244646, 7168 },
	{ 232209, 8192 },
	{ 220471, 9216 },
	{ 209390, 10240 },
	{ 198926, 11264 },
	{ 189040, 12288 },
	{ 179698, 13312 },
	{ 170868, 14336 },
	{ 162519, 15360 },
	{ 154622, 16384 },
	{ 147150, 17408 },
	{ 140079, 18432 },
	{ 133385, 19456 },
	{ 127046, 20480 },
	{ 121042, 21504 },
	{ 115352, 22528 },
	{ 109960, 23552 },
	{ 104848, 24576 },
	{ 100000, 25600 },
	{ 95402, 26624 },
	{ 91038, 27648 },
	{ 86897, 28672 },
	{ 82965, 29696 },
	{ 79232, 30720 },
	{ 75686, 31744 },
	{ 72316, 32768 },
	{ 69114, 33792 },
	{ 66070, 34816 },
	{ 63176, 35840 },
	{ 60423, 36864 },
	{ 57804, 37888 },
	{ 55312, 38912 },
	{ 52940, 39936 },
	{ 50681, 40960 },
	{ 48531, 41984 },
	{ 46482, 43008 },
	{ 44530, 44032 },
	{ 42670, 45056 },
	{ 40897, 46080 },
	{ 39207, 47104 },
	{ 37595, 48128 },
	{ 36057, 49152 },
	{ 34590, 50176 },
	{ 33190, 51200 },
	{ 31853, 52224 },
	{ 30577, 53248 },
	{ 29358, 54272 },
	{ 28194, 55296 },
	{ 27082, 56320 },
	{ 26020, 57344 },
	{ 25004, 58368 },
	{ 24033, 59392 },
	{ 23104, 60416 },
	{ 22216, 61440 },
	{ 21367, 62464 },
	{ 20554, 63488 },
	{ 19776, 64512 },
	{ 19031, 65536 },
	{ 18318, 66560 },
	{ 17636, 67584 },
	{ 16982, 68608 },
	{ 16355, 69632 },
	{ 15755, 70656 },
	{ 15180, 71680 },
	{ 14628, 72704 },
	{ 14099, 73728 },
	{ 13592, 74752 },
	{ 13106, 75776 },
	{ 12640, 76800 },
	{ 12192, 77824 },
	{ 11762, 78848 },
	{ 11350, 79872 },
	{ 10954, 80896 },
	{ 10574, 81920 },
	{ 10209, 82944 },
	{ 9858, 83968 },
	{ 9521, 84992 },
	{ 9197, 86016 },
	{ 8886, 87040 },
	{ 8587, 88064 },
	{ 8299, 89088 },
	{ 8023, 90112 },
	{ 7757, 91136 },
	{ 7501, 92160 },
	{ 7254, 93184 },
	{ 7017, 94208 },
	{ 6789, 95232 },
	{ 6570, 96256 },
	{ 6358, 97280 },
	{ 6155, 98304 },
	{ 5959, 99328 },
	{ 5770, 100352 },
	{ 5588, 101376 },
	{ 5412, 102400 },
	{ 5243, 103424 },
	{ 5080, 104448 },
	{ 4923, 105472 },
	{ 4771, 106496 },
	{ 4625, 107520 },
	{ 4484, 108544 },
	{ 4348, 109568 },
	{ 4217, 110592 },
	{ 4090, 111616 },
	{ 3968, 112640 },
	{ 3850, 113664 },
	{ 3736, 114688 },
	{ 3626, 115712 },
	{ 3519, 116736 },
	{ 3417, 117760 },
	{ 3317, 118784 },
	{ 3221, 119808 },
	{ 3129, 120832 },
	{ 3039, 121856 },
	{ 2952, 122880 },
	{ 2868, 123904 },
	{ 2787, 124928 },
	{ 2709, 125952 },
	{ 2633, 126976 },
	{ 2560, 128000 },
	{ 2489, 129024 },
	{ 2420, 130048 }
};

/*
 * Resistance to temperature table for batt_therm.
 */
static const struct vadc_map_pt adcmap_gen3_batt_therm_100k[] = {
	{ 5319890, -400 },
	{ 4555860, -380 },
	{ 3911780, -360 },
	{ 3367320, -340 },
	{ 2905860, -320 },
	{ 2513730, -300 },
	{ 2179660, -280 },
	{ 1894360, -260 },
	{ 1650110, -240 },
	{ 1440520, -220 },
	{ 1260250, -200 },
	{ 1104850, -180 },
	{ 970600,  -160 },
	{ 854370,  -140 },
	{ 753530,  -120 },
	{ 665860,  -100 },
	{ 589490,  -80 },
	{ 522830,  -60 },
	{ 464540,  -40 },
	{ 413470,  -20 },
	{ 368640,  0 },
	{ 329220,  20 },
	{ 294490,  40 },
	{ 263850,  60 },
	{ 236770,  80 },
	{ 212790,  100 },
	{ 191530,  120 },
	{ 172640,  140 },
	{ 155840,  160 },
	{ 140880,  180 },
	{ 127520,  200 },
	{ 115590,  220 },
	{ 104910,  240 },
	{ 95350,   260 },
	{ 86760,   280 },
	{ 79050,   300 },
	{ 72110,   320 },
	{ 65860,   340 },
	{ 60220,   360 },
	{ 55130,   380 },
	{ 50520,   400 },
	{ 46350,   420 },
	{ 42570,   440 },
	{ 39140,   460 },
	{ 36030,   480 },
	{ 33190,   500 },
	{ 30620,   520 },
	{ 28260,   540 },
	{ 26120,   560 },
	{ 24160,   580 },
	{ 22370,   600 },
	{ 20730,   620 },
	{ 19230,   640 },
	{ 17850,   660 },
	{ 16580,   680 },
	{ 15420,   700 },
	{ 14350,   720 },
	{ 13370,   740 },
	{ 12470,   760 },
	{ 11630,   780 },
	{ 10860,   800 },
	{ 10150,   820 },
	{ 9490,    840 },
	{ 8880,    860 },
	{ 8320,    880 },
	{ 7800,    900 },
	{ 7310,    920 },
	{ 6860,    940 },
	{ 6450,    960 },
	{ 6060,    980 }
};

#if IS_ENABLED(CONFIG_ARCH_SONY_COLUMBIA)
/*
 * Resistance to temperature table for sys-therm-3 for SoMC Columbia.

*/
static const struct vadc_map_pt adcmap_sys_therm_100k_columbia[] = {
   { 4251000,	-40000 },
   { 3962000,	-39000 },
   { 3695000,	-38000 },
   { 3447000,	-37000 },
   { 3218000,	-36000 },
   { 3005000,	-35000 },
   { 2807000,	-34000 },
   { 2624000,	-33000 },
   { 2454000,	-32000 },
   { 2296000,	-31000 },
   { 2149000,	-30000 },
   { 2012000,	-29000 },
   { 1885000,	-28000 },
   { 1767000,	-27000 },
   { 1656000,	-26000 },
   { 1554000,	-25000 },
   { 1458000,	-24000 },
   { 1369000,	-23000 },
   { 1286000,	-22000 },
   { 1208000,	-21000 },
   { 1135000,	-20000 },
   { 1068000,	-19000 },
   { 1004000,	-18000 },
   { 945000,	-17000 },
   { 889600,	-16000 },
   { 837800,	-15000 },
   { 789300,	-14000 },
   { 743900,	-13000 },
   { 701300,	-12000 },
   { 661500,	-11000 },
   { 624100,	-10000 },
   { 589000,	-9000 },
   { 556200,	-8000 },
   { 525300,	-7000 },
   { 496300,	-6000 },
   { 469100,	-5000 },
   { 443500,	-4000 },
   { 419500,	-3000 },
   { 396900,	-2000 },
   { 375600,	-1000 },
   { 355600,	0 },
   { 336800,	1000 },
   { 319100,	2000 },
   { 302400,	3000 },
   { 286700,	4000 },
   { 271800,	5000 },
   { 257800,	6000 },
   { 244700,	7000 },
   { 232200,	8000 },
   { 220500,	9000 },
   { 209400,	10000 },
   { 198900,	11000 },
   { 189000,	12000 },
   { 179700,	13000 },
   { 170900,	14000 },
   { 162500,	15000 },
   { 154600,	16000 },
   { 147200,	17000 },
   { 140100,	18000 },
   { 133400,	19000 },
   { 127000,	20000 },
   { 121000,	21000 },
   { 115400,	22000 },
   { 110000,	23000 },
   { 104800,	24000 },
   { 100000,	25000 },
   { 95400,	26000 },
   { 91040,	27000 },
   { 86900,	28000 },
   { 82970,	29000 },
   { 79230,	30000 },
   { 75690,	31000 },
   { 72320,	32000 },
   { 69120,	33000 },
   { 66070,	34000 },
   { 63180,	35000 },
   { 60420,	36000 },
   { 57810,	37000 },
   { 55310,	38000 },
   { 52940,	39000 },
   { 50680,	40000 },
   { 48530,	41000 },
   { 46490,	42000 },
   { 44530,	43000 },
   { 42670,	44000 },
   { 40900,	45000 },
   { 39210,	46000 },
   { 37600,	47000 },
   { 36060,	48000 },
   { 34600,	49000 },
   { 33190,	50000 },
   { 31860,	51000 },
   { 30580,	52000 },
   { 29360,	53000 },
   { 28200,	54000 },
   { 27090,	55000 },
   { 26030,	56000 },
   { 25010,	57000 },
   { 24040,	58000 },
   { 23110,	59000 },
   { 22220,	60000 },
   { 21370,	61000 },
   { 20560,	62000 },
   { 19780,	63000 },
   { 19040,	64000 },
   { 18320,	65000 },
   { 17640,	66000 },
   { 16990,	67000 },
   { 16360,	68000 },
   { 15760,	69000 },
   { 15180,	70000 },
   { 14630,	71000 },
   { 14100,	72000 },
   { 13600,	73000 },
   { 13110,	74000 },
   { 12640,	75000 },
   { 12190,	76000 },
   { 11760,	77000 },
   { 11350,	78000 },
   { 10960,	79000 },
   { 10580,	80000 },
   { 10210,	81000 },
   { 9859,	82000 },
   { 9522,	83000 },
   { 9198,	84000 },
   { 8887,	85000 },
   { 8587,	86000 },
   { 8299,	87000 },
   { 8022,	88000 },
   { 7756,	89000 },
   { 7500,	90000 },
   { 7254,	91000 },
   { 7016,	92000 },
   { 6788,	93000 },
   { 6568,	94000 },
   { 6357,	95000 },
   { 6153,	96000 },
   { 5957,	97000 },
   { 5768,	98000 },
   { 5586,	99000 },
   { 5410,	100000 },
   { 5241,	101000 },
   { 5078,	102000 },
   { 4921,	103000 },
   { 4769,	104000 },
   { 4623,	105000 },
   { 4482,	106000 },
   { 4346,	107000 },
   { 4215,	108000 },
   { 4088,	109000 },
   { 3966,	110000 },
   { 3848,	111000 },
   { 3734,	112000 },
   { 3624,	113000 },
   { 3518,	114000 },
   { 3415,	115000 },
   { 3316,	116000 },
   { 3220,	117000 },
   { 3128,	118000 },
   { 3038,	119000 },
   { 2952,	120000 },
   { 2868,	121000 },
   { 2787,	122000 },
   { 2709,	123000 },
   { 2634,	124000 },
   { 2561,	125000 }
};
#endif

static const struct u32_fract adc5_prescale_ratios[] = {
	{ .numerator =  1, .denominator =  1 },
	{ .numerator =  1, .denominator =  3 },
	{ .numerator =  1, .denominator =  4 },
	{ .numerator =  1, .denominator =  6 },
	{ .numerator =  1, .denominator = 20 },
	{ .numerator =  1, .denominator =  8 },
	{ .numerator = 10, .denominator = 81 },
	{ .numerator =  1, .denominator = 10 },
	{ .numerator =  1, .denominator = 16 },
	{ .numerator = 40, .denominator = 41 },		/* PM7_SMB_TEMP */
	/* Prescale ratios for current channels below */
	{ .numerator = 32, .denominator = 100 },	/* IIN_FB, IIN_SMB */
	{ .numerator = 16, .denominator = 100 },	/* ICHG_SMB */
	{ .numerator = 1280, .denominator = 4100 },	/* IIN_SMB_new */
	{ .numerator = 640, .denominator = 4100 },	/* ICHG_SMB_new */
	{ .numerator = 1000, .denominator = 305185 },	/* ICHG_FB */
	{ .numerator = 1000, .denominator = 610370 },	/* ICHG_FB_2X, ICHG_FB for ADC5_GEN4 */
	{ .numerator = 1000, .denominator = 366220 },	/* ICHG_FB for ADC5_GEN3 */
	{ .numerator = 1000, .denominator = 732440 },	/* ICHG_FB_2X for ADC5_GEN3 */
	{ .numerator = 1000, .denominator = 1220740 },	/* ICHG_FB_2X for ADC5_GEN4*/
};

static int qcom_vadc_scale_hw_calib_volt(
				const struct u32_fract *prescale,
				const struct adc5_data *data,
				u16 adc_code, int *result_uv);
/* Current scaling for PMIC7 */
static int qcom_vadc_scale_hw_calib_current(
				const struct u32_fract *prescale,
				const struct adc5_data *data,
				u16 adc_code, int *result_ua);
/* Raw current for PMIC7 */
static int qcom_vadc_scale_hw_calib_current_raw(
				const struct u32_fract *prescale,
				const struct adc5_data *data,
				u16 adc_code, int *result_ua);
/* Current scaling for PMIC5 */
static int qcom_vadc5_scale_hw_calib_current(
				const struct u32_fract *prescale,
				const struct adc5_data *data,
				u16 adc_code, int *result_ua);
static int qcom_vadc_scale_hw_calib_therm(
				const struct u32_fract *prescale,
				const struct adc5_data *data,
				u16 adc_code, int *result_mdec);
static int qcom_vadc_scale_hw_calib_batt_therm_100(
				const struct u32_fract *prescale,
				const struct adc5_data *data,
				u16 adc_code, int *result_mdec);
#if IS_ENABLED(CONFIG_ARCH_SONY_COLUMBIA)
/* other sys therm */
static int qcom_vadc_scale_hw_calib_therm_100_columbia(
				const struct u32_fract *prescale,
				const struct adc5_data *data,
				u16 adc_code, int *result_mdec);
/*  sys therm 3 */
static int qcom_vadc_scale_hw_calib_sys_therm_100_columbia(
				const struct u32_fract *prescale,
				const struct adc5_data *data,
				u16 adc_code, int *result_mdec);
#endif
static int qcom_vadc_scale_hw_calib_batt_therm_30(
				const struct u32_fract *prescale,
				const struct adc5_data *data,
				u16 adc_code, int *result_mdec);
static int qcom_vadc_scale_hw_calib_batt_therm_400(
				const struct u32_fract *prescale,
				const struct adc5_data *data,
				u16 adc_code, int *result_mdec);
static int qcom_vadc7_scale_hw_calib_therm(
				const struct u32_fract *prescale,
				const struct adc5_data *data,
				u16 adc_code, int *result_mdec);
static int qcom_vadc_scale_hw_smb_temp(
				const struct u32_fract *prescale,
				const struct adc5_data *data,
				u16 adc_code, int *result_mdec);
static int qcom_vadc_scale_hw_pm7_smb_temp(
				const struct u32_fract *prescale,
				const struct adc5_data *data,
				u16 adc_code, int *result_mdec);
static int qcom_vadc_scale_hw_smb1398_temp(
				const struct u32_fract *prescale,
				const struct adc5_data *data,
				u16 adc_code, int *result_mdec);
static int qcom_vadc_scale_hw_pm2250_s3_die_temp(
				const struct u32_fract *prescale,
				const struct adc5_data *data,
				u16 adc_code, int *result_mdec);
static int qcom_adc5_gen3_scale_hw_calib_batt_therm_100(
				const struct u32_fract *prescale,
				const struct adc5_data *data,
				u16 adc_code, int *result_mdec);
static int qcom_adc5_gen3_scale_hw_calib_batt_id_100(
				const struct u32_fract *prescale,
				const struct adc5_data *data,
				u16 adc_code, int *result_mdec);
static int qcom_adc5_gen3_scale_hw_calib_usb_in_current(
				const struct u32_fract *prescale,
				const struct adc5_data *data,
				u16 adc_code, int *result_mdec);
static int qcom_vadc_scale_hw_chg5_temp(
				const struct u32_fract *prescale,
				const struct adc5_data *data,
				u16 adc_code, int *result_mdec);
static int qcom_vadc_scale_hw_pm7_chg_temp(
				const struct u32_fract *prescale,
				const struct adc5_data *data,
				u16 adc_code, int *result_mdec);
static int qcom_vadc_scale_hw_calib_die_temp(
				const struct u32_fract *prescale,
				const struct adc5_data *data,
				u16 adc_code, int *result_mdec);
static int qcom_vadc7_scale_hw_calib_die_temp(
				const struct u32_fract *prescale,
				const struct adc5_data *data,
				u16 adc_code, int *result_mdec);
static int qcom_adc5_gen4_scale_hw_calib_batt_therm_10(
				const struct u32_fract *prescale,
				const struct adc5_data *data,
				u16 adc_code, int *result_mdec);
static int qcom_adc5_gen4_scale_hw_calib_batt_id_10(
				const struct u32_fract *prescale,
				const struct adc5_data *data,
				u16 adc_code, int *result_mdec);

static struct qcom_adc5_scale_type scale_adc5_fn[] = {
	[SCALE_HW_CALIB_DEFAULT] = {qcom_vadc_scale_hw_calib_volt},
	[SCALE_HW_CALIB_CUR] = {qcom_vadc_scale_hw_calib_current},
	[SCALE_HW_CALIB_CUR_RAW] = {qcom_vadc_scale_hw_calib_current_raw},
	[SCALE_HW_CALIB_PM5_CUR] = {qcom_vadc5_scale_hw_calib_current},
	[SCALE_HW_CALIB_THERM_100K_PULLUP] = {qcom_vadc_scale_hw_calib_therm},
	[SCALE_HW_CALIB_BATT_THERM_100K] = {
				qcom_vadc_scale_hw_calib_batt_therm_100},
	[SCALE_HW_CALIB_BATT_THERM_30K] = {
				qcom_vadc_scale_hw_calib_batt_therm_30},
	[SCALE_HW_CALIB_BATT_THERM_400K] = {
				qcom_vadc_scale_hw_calib_batt_therm_400},
	[SCALE_HW_CALIB_XOTHERM] = {qcom_vadc_scale_hw_calib_therm},
	[SCALE_HW_CALIB_THERM_100K_PU_PM7] = {
					qcom_vadc7_scale_hw_calib_therm},
#if IS_ENABLED(CONFIG_ARCH_SONY_COLUMBIA)
	[SCALE_HW_CALIB_THERM_100K_PU_COLUMBIA] = {
					qcom_vadc_scale_hw_calib_therm_100_columbia},
	[SCALE_HW_CALIB_THERM_100K_PU_PM7_COLUMBIA] = {
					qcom_vadc_scale_hw_calib_sys_therm_100_columbia},
#endif
	[SCALE_HW_CALIB_PMIC_THERM] = {qcom_vadc_scale_hw_calib_die_temp},
	[SCALE_HW_CALIB_PMIC_THERM_PM7] = {
					qcom_vadc7_scale_hw_calib_die_temp},
	[SCALE_HW_CALIB_PM5_CHG_TEMP] = {qcom_vadc_scale_hw_chg5_temp},
	[SCALE_HW_CALIB_PM5_SMB_TEMP] = {qcom_vadc_scale_hw_smb_temp},
	[SCALE_HW_CALIB_PM5_SMB1398_TEMP] = {qcom_vadc_scale_hw_smb1398_temp},
	[SCALE_HW_CALIB_PM2250_S3_DIE_TEMP] = {qcom_vadc_scale_hw_pm2250_s3_die_temp},
	[SCALE_HW_CALIB_PM5_GEN3_BATT_THERM_100K] = {qcom_adc5_gen3_scale_hw_calib_batt_therm_100},
	[SCALE_HW_CALIB_PM5_GEN3_BATT_ID_100K] = {qcom_adc5_gen3_scale_hw_calib_batt_id_100},
	[SCALE_HW_CALIB_PM5_GEN3_USB_IN_I] = {qcom_adc5_gen3_scale_hw_calib_usb_in_current},
	[SCALE_HW_CALIB_PM7_SMB_TEMP] = {qcom_vadc_scale_hw_pm7_smb_temp},
	[SCALE_HW_CALIB_PM7_CHG_TEMP] = {qcom_vadc_scale_hw_pm7_chg_temp},
	[SCALE_HW_CALIB_PM5_GEN4_BATT_THERM_10K] = {qcom_adc5_gen4_scale_hw_calib_batt_therm_10},
	[SCALE_HW_CALIB_PM5_GEN4_BATT_ID_10K] = {qcom_adc5_gen4_scale_hw_calib_batt_id_10},
};

static int qcom_vadc_map_voltage_temp(const struct vadc_map_pt *pts,
				      u32 tablesize, s32 input, int *output)
{
	u32 i = 0;

	if (!pts)
		return -EINVAL;

	while (i < tablesize && pts[i].x > input)
		i++;

	if (i == 0) {
		*output = pts[0].y;
	} else if (i == tablesize) {
		*output = pts[tablesize - 1].y;
	} else {
		/* interpolate linearly */
		*output = fixp_linear_interpolate(pts[i - 1].x, pts[i - 1].y,
						  pts[i].x, pts[i].y,
						  input);
	}

	return 0;
}

static s32 qcom_vadc_map_temp_voltage(const struct vadc_map_pt *pts,
				      u32 tablesize, int input)
{
	u32 i = 0;

	/*
	 * Table must be sorted, find the interval of 'y' which contains value
	 * 'input' and map it to proper 'x' value
	 */
	while (i < tablesize && pts[i].y < input)
		i++;

	if (i == 0)
		return pts[0].x;
	if (i == tablesize)
		return pts[tablesize - 1].x;

	/* interpolate linearly */
	return fixp_linear_interpolate(pts[i - 1].y, pts[i - 1].x,
			pts[i].y, pts[i].x, input);
}

static void qcom_vadc_scale_calib(const struct vadc_linear_graph *calib_graph,
				  u16 adc_code,
				  bool absolute,
				  s64 *scale_voltage)
{
	*scale_voltage = (adc_code - calib_graph->gnd);
	*scale_voltage *= calib_graph->dx;
	*scale_voltage = div64_s64(*scale_voltage, calib_graph->dy);
	if (absolute)
		*scale_voltage += calib_graph->dx;

	if (*scale_voltage < 0)
		*scale_voltage = 0;
}

static int qcom_vadc_scale_volt(const struct vadc_linear_graph *calib_graph,
				const struct u32_fract *prescale,
				bool absolute, u16 adc_code,
				int *result_uv)
{
	s64 voltage = 0, result = 0;

	qcom_vadc_scale_calib(calib_graph, adc_code, absolute, &voltage);

	voltage *= prescale->denominator;
	result = div64_s64(voltage, prescale->numerator);
	*result_uv = result;

	return 0;
}

static int qcom_vadc_scale_therm(const struct vadc_linear_graph *calib_graph,
				 const struct u32_fract *prescale,
				 bool absolute, u16 adc_code,
				 int *result_mdec)
{
	s64 voltage = 0;
	int ret;

	qcom_vadc_scale_calib(calib_graph, adc_code, absolute, &voltage);

	if (absolute)
		voltage = div64_s64(voltage, 1000);

	ret = qcom_vadc_map_voltage_temp(adcmap_100k_104ef_104fb,
					 ARRAY_SIZE(adcmap_100k_104ef_104fb),
					 voltage, result_mdec);
	if (ret)
		return ret;

	return 0;
}

static int qcom_vadc_scale_die_temp(const struct vadc_linear_graph *calib_graph,
				    const struct u32_fract *prescale,
				    bool absolute,
				    u16 adc_code, int *result_mdec)
{
	s64 voltage = 0;
	u64 temp; /* Temporary variable for do_div */

	qcom_vadc_scale_calib(calib_graph, adc_code, absolute, &voltage);

	if (voltage > 0) {
		temp = voltage * prescale->denominator;
		do_div(temp, prescale->numerator * 2);
		voltage = temp;
	} else {
		voltage = 0;
	}

	*result_mdec = milli_kelvin_to_millicelsius(voltage);

	return 0;
}

static int qcom_vadc_scale_chg_temp(const struct vadc_linear_graph *calib_graph,
				    const struct u32_fract *prescale,
				    bool absolute,
				    u16 adc_code, int *result_mdec)
{
	s64 voltage = 0, result = 0;

	qcom_vadc_scale_calib(calib_graph, adc_code, absolute, &voltage);

	voltage *= prescale->denominator;
	voltage = div64_s64(voltage, prescale->numerator);
	voltage = ((PMI_CHG_SCALE_1) * (voltage * 2));
	voltage = (voltage + PMI_CHG_SCALE_2);
	result =  div64_s64(voltage, 1000000);
	*result_mdec = result;

	return 0;
}

/* convert voltage to ADC code, using 1.875V reference */
static u16 qcom_vadc_scale_voltage_code(s32 voltage,
					const struct u32_fract *prescale,
					const u32 full_scale_code_volt,
					unsigned int factor)
{
	s64 volt = voltage;
	s64 adc_vdd_ref_mv = 1875; /* reference voltage */

	volt *= prescale->numerator * factor * full_scale_code_volt;
	volt = div64_s64(volt, (s64)prescale->denominator * adc_vdd_ref_mv * 1000);

	return volt;
}

static int qcom_vadc_scale_code_voltage_factor(u16 adc_code,
				const struct u32_fract *prescale,
				const struct adc5_data *data,
				unsigned int factor)
{
	s64 voltage, temp, adc_vdd_ref_mv = 1875;

	/*
	 * The normal data range is between 0V to 1.875V. On cases where
	 * we read low voltage values, the ADC code can go beyond the
	 * range and the scale result is incorrect so we clamp the values
	 * for the cases where the code represents a value below 0V
	 */
	if (adc_code > VADC5_MAX_CODE)
		adc_code = 0;

	/* (ADC code * vref_vadc (1.875V)) / full_scale_code */
	voltage = (s64) adc_code * adc_vdd_ref_mv * 1000;
	voltage = div64_s64(voltage, data->full_scale_code_volt);
	if (voltage > 0) {
		voltage *= prescale->denominator;
		temp = prescale->numerator * factor;
		voltage = div64_s64(voltage, temp);
	} else {
		voltage = 0;
	}

	return (int) voltage;
}

static s64 adc_code_to_resistance(u16 adc_code, int pull_up, u16 full_scale_code)
{
	/* Resistance = (ADC code * R_PULLUP) / (full_scale_code - ADC code) */
	return div64_s64((s64) adc_code * pull_up, full_scale_code - adc_code);
}

static int qcom_vadc7_scale_hw_calib_therm(
				const struct u32_fract *prescale,
				const struct adc5_data *data,
				u16 adc_code, int *result_mdec)
{
	s64 resistance;
	int ret, result;

	if (adc_code >= data->full_scale_code_raw)
		return -EINVAL;

	resistance = adc_code_to_resistance(adc_code, R_PU_100K, data->full_scale_code_raw);

	ret = qcom_vadc_map_voltage_temp(adcmap7_100k,
				 ARRAY_SIZE(adcmap7_100k),
				 resistance, &result);
	if (ret)
		return ret;

	*result_mdec = result;

	return 0;
}

#if IS_ENABLED(CONFIG_ARCH_SONY_COLUMBIA)
static int qcom_vadc_scale_hw_calib_therm_100_columbia(
				const struct u32_fract *prescale,
				const struct adc5_data *data,
				u16 adc_code, int *result_mdec)
{
	int voltage;

	voltage = qcom_vadc_scale_code_voltage_factor(adc_code,
				prescale, data, 1000);

	/* Map voltage to temperature from look-up table */
	return qcom_vadc_map_voltage_temp(adcmap_100k_1875_vref_columbia,
				 ARRAY_SIZE(adcmap_100k_1875_vref_columbia),
				 voltage, result_mdec);
}

static int qcom_vadc_scale_hw_calib_sys_therm_100_columbia(
				const struct u32_fract *prescale,
				const struct adc5_data *data,
				u16 adc_code, int *result_mdec)
{
	s64 resistance = adc_code;
	int ret, result;

	if (adc_code >= RATIO_MAX_ADC7)
		return -EINVAL;

	/* (ADC code * R_PULLUP (100Kohm)) / (full_scale_code - ADC code) */
	resistance *= R_PU_100K;
	resistance = div64_s64(resistance, RATIO_MAX_ADC7 - adc_code);

	ret = qcom_vadc_map_voltage_temp(adcmap_sys_therm_100k_columbia,
				 ARRAY_SIZE(adcmap_sys_therm_100k_columbia),
				 resistance, &result);
	if (ret)
		return ret;

	*result_mdec = result;

	return 0;
}
#endif

static int qcom_vadc_scale_hw_calib_current_raw(
				const struct u32_fract *prescale,
				const struct adc5_data *data,
				u16 adc_code, int *result_ua)
{
	s64 temp;

	if (!prescale->numerator)
		return -EINVAL;

	temp = div_s64((s64)(s16)adc_code * prescale->denominator,
			prescale->numerator);
	*result_ua = (int) temp;
	pr_debug("raw adc_code: %#x result_ua: %d\n", adc_code, *result_ua);

	return 0;
}

static int qcom_vadc_scale_hw_calib_current(
				const struct u32_fract *prescale,
				const struct adc5_data *data,
				u16 adc_code, int *result_ua)
{
	u32 adc_vdd_ref_mv = 1875;
	s64 voltage;

	if (!prescale->numerator)
		return -EINVAL;

	/* (ADC code * vref_vadc (1.875V)) / full_scale_code */
	voltage = (s64)(s16) adc_code * adc_vdd_ref_mv * 1000;
	voltage = div_s64(voltage, data->full_scale_code_volt);
	voltage = div_s64(voltage * prescale->denominator, prescale->numerator);
	*result_ua = (int) voltage;
	pr_debug("adc_code: %#x result_ua: %d\n", adc_code, *result_ua);

	return 0;
}

static int qcom_vadc5_scale_hw_calib_current(
				const struct u32_fract *prescale,
				const struct adc5_data *data,
				u16 adc_code, int *result_ua)
{
	s64 voltage = 0, result = 0;
	bool positive = true;

	if (adc_code & ADC5_USR_DATA_CHECK) {
		adc_code = ~adc_code + 1;
		positive = false;
	}

	voltage = (s64)(s16) adc_code * data->full_scale_code_cur * 1000;
	voltage = div64_s64(voltage, VADC5_MAX_CODE);
	result = div64_s64(voltage * prescale->denominator, prescale->numerator);
	*result_ua = result;

	if (!positive)
		*result_ua = -result;

	return 0;
}

static int qcom_vadc_scale_hw_calib_volt(
				const struct u32_fract *prescale,
				const struct adc5_data *data,
				u16 adc_code, int *result_uv)
{
	*result_uv = qcom_vadc_scale_code_voltage_factor(adc_code,
				prescale, data, 1);

	return 0;
}

static int qcom_vadc_scale_hw_calib_therm(
				const struct u32_fract *prescale,
				const struct adc5_data *data,
				u16 adc_code, int *result_mdec)
{
	int voltage;

	voltage = qcom_vadc_scale_code_voltage_factor(adc_code,
				prescale, data, 1000);

	/* Map voltage to temperature from look-up table */
	return qcom_vadc_map_voltage_temp(adcmap_100k_104ef_104fb_1875_vref,
				 ARRAY_SIZE(adcmap_100k_104ef_104fb_1875_vref),
				 voltage, result_mdec);
}

static int qcom_vadc_scale_hw_calib_batt_therm_100(
				const struct u32_fract *prescale,
				const struct adc5_data *data,
				u16 adc_code, int *result_mdec)
{
	int voltage;

	voltage = qcom_vadc_scale_code_voltage_factor(adc_code,
				prescale, data, 1000);

	/* Map voltage to temperature from look-up table */
	return qcom_vadc_map_voltage_temp(adcmap_batt_therm_100k,
				 ARRAY_SIZE(adcmap_batt_therm_100k),
				 voltage, result_mdec);
}

static int qcom_vadc_scale_hw_calib_batt_therm_30(
				const struct u32_fract *prescale,
				const struct adc5_data *data,
				u16 adc_code, int *result_mdec)
{
	int voltage;

	voltage = qcom_vadc_scale_code_voltage_factor(adc_code,
				prescale, data, 1000);

	/* Map voltage to temperature from look-up table */
	return qcom_vadc_map_voltage_temp(adcmap_batt_therm_30k,
				 ARRAY_SIZE(adcmap_batt_therm_30k),
				 voltage, result_mdec);
}

static int qcom_vadc_scale_hw_calib_batt_therm_400(
				const struct u32_fract *prescale,
				const struct adc5_data *data,
				u16 adc_code, int *result_mdec)
{
	int voltage;

	voltage = qcom_vadc_scale_code_voltage_factor(adc_code,
				prescale, data, 1000);

	/* Map voltage to temperature from look-up table */
	return qcom_vadc_map_voltage_temp(adcmap_batt_therm_400k,
				 ARRAY_SIZE(adcmap_batt_therm_400k),
				 voltage, result_mdec);
}

static int qcom_vadc_scale_hw_calib_die_temp(
				const struct u32_fract *prescale,
				const struct adc5_data *data,
				u16 adc_code, int *result_mdec)
{
	*result_mdec = qcom_vadc_scale_code_voltage_factor(adc_code,
				prescale, data, 2);
	*result_mdec = milli_kelvin_to_millicelsius(*result_mdec);

	return 0;
}

static int qcom_vadc7_scale_hw_calib_die_temp(
				const struct u32_fract *prescale,
				const struct adc5_data *data,
				u16 adc_code, int *result_mdec)
{

	int voltage;

	voltage = qcom_vadc_scale_code_voltage_factor(adc_code,
				prescale, data, 1);

	return qcom_vadc_map_voltage_temp(adcmap7_die_temp, ARRAY_SIZE(adcmap7_die_temp),
			voltage, result_mdec);
}

static int qcom_vadc_scale_hw_pm7_chg_temp(
				const struct u32_fract *prescale,
				const struct adc5_data *data,
				u16 adc_code, int *result_mdec)
{
	s64 temp;
	int result_uv;

	result_uv = qcom_vadc_scale_code_voltage_factor(adc_code,
				prescale, data, 1);

	/* T(C) = Vadc/0.0033 – 277.12 */
	temp = div_s64((30303LL * result_uv) - (27712 * 1000000LL), 100000);
	pr_debug("adc_code: %u result_uv: %d temp: %lld\n", adc_code, result_uv,
		temp);
	*result_mdec = temp > 0 ? temp : 0;

	return 0;
}

static int qcom_vadc_scale_hw_pm7_smb_temp(
				const struct u32_fract *prescale,
				const struct adc5_data *data,
				u16 adc_code, int *result_mdec)
{
	s64 temp;
	int result_uv;

	result_uv = qcom_vadc_scale_code_voltage_factor(adc_code,
				prescale, data, 1);

	/* T(C) = 25 + (25*Vadc - 24.885) / 0.0894 */
	temp = div_s64(((25000LL * result_uv) - (24885 * 1000000LL)) * 10000,
			894 * 1000000) + 25000;
	pr_debug("adc_code: %#x result_uv: %d temp: %lld\n", adc_code,
		result_uv, temp);
	*result_mdec = temp > 0 ? temp : 0;

	return 0;
}

static int qcom_vadc_scale_hw_smb_temp(
				const struct u32_fract *prescale,
				const struct adc5_data *data,
				u16 adc_code, int *result_mdec)
{
	*result_mdec = qcom_vadc_scale_code_voltage_factor(adc_code * 100,
				prescale, data, PMIC5_SMB_TEMP_SCALE_FACTOR);
	*result_mdec = PMIC5_SMB_TEMP_CONSTANT - *result_mdec;

	return 0;
}

static int qcom_vadc_scale_hw_smb1398_temp(
				const struct u32_fract *prescale,
				const struct adc5_data *data,
				u16 adc_code, int *result_mdec)
{
	s64 voltage = 0, adc_vdd_ref_mv = 1875;
	u64 temp;

	if (adc_code > VADC5_MAX_CODE)
		adc_code = 0;

	/* (ADC code * vref_vadc (1.875V)) / full_scale_code */
	voltage = (s64) adc_code * adc_vdd_ref_mv * 1000;
	voltage = div64_s64(voltage, data->full_scale_code_volt);
	if (voltage > 0) {
		temp = voltage * prescale->denominator;
		temp *= 100;
		do_div(temp, prescale->numerator * PMIC5_SMB1398_TEMP_SCALE_FACTOR);
		voltage = temp;
	} else {
		voltage = 0;
	}

	voltage = voltage - PMIC5_SMB1398_TEMP_CONSTANT;
	*result_mdec = voltage;

	return 0;
}

static int qcom_vadc_scale_hw_pm2250_s3_die_temp(
				const struct u32_fract *prescale,
				const struct adc5_data *data,
				u16 adc_code, int *result_mdec)
{
	s64 voltage = 0, adc_vdd_ref_mv = 1875;

	if (adc_code > VADC5_MAX_CODE)
		adc_code = 0;

	/* (ADC code * vref_vadc (1.875V)) / full_scale_code */
	voltage = (s64) adc_code * adc_vdd_ref_mv * 1000;
	voltage = div64_s64(voltage, data->full_scale_code_volt);
	if (voltage > 0) {
		voltage *= prescale->denominator;
		voltage = div64_s64(voltage, prescale->numerator);
	} else {
		voltage = 0;
	}

	voltage = PMIC5_PM2250_S3_DIE_TEMP_CONSTANT - voltage;
	voltage *= 100000;
	voltage = div64_s64(voltage, PMIC5_PM2250_S3_DIE_TEMP_SCALE_FACTOR);

	*result_mdec = voltage;

	return 0;
}

static int qcom_adc5_gen3_scale_hw_calib_batt_therm_100(
				const struct u32_fract *prescale,
				const struct adc5_data *data,
				u16 adc_code, int *result_mdec)
{
	s64 resistance;
	int ret, result = 0;

	if (adc_code >= data->full_scale_code_raw)
		return -EINVAL;

	resistance = adc_code_to_resistance(adc_code, R_PU_100K, data->full_scale_code_raw);

	ret = qcom_vadc_map_voltage_temp(adcmap_gen3_batt_therm_100k,
				 ARRAY_SIZE(adcmap_gen3_batt_therm_100k),
				 resistance, &result);
	if (ret)
		return ret;

	*result_mdec = result;

	return 0;
}

static int qcom_adc5_gen3_scale_hw_calib_batt_id_100(
				const struct u32_fract *prescale,
				const struct adc5_data *data,
				u16 adc_code, int *result_mdec)
{
	if (adc_code >= data->full_scale_code_raw)
		return -EINVAL;

	*result_mdec = (int)adc_code_to_resistance(adc_code, R_PU_100K, data->full_scale_code_raw);

	return 0;
};

static int qcom_adc5_gen3_scale_hw_calib_usb_in_current(
				const struct u32_fract *prescale,
				const struct adc5_data *data,
				u16 adc_code, int *result_ua)
{
	s64 voltage = 0, result = 0;
	bool positive = true;

	if (adc_code & ADC5_USR_DATA_CHECK) {
		adc_code = ~adc_code + 1;
		positive = false;
	}

	voltage = (s64)(s16) adc_code * 1000000;
	voltage = div64_s64(voltage, PMIC5_GEN3_USB_IN_I_SCALE_FACTOR);
	result = div64_s64(voltage * prescale->denominator, prescale->numerator);
	*result_ua = (int)result;

	if (!positive)
		*result_ua = -(int)result;

	return 0;
};

static int qcom_vadc_scale_hw_chg5_temp(
				const struct u32_fract *prescale,
				const struct adc5_data *data,
				u16 adc_code, int *result_mdec)
{
	*result_mdec = qcom_vadc_scale_code_voltage_factor(adc_code,
				prescale, data, 4);
	*result_mdec = PMIC5_CHG_TEMP_SCALE_FACTOR - *result_mdec;

	return 0;
}

static int qcom_adc5_gen4_scale_hw_calib_batt_therm_10(
				const struct u32_fract *prescale,
				const struct adc5_data *data,
				u16 adc_code, int *result_mdec)
{
	s64 resistance;
	int ret, result = 0;

	if (adc_code >= data->full_scale_code_raw)
		return -EINVAL;

	resistance = adc_code_to_resistance(adc_code, R_PU_10K, data->full_scale_code_raw);

	ret = qcom_vadc_map_voltage_temp(adcmap_gen3_batt_therm_100k,
				 ARRAY_SIZE(adcmap_gen3_batt_therm_100k),
				 resistance, &result);
	if (ret)
		return ret;

	*result_mdec = result;

	return 0;
}

static int qcom_adc5_gen4_scale_hw_calib_batt_id_10(
				const struct u32_fract *prescale,
				const struct adc5_data *data,
				u16 adc_code, int *result_mdec)
{
	if (adc_code >= data->full_scale_code_raw)
		return -EINVAL;

	*result_mdec = (int)adc_code_to_resistance(adc_code, R_PU_10K, data->full_scale_code_raw);

	return 0;
};

void adc_tm_scale_therm_voltage_100k_gen3(struct adc_tm_config *param, const struct adc5_data *data)
{
	int temp, ret;
	int64_t resistance = 0;

	/*
	 * High temperature maps to lower threshold voltage.
	 * Same API can be used for resistance-temperature table
	 */
	resistance = qcom_vadc_map_temp_voltage(adcmap7_100k,
						ARRAY_SIZE(adcmap7_100k),
						param->high_thr_temp);

	param->low_thr_voltage = resistance * data->full_scale_code_raw;
	param->low_thr_voltage = div64_s64(param->low_thr_voltage,
						(resistance + R_PU_100K));

	/*
	 * low_thr_voltage is ADC raw code corresponding to upper temperature
	 * threshold.
	 * Instead of returning the ADC raw code obtained at this point,we first
	 * do a forward conversion on the (low voltage / high temperature) threshold code,
	 * to temperature, to check if that code, when read by TM, would translate to
	 * a temperature greater than or equal to the upper temperature limit (which is
	 * expected). If it is instead lower than the upper limit (not expected for correct
	 * TM functionality), we lower the raw code of the threshold written by 1
	 * to ensure TM does see a violation when it reads raw code corresponding
	 * to the upper limit temperature specified.
	 */
	ret = qcom_vadc7_scale_hw_calib_therm(NULL, data, param->low_thr_voltage, &temp);
	if (ret < 0)
		return;

	if (temp < param->high_thr_temp)
		param->low_thr_voltage--;

	/*
	 * Low temperature maps to higher threshold voltage
	 * Same API can be used for resistance-temperature table
	 */
	resistance = qcom_vadc_map_temp_voltage(adcmap7_100k,
						ARRAY_SIZE(adcmap7_100k),
						param->low_thr_temp);

	param->high_thr_voltage = resistance * data->full_scale_code_raw;
	param->high_thr_voltage = div64_s64(param->high_thr_voltage,
						(resistance + R_PU_100K));

	/*
	 * high_thr_voltage is ADC raw code corresponding to lower temperature
	 * threshold.
	 * Similar to what is done above for low_thr voltage, we first
	 * do a forward conversion on the (high voltage / low temperature)threshold code,
	 * to temperature, to check if that code, when read by TM, would translate to a
	 * temperature less than or equal to the lower temperature limit (which is expected).
	 * If it is instead greater than the lower limit (not expected for correct
	 * TM functionality), we increase the raw code of the threshold written by 1
	 * to ensure TM does see a violation when it reads raw code corresponding
	 * to the lower limit temperature specified.
	 */
	ret = qcom_vadc7_scale_hw_calib_therm(NULL, data, param->high_thr_voltage, &temp);
	if (ret < 0)
		return;

	if (temp > param->low_thr_temp)
		param->high_thr_voltage++;
}
EXPORT_SYMBOL_GPL(adc_tm_scale_therm_voltage_100k_gen3);

int32_t adc_tm_absolute_rthr_gen3(struct adc_tm_config *tm_config)
{
	int64_t low_thr = 0, high_thr = 0;

	low_thr =  tm_config->low_thr_voltage;
	low_thr *= ADC5_FULL_SCALE_CODE;

	low_thr = div64_s64(low_thr, ADC_VDD_REF);
	tm_config->low_thr_voltage = low_thr;

	high_thr =  tm_config->high_thr_voltage;
	high_thr *= ADC5_FULL_SCALE_CODE;

	high_thr = div64_s64(high_thr, ADC_VDD_REF);
	tm_config->high_thr_voltage = high_thr;

	return 0;
}
EXPORT_SYMBOL_GPL(adc_tm_absolute_rthr_gen3);

int qcom_vadc_scale(enum vadc_scale_fn_type scaletype,
		    const struct vadc_linear_graph *calib_graph,
		    const struct u32_fract *prescale,
		    bool absolute,
		    u16 adc_code, int *result)
{
	switch (scaletype) {
	case SCALE_DEFAULT:
		return qcom_vadc_scale_volt(calib_graph, prescale,
					    absolute, adc_code,
					    result);
	case SCALE_THERM_100K_PULLUP:
	case SCALE_XOTHERM:
		return qcom_vadc_scale_therm(calib_graph, prescale,
					     absolute, adc_code,
					     result);
	case SCALE_PMIC_THERM:
		return qcom_vadc_scale_die_temp(calib_graph, prescale,
						absolute, adc_code,
						result);
	case SCALE_PMI_CHG_TEMP:
		return qcom_vadc_scale_chg_temp(calib_graph, prescale,
						absolute, adc_code,
						result);
	default:
		return -EINVAL;
	}
}
EXPORT_SYMBOL(qcom_vadc_scale);

u16 qcom_adc_tm5_temp_volt_scale(unsigned int prescale_ratio,
				 u32 full_scale_code_volt, int temp)
{
	const struct u32_fract *prescale = &adc5_prescale_ratios[prescale_ratio];
	s32 voltage;

	voltage = qcom_vadc_map_temp_voltage(adcmap_100k_104ef_104fb_1875_vref,
					     ARRAY_SIZE(adcmap_100k_104ef_104fb_1875_vref),
					     temp);
	return qcom_vadc_scale_voltage_code(voltage, prescale, full_scale_code_volt, 1000);
}
EXPORT_SYMBOL(qcom_adc_tm5_temp_volt_scale);

u16 qcom_adc_tm5_gen2_temp_res_scale(int temp)
{
	int64_t resistance;

	resistance = qcom_vadc_map_temp_voltage(adcmap7_100k,
		ARRAY_SIZE(adcmap7_100k), temp);

	return div64_s64(resistance * RATIO_MAX_ADC7, resistance + R_PU_100K);
}
EXPORT_SYMBOL(qcom_adc_tm5_gen2_temp_res_scale);

int qcom_adc5_hw_scale(enum vadc_scale_fn_type scaletype,
		    unsigned int prescale_ratio,
		    const struct adc5_data *data,
		    u16 adc_code, int *result)
{
	const struct u32_fract *prescale = &adc5_prescale_ratios[prescale_ratio];

	if (!(scaletype >= SCALE_HW_CALIB_DEFAULT &&
		scaletype < SCALE_HW_CALIB_INVALID)) {
		pr_err("Invalid scale type %d\n", scaletype);
		return -EINVAL;
	}

	return scale_adc5_fn[scaletype].scale_fn(prescale, data,
					adc_code, result);
}
EXPORT_SYMBOL(qcom_adc5_hw_scale);

int qcom_adc5_prescaling_from_dt(u32 numerator, u32 denominator)
{
	unsigned int pre;

	for (pre = 0; pre < ARRAY_SIZE(adc5_prescale_ratios); pre++)
		if (adc5_prescale_ratios[pre].numerator == numerator &&
		    adc5_prescale_ratios[pre].denominator == denominator)
			break;

	if (pre == ARRAY_SIZE(adc5_prescale_ratios))
		return -EINVAL;

	return pre;
}
EXPORT_SYMBOL(qcom_adc5_prescaling_from_dt);

int qcom_adc5_hw_settle_time_from_dt(u32 value,
				     const unsigned int *hw_settle)
{
	unsigned int i;

	for (i = 0; i < VADC_HW_SETTLE_SAMPLES_MAX; i++) {
		if (value == hw_settle[i])
			return i;
	}

	return -EINVAL;
}
EXPORT_SYMBOL(qcom_adc5_hw_settle_time_from_dt);

int qcom_adc5_avg_samples_from_dt(u32 value)
{
	if (!is_power_of_2(value) || value > ADC5_AVG_SAMPLES_MAX)
		return -EINVAL;

	return __ffs(value);
}
EXPORT_SYMBOL(qcom_adc5_avg_samples_from_dt);

int qcom_adc5_decimation_from_dt(u32 value, const unsigned int *decimation)
{
	unsigned int i;

	for (i = 0; i < ADC5_DECIMATION_SAMPLES_MAX; i++) {
		if (value == decimation[i])
			return i;
	}

	return -EINVAL;
}
EXPORT_SYMBOL(qcom_adc5_decimation_from_dt);

int qcom_vadc_decimation_from_dt(u32 value)
{
	if (!is_power_of_2(value) || value < VADC_DECIMATION_MIN ||
	    value > VADC_DECIMATION_MAX)
		return -EINVAL;

	return __ffs64(value / VADC_DECIMATION_MIN);
}
EXPORT_SYMBOL(qcom_vadc_decimation_from_dt);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Qualcomm ADC common functionality");
