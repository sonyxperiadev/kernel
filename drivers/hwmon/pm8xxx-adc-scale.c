/*
 * Copyright (c) 2011-2012, The Linux Foundation. All rights reserved.
 * Copyright (C) 2012-2013 Sony Mobile Communications AB.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/module.h>
#include <linux/mfd/pm8xxx/pm8xxx-adc.h>
#define KELVINMIL_DEGMIL	273160
#define PM8xxx_ADC_BL_THERM_INVALID_TEMP	0xFFFFFFFF

/* Units for temperature below (on x axis) is in 0.1DegC as
   required by the battery driver. Note the resolution used
   here to compute the table was done for DegC to milli-volts.
   In consideration to limit the size of the table for the given
   temperature range below, the result is linearly interpolated
   and provided to the battery driver in the units desired for
   their framework which is 0.1DegC. True resolution of 0.1DegC
   will result in the below table size to increase by 10 times */
static struct pm8xxx_adc_map_pt adcmap_btm_threshold[] = {
	{-300,	1377},
	{-290,	1375},
	{-280,	1372},
	{-270,	1370},
	{-260,	1367},
	{-250,	1364},
	{-240,	1361},
	{-230,	1357},
	{-220,	1354},
	{-210,	1350},
	{-200,	1347},
	{-190,	1343},
	{-180,	1339},
	{-170,	1334},
	{-160,	1330},
	{-150,	1325},
	{-140,	1320},
	{-130,	1315},
	{-120,	1310},
	{-110,	1305},
	{-100,	1299},
	{-90,	1294},
	{-80,	1288},
	{-70,	1282},
	{-60,	1275},
	{-50,	1269},
	{-40,	1262},
	{-30,	1255},
	{-20,	1248},
	{-10,	1241},
	{0,	1233},
	{10,	1226},
	{20,	1218},
	{30,	1210},
	{40,	1202},
	{50,	1193},
	{60,	1185},
	{70,	1176},
	{80,	1167},
	{90,	1158},
	{100,	1149},
	{110,	1139},
	{120,	1130},
	{130,	1120},
	{140,	1111},
	{150,	1101},
	{160,	1091},
	{170,	1081},
	{180,	1071},
	{190,	1061},
	{200,	1050},
	{210,	1040},
	{220,	1030},
	{230,	1019},
	{240,	1009},
	{250,	999},
	{260,	988},
	{270,	978},
	{280,	968},
	{290,	957},
	{300,	947},
	{310,	937},
	{320,	927},
	{330,	916},
	{340,	906},
	{350,	896},
	{360,	886},
	{370,	877},
	{380,	867},
	{390,	857},
	{400,	848},
	{410,	839},
	{420,	829},
	{430,	820},
	{440,	811},
	{450,	802},
	{460,	794},
	{470,	785},
	{480,	777},
	{490,	768},
	{500,	760},
	{510,	752},
	{520,	745},
	{530,	737},
	{540,	729},
	{550,	722},
	{560,	715},
	{570,	708},
	{580,	701},
	{590,	694},
	{600,	688},
	{610,	681},
	{620,	675},
	{630,	669},
	{640,	663},
	{650,	657},
	{660,	651},
	{670,	646},
	{680,	640},
	{690,	635},
	{700,	630},
	{710,	625},
	{720,	620},
	{730,	616},
	{740,	611},
	{750,	606},
	{760,	602},
	{770,	598},
	{780,	594},
	{790,	590},
};

