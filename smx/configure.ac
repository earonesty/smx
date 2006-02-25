#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.57)
AC_INIT(smx, 1.3.1, bugs@smxlang.org)
AC_CONFIG_SRCDIR([config.h.in])
AM_CONFIG_HEADER([config.h])
AM_INIT_AUTOMAKE

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_LIBTOOL

# what does this do
AM_MAINTAINER_MODE

# Checks for libraries.
AC_CHECK_LIB([crypt], [crypt])
AC_CHECK_LIB([crypto], [EVP_EncodeBlock])
AC_CHECK_LIB([odbc], [SQLConnect])
AC_CHECK_LIB([sqlite3], [sqlite3_open],,[])
AC_CHECK_LIB([pthread], [pthread_create])
AC_CHECK_LIB([stdc++], [main])
AC_CHECK_LIB([tdb], [tdb_fetch],,[
AX_PATH_BDB([3],[
        LIBS="$BDB_LIBS $LIBS"
        LDFLAGS="$BDB_LDFLAGS -ldb_cxx $LDFLAGS"
        CPPFLAGS="$CPPFLAGS $BDB_CPPFLAGS"
])])

# sets apache version & include paths
APACHE_APXS
APACHE_APR
AC_SUBST(APXS)
CPPFLAGS="$CPPFLAGS -I $APXS_INCLUDE -I $APR_INCLUDE"

# Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([arpa/inet.h fcntl.h limits.h malloc.h memory.h netdb.h netinet/in.h stddef.h stdlib.h string.h sys/file.h sys/ioctl.h sys/socket.h sys/time.h unistd.h wchar.h wctype.h apr_strings.h httpd.h sqlite3.h sql.h pthread.h openssl/evp.h tdb.h signal.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_STRUCT_TM

# Checks for library functions.
AC_FUNC_ALLOCA
AC_FUNC_FORK
AC_FUNC_MEMCMP
AC_FUNC_MKTIME
AC_FUNC_SELECT_ARGTYPES
AC_FUNC_STAT
AC_FUNC_STRERROR_R
AC_FUNC_STRTOD
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([bzero dup2 gethostbyname gettimeofday inet_ntoa memchr memmove memset mkdir pow putenv rmdir select setenv socket sqrt strchr strerror strpbrk strrchr strspn strstr strtol strtoul tzset])

AC_CONFIG_FILES([Makefile smx/Makefile libsmx/Makefile modsmx/Makefile])
AC_OUTPUT
