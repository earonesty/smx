dnl ---------------------------------------------------------------------------
dnl Macro: APACHE_APXS
dnl ---------------------------------------------------------------------------
AC_DEFUN([APACHE_APXS], [
  AC_MSG_CHECKING(for apxs)
  AC_ARG_WITH(apxs,
    [[  --with-apxs[=FILE]      Build shared Apache module.  FILE is the
                          optional pathname to the Apache apxs tool; 
                          defaults to "apxs".]],
    [
    if test "$withval" = "yes"; then
      for i in /usr/local/apache/bin /usr/local/apache2/bin /usr/sbin /usr/bin ; do
        if test -f "$i/apxs2"; then
          APXS="$i/apxs2"
        fi
      done
      for i in /usr/local/apache/bin /usr/local/apache2/bin /usr/sbin /usr/bin ; do
        if test -f "$i/apxs"; then
          APXS="$i/apxs"
        fi
      done
      if test "$APXS" = ""; then
        AC_MSG_ERROR(["could not find apxs"])
      fi
    else
      APXS="$withval"
      if test ! -f "$APXS"; then
        AC_MSG_ERROR(["could not find apxs"])
      fi
    fi

    HTTPD_EXE="`$APXS -q TARGET`"
    HTTPD_DIR="`$APXS -q SBINDIR`"
    HTTPD="$HTTPD_DIR/$HTTPD_EXE"
    APXS_CC="`$APXS -q CC`"
    APXS_INCLUDE="`$APXS -q INCLUDEDIR`"
    APXS_CONFDIR="`$APXS -q SYSCONFDIR`"
    APACHE_LIBEXECDIR="`$APXS -q libexecdir`"

    if test "$HTTPD" = ""; then
      AC_MSG_ERROR(["could not find $HTTPD_EXE (httpd) from apxs"])
    else
      HTTPD_VERSION="`$HTTPD -V | grep 1.3`"
      if test -n "$HTTPD_VERSION"; then
        AC_MSG_RESULT(["$APXS Version $HTTPD_VERSION"])
      else
        AC_DEFINE([APACHE2], [1], [Enables Apache 2 Support])
        APACHE2='yes'
        AC_MSG_RESULT(["$APXS Version 2.X"])
      fi
    fi
    ],
    [
      for i in /usr/local/apache/bin /usr/local/apache2/bin /usr/sbin ; do
        if test -f "$i/apxs2"; then
          APXS="$i/apxs2"
        fi
      done
      for i in /usr/local/apache/bin /usr/local/apache2/bin /usr/sbin ; do
        if test -f "$i/apxs"; then
          APXS="$i/apxs"
        fi
      done
      if test "$APXS" = ""; then
      if test "$APXS" = ""; then
        AC_MSG_ERROR(["could not find apxs"])
      fi

      HTTPD_EXE="`$APXS -q TARGET`"
      HTTPD_DIR="`$APXS -q SBINDIR`"
      HTTPD="$HTTPD_DIR/$HTTPD_EXE"
      APXS_CC="`$APXS -q CC`"
      APXS_INCLUDE="`$APXS -q INCLUDEDIR`"
      APXS_CONFDIR="`$APXS -q SYSCONFDIR`"
      APACHE_LIBEXECDIR="`$APXS -q libexecdir`"

      if test "$HTTPD" = ""; then
        AC_MSG_ERROR(["could not find $HTTPD_EXE (httpd) from apxs"])
      else
        HTTPD_VERSION="`$HTTPD -V | grep 1.3`"
        if test -n "$HTTPD_VERSION"; then
          AC_MSG_RESULT(["$APXS Version $HTTPD_VERSION"])
        else
          AC_DEFINE([APACHE2], [1], [Enables Apache 2 Support])
          APACHE2='yes'
          AC_MSG_RESULT(["$APXS Version 2.X"])
        fi
      fi
    ])
])

dnl ---------------------------------------------------------------------------
dnl END of APACHE_APXS
dnl ---------------------------------------------------------------------------

dnl ---------------------------------------------------------------------------
dnl Macro: APACHE_APR
dnl ---------------------------------------------------------------------------
AC_DEFUN([APACHE_APR], [
  AC_MSG_CHECKING(for apr)
  AC_ARG_WITH(apr,
    [[  --with-apr[=FILE]     Used to determine which version of Apache we
                          are build for.]],
    [
      if test "$APACHE2" = "yes";
      then
        if test "$withval" = "yes";
        then
          for i in /usr/sbin /usr/local/apache/bin /usr/local/apache2/bin \
            /usr/local/apache/sbin /usr/local/apache2/sbin /usr/bin ; do
            if test -f "$i/apr-config"; then
              APR="$i/apr-config"
            fi
          done
          if test "$APR" = ""; then
            AC_MSG_ERROR(["could not find apr-config"])
          else
            AC_MSG_RESULT(["$APR"])
          fi
        else
          APR="$withval"
          APR_VERSION="`$APR -V | grep 1.3`"
          if test -n "$APR_VERSION"; then
            AC_MSG_RESULT(["$APR Version $APR_VERSION"])
          else
            AC_MSG_RESULT(["$APR Version 2.X"])
            AC_DEFINE([APACHE2], [1], [Enables Apache 2 Support])
          fi
        fi
        APR_INCLUDE="`$APR --includedir`"
        APR_LIBS="`$APR --libs`"
      else
        AC_MSG_ERROR(["no need for apr-config"])
      fi
    ],
    [
      if test "$APACHE2" = "yes";
      then
        for i in /usr/bin /usr/local/apache/bin /usr/local/apache2/bin ; do
          if test -f "$i/apr-config"; then
            APR="$i/apr-config"
          fi
        done
        if test "$APR" = ""; then
          AC_MSG_ERROR(["could not find apr-config"])
        else
          AC_MSG_RESULT(["$APR"])
        fi
        APR_INCLUDE="`$APR --includedir`"
        APR_LIBS="`$APR --libs`"
      else
        AC_MSG_RESULT(["no need for apr-config"])
      fi
    ])
])

dnl ---------------------------------------------------------------------------
dnl END of APACHE_APR
dnl ---------------------------------------------------------------------------
