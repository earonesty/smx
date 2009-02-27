#
# Copyright (c) 2006, Erik Aronesty
# All rights reserved.
#
# See documentation and licensing below.

package Smx;

$Smx::VERSION='1.02';

use strict;
use Regexp::Common;
use Exporter;

our @ISA=qw(Exporter);
our @EXPORT=qw(smx smx_quote);
our @EXPORT_OK=qw(smx_funcs);

my $debug=0;

my %smx_funcs = (
        'lf' => sub { "\n" },
        'cr' => sub { "\r" },
        'iadd' => sub { return $_[1]+$_[2]; },
        'isub' => sub { return $_[1]-$_[2]; },
        'equal' => sub { return $_[1] =~ /^$_[2]$/; },
        'equ' => sub { return $_[1] == $_[2]; },
        'gt' => sub { return $_[1] > $_[2]; },
        'lt' => sub { return $_[1] < $_[2]; },
        'lte' => sub { return $_[1] <= $_[2]; },
        'gte' => sub { return $_[1] >= $_[2]; },
        'xequal' => sub { return $_[1] eq $_[2]; },
        'expand' => sub { return smx($_[1], $_[0]); },
        'set' => sub { ${$_[0]}{$_[1]}=$_[2]; return '';},
        'gset' => sub { 
        	my $ctx = $_[0];
		my $pctx;
        	while ($pctx=$pctx->{"<pctx>"}) {
			$ctx->{$_[1]}=undef;
              		$ctx = $pctx;
        	}
		$ctx->{$_[1]}=$_[2];
		return '';
	},
        'let' => sub {
                my $ctx = $_[0];
                my $sctx=$ctx;
                while ($sctx) {
                        last if defined($sctx->{$_[1]});
                	$sctx=$sctx->{"<pctx>"};
                }
		$ctx = $sctx if $sctx;
                $ctx->{$_[1]}=$_[2];
		return '';
        },
        'null' => sub { return '';},
        'for' => [ sub { 
		my %ctx;
		$ctx{"<pctx>"}=$_[0];
		my $i;
		$ctx{$_[1]}=\$i;
		my $out;
		for ($i = $_[2]; $i <= $_[3]; ++$i) {
			$out .= smx($_[4], \%ctx);
		}
		return $out;
	} , 0 ],
        'define' => [ sub {
		my $code = $_[2];
		my @aname;
		my $i; for ($i=3;$i<@_;++$i) {
			push @aname, $_[$i];
		}
		${$_[0]}{$_[1]}=sub{
			my %ctx;
			$ctx{"<pctx>"}=shift @_;
			my @args = @_;
			$ctx{"arg"}=sub{return $args[$_[1]]}; 
			my $i=0;
			for (@aname) {
				$ctx{$_}=\$args[$i++];
			}
			return smx($code,\%ctx);
		};
		return '';
	 } , 0],
        'left' => sub { return substr($_[1],0,$_[2]); },
        'invoke' => \&invoke,
        'mid' => sub { return length($_[3]) ? substr($_[1],$_[2],$_[3]) : substr($_[1],$_[2]); },
        'right' => sub { return substr($_[1],-$_[2]); },
        'include' => sub { my $ret; if (open(IN, $_[1])) { local $/=undef; $ret=<IN>; close IN; } return $ret; },
        'if' => [ sub { if (length(smx($_[1],$_[0]))) { return smx($_[2],$_[0]) } else { return smx($_[3],$_[0]) } }, 0 ],
);

sub smx
{
	my ($str, $ctx) = @_;
	my %ctx;
	$ctx=\%ctx unless $ctx;
	$str =~ s/(%+[\w-]+)(?:($RE{balanced}{-parens=>'(  )'})|%)/&smx_func($ctx, $1, $2)/ges;
	return $str;
}

sub invoke
{
	my ($ctx, $fname, @args) = @_;
        my $func=$ctx->{$fname};
        my $pctx = $ctx;
        while (!defined($func) && ($pctx=$pctx->{"<pctx>"})) {
                $func=$pctx->{$fname};
        }
	$func=$smx_funcs{$fname} if (!defined($func));
	$func = '' if !defined($func);
        return $func if (!ref($func));
        return ${$func} if (ref($func) eq 'SCALAR');
        my $res = &{$func}($ctx, @args);
        return $res;
}

