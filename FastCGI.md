# Introduction #

The smx-fcgi program is not yet installed by default, so you'll have to compile it.  It has only been tested, so far, on Linux & BSD.

# Details #

Use smx-fcgi for servers that support fast-cgi.  It's much faster.

The environment varialbe SMX\_INIT can be used for initialization macros (analogous to the Apache SMXInit directive).

smx-fcgi can be used as an auth filter if the 'http-user' macro
is defined somewhere via SMX\_INIT, just like under apache.

  * http-user is only used if smx-fcgi is also installed as an fast-cgi auth filter
  * http-init is supported as a per-page macro.
  * http-nomagic is supported, just like in Apache, to turn off magic processing, if it evaluates to a true value then all pages will get parsed.

Again, all these macros must be defined by setting someting in the SMX\_INIT environment.

For example in lighthttpd:

> setenv.add-environment = (
> > "SMX\_INIT" => "%module(/var/www/global.smx)",

> )