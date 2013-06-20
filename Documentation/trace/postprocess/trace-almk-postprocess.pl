#!/usr/bin/perl
#/**********************************************************************#
#*                                                                      #
#*  Copyright 2012  Broadcom Corporation                                #
#*                                                                      #
#* Unless you and Broadcom execute a separate written software license  #
#* agreement governing use of this software, this software is licensed  #
#* to you under the terms of the GNU General Public License version 2   #
#* (the GPL), available at						#
#*                                                                      #
#*          http://www.broadcom.com/licenses/GPLv2.php                  #
#*                                                                      #
#*  with the following added to such license:                           #
#*                                                                      #
#*  As a special exception, the copyright holders of this software give #
#*  you permission to link this software with independent modules, and  #
#*  to copy and distribute the resulting executable under terms of your #
#*  choice, provided that you also meet, for each linked independent    #
#*  module, the terms and conditions of the license of that module. An  #
#*  independent module is a module which is not derived from this       #
#*  software.  The special   exception does not apply to any            #
#*  modifications of the software.					#
#*									#
#*  Notwithstanding the above, under no circumstances may you combine	#
#*  this software in any way with any other Broadcom software provided	#
#*  under a license other than the GPL, without Broadcom's express	#
#*  prior written consent.						#
#*									#
#***********************************************************************#
use strict;

use constant OOM_SCORE_ADJ_MAX	=> 1000;
use constant OOM_ADJUST_MAX	=> 15;
use constant OOM_DISABLE	=> 17;
use constant MAX_VAL_INT	=> 2147483647;

my $file;
my @line;
my $FILE_HANDLE;

my $almk_trigger_count = 0;
my $total_ks_count = 0;
my $ks_zero_nr_to_scan = 0;
my $ks_nonzero_nr_to_scan = 0;
my @s_ks_gfp_counter;
my $s_ks_gfp_counter_index = 0;
my @s_ks_adj_counter;
my $s_ks_adj_counter_index = 0;
my @s_ks_minfree_counter;
my $s_ks_minfree_counter_index = 0;

my $total_dr_count = 0;
my $dr_zero_nr_to_scan = 0;
my $dr_nonzero_nr_to_scan = 0;
my @s_dr_gfp_counter;
my $s_dr_gfp_counter_index = 0;
my @s_dr_adj_counter;
my $s_dr_adj_counter_index = 0;
my @s_dr_minfree_counter;
my $s_dr_minfree_counter_index = 0;

my $e_ks_return_memdie = 0;
my $e_ks_didnt_select = 0;
my $e_ks_sigkill = 0;

my $e_dr_return_memdie = 0;
my $e_dr_didnt_select = 0;
my $e_dr_sigkill = 0;

my @e_ks_gfp_counter;
my $e_ks_gfp_counter_index = 0;
my @e_ks_adj_counter;
my $e_ks_adj_counter_index = 0;
my @e_ks_minfree_counter;
my $e_ks_minfree_counter_index = 0;

my @e_dr_gfp_counter;
my $e_dr_gfp_counter_index = 0;
my @e_dr_adj_counter;
my $e_dr_adj_counter_index = 0;
my @e_dr_minfree_counter;
my $e_dr_minfree_counter_index = 0;

