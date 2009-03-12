#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.57)
AC_INIT(smx, 1.1.4, bugs@smxlang.org)
AC_CONFIG_SRCDIR([config.h.in])
AM_CONFIG_HEADER([config.h])
AM_INIT_AUTOMAKE

AM_MAINTAINER_MODE

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_RANLIB
AC_PROG_LIBTOOL

# Platform-specific tweaks
case $host in
*solaris*)
    LDFLAGS="$LDFLAGS -L/usr/local/ssl/lib";;
*freebsd*)
    CPPFLAGS="$CPPFLAGS -I/usr/local/include"
    LDFLAGS="$LDFLAGS -L/usr/local/lib";;
*openbsd*)
    LIBS="-ldes $LIBS";;
esac

# add -l linker options, if necessary
AC_SEARCH_LIBS([gethostbyname], [nsl socket])

# search for openssl's funcs exhaustively, since OPENSSL_NO_XXX is unreliable  

# Checks for libraries.
AC_CHECK_LIB([crypt], [crypt])
AC_CHECK_LIB([crypto], [EVP_EncodeBlock])
AC_CHECK_LIB([crypto], [EVP_idea_cbc], [AC_DEFINE([HAVE_IDEA], [1], [Enable idea cipher])])
AC_CHECK_LIB([crypto], [EVP_rc5_32_12_16_cbc], [AC_DEFINE([HAVE_RC5], [1], [Enable rc5 cipher])])

AC_CHECK_LIB([odbc], [SQLConnect],[],[
        echo "Error! libodbc is required, maybe you need to set LDFLAGS="-L/usr/local/lib""
        exit -1
])
AC_CHECK_LIB([sqlite3], [sqlite3_open])
AC_CHECK_LIB([pthread], [pthread_create])
AC_CHECK_LIB([gd], [gdImageCreate],[],[
	echo "%image macro disabled, gd libs not available"
])
AC_CHECK_LIB([stdc++], [main],[],[
        echo "Error! libstdc++ is required, probably need to install it"
        exit -1
])
AC_CHECK_LIB([tdb], [tdb_fetch])

# sets apache version & include paths
APACHE_APXS
APACHE_APR
AC_SUBST(APXS)

# Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_LANG_PUSH(C++)

AC_CHECK_HEADERS([sys/io.h sqlite3.h sql.h pthread.h openssl/evp.h tdb.h signal.h gd.h bits/atomicity.h ext/atomicity.h])
AC_FUNC_STRERROR_R

AC_LANG_POP

#check to see if gmtoff is a member of struct tm
AC_CHECK_MEMBER(struct tm.tm_gmtoff,[AC_DEFINE([HAVE_STRUCT_TM_GMTOFF],[1],[gmtoff in struct tm])],,[#include <time.h>])

AC_CHECK_PROG(EXTPERL, perl, extperl)

AC_CHECK_HEADER(fcgi_stdio.h, [AC_SUBST([FCGI],[fcgi])])

AC_PATH_PROG([SVNVERSION], [svnversion])
RELEASE=`($SVNVERSION > /dev/null 2>&1) && ($SVNVERSION | grep [[0-9]] > /dev/null) && ($SVNVERSION | sed 's/.*://' | sed 's/[[MSP]]$//' > release.txt); cat release.txt`
AC_SUBST(RELEASE)


# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_STRUCT_TM
AC_TYPE_INT32_T
AC_TYPE_MODE_T
AC_TYPE_OFF_T
AC_TYPE_UINT8_T


# Checks for library functions.
AC_CHECK_FUNCS([gcvt dtoa fcvt sigset])

AC_CONFIG_FILES([Makefile fcgi/Makefile smx/Makefile libsmx/Makefile extperl/Makefile modsmx/Makefile smx.spec])

AC_OUTPUT

