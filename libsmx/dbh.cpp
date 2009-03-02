/*
Copyright 1998-2005 Erik Aronesty. All rights reserved.

        1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

        2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

	3. Original source code, information and documentation is available at www.smxlang.org

THIS SOFTWARE IS PROVIDED 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE SMX AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#include <memory.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "dbh.h"
#include "dbd.h"

#include "util.h"

#ifdef HAVE_SQLITE3_H
        #define DBH_SQLITE3
	#include "dbd_sqlite3.h"
#endif

#ifdef HAVE_TDB_H
        #define DBH_TDB
	#include "dbd_tdb.h"
#endif

#ifdef HAVE_DBCXX_H
        #define DBH_BDB
	#include "dbd_bdb.h"
#endif

#define KEY_CLEAN(KEY) \
        {const char *b = KEY ? KEY : ""; \
        const char *p = b;\
        while (*p == '/' || isspace(*p)) \
                ++p;\
        KEY = p;}

CDBHash::~CDBHash() {
	if (m_v) delete m_v;
}

bool CDBHash::IsOpen() { 
	return m_v ? m_v->IsOpen() : false; 
}

bool CDBHash::Close() {
	return m_v ? m_v->Close() : false; 
}

HTRANS CDBHash::BeginTrans() { 
	return m_v ? m_v->BeginTrans() : NULL; 
}

bool CDBHash::Commit(HTRANS trans) { 
	return m_v ? m_v->Commit(trans) : false; 
}

bool CDBHash::Rollback(HTRANS trans) { 
	return m_v ? m_v->Rollback(trans) : false; 
};

bool CDBHash::SetPath(const char *path)
{
        if (path && *path) {
		if (strlen(path) >= MAX_PATH) 
			return false;

		if (strncmp(m_path, path, MAX_PATH)) { 
		        strncpy(m_path, path, MAX_PATH);
		        m_path[MAX_PATH-1] = '\0';
			m_triedthispath = 0;
			return Close();
		} else 
			return true;
	}
	return false;
}

bool CDBHash::Open()
{
	if (IsOpen()) return true;
	if (!*m_path) return false;

	smx_log_pf(SMXLOGLEVEL_DEBUG, 0, "DbOpenStart", m_path);

	if (++m_triedthispath > 5) {
		smx_log_pf(SMXLOGLEVEL_WARNING, 0, "Giving up on this db", m_path);
		return false;
	}

	CDBDriver *v = NULL;
	CDBDriver *t = NULL;

#ifdef DBH_SQLITE3
	t = new CDBDriverSqlite(m_path);
	if (t->IsOpen()) v = t;
#endif

#ifdef DBH_TDB
        t = new CDBDriverTdb(m_path);
	if (t->IsOpen()) v = t;
#endif

#ifdef DBH_BDB
        t = new CDBDriverBdb(m_path);
	if (t->IsOpen()) v = t;
#endif

        if (v) {
                m_v = v;
                return true;
        }

	return false;
}

CStr CDBHash::Get(const char *key, HTRANS txn) {
	if (!Open()) return (const char *) NULL;
	if (!key) return false;
	KEY_CLEAN(key);  
	return m_v->Get(key, txn);
}

bool CDBHash::Exists(const char *key) {
	if (!Open()) return false;
	if (!key) return false;
        KEY_CLEAN(key);
	return m_v->Exists(key);
}

bool CDBHash::Del(const char *key, HTRANS txn) {
	if (!Open()) return false;
	if (!key)  return false;
	KEY_CLEAN(key);
	return m_v->Del(key, txn);
}

bool CDBHash::Set(const char *key, const char *val, int vlen, HTRANS txn) {
	if (!Open()) return false;
	if (!key)  return false;
 	KEY_CLEAN(key); 
	return m_v->Set(key, val, vlen, txn);
}

int CDBHash::Enum(void *obj, const char *key, int mode, bool (*EnumCallback)(void *obj, char *key, int klen, char *val, int vlen)) {
        if (!Open()) return false;
        if (!key)  return false;
        KEY_CLEAN(key);
        return m_v->Enum(obj, key, mode, EnumCallback);
}
