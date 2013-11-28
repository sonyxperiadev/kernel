/*
 * Application for Gesture Algorithm implement in AMS-TAOS TMG3992/3.
 *
 * Copyright (c) 2013, ams AG, Inc.
 *
 * Author: Byron Shi <byron.shi@ams.com>
*/
#include <linux/kernel.h>
#include <linux/i2c.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/string.h>
#include <linux/mutex.h>
#include <linux/unistd.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/slab.h>
#include <linux/time.h>
#include <linux/pm.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/string.h>
#include <linux/tmg399x.h>
#include "tmg399x_common.h"
#define ARRAY_LENGTH	500

/* raw data states enum */
enum sRawDataStates { IDLE, TENTRY, STARTED, TENDED };

/* gesture events enum */
enum eDirection { North, Northeast, East, Southeast, South, Southwest, West,
	Northwest
};
enum eFunction { Cross, Tap };
enum sGestureLanguage { Waiting, Started, Cross_N_S, Cross_S_N, Cross_E_W,
	Cross_W_E, Cross_NE_SW, Cross_SW_NE, Cross_NW_SE,
	Cross_SE_NW, Tap_N, Tap_S, Tap_E, Tap_W, Button_N,
	Button_S, Button_E, Button_W, Button_Held,
	Button_Released, Not_Defined
};

/* NSWE data structure */
struct sNSWE {
	int north;
	int south;
	int west;
	int east;
};

/* gesture raw data structure */
struct sRawData {
	u8 state;
	struct timeval cur_time;
	struct sNSWE nswe;
	int gproxmax;
	int count;
};

/* gesture raw data array structure */
struct sRawDataArray {
	struct timeval start_time;
	struct timeval end_time;
	int peak_gprox_index;
	int count;		/* number of gesture data sets received */
	int length;		/* length of rawdata array */
	struct sNSWE nswe_offset;
	struct sNSWE nswe_scale100;
	struct sRawData rawdata[ARRAY_LENGTH];
	int next_long_time;
	int long_count;
};

struct sGesture_Max {
	int index;
	bool centered;
	int nswe;
};

struct sGesture_Diff {
	int index;
	struct sNSWE nswe;
	int n_s;
	int e_w;
	int ne_sw;
	int nw_se;
};

/* gesture vector structure */
struct sGesture_Vector {
	struct timeval start_time;
	struct timeval end_time;
	struct sGesture_Diff ss_strength;
	struct sGesture_Diff enter;
	struct sGesture_Diff exit;

	int test;
	int enter_angle;
	int exit_angle;
	long duration_ms;
	struct sGesture_Max max;
};

/* gesture event structure*/
struct sGesture_DS {
	u8 dir;
	int strength;
};

struct sGesture_DFS {
	u8 enter_dir;
	u8 exit_dir;
	u8 function;
	u8 gesture;
};

struct sGesture_Event {
	struct sGesture_DFS ns;
	struct sGesture_DFS ew;
	struct sGesture_DFS nswe;
	struct sGesture_DFS dir4;
	struct sGesture_DFS dir8;
};

/* RGBC data structure */
struct sColor4 {
	unsigned int red;
	unsigned int green;
	unsigned int blue;
	unsigned int clear;
};

/* gesture variables and processing parameters */
struct sRawData mrawdata;
struct sNSWE nswe_data;
struct sNSWE nswe_buf[2];

struct sNSWE nswe_dcoffset_slow200;
struct sNSWE nswe_dcoffset_fast10;
struct sNSWE nswe_dcoffset_slow;
struct sNSWE nswe_dcoffset_fast;
struct sNSWE nswe_offset;
struct sNSWE nswe_scale100;
struct sNSWE nswe_scale_filter1000;

int ges_exit_threshold;
int runt_threshold;
int mfilter_dc_count;

/* check long gesture variables */
int long_first_time_inc;
int long_time_inc;
u8 max_long_count;

/* gesture raw data array */
struct sRawDataArray mrawdataarray;
int array_length;

struct sGesture_Vector ges_vector;
struct sGesture_Event ges_event;

/* filter variables */
int stdev_filter100_10;
int slow_thresh_filter200;
int med_thresh_filter50;
int fast_thresh_filter10;
int stdev_filter100;
int stdev_default100;
int slow_thresh_filter;
int med_thresh_filter;
int fast_thresh_filter;

int mnumnpoints;
int mnumncount;
int mstdev;
int mavg;
int mlast_avg;

int numstdevs10;

/* global variables for Proximity */
u8 prox_data;
u8 prox_buf[2];
u8 prox_state;

int prox_baseline10;
int prox_baseline_to_threshold10;
int prox_entry_threshold;
int prox_exit_threshold;
int exit_factor10;

/* state for prox and gesture */
u8 prox_dc_ok;
u8 ges_dc_ok;
u8 hidden_data;
u8 hidden_data_enable;

u8 raw_data_state;

/* global variables for RGBC */
struct sColor4 color_data;
struct sColor4 colorfilter;
struct sColor4 colorfilter16;

bool average2 = true;

int arctan_table_x1024[] = {
	0, 17, 35, 53, 71, 89, 107, 125,
	143, 162, 180, 199, 217, 236, 255, 274,
	293, 313, 332, 352, 372, 393, 413, 434,
	455, 477, 499, 521, 544, 567, 591, 615,
	639, 664, 690, 716, 743, 771, 800, 829,
	859, 890, 921, 954, 988, 1023, 1060, 1098,
	1137, 1177, 1220, 1264, 1310, 1358, 1409, 1462,
	1518, 1576, 1638, 1704, 1773, 1847, 1925, 2009,
	2099, 2195, 2299, 2412, 2534, 2667, 2813, 2973,
	3151, 3349, 3570, 3821, 4106, 4434, 4817, 5267,
	5806, 6464, 7284, 8338, 9740, 11701, 14638, 19529,
	29301, 58577
};