static const struct pm8xxx_adc_map_pt adcmap_pa_therm[] = {
	{1731,	-30},
	{1726,	-29},
	{1721,	-28},
	{1715,	-27},
	{1710,	-26},
	{1703,	-25},
	{1697,	-24},
	{1690,	-23},
	{1683,	-22},
	{1675,	-21},
	{1667,	-20},
	{1659,	-19},
	{1650,	-18},
	{1641,	-17},
	{1632,	-16},
	{1622,	-15},
	{1611,	-14},
	{1600,	-13},
	{1589,	-12},
	{1577,	-11},
	{1565,	-10},
	{1552,	-9},
	{1539,	-8},
	{1525,	-7},
	{1511,	-6},
	{1496,	-5},
	{1481,	-4},
	{1465,	-3},
	{1449,	-2},
	{1432,	-1},
	{1415,	0},
	{1398,	1},
	{1380,	2},
	{1362,	3},
	{1343,	4},
	{1324,	5},
	{1305,	6},
	{1285,	7},
	{1265,	8},
	{1245,	9},
	{1224,	10},
	{1203,	11},
	{1182,	12},
	{1161,	13},
	{1139,	14},
	{1118,	15},
	{1096,	16},
	{1074,	17},
	{1052,	18},
	{1030,	19},
	{1008,	20},
	{986,	21},
	{964,	22},
	{943,	23},
	{921,	24},
	{899,	25},
	{878,	26},
	{857,	27},
	{836,	28},
	{815,	29},
	{794,	30},
	{774,	31},
	{754,	32},
	{734,	33},
	{714,	34},
	{695,	35},
	{676,	36},
	{657,	37},
	{639,	38},
	{621,	39},
	{604,	40},
	{586,	41},
	{570,	42},
	{553,	43},
	{537,	44},
	{521,	45},
	{506,	46},
	{491,	47},
	{476,	48},
	{462,	49},
	{448,	50},
	{435,	51},
	{421,	52},
	{409,	53},
	{396,	54},
	{384,	55},
	{372,	56},
	{361,	57},
	{350,	58},
	{339,	59},
	{329,	60},
	{318,	61},
	{309,	62},
	{299,	63},
	{290,	64},
	{281,	65},
	{272,	66},
	{264,	67},
	{256,	68},
	{248,	69},
	{240,	70},
	{233,	71},
	{226,	72},
	{219,	73},
	{212,	74},
	{206,	75},
	{199,	76},
	{193,	77},
	{187,	78},
	{182,	79},
	{176,	80},
	{171,	81},
	{166,	82},
	{161,	83},
	{156,	84},
	{151,	85},
	{147,	86},
	{142,	87},
	{138,	88},
	{134,	89},
	{130,	90},
	{126,	91},
	{122,	92},
	{119,	93},
	{115,	94},
	{112,	95},
	{109,	96},
	{106,	97},
	{103,	98},
	{100,	99},
	{97,	100},
	{94,	101},
	{91,	102},
	{89,	103},
	{86,	104},
	{84,	105},
	{82,	106},
	{79,	107},
	{77,	108},
	{75,	109},
	{73,	110},
	{71,	111},
	{69,	112},
	{67,	113},
	{65,	114},
	{64,	115},
	{62,	116},
	{60,	117},
	{59,	118},
	{57,	119},
	{56,	120},
	{54,	121},
	{53,	122},
	{51,	123},
	{50,	124},
	{49,	125}
};

static const struct pm8xxx_adc_map_pt adcmap_ntcg_104ef_104fb[] = {
	{696483,	-40960},
	{649148,	-39936},
	{605368,	-38912},
	{564809,	-37888},
	{527215,	-36864},
	{492322,	-35840},
	{460007,	-34816},
	{429982,	-33792},
	{402099,	-32768},
	{376192,	-31744},
	{352075,	-30720},
	{329714,	-29696},
	{308876,	-28672},
	{289480,	-27648},
	{271417,	-26624},
	{254574,	-25600},
	{238903,	-24576},
	{224276,	-23552},
	{210631,	-22528},
	{197896,	-21504},
	{186007,	-20480},
	{174899,	-19456},
	{164521,	-18432},
	{154818,	-17408},
	{145744,	-16384},
	{137265,	-15360},
	{129307,	-14336},
	{121866,	-13312},
	{114896,	-12288},
	{108365,	-11264},
	{102252,	-10240},
	{96499,		-9216},
	{91111,		-8192},
	{86055,		-7168},
	{81308,		-6144},
	{76857,		-5120},
	{72660,		-4096},
	{68722,		-3072},
	{65020,		-2048},
	{61538,		-1024},
	{58261,		0},
	{55177,		1024},
	{52274,		2048},
	{49538,		3072},
	{46962,		4096},
	{44531,		5120},
	{42243,		6144},
	{40083,		7168},
	{38045,		8192},
	{36122,		9216},
	{34308,		10240},
	{32592,		11264},
	{30972,		12288},
	{29442,		13312},
	{27995,		14336},
	{26624,		15360},
	{25333,		16384},
	{24109,		17408},
	{22951,		18432},
	{21854,		19456},
	{20807,		20480},
	{19831,		21504},
	{18899,		22528},
	{18016,		23552},
	{17178,		24576},
	{16384,		25600},
	{15631,		26624},
	{14916,		27648},
	{14237,		28672},
	{13593,		29696},
	{12976,		30720},
	{12400,		31744},
	{11848,		32768},
	{11324,		33792},
	{10825,		34816},
	{10354,		35840},
	{9900,		36864},
	{9471,		37888},
	{9062,		38912},
	{8674,		39936},
	{8306,		40960},
	{7951,		41984},
	{7616,		43008},
	{7296,		44032},
	{6991,		45056},
	{6701,		46080},
	{6424,		47104},
	{6160,		48128},
	{5908,		49152},
	{5667,		50176},
	{5439,		51200},
	{5219,		52224},
	{5010,		53248},
	{4810,		54272},
	{4619,		55296},
	{4440,		56320},
	{4263,		57344},
	{4097,		58368},
	{3938,		59392},
	{3785,		60416},
	{3637,		61440},
	{3501,		62464},
	{3368,		63488},
	{3240,		64512},
	{3118,		65536},
	{2998,		66560},
	{2889,		67584},
	{2782,		68608},
	{2680,		69632},
	{2581,		70656},
	{2490,		71680},
	{2397,		72704},
	{2310,		73728},
	{2227,		74752},
	{2147,		75776},
	{2064,		76800},
	{1998,		77824},
	{1927,		78848},
	{1860,		79872},
	{1795,		80896},
	{1736,		81920},
	{1673,		82944},
	{1615,		83968},
	{1560,		84992},
	{1507,		86016},
	{1456,		87040},
	{1407,		88064},
	{1360,		89088},
	{1314,		90112},
	{1271,		91136},
	{1228,		92160},
	{1189,		93184},
	{1150,		94208},
	{1112,		95232},
	{1076,		96256},
	{1042,		97280},
	{1008,		98304},
	{976,		99328},
	{945,		100352},
	{915,		101376},
	{886,		102400},
	{859,		103424},
	{832,		104448},
	{807,		105472},
	{782,		106496},
	{756,		107520},
	{735,		108544},
	{712,		109568},
	{691,		110592},
	{670,		111616},
	{650,		112640},
	{631,		113664},
	{612,		114688},
	{594,		115712},
	{577,		116736},
	{560,		117760},
	{544,		118784},
	{528,		119808},
	{513,		120832},
	{498,		121856},
	{483,		122880},
	{470,		123904},
	{457,		124928},
	{444,		125952},
	{431,		126976},
	{419,		128000}
};