sub report
{
	my $loop;

	printf "Number of times LMK was triggerd (total): %d\n",
		$almk_trigger_count;

	print "\n";

	print "Number of times LMK was triggered from:\n";

	#kswapd
	printf "kswapd (total): %d\n",
		$total_ks_count;
	printf "kswapd (nr_to_scan == 0): %d\n",
		$ks_zero_nr_to_scan;
	printf "kswapd (nr_to_scan != 0): %d\n",
		$ks_nonzero_nr_to_scan;

	for ($loop = 0; $loop < $s_ks_gfp_counter_index; $loop++) {
		printf "kswapd for gfp_mask 0x%x: %d\n",
			$s_ks_gfp_counter[$loop * 2],
			$s_ks_gfp_counter[($loop * 2) + 1];
	}

	for ($loop = 0; $loop < $s_ks_adj_counter_index; $loop++) {
		printf "kswapd for adj %d: %d\n",
			$s_ks_adj_counter[$loop * 2],
			$s_ks_adj_counter[($loop * 2) + 1];
	}

	for ($loop = 0; $loop < $s_ks_minfree_counter_index; $loop++) {
		printf "kswapd for minfree %d: %d\n",
			$s_ks_minfree_counter[$loop * 2],
			$s_ks_minfree_counter[($loop * 2) + 1];
	}

	print "\n";

	# direct reclaims
	printf "direct reclaim path (total): %d\n",
		$total_dr_count;
	printf "direct reclaim path (nr_to_scan == 0): %d\n",
		$dr_zero_nr_to_scan;
	printf "direct reclaim path (nr_to_scan != 0): %d\n",
		$dr_nonzero_nr_to_scan;

	for ($loop = 0; $loop < $s_dr_gfp_counter_index; $loop++) {
		printf "direct reclaim path for gfp_mask 0x%x: %d\n",
			$s_dr_gfp_counter[$loop * 2],
			$s_dr_gfp_counter[($loop * 2) + 1];
	}

	for ($loop = 0; $loop < $s_dr_adj_counter_index; $loop++) {
		printf "direct reclaim path for adj %d: %d\n",
			$s_dr_adj_counter[$loop * 2],
			$s_dr_adj_counter[($loop * 2) + 1];
	}

	for ($loop = 0; $loop < $s_dr_minfree_counter_index; $loop++) {
		printf "direct reclaim path for minfree %d: %d\n",
			$s_dr_minfree_counter[$loop * 2],
			$s_dr_minfree_counter[($loop * 2) + 1];
	}

	print "\n";

	#almk end

	#kswapd

	printf "kswapd and killed a task: %d\n",
		$e_ks_sigkill;

	printf "kswapd and returned because of pending kill: %d\n",
		$e_ks_return_memdie;

	printf "kswapd and returned without selecting: %d\n",
		$e_ks_didnt_select;

	print "\n";

	#direct reclaim
	printf "direct reclaim path and killed a task: %d\n",
		$e_dr_sigkill;

	printf "direct reclaim path and returned because of pending kill: %d\n",
		$e_dr_return_memdie;

	printf "direct reclaim path and returned without selecting: %d\n",
		$e_dr_didnt_select;


	print "\n";

	for ($loop = 0; $loop < $e_ks_gfp_counter_index; $loop++) {
		printf "kswapd and killed a task for gfp_mask 0x%x: %d\n",
			$e_ks_gfp_counter[$loop * 2],
			$e_ks_gfp_counter[($loop * 2) + 1];
	}

	for ($loop = 0; $loop < $e_ks_adj_counter_index; $loop++) {
		printf "kswapd and killed a task of adj %d: %d ",
			$e_ks_adj_counter[$loop * 3],
			$e_ks_adj_counter[($loop * 3) + 1];
		printf "(Total tasksize reclaimed for this adj: %d pages)\n",
			$e_ks_adj_counter[($loop * 3) + 2];
	}

	for ($loop = 0; $loop < $e_ks_minfree_counter_index; $loop++) {
		printf "kswapd and killed a task for minfree %d: %d\n",
			$e_ks_minfree_counter[$loop * 2],
			$e_ks_minfree_counter[($loop * 2) + 1];
	}

	print "\n";

	for ($loop = 0; $loop < $e_dr_gfp_counter_index; $loop++) {
		printf "direct reclaim path and killed a task for gfp_mask ";
		printf "0x%x: %d\n",
			$e_dr_gfp_counter[$loop * 2],
			$e_dr_gfp_counter[($loop * 2) + 1];
	}

	for ($loop = 0; $loop < $e_dr_adj_counter_index; $loop++) {
		printf "direct reclaim path and killed a task of adj %d: ",
			$e_dr_adj_counter[$loop * 3];
		printf "%d (Total tasksize reclaimed for this adj: ",
			$e_dr_adj_counter[($loop * 3) + 1];
		printf "%d pages)\n",
			$e_dr_adj_counter[($loop * 3) + 2];
	}

	for ($loop = 0; $loop < $e_dr_minfree_counter_index; $loop++) {
		printf "direct reclaim path and killed a task for minfree";
		printf "%d: %d\n",
			$e_dr_minfree_counter[$loop * 2],
			$e_dr_minfree_counter[($loop * 2) + 1];
	}

	return;
}