static int fix_sqrt(unsigned int x)
{
	unsigned int b, m, y = 0;

	if (x <= 1)
		return x;

	m = (1 << 30);
	while (m != 0) {
		b = y + m;
		y >>= 1;

		if (x >= b) {
			x -= b;
			y += m;
		}

		m >>= 2;
	}

	return y;
}

static int fix_arctan_x1024(int value)
{
	int begin = 1;
	int end = 89;
	int index;

	if (value < arctan_table_x1024[1])
		return 0;
	if (value > arctan_table_x1024[89])
		return 89;

	while (1) {
		index = (begin + end) / 2;

		if (value < arctan_table_x1024[index]) {
			end = index;
			continue;
		} else if (value > arctan_table_x1024[index + 1]) {
			begin = index + 1;
			continue;
		} else
			return index;
	}
}

/* functions */
static void init_nswe_data(struct sNSWE *dest, int value)
{
	dest->north = value;
	dest->south = value;
	dest->west = value;
	dest->east = value;
}

static void set_nswe_data(struct sNSWE *dest, struct sNSWE src)
{
	dest->north = src.north;
	dest->south = src.south;
	dest->west = src.west;
	dest->east = src.east;
}

static void mpy_nswe_data(struct sNSWE *dest, struct sNSWE src, int mul)
{
	dest->north = src.north * mul;
	dest->south = src.south * mul;
	dest->west = src.west * mul;
	dest->east = src.east * mul;
}

static int get_nswe_max(struct sNSWE nswe)
{
	int max = nswe.north;
	if (nswe.south > max)
		max = nswe.south;
	if (nswe.west > max)
		max = nswe.west;
	if (nswe.east > max)
		max = nswe.east;
	return max;
}

static int sum_nswe_data(struct sNSWE nswe)
{
	int ret;

	ret = nswe.north + nswe.south + nswe.west + nswe.east;
	return ret;
}

static int filter(int speed, int new_data, int *filtered_data)
{
	int speed1, fd;

	speed1 = speed + 1;
	fd = *filtered_data + new_data;
	fd = fd * speed + speed / 2;
	fd = fd / speed1;
	*filtered_data = fd;
	fd = (fd + speed / 2) / speed;
	return fd;
}

static void filter4(int speed, struct sNSWE *dest, struct sNSWE src1,
		    struct sNSWE *src2)
{
	dest->north = filter(speed, src1.north, &(src2->north));
	dest->south = filter(speed, src1.south, &(src2->south));
	dest->west = filter(speed, src1.west, &(src2->west));
	dest->east = filter(speed, src1.east, &(src2->east));
}

void set_visible_data_mode(struct tmg399x_chip *chip)
{
	tmg399x_start_calibration(chip);
	hidden_data = false;
	prox_dc_ok = false;
	ges_dc_ok = false;
	mfilter_dc_count = 0;
}


static void set_hidden_data_mode(struct tmg399x_chip *chip)
{
	ges_exit_threshold = get_nswe_max(nswe_offset) + 5;
	tmg399x_set_ges_thresh(chip, prox_entry_threshold, ges_exit_threshold);
	hidden_data = true;
	mfilter_dc_count = 0;
}

static void reset_stdev(u8 data)
{
	mnumncount = 0;
	mstdev = 0;
	mavg = data;
	mlast_avg = data;
}

static void init_stdev_filters(u8 baseline)
{
	stdev_filter100_10 = stdev_default100 * 1000;
	stdev_filter100 = stdev_default100;
	reset_stdev(baseline);
}

static void init_thresh_filters(u8 baseline)
{
	slow_thresh_filter200 = baseline * 200;
	med_thresh_filter50 = baseline * 50;
	fast_thresh_filter10 = baseline * 10;

	slow_thresh_filter = baseline;
	med_thresh_filter = baseline;
	fast_thresh_filter = baseline;
}

static void set_thresholds(int avg10, int delta10)
{
	if (avg10 > 640)
		avg10 = 640;
	if (delta10 < 10)
		delta10 = 10;
	else if (delta10 > 100)
		delta10 = 100;

	prox_baseline10 = avg10;
	prox_baseline_to_threshold10 = delta10;
	prox_entry_threshold = (avg10 + delta10 + 5) / 10;
	prox_exit_threshold = (avg10 +
			       (delta10 * exit_factor10 + 5) / 10 + 5) / 10;
}

/* Gesture processing */
static void average_ges_data(struct sNSWE *nswe)
{
	u8 gindex = 0;

	if (average2) {
		nswe_buf[gindex].north = nswe->north;
		nswe_buf[gindex].south = nswe->south;
		nswe_buf[gindex].west = nswe->west;
		nswe_buf[gindex].east = nswe->east;
		nswe->north = (nswe_buf[0].north + nswe_buf[1].north) / 2;
		nswe->south = (nswe_buf[0].south + nswe_buf[1].south) / 2;
		nswe->west = (nswe_buf[0].west + nswe_buf[1].west) / 2;
		nswe->east = (nswe_buf[0].east + nswe_buf[1].east) / 2;
		gindex = (gindex + 1) & 0x01;
	}
}

static void init_nswe_dc_values(struct sNSWE nswe)
{
	/* init NSWE DC offset filters */
	mpy_nswe_data(&nswe_dcoffset_slow200, nswe, 200);
	mpy_nswe_data(&nswe_dcoffset_fast10, nswe, 10);
	set_nswe_data(&nswe_dcoffset_slow, nswe);
	set_nswe_data(&nswe_dcoffset_fast, nswe);
	ges_dc_ok = true;
}

