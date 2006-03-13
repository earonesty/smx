/*
Copyright 1998-2005 Erik Aronesty. All rights reserved.

        1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

        2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

	3. Original source code, information and documentation is available at www.smxlang.org

THIS SOFTWARE IS PROVIDED 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE SMX AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

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

#include "dbpset.h"
#include "crit.h"
#include "util.h"

static CMutex gEnvLock("smx-dbpset.cpp-gEnvLock");

#ifndef HAVE_LIBTDB
bool gEnvOpen = false;
#endif

#ifndef HAVE_LIBTDB

class DestroyableDBEnv {
	DbEnv  *myEnv;
	u_int32_t myFlags;

public:

	CStr Dir;
  
	DbEnv *Env() {
		if (!myEnv) {
			myEnv = new DbEnv(myFlags);
			myEnv->set_error_stream(&std::cerr);
		}
		return myEnv;
	}

	DestroyableDBEnv(u_int32_t flags, bool detector = false) {
		myEnv = NULL;
		myFlags = flags;
	}

	~DestroyableDBEnv() {
		try {
			if (myEnv) {
				if (gEnvOpen) {
					myEnv->close(0);
					gEnvOpen = false;
				}
  /* win32 debug mode gets exceptions here.  probably should find out why */
#if !(defined(WIN32) && defined(_DEBUG))
				try {
					delete myEnv;
				} catch (...) {
					smx_log_pf(SMXLOGLEVEL_WARNING, 0, "DbEnvDelete", "Unhandled error");
				}
#endif
				myEnv = NULL;
			}
		} catch (DbException x) {
			if (x.get_errno() != 22 && x.get_errno() != 16) {
	   			smx_log_pf(SMXLOGLEVEL_WARNING, x.get_errno(), "DbEnvClose", x.what());
			}
		} catch (...) {
		}

		try {
			DbEnv tmp(myFlags);
			tmp.remove(Dir, 0);
		} catch (DbException x) {
			if (x.get_errno() != 2) {
	   			smx_log_pf(SMXLOGLEVEL_WARNING, x.get_errno(), "DbEnvRemove", x.what());
			}
		} catch (...) {
		}
	}
};

static DestroyableDBEnv gEnv(0, true);

extern "C" {
	int bt_ci_compare(DB *pdb, const DBT *a, const DBT *b) {
		if (a->size == b->size) {
			return memicmp(a->data, b->data, a->size);
		} else {
			return stricmp((const char *)a->data, (const char *)b->data);
		}
	}
};


bool CDBHash::Repair()
{
	smx_log_pf(SMXLOGLEVEL_WARNING, 0,"DBRepair");

	bool ok = false;
	CMutexLock lock(gEnvLock);

	try {
		smx_log_pf(SMXLOGLEVEL_WARNING, 0, "DBRepairClose", m_path);
		Close();
		Sleep(200);
	} catch (...) {
		smx_log_pf(SMXLOGLEVEL_WARNING, 0, "DBRepairClose", "Unhandled Exception", m_path);
	}

	ok = Open();

	lock.Leave();
	return ok;
}

#endif // #ifndef HAVE_LIBTDB

bool CDBHash::SetPath(const char *path)
{
        if (path && *path) {
		if (strlen(path) >= MAX_PATH) 
			return false ;
	        strncpy(m_path, path, MAX_PATH);
	        m_path[MAX_PATH-1] = '\0';
		m_triedthispath = 0;
	}
	return Close();
}