static const struct pm8xxx_adc_map_pt adcmap_pba_therm[] = {
	{1731,	-30},
	{1726,	-29},
	{1721,	-28},
	{1716,	-27},
	{1710,	-26},
	{1704,	-25},
	{1697,	-24},
	{1691,	-23},
	{1683,	-22},
	{1676,	-21},
	{1668,	-20},
	{1660,	-19},
	{1651,	-18},
	{1642,	-17},
	{1632,	-16},
	{1622,	-15},
	{1612,	-14},
	{1601,	-13},
	{1590,	-12},
	{1578,	-11},
	{1565,	-10},
	{1553,	-9},
	{1539,	-8},
	{1526,	-7},
	{1511,	-6},
	{1497,	-5},
	{1482,	-4},
	{1466,	-3},
	{1450,	-2},
	{1433,	-1},
	{1416,	0},
	{1399,	1},
	{1381,	2},
	{1363,	3},
	{1344,	4},
	{1325,	5},
	{1306,	6},
	{1286,	7},
	{1266,	8},
	{1245,	9},
	{1225,	10},
	{1204,	11},
	{1183,	12},
	{1162,	13},
	{1140,	14},
	{1118,	15},
	{1097,	16},
	{1075,	17},
	{1053,	18},
	{1031,	19},
	{1009,	20},
	{987,	21},
	{965,	22},
	{943,	23},
	{922,	24},
	{900,	25},
	{879,	26},
	{857,	27},
	{836,	28},
	{815,	29},
	{795,	30},
	{774,	31},
	{754,	32},
	{734,	33},
	{715,	34},
	{695,	35},
	{676,	36},
	{658,	37},
	{639,	38},
	{621,	39},
	{604,	40},
	{587,	41},
	{570,	42},
	{553,	43},
	{537,	44},
	{521,	45},
	{506,	46},
	{491,	47},
	{476,	48},
	{462,	49},
	{448,	50},
	{435,	51},
	{422,	52},
	{409,	53},
	{396,	54},
	{384,	55},
	{372,	56},
	{361,	57},
	{350,	58},
	{339,	59},
	{329,	60},
	{318,	61},
	{309,	62},
	{299,	63},
	{290,	64},
	{281,	65},
	{272,	66},
	{264,	67},
	{256,	68},
	{248,	69},
	{240,	70},
	{233,	71},
	{226,	72},
	{219,	73},
	{212,	74},
	{205,	75},
	{199,	76},
	{193,	77},
	{187,	78},
	{182,	79},
	{176,	80},
	{171,	81},
	{166,	82},
	{161,	83},
	{156,	84},
	{151,	85},
	{147,	86},
	{142,	87},
	{138,	88},
	{134,	89},
	{130,	90},
	{126,	91},
	{122,	92},
	{119,	93},
	{115,	94},
	{112,	95},
	{109,	96},
	{106,	97},
	{103,	98},
	{100,	99},
	{97,	100},
	{94,	101},
	{91,	102},
	{89,	103},
	{86,	104},
	{84,	105},
	{82,	106},
	{79,	107},
	{77,	108},
	{75,	109},
	{73,	110},
	{71,	111},
	{69,	112},
	{67,	113},
	{65,	114},
	{64,	115},
	{62,	116},
	{60,	117},
	{59,	118},
	{57,	119},
	{56,	120},
	{54,	121},
	{53,	122},
	{51,	123},
	{50,	124},
	{49,	125}
};