sub oom_score_adj_to_oom_adj
{
	if ($_[0] == OOM_SCORE_ADJ_MAX) {
		return OOM_ADJUST_MAX;
	} else {
		return ($_[0] * OOM_DISABLE) / OOM_SCORE_ADJ_MAX;
	}
}

sub process_end_kswapd
{
	my $loop;
	my $found = 0;

	if ($line[3] == -2) {
		$e_ks_return_memdie++;
	} elsif ($line[3] == -1) {
		$e_ks_didnt_select++;
	} else {

		$e_ks_sigkill++;

		for ($loop = 0; $loop <= $e_ks_gfp_counter_index;
				$loop++) {

			if ($e_ks_gfp_counter[$loop * 2] ==
					hex($line[4])) {
				$e_ks_gfp_counter[($loop * 2) + 1]++;
				$found = 1;
				last;
			}
		}

		if (!$found) {
			$e_ks_gfp_counter[$e_ks_gfp_counter_index * 2]
					= hex($line[4]);
			$e_ks_gfp_counter[($e_ks_gfp_counter_index * 2) + 1]
					= 1;
			$e_ks_gfp_counter_index++;
		}

		$found = 0;

		for ($loop = 0; $loop <= $e_ks_adj_counter_index; $loop++) {

			if ($line[3] == OOM_SCORE_ADJ_MAX + 1) {
				$found = 1; # to make it skip and return
				last;
			}

			if ($e_ks_adj_counter[$loop * 3]
					== oom_score_adj_to_oom_adj($line[3])) {
				$e_ks_adj_counter[($loop * 3) + 1]++;
				$e_ks_adj_counter[($loop * 3) + 2] += $line[5];
				$found = 1;
				last;
			}
		}

		if (!$found) {
			$e_ks_adj_counter[$e_ks_adj_counter_index * 3]
					= oom_score_adj_to_oom_adj($line[3]);
			$e_ks_adj_counter[($e_ks_adj_counter_index * 3) + 1]
					= 1;
			$e_ks_adj_counter[($e_ks_adj_counter_index * 3) + 2]
					= $line[5];
			$e_ks_adj_counter_index++;
		}

		$found = 0;

		for ($loop = 0; $loop <= $e_ks_minfree_counter_index; $loop++) {

			if (MAX_VAL_INT == $line[6]) {
				$found = 1; # to make it skip and return
				last;
			}

			if ($e_ks_minfree_counter[$loop * 2] == $line[6]) {
				$e_ks_minfree_counter[($loop * 2) + 1]++;
				$found = 1;
				last;
			}
		}

		if (!$found) {
			$e_ks_minfree_counter[$e_ks_minfree_counter_index * 2]
				= $line[6];
			$e_ks_minfree_counter[(
				$e_ks_minfree_counter_index * 2) + 1] = 1;
			$e_ks_minfree_counter_index++;
		}
	}

	return;
}