bool CDBHash::Open()
{
	if (m_db) return true;
	if (!*m_path) return false;

	smx_log_pf(SMXLOGLEVEL_DEBUG, 0, "DbOpenStart", m_path);

	if (++m_triedthispath > 5) {
		smx_log_pf(SMXLOGLEVEL_WARNING, 0, "Giving up on this db", m_path);
		return false;
	}

#ifdef HAVE_LIBTDB
	TDB_CONTEXT *t;
	t = tdb_open(m_path, 0, 0, O_CREAT|O_RDWR, 0666);
	if (!t) {
		usleep(100); 
		t = tdb_open(m_path, 0, 0, O_CREAT|O_RDWR, 0666);
	}
	if (!t) {
		smx_log_pf(SMXLOGLEVEL_WARNING, errno, "TDB Open Failed", m_path, strerror(errno));
	}
#else
	if (m_path) {
	  if (!gEnvOpen) {
	    CMutexLock lock(gEnvLock);
	    CStr env_dir = getenv("SMXHOME");

	    if (env_dir.IsEmpty()) {
#ifdef WIN32
	    env_dir = getenv("TEMP");

	    if (env_dir.IsEmpty()) {
		env_dir = getenv("HOMEDRIVE");
	    	if (env_dir.IsEmpty()) 
		    env_dir = ".";
	    	else
		    env_dir += getenv("HOMEPATH");
	    }
	    env_dir.RTrim('/'); env_dir.RTrim('\\');
	    env_dir = env_dir + "\\.smx";
	    CreateDirectory(env_dir,NULL);
	    char c; for (c = '1'; c < '9'; ++c)
	        remove(((env_dir + "\\__db.00") << c));
#else
	    env_dir = getenv("HOME");
	    if (env_dir.IsEmpty()) 
		    env_dir = "/tmp";
	    env_dir = env_dir + "/.smx";
	    mkdir(env_dir,0775);
#endif
	    }

	    smx_log_pf(SMXLOGLEVEL_DEBUG, 0, "EnvOpen", env_dir);

	    int i;
	    for (i = 0; !gEnvOpen && i < 3; ++i) {
        
            	// try join
	    	try {
	    		if (!gEnv.Env()->open(env_dir, DB_JOINENV,0)) {
  	    			gEnvOpen = true;
	    			gEnv.Dir = env_dir;
            		} 
		} catch (DbException x) {
			if (x.get_errno() != 2) 
				smx_log_pf(SMXLOGLEVEL_WARNING, x.get_errno(), "DbEnvJoin", x.what());
		} catch (...) {
			smx_log_pf(SMXLOGLEVEL_ERROR, 0, "DbEnvJoin", "Unhandled exception", env_dir);
	  	}

	    	if (!gEnvOpen) {
		 try {
			if (!gEnv.Env()->open(env_dir, DB_CREATE | DB_THREAD | DB_INIT_LOCK | DB_INIT_LOG | DB_INIT_TXN | DB_INIT_MPOOL, 0666)) {
  				gEnvOpen = true;
	        		gEnv.Dir = env_dir;
        	   	}
		 } catch (DbException x) {
			smx_log_pf(SMXLOGLEVEL_WARNING, x.get_errno(), "DbEnvOpen", x.what());
			if (x.get_errno() == DB_RUNRECOVERY) {
				try {
					DestroyableDBEnv tEnv(0);
					tEnv.Env()->open(env_dir, DB_CREATE | DB_THREAD | DB_INIT_LOCK | DB_INIT_LOG | DB_INIT_TXN | DB_INIT_MPOOL | DB_RECOVER_FATAL, 0666);
					tEnv.Env()->lock_detect(0, DB_LOCK_DEFAULT, NULL);
					tEnv.Env()->close(0);
				} catch (...) {
				}
			} else {
				smx_log_pf(SMXLOGLEVEL_WARNING, x.get_errno(), "EnvOpen", x.what(), env_dir);
				gEnvOpen = false;
			}
			Sleep(100);
		} catch (...) {
			smx_log_pf(SMXLOGLEVEL_ERROR, 0, "DbEnvOpen", "Unhandled exception", env_dir);
			Sleep(100);
	  	}
               }
	     }
	  }
	}

	m_oktxn = (gEnvOpen && m_path);

	smx_log_pf(SMXLOGLEVEL_DEBUG, m_oktxn, "DBOKTxn", m_path);

	Db *t = new Db(m_oktxn ? gEnv.Env() : NULL, 0);

        try {
		t->set_bt_compare(bt_ci_compare);
        } catch (DbException x) {
		smx_log_pf(SMXLOGLEVEL_WARNING, x.get_errno(), "DbSetCompare", x.what());
        }

#ifndef DB_DIRTY_READ
  #define DB_DIRTY_READ 0
#endif

	try {
#if DB_VERSION_MAJOR == 3 || (DB_VERSION_MAJOR == 4 && DB_VERSION_MINOR == 0)
	   t->open((m_path && *m_path) ? m_path : NULL, NULL, DB_BTREE, DB_CREATE, 0);
#else
	   t->open(NULL, (m_path && *m_path) ? m_path : NULL, NULL, DB_BTREE, DB_CREATE, 0);
#endif
	} catch (DbException x) {
		smx_log_pf(SMXLOGLEVEL_WARNING, x.get_errno(), "DbOpen", x.what(), m_path);
  		m_oktxn = false;
		delete t;
  		return false;
        } catch (...) {
		smx_log_pf(SMXLOGLEVEL_ERROR, 0, "DbOpen", "unknown exception", m_path);
  		m_oktxn = false;
		delete t;
  		return false;
	}

	smx_log_pf(SMXLOGLEVEL_DEBUG, m_oktxn, "DBDoneOpen",m_path);
#endif // HAVE_LIBTDB

	if (t) {
		m_db = t;
		return true;
	} else {
		return false;
	}
}

