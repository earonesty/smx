lib_LTLIBRARIES = libmodsmx.la

libmodsmx_la_SOURCES = modsmx.cpp 
libmodsmx_la_LIBADD  = ../libsmx/libsmx.la
libmodsmx_la_LDFLAGS = -version-info 0:0:0 --shared
INCLUDES = -I../libsmx

install-data:
	@APXS@ -i -n 'mod_smx' -a libmodsmx.la