sub process_end_direct_reclaim
{
	my $loop;
	my $found = 0;

	if ($line[3] == -2) {
		$e_dr_return_memdie++;
	} elsif ($line[3] == -1) {
		$e_dr_didnt_select++;
	} else {
		$e_dr_sigkill++;

		for ($loop = 0; $loop <= $e_dr_gfp_counter_index; $loop++) {

			if ($e_dr_gfp_counter[$loop * 2] == hex($line[4])) {
				$e_dr_gfp_counter[($loop * 2) + 1]++;
				$found = 1;
				last;
			}
		}

		if (!$found) {
			$e_dr_gfp_counter[$e_dr_gfp_counter_index * 2]
					= hex($line[4]);
			$e_dr_gfp_counter[($e_dr_gfp_counter_index * 2) + 1]
					= 1;
			$e_dr_gfp_counter_index++;
		}

		$found = 0;

		for ($loop = 0; $loop <= $e_dr_adj_counter_index; $loop++) {

			if ($line[3] == OOM_SCORE_ADJ_MAX + 1) {
				$found = 1; # to make it skip and return
				last;
			}

			if ($e_dr_adj_counter[$loop * 3] ==
					oom_score_adj_to_oom_adj($line[3])) {
				$e_dr_adj_counter[($loop * 3) + 1]++;
				$e_dr_adj_counter[($loop * 3) + 2] += $line[5];
				$found = 1;
				last;
			}
		}

		if (!$found) {
			$e_dr_adj_counter[$e_dr_adj_counter_index * 3]
					= oom_score_adj_to_oom_adj($line[3]);
			$e_dr_adj_counter[($e_dr_adj_counter_index * 3) + 1]
					= 1;
			$e_dr_adj_counter[($e_dr_adj_counter_index * 3) + 2]
					= $line[5];
			$e_dr_adj_counter_index++;
		}

		$found = 0;

		for ($loop = 0; $loop <= $e_dr_minfree_counter_index; $loop++) {

			if (MAX_VAL_INT == $line[6]) {
				$found = 1; # to make it skip and return
				last;
			}

			if ($e_dr_minfree_counter[$loop * 2] == $line[6]) {
				$e_dr_minfree_counter[($loop * 2) + 1]++;
				$found = 1;
				last;
			}
		}

		if (!$found) {
			$e_dr_minfree_counter[$e_dr_minfree_counter_index * 2]
				= $line[6];
			$e_dr_minfree_counter[(
				$e_dr_minfree_counter_index * 2) + 1] = 1;
			$e_dr_minfree_counter_index++;
		}
	}

	return;
}

sub process_start_kswapd
{
	my $loop;
	my $found = 0;

	$total_ks_count += 1;

	if ($line[3] == 0) {
		$ks_zero_nr_to_scan += 1;
	} else {
		$ks_nonzero_nr_to_scan += 1;
	}

	for ($loop = 0; $loop <= $s_ks_gfp_counter_index; $loop++) {

		if ($s_ks_gfp_counter[$loop * 2] == hex($line[4])) {
			$s_ks_gfp_counter[($loop * 2) + 1]++;
			$found = 1;
			last;
		}
	}

	if (!$found) {
		$s_ks_gfp_counter[$s_ks_gfp_counter_index * 2] = hex($line[4]);
		$s_ks_gfp_counter[($s_ks_gfp_counter_index * 2) + 1] = 1;
		$s_ks_gfp_counter_index++;
	}

	$found = 0;

	for ($loop = 0; $loop <= $s_ks_adj_counter_index; $loop++) {

		if ($line[5] == OOM_SCORE_ADJ_MAX + 1) {
			$found = 1; # to make it skip and return
			last;
		}

		if ($s_ks_adj_counter[$loop * 2] ==
				oom_score_adj_to_oom_adj($line[5])) {
			$s_ks_adj_counter[($loop * 2) + 1]++;
			$found = 1;
			last;
		}
	}

	if (!$found) {
		$s_ks_adj_counter[$s_ks_adj_counter_index * 2] =
			oom_score_adj_to_oom_adj($line[5]);
		$s_ks_adj_counter[($s_ks_adj_counter_index * 2) + 1] = 1;
		$s_ks_adj_counter_index++;
	}

	$found = 0;

	for ($loop = 0; $loop <= $s_ks_minfree_counter_index; $loop++) {

		if (MAX_VAL_INT == $line[6]) {
			$found = 1; # to make it skip and return
			last;
		}

		if ($s_ks_minfree_counter[$loop * 2] == $line[6]) {
			$s_ks_minfree_counter[($loop * 2) + 1]++;
			$found = 1;
			last;
		}
	}

	if (!$found) {
		$s_ks_minfree_counter[$s_ks_minfree_counter_index * 2]
				= $line[6];
		$s_ks_minfree_counter[($s_ks_minfree_counter_index * 2) + 1]
				= 1;
		$s_ks_minfree_counter_index++;
	}

	return;
}