static int get_angle(int north, int east)
{
	int angle, angle1, x, y;

	if (north == 0)
		angle1 = 90;
	else {
		if (east < 0)
			x = -east;
		else
			x = east;
		if (north < 0)
			y = -north;
		else
			y = north;
		angle1 = fix_arctan_x1024((x << 10) / y);
	}

	if ((north < 0) && (east >= 0))
		angle = 180 - angle1;
	else if ((north < 0) && (east < 0))
		angle = 180 + angle1;
	else if ((north > 0) && (east < 0))
		angle = 360 - angle1;
	else
		angle = angle1;

	return angle;
}

static u8 get_opposite(u8 dir)
{
	u8 opposite = 0;

	switch (dir) {
	case North:
		opposite = South;
		break;
	case Northeast:
		opposite = Southwest;
		break;
	case East:
		opposite = West;
		break;
	case Southeast:
		opposite = Northwest;
		break;
	case South:
		opposite = North;
		break;
	case Southwest:
		opposite = Northeast;
		break;
	case West:
		opposite = East;
		break;
	case Northwest:
		opposite = Southeast;
		break;
	}

	return opposite;
}

static void get_ges_dfs8(struct sGesture_DFS *dir8,
			 struct sGesture_DS enter_dir,
			 struct sGesture_DS exit_dir)
{
	u8 dif, enter_opposite, exit_opposite;

	dir8->enter_dir = enter_dir.dir;
	dir8->exit_dir = exit_dir.dir;

	enter_opposite = get_opposite(enter_dir.dir);
	exit_opposite = get_opposite(exit_dir.dir);

	dif = enter_dir.dir - exit_dir.dir;
	if (dif == 7 || dif == -7 || dif == -1)
		dif = 1;
	if (dif != 1 && dif != 0) {
		if (enter_dir.strength > exit_dir.strength)
			dir8->exit_dir = enter_opposite;
		else
			dir8->enter_dir = exit_opposite;
		dir8->function = Cross;
	} else {
		dir8->exit_dir = enter_dir.dir;
		dir8->function = Tap;
	}
}

static void get_direction8(struct sGesture_DS *dir8,
			   struct sGesture_Vector *vector, int angle)
{
	int inc = 23;

	if (angle < 23 || angle > (360 - 23)) {
		dir8->dir = North;
		dir8->strength = vector->ss_strength.n_s;
	} else if (angle < (inc + 45)) {
		dir8->dir = Northeast;
		dir8->strength = vector->ss_strength.ne_sw;
	} else if (angle < (inc + 90)) {
		dir8->dir = East;
		dir8->strength = vector->ss_strength.e_w;
	} else if (angle < (inc + 135)) {
		dir8->dir = Southeast;
		dir8->strength = vector->ss_strength.nw_se;
	} else if (angle < (inc + 180)) {
		dir8->dir = South;
		dir8->strength = vector->ss_strength.n_s;
	} else if (angle < (inc + 225)) {
		dir8->dir = Southwest;
		dir8->strength = vector->ss_strength.ne_sw;
	} else if (angle < (inc + 270)) {
		dir8->dir = West;
		dir8->strength = vector->ss_strength.e_w;
	} else {
		dir8->dir = Northwest;
		dir8->strength = vector->ss_strength.nw_se;
	}
}

static void get_ges_dfs4(struct sGesture_DFS *dir4,
			 struct sGesture_DS enter_dir,
			 struct sGesture_DS exit_dir)
{
	u8 dif, enter_opposite, exit_opposite;

	dir4->enter_dir = enter_dir.dir;
	dir4->exit_dir = exit_dir.dir;

	enter_opposite = get_opposite(enter_dir.dir);
	exit_opposite = get_opposite(exit_dir.dir);

	dif = enter_dir.dir - exit_dir.dir;
	if (dif != 0) {
		if (enter_dir.strength > exit_dir.strength)
			dir4->exit_dir = enter_opposite;
		else
			dir4->enter_dir = exit_opposite;
		dir4->function = Cross;
	} else {
		dir4->exit_dir = enter_dir.dir;
		dir4->function = Tap;
	}
}

static void get_direction4(struct sGesture_DS *dir4,
			   struct sGesture_Vector *vector, int angle)
{
	int inc = 45;

	if (angle < 45 || angle > (360 - 45)) {
		dir4->dir = North;
		dir4->strength = vector->ss_strength.n_s;
	} else if (angle < (inc + 90)) {
		dir4->dir = East;
		dir4->strength = vector->ss_strength.e_w;
	} else if (angle < (inc + 180)) {
		dir4->dir = South;
		dir4->strength = vector->ss_strength.n_s;
	} else {
		dir4->dir = West;
		dir4->strength = vector->ss_strength.e_w;
	}
}

static void create_ges_event(struct tmg399x_chip *chip)
{
	struct sGesture_DS enter_dir;
	struct sGesture_DS exit_dir;

	char direction[8][10];

	strcpy(direction[0], "North");
	strcpy(direction[1], "Northeast");
	strcpy(direction[2], "East");
	strcpy(direction[3], "Southeast");
	strcpy(direction[4], "South");
	strcpy(direction[5], "Southwest");
	strcpy(direction[6], "West");
	strcpy(direction[7], "Northwest");

	/* simulate touch events here */
	get_direction4(&enter_dir, &ges_vector, ges_vector.enter_angle);
	get_direction4(&exit_dir, &ges_vector, ges_vector.exit_angle);

	get_ges_dfs4(&ges_event.dir4, enter_dir, exit_dir);
	if (ges_event.dir8.function == Cross)
		tmg399x_report_ges(chip, ges_event.dir4.exit_dir);

	/* get enter and exit directions (8 directions here) */
	get_direction8(&enter_dir, &ges_vector, ges_vector.enter_angle);
	get_direction8(&exit_dir, &ges_vector, ges_vector.exit_angle);

	get_ges_dfs8(&ges_event.dir8, enter_dir, exit_dir);

	if (ges_event.dir4.function == Tap)
		printk(KERN_INFO "%s %d --> %s %d, Tap, duration:%ldms\n",
		       direction[ges_event.dir8.enter_dir],
		       ges_vector.enter_angle,
		       direction[ges_event.dir8.exit_dir],
		       ges_vector.exit_angle, ges_vector.duration_ms);
	else
		printk(KERN_INFO "%s %d --> %s %d, Cross, duration:%ldms\n",
		       direction[ges_event.dir8.enter_dir],
		       ges_vector.enter_angle,
		       direction[ges_event.dir8.exit_dir],
		       ges_vector.exit_angle, ges_vector.duration_ms);

}

