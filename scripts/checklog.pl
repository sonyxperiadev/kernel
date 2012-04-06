#!/usr/bin/perl -w
# Copyright 2010 -- 2012 Broadcom Corporation.
#
# Unless you and Broadcom execute a separate written software license
# agreement governing use of this software, this software is licensed to you
# under the terms of the GNU General Public License version 2, available at
# http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
use strict;

my $P = $0;
$P =~ s@.*/@@g;

my $V = '0.10';

use Getopt::Long qw(:config no_auto_abbrev);

my $subject = 0;
my $body = 0;
my $terse = 0;
my $quiet = 0;
my $summary = 1;
my %debug;
my $help = 0;
my $pre_log_line = "";
my $section = 0;

my $COLUMN = 80;

sub help {
	my ($exitcode) = @_;

	print << "EOM";
Usage: $P [OPTION]... [FILE]...
Version: $V

Options:
	-s, --subject         check single subject of git commit log message
	-b, --body            check single body of git commit log message
	--summary-file        include the filename in summary
	--debug KEY=[0|1]     turn on/off debugging of KEY, where KEY is one of
				'log', 'msg', 'type', and 'attr' (default
				is all off)
	-h, --help, --version display this help and exit

When FILE is - read standard input.
EOM

	exit($exitcode);
}

GetOptions(
	's|subject!'	=> \$subject,
	'b|body'	=> \$body,
	'summary!'	=> \$summary,
	'debug=s'	=> \%debug,
	'h|help'	=> \$help,
	'version'	=> \$help
) or help(1);

help(0) if ($help);

my $exit = 0;

if ($#ARGV < 0) {
	print "$P: no input files\n";
	exit(1);
}

my $dbg_log = 0;
my $dbg_msg = 0;
my $dbg_type = 0;
my $dbg_attr = 0;
for my $key (keys %debug) {
	## no critic
	eval "\${dbg_$key} = '$debug{$key}';";
	die "$@" if ($@);
}

my @rawlines = ();
my @lines = ();
my $vname;
for my $filename (@ARGV) {
	my $FILE;
	if ($filename eq '-') {
		open($FILE, '<&STDIN');
	} else {
		open($FILE, '<', "$filename") ||
			die "$P: $filename: open failed - $!\n";
	}
	if ($filename eq '-') {
		$vname = 'Your patch';
	} else {
		$vname = $filename;
	}
	while (<$FILE>) {
		chomp;
		push(@rawlines, $_);
	}
	close($FILE);
	if (!process($filename)) {
		$exit = 1;
	}
	@rawlines = ();
	@lines = ();
}
exit($exit);

my $prefix = '';
my @commit_head = ();
my @subjectstr = ();
my @bodystr = ();

sub report {
	my $line = $prefix . $_[0];

	push(our @report, $line);

	return 1;
}

sub report_dump {
	our @report;
}

sub ERROR {
	if (report("ERROR: $_[0]\n")) {
		our $clean = 0;
		our $cnt_error++;
	}
}

sub WARN {
	if (report("WARNING: $_[0]\n")) {
		our $clean = 0;
		our $cnt_warn++;
	}
}

sub print_commit_head {
	$prefix = "";
	report("=========================================================\n");
	foreach my $line (@commit_head) {
		report("$line\n");
	}
	report("\n");
}

sub print_commit_subject {
	$prefix = "";
	foreach my $line (@subjectstr) {
		report("$line\n");
	}
	report("\n");
}

sub print_commit_body {
	$prefix = "";
	foreach my $line (@bodystr) {
		report("$line\n");
	}
	report("\n");
}

sub parse_commit_subject {
	my $line_cnt = 0;
	my $sub_string = "";

	foreach my $line (@subjectstr) {
		$line_cnt++;
		$sub_string .= $line;
	}
	check_subject($sub_string);
}

