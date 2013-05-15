#!/usr/bin/perl

# scripts/defconfig.pl
#
# Copyright (C) 2011 Sony Ericsson Mobile Communications AB.
#
# Author: Martin Danielsson <martin.danielsson@sonyericsson.com>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2, as
# published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.

use strict;
use warnings;

use Cwd 'abs_path';
use File::Copy;

my @supported_platforms = ("fusion3");
my $kernel_dir;
my $config_dir;
my @products = ();

# Print usage instructions
#
# In: Error message to display
sub usage($)
{
	my ($msg) = @_;

	print "\n";
	print "Error: $msg\n\n" if ($msg ne "");

	print "This script is used to perform one or more tasks on a set of\n";
	print "defconfig files. The result will be the same as if the\n";
	print "product configuration was done using menuconfig or similar\n";
	print "tool.\n\n";

	print "Usage: ";
	print "autoconfig.pl -p <platform> -t <action>[:<key>[=<value>]] ";
	print "<products>\n\n";

	print "<platform> - set the platform\n";
	print "\tall of the following platforms are set as default\n";
	print "\t@supported_platforms\n";
	print "<action> - action to perform\n";
	print "\ts: sync defconfig with Kconfig (s)\n";
	print "\ta: set a configuration (a:<key>=<value>)\n";
	print "\td: unset a configuration (d:<key>)\n";
	print "<key> - a configuration flag, for instance CONFIG_SWAP\n";
	print "<value> - the value to set, for instance y, n or 1000\n";
	print "<products> - one or more of the following products\n";
	opendir DH, $config_dir or exit 0;
	foreach (readdir DH) {
		if (/([0-9a-zA-Z]+)_([\w-]+)_defconfig/) {
			if ((is_platform_supported($1) == 1) &&
				(is_product_excluded($2) == 0)) {
				print "\t$2\n";
			}
		}
	}

	print "\n";
	closedir DH;

	print "Examples:\n";
	print "\tSync all defconfigs with Kconfig\n";
	print "\t\$ autoconfig.pl -t s\n\n";
	print "\tEnable CONFIG_SWAP for Anzu and Hallon\n";
	print "\t\$ autoconfig.pl -t a:CONFIG_SWAP=y anzu hallon\n\n";
	print "\tDisable CONFIG_USB_SUPPORT for all products\n";
	print "\t\$ autoconfig.pl -t d:CONFIG_USB_SUPPORT\n\n";
	print "\tSet CONFIG_MSM_AMSS_VERSION to 1000 for Anzu\n";
	print "\t\$ autoconfig.pl -t a:CONFIG_MSM_AMSS_VERSION anzu\n";
	print "\n";
}

# Apply a modification to a defconfig file
#
# In: The key
# In: The value
# In: The defconfig
sub apply_modification($$$)
{
	my ($key, $value, $defconfig) = @_;

	# Settings are applied by inserting them at the end of the defconfig
	# file. The kernel build system will later move it to the correct
	# location and handle any duplicate entries. It will also perform
	# validation of integer values.
	my $file = ".config";
	open DEFCONFIG, ">>$file" or die "Failed to open file, $file";
	if ($value eq "n") {
		print DEFCONFIG "\n# $key is not set\n";
	} else {
		print DEFCONFIG "\n$key=$value\n";
	}
	close DEFCONFIG;
}

# Perform a task on a specific defconfig
#
# In: The action
# In: The key
# In: The value
# In: The defconfig
sub perform_task($$$$)
{
	my ($action, $key, $value, $defconfig) = @_;

	if ($action eq "s") {
		print "Syncing $defconfig ...\n";
	} elsif ($action eq "a") {
		print "Setting $key=$value in $defconfig ...\n";
	} elsif ($action eq "d") {
		print "Removing $key from $defconfig ...\n";
	}


	my $option ="KCONFIG_NOTIMESTAMP=true";
	system "make ARCH=arm `basename $defconfig` > /dev/null 2>&1";
	if ($action ne "s") {
		apply_modification($key, $value, $defconfig);
	}
	system "make ARCH=arm savedefconfig $option > /dev/null";
	move("defconfig", "$config_dir/$defconfig")
		or die "failed to copy file";
}

