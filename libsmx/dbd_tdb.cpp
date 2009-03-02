/*
Copyright 1998-2005 Erik Aronesty. All rights reserved.

        1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

        2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

	3. Original source code, information and documentation is available at www.smxlang.org

THIS SOFTWARE IS PROVIDED 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE SMX AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#include "unix.h"

#ifdef HAVE_TDB_H

#define MAP_THRESH_N 5
#define MAP_THRESH_D 6

#include <memory.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <iostream>
#include <fcntl.h>

#ifdef WIN32
	#include <process.h>
	#define STDCALL _stdcall
#endif


#include "dbd.h"
#include "dbd_tdb.h"

#include "crit.h"
#include "util.h"

static CMutex gEnvLock("smx-dbpset.cpp-gEnvLock");

#include <tdb.h>

CDBDriverTdb::CDBDriverTdb(const char * path)
{
	m_path = path;
	Reopen();
}

bool CDBDriverTdb::Reopen() {
	TDB_CONTEXT *t;
	t = tdb_open((char *) m_path, 0, 0, O_CREAT|O_RDWR, 0666);
	if (!t) {
		usleep(100); 
		t = tdb_open((char *) m_path, 0, 0, O_CREAT|O_RDWR, 0666);
	}
	if (!t) {
		smx_log_pf(SMXLOGLEVEL_WARNING, errno, "TDB Open Failed", m_path, strerror(errno));
	}
	if (t) {
		m_db = t;
		return true;
	} else {
		m_db = NULL;
		return false;
	}
}

CStr CDBDriverTdb::Get(const char *name, HTRANS txn) {
	CStr str;
	TDB_DATA key;
	TDB_DATA data;
	key.dptr=(char *)name;
	key.dsize=strlen(name);

	int retry = 0;
	do {
	data=tdb_fetch(m_db, key);
	if (data.dptr) {
		str.Grow(data.dsize);
		memcpy(str.GetBuffer(),data.dptr,data.dsize);
		free(data.dptr);
                return str;
        } else {
                if (tdb_error(m_db) != TDB_ERR_NOEXIST) 
			smx_log_pf(SMXLOGLEVEL_WARNING, (long) m_db, "TDB Get", name, tdb_errorstr(m_db));
		if (tdb_error(m_db) == TDB_ERR_CORRUPT) {
			CMutexLock lock(gEnvLock);
			rename(m_path, CStr(m_path) + ".bak");
			//Close();
			if (Reopen()) {++retry;}
		}
	}
	} while (retry && (retry < 4));
}

bool CDBDriverTdb::Exists(const char *name) {
        TDB_DATA key;
        key.dptr=(char *)name;
        key.dsize=strlen(name);
        return tdb_exists(m_db, key);
}

bool CDBDriverTdb::Del(const char *name, HTRANS txn) {
        CStr str;
        TDB_DATA key;
        key.dptr=(char *)name;
        key.dsize=strlen(name);
        return tdb_delete(m_db, key) == 0;
}

bool CDBDriverTdb::Set(const char *name, const char *val, int vlen, HTRANS txn) {
	int retry = 0;
        do {
        CStr str;
        TDB_DATA key;
        TDB_DATA data;
        key.dptr=(char *)name;
        key.dsize=strlen(name);
        data.dptr=(char *)val;
        data.dsize=vlen;
        if (tdb_store(m_db, key, data, TDB_REPLACE) == 0) {
		return true;
	} else {
		smx_log_pf(SMXLOGLEVEL_WARNING, (long) m_db, "TDB Set", name, tdb_errorstr(m_db));
                if (tdb_error(m_db) == TDB_ERR_CORRUPT) {
                        CMutexLock lock(gEnvLock);
                        rename(m_path, CStr(m_path) + ".bak");
                        if (Reopen()) {++retry;}
                }

	}
	} while (retry && (retry < 4));

	return false;
}

bool CDBDriverTdb::Close()
{
  if (!m_db) return true;

  bool ret;
  TDB_CONTEXT *t = m_db;
  m_db = NULL;
  ret = tdb_close(t);
  return ret;
}

#endif