CStr CDBHash::Get(const char *path, HTRANS txn) {
	if (!Open()) return (const char *) NULL;
  
	
	const char *b = path ? path : "";
	const char *p = b;

	while (*p == '/' || isspace(*p))
		++p;

	b = p;
	p = b + strlen(b);

	if ((p-b) <= 0) return "";


#ifdef HAVE_LIBTDB
	CStr str;
	TDB_DATA key;
	TDB_DATA data;
	key.dptr=(char *)b;
	key.dsize=p-b;

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
			smx_log_pf(SMXLOGLEVEL_WARNING, (int) m_db, "TDB Get", path, tdb_errorstr(m_db));
		if (tdb_error(m_db) == TDB_ERR_CORRUPT) {
			CMutexLock lock(gEnvLock);
			rename(m_path, CStr(m_path) + ".bak");
			//Close();
			if (Open()) {++retry;}
		}
	}
	} while (retry && (retry < 4));

#else

	CStr str(16);

	Dbt key((void *)b,p-b);
 	key.set_flags(DB_DBT_USERMEM);

	Dbt data;

	data.set_data(str.GetBuffer());
	data.set_ulen(str.Length());
	data.set_flags(DB_DBT_USERMEM);

	int ret;
	try {
		ret = m_db->get((DbTxn*)txn, &key, &data, 0);
	} catch (DbException x) {
		
		DBTYPE t = (DBTYPE) 0;
		try{m_db->get_type(&t);}catch(...){};

		if (x.get_errno() != ENOMEM) {
			smx_log_pf(SMXLOGLEVEL_WARNING, t, "DbGet", x.what(), b);
			ret = -1;
		} else {
			ret = 0;
		}

		if (x.get_errno() == DB_RUNRECOVERY && Repair())
			try {
				ret = m_db->get((DbTxn*)txn, &key, &data, 0);
			} catch (DbException x) {
				if (x.get_errno() == DB_RUNRECOVERY)
					exit(0);
				if (x.get_errno() != ENOMEM)
					throw x;
			}
		else if (x.get_errno() != ENOMEM)
			throw x;

	} catch (...) {
		ret = -1;
		smx_log_pf(SMXLOGLEVEL_WARNING, 0, "DbGet", "Unhandled exception", b);
	}

	if (ret == 0) {
		if (data.get_size() > (unsigned int) str.Length()) {
			str.Grow(data.get_size());
			data.set_data(str.GetBuffer());
			data.set_ulen(str.Length());
			if ((ret = m_db->get(NULL, &key, &data, 0)) == 0) {
				return str;
			}
		} else {
			str.Grow(data.get_size());
			return str;
		}
	}
#endif

	return (const char *) NULL;
}

