# Introduction #

SMX is normally used within a web server, from the command line, or as a CGI.

# Details #

Configuring for Apache:

```

#/etc/httpd/conf.d/smx.conf
LoadModule smx_module /usr/lib/libmodsmx.so
AddHandler smx-parsed .html
AddHandler smx-parsed .htm
SMXInit %expand(%include(/your/init/file.htx))
```

On Windows, the paths are different, but it works the same.

After you set this up, you should be able to put "%expand%" at the top of any HTML page, and start making them dynamic.

If you plan on using the %counter macro, the /your/init/file.htx file should probably have the following: %pdbfile(/path/to/file.db), otherwise the system will choose a default which might be in /tmp/pset.db or ~/.smx/pset.db, and could be odd.  You can call %pdbfile% with no arguments on a page to see where it's putting things.