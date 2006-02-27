####### kdevelop will overwrite this part!!! (begin)##########
bin_PROGRAMS = smx

smx_SOURCES = qscgi.cpp 
smx_LDADD   = ../libsmx/libsmx.la

#EXTRA_DIST = qscgi.h qscgi.cpp 
INCLUDES = -I../libsmx

install-data-local:
	thisdir=`pwd` && pushd ../docs && $$thisdir/smx doc2man $(DESTDIR)${mandir} && popd

TESTS=tests/smxtest