static int search_enter_level(int divisor)
{
	int index, thresh;

	thresh = mrawdataarray.rawdata[mrawdataarray.length / 2].gproxmax
	    / divisor;
	for (index = 0; index < mrawdataarray.peak_gprox_index; index++) {
		if (mrawdataarray.rawdata[index].gproxmax > thresh)
			break;
	}

	return index;
}

static int search_exit_level(int divisor)
{
	int index, thresh, avg;

	thresh =
	    mrawdataarray.rawdata[mrawdataarray.peak_gprox_index].gproxmax
	    / divisor;
	for (index = mrawdataarray.length - 1;
	     index > mrawdataarray.peak_gprox_index; index--) {
		avg = mrawdataarray.rawdata[index].gproxmax;
		if (mrawdataarray.rawdata[index].gproxmax > thresh / 2)
			return index;
	}

	return index;
}

static void get_vector(int index, struct sGesture_Diff *vector)
{
	int ne, nw, se, sw;

	vector->index = index;

	vector->nswe.north = mrawdataarray.rawdata[index].nswe.north;
	vector->nswe.south = mrawdataarray.rawdata[index].nswe.south;
	vector->nswe.west = mrawdataarray.rawdata[index].nswe.west;
	vector->nswe.east = mrawdataarray.rawdata[index].nswe.east;

	vector->n_s = vector->nswe.north - vector->nswe.south;
	vector->e_w = vector->nswe.east - vector->nswe.west;

	ne = (vector->nswe.north + vector->nswe.east) / 2;
	nw = (vector->nswe.north + vector->nswe.west) / 2;
	se = (vector->nswe.south + vector->nswe.east) / 2;
	sw = (vector->nswe.south + vector->nswe.west) / 2;

	vector->ne_sw = ne - sw;
	vector->nw_se = nw - se;
}

static void get_exit_vector(struct sGesture_Diff *vector)
{
	int index, index10, to_end;

	index = search_exit_level(2);
	to_end = mrawdataarray.length - 1 - index;
	if (to_end > 6)
		to_end = 6;
	if (to_end == 0)
		to_end = 1;

	index10 = search_exit_level(5);
	if (index10 < index)
		index = index10;

	to_end = mrawdataarray.length - 1 - index;
	if (to_end > 12)
		to_end = 12;

	index = mrawdataarray.length - 1 - to_end;
	get_vector(index, vector);
}

static void get_enter_vector(struct sGesture_Diff *vector)
{
	int index, index10;

	index = search_enter_level(2);
	if (index > 6)
		index = 6;

	index10 = search_enter_level(5);
	if (index10 >= index)
		index = index10;

	if (index > 12)
		index = 12;

	get_vector(index, vector);
}

static void get_max_vector(void)
{
	ges_vector.max.nswe = 0;
	ges_vector.max.centered = true;
	ges_vector.max.index = mrawdataarray.peak_gprox_index;

	if (ges_vector.max.index <= 1 ||
	    (mrawdataarray.length - ges_vector.max.index) <= 1) {
		ges_vector.max.centered = false;
		return;
	}
	ges_vector.max.nswe =
	    sum_nswe_data(mrawdataarray.rawdata[ges_vector.max.index].nswe);
}

static void get_nswe_strength_ss(struct sNSWE *nswe)
{
	int i;

	memset((void *)nswe, 0x00, sizeof(struct sNSWE));
	for (i = 0; i < mrawdataarray.length; i++) {
		nswe->north += mrawdataarray.rawdata[i].nswe.north *
		    mrawdataarray.rawdata[i].nswe.north;
		nswe->south += mrawdataarray.rawdata[i].nswe.south *
		    mrawdataarray.rawdata[i].nswe.south;
		nswe->west += mrawdataarray.rawdata[i].nswe.west *
		    mrawdataarray.rawdata[i].nswe.west;
		nswe->east += mrawdataarray.rawdata[i].nswe.east *
		    mrawdataarray.rawdata[i].nswe.east;
	}
}

static int get_dnorth_strength_ss(void)
{
	int i, dnorth, ss;

	ss = 0;
	for (i = 0; i < mrawdataarray.length; i++) {
		dnorth = mrawdataarray.rawdata[i].nswe.north -
		    mrawdataarray.rawdata[i].nswe.south;
		ss += dnorth * dnorth;
	}
	return ss;
}

static int get_deast_strength_ss(void)
{
	int i, deast, ss;

	ss = 0;
	for (i = 0; i < mrawdataarray.length; i++) {
		deast = mrawdataarray.rawdata[i].nswe.east -
		    mrawdataarray.rawdata[i].nswe.west;
		ss += deast * deast;
	}
	return ss;
}

static int get_dne_strength_ss(void)
{
	int i, ne, sw, dne, ss;

	ss = 0;
	for (i = 0; i < mrawdataarray.length; i++) {
		ne = (mrawdataarray.rawdata[i].nswe.north +
		      mrawdataarray.rawdata[i].nswe.east) / 2;
		sw = (mrawdataarray.rawdata[i].nswe.south +
		      mrawdataarray.rawdata[i].nswe.west) / 2;

		dne = ne - sw;
		ss += dne * dne;
	}
	return ss;
}

