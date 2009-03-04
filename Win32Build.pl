### reads in Makefile.am's and attempts to build on Win32
### LOTS of crappy hardcoded stuff, will put everything in a Win32.config or something

use strict;
use Getopt::Long;
use Cwd qw(cwd);
use Carp qw(confess cluck);
use Data::Dumper;

our $opt_remake;
our $opt_debug;

GetOptions("remake", "debug") || die "USAGE: perl Win32Build.pl [-remake] [-debug] [dist|test]";

our $opt_command = shift @ARGV;

### this file contains all the "paths" and any "flags" used
my $CFG_FILE = "Win32Build.conf";                  	# location of config file
my %CFG;

loadcfg();
checkcfg();

my %PATH = %{$CFG{paths}};                                               # path hash

### list of *required* files
my %FILE = (
	sqlite_dll=>"sqlite3.dll",
	libapr_lib=>"$PATH{apache}\\lib\\libapr-1.lib",
	libhttpd_lib=>"$PATH{apache}\\lib\\libhttpd.lib",
	apache_inc=>"$PATH{apache}\\include",
	g_pp=>"$PATH{mingw}\\bin\\g++.exe",
);

checkfiles();

$PATH{perl} = abs_path($^X);  
$PATH{perl} =~ s/\\[^\\]*$//;

my ($perl_dll_base) = grep(/perl\d*\.dll$/, wcpath($PATH{perl} . '\perl*.dll'));
   $perl_dll_base =~ s/.*\\//;
   $perl_dll_base =~ s/\.dll$//;

### extract package version, add to defs
my @def;

my $PACKAGE_VERSION = getpackageversion() || die "Can't find package version";

push @def, "PACKAGE_VERSION='$PACKAGE_VERSION'";
push @def, 'HAVE_SQLITE3_H';
push @def, 'HAVE_OPENSSL';
push @def, 'OPENSSL_NO_IDEA';
push @def, 'HAVE_GD' if $PATH{gd};

my @inc;

### look for various directories, prompt if necessary

push @inc, "$PATH{apache}\\include";
push @inc, "$PATH{mingw}\\include";
push @inc, "sqlite";
push @inc, "$PATH{openssl}\\include";
push @inc, 'libsmx';

checkinc();

if (!lookfor('atlbase.h', $ENV{INC})) {
	print "Setting -DNOACTIVEX\n";
	push @def, 'NOACTIVEX';
}

my $cl_cmd = $FILE{g_pp}; $cl_cmd .= " -g" if $opt_debug;

for (@inc) {
	next unless $_;
	$cl_cmd .= " -I\"$_\"";
};

for (@def) {
	next unless $_;
	$cl_cmd .= " -D$_";
};

my $ld_cmd = $FILE{g_pp};
$ld_cmd .= " -g" if $opt_debug;

my $ld_libs = "$FILE{sqlite_dll} -lwsock32 -lws2_32 -lodbc32 -lshlwapi -L$PATH{openssl} -llibeay32 -L\"$PATH{apache}\\bin\" -lapr-1 -lhttpd \"$PATH{apache}\\lib\\libapr-1.lib\"  \"$PATH{apache}\\lib\\libhttpd.lib\"";

my $make = make('.');
	
if ($ARGV[0] eq 'release') {
	my $cmd = "tar -cvf smx-$PACKAGE_VERSION-win32-x86.tar " . join ' ', release_targets($make);
	print "$cmd\n";
	system($cmd);

	my $cmd = "gzip smx-$PACKAGE_VERSION-win32-x86.tar";
	print "$cmd\n";
	system($cmd);
}

sub checkcfg {
	my $save = 0;
	for my $name (keys(%{$CFG{paths}})) {
		my $path = $CFG{paths}{$name};
		my $found;
		for (split(/;/,$path)) {
			if (my $actual = wcpath($_)) {
				$CFG{paths}{$name} = $actual;
				$found = 1;
				last;
			}
		}
		if (!$found) {
			$CFG{paths}{$name} = prompt("Path for $name: ");
			$save = 1;
		}
	}
	if ($save) {
		savecfg();
	}
}

sub release_targets {
	my ($make, $dir) = @_;
	my @targ;
	$dir = '.' if !$dir;
	for (split /\s+/, $make->{SUBDIRS}) {
		push @targ, release_targets($make->{$_}, $_);
	}
	for (@{$make->{exes}}) {
		next if /test_/;
		push @targ, "$dir\\$_";
	}
	for (@{$make->{libs}}) {
		next if /test_/;
		push @targ, "$dir\\$_";
	}
	return @targ;
}

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
		$ldflags =~ s/-rpath \S+//;
		
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
	return undef if ! -d $dir;
	
	pushdir($dir);

	my $make = readam();

	print "Skipping $dir\n" if !$make;

	for my $sub (split(/\s+/, $make->{SUBDIRS})) {
		$make->{$sub} = make($sub);
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

	return $make;
}

