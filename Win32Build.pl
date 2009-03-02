### reads in Makefile.am's and attempts to build on Win32
###

use strict;
use Getopt::Long;
use Cwd qw(cwd);
use Carp qw(confess);

### change these or reply to prompts
$ENV{PATH}   .= ';\MinGW\bin';
$ENV{INC}    .= ';\MinGW\include';
$ENV{LIB}    .= ';\MinGW\lib';
$ENV{APPATH} .= ';\Program Files\Apache Software Foundation\Apache*';
$ENV{LDADD}   = 'libstdc++.a' unless $ENV{LDADD};

### read in the 'nix "configure" command
my $unixconf = grab('configure');
die "Need 'configure', even if i'm not going to run it\n" unless $unixconf;

### extract package version, add to defs
my @def;
if ($unixconf =~ m/(PACKAGE_VERSION=[^\n]+)/) {
	push @def, $1;
} else {
	die "Can't get PACKAGE_VERSION from configure";
}

push @def, 'HAVE_SQLITE3_H';
push @def, 'HAVE_OPENSSL';

### find compiler
my $cl;
$cl = lookfor("g++.exe") if !$cl;

die "Can't find g++.exe" unless $cl;

### find linker
my $ld;
$ld = lookfor("ld.exe") if !$ld;

die "Can't find ld.exe" unless $ld;

my @inc;

### look for various directories, prompt if necessary
my $APACHEDIR = lookfor('include' , $ENV{APPATH}, prompt=>'Apache Directory', pathonly=>1);

push @inc, "$APACHEDIR\\include";
push @inc, lookfor('winbase.h', $ENV{INC}, pathonly=>1, prompt=>'MinGW or Visual C++ includes');
push @inc, lookfor('sqlite3.h', $ENV{INC} . ';sqlite', pathonly=>1);
push @inc, lookfor('openssl\evp.h', '\openssl\include;..\openssl\include', pathonly=>1, prompt=>'OpenSSL Directory');
push @inc, '..\\libsmx';

if (!lookfor('atlbase.h', $ENV{INC})) {
	print "Setting -DNOACTIVEX";
	push @def, 'NOACTIVEX';
}

my $cl_cmd = $cl;
for (@inc) {
	next unless $_;
	$cl_cmd .= " -I\"$_\"";
};
for (@def) {
	next unless $_;
	$cl_cmd .= " -D$_";
};


my $ld_cmd = $cl;
my $ld_libs = " -lwsock32 -lws2_32 -lodbc32 -lshlwapi  -Lc:\\dev\\openssl -llibeay32 -L\"$APACHEDIR\\bin\" -lapr-1 -lhttpd \"$APACHEDIR\\lib\\libapr-1.lib\"  \"$APACHEDIR\\lib\\libhttpd.lib\"";

make('.');

sub amname {
	my $n = shift;
	$n =~ s/[^a-zA-Z0-9_]/_/g;
	return $n;
}

sub make_gettargets {
	my ($make, $name, $type) = @_;
	for my $target (split /\s+/, $make->{$name}) {
		my $src = delete $make->{amname($target) . "_SOURCES"};
		my $ldadd = delete $make->{amname($target) . "_LDADD"};

		   $ldadd .= delete $make->{amname($target) . "_LIBADD"};

		$ldadd =~ s/\.la/\.dll/g;

		my $cflags = delete $make->{amname($target) . "_CXXFLAGS"};
		my $ldflags = delete $make->{amname($target) . "_LDFLAGS"};
		$ldflags =~ s/-version-info \S+//;
		
		if (!$src) {
			die "Can't find SOURCES for $target in $make->{AMFILE}";
		}

		if ($type eq 'dll') {
			$target =~ s/\.la$/.dll/;
			push @{$make->{libs}}, $target;
		} elsif ($type eq 'exe') {
			$target =~ s/$/.exe/;
			push @{$make->{exes}}, $target;
		} else {
			die "Unknown target type $type\n"
		}
		
		$make->{$target}->{sources} = $src;
		$make->{$target}->{ldadd} = $ldadd;		
		$make->{$target}->{cflags} = $cflags;		
		$make->{$target}->{ldflags} = $ldflags;		
	}
}

sub make {
	my $dir = shift;
	pushdir($dir);

	my $make = readam();

	for my $sub (split(/\s+/, $make->{SUBDIRS})) {
		make($sub);
	}

	for (keys(%$make)) {
		if (/LTLIBRARIES/) {
			make_gettargets($make, $_, 'dll');
		}
		if (/PROGRAMS/) {
			make_gettargets($make, $_, 'exe');
		}
	}

	for my $target (@{$make->{libs}}) {
		make_target($make, $target);		
	}

	for my $target (@{$make->{exes}}) {
		make_target($make, $target);		
	}

	popdir();
}

