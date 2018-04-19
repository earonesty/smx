/*
Copyright 1998-2005 Erik Aronesty. All rights reserved.

        1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

        2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

	3. Original source code, information and documentation is available at www.smxlang.org

THIS SOFTWARE IS PROVIDED 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE SMX AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#include "unix.h"

#ifdef HAVE_SQLITE3_H

#include <memory.h>
#include <string.h>
#include <stdlib.h>

#include "dbd.h"
#include "dbd_sqlite3.h"
#include "util.h"

CDBDriverSqlite::CDBDriverSqlite(const char *path)
{

#ifdef sqlite3_prepare_v2
	#define SQLITE3_PREP sqlite3_prepare_v2
#else
	#define SQLITE3_PREP sqlite3_prepare
#endif

	m_db = NULL;
	m_st_get = NULL;	
	m_st_set = NULL;	
	m_st_del = NULL;	
        int err = sqlite3_open(path, &m_db);
        if (err) {
                smx_log_pf(SMXLOGLEVEL_WARNING, err, "SQLITE3 open failed", path, m_db ? sqlite3_errmsg(m_db) : "");
        } else {
		const char *p;
		sqlite3_busy_timeout(m_db, 400);
		char *msg = NULL;
		if (err=sqlite3_exec(m_db, "create table if not exists h (k text primary key, v text)", NULL, NULL, &msg))  {
                	if (err != SQLITE_NOTADB) {
				smx_log_pf(SMXLOGLEVEL_WARNING, err, "SQLITE3 create table failed", path, msg);
			}
		}
                if (err != SQLITE_NOTADB) {
			if (err=SQLITE3_PREP(m_db, "select v from h where k=?", -1, &m_st_get, &p)) {
				smx_log_pf(SMXLOGLEVEL_WARNING, err, "SQLITE3 prepare stmt get failed", path, sqlite3_errmsg(m_db));
			}
			if (err=SQLITE3_PREP(m_db, "delete from h where k=?", -1, &m_st_del, &p)) {
				smx_log_pf(SMXLOGLEVEL_WARNING, err, "SQLITE3 prepare stmt del failed", path, sqlite3_errmsg(m_db));
			}
			if (err=SQLITE3_PREP(m_db, "insert or replace into h (k, v) values (?, ?)", -1, &m_st_set, &p)) {
				smx_log_pf(SMXLOGLEVEL_WARNING, err, "SQLITE3 prepare stmt set failed", path, sqlite3_errmsg(m_db));
			}
		}
	}

	if (m_db) {
		if (!m_st_get || !m_st_del || !m_st_set) {
			sqlite3_close(m_db);
			m_db = NULL;
		} else {
			m_path = path;
		}
	} else {
		m_db = NULL;
	}
}

CStr CDBDriverSqlite::Get(const char *key, HTRANS txn) {
	sqlite3_bind_text(m_st_get, 1, key, strlen(key), SQLITE_STATIC);
	int rv = sqlite3_step(m_st_get);
	if (rv == SQLITE_ROW) {
		CStr str((const char *) sqlite3_column_text(m_st_get, 0), sqlite3_column_bytes(m_st_get, 0));
		sqlite3_reset(m_st_get);
		return str;
	}
	if (rv == SQLITE_DONE) {
		sqlite3_reset(m_st_get);
	} else {
        	smx_log_pf(SMXLOGLEVEL_WARNING, rv, "SQLITE3 get error", m_path, sqlite3_errmsg(m_db));
		Close();
	}
	return (const char *) NULL;
}

bool CDBDriverSqlite::Exists(const char *key) {
        sqlite3_bind_text(m_st_get, 1, key, strlen(key), SQLITE_STATIC);
        int rv = sqlite3_step(m_st_get);
        if (rv == SQLITE_ROW) {
                sqlite3_reset(m_st_get);
                return true;
        } else if (rv == SQLITE_DONE) {
                sqlite3_reset(m_st_get);
        } else {
        	smx_log_pf(SMXLOGLEVEL_WARNING, rv, "SQLITE3 exists error", m_path, sqlite3_errmsg(m_db));
                Close();
        }
	return false;
}

bool CDBDriverSqlite::Del(const char *key, HTRANS txn) {
	int rv;
	sqlite3_bind_text(m_st_del, 1, key, strlen(key), SQLITE_STATIC);
	if ((rv = sqlite3_step(m_st_del)) != SQLITE_DONE) {
        	smx_log_pf(SMXLOGLEVEL_WARNING, rv, "SQLITE3 del error", m_path, sqlite3_errmsg(m_db));
	}
	sqlite3_reset(m_st_del);
}

bool CDBDriverSqlite::Set(const char *key, const char *val, int vlen, HTRANS txn) {
        sqlite3_bind_text(m_st_set, 1, key, strlen(key), SQLITE_STATIC);
        sqlite3_bind_text(m_st_set, 2, val, vlen, SQLITE_STATIC);
        int rv = sqlite3_step(m_st_set);
        if (rv == SQLITE_DONE) {
                sqlite3_reset(m_st_set);
        } else {
                smx_log_pf(SMXLOGLEVEL_WARNING, rv, "SQLITE3 set error", m_path, sqlite3_errmsg(m_db));
                Close();
        }
}

CDBDriverSqlite::~CDBDriverSqlite() 
{
	Close();
}

bool CDBDriverSqlite::Close()
{
  if (!m_db) return true;

  if (m_st_set) sqlite3_finalize(m_st_set);
  if (m_st_get) sqlite3_finalize(m_st_get);
  if (m_st_del) sqlite3_finalize(m_st_del);
 
  bool ret;
  ret = !sqlite3_close(m_db);
  m_db = NULL;
  return ret;
}

#endif
