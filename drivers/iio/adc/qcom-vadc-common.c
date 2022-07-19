// SPDX-License-Identifier: GPL-2.0
#include <linux/bug.h>
#include <linux/kernel.h>
#include <linux/bitops.h>
#include <linux/math64.h>
#include <linux/log2.h>
#include <linux/err.h>
#include <linux/module.h>

#include "qcom-vadc-common.h"

/* Voltage to temperature */
static const struct vadc_map_pt adcmap_100k_104ef_104fb[] = {
	{1758,	-40},
	{1742,	-35},
	{1719,	-30},
	{1691,	-25},
	{1654,	-20},
	{1608,	-15},
	{1551,	-10},
	{1483,	-5},
	{1404,	0},
	{1315,	5},
	{1218,	10},
	{1114,	15},
	{1007,	20},
	{900,	25},
	{795,	30},
	{696,	35},
	{605,	40},
	{522,	45},
	{448,	50},
	{383,	55},
	{327,	60},
	{278,	65},
	{237,	70},
	{202,	75},
	{172,	80},
	{146,	85},
	{125,	90},
	{107,	95},
	{92,	100},
	{79,	105},
	{68,	110},
	{59,	115},
	{51,	120},
	{44,	125}
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


static const struct vadc_map_pt adcmap_voltage_to_therm_100k_QN5965[] = {
	{1833 , -400 },
	{1830 , -390 },
	{1827 , -380 },
	{1824 , -370 },
	{1820 , -360 },
	{1816 , -350 },
	{1812 , -340 },
	{1808 , -330 },
	{1803 , -320 },
	{1798 , -310 },
	{1793 , -300 },
	{1788 , -290 },
	{1782 , -280 },
	{1776 , -270 },
	{1770 , -260 },
	{1764 , -250 },
	{1757 , -240 },
	{1749 , -230 },
	{1742 , -220 },
	{1734 , -210 },
	{1725 , -200 },
	{1716 , -190 },
	{1707 , -180 },
	{1697 , -170 },
	{1687 , -160 },
	{1677 , -150 },
	{1666 , -140 },
	{1655 , -130 },
	{1643 , -120 },
	{1631 , -110 },
	{1618 , -100 },
	{1605 , -90  },
	{1591 , -80  },
	{1577 , -70  },
	{1562 , -60  },
	{1547 , -50  },
	{1531 , -40  },
	{1515 , -30  },
	{1499 , -20  },
	{1482 , -10  },
	{1465 , 0   },
	{1447 , 10   },
	{1429 , 20   },
	{1410 , 30   },
	{1391 , 40   },
	{1372 , 50   },
	{1352 , 60   },
	{1332 , 70   },
	{1311 , 80   },
	{1291 , 90   },
	{1270 , 100  },
	{1248 , 110  },
	{1227 , 120  },
	{1205 , 130  },
	{1183 , 140  },
	{1161 , 150  },
	{1139 , 160  },
	{1117 , 170  },
	{1094 , 180  },
	{1072 , 190  },
	{1049 , 200  },
	{1027 , 210  },
	{1004 , 220  },
	{982  , 230  },
	{960  , 240  },
	{938  , 250  },
	{915  , 260  },
	{893  , 270  },
	{872  , 280  },
	{850  , 290  },
	{829  , 300  },
	{808  , 310  },
	{787  , 320  },
	{766  , 330  },
	{746  , 340  },
	{726  , 350  },
	{706  , 360  },
	{687  , 370  },
	{668  , 380  },
	{649  , 390  },
	{631  , 400  },
	{613  , 410  },
	{595  , 420  },
	{578  , 430  },
	{561  , 440  },
	{544  , 450  },
	{528  , 460  },
	{512  , 470  },
	{497  , 480  },
	{482  , 490  },
	{467  , 500  },
	{453  , 510  },
	{439  , 520  },
	{426  , 530  },
	{412  , 540  },
	{400  , 550  },
	{387  , 560  },
	{375  , 570  },
	{363  , 580  },
	{352  , 590  },
	{341  , 600  },
	{330  , 610  },
	{320  , 620  },
	{310  , 630  },
	{300  , 640  },
	{290  , 650  },
	{281  , 660  },
	{272  , 670  },
	{264  , 680  },
	{255  , 690  },
	{247  , 700  },
	{239  , 710  },
	{232  , 720  },
	{224  , 730  },
	{217  , 740  },
	{210  , 750  },
	{204  , 760  },
	{197  , 770  },
	{191  , 780  },
	{185  , 790  },
	{179  , 800  },
	{174  , 810  },
	{168  , 820  },
	{163  , 830  },
	{158  , 840  },
	{153  , 850  },
	{148  , 860  },
	{143  , 870  },
	{139  , 880  },
	{135  , 890  },
	{131  , 900  },
	{126  , 910  },
	{123  , 920  },
	{119  , 930  },
	{115  , 940  },
	{112  , 950  },
	{108  , 960  },
	{105  , 970  },
	{102  , 980  },
	{99   , 990  },
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
	{ 433700, 1967},
	{ 473100, 1964},
	{ 512400, 1957},
	{ 551500, 1949},
	{ 590500, 1940},
	{ 629300, 1930},
	{ 667900, 1921},
	{ 706400, 1910},
	{ 744600, 1896},
	{ 782500, 1878},
	{ 820100, 1859},
	{ 857300, 0},
};

static const struct vadc_map_pt adcmap7_res_to_temp_100k_QN5965[] = {
	{4397119 ,	-40000 },
	{4092873 ,	-39000 },
	{3811717 ,	-38000 },
	{3551748 ,	-37000 },
	{3311235 ,	-36000 },
	{3088598 ,	-35000 },
	{2882395 ,	-34000 },
	{2691309 ,	-33000 },
	{2514137 ,	-32000 },
	{2349777 ,	-31000 },
	{2197225 ,	-30000 },
	{2055557 ,	-29000 },
	{1923931 ,	-28000 },
	{1801573 ,	-27000 },
	{1687773 ,	-26000 },
	{1581880 ,	-25000 },
	{1483099 ,	-24000 },
	{1391113 ,	-23000 },
	{1305412 ,	-22000 },
	{1225530 ,	-21000 },
	{1151036 ,	-20000 },
	{1081535 ,	-19000 },
	{1016661 ,	-18000 },
	{956079	,	-17000 },
	{899480	,	-16000 },
	{846578	,	-15000 },
	{797111	,	-14000 },
	{750834	,	-13000 },
	{707523	,	-12000 },
	{666972	,	-11000 },
	{628988	,	-10000 },
	{593342	,	-9000 },
	{559930	,	-8000 },
	{528601	,	-7000 },
	{499212	,	-6000 },
	{471632	,	-5000 },
	{445771	,	-4000 },
	{421479	,	-3000 },
	{398652	,	-2000 },
	{377192	,	-1000 },
	{357011	,	0  },
	{338005	,	1000 },
	{320121	,	2000 },
	{303286	,	3000 },
	{287433	,	4000 },
	{272499	,	5000 },
	{258426	,	6000 },
	{245159	,	7000 },
	{232649	,	8000 },
	{220847	,	9000 },
	{209709	,	10000 },
	{199196	,	11000 },
	{189268	,	12000 },
	{179889	,	13000 },
	{171027	,	14000 },
	{162650	,	15000 },
	{154726	,	16000 },
	{147232	,	17000 },
	{140142	,	18000 },
	{133432	,	19000 },
	{127080	,	20000 },
	{121065	,	21000 },
	{115368	,	22000},
	{109969	,	23000 },
	{104852	,	24000 },
	{100000	,	25000 },
	{95398	,	26000 },
	{91032	,	27000 },
	{86889	,	28000 },
	{82956	,	29000 },
	{79221	,	30000 },
	{75675	,	31000 },
	{72306	,	32000 },
	{69104	,	33000 },
	{66060	,	34000 },
	{63167	,	35000 },
	{60415	,	36000 },
	{57796	,	37000 },
	{55305	,	38000 },
	{52934	,	39000 },
	{50676	,	40000 },
	{48528	,	41000 },
	{46482	,	42000 },
	{44532	,	43000 },
	{42674	,   	44000 },
	{40903	,   	45000 },
	{39213	,   	46000 },
	{37601  , 	47000 },
	{36062	,   	48000 },
	{34595	,   	49000 },
	{33194	,   	50000 },
	{31859	,   	51000 },
	{30583	,   	52000 },
	{29366  , 	53000 },
	{28202	,   	54000 },
	{27090	,   	55000 },
	{26028	,   	56000 },
	{25012	,   	57000 },
	{24041	,   	58000 },
	{23112	,   	59000 },
	{22224	,   	60000 },
	{21374	,   	61000 },
	{20560	,   	62000 },
	{19782  , 	63000 },
	{19036	,   	64000 },
	{18322	,   	65000 },
	{17640	,   	66000 },
	{16986	,   	67000 },
	{16360  , 	68000 },
	{15759	,   	69000 },
	{15184	,   	70000 },
	{14631  , 	71000 },
	{14100	,   	72000 },
	{13591	,   	73000 },
	{13103	,   	74000 },
	{12635	,   	75000 },
	{12187	,   	76000 },
	{11756	,   	77000 },
	{11343	,  	78000 },
	{10946	,  	79000 },
	{10565	,  	80000 },
	{10199	,   	81000 },
	{9847	,   	82000 },
	{9509	,   	83000 },
	{9184	,   	84000 },
	{8872	,   	85000 },
	{8572	,   	86000 },
	{8283	,   	87000 },
	{8005	,   	88000 },
	{7738	,   	89000 },
	{7481	,   	90000 },
	{7234	,   	91000 },
	{6997	,   	92000 },
	{6768	,   	93000 },
	{6548	,   	94000 },
	{6336	,   	95000 },
	{6131	,   	96000 },
	{5934	,   	97000 },
	{5743	,   	98000 },
	{5560	,   	99000 },
	{5383	,   	100000 },
	{5214	,   	101000 },
	{5050	,   	102000 },
	{4893   ,	103000 },
	{4740	,   	104000 },
	{4594	,   	105000 },
	{4452	,   	106000 },
	{4316	,   	107000 },
	{4184	,   	108000 },
	{4057   ,	109000 },
	{3934	,   	110000 },
	{3815	,   	111000 },
	{3701	,   	112000 },
	{3590	,   	113000 },
	{3483	,   	114000 },
	{3380	,   	115000 },
	{3281	,   	116000 },
	{3185	,   	117000 },
	{3092	,   	118000 },
	{3003	,   	119000 },
	{2916	,   	120000 },
	{2832	,   	121000 },
	{2750	,   	122000 },
	{2672   ,	123000 },
	{2595	,   	124000 },
	{2522	,   	125000 }

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

static int qcom_vadc_scale_hw_calib_volt(
				const struct vadc_prescale_ratio *prescale,
				const struct adc5_data *data,
				u16 adc_code, int *result_uv);
/* Current scaling for PMIC7 */
static int qcom_vadc_scale_hw_calib_current(
				const struct vadc_prescale_ratio *prescale,
				const struct adc5_data *data,
				u16 adc_code, int *result_ua);
/* Raw current for PMIC7 */
static int qcom_vadc_scale_hw_calib_current_raw(
				const struct vadc_prescale_ratio *prescale,
				const struct adc5_data *data,
				u16 adc_code, int *result_ua);
/* Current scaling for PMIC5 */
static int qcom_vadc5_scale_hw_calib_current(
				const struct vadc_prescale_ratio *prescale,
				const struct adc5_data *data,
				u16 adc_code, int *result_ua);
static int qcom_vadc_scale_hw_calib_therm(
				const struct vadc_prescale_ratio *prescale,
				const struct adc5_data *data,
				u16 adc_code, int *result_mdec);
static int qcom_vadc_scale_hw_calib_batt_therm_100(
				const struct vadc_prescale_ratio *prescale,
				const struct adc5_data *data,
				u16 adc_code, int *result_mdec);
static int qcom_vadc_scale_hw_calib_batt_therm_30(
				const struct vadc_prescale_ratio *prescale,
				const struct adc5_data *data,
				u16 adc_code, int *result_mdec);
static int qcom_vadc_scale_hw_calib_batt_therm_400(
				const struct vadc_prescale_ratio *prescale,
				const struct adc5_data *data,
				u16 adc_code, int *result_mdec);
static int qcom_vadc7_scale_hw_calib_therm(
				const struct vadc_prescale_ratio *prescale,
				const struct adc5_data *data,
				u16 adc_code, int *result_mdec);
static int qcom_vadc_scale_hw_smb_temp(
				const struct vadc_prescale_ratio *prescale,
				const struct adc5_data *data,
				u16 adc_code, int *result_mdec);
static int qcom_vadc_scale_hw_pm7_smb_temp(
				const struct vadc_prescale_ratio *prescale,
				const struct adc5_data *data,
				u16 adc_code, int *result_mdec);
static int qcom_vadc_scale_hw_smb1398_temp(
				const struct vadc_prescale_ratio *prescale,
				const struct adc5_data *data,
				u16 adc_code, int *result_mdec);
static int qcom_vadc_scale_hw_pm2250_s3_die_temp(
				const struct vadc_prescale_ratio *prescale,
				const struct adc5_data *data,
				u16 adc_code, int *result_mdec);
static int qcom_vadc_scale_hw_chg5_temp(
				const struct vadc_prescale_ratio *prescale,
				const struct adc5_data *data,
				u16 adc_code, int *result_mdec);
static int qcom_vadc_scale_hw_pm7_chg_temp(
				const struct vadc_prescale_ratio *prescale,
				const struct adc5_data *data,
				u16 adc_code, int *result_mdec);
static int qcom_vadc_scale_hw_calib_die_temp(
				const struct vadc_prescale_ratio *prescale,
				const struct adc5_data *data,
				u16 adc_code, int *result_mdec);
static int qcom_vadc7_scale_hw_calib_die_temp(
				const struct vadc_prescale_ratio *prescale,
				const struct adc5_data *data,
				u16 adc_code, int *result_mdec);
static int qcom_vadc_scale_hw_calib_therm_100_QN5965(
				const struct vadc_prescale_ratio *prescale,
				const struct adc5_data *data,
				u16 adc_code, int *result_mdec);
static int qcom_vadc_scale_hw_calib_batt_therm_100_QN5965(
				const struct vadc_prescale_ratio *prescale,
				const struct adc5_data *data,
				u16 adc_code, int *result_mdec);
/*
static int qcom_vadc_scale_hw_calib_volt_100_QN5965(
				const struct vadc_prescale_ratio *prescale,
				const struct adc5_data *data,
 				u16 adc_code, int *result_uv);
*/

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
	[SCALE_HW_CALIB_PMIC_THERM] = {qcom_vadc_scale_hw_calib_die_temp},
	[SCALE_HW_CALIB_PMIC_THERM_PM7] = {
					qcom_vadc7_scale_hw_calib_die_temp},
	[SCALE_HW_CALIB_PM5_CHG_TEMP] = {qcom_vadc_scale_hw_chg5_temp},
	[SCALE_HW_CALIB_PM5_SMB_TEMP] = {qcom_vadc_scale_hw_smb_temp},
	[SCALE_HW_CALIB_PM5_SMB1398_TEMP] = {qcom_vadc_scale_hw_smb1398_temp},
	[SCALE_HW_CALIB_PM2250_S3_DIE_TEMP] = {qcom_vadc_scale_hw_pm2250_s3_die_temp},
	[SCALE_HW_CALIB_PM7_SMB_TEMP] = {qcom_vadc_scale_hw_pm7_smb_temp},
	[SCALE_HW_CALIB_PM7_CHG_TEMP] = {qcom_vadc_scale_hw_pm7_chg_temp},
	[SCALE_HW_CALIB_THERM_100K_QN5965] = {qcom_vadc_scale_hw_calib_therm_100_QN5965},
	[SCALE_HW_CALIB_BATT_THERM_100K_QN5965] = {qcom_vadc_scale_hw_calib_batt_therm_100_QN5965},
};

static int qcom_vadc_map_voltage_temp(const struct vadc_map_pt *pts,
				      u32 tablesize, s32 input, int *output)
{
	bool descending = 1;
	u32 i = 0;

	if (!pts)
		return -EINVAL;

	/* Check if table is descending or ascending */
	if (tablesize > 1) {
		if (pts[0].x < pts[1].x)
			descending = 0;
	}

	while (i < tablesize) {
		if ((descending) && (pts[i].x < input)) {
			/* table entry is less than measured*/
			 /* value and table is descending, stop */
			break;
		} else if ((!descending) &&
				(pts[i].x > input)) {
			/* table entry is greater than measured*/
			/*value and table is ascending, stop */
			break;
		}
		i++;
	}

	if (i == 0) {
		*output = pts[0].y;
	} else if (i == tablesize) {
		*output = pts[tablesize - 1].y;
	} else {
		/* result is between search_index and search_index-1 */
		/* interpolate linearly */
		*output = (((s32)((pts[i].y - pts[i - 1].y) *
			(input - pts[i - 1].x)) /
			(pts[i].x - pts[i - 1].x)) +
			pts[i - 1].y);
	}

	return 0;
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
				const struct vadc_prescale_ratio *prescale,
				bool absolute, u16 adc_code,
				int *result_uv)
{
	s64 voltage = 0, result = 0;

	qcom_vadc_scale_calib(calib_graph, adc_code, absolute, &voltage);

	voltage = voltage * prescale->den;
	result = div64_s64(voltage, prescale->num);
	*result_uv = result;

	return 0;
}

static int qcom_vadc_scale_therm(const struct vadc_linear_graph *calib_graph,
				 const struct vadc_prescale_ratio *prescale,
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

	*result_mdec *= 1000;

	return 0;
}

static int qcom_vadc_scale_die_temp(const struct vadc_linear_graph *calib_graph,
				    const struct vadc_prescale_ratio *prescale,
				    bool absolute,
				    u16 adc_code, int *result_mdec)
{
	s64 voltage = 0;
	u64 temp; /* Temporary variable for do_div */

	qcom_vadc_scale_calib(calib_graph, adc_code, absolute, &voltage);

	if (voltage > 0) {
		temp = voltage * prescale->den;
		do_div(temp, prescale->num * 2);
		voltage = temp;
	} else {
		voltage = 0;
	}

	voltage -= KELVINMIL_CELSIUSMIL;
	*result_mdec = voltage;

	return 0;
}

static int qcom_vadc_scale_chg_temp(const struct vadc_linear_graph *calib_graph,
				    const struct vadc_prescale_ratio *prescale,
				    bool absolute,
				    u16 adc_code, int *result_mdec)
{
	s64 voltage = 0, result = 0;

	qcom_vadc_scale_calib(calib_graph, adc_code, absolute, &voltage);

	voltage = voltage * prescale->den;
	voltage = div64_s64(voltage, prescale->num);
	voltage = ((PMI_CHG_SCALE_1) * (voltage * 2));
	voltage = (voltage + PMI_CHG_SCALE_2);
	result =  div64_s64(voltage, 1000000);
	*result_mdec = result;

	return 0;
}

static int qcom_vadc_scale_code_voltage_factor(u16 adc_code,
				const struct vadc_prescale_ratio *prescale,
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
		voltage *= prescale->den;
		temp = prescale->num * factor;
		voltage = div64_s64(voltage, temp);
	} else {
		voltage = 0;
	}

	return (int) voltage;
}