#if defined(CONFIG_SENSORS_PM8XXX_PULLUP_100K_BLTHERM)
static const struct pm8xxx_adc_map_pt adcmap_bl_therm[] = {
	{1731,	-30},
	{1726,	-29},
	{1721,	-28},
	{1716,	-27},
	{1710,	-26},
	{1704,	-25},
	{1697,	-24},
	{1691,	-23},
	{1683,	-22},
	{1676,	-21},
	{1668,	-20},
	{1660,	-19},
	{1651,	-18},
	{1642,	-17},
	{1632,	-16},
	{1622,	-15},
	{1612,	-14},
	{1601,	-13},
	{1590,	-12},
	{1578,	-11},
	{1565,	-10},
	{1553,	-9},
	{1539,	-8},
	{1526,	-7},
	{1511,	-6},
	{1497,	-5},
	{1482,	-4},
	{1466,	-3},
	{1450,	-2},
	{1433,	-1},
	{1416,	0},
	{1399,	1},
	{1381,	2},
	{1363,	3},
	{1344,	4},
	{1325,	5},
	{1306,	6},
	{1286,	7},
	{1266,	8},
	{1245,	9},
	{1225,	10},
	{1204,	11},
	{1183,	12},
	{1162,	13},
	{1140,	14},
	{1118,	15},
	{1097,	16},
	{1075,	17},
	{1053,	18},
	{1031,	19},
	{1009,	20},
	{987,	21},
	{965,	22},
	{943,	23},
	{922,	24},
	{900,	25},
	{879,	26},
	{857,	27},
	{836,	28},
	{815,	29},
	{795,	30},
	{774,	31},
	{754,	32},
	{734,	33},
	{715,	34},
	{695,	35},
	{676,	36},
	{658,	37},
	{639,	38},
	{621,	39},
	{604,	40},
	{587,	41},
	{570,	42},
	{553,	43},
	{537,	44},
	{521,	45},
	{506,	46},
	{491,	47},
	{476,	48},
	{462,	49},
	{448,	50},
	{435,	51},
	{422,	52},
	{409,	53},
	{396,	54},
	{384,	55},
	{372,	56},
	{361,	57},
	{350,	58},
	{339,	59},
	{329,	60},
	{318,	61},
	{309,	62},
	{299,	63},
	{290,	64},
	{281,	65},
	{272,	66},
	{264,	67},
	{256,	68},
	{248,	69},
	{240,	70},
	{233,	71},
	{226,	72},
	{219,	73},
	{212,	74},
	{205,	75},
	{199,	76},
	{193,	77},
	{187,	78},
	{182,	79},
	{176,	80},
	{171,	81},
	{166,	82},
	{161,	83},
	{156,	84},
	{151,	85},
	{147,	86},
	{142,	87},
	{138,	88},
	{134,	89},
	{130,	90},
	{126,	91},
	{122,	92},
	{119,	93},
	{115,	94},
	{112,	95},
	{109,	96},
	{106,	97},
	{103,	98},
	{100,	99},
	{97,	100},
	{94,	101},
	{91,	102},
	{89,	103},
	{86,	104},
	{84,	105},
	{82,	106},
	{79,	107},
	{77,	108},
	{75,	109},
	{73,	110},
	{71,	111},
	{69,	112},
	{67,	113},
	{65,	114},
	{64,	115},
	{62,	116},
	{60,	117},
	{59,	118},
	{57,	119},
	{56,	120},
	{54,	121},
	{53,	122},
	{51,	123},
	{50,	124},
	{49,	125}
};
#elif defined(CONFIG_SENSORS_PM8XXX_PULLUP_68K_BLTHERM)
static const struct pm8xxx_adc_map_pt adcmap_bl_therm[] = {
	{1746,	-30},
	{1742,	-29},
	{1739,	-28},
	{1735,	-27},
	{1730,	-26},
	{1726,	-25},
	{1721,	-24},
	{1716,	-23},
	{1711,	-22},
	{1705,	-21},
	{1700,	-20},
	{1694,	-19},
	{1687,	-18},
	{1680,	-17},
	{1673,	-16},
	{1666,	-15},
	{1659,	-14},
	{1651,	-13},
	{1642,	-12},
	{1633,	-11},
	{1624,	-10},
	{1615,	-9},
	{1605,	-8},
	{1595,	-7},
	{1584,	-6},
	{1573,	-5},
	{1562,	-4},
	{1550,	-3},
	{1538,	-2},
	{1525,	-1},
	{1512,	0},
	{1499,	1},
	{1485,	2},
	{1470,	3},
	{1456,	4},
	{1441,	5},
	{1425,	6},
	{1409,	7},
	{1393,	8},
	{1376,	9},
	{1359,	10},
	{1342,	11},
	{1324,	12},
	{1306,	13},
	{1288,	14},
	{1269,	15},
	{1250,	16},
	{1231,	17},
	{1212,	18},
	{1192,	19},
	{1173,	20},
	{1153,	21},
	{1132,	22},
	{1112,	23},
	{1092,	24},
	{1071,	25},
	{1051,	26},
	{1030,	27},
	{1010,	28},
	{989,	29},
	{969,	30},
	{948,	31},
	{928,	32},
	{907,	33},
	{887,	34},
	{867,	35},
	{847,	36},
	{827,	37},
	{807,	38},
	{788,	39},
	{769,	40},
	{750,	41},
	{731,	42},
	{712,	43},
	{694,	44},
	{676,	45},
	{658,	46},
	{641,	47},
	{624,	48},
	{607,	49},
	{590,	50},
	{574,	51},
	{558,	52},
	{543,	53},
	{528,	54},
	{513,	55},
	{498,	56},
	{484,	57},
	{470,	58},
	{457,	59},
	{443,	60},
	{430,	61},
	{418,	62},
	{406,	63},
	{394,	64},
	{382,	65},
	{371,	66},
	{360,	67},
	{349,	68},
	{339,	69},
	{329,	70},
	{319,	71},
	{309,	72},
	{300,	73},
	{291,	74},
	{282,	75},
	{274,	76},
	{265,	77},
	{257,	78},
	{250,	79},
	{242,	80},
	{235,	81},
	{228,	82},
	{221,	83},
	{214,	84},
	{208,	85},
	{202,	86},
	{195,	87},
	{190,	88},
	{184,	89},
	{178,	90},
	{173,	91},
	{168,	92},
	{163,	93},
	{158,	94},
	{153,	95},
	{149,	96},
	{144,	97},
	{140,	98},
	{136,	99},
	{132,	100},
	{128,	101},
	{124,	102},
	{121,	103},
	{117,	104},
	{114,	105},
	{111,	106},
	{107,	107},
	{104,	108},
	{101,	109},
	{98,	110},
	{96,	111},
	{93,	112},
	{90,	113},
	{88,	114},
	{85,	115},
	{83,	116},
	{81,	117},
	{78,	118},
	{76,	119},
	{74,	120},
	{72,	121},
	{70,	122},
	{68,	123},
	{66,	124},
	{64,	125}
};
#else
static const struct pm8xxx_adc_map_pt adcmap_bl_therm[] = {
	{1731,	-30},
	{1726,	-29},
	{1721,	-28},
	{1715,	-27},
	{1710,	-26},
	{1703,	-25},
	{1697,	-24},
	{1690,	-23},
	{1683,	-22},
	{1675,	-21},
	{1667,	-20},
	{1659,	-19},
	{1650,	-18},
	{1641,	-17},
	{1632,	-16},
	{1622,	-15},
	{1611,	-14},
	{1600,	-13},
	{1589,	-12},
	{1577,	-11},
	{1565,	-10},
	{1552,	-9},
	{1539,	-8},
	{1525,	-7},
	{1511,	-6},
	{1496,	-5},
	{1481,	-4},
	{1465,	-3},
	{1449,	-2},
	{1432,	-1},
	{1415,	0},
	{1398,	1},
	{1380,	2},
	{1362,	3},
	{1343,	4},
	{1324,	5},
	{1305,	6},
	{1285,	7},
	{1265,	8},
	{1245,	9},
	{1224,	10},
	{1203,	11},
	{1182,	12},
	{1161,	13},
	{1139,	14},
	{1118,	15},
	{1096,	16},
	{1074,	17},
	{1052,	18},
	{1030,	19},
	{1008,	20},
	{986,	21},
	{964,	22},
	{943,	23},
	{921,	24},
	{900,	25},
	{878,	26},
	{856,	27},
	{834,	28},
	{812,	29},
	{790,	30},
	{769,	31},
	{748,	32},
	{727,	33},
	{706,	34},
	{686,	35},
	{666,	36},
	{647,	37},
	{628,	38},
	{609,	39},
	{591,	40},
	{573,	41},
	{555,	42},
	{538,	43},
	{521,	44},
	{504,	45},
	{488,	46},
	{473,	47},
	{458,	48},
	{443,	49},
	{428,	50},
	{414,	51},
	{401,	52},
	{388,	53},
	{375,	54},
	{362,	55},
	{350,	56},
	{339,	57},
	{327,	58},
	{316,	59},
	{306,	60},
	{295,	61},
	{285,	62},
	{276,	63},
	{266,	64},
	{257,	65},
	{249,	66},
	{240,	67},
	{232,	68},
	{224,	69},
	{217,	70},
	{209,	71},
	{202,	72},
	{195,	73},
	{189,	74},
	{182,	75},
	{176,	76},
	{170,	77},
	{164,	78},
	{159,	79},
	{154,	80},
	{148,	81},
	{143,	82},
	{139,	83},
	{134,	84},
	{130,	85},
	{125,	86},
	{121,	87},
	{117,	88},
	{113,	89},
	{110,	90},
	{106,	91},
	{102,	92},
	{99,	93},
	{96,	94},
	{93,	95},
	{90,	96},
	{87,	97},
	{84,	98},
	{81,	99},
	{79,	100},
	{76,	101},
	{74,	102},
	{72,	103},
	{69,	104},
	{67,	105},
	{65,	106},
	{63,	107},
	{61,	108},
	{59,	109},
	{57,	110},
	{56,	111},
	{54,	112},
	{52,	113},
	{51,	114},
	{49,	115},
	{48,	116},
	{46,	117},
	{45,	118},
	{43,	119},
	{42,	120},
	{41,	121},
	{40,	122},
	{39,	123},
	{37,	124},
	{36,	125}
};
#endif