sub process_start_direct_reclaim
{
	my $loop;
	my $found = 0;

	$total_dr_count += 1;

	if ($line[3] == 0) {
		$dr_zero_nr_to_scan += 1;
	} else {
		$dr_nonzero_nr_to_scan += 1;
	}

	for ($loop = 0; $loop <= $s_dr_gfp_counter_index; $loop++) {

		if ($s_dr_gfp_counter[$loop * 2] == hex($line[4])) {
			$s_dr_gfp_counter[($loop * 2) + 1]++;

			$found = 1;
			last;
		}
	}

	if (!$found) {
		$s_dr_gfp_counter[$s_dr_gfp_counter_index * 2] = hex($line[4]);
		$s_dr_gfp_counter[($s_dr_gfp_counter_index * 2) + 1] = 1;
		$s_dr_gfp_counter_index++;
	}

	$found = 0;

	for ($loop = 0; $loop <= $s_dr_adj_counter_index; $loop++) {

		if ($line[5] == OOM_SCORE_ADJ_MAX + 1) {
			$found = 1; # to make it skip and return
			last;
		}

		if ($s_dr_adj_counter[$loop * 2] ==
				oom_score_adj_to_oom_adj($line[5])) {
			$s_dr_adj_counter[($loop * 2) + 1]++;
			$found = 1;
			last;
		}
	}

	if (!$found) {
		$s_dr_adj_counter[$s_dr_adj_counter_index * 2] =
				oom_score_adj_to_oom_adj($line[5]);
		$s_dr_adj_counter[($s_dr_adj_counter_index * 2) + 1] = 1;
		$s_dr_adj_counter_index++;
	}

	$found = 0;

	for ($loop = 0; $loop <= $s_dr_minfree_counter_index; $loop++) {

		if (MAX_VAL_INT == $line[6]) {
			$found = 1; # to make it skip and return
			last;
		}

		if ($s_dr_minfree_counter[$loop * 2] == $line[6]) {
			$s_dr_minfree_counter[($loop * 2) + 1]++;
			$found = 1;
			last;
		}
	}

	if (!$found) {
		$s_dr_minfree_counter[$s_dr_minfree_counter_index * 2]
				= $line[6];
		$s_dr_minfree_counter[($s_dr_minfree_counter_index * 2) + 1]
				= 1;
		$s_dr_minfree_counter_index++;
	}

	return;
}

sub process_almk_start
{
	$almk_trigger_count += 1;

	if ($line[2] =~ m/kswapd/) {
		process_start_kswapd();
	} else {
		process_start_direct_reclaim();
	}

	return;
}

sub process_almk_end
{
	if ($line[2] =~ m/kswapd/) {
		process_end_kswapd();
	} else {
		process_end_direct_reclaim();
	}

	return;
}

sub process_lines
{
	@line = split(':',$_);

	if (m/almk_start/) {
		process_almk_start();
	} elsif (m/almk_end/) {
		process_almk_end();
	} else {
		#print "Unknown lines present in the trace file\n";
		#print $_;
	}

	return;
}

sub usage
{
	print "Usage:\n";
	print "trace-almk-postprocess.pl [trace file to be parsed]\n";
	return;
}

if (($#ARGV < 0) || ($#ARGV > 1)) {
	usage();
	die "Input proper arguments\n"
}

$file = $ARGV[0];
open($FILE_HANDLE, "<", $file) or die $!;

while (<$FILE_HANDLE>) {
	process_lines();
}

close($FILE_HANDLE);

report();