static int qcom_vadc7_scale_hw_calib_therm(
				const struct vadc_prescale_ratio *prescale,
				const struct adc5_data *data,
				u16 adc_code, int *result_mdec)
{
	s64 resistance = 0;
	int ret, result = 0;

	if (adc_code >= RATIO_MAX_ADC7)
		return -EINVAL;

	/* (ADC code * R_PULLUP (100Kohm)) / (full_scale_code - ADC code)*/
	resistance = (s64) adc_code * R_PU_100K;
	resistance = div64_s64(resistance, (RATIO_MAX_ADC7 - adc_code));

	ret = qcom_vadc_map_voltage_temp(adcmap7_100k,
				 ARRAY_SIZE(adcmap7_100k),
				 resistance, &result);
	if (ret)
		return ret;

	*result_mdec = result;

	return 0;
}

static int qcom_vadc_scale_hw_calib_current_raw(
				const struct vadc_prescale_ratio *prescale,
				const struct adc5_data *data,
				u16 adc_code, int *result_ua)
{
	s64 temp;

	if (!prescale->num)
		return -EINVAL;

	temp = div_s64((s64)(s16)adc_code * prescale->den, prescale->num);
	*result_ua = (int) temp;
	pr_debug("raw adc_code: %#x result_ua: %d\n", adc_code, *result_ua);

	return 0;
}

