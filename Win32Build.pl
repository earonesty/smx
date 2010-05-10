### reads in Makefile.am's and attempts to build on Win32
### LOTS of crappy hardcoded stuff, will put everything in a Win32.config or something

use strict;
use Getopt::Long;
use Cwd qw(cwd);
use Carp qw(confess cluck);
use Data::Dumper;

our $opt_remake;
our $opt_debug;
our $opt_verbose;

GetOptions("remake", "debug", "verbose") || die "USAGE: perl Win32Build.pl [-remake] [-debug] [release|test|clean]";

our $opt_command = shift @ARGV;

### this file contains all the "paths" and any "flags" used
my $CFG_FILE = "Win32Build.conf";                  	# location of config file
my %CFG;

loadcfg();			# load config file
checkcfg();			# resolves all paths, prompts for them, saves the config if there are changes

my %PATH = %{$CFG{paths}};      # path hash

### list of *required* files
my %FILE = (
	sqlite_dll=>"sqlite3.dll",
	libapr_lib=>"$PATH{apache}\\lib\\libapr-1.lib",
	libhttpd_lib=>"$PATH{apache}\\lib\\libhttpd.lib",
	apache_inc=>"$PATH{apache}\\include",
	g_pp=>"$PATH{mingw}\\bin\\g++.exe",
);

checkfiles();			# this resolves all files down to a single file with a full path

$PATH{perl} = abs_path($^X);    # add "perl's bin path" to the path hash
$PATH{perl} =~ s/\\[^\\]*$//;

				# linker name for perl's dll
my ($perl_dll_base) = grep(/perl\d*\.dll$/, wcpath($PATH{perl} . '\perl*.dll'));
   $perl_dll_base =~ s/.*\\//;
   $perl_dll_base =~ s/\.dll$//;

### extract package version, add to defs
my @def;
our ($PACKAGE_NAME, $PACKAGE_VERSION, $PACKAGE_EMAIL, $PACKAGE_RELEASE) = getpackageversion();

die "Can't find package version" if ! $PACKAGE_VERSION;

### add other defs for g++
push @def, "PACKAGE_VERSION=\\\"$PACKAGE_VERSION.$PACKAGE_RELEASE\\\"";
push @def, 'HAVE_SQLITE3_H';
push @def, 'HAVE_OPENSSL';
push @def, 'HAVE_FCVT';
push @def, 'HAVE_GCVT';
push @def, 'HAVE_GD' if $PATH{gd};
push @def, 'DEBUG' if $opt_debug;

my @inc;

### add paths to g++ @inc list
push @inc, "$PATH{apache}\\include";
push @inc, "$PATH{mingw}\\include";
push @inc, "sqlite";
push @inc, "$PATH{openssl}\\include";
push @inc, 'libsmx';

checkinc();			# die if any of the inc's don't exist

# define NOACTIVEX, if it's not availalbe
if (!lookfor('atlbase.h', join(';', @inc))) {
	print "Setting -DNOACTIVEX\n";
	push @def, 'NOACTIVEX';
}

# 
my $cl_cmd = $FILE{g_pp}; 

$cl_cmd .= " -g" if $opt_debug;
$cl_cmd .= " -O2" if !$opt_debug;

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

##### ok, run the 'fake make'  #####

my $make = make('.');

##### making a 'release' package  #####

if ($opt_command eq 'test') {
}

if ($opt_command eq 'release') {
	my $dest = $opt_debug ? "debug" : "release";
	mkdir $dest;
	
	my $rel = $PACKAGE_RELEASE > 0 ? "-$PACKAGE_RELEASE" : "";
	my $suffix = $opt_debug ? '-debug' : '';
	
	for (release_targets($make)) {
		my $fn = $_; $fn =~ s/.*\\//;	
		$_ =~ s/\\([^\\]+)$/\\.debug\\$1/ if $opt_debug;
		if ((stat($_))[9] > (stat("$dest\\$fn"))[9]) {
			system("copy \"$_\" $dest");
		}
	}

	my $cmd = '';
	if ($cmd = lookfor('7z.exe', "$ENV{PATH};$ENV{ProgramFiles}\\7-zip")) {
		pushdir($dest);
		$cmd = "\"$cmd\" A -tzip ..\\$PACKAGE_NAME-$PACKAGE_VERSION${rel}-win32$suffix.zip *";
		print "$cmd\n";
		system($cmd);
		popdir();
	} elsif ($cmd = lookfor('tar.exe')) {
		pushdir($dest);
		$cmd = "\"$cmd\" -cvf ..\\$PACKAGE_NAME-$PACKAGE_VERSION${rel}-win32$suffix.tar *";
		print "$cmd\n";
		system($cmd);
		popdir();
		
		if ($cmd = lookfor('gzip.exe')) {
			$cmd = "\"$cmd\" $PACKAGE_NAME-$PACKAGE_VERSION${rel}-win32$suffix.tar";
			print "$cmd\n";
			system($cmd);
		}
	}
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

	if ($make->{libs} || $make->{exes}) {
		mkdir('.debug') if $opt_debug;
	}

	if ($opt_command eq 'clean') {
		for my $target (@{$make->{libs}}) {
			clean_target($make, $target);		
		}

		for my $target (@{$make->{exes}}) {
			clean_target($make, $target);		
		}
	} else {
		for my $target (@{$make->{libs}}) {
			make_target($make, $target);		
		}

		for my $target (@{$make->{exes}}) {
			make_target($make, $target);		
		}
	}
	
	popdir();

	return $make;
}