static int32_t pm8xxx_adc_map_linear(const struct pm8xxx_adc_map_pt *pts,
		uint32_t tablesize, int32_t input, int64_t *output)
{
	bool descending = 1;
	uint32_t i = 0;

	if ((pts == NULL) || (output == NULL))
		return -EINVAL;

	/* Check if table is descending or ascending */
	if (tablesize > 1) {
		if (pts[0].x < pts[1].x)
			descending = 0;
	}

	while (i < tablesize) {
		if ((descending == 1) && (pts[i].x < input)) {
			/* table entry is less than measured
				value and table is descending, stop */
			break;
		} else if ((descending == 0) &&
				(pts[i].x > input)) {
			/* table entry is greater than measured
				value and table is ascending, stop */
			break;
		} else {
			i++;
		}
	}

	if (i == 0)
		*output = pts[0].y;
	else if (i == tablesize)
		*output = pts[tablesize-1].y;
	else {
		/* result is between search_index and search_index-1 */
		/* interpolate linearly */
		*output = (((int32_t) ((pts[i].y - pts[i-1].y)*
			(input - pts[i-1].x))/
			(pts[i].x - pts[i-1].x))+
			pts[i-1].y);
	}

	return 0;
}

static int32_t pm8xxx_adc_map_batt_therm(const struct pm8xxx_adc_map_pt *pts,
		uint32_t tablesize, int32_t input, int64_t *output)
{
	bool descending = 1;
	uint32_t i = 0;

	if ((pts == NULL) || (output == NULL))
		return -EINVAL;

	/* Check if table is descending or ascending */
	if (tablesize > 1) {
		if (pts[0].y < pts[1].y)
			descending = 0;
	}

	while (i < tablesize) {
		if ((descending == 1) && (pts[i].y < input)) {
			/* table entry is less than measured
				value and table is descending, stop */
			break;
		} else if ((descending == 0) && (pts[i].y > input)) {
			/* table entry is greater than measured
				value and table is ascending, stop */
			break;
		} else {
			i++;
		}
	}

	if (i == 0) {
		*output = pts[0].x;
	} else if (i == tablesize) {
		*output = pts[tablesize-1].x;
	} else {
		/* result is between search_index and search_index-1 */
		/* interpolate linearly */
		*output = (((int32_t) ((pts[i].x - pts[i-1].x)*
			(input - pts[i-1].y))/
			(pts[i].y - pts[i-1].y))+
			pts[i-1].x);
	}

	return 0;
}

