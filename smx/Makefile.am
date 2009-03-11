bin_PROGRAMS = smx

smx_SOURCES = qscgi.cpp qscgi.h 
smx_LDADD   = ../libsmx/libsmx.la

INCLUDES = -I../libsmx

install-data-local:
	thisdir=`pwd` && cd ../docs && $$thisdir/smx doc2man $(DESTDIR)$(datadir)/man && cd ../smx

TESTS=tests/smxtest
