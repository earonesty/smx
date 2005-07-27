####### kdevelop will overwrite this part!!! (begin)##########
bin_PROGRAMS = smx

smx_SOURCES = qscgi.cpp 
smx_LDADD   = ../libsmx/libsmx.la

#EXTRA_DIST = qscgi.h qscgi.cpp 
INCLUDES = -I../libsmx

install-data-local:
	pushd ../docs && smx doc2man && popd

TESTS=tests/smxtest