sub check_subject {
	my ($line) = @_;
	my $realline;

	$prefix = "SUBJECT ";
	$realline = "$line\n";

	if ( length($line) < 20) {
		ERROR("The length of one-line summary is too short\n"
		. $realline);
	}
	if ( $line =~ /^\s/) {
		ERROR("Space or tab is prohibited at the beginning of "
		. "subject line\n" . $realline);
	} elsif ( $line =~ /^\W/) {
		ERROR("Any character not a word charater is prohibited at"
		. "the start of subject line\n" . $realline);
	}
	if ( $line =~ /\:\s\s/) {
		ERROR("One whitespace is allowed after : symbol for"
		. " component name\n" . $realline);
	}
	if ( $line =~ /\:\S/) {
		if (!($line =~ /^Merge\ branch\ / ||
			$line =~ /^Revert\ \"/ ||
			$line =~ /ssh\:/ ||
			$line =~ /http\:/ ||
			$line =~ /https\:/)) {
		    ERROR("Add whitespace after : symbol for component name\n"
		    . $realline);
		}
	}
	if (!($line =~ /\:/)) {
		WARN("Add component name iterated by : symbol ex)component:"
		. " summary of commit\n" . $realline);
	}
	if ( length($line) > $COLUMN) {
		WARN("Subject line is larger than $COLUMN column. "
		. "About 50 charaters limit recommended.\n"
		. $realline);
	}
	$prefix = "";
}

sub parse_commit_body {
	$pre_log_line = "";
	foreach my $line (@bodystr) {
		check_body($line);
	}
}

sub check_body {
	my ($line) = @_;
	my $realline;
	my $no_space_line = "";

	$prefix = "";
	$realline = "$line\n";

	$no_space_line = $line;
	$no_space_line =~ s/\ //g;
	$no_space_line =~ s/\t//g;

	if ( length($line) > $COLUMN) {
		WARN("Commit log message line is larger than $COLUMN column. "
		. "72 charaters limit recommended.\n"
		. $realline);
	}

	if ($section > 0) {
		if (length($line) == 0) {
			$prefix = $pre_log_line . "\n";
			WARN("No empty line is allowed after section\n"
			. $realline);
		} elsif ($line =~ /^\s\s/ && $section != 5) {
			WARN("No space or one space is recommended "
			. "at the beginning of section description\n"
			. $realline);
		}
	}

	if ($section == 1 && $no_space_line =~ /MobC/) {
		if ( !($no_space_line =~ /^MobC/)) {
			WARN("Please use proper CQ number style after "
			. "[Defect/Enhancement/New Feature] section. ex)"
			. "MobC00123456\n" . $realline);
		}
	}

	if ($no_space_line =~ /^\[Defect\/Enhancement\/NewFeature\]$/ ||
		$no_space_line =~ /^\[Problem\]$/ ||
		$no_space_line =~ /^\[Solution\]$/ ||
		$no_space_line =~ /^\[Reviewers\]$/ ||
		$no_space_line =~ /^\[Testing\]$/ ) {
		if(length($pre_log_line) > 0){
			WARN("Add blank line before new section\n"
			. $realline);
		}
		if ($line =~ /\s\[/) {
			ERROR("Space or tab is not allowed before [\n"
			 . $realline);
		}
		if ($line =~ /\[\s/) {
			ERROR("Space or tab is not allowed after [\n"
			. $realline);
		}
		if ($line =~ /\]\s/) {
			ERROR("Space or tab is not allowed after ]\n"
			. $realline);
		}
		if ($line =~ /\s\]/) {
			ERROR("Space or tab is not allowed before ]\n"
			. $realline);
		}
		#1 : [Defect/Enhancement/NewFeature\]
		#2 : [Problem]
		#3 : [Solution]
		#4 : [Reviewers]
		#5 : [Testing]
		#6 : Other
		if ($no_space_line =~ /\[Defect\/Enhancement\/NewFeature\]/) {
			$section = 1;
		} elsif ($no_space_line =~ /\[Testing\]/) {
			$section = 5;
		} else {
			$section = 6;
		}
	} else {
		$section = 0;
	}

	$pre_log_line = $line;
}

