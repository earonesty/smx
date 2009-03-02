####### kdevelop will overwrite this part!!! (begin)##########
lib_LTLIBRARIES = libsmx.la libtest_ext.la

# include_HEADERS = crit.h qctx.h qobj-cache.h unix.h qmail_resource.h qobj-ctx.h pstime.h qobj.h qenv.h ary.h qarg.h str.h buf.h buf_ref.h qlib.h unix_interlock.h opt.h libs.h stdafx.h map.h lst.h hash.h crit_win.h crit_unix.h qstr.h smximpl.h smxext.h psxutil.h

libsmx_la_SOURCES = base64.cpp qmail.cpp buf_ref.cpp pscache.cpp math.cpp strx.cpp util.cpp unix.cpp time.cpp tabtd.cpp tabpre.cpp tabfmt.cpp string.cpp stdafx.cpp sqlgrp.cpp sql.cpp sock.cpp sha.cpp regx.cpp qthread.cpp qstr.cpp qsched.cpp qpriq.cpp qobj-ctx.cpp qobj.cpp qfopen.cpp qenv.cpp qctx.cpp qctx-comp.cpp smximpl.cpp pstable.cpp proto.cpp process.cpp opt.cpp open-enc.cpp map.cpp hset.cpp file.cpp ex.cpp dparse.cpp dbh.cpp dbd_sqlite3.cpp dbd_tdb.cpp crit_unix.cpp core.cpp cgi.cpp card.cpp hash.c mapstr.cpp smx_sqlite.cpp psxutil.cpp ary.h open-enc.h qctx-based.h qmail_resource.h qstr-pipe.h sha.h tabpre.h base64.h ex.h optex.h qctx-comp.h qnag.h qstrsock.h smx_sql.h tabtd.h buf.h fbuf.h opt.h qctx.h qobj-cache.h qthread.h smx_sqlite.h tset.h buf_ref.h hash.h pstime.h qenv.h qobj-ctx.h qthread-unix.h sock.h unix.h core.h hset.h smxext.h qobj.h qthread-win.h sqlgrp.h unix_interlock.h crit.h smximpl.h qpriq.h stdafx.h unixsock.h crit_unix.h libs.h qfopen.h qpriqx.h rcex.h strary.h util.h crit_win.h lst.h qgargs.h qsched.h regx.h strex.h dbh.h lstx.h qarg.h qgif.h qstr.h res.h str.h debug.h map.h qlib.h dparse.h mapstr.h qmail.h tabfmt.h psxutil.h gd.cpp dbd_sqlite3.h dbd_tdb.h dbh.h dbd.h

libtest_ext_la_SOURCES = test_ext.cpp

test_ext2_SOURCES = test_ext2.cpp
test_ext2_LDADD = libsmx.la

test_map_SOURCES = test_map.cpp
test_map_LDADD = libsmx.la

test_str_SOURCES = test_str.cpp
test_str_LDADD = libsmx.la

test_eval_SOURCES = test_eval.cpp
test_eval_LDADD = libsmx.la

check_PROGRAMS = test_map test_str test_eval test_ext2

TESTS=test_map test_str test_eval test_ext2