sub make_target {
	my ($make, $target) = @_;
	
	my $mtime;
	die "No sources for target $target\n" unless $make->{$target}->{sources};
	for (split(/\s+/, $make->{$target}->{sources})) {
		my $obj;
		$obj = make_cpp($make, $_, $make->{$target}->{cflags}) if $_ =~ /\.cp?p?$/;
		$make->{$target}->{objs} .= "$obj " if $obj;
		$mtime = (stat($obj))[9] if (stat($obj))[9] > $mtime;
	}
	if ($mtime > (stat($target))[9]) {
		$make->{$target}->{objs} =~ s/^ //;
		if ($make->{$target}->{objs}) {
			my $shared = ' -shared' if $target =~ /.dll$/;
			my $cmd = "$ld_cmd " . $make->{$target}->{objs} . "$shared -o $target";
			$make->{$target}->{ldflags} =~ s/`([^`]+)`/`$1`/ge;
			$cmd .= " $ld_libs $make->{$target}->{ldadd} $make->{$target}->{ldflags}";
			print "$cmd\n";
			system($cmd) && die;
		}
	}
}

sub make_cpp {
	my ($make, $fil, $cflags) = @_;
	my $fbase = $fil; $fbase =~ s/\.[^.]+$//;
	my $obj = "$fbase.o";
	my $ft = (stat($fil))[9];
	my $tt = (stat($obj))[9];
	return $obj if $tt >= $ft;
	$cflags =~ s/`([^`]+)`/`$1`/ge;
	my $fcmd = "$cl_cmd $cflags \"$fil\" -c -o \"$obj\"";
	print "$fcmd\n";
	system($fcmd) && die;
	return $obj;
}

my @PD;
sub pushdir {
	my $dir = shift;
	push @PD, cwd();
	chdir $dir;
	print "Current directory : " . cwd() . "\n";
}

sub popdir {
	chdir pop @PD;
	print "Current directory : " . cwd() . "\n";
}

sub is_main {
	my ($fil) = @_;
	return 0;
}

sub lookfor {
	my ($file, $path, %flags) = @_;
	
	$path = $ENV{PATH} if !$path;
	
	for (split(/;/, ".;" . $path)) {
		next unless $_;
		if ($_ =~ /\*/) {
			for (wcpath($_)) {
				if (-e "$_\\$file") {
					$ENV{$flags{env}} = "$_\\$file" if $flags{env};
					return abs_path($flags{pathonly} ? $_ : "$_\\$file") 
				}
			}
		} else {
			if (-e "$_\\$file") {
				$ENV{$flags{env}} = "$_\\$file" if $flags{env};
				return abs_path($flags{pathonly} ? $_ : "$_\\$file") 
			}
		}
	}
	if ($flags{prompt}) {
		do {
			print "$flags{prompt} : ";
			$_ = <>;
		} while ($_ && ! -e "$_\\$file");
		if (-e "$_\\$file") {
			$ENV{$flags{env}} = "$_\\$file" if $flags{env};
			return abs_path("$_\\$file");
		}
	}
}

sub wcpath {
	my ($path) = @_;
	
	if ($path =~ /^(.*?)((?:[^\\]*\*)+)(.*)/) {
		my ($base, $pat, $rest) = ($1, $2, $3);
		my @res;
		my @dir = grabdir($base);
		for my $dir (@dir) {
			next if $dir =~ /^\./;
			next unless wcmatch($dir, $pat);
			$dir = $base . $dir;
			if ($rest) {
				$dir = wcpath($dir . "\\" . $rest);
				next unless $dir;
			}
			push @res, $dir;
		}
		closedir(D);
		return @res;
	} else {
		return ($path) if -e $path;
	}
}

sub grabdir {
	opendir(D, shift); 
	my @dir = readdir(D); 
	closedir(D); 
	return @dir;
}

sub grab {
	local $/ = undef;
	open(F, shift);
	my $dat = <F>;
	close(F);
	return $dat;
}

sub wcmatch {
	my ($str, $pat) = @_;
	$pat =~ s/\*/.*/g;
	$str =~	$pat;
}

sub readam {
	my %am;
	open(IN, 'Makefile.am');
	while(<IN>) {
		chomp;
		next if /^\s*#/;
		next if /^\s*$/;
		next if ! /=/;
		
		die "Can't understnd this Makefile.am, don't use multiline assignments" if !/=/;
		my ($name, $val) = split(/\s*=\s*/,$_);
		$am{$name} = $val;
	}
	return \%am;
}

sub abs_path {
	my $f = Cwd::abs_path(shift);
	$f =~ s/\//\\/g;
	return $f;
}
