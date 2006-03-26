rm -f config.cache
rm -f config.h
libtoolize --automake --copy --force
aclocal -I m4
autoconf
touch README AUTHORS ChangeLog
autoheader
automake -a -c