static int get_dnw_strength_ss(void)
{
	int i, nw, se, dnw, ss;

	ss = 0;
	for (i = 0; i < mrawdataarray.length; i++) {
		nw = (mrawdataarray.rawdata[i].nswe.north +
		      mrawdataarray.rawdata[i].nswe.west) / 2;
		se = (mrawdataarray.rawdata[i].nswe.south +
		      mrawdataarray.rawdata[i].nswe.east) / 2;

		dnw = nw - se;
		ss += dnw * dnw;
	}
	return ss;
}

static void sqrt_strength(struct sGesture_Diff *pnstrength)
{
	pnstrength->nswe.north = fix_sqrt(pnstrength->nswe.north);
	pnstrength->nswe.south = fix_sqrt(pnstrength->nswe.south);
	pnstrength->nswe.west = fix_sqrt(pnstrength->nswe.west);
	pnstrength->nswe.east = fix_sqrt(pnstrength->nswe.east);

	pnstrength->n_s = fix_sqrt(pnstrength->n_s);
	pnstrength->e_w = fix_sqrt(pnstrength->e_w);
	pnstrength->ne_sw = fix_sqrt(pnstrength->ne_sw);
	pnstrength->nw_se = fix_sqrt(pnstrength->nw_se);
}

static void get_sss_strength(void)
{
	get_nswe_strength_ss(&(ges_vector.ss_strength.nswe));
	ges_vector.ss_strength.n_s = get_dnorth_strength_ss();
	ges_vector.ss_strength.e_w = get_deast_strength_ss();
	ges_vector.ss_strength.ne_sw = get_dne_strength_ss();
	ges_vector.ss_strength.nw_se = get_dnw_strength_ss();
	ges_vector.ss_strength.index = 0;

	sqrt_strength(&ges_vector.ss_strength);
}

static void create_ges_vector(int test)
{
	struct sGesture_Diff enter;
	struct sGesture_Diff exit;

	memset((void *)&enter, 0x00, sizeof(struct sGesture_Diff));
	memset((void *)&exit, 0x00, sizeof(struct sGesture_Diff));
	ges_vector.start_time.tv_sec = mrawdataarray.start_time.tv_sec;
	ges_vector.start_time.tv_usec = mrawdataarray.start_time.tv_usec;
	do_gettimeofday(&(ges_vector.end_time));
	ges_vector.duration_ms =
	    (ges_vector.end_time.tv_sec - ges_vector.start_time.tv_sec)
	    * 1000 +
	    (ges_vector.end_time.tv_usec - ges_vector.start_time.tv_usec)
	    / 1000;
	/* get diff strength */
	get_sss_strength();
	/* get max vector */
	get_max_vector();

	switch (test) {
	case 0:
		/* get enter and exit vector by level searching */
		get_enter_vector(&ges_vector.enter);
		get_exit_vector(&ges_vector.exit);
		break;
	default:
		break;
	}

	/* get enter and exit angle from the vectors */
	ges_vector.enter_angle = get_angle(ges_vector.enter.n_s,
					   ges_vector.enter.e_w);
	ges_vector.exit_angle = get_angle(ges_vector.exit.n_s,
					  ges_vector.exit.e_w);
	ges_vector.test = test;
}

static void process_ges_rawdataarray(void)
{
	set_nswe_data(&(mrawdataarray.nswe_offset), nswe_offset);
	set_nswe_data(&(mrawdataarray.nswe_scale100), nswe_scale100);
}

static void do_ges_end_event(struct tmg399x_chip *chip)
{
	process_ges_rawdataarray();

	create_ges_vector(0);
	create_ges_event(chip);
}

static void get_ges_scale_calibration100(struct sNSWE *pnswe,
					 struct sNSWE *scale100)
{
	int max;

	max = get_nswe_max(*pnswe);

	scale100->north = pnswe->north * 100 / max;
	scale100->south = pnswe->south * 100 / max;
	scale100->west = pnswe->west * 100 / max;
	scale100->east = pnswe->east * 100 / max;
}

static void check_cal_scale(void)
{
	struct sNSWE new_scale100;
	struct sNSWE *pnswe;
	int max;

	max = mrawdataarray.rawdata[mrawdataarray.peak_gprox_index].gproxmax;
	pnswe = &mrawdataarray.rawdata[mrawdataarray.peak_gprox_index].nswe;
	if (mrawdataarray.length < 50) {
		if ((max < 250) && (max > 80)) {
			get_ges_scale_calibration100(pnswe, &new_scale100);
			filter4(10, &nswe_scale100, new_scale100,
				&nswe_scale_filter1000);
		}
	}
}

static int check_runt_ges(void)
{
	int gexit, dcoff, max, runt;

	gexit = ges_exit_threshold;
	dcoff = get_nswe_max(nswe_offset);
	max = mrawdataarray.rawdata[mrawdataarray.peak_gprox_index].gproxmax
	    - dcoff;
	runt = runt_threshold + gexit - dcoff;
	if (max > runt)
		return false;

	printk(KERN_INFO "Runt gesture\n");
	return true;
}

static int check_short_ges(void)
{
	if (mrawdataarray.length < 5) {
		printk(KERN_INFO "Short gesture\n");
		return true;
	}
	return false;
}

static void abort_long_ges(struct tmg399x_chip *chip)
{
	mrawdataarray.long_count = 0;
	mrawdataarray.next_long_time = long_first_time_inc;
	/* abort gesture, and recalibrate the offsets */
	set_visible_data_mode(chip);
}

