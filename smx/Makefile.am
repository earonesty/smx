bin_PROGRAMS = smx

smx_SOURCES = smx.cpp
smx_LDADD   = ../libsmx/libsmx.la

AM_CPPFLAGS = -I../libsmx

install-data-local:
	thisdir=`pwd` && cd ../docs && $$thisdir/smx doc2man $(DESTDIR)$(mandir) && cd ../smx

TESTS=tests/smxtest