sub process {
	my $filename = shift;
	my $linenr=0;
	our $clean = 1;
	my $new_commit = 0;
	my $new_commit_cnt = 0;
	my $is_subject = 0;
	my $is_body = 0;
	my $multi_log = 0;
	my $pre_multi_log = 0;

	our @report = ();
	our $cnt_error = 0;
	our $cnt_warn = 0;
	our $cnt_chk = 0;

	$pre_log_line = "";
	$prefix = "";
	$section = 0;
	my $line;

	foreach my $rawline (@rawlines) {
		$linenr++;
		$line = $rawline;
		if ($subject == 0 && $body == 0) {
			if ($line =~ /^commit\ /) {
				if ($multi_log > 0 &&
					$multi_log != $pre_multi_log) {
					$pre_multi_log = $multi_log;
					if ($dbg_log == 1) {
						print_commit_head();
						print_commit_subject();
						print_commit_body();
					}
					report($commit_head[0] . " <===\n\n");
					parse_commit_subject();
					parse_commit_body();

					@commit_head = ();
					@subjectstr = ();
					@bodystr = ();
				}
				$multi_log++;

				$new_commit = 0;
				$is_subject = 0;
				$is_body = 0;
				$new_commit_cnt = 1;
				push(@commit_head, $line);
			} elsif ($new_commit_cnt == 1 &&
					$line =~ /^Author\:\ */) {
				$new_commit_cnt++;
				push(@commit_head, $line);
			} elsif ($new_commit_cnt == 2 &&
					$line =~ /^Date\: \ */) {
				$new_commit_cnt++;
				push(@commit_head, $line);
			} elsif ($new_commit_cnt == 3 &&
					length($line) == 0) {
				$new_commit_cnt++;
			} elsif ($new_commit_cnt == 4 &&
					$line =~ /^\ \ \ \ */) {
				$new_commit_cnt++;
				$new_commit = 1;
				$is_subject = 1;
			}

			if ($new_commit == 1 && $line =~ /^diff\ \-\-git\ /) {
				$is_subject = 0;
				$is_body = 0;
			} elsif ($new_commit == 1 && $is_subject == 1 &&
				$line =~ /^\ \ \ \ */) {
				if ($line =~ /^\ \ \ \ $/) {
					$is_subject = 0;
					$is_body = 1;
				} else {
					$line =~ s/^\ \ \ \ //g;
					push(@subjectstr, $line);
				}
			} elsif ($new_commit == 1 && $is_body == 1 &&
				$line =~ /^\ \ \ \ */) {
				$line =~ s/^\ \ \ \ //g;
				push(@bodystr, $line);
			}

		} elsif ($subject == 1 && $body == 0) {
			push(@subjectstr, $line);
		} elsif ($subject == 0 && $body == 1) {
			push(@bodystr, $line);
		}
	}

	if ($dbg_log == 1) {
		print_commit_head();
		print_commit_subject();
		print_commit_body();
	}

	if ($multi_log > 1) {
		report($commit_head[0] . " <===\n\n");
		parse_commit_subject();
		parse_commit_body();
	} else {
		if (($subject == 0 && $body == 0) ||
		    ($subject == 1 && $body == 0)){
			parse_commit_subject();
		}
		if (($subject == 0 && $body == 0) ||
			($subject == 0 && $body == 1)){
			parse_commit_body();
		}
	}

	@commit_head = ();
	@subjectstr = ();
	@bodystr = ();

	print report_dump();
	if ($summary && !($clean == 1 && $quiet == 1)) {
		print "total: $cnt_error errors, $cnt_warn warnings.\n";
		print "\n" if ($quiet == 0);
	}

	if ($clean == 1 && $quiet == 0) {
		print "$vname has no obvious commit message style problems.\n"
	}
	if ($clean == 0 && $quiet == 0) {
		print "$vname has commit message style problems. "
		. "Please review and follow commit message guide line.\n";
		print "http://confluence.broadcom.com/display/"
			. "ANDROIDPHONESW/Git+commit+message+guideline\n";
	}

	return $clean;
}