HTRANS CDBHash::BeginTrans() {
	if (!Open()) return NULL;
#ifndef HAVE_LIBTDB
	DbTxn * txn;
  try {
	  if (m_oktxn && gEnvOpen && gEnv.Env()->txn_begin(NULL, &txn, DB_TXN_NOSYNC) == 0) {
		 return (HTRANS) txn;
	  }
	} catch (DbException x) {
	   smx_log_pf(SMXLOGLEVEL_WARNING, x.get_errno(), "DbBeginTrans", x.what());
	} catch (...) {
	}
#endif
	return NULL;
}

bool CDBHash::Commit(HTRANS txn) {
	if (!Open()) return false;
#ifndef HAVE_LIBTDB
	if (txn) {
		try {
	  		int ret = ((DbTxn *)txn)->commit(DB_TXN_NOSYNC);
			return 0 == ret;
  		} catch (DbException x) {
		   smx_log_pf(SMXLOGLEVEL_WARNING, x.get_errno(), "DbCommit", x.what());
  		} catch (...) {
  		}
	}
#endif
	return false;
}

bool CDBHash::Rollback(HTRANS txn) {
	if (!Open()) return false;
#ifndef HAVE_LIBTDB
	if (txn) {
		int ret = ((DbTxn *)txn)->abort();
		return 0 == ret;
	}
#endif
	return false;
}

bool CDBHash::Exists(const char *path) {
	if (!Open()) return false;
	if (!path) return false;
	
	const char *b = path ? path : "";
	const char *p = b;

	while (*p == '/' || isspace(*p))
		++p;

	b = p;
	p = b + strlen(b);

#ifdef HAVE_LIBTDB
        TDB_DATA key;
        key.dptr=(char *)b;
        key.dsize=p-b;
        return tdb_exists(m_db, key);
#else

	Dbt key((void *)b,p-b);
	key.set_flags(DB_DBT_USERMEM);

	Dbt data;
	CStr str(16);
	data.set_data(str.GetBuffer());
	data.set_ulen(str.Length());
	data.set_flags(DB_DBT_USERMEM);

	int ret=-1;
	try {
		ret = m_db->get(NULL, &key, &data, 0);
	} catch (DbException x) {
		smx_log_pf(SMXLOGLEVEL_WARNING, x.get_errno(), "DbExists", x.what());
		if (x.get_errno() == DB_RUNRECOVERY && Repair())
			try {
				ret = m_db->get(NULL, &key, &data, 0);
			} catch (DbException x) {
				if (x.get_errno() == DB_RUNRECOVERY)
					exit(0);
				if (x.get_errno() != ENOMEM)
					throw x;
			}
		else if (x.get_errno() != ENOMEM)
			throw x;
	}

	return (ret == 0);
#endif
}

