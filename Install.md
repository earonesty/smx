# Install Linux #

rpm -i smx-(ver)-(rel).rpm

RPM will warn you if the requirements aren't there, etc.

If you need to, you can rpmbuild --rebuild from the smx-(ver)-(rel).src.rpm

# Install Windows #

On Windows, there's currently no installer, so you get a zipped up file
with the following:

  * smx.exe - command-line and/or cgi client
  * libsmx.dll - main dll
  * libmodsmx.dll - dll file for apache 2, use LoadModule to load it
  * libextperl.dll - dll file for perl extension, use %module() to load it

Drop them all in the "windows" directory, so they show up in the path, and they can be loaded as libraries.

However, these all depend on several other files being installed first:

  * SQLite : Just install the sqlitedll version for Windows
> > http://www.sqlite.org/download.html

  * Apache : Latest binary with or without ssl, as needed:
> > http://httpd.apache.org/download.cgi

  * OpenSSL : Latest binary release, unless you already installed apache, in which case you can just copy the libeay32.dll to your windows directory:
> > http://www.openssl.org/related/binaries.html

# See Also #

[How To Configure](Configure.md)