int32_t pm8xxx_adc_scale_default(int32_t adc_code,
		const struct pm8xxx_adc_properties *adc_properties,
		const struct pm8xxx_adc_chan_properties *chan_properties,
		struct pm8xxx_adc_chan_result *adc_chan_result)
{
	bool negative_rawfromoffset = 0, negative_offset = 0;
	int64_t scale_voltage = 0;

	if (!chan_properties || !chan_properties->offset_gain_numerator ||
		!chan_properties->offset_gain_denominator || !adc_properties
		|| !adc_chan_result)
		return -EINVAL;

	scale_voltage = (adc_code -
		chan_properties->adc_graph[ADC_CALIB_ABSOLUTE].adc_gnd)
		* chan_properties->adc_graph[ADC_CALIB_ABSOLUTE].dx;
	if (scale_voltage < 0) {
		negative_offset = 1;
		scale_voltage = -scale_voltage;
	}
	do_div(scale_voltage,
		chan_properties->adc_graph[ADC_CALIB_ABSOLUTE].dy);
	if (negative_offset)
		scale_voltage = -scale_voltage;
	scale_voltage += chan_properties->adc_graph[ADC_CALIB_ABSOLUTE].dx;

	if (scale_voltage < 0) {
		if (adc_properties->bipolar) {
			scale_voltage = -scale_voltage;
			negative_rawfromoffset = 1;
		} else {
			scale_voltage = 0;
		}
	}

	adc_chan_result->measurement = scale_voltage *
				chan_properties->offset_gain_denominator;

	/* do_div only perform positive integer division! */
	do_div(adc_chan_result->measurement,
				chan_properties->offset_gain_numerator);

	if (negative_rawfromoffset)
		adc_chan_result->measurement = -adc_chan_result->measurement;

	/* Note: adc_chan_result->measurement is in the unit of
	 * adc_properties.adc_reference. For generic channel processing,
	 * channel measurement is a scale/ratio relative to the adc
	 * reference input */
	adc_chan_result->physical = adc_chan_result->measurement;