sub clean_target {
	my ($make, $target) = @_;
	
	print "del ";
	for (split(/\s+/, $make->{$target}->{sources})) {
		if ($_ =~ /\.cp?p?$/) {
			my $fbase = $_; $fbase =~ s/\.[^.]+$//;
			my $obj = "$fbase.o";
			my $dep = "$fbase.d";
			$obj =~ s/^/.debug\\/ if $opt_debug;
			print "$obj ";
			print "$dep ";
			unlink $obj;
		}
	}
	my $out = $target;
	$out =~ s/^/.debug\\/ if $opt_debug;
	print "$out\n";
	unlink $out;
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
			$mtime = (stat($obj))[9] if (stat($obj))[9] > $mtime;
		}
	}

	for (split (/\s+/, $make->{$target}->{ldadd})) {
		$mtime = (stat($_))[9] if (stat($_))[9] > $mtime;
	}

	$mtime = (time()+1) if $opt_remake;	
	
	my $out = $target;
	$out =~ s/^/.debug\\/ if $opt_debug;
	
	if ($mtime > (stat($out))[9]) {
		$make->{$target}->{objs} =~ s/^ //;
		if ($make->{$target}->{objs}) {
			my $shared = ' -shared' if $target =~ /.dll$/;
			
			my $cmd = "$ld_cmd " . $make->{$target}->{objs} . "$shared -o $out";
			
			$make->{$target}->{ldflags} =~ s/-lperl\b//;		# on unix this is ok, on win it isn't
			
			$make->{$target}->{ldflags} =~ s/`([^`]+)`/`$1`/ge;
			if ($make->{$target}->{ldflags} =~ /perl/i) {
				# perl's extutils rarely outputs what we need for mingw, so clean things up
				$make->{$target}->{ldflags} =~ s/-libpath:/-L:/;
				$make->{$target}->{ldflags} =~ s/-nologo//g;
				$make->{$target}->{ldflags} =~ s/-nodefaultlib//;
				$make->{$target}->{ldflags} =~ s/-debug//;
				$make->{$target}->{ldflags} =~ s/-Gf//;
				$make->{$target}->{ldflags} =~ s/-Zi//;
				$make->{$target}->{ldflags} =~ s/-opt:\S+//;
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

	$obj =~ s/^/.debug\\/ if $opt_debug;
	
	my $tt = (stat($obj))[9];
	return $obj if $tt >= $dt && !$opt_remake;
	
	print "cflags before exec: $cflags\n" if $opt_verbose && $cflags =~ /\`/;
	$cflags =~ s/-nologo//ge;
	$cflags =~ s/-Gf//;
	$cflags =~ s/-Zi//;

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
	my $ac = grab('configure.ac');
	for ($ac =~ m|AC_CHECK_PROG\(([^\(\)]+)\)|g) {
		my ($var, $prog, $ok, $nok) = split(/\s*,\s*/, $_);
		$am{$var} = lookfor($prog . ".exe") ? $ok : $nok;
	}
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
	my $f = shift;
	if ( -s $f && $f =~ /^[a-z]:/i) {
		# no need, already abs
	} else {
		$f = Cwd::abs_path($f);
	}
	$f =~ s/\//\\/g;
	return $f;
}


sub loadcfg
{
        require Safe;                           # somewhat safe evaluation container
        my $cont = new Safe;
        my $cfg = eval {
                package CFG;
		# copy key environment vars to the container
		for (qw(OPENSSL_CONF LOCALAPPDATA ProgramFiles SystemRoot TEMP TMP USERPROFILE)) {
			$cont->reval("\$ENV{$_}='$ENV{$_}'");
		}
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

sub quoteshell {
	my $s = shift;
	$s =~ s/\"/\"\"/g;
	return $s;
}

sub getpackageversion {
	### munge the 'nix "configure.ac"
	
	my $unixconf = grab('configure.ac');
	my ($name, $ver, $em) = $unixconf=~ m|AC_INIT\(\s*([^,\s]+)\s*,\s*([^,\s]+)\s*,\s*([^,\s\(\)]+)\s*\)|;
	our %PROG = $unixconf=~ m|AC_PATH_PROG\(\s*([^,\s]+)\s*,\s*([^,\s\(\)]+)\s*\)|g;
	for (keys(%PROG)) {
		$PROG{$_} = lookfor($PROG{$_} . '.exe');
	}

	my $spec= grab($name . '.spec.in');
	my ($rel) = $spec =~ m|%define rel (\S+)|i;
	
	if ($rel =~ m|\@(\S+)\@|) {
		($rel) = $unixconf =~ m|$1\s*=\s*([^\n]+)|;
		if ($rel =~ /svnversion/i) {
			$rel = `svnversion`;
			$rel =~ s/\s+$//s;
			$rel =~ s/^(?:.*:)?(.*)[MSP]$/$1/;
			warn "Can't determine release" if !$rel;
			$rel = 0+grab("release.txt") if !$rel;
		} else {
			$rel =~ s/\$(\w+)/\"$PROG{$1}\"/g;
			$rel =~ s/\bcat\b/type/g;
			if ($rel =~ s/\bsed\b/$^X -pe/g) {
				$rel =~ s/\[\[/\[/g;
				$rel =~ s/\]\]/\]/g;
			}
			$rel =~ s/'([^']+)'/'"' . quoteshell($1) . '"'/ge;
			$rel =~ s|/dev/null|nul|g;
			$rel =~ s/;/&&/;
			$rel =~ s/`([^`]+)`/`$1`/ge;
			die "Can't determine release: $!" if $!;
		}
	}
	return ($name, $ver, $em, $rel);
}


sub max {
	return $_[0] > $_[1] ? $_[0] : $_[1];
}
