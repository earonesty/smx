# Introduction #

Currently, smx comes with a perl program that parses the unix Makefile.am's, and compiles based somwwhat on them.

# Details #

  * install the development versions of [MinGW ](http://sourceforge.net/project/showfiles.php?group_id=2435&package_id=240780), [OpenSSL](http://www.openssl.org/related/binaries.html), [Apache](http://httpd.apache.org/download.cgi) & [SQLite](http://www.sqlite.org/download.html)

  * Open up Win32Build.conf in a text editor

  * If you need to, change the MinGW, OpenSSL & Apache paths to match wherever you installed them, most likely they don't need to be changed.

  * To build run

> perl Win32Build.pl

  * to make a release tarball, run

> perl Win32Build.pl release

  * to run tests

> perl Win32Build.pl check

  * to set the "debug" flag on (while building any of the above)

> perl Win32Build.pl -debug