sub make_target {
	my ($make, $target) = @_;
	
	my $mtime;
	die "No sources for target $target\n" unless $make->{$target}->{sources};

	for (split(/\s+/, $make->{$target}->{sources})) {
		$mtime = (stat($_))[9] if (stat($_))[9] > $mtime;
		if ($_ =~ /\.cp?p?$/) {
			my $obj = make_cpp($make, $_, $make->{$target}->{cflags});
			$make->{$target}->{objs} .= "$obj ";
			die unless $obj;
		}
	}

	for (split (/\s+/, $make->{$target}->{ldadd})) {
		$mtime = (stat($_))[9] if (stat($_))[9] > $mtime;
	}

	$mtime = (time()+1) if $opt_remake;	
	
	if ($mtime > (stat($target))[9]) {
		$make->{$target}->{objs} =~ s/^ //;
		if ($make->{$target}->{objs}) {
			my $shared = ' -shared' if $target =~ /.dll$/;
			my $out = $target;
			$out =~ s/\./_dbg\./ if $opt_debug;
			my $cmd = "$ld_cmd " . $make->{$target}->{objs} . "$shared -o $out";
			
			$make->{$target}->{ldflags} =~ s/`([^`]+)`/`$1`/ge;
			
			if ($make->{$target}->{ldflags} =~ /perl/i) {
				$make->{$target}->{ldflags} =~ s/-lperl\b/-l$perl_dll_base/;
				$make->{$target}->{ldflags} =~ s/-libpath:/-L:/;
				$make->{$target}->{ldflags} = "-L$PATH{perl} " . $make->{$target}->{ldflags};
			}
			
			$cmd .= " $make->{$target}->{ldadd} $ld_libs $make->{$target}->{ldflags}";
			print "$cmd\n";
			system($cmd) && die;
		}
	}
}

sub make_cpp {
	my ($make, $fil, $cflags) = @_;
	my $fbase = $fil; $fbase =~ s/\.[^.]+$//;
	my $obj = "$fbase.o";
	my $dep = grab("$fbase.d");
	$dep =~ s/\\(\r?\n)/ /gs;
	my $dt;
	for (split(/\s+/,$dep)) {
		next unless -e $_;
		$dt = max($dt, ((stat($_))[9]));
	}
	$dt = max($dt, (stat($fil))[9]);
	
	my $tt = (stat($obj))[9];
	return $obj if $tt >= $dt && !$opt_remake;
	
	$cflags =~ s/`([^`]+)`/`$1`/ge;
	my $dep = $obj;  $dep =~ s/\.[^.]+$/.Plo/;
	my $fcmd = "$cl_cmd $cflags \"$fil\" -c -MMD -o \"$obj\"";
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
	
	if ($path =~ /^(.*?)((?:[^\\]*\*[^\\]*)+)(.*)/) {
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
			$dir =~ s/\\\\/\\/g;
			push @res, $dir;
		}
		closedir(D);
		return wantarray ? @res : $res[0];
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
	$str =~	/$pat/i;
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
		s/^\s+//;
		my ($name, $val) = split(/\s*=\s*/,$_);
		$val =~ s/\$\((\w+)\)/$am{$1}/;
		$am{$name} = $val;
	}
	return \%am;
}

sub abs_path {
	my $f = Cwd::abs_path(shift);
	$f =~ s/\//\\/g;
	return $f;
}


sub loadcfg
{
        require Safe;                           # somewhat safe evaluation container
        my $cont = new Safe;
        my $cfg = eval {
                package CFG;
                $cont->rdo($CFG_FILE);          # read in the config
        };
        if ($cfg) {
                %CFG = %{$cfg};                         # assign to the global hash
        }
}

sub savecfg {
        require Data::Dumper;                   # dump config hash into the config file
        scrib($CFG_FILE, Data::Dumper->Dump([\%CFG]));
}

sub scrib {
        my ($f, $d) = @_;
        open OUT, ">$f";                        # write out a string as a whole file
        print OUT $d;
        close OUT;
}

sub prompt {
        print $_[0] . ($_[1] ? " ($_[1]) " : "");       # show the prompt, and the default value if any
        my $r = <STDIN>;
        chomp($r);
        $r = $_[1] if !$r;                      # assign the default value if needed
        return $r;
}

sub checkfiles {
	for (keys(%FILE)) {
		my $path = $FILE{$_};
		if ($path !~ /\\/) {
			$path = lookfor($path);
			$FILE{$_} = $path;
		}
		die "Can't find $path" unless -e $path;
	}
}

sub checkinc {
	for (@inc) {
		$_ = abs_path($_);
		die "Can't find $_" unless -e $_;
	}
}

sub getpackageversion {
	### read in the 'nix "configure" command
	my $unixconf = grab('configure');

	die "Need 'configure', even if i'm not going to run it\n" 
		unless $unixconf;
	my @def;
	if ($unixconf =~ m/(PACKAGE_VERSION=([^\n]+))/) {
		return eval($2);
	} else {
		die "Can't get PACKAGE_VERSION from configure";
	}
}


sub max {
	return $_[0] > $_[1] ? $_[0] : $_[1];
}