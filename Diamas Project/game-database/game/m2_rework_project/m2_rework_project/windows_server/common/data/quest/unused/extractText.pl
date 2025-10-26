#!/usr/bin/perl 
use strict;
use warnings;

#
# Right number of arguments?
#
if (scalar @ARGV != 1) {
    die "Wrong number of arguments!\nUsage: <file>\n";
}
my ($file) = @ARGV;

#
# Open the file and declear the basic vars 
#
open IN_FILE, "< $file" or die "Could not open: $!\nFile: $file\n";
open NQ_FILE, "> $file.new" or die "Could not open: $!\nFile: $file.new\n";

my $questName = $file;
$questName =~ s/[^\/]*\/[^\/]*\/([^\/]*)\.lua$/$1/;
my $luaVarBase = "gameforge.$questName";
my $luaCnt = 0;
my %savedKeys = ();
my $lastBlockType = "";
my $lastBlockText = "";
my $lastPrefix = "";
my $key = "";
my $value = "";
my $prefix = "";
my $postfix = "";
my $selectFirst = 0;
my $stringFormat = 0;
my $autoFormat = 0;
my @autoFormat = ();
my $temp = "";
my $firstEntry = 0;

print "Parsing: $questName...\n";

open LUA_FILE, "< extractQuest.lua" or die "Could not open: $!\nFile: $file.lua\n";
while (<LUA_FILE>) {
	if ($_ =~ m/([^=]*) = "(.*)"/) {
		$savedKeys{$2} = $1;
	}
}
close LUA_FILE;

open LUA_FILE, ">> extractQuest.lua" or die "Could not open: $!\nFile: $file.lua\n";

#
# Some functions to organize everything
# 
sub writeToLua() {
	# Remove last [ENTER]. 
	# These "wrong" [ENTER]s are from empty-format messages after the block!
	$lastBlockText =~ s/\[ENTER\]$//;
	$lastBlockText =~ s/" \.\."//;	
	$lastBlockText =~ s/"\.\."//;	

	$autoFormat = 0;
	@autoFormat = ();
	while ($lastBlockText =~ m/" *(\.\.|,)(.*)(\.\.|,) *"/) {
		$autoFormat = 1;
		push(@autoFormat, $2);
		$temp = quotemeta($2);
		$lastBlockText =~ s/" *$1$temp$3 *"/%s/;			
	}

	my $counter = 0;

	if ($lastBlockText eq "") {
		return;
	}

	if (defined $savedKeys{$lastBlockText}) {
		$counter = $savedKeys{$lastBlockText};
	} else {
		$luaCnt = $luaCnt + 10;
		$counter = "_" . $luaCnt . "_" . $lastBlockType;

		if ($lastBlockType eq "test_chat") {
			$counter = "_" . $luaCnt . "_testChat";
		}
		if ($lastBlockType eq "string.format") {
			$counter = "_" . $luaCnt . "_stringFormat";
		}
		if ($lastBlockType eq "send_letter") {
			$counter = "_" . $luaCnt . "_sendLetter";
		}
		if ($lastBlockType eq "q.set_title") {
			$counter = "_" . $luaCnt . "_qSetTitle";
		}
		if ($lastBlockType eq "d.notice") {
			$counter = "_" . $luaCnt . "_dNotice";
		}
		if ($lastBlockType eq "say_reward") {
			$counter = "_" . $luaCnt . "_sayReward";
		}
		if ($lastBlockType eq "say_title") {
			$counter = "_" . $luaCnt . "_sayTitle";
		}
				
		$counter = "$luaVarBase.$counter";
		$savedKeys{$lastBlockText} = $counter;

		if ($firstEntry eq 0) {
			$firstEntry = 1;
			print LUA_FILE "$luaVarBase = {}\n";
		}
		print LUA_FILE "$counter = \"$lastBlockText\"\n"; 
	}
	if ($lastBlockText ne '') {
		if ($autoFormat eq 1) {
			$counter = "string.format($counter, " . join(', ', @autoFormat) . ")";
		}

		if ($lastBlockType eq "select") {
			print NQ_FILE $counter;
		} elsif ($lastBlockType eq "npcChat") {
			print NQ_FILE $counter;
		} elsif ($lastBlockType eq "sayItem") {
			print NQ_FILE $counter;
		} elsif ($lastBlockType eq "targetNpc") {
			print NQ_FILE $counter;
		} elsif ($lastBlockType eq "confirm") {
			print NQ_FILE $counter;
		} elsif ($lastBlockType eq "targetVid") {
			print NQ_FILE $counter;
		} elsif ($lastBlockType eq "tableInsert") {
			print NQ_FILE $counter;
		} elsif ($lastBlockType eq "string.format") {
			print NQ_FILE "$lastBlockType($counter";
		} else {
			if ($stringFormat eq 1) {
				print NQ_FILE "$lastBlockType(string.format($counter";
			} else {
				print NQ_FILE "$lastBlockType($counter)\n";
			}
		}
	}
}