static int qcom_vadc_scale_hw_calib_current(
				const struct vadc_prescale_ratio *prescale,
				const struct adc5_data *data,
				u16 adc_code, int *result_ua)
{
	u32 adc_vdd_ref_mv = 1875;
	s64 voltage;

	if (!prescale->num)
		return -EINVAL;

	/* (ADC code * vref_vadc (1.875V)) / full_scale_code */
	voltage = (s64)(s16) adc_code * adc_vdd_ref_mv * 1000;
	voltage = div_s64(voltage, data->full_scale_code_volt);
	voltage = div_s64(voltage * prescale->den, prescale->num);
	*result_ua = (int) voltage;
	pr_debug("adc_code: %#x result_ua: %d\n", adc_code, *result_ua);

	return 0;
}

static int qcom_vadc5_scale_hw_calib_current(
				const struct vadc_prescale_ratio *prescale,
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
	result = div64_s64(voltage * prescale->den, prescale->num);
	*result_ua = result;

	if (!positive)
		*result_ua = -result;

	return 0;
}

static int qcom_vadc_scale_hw_calib_volt(
				const struct vadc_prescale_ratio *prescale,
				const struct adc5_data *data,
				u16 adc_code, int *result_uv)
{
	*result_uv = qcom_vadc_scale_code_voltage_factor(adc_code,
				prescale, data, 1);

	return 0;
}

