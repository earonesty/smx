/*
Copyright 1998-2005 Erik Aronesty. All rights reserved.

        1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

        2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

	3. Original source code, information and documentation is available at www.smxlang.org

THIS SOFTWARE IS PROVIDED 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE SMX AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#ifndef _DBH_H_
#define _DBH_H_

#include <assert.h>
#include "str.h"

#define HENUM_TREE    0
#define HENUM_KEYS    1
#define HENUM_VALUES  2

typedef void * HTRANS;

class CDBDriver;

class CDBHash {
private:

	CDBDriver * m_v;

	bool Open();

protected:
	char m_path[MAX_PATH];
	bool m_oktxn;
	int  m_triedthispath;
public:
	CDBHash() {
		*m_path = '\0';
		m_oktxn = false;
		m_triedthispath = 0;
		m_v = 0;
	}
	~CDBHash();

	bool IsOpen();
	bool Close();

	bool SetPath(const char *path);
	char *GetPath() {return m_path;}

	bool Set(const char *path, const char *value, HTRANS txn = NULL) {
		return Set(path, value, strlen(value), txn);
	}

	bool Set(const char *path, const char *value, int vlen, HTRANS txn = NULL);
	CStr Get(const char *path, HTRANS txn = NULL);
	bool Del(const char *path, HTRANS txn = NULL);
	bool Exists(const char *path);
	int  Enum(void *obj, const char *path, int mode, 
		bool (*EnumCallback)(void *obj, char *key, int klen, char *val, int vlen));

	HTRANS BeginTrans(); 
	bool Commit(HTRANS trans); 
	bool Rollback(HTRANS trans);
};

#endif