	return 0;
}
EXPORT_SYMBOL_GPL(pm8xxx_adc_scale_default);

static int64_t pm8xxx_adc_scale_ratiometric_calib(int32_t adc_code,
		const struct pm8xxx_adc_properties *adc_properties,
		const struct pm8xxx_adc_chan_properties *chan_properties)
{
	int64_t adc_voltage = 0;
	bool negative_offset = 0;

	if (!chan_properties || !chan_properties->offset_gain_numerator ||
		!chan_properties->offset_gain_denominator || !adc_properties)
		return -EINVAL;

	adc_voltage = (adc_code -
		chan_properties->adc_graph[ADC_CALIB_RATIOMETRIC].adc_gnd)
		* adc_properties->adc_vdd_reference;
	if (adc_voltage < 0) {
		negative_offset = 1;
		adc_voltage = -adc_voltage;
	}
	do_div(adc_voltage,
		chan_properties->adc_graph[ADC_CALIB_RATIOMETRIC].dy);
	if (negative_offset)
		adc_voltage = -adc_voltage;

	return adc_voltage;
}

int32_t pm8xxx_adc_scale_batt_therm(int32_t adc_code,
		const struct pm8xxx_adc_properties *adc_properties,
		const struct pm8xxx_adc_chan_properties *chan_properties,
		struct pm8xxx_adc_chan_result *adc_chan_result)
{
	int64_t bat_voltage = 0;

	bat_voltage = pm8xxx_adc_scale_ratiometric_calib(adc_code,
			adc_properties, chan_properties);

	return pm8xxx_adc_map_batt_therm(
			adcmap_btm_threshold,
			ARRAY_SIZE(adcmap_btm_threshold),
			bat_voltage,
			&adc_chan_result->physical);
}
EXPORT_SYMBOL_GPL(pm8xxx_adc_scale_batt_therm);

int32_t pm8xxx_adc_scale_pa_therm(int32_t adc_code,
		const struct pm8xxx_adc_properties *adc_properties,
		const struct pm8xxx_adc_chan_properties *chan_properties,
		struct pm8xxx_adc_chan_result *adc_chan_result)
{
	int64_t pa_voltage = 0;

	pa_voltage = pm8xxx_adc_scale_ratiometric_calib(adc_code,
			adc_properties, chan_properties);

	return pm8xxx_adc_map_linear(
			adcmap_pa_therm,
			ARRAY_SIZE(adcmap_pa_therm),
			pa_voltage,
			&adc_chan_result->physical);
}
EXPORT_SYMBOL_GPL(pm8xxx_adc_scale_pa_therm);

int32_t pm8xxx_adc_scale_batt_id(int32_t adc_code,
		const struct pm8xxx_adc_properties *adc_properties,
		const struct pm8xxx_adc_chan_properties *chan_properties,
		struct pm8xxx_adc_chan_result *adc_chan_result)
{
	int64_t batt_id_voltage = 0;

	batt_id_voltage = pm8xxx_adc_scale_ratiometric_calib(adc_code,
			adc_properties, chan_properties);
	adc_chan_result->physical = batt_id_voltage;
	adc_chan_result->physical = adc_chan_result->measurement;

	return 0;
}
EXPORT_SYMBOL_GPL(pm8xxx_adc_scale_batt_id);

int32_t pm8xxx_adc_scale_pmic_therm(int32_t adc_code,
		const struct pm8xxx_adc_properties *adc_properties,
		const struct pm8xxx_adc_chan_properties *chan_properties,
		struct pm8xxx_adc_chan_result *adc_chan_result)
{
	int64_t pmic_voltage = 0;
	bool negative_offset = 0;

	if (!chan_properties || !chan_properties->offset_gain_numerator ||
		!chan_properties->offset_gain_denominator || !adc_properties
		|| !adc_chan_result)
		return -EINVAL;

	pmic_voltage = (adc_code -
		chan_properties->adc_graph[ADC_CALIB_ABSOLUTE].adc_gnd)
		* chan_properties->adc_graph[ADC_CALIB_ABSOLUTE].dx;
	if (pmic_voltage < 0) {
		negative_offset = 1;
		pmic_voltage = -pmic_voltage;
	}
	do_div(pmic_voltage,
		chan_properties->adc_graph[ADC_CALIB_ABSOLUTE].dy);
	if (negative_offset)
		pmic_voltage = -pmic_voltage;
	pmic_voltage += chan_properties->adc_graph[ADC_CALIB_ABSOLUTE].dx;

	if (pmic_voltage > 0) {
		/* 2mV/K */
		adc_chan_result->measurement = pmic_voltage*
			chan_properties->offset_gain_denominator;

		do_div(adc_chan_result->measurement,
			chan_properties->offset_gain_numerator * 2);
	} else {
		adc_chan_result->measurement = 0;
	}
	/* Change to .001 deg C */
	adc_chan_result->measurement -= KELVINMIL_DEGMIL;
	adc_chan_result->physical = (int32_t)adc_chan_result->measurement;