static void check_long_ges(struct tmg399x_chip *chip)
{
	long duration_ms;
	int inc = 45;

	do_gettimeofday(&mrawdataarray.end_time);
	duration_ms =
	    (mrawdata.cur_time.tv_sec - mrawdataarray.start_time.tv_sec)
	    * 1000 +
	    (mrawdata.cur_time.tv_usec - mrawdataarray.start_time.tv_usec)
	    / 1000;

	if (duration_ms < mrawdataarray.next_long_time)
		return;

	mrawdataarray.next_long_time += long_time_inc;

	if (mrawdataarray.long_count == 0) {
		/* fisrt long count, it's pushed event */
		mrawdataarray.long_count++;
		create_ges_vector(0);
		if (ges_vector.enter_angle < inc
		    || ges_vector.enter_angle > (360 - inc))
			printk(KERN_INFO "North ");
		else if (ges_vector.enter_angle < (inc + 90))
			printk(KERN_INFO "East ");
		else if (ges_vector.enter_angle < (inc + 180))
			printk(KERN_INFO "South ");
		else
			printk(KERN_INFO "West ");

		printk(KERN_INFO "button pushed\n");
		return;
	}
	if (mrawdataarray.long_count > max_long_count) {
		/* it's too long, abort gesture */
		printk(KERN_INFO "long gesture abort\n");
		abort_long_ges(chip);
		return;
	}

	/* after push event, send held event before abort long gesture */
	mrawdataarray.long_count++;
	printk(KERN_INFO "button held\n");
}

static void add_ges_rawdata(struct tmg399x_chip *chip,
			    struct sRawData *prawdata)
{
	/* add gesture raw data into gesture raw data array */
	prawdata->gproxmax = get_nswe_max(prawdata->nswe);
	/* check prox event here */
	if (chip->prx_enabled) {
		if (hidden_data) {
			if (!prox_state && prawdata->gproxmax
			    > PROX_THRESH_HIGH) {
				/* report prox near event here */
				prox_state = true;
				tmg399x_report_prox(chip, prox_state);
			} else if (prox_state
				   && prawdata->gproxmax < PROX_THRESH_LOW) {
				/* report prox far event here */
				prox_state = false;
				tmg399x_report_prox(chip, prox_state);
			}
		}
	}

	if (mrawdataarray.count < array_length) {
		if (mrawdataarray.rawdata
		    [mrawdataarray.peak_gprox_index].gproxmax
		    < prawdata->gproxmax) {
			mrawdataarray.peak_gprox_index = mrawdataarray.length;
		}
		memcpy((void *)&mrawdataarray.rawdata[mrawdataarray.length],
		       (void *)prawdata, sizeof(struct sRawData));
		mrawdataarray.length++;
	}

	mrawdataarray.count++;
}

static void process_ges_states(struct tmg399x_chip *chip,
			       struct sRawData *prawdata)
{
	long duration_ms;
	switch (prawdata->state) {
	case TENTRY:
		do_gettimeofday(&mrawdataarray.start_time);
		mrawdataarray.peak_gprox_index = 0;
		mrawdataarray.count = 0;
		mrawdataarray.length = 0;
		mrawdataarray.next_long_time = long_first_time_inc;
		mrawdataarray.long_count = 0;
		add_ges_rawdata(chip, prawdata);
		break;
	case STARTED:
		add_ges_rawdata(chip, prawdata);
		check_long_ges(chip);
		break;
	case TENDED:
		/* short and runt gestures are hard to get good gesture */
		if (check_short_ges())
			return;
		if (check_runt_ges())
			return;

		/* filter scale factor */
		check_cal_scale();
		if (mrawdataarray.long_count > 0) {
			/* long count bigger than 0, has been in pushed state,
			   need to report released event here */
			do_gettimeofday(&mrawdataarray.end_time);
			duration_ms =
			    (mrawdataarray.end_time.tv_sec
			     - mrawdataarray.start_time.tv_sec) * 1000 +
			    (mrawdataarray.end_time.tv_usec
			     - mrawdataarray.start_time.tv_usec) / 1000;
			pr_info("button released, duration:%ldms\n",
				duration_ms);
		} else
			/* it's good gesture process */
			do_ges_end_event(chip);
		break;
	default:
		break;
	}
}

static void process_ges_rawdata(struct sRawData *prawdata)
{
	prawdata->nswe.north = prawdata->nswe.north - nswe_offset.north;
	prawdata->nswe.south = prawdata->nswe.south - nswe_offset.south;
	prawdata->nswe.west = prawdata->nswe.west - nswe_offset.west;
	prawdata->nswe.east = prawdata->nswe.east - nswe_offset.east;
}

static void do_raw_ges_data(struct tmg399x_chip *chip,
			    struct sRawData *prawdata)
{
	/* remove DC from gesture raw data */
	process_ges_rawdata(prawdata);
	process_ges_states(chip, prawdata);
}

static void filter_nswe_dcoffset_from_background(struct sNSWE nswe)
{
	int sumfast, sumslow;
	mfilter_dc_count++;

	filter4(200, &nswe_dcoffset_slow, nswe, &nswe_dcoffset_slow200);
	filter4(10, &nswe_dcoffset_fast, nswe, &nswe_dcoffset_fast10);

	sumfast = sum_nswe_data(nswe_dcoffset_fast);
	sumslow = sum_nswe_data(nswe_dcoffset_slow);
	if (sumfast < sumslow)
		set_nswe_data(&nswe_offset, nswe_dcoffset_fast);
	else
		set_nswe_data(&nswe_offset, nswe_dcoffset_slow);
}