#
# Main work starts here!
#
while (<IN_FILE>) {	
	if ($_ =~ m/^ *--/) {
		# IGNORE!
	} elsif ($_ =~ m/([a-zA-Z_\.]+)\( *string\.format *\( *["']{1}(.*)["']{1} *(,.*)/) {
		if ($lastBlockType) {
			print NQ_FILE $lastPrefix;
			writeToLua();
			$lastBlockType = "";
			$lastPrefix = "";
			$lastBlockText = "";
		}
		$stringFormat = 1;
		$lastBlockType = $1;
		$lastBlockText = $2;
		writeToLua();
		print NQ_FILE "$3\n";
		$stringFormat = 0;
		$lastBlockType = "";
	} elsif ($_ =~ m/(.*)string\.format *\( *["']{1}(.*)["']{1} *(,.*)/) {
		if ($lastBlockType) {
			print NQ_FILE $lastPrefix;
			writeToLua();
			$lastPrefix = "";
			$lastBlockType = "";
			$lastBlockText = "";
		}
		$lastBlockType = "string.format";
		$lastBlockText = $2;
		print NQ_FILE "$1";
		writeToLua();
		print NQ_FILE "$3\n";
		$lastBlockType = "";
	} elsif ($_ =~ m/(.*)say_item *\( *"(.*)"( *,.*)/) {
		if ($lastBlockType) {
			print NQ_FILE $lastPrefix;
			writeToLua();
			$lastPrefix = "";
			$lastBlockType = "";
			$lastBlockText = "";
		}
		$lastBlockType = "sayItem";
		$lastBlockText = $2;
		print NQ_FILE $1 . "say_item(";
		writeToLua();
		print NQ_FILE "$3\n";
		$lastBlockType = "";
	} elsif ($_ =~ m/(.*target\.npc([^,]*,[^,]*,)) *"(.*)"(.*)/) {
		if ($lastBlockType) {
			print NQ_FILE $lastPrefix;
			writeToLua();
			$lastPrefix = "";
			$lastBlockType = "";
			$lastBlockText = "";
		}
		$lastBlockType = "targetNpc";
		$lastBlockText = $3;
		print NQ_FILE "$1 ";
		writeToLua();
		print NQ_FILE "$4\n";
		$lastBlockType = "";
	} elsif ($_ =~ m/(.*confirm([^,]*,)) *"(.*)"(.*)/) {
		if ($lastBlockType) {
			print NQ_FILE $lastPrefix;
			writeToLua();
			$lastPrefix = "";
			$lastBlockType = "";
			$lastBlockText = "";
		}
		$lastBlockType = "confirm";
		$lastBlockText = $3;
		print NQ_FILE "$1 ";
		writeToLua();
		print NQ_FILE "$4\n";
		$lastBlockType = "";
	} elsif ($_ =~ m/(.*target\.vid([^,]*,[^,]*,)) *"(.*)"(.*)/) {
		if ($lastBlockType) {
			print NQ_FILE $lastPrefix;
			writeToLua();
			$lastPrefix = "";
			$lastBlockType = "";
			$lastBlockText = "";
		}
		$lastBlockType = "targetVid";
		$lastBlockText = $3;
		print NQ_FILE "$1 ";
		writeToLua();
		print NQ_FILE "$4\n";
		$lastBlockType = "";
	} elsif ($_ =~ m/(.*table\.insert([^,]*,)) *"(.*)"(.*)/) {
		if ($lastBlockType) {
			print NQ_FILE $lastPrefix;
			writeToLua();
			$lastPrefix = "";
			$lastBlockType = "";
			$lastBlockText = "";
		}
		$lastBlockType = "tableInsert";
		$lastBlockText = $3;
		print NQ_FILE "$1 ";
		writeToLua();
		print NQ_FILE "$4\n";
		$lastBlockType = "";
	} elsif ($_ =~ m/(.*[0-9]*\.chat\.) *"([^"]*)"(.*)/) {
		if ($lastBlockType) {
			print NQ_FILE $lastPrefix;
			writeToLua();
			$lastPrefix = "";
			$lastBlockType = "";
			$lastBlockText = "";
		}
		$lastBlockType = "npcChat";
		$lastBlockText = $2;
		print NQ_FILE "$1";
		writeToLua();
		print NQ_FILE "$3\n";
		$lastBlockType = "";
	} elsif ($_ =~ m/(.*)select *\((.*)\)(.*)/) {
		if ($lastBlockType) {
			print NQ_FILE $lastPrefix;
			writeToLua();
			$lastPrefix = "";
			$lastBlockType = "";
			$lastBlockText = "";
		}
		$selectFirst = 0;
		$prefix = $1;
		$postfix = $3;

		$lastBlockType = "select";
		print NQ_FILE "$prefix select(";

		$value = $2 . " ";

		foreach ($value =~ m/( *"[^"]*"|[^,]*) *,?(.*)/) {
			if ($selectFirst eq 0) {
				$selectFirst = 1;
			} else {
				print NQ_FILE ", ";
			}

			if ($lastBlockText eq $1) {
				$prefix = $value;
				$value = "";
			} else {
				$prefix = $1;
				$value = $2;
			}
		
			if ($prefix =~ m/["'](.*)["']/) {
				$lastBlockText = $1;
				writeToLua();
			} else {
				print NQ_FILE $prefix;
			}
		}

		print NQ_FILE ")$postfix\n";
		$lastBlockType = "";
	} elsif ($_ =~ m/(.*)(chat|notice_all|test_chat|d\.notice|q\.set_title|send_letter|makequestbutton|say|say_title|say_reward) *\( *(.*["']{1}.*["']{1}.*) *\)/) {
		# We found text to extract!
		$prefix = $1;
		$key = $2;
		$value = $3;

		if (substr($value, 0, 1) ne '"') {
			$value = '""..' . $value;
		}		
		if (substr($value, length($value) - 1, 1) ne '"') {
			$value = $value . '..""';
		}	
		$value =~ s/^"//;
		$value =~ s/"$//;

		if ($lastBlockType) {
			# We are currently parsing a block!
			if ($lastBlockType eq $key) {
				$lastPrefix = $prefix;

				# We are sill in our block!
				if ($lastBlockText eq "")  {
					# There are some empty format-messages out there!
					$lastBlockText = $value;
				} else {
					# We add the text to our "savebox"
					$lastBlockText = "$lastBlockText [ENTER]$value";
				}
			} else {
				# We found text! But from a diffrent kind!
				print NQ_FILE $lastPrefix;
				writeToLua(); 
				$lastBlockText = $value;
				$lastBlockType = $key;
				$lastPrefix = $prefix;
			}
		} else {
			# Start the block-modus!
			$lastPrefix = $prefix;
			$lastBlockType = $key;
			$lastBlockText = $value;
		}
	} else {
		if ($_ =~ m/^ *$/) {
			print NQ_FILE $_;
		} else {
			# We reach the end of our textblock! 
			if ($lastBlockType) {
				print NQ_FILE $lastPrefix;
				writeToLua();
				$lastBlockText = "";
				$lastPrefix = "";
				$lastBlockType = "";
			} 
			print NQ_FILE $_;
		}
	}
}
