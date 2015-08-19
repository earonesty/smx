# Introduction #

Unix compilation caveats

# Details #

Currently the configure script will not fail if you don't have apache, openssl, gd, sqlite or tdb installed.  It will simply notice they are not there and build smx without it.

( I think it would be nice if it advertised this in big bold print )

If you're using RedHat/Centos:
```

yum install gcc-c++ make unixODBC-devel sqlite-devel perl-devel \
openssl-devel httpd-devel 'perl(ExtUtils::Embed)'
```

The system uses automake, and is pretty standard, so usually stuff just works:```

./configure
make
make install
```

That being said, I've noticed that when moving to a new system, the typical issues are:

  * the ac\_apache.m4 script doesn't take something into account and fails

  * the configure script needs  to be rebuilt ```
./autogen.sh```, since the new system has a newer version of autoconf and has newer m4 scripts that are needed

  * newer version of g++ complains about some hack that works fine but is now deprecated

  * very old system complains about lib being missing

Generally, they haven't been too hard to solve.