static int qcom_vadc_scale_hw_calib_therm(
				const struct vadc_prescale_ratio *prescale,
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
				const struct vadc_prescale_ratio *prescale,
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

/*  add adc conversion begin */

static int qcom_vadc_scale_hw_calib_batt_therm_100_QN5965(
				const struct vadc_prescale_ratio *prescale,
				const struct adc5_data *data,
				u16 adc_code, int *result_mdec)
{
	int voltage;

	voltage = qcom_vadc_scale_code_voltage_factor(adc_code,
				prescale, data, 1000);

	/* Map voltage to temperature from look-up table */
	return qcom_vadc_map_voltage_temp(adcmap_voltage_to_therm_100k_QN5965,
				 ARRAY_SIZE(adcmap_voltage_to_therm_100k_QN5965),
				 voltage, result_mdec);
}

static int qcom_vadc_scale_hw_calib_therm_100_QN5965(
				const struct vadc_prescale_ratio *prescale,
				const struct adc5_data *data,
				u16 adc_code, int *result_mdec)
{	
	s64 resistance = 0;
	int ret, result = 0;

	if (adc_code >= RATIO_MAX_ADC7)
	return -EINVAL;

	/* (ADC code * R_PULLUP (100Kohm)) / (full_scale_code - ADC code)*/
	resistance = (s64) adc_code * R_PU_100K;
	resistance = div64_s64(resistance, (RATIO_MAX_ADC7 - adc_code));

	ret = qcom_vadc_map_voltage_temp(adcmap7_res_to_temp_100k_QN5965,
				ARRAY_SIZE(adcmap7_res_to_temp_100k_QN5965),
				resistance, &result);

	if (ret)
		return ret;

	*result_mdec = result;

	return 0;	
}

/*  add adc conversion end */

static int qcom_vadc_scale_hw_calib_batt_therm_30(
				const struct vadc_prescale_ratio *prescale,
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
				const struct vadc_prescale_ratio *prescale,
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
				const struct vadc_prescale_ratio *prescale,
				const struct adc5_data *data,
				u16 adc_code, int *result_mdec)
{
	*result_mdec = qcom_vadc_scale_code_voltage_factor(adc_code,
				prescale, data, 2);
	*result_mdec -= KELVINMIL_CELSIUSMIL;

	return 0;
}

static int qcom_vadc7_scale_hw_calib_die_temp(
				const struct vadc_prescale_ratio *prescale,
				const struct adc5_data *data,
				u16 adc_code, int *result_mdec)
{

	int voltage, vtemp0, temp, i = 0;

	voltage = qcom_vadc_scale_code_voltage_factor(adc_code,
				prescale, data, 1);

	while (i < ARRAY_SIZE(adcmap7_die_temp)) {
		if (adcmap7_die_temp[i].x > voltage)
			break;
		i++;
	}

	if (i == 0) {
		*result_mdec = DIE_TEMP_ADC7_SCALE_1;
	} else if (i == ARRAY_SIZE(adcmap7_die_temp)) {
		*result_mdec = DIE_TEMP_ADC7_MAX;
	} else {
		vtemp0 = adcmap7_die_temp[i-1].x;
		voltage = voltage - vtemp0;
		temp = div64_s64(voltage * DIE_TEMP_ADC7_SCALE_FACTOR,
				adcmap7_die_temp[i-1].y);
		temp += DIE_TEMP_ADC7_SCALE_1 + (DIE_TEMP_ADC7_SCALE_2 * (i-1));
		*result_mdec = temp;
	}

	return 0;
}

static int qcom_vadc_scale_hw_pm7_chg_temp(
				const struct vadc_prescale_ratio *prescale,
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
				const struct vadc_prescale_ratio *prescale,
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
				const struct vadc_prescale_ratio *prescale,
				const struct adc5_data *data,
				u16 adc_code, int *result_mdec)
{
	*result_mdec = qcom_vadc_scale_code_voltage_factor(adc_code * 100,
				prescale, data, PMIC5_SMB_TEMP_SCALE_FACTOR);
	*result_mdec = PMIC5_SMB_TEMP_CONSTANT - *result_mdec;

	return 0;
}

static int qcom_vadc_scale_hw_smb1398_temp(
				const struct vadc_prescale_ratio *prescale,
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
		temp = voltage * prescale->den;
		temp *= 100;
		do_div(temp, prescale->num * PMIC5_SMB1398_TEMP_SCALE_FACTOR);
		voltage = temp;
	} else {
		voltage = 0;
	}

	voltage = voltage - PMIC5_SMB1398_TEMP_CONSTANT;
	*result_mdec = voltage;

	return 0;
}

