#!/usr/bin/perl

copyinit();
copydir('.');

sub copydir
{
  my ($d) = @_;

  my $ig;
  open(ig, "$d/.cvsignore");
  while(<ig>) {
	chomp;
	$_ =~ s/\./\\\./g;
	$_ =~ s/\*/\.\*/g;
	$ig .= "|($_)";
  }
  close ig;
  $ig = substr($ig, 1);

  opendir(d, $d);
  my @d = readdir(d);
  closedir(d);
  my $f;
  for $f (@d) {
	next if $f =~ /^$ig$/;
	next if $f =~ /^[.]/;
	copydir("$d/$f") if -d "$d/$f";
	next unless $f =~ /\.(h|cpp|c)$/;
	copyfil("$d/$f");
  }
  
}

sub copyfil
{
	my ($fp) = @_;
	open (in, $fp);
	local $/ = undef;
	my $text = <in>;
	close in;
# skip these 
	return if $text =~ $copysig;
	return if $text =~ 'Joe O\'Leary';
	return if $text =~ 'Jef Poskanzer';
# end skip
	$text =~ s|/\*[^*]+copyright[^*]+\*/||si;
	$text =~ s|// Copyright.*Prime Data.*\@primedata\.org\s*||si;

	(print($text) && die) if $text =~ /copyright/i;

	$text = $copytext . "\n" . $text;

	print "modifying $fp\n";
	open (out, ">$fp.tmp");
	print out $text;
	close out;
	rename "$fp", "$fp.bak";
	rename "$fp.tmp", $fp;
}

sub copyinit
{
our $copytext = <<'EOF';
/*
Copyright 1998-2005 Erik Aronesty. All rights reserved.

        1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

        2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

	3. Original source code, information and documentation is available at www.smxlang.org

THIS SOFTWARE IS PROVIDED 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE SMX AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/
EOF

our $copysig = 'www.smxlang.org';
}