# Parse a task description string
#
# In: The task description string
# Out: A ($action, $key, $value) array. In case of error $action is set to "-"
sub parse_task_description($)
{
	my ($task) = @_;

	my $action = "";
	my $key = "";
	my $value = "";
	if ($task =~ /^([sda])/) {
		$action = $1;
	} else {
		print "unknown action, $task\n";
		return ("-", "", "");
	}
	my $error = 1;
	if ($action eq "s") {
		$error = 0;
	} elsif ($action eq "a") {
		if ($task =~ /^a:(\w+)=(.+)/) {
			$key = $1;
			$value = $2;
			$error = 0;
		}
	} elsif ($action eq "d") {
		if ($task =~ /^d:(\w+)/) {
			$key = $1;
			$value = "n";
			$error = 0;
		}
	}
	if ($error == 1) {
		print "incorrect task description, $task\n";
		return ("-", "", "");
	}

	return ($action, $key, $value);
}

# Assuming a naming convention of <platform>_<product>_defconfig,
# check if the platform is one that is supported by this script.
#
# In: The platform to check
# Out: 1 if the platform is supported, 0 otherwise
sub is_platform_supported($)
{
	my ($platform) = @_;

	foreach (@supported_platforms) {
		if ($platform eq $_) {
			return 1;
		}
	}

	return 0;
}

# Check if a product should be excluded
#
# In: The product to check
# Out: 1 if the product is excluded, 0 otherwise
sub is_product_excluded($)
{
	my ($product) = @_;

	if ($product =~ /_capk/) {
		return 1;
	}

	return 0;
}

# Check if a product has been selected for processing
#
# In: The product to check
# Out: 1 if the product selected, 0 otherwise
sub is_product_selected($)
{
	my ($product) = @_;

	if (@products == 0) {
		return 1;
	}
	foreach (@products) {
		if ($product eq $_) {
			return 1;
		}
	}

	return 0;
}

### Program starts here ###

# Figure out the path to the kernel directory and move to it
$kernel_dir = abs_path($0);
$kernel_dir =~ s!/scripts/.*\.pl!!;
chdir $kernel_dir or die "couldn't move to kernel directory, $kernel_dir\n";
$config_dir = "$kernel_dir/arch/arm/configs";

# Parse command line arguments
my @tasks = ();
my $iter = 0;
my $flag = 0;
while ($iter < @ARGV) {
	if ($ARGV[$iter] eq "-t") {
		$iter++;
		if ($iter >= @ARGV) {
			usage("not enough arguments");
			exit 0;
		}
		push @tasks, $ARGV[$iter];
	} elsif ($ARGV[$iter] eq "-p") {
		if ($flag == 0 ) {
		    @supported_platforms = ();
		    $flag = 1;
		}
		$iter++;
		if ($iter >= @ARGV) {
			usage("not enough arguments");
			exit 0;
		}
		push @supported_platforms, $ARGV[$iter];
	} elsif ($ARGV[$iter] eq "-h") {
		usage("");
		exit 0;
	} else {
		push @products, $ARGV[$iter];
	}
	$iter++;
}

# Find all relevant defconfig files
opendir DH, $config_dir or die "couldn't open config directory, $config_dir\n";
my @defconfigs;
foreach (readdir DH) {
	if (/([0-9a-zA-Z]+)_([\w-]+)_defconfig/) {
		if ((is_platform_supported($1) == 1) &&
			(is_product_selected($2) == 1) &&
			(is_product_excluded($2) == 0)) {
			push @defconfigs, $_;
		}
	}
}
closedir DH;

# Apply the tasks one at a time for each selected product
if (@tasks == 0) {
	usage("no task specified");
	exit 0;
}
my $task;
my $defconfig;
foreach $task (@tasks) {
	my ($action, $key, $value) = parse_task_description($task);
	next if ($action eq "-");
	foreach $defconfig (@defconfigs) {
		perform_task($action, $key, $value, $defconfig);
	}
}