int CDBHash::Enum(void *obj, const char *path, int mode, bool (*EnumCallback)(void *obj, char *key, int klen, char *val, int vlen)) 
{
	if (!Open()) return false;

#ifndef HAVE_LIBTDB
  
	int n, ret, flag;
	
	char *b = (char *) (path ? path : ""), 
		 *p = b, 
		 *k;

	while (*p == '/' || isspace(*p))
		++p;


	b = p;
	p = b + strlen(b);

	Dbc *dbc;

	m_db->cursor(NULL, &dbc, 0);

	try {

	CStr str(16);
	CStr kstr(b, p-b + 16);
	CStr prev;

	Dbt data;
	data.set_data(str.GetBuffer());
	data.set_ulen(str.Length());
	data.set_flags(DB_DBT_USERMEM);

	Dbt key;
	key.set_data(kstr.GetBuffer());
	key.set_ulen(kstr.Length());
	key.set_size(p-b);
	key.set_flags(DB_DBT_USERMEM);


	if (p-b>0) {
		flag = DB_SET_RANGE;
	} else {
		flag = DB_FIRST;
	}

	try {
		ret = dbc->get(&key, &data, flag);
	} catch (DbException x) {
		if (x.get_errno() != ENOMEM)
			smx_log_pf(SMXLOGLEVEL_WARNING, x.get_errno(), "DbEnum", x.what());

		if (x.get_errno() == DB_RUNRECOVERY && Repair()) {
			try {
				ret = m_db->get(NULL, &key, &data, 0);
			} catch (DbException x) {
				if (x.get_errno() == DB_RUNRECOVERY)
					exit(0);
				if (x.get_errno() != ENOMEM)
					throw x;
			}
		} else if (x.get_errno() != ENOMEM)
			throw x;

		if ((unsigned int) data.get_size() > (unsigned int) str.Length()) {
			str.Grow(data.get_size());
			data.set_data(str.GetBuffer());
			data.set_ulen(str.Length());
		}

		if ((unsigned int) key.get_size() > (unsigned int) kstr.Length()) {
			kstr.Grow(key.get_size());
			key.set_data(kstr.GetBuffer());
			key.set_ulen(kstr.Length());

		}

		try {
			ret = dbc->get(&key, &data, flag);
		} catch (DbException x) {
			if ((unsigned int) data.get_size() > (unsigned int) str.Length()) {
				str.Grow(data.get_size());
				data.set_data(str.GetBuffer());
				data.set_ulen(str.Length());
			}

			if ((unsigned int) key.get_size() > (unsigned int) kstr.Length()) {
				kstr.Grow(key.get_size());
				key.set_data(kstr.GetBuffer());
				key.set_ulen(kstr.Length());

			}
		}
		ret = dbc->get(&key, &data, flag);
	}

	const char *ke;

	n = 0;
	while (ret == 0) {
		++n;

		k = (char *) key.get_data();
		ke = k + key.get_size();

		p = b;
		while (k < ke && *p) {
			if (*k != *p) {
				goto enum_done;
			}
			++k;
			++p;
		}
		if (*p) {
			dbc->close();
			goto enum_done;
		}

		if (mode == HENUM_VALUES) {
			++k;
			while (k < ke) {
				if (*k == '/') {
					goto next_key;
				}
				++k;
			}
		} else if (mode == HENUM_KEYS) {
			if (k >= ke || *k != '/') {
				goto next_key;
			}

			bool ok = false;
			++k;
			while (k < ke) {
				if (*k == '/') {
					ok = true;


					break;
				}
				++k;
			}


			if (!ok)
				goto next_key;


			*k = '\0';
			key.set_size((k - (char *) key.get_data()) - (p-b) - 1);
			key.set_data((char *) key.get_data() + (p-b) + 1);

			if (prev.GetBuffer()) {
				if (!strnicmp(prev, (char*)key.get_data(), prev.Length())) {
					goto next_key;
				}
			}

			prev.Grow(key.get_size());
			memcpy(prev.GetBuffer(), key.get_data(), key.get_size());

			data.set_size(0);
		}
			
		((char *) data.get_data())[data.get_size()] = '\0';

		((char *) key.get_data())[key.get_size()] = '\0';
		
		if (!EnumCallback(obj, (char *)key.get_data(), key.get_size(), (char *)data.get_data(), data.get_size())) {
			dbc->close();
			goto enum_done;
		}

next_key:
		
		try {
			ret = dbc->get(&key, &data, DB_NEXT);
		} catch (DbException x) {
			smx_log_pf(SMXLOGLEVEL_WARNING, x.get_errno(), "DbEnum", x.what());
			if (x.get_errno() == DB_RUNRECOVERY && Repair()) {
				try {
					ret = m_db->get(NULL, &key, &data, 0);
				} catch (DbException x) {
					if (x.get_errno() == DB_RUNRECOVERY)
						exit(0);
					if (x.get_errno() != ENOMEM)

						throw x;
				}
			} else if (x.get_errno() != ENOMEM)
				throw x;
			
			if ((unsigned int) data.get_size() > (unsigned int)str.Length()) {
				str.Grow(data.get_size());
				data.set_data(str.GetBuffer());
				data.set_ulen(str.Length());
			}

			if ((unsigned int)key.get_size() > (unsigned int)kstr.Length()) {
				kstr.Grow(key.get_size());
				key.set_data(kstr.GetBuffer());
				key.set_ulen(kstr.Length());
			}


			ret = dbc->get(&key, &data, DB_NEXT);
		}
	}

	} catch (...) {
		smx_log_pf(SMXLOGLEVEL_WARNING, -1, "DbEnum", "Unhandled exception");
	}

enum_done:

	if (dbc)
		dbc->close();

	return n;
#else
	return 0;
#endif
}


