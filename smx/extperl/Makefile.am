if HAVE_PERL

lib_LTLIBRARIES = libextperl.la

libextperl_la_SOURCES = extperl.cpp
libextperl_la_CXXFLAGS = -I../libsmx `perl -MExtUtils::Embed -e ccopts`
libextperl_la_LDFLAGS = `perl -MExtUtils::Embed -e ldopts` -lperl

test_perl_SOURCES = test_perl.cpp
test_perl_LDADD = ../libsmx/libsmx.la
test_perl_CXXFLAGS = -I../libsmx

check_PROGRAMS = test_perl

TESTS=test_perl

endif
