/*
Copyright 1998-2005 Erik Aronesty. All rights reserved.

        1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

        2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

        3. Original source code, information and documentation is available at www.smxlang.org

THIS SOFTWARE IS PROVIDED 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE SMX AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#include "dbh.h"

class CDBDriver {
protected:
	const char *m_path;
public:
// required
        virtual bool IsOpen() = 0;
        virtual bool Set(const char *key, const char *value, int vlen, HTRANS txn = NULL) = 0;
        virtual CStr Get(const char *key, HTRANS txn = NULL) = 0;
        virtual bool Del(const char *key, HTRANS txn = NULL) = 0;
        virtual bool Exists(const char *key) = 0;
        virtual bool Close() = 0;

// optional
        virtual HTRANS BeginTrans() {return NULL;}
        virtual bool Commit(HTRANS trans) {return false;}
        virtual bool Rollback(HTRANS trans) {return false;}
        virtual int  Enum(void *obj, const char *key, int mode, bool (*EnumCallback)(void *obj, char *key, int klen, char *val, int vlen)) {
		return 0;
	}
};