	return 0;
}
EXPORT_SYMBOL_GPL(pm8xxx_adc_scale_pmic_therm);

/* Scales the ADC code to 0.001 degrees C using the map
 * table for the XO thermistor.
 */
int32_t pm8xxx_adc_tdkntcg_therm(int32_t adc_code,
		const struct pm8xxx_adc_properties *adc_properties,
		const struct pm8xxx_adc_chan_properties *chan_properties,
		struct pm8xxx_adc_chan_result *adc_chan_result)
{
	int64_t xo_thm = 0;

	if (!chan_properties || !chan_properties->offset_gain_numerator ||
		!chan_properties->offset_gain_denominator || !adc_properties
		|| !adc_chan_result)
		return -EINVAL;

	xo_thm = pm8xxx_adc_scale_ratiometric_calib(adc_code,
			adc_properties, chan_properties);
	xo_thm <<= 4;
	pm8xxx_adc_map_linear(adcmap_ntcg_104ef_104fb,
		ARRAY_SIZE(adcmap_ntcg_104ef_104fb),
		xo_thm, &adc_chan_result->physical);

	return 0;
}
EXPORT_SYMBOL_GPL(pm8xxx_adc_tdkntcg_therm);

int32_t pm8xxx_adc_batt_scaler(struct pm8xxx_adc_arb_btm_param *btm_param,
		const struct pm8xxx_adc_properties *adc_properties,
		const struct pm8xxx_adc_chan_properties *chan_properties)
{
	int rc;

	rc = pm8xxx_adc_map_linear(
		adcmap_btm_threshold,
		ARRAY_SIZE(adcmap_btm_threshold),
		(btm_param->low_thr_temp),
		&btm_param->low_thr_voltage);
	if (rc)
		return rc;

	btm_param->low_thr_voltage *=
		chan_properties->adc_graph[ADC_CALIB_RATIOMETRIC].dy;
	do_div(btm_param->low_thr_voltage, adc_properties->adc_vdd_reference);
	btm_param->low_thr_voltage +=
		chan_properties->adc_graph[ADC_CALIB_RATIOMETRIC].adc_gnd;

	rc = pm8xxx_adc_map_linear(
		adcmap_btm_threshold,
		ARRAY_SIZE(adcmap_btm_threshold),
		(btm_param->high_thr_temp),
		&btm_param->high_thr_voltage);
	if (rc)
		return rc;

	btm_param->high_thr_voltage *=
		chan_properties->adc_graph[ADC_CALIB_RATIOMETRIC].dy;
	do_div(btm_param->high_thr_voltage, adc_properties->adc_vdd_reference);
	btm_param->high_thr_voltage +=
		chan_properties->adc_graph[ADC_CALIB_RATIOMETRIC].adc_gnd;


	return rc;
}
EXPORT_SYMBOL_GPL(pm8xxx_adc_batt_scaler);

int32_t pm8xxx_adc_scale_pba_therm(int32_t adc_code,
		const struct pm8xxx_adc_properties *adc_properties,
		const struct pm8xxx_adc_chan_properties *chan_properties,
		struct pm8xxx_adc_chan_result *adc_chan_result)
{
	int64_t pba_thm = 0;

	if (!chan_properties || !chan_properties->offset_gain_numerator ||
		!chan_properties->offset_gain_denominator || !adc_properties
		|| !adc_chan_result)
		return -EINVAL;

	pba_thm = pm8xxx_adc_scale_ratiometric_calib(adc_code,
			adc_properties, chan_properties);

	pm8xxx_adc_map_linear(adcmap_pba_therm,
		ARRAY_SIZE(adcmap_pba_therm),
		pba_thm, &adc_chan_result->physical);

	return 0;
}
EXPORT_SYMBOL_GPL(pm8xxx_adc_scale_pba_therm);

int32_t pm8xxx_adc_scale_bl_therm(int32_t adc_code,
		const struct pm8xxx_adc_properties *adc_properties,
		const struct pm8xxx_adc_chan_properties *chan_properties,
		struct pm8xxx_adc_chan_result *adc_chan_result)
{
	int64_t bl_voltage = 0;
	uint32_t tablesize;
	int rc = 0;

	bl_voltage = pm8xxx_adc_scale_ratiometric_calib(adc_code,
			adc_properties, chan_properties);
	tablesize = ARRAY_SIZE(adcmap_bl_therm);

	if (bl_voltage > adcmap_bl_therm[0].x
		|| bl_voltage < adcmap_bl_therm[tablesize-1].x){
		adc_chan_result->physical = PM8xxx_ADC_BL_THERM_INVALID_TEMP;
	} else {
		rc = pm8xxx_adc_map_linear(
				adcmap_bl_therm,
				tablesize,
				bl_voltage,
				&adc_chan_result->physical);
	}

	return rc;
}
EXPORT_SYMBOL_GPL(pm8xxx_adc_scale_bl_therm);