static void process_hidden_ges_states(struct tmg399x_chip *chip)
{
	switch (raw_data_state) {
	case IDLE:
		mrawdata.count = 0;
		mrawdata.state = TENTRY;
		do_gettimeofday(&mrawdata.cur_time);
		set_nswe_data(&(mrawdata.nswe), nswe_data);
		do_raw_ges_data(chip, &mrawdata);
		raw_data_state = STARTED;
		break;
	case STARTED:
		mrawdata.count++;
		mrawdata.state = STARTED;
		do_gettimeofday(&mrawdata.cur_time);
		set_nswe_data(&(mrawdata.nswe), nswe_data);
		do_raw_ges_data(chip, &mrawdata);
		break;
	default:
		break;
	}
}

static void process_visible_ges_states(struct tmg399x_chip *chip)
{
	switch (raw_data_state) {
	case TENTRY:
		mrawdata.count = 0;
		mrawdata.state = TENTRY;
		do_gettimeofday(&mrawdata.cur_time);
		set_nswe_data(&(mrawdata.nswe), nswe_data);
		do_raw_ges_data(chip, &mrawdata);
		raw_data_state = STARTED;
		break;
	case STARTED:
		mrawdata.state = STARTED;
		do_gettimeofday(&mrawdata.cur_time);
		set_nswe_data(&(mrawdata.nswe), nswe_data);
		mrawdata.count++;
		do_raw_ges_data(chip, &mrawdata);
		break;
	case TENDED:
		mrawdata.state = TENDED;
		do_gettimeofday(&mrawdata.cur_time);
		set_nswe_data(&(mrawdata.nswe), nswe_data);
		do_raw_ges_data(chip, &mrawdata);
		raw_data_state = IDLE;
		break;
	case IDLE:
		filter_nswe_dcoffset_from_background(nswe_data);
		break;
	default:
		break;
	}
}

static void process_hidden_ges(struct tmg399x_chip *chip)
{
	process_hidden_ges_states(chip);
}

static void process_visible_ges(struct tmg399x_chip *chip)
{
	process_visible_ges_states(chip);

	if (hidden_data_enable) {
		/* if hidden mode is allowed, enter hidden mode
		   after 100 times IDLE states */
		if (mfilter_dc_count > 100)
			set_hidden_data_mode(chip);
	}
}

static void process_ges_data(struct tmg399x_chip *chip, struct sNSWE nswe)
{
	if (!ges_dc_ok) {
		/* init DC values for gesture */
		init_nswe_dc_values(nswe);
		return;
	}

	if (hidden_data)
		process_hidden_ges(chip);
	else
		process_visible_ges(chip);
}

/* Proximity processing and thresholds adaptive */
static void average_prox_data(u8 *data)
{
	u8 pindex = 0;

	if (average2) {
		prox_buf[pindex] = *data;
		*data = (prox_buf[0] + prox_buf[1]) / 2;
		pindex = (pindex + 1) & 0x01;
	}
}

static void init_prox_dc_values(u8 prox)
{
	int delta10, baseline10;

	init_stdev_filters(prox);
	init_thresh_filters(prox);
	delta10 = stdev_filter100 * numstdevs10 / 100;
	baseline10 = prox * 10;
	set_thresholds(baseline10, delta10);
	prox_dc_ok = true;
}

static void prox_hidden_threshold_states(struct tmg399x_chip *chip, u8 prox)
{
	/* gesture must have exited, and this is the first prox data point,
	   send gesture ended message */
	if (raw_data_state == STARTED) {
		mrawdata.state = TENDED;
		do_gettimeofday(&mrawdata.cur_time);
		set_nswe_data(&(mrawdata.nswe), nswe_data);
		do_raw_ges_data(chip, &mrawdata);
		raw_data_state = IDLE;
		return;
	}
}

static void prox_visible_threshold_states(u8 prox)
{
	/* threshold engine can move state machine from IDLE to ENTRY or
	   from STARTED to RELEASE only, gesture engine must move from ENTRY
	   to STARTED or RELEASE to IDLE */
	if (raw_data_state == STARTED) {
		/* check gesture exit level */
		if (prox < prox_exit_threshold)
			raw_data_state = TENDED;
		return;
	} else if (raw_data_state == IDLE) {
		/* check gesture entry level */
		if (prox > prox_entry_threshold)
			raw_data_state = TENTRY;
		return;
	}
}

static int moving_stdev(int data)
{
	int ret, mstdev_temp;

	if (mnumncount >= mnumnpoints) {
		mavg = mavg / mnumnpoints;
		mstdev_temp = mstdev;
		mstdev = mstdev / mnumnpoints;
		mstdev = fix_sqrt(mstdev);
		if (mstdev_temp > 0 && mstdev == 0)
			mstdev = 1;

		if (mstdev > 6) {
			reset_stdev((u8) data);
			return 0;
		}
		mlast_avg = mavg;
		mavg = data;
		mnumncount = 0;
		ret = mstdev;
		mstdev = 0;
		return ret;
	}

	mnumncount++;
	mavg = mavg + data;
	mstdev = mstdev + (mlast_avg - data) * (mlast_avg - data);
	return 0;
}

static void update_stdev_filter(int data)
{
	int stdev, stdev100;

	/* calculate standard deviation */
	stdev = moving_stdev(data);
	stdev100 = stdev * 100;

	if (stdev100 != 0)
		stdev_filter100 = filter(10, stdev100, &stdev_filter100_10);
}

static void filter_thresh_from_prox(u8 prox)
{
	int threshold10, baseline10;

	/* filter thresholda and update standard deviation */
	slow_thresh_filter = filter(200, (int)prox, &slow_thresh_filter200);
	med_thresh_filter = filter(50, (int)prox, &med_thresh_filter50);
	fast_thresh_filter = filter(10, (int)prox, &fast_thresh_filter10);
	update_stdev_filter((int)prox);

	threshold10 = stdev_filter100 * numstdevs10 / 100;
	if (med_thresh_filter < slow_thresh_filter)
		baseline10 = med_thresh_filter50 / 5;
	else
		baseline10 = slow_thresh_filter200 / 20;
	set_thresholds(baseline10, threshold10);
}