static int qcom_vadc_scale_hw_pm2250_s3_die_temp(
				const struct vadc_prescale_ratio *prescale,
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
		voltage *= prescale->den;
		voltage = div64_s64(voltage, prescale->num);
	} else {
		voltage = 0;
	}

	voltage = PMIC5_PM2250_S3_DIE_TEMP_CONSTANT - voltage;
	voltage *= 100000;
	voltage = div64_s64(voltage, PMIC5_PM2250_S3_DIE_TEMP_SCALE_FACTOR);

	*result_mdec = voltage;

	return 0;
}

static int qcom_vadc_scale_hw_chg5_temp(
				const struct vadc_prescale_ratio *prescale,
				const struct adc5_data *data,
				u16 adc_code, int *result_mdec)
{
	*result_mdec = qcom_vadc_scale_code_voltage_factor(adc_code,
				prescale, data, 4);
	*result_mdec = PMIC5_CHG_TEMP_SCALE_FACTOR - *result_mdec;

	return 0;
}

int qcom_vadc_scale(enum vadc_scale_fn_type scaletype,
		    const struct vadc_linear_graph *calib_graph,
		    const struct vadc_prescale_ratio *prescale,
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

int qcom_adc5_hw_scale(enum vadc_scale_fn_type scaletype,
		    const struct vadc_prescale_ratio *prescale,
		    const struct adc5_data *data,
		    u16 adc_code, int *result)
{
	if (!(scaletype >= SCALE_HW_CALIB_DEFAULT &&
		scaletype < SCALE_HW_CALIB_INVALID)) {
		pr_err("Invalid scale type %d\n", scaletype);
		return -EINVAL;
	}

	return scale_adc5_fn[scaletype].scale_fn(prescale, data,
					adc_code, result);
}
EXPORT_SYMBOL(qcom_adc5_hw_scale);

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
