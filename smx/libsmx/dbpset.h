#ifdef unix
  #include "unix.h"
#endif

#ifdef HAVE_LIBTDB
  #include <tdb.h>
#else
#ifdef WIN32
  #include <windows.h>
  #include <db_cxx.h>
#else
  #undef min
  #undef max
  #include <db4/db_cxx.h>
#endif
#endif

#include <assert.h>
#include "str.h"

#define HENUM_TREE    0
#define HENUM_KEYS    1
#define HENUM_VALUES  2

typedef void * HTRANS;

// get the latest from www.sleepycat.com

class CDBHash {

#ifdef HAVE_LIBTDB
	TDB_CONTEXT *m_db;
#else
	Db *m_db;
#endif

	char m_path[MAX_PATH];
	bool m_oktxn;
	int  m_triedthispath;
	bool Open();

public:
	CDBHash() {
		m_db = NULL;
		*m_path = '\0';
		m_oktxn = false;
		m_triedthispath = 0;
	}
	~CDBHash() {
		if (m_db) Close();
	}

	bool SetPath(const char *path);
	bool Close();
	char *GetPath() {return m_path;}

	bool Set(const char *path, const char *value, HTRANS txn = NULL) {
		return Set(path, value, strlen(value), txn);
	}
	bool Set(const char *path, const char *value, int vlen, HTRANS txn = NULL);
	CStr Get(const char *path, HTRANS txn = NULL);
	bool Del(const char *path, HTRANS txn = NULL);
	bool Exists(const char *path);
	int Enum(void *obj, const char *path, int mode, bool (*EnumCallback)(void *obj, char *key, int klen, char *val, int vlen));

	HTRANS BeginTrans();
	bool Commit(HTRANS trans);
	bool Rollback(HTRANS trans);

#ifndef HAVE_LIBTDB
	bool Repair();
#endif
};