sub smx_func
{
	my ($ctx, $fname, $args) = @_;

	$fname=substr($fname,1);
	print "<$fname\n" if $debug;

	my $func=$ctx->{$fname};
	my $pctx = $ctx;
	while (!defined($func) && ($pctx=$pctx->{"<pctx>"})) {
		$func=$pctx->{$fname};
	}
	$func=$smx_funcs{$fname} if (!defined($func));

	if (!defined($func)) {
		$fname =~ s/^%//;
		$fname =~ s/%%/%/g;
		$args = '%' if !$args;
		return "%$fname$args";
	}

	return $func if (!ref($func));
	return ${$func} if (ref($func) eq 'SCALAR');

	my $parse_args = 1;
	if (ref($func) eq 'ARRAY') {
		$parse_args = ${$func}[1];
		$func = ${$func}[0];
	}


	my $arg;
	my $quoted = 0;
	$args=substr($args,1,length($args)-2);
	my @args;
	do {
		$args =~ s/^\s+//;	
		if (substr($args,0,1) eq '\'') {
			$quoted = 1;
			$args=substr($args,1);
			$args =~ s/^\s+//;	
		}
        	if (substr($args,0,1) eq '"') {
			if ($args =~ s/^($RE{delimited}{-delim=>'"',-esc=>'\\'})([^,]*)//s) {
				my $leftover = $2;
				$arg = substr($1,1,length($1)-2);
				$arg =~ s/\\(.)/$1/g;
				if ($leftover !~ /^\s*$/) {
					print STDERR "junk after quoted argument ignored\n";
				}
			} else {
				print STDERR "syntax error near $args, expected quote-delimited\n";
				$args = '';
			}
		} elsif ($args =~ s/^((?:[^,]*?$RE{balanced}{-parens=>'(  )'})*[^,]*)//s) {
			$arg = $1;
			$arg =~ s/\\(.)/$1/g;
        	} else {
			print STDERR "syntax error near $args, expected arg";
			$args = '';
		}
		$arg = smx($arg, $ctx) if $parse_args && !$quoted;
		push @args, $arg;
		print "$arg,\n" if $debug;
		if ($args) {
		  my $asep = substr($args,0,1);
		  if ($asep && !($asep eq ',')) {
			print STDERR "syntax error near $args, expected comma\n";
			$args = '';
		  } else {
			$args=substr($args,1);
		  }
		}
		$quoted = 0;
	} while ($args);
	my $res = &{$func}($ctx, @args);
	print "=$res>\n" if $debug;
	return $res;
	
}

sub smx_quote
{
	my ($str) = @_;
	if ($str) {
		$str =~ s/\"/\\\"/g;
		$str =~ s/%/%%/g;
		return '"' . $str . '"';
	} else {
		return '""';
	}
}

1;

__END__;

=head1 NAME

Smx - Perl implementation of the SMX scripting language

=head1 SYNOPSIS

use Smx;

print smx("%iadd(1,1)");

my %ctx;
$ctx{var}='bob';
print smx("the var is %bob%", $ctx);

# load code
smx("%expand(%include(code.htx))", $ctx);

# run %specific-function within that code
smx("%specific-function%", $ctx);

=head1 DESCRIPTION

The Smx module allows you to parse SMX code, or configuration files stored in 
SMX format, such as those produces by the SMX Bulleten Board or Weblog systems, 
allowing ease of integration with perl

=head1 FUNCTIONS

=over 1

=item smx($string[,$context_hash]);

Parses the string, expanding any macros found.  Any global variables set during the 
parse will be placed in the $context_hash.   Any variables in the $context_hash
will be available to the parser.

=head1 SEE ALSO

http://www.smxlang.org
http://en.wikipedia.org/wiki/SMX

=head1 COPYRIGHT

This library is free software; you can redistribute it and/or
modify it under the same terms as Perl itself.

 Copyright 2006 Erik Aronesty

=cut