bool CDBHash::Del(const char *path, HTRANS txn) {
	if (!Open()) return false;
	if (!path)  return false;

	const char *b = (path ? path : "");
	const char *p = b;

	while (*p == '/' || isspace(*p))
		++p;

	b = p;
	p = b + strlen(b);

#ifdef HAVE_LIBTDB
        CStr str;
        TDB_DATA key;
        key.dptr=(char *)b;
        key.dsize=p-b;
        return tdb_delete(m_db, key) == 0;
#else
	Dbt key((void *)b,p-b);
	key.set_flags(DB_DBT_USERMEM);
	Dbt data;
	data.set_flags(DB_DBT_USERMEM);

	try {
		if (m_db->del((DbTxn*)txn, &key, 0) == 0) {
			return true;
		} else {
			return false;
		}
	} catch (...) {
		return false;
	}

#endif
}

bool CDBHash::Set(const char *path, const char *val, int vlen, HTRANS txn) {
	if (!Open()) return false;
	if (!path)  return false;
  
	const char *b = path ? path : "";
	const char *p = b;

	while (*p == '/' || isspace(*p))
		++p;

	b = p;
	p = b + strlen(b);

#ifdef HAVE_LIBTDB
	int retry = 0;
        do {
        CStr str;
        TDB_DATA key;
        TDB_DATA data;
        key.dptr=(char *)b;
        key.dsize=p-b;
        data.dptr=(char *)val;
        data.dsize=vlen;
        if (tdb_store(m_db, key, data, TDB_REPLACE) == 0) {
		return true;
	} else {
		smx_log_pf(SMXLOGLEVEL_WARNING, (int) m_db, "TDB Set", path, tdb_errorstr(m_db));
                if (tdb_error(m_db) == TDB_ERR_CORRUPT) {
                        CMutexLock lock(gEnvLock);
                        rename(m_path, CStr(m_path) + ".bak");
			//Close();
                        if (Open()) {++retry;}
                }

	}
	} while (retry && (retry < 4));

	return false;
#else
	Dbt key((void *)b,p-b);
	key.set_flags(DB_DBT_USERMEM);
	Dbt data((void *)val, vlen);
	data.set_flags(DB_DBT_USERMEM);

	try {
		if ((m_db->put((DbTxn*)txn, &key, &data, 0)) == 0) {
			return true;
		} else {
	//		m_db->err(ret, "%s", path);
			return false;
		}
	} catch (...) {
		return false;
	}
#endif
}

bool CDBHash::Close()
{
  if (!m_db) return true;
 
  smx_log_pf(SMXLOGLEVEL_DEBUG, 0, "DBClose", m_path);

#ifdef HAVE_LIBTDB
  TDB_CONTEXT *t = m_db;
  m_db = NULL;
  bool ret;
  ret = tdb_close(t);

// i think tdb_lib calls free
// delete t;

#else
  Db *t = m_db;
  m_db = NULL;
  m_triedthispath=0;

  bool ret = false;
  try {
	ret = (t->close(0) == 0);
  } catch (DbException x) {
	smx_log_pf(SMXLOGLEVEL_WARNING, x.get_errno(), "DbClose", x.what());
  } catch (...) {
	smx_log_pf(SMXLOGLEVEL_WARNING, -1, "DbClose","Unhandled Exception");
  }

  /* win32 debug mode gets exceptions here.  probably should find out why */
#if !(defined(WIN32) && defined(_DEBUG))
  try {
	  delete t;
  } catch (...) {
	  smx_log_pf(SMXLOGLEVEL_WARNING, 0, "DbDelete", "Unhandled error");
  }
#endif

#endif

  return ret;
}

