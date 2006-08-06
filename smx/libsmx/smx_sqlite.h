/*
Copyright 1998-2005 Erik Aronesty. All rights reserved.

        1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

        2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

	3. Original source code, information and documentation is available at www.smxlang.org

THIS SOFTWARE IS PROVIDED 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE SMX AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#include "smx_sql.h"
#include <sqlite3.h>

class qObjSqlite : public qObjSql {
	int myArgC;
	char ** myArgV;
	char ** myColNames;
        sqlite3 *myDB;
	bool myDBOpen;
	int myRow;
	CMapStr<int> myColMap;
	bool myMapped;
	void MakeColMap();

	bool myOK;
	qCtx *myCtx;
	qStr *myOut;	
	CStr myHead;
	CStr myBody;

public:
        qObjSqlite(const char *dsn, qCtx *ctx);
        ~qObjSqlite();
        bool Open(const char *dsn, qCtx *ctx);
	void Execute(qCtx *ctx, qStr *out, const char *sql, CStr &body, CStr &head, CStr &foot);
	int Callback(int argc, char **argv, char **azColName);
        void EvalColName(qCtx *ctx, qStr *out, qArgAry *args);
        void EvalColType(qCtx *ctx, qStr *out, qArgAry *args);
        void EvalRow(qCtx *ctx, qStr *out, qArgAry *args);
        void EvalSkipRows(qCtx *ctx, qStr *out, qArgAry *args);
        void EvalCol(qCtx *ctx, qStr *out, qArgAry *args) ;
        void EvalColumn(qCtx *ctx, qStr *out, qArgAry *args) ;
        void EvalEnumCol(qCtx *ctx, qStr *out, qArgAry *args) ;
};