static void process_prox_threshold_states(struct tmg399x_chip *chip, u8 prox)
{
	/* move threshold state in hidden and visible data mode */
	/* gesture is in hidden data mode */
	if (hidden_data)
		prox_hidden_threshold_states(chip, prox);
	return;

	/* gesture is in IDLE mode, adaptive threshold for gesture from prox */
	if (raw_data_state == IDLE)
		filter_thresh_from_prox(prox);

	/* gesture is in visible data mode */
	prox_visible_threshold_states(prox);
}

static void process_prox_data(struct tmg399x_chip *chip, u8 prox)
{
	u8 prox_zero_count = 0;

	if (prox == 0) {
		if (prox_zero_count++ > 5)
			/* prox zero count bigger than 5, need to calibrate
			   the offset for prox & gesture, also need to
			recalculate prox & gesture DC */
			set_visible_data_mode(chip);
		return;
	} else {
		prox_zero_count = 0;
		if (chip->prx_enabled) {
			if (!prox_state && prox_data > PROX_THRESH_HIGH) {
				/* report prox near event here */
				prox_state = true;
				tmg399x_report_prox(chip, prox_state);
			} else if (prox_state && prox_data < PROX_THRESH_LOW) {
				/* report prox far event here */
				prox_state = false;
				tmg399x_report_prox(chip, prox_state);
			}
		}
	}

	if (!prox_dc_ok) {
		/* init the DC values for prox */
		init_prox_dc_values(prox);
		return;
	}

	/* adaptive threshold for gesture */
	process_prox_threshold_states(chip, prox);
}

/* RGBC processing */
void process_rgbc_data(struct tmg399x_chip *chip, struct sColor4 *data)
{
	/* filter RGBC raw data */
	colorfilter.clear = filter(16, data->clear, &colorfilter16.clear);
	colorfilter.red = filter(16, data->red, &colorfilter16.red);
	colorfilter.green = filter(16, data->green, &colorfilter16.green);
	colorfilter.blue = filter(16, data->blue, &colorfilter16.blue);

	if (chip->rgbc_poll_flag) {
		chip->als_inf.ir = (colorfilter.red + colorfilter.red
				    + colorfilter.blue
				    - colorfilter.clear + 1) / 2;
		if (chip->als_inf.ir < 0)
			chip->als_inf.ir = 0;

		chip->als_inf.red_raw = colorfilter.red;
		chip->als_inf.green_raw = colorfilter.green;
		chip->als_inf.blue_raw = colorfilter.blue;
		chip->als_inf.clear_raw = colorfilter.clear;

		tmg399x_report_als(chip);
	}
}

/* Main entry for Prox/Ges/RGBC processing */
void process_rgbc_prox_ges_raw_data(struct tmg399x_chip *chip, u8 type,
				    u8 *data, u8 datalen)
{
	u8 i;
	u8 nswe_len;

	if (type == PROX_DATA) {
		/* get proximity raw data */
		prox_data = data[0];

		average_prox_data(&prox_data);
		process_prox_data(chip, prox_data);
	} else if (type == GES_DATA) {
		/* calculate how many NSWE data sets to process */
		nswe_len = datalen / 4;
		for (i = 0; i < nswe_len; i++) {
			/* get gesture raw data */
			if (data[i * 4] == 0)
				continue;
			nswe_data.north = data[i * 4];
			nswe_data.south = data[i * 4 + 1];
			nswe_data.west = data[i * 4 + 2];
			nswe_data.east = data[i * 4 + 3];

			average_ges_data(&nswe_data);
			process_ges_data(chip, nswe_data);
		}
	} else if (type == RGBC_DATA) {
		/* get RGBC raw data */
		color_data.clear = le16_to_cpup((const __le16 *)&data[0]);
		color_data.red = le16_to_cpup((const __le16 *)&data[2]);
		color_data.green = le16_to_cpup((const __le16 *)&data[4]);
		color_data.blue = le16_to_cpup((const __le16 *)&data[6]);

		process_rgbc_data(chip, &color_data);
	}
}


/* Prox/Ges/RGBC processing parameters init */
void init_params_rgbc_prox_ges(void)
{
	memset((void *)nswe_buf, 0x00, sizeof(struct sNSWE) * 2);

	init_nswe_data(&nswe_dcoffset_slow200, 200);
	init_nswe_data(&nswe_dcoffset_fast10, 10);
	init_nswe_data(&nswe_dcoffset_slow, 0);
	init_nswe_data(&nswe_dcoffset_fast, 0);
	init_nswe_data(&nswe_offset, 0);
	init_nswe_data(&nswe_scale100, 100);
	init_nswe_data(&nswe_scale_filter1000, 1000);

	runt_threshold = 4;
	mfilter_dc_count = 0;

	long_first_time_inc = 1000;
	long_time_inc = 1000;
	max_long_count = 5;

	array_length = ARRAY_LENGTH;

	stdev_default100 = 200;

	mnumnpoints = 100;
	numstdevs10 = 30;
	prox_data = 0;
	memset((void *)prox_buf, 0x00, sizeof(u8) * 2);
	prox_state = false;

	prox_baseline10 = 120;
	prox_baseline_to_threshold10 = 90;
	prox_entry_threshold = 21;
	prox_exit_threshold = 17;
	exit_factor10 = 5;

	prox_dc_ok = false;
	ges_dc_ok = false;
	hidden_data = false;
	hidden_data_enable = true;

	raw_data_state = IDLE;

	memset((void *)&color_data, 0x00, sizeof(struct sColor4));
	memset((void *)&colorfilter, 0x00, sizeof(struct sColor4));
	memset((void *)&colorfilter16, 0x00, sizeof(struct sColor4));
}
