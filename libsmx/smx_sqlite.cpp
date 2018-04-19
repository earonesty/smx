/*
Copyright 1998-2005 Erik Aronesty. All rights reserved.

        1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

        2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

	3. Original source code, information and documentation is available at www.smxlang.org

THIS SOFTWARE IS PROVIDED 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE SMX AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/


#include "stdafx.h"
#include "util.h"

#ifdef HAVE_SQLITE3_H

#include "qstr.h"
#include "qobj.h"
#include "qctx.h"

#include <sqlite3.h>

#include "smx_sqlite.h"

qObjSqlite::qObjSqlite(const char *dsn, qCtx *ctx)
{
	myDB = NULL;
	myDBOpen = false;
	myMapped = false;
	Open(dsn, ctx);
}

qObjSqlite::~qObjSqlite()
{
	if (myDB) {
		sqlite3_close(myDB);
		myDB=NULL;
		myDBOpen=false;
	}
}

bool qObjSqlite::Open(const char *dsn, qCtx *ctx)
{
	if (myDB) {
		sqlite3_close(myDB);
		myDB=NULL;
		myDBOpen=false;
	}
	if(!safe_fcheck(ctx, dsn, 'w')) {
		return false;
	}
        int rc = sqlite3_open(dsn, &myDB);
        myDBOpen = !rc;
	if (myDBOpen)
		sqlite3_busy_timeout(myDB, 100);
	return myDBOpen;
}

static int my_Callback(void *obj, int argc, char **argv, char **azColName) {
  return ((qObjSqlite *)obj)->Callback(argc, argv, azColName);
}

int qObjSqlite::Callback(int argc, char **argv, char **azColName) {
	myArgC = argc;
	myArgV = argv;
	myColNames = azColName;
	++myRow;
	if (myRow == 1) {
		myCtx->Parse(myHead, myOut);
	}
	if (myOK)
		myCtx->Parse(myBody, myOut);
	if (!myOK)
		return -1;
	return 0;
}

void qObjSqlite::Execute(qCtx *ctx, qStr *out, const char *sql, CStr &body, CStr &head, CStr &foot)
{
	if (!myDBOpen) {
		if (myDB) {
			ctx->Throw(out, -1, sqlite3_errmsg(myDB));
		} else {
			ctx->Throw(out, -1, "database could not be opened");
		}
		return;
	}
	myMapped = false;
	char *zErrMsg = 0;
	myCtx = ctx;
	myOut = out;
	myBody=body;
	myHead=head;
	myRow=0;
	myOK=true;
	ctx->MapObj(&myOK, (QOBJFUNC) EvalBreak, "break");
	int rc = sqlite3_exec(myDB, sql, my_Callback, (void *) this, &zErrMsg);
	if (rc!=SQLITE_OK) {
		ctx->Throw(out, -1, zErrMsg);
		return;
	}
}

void qObjSqlite::EvalColName(qCtx *ctx, qStr *out, qArgAry *args)
{
        int i = ParseInt((*args)[0]);
	if (i < myArgC) out->PutS(myColNames[i]);
}

void qObjSqlite::EvalColType(qCtx *ctx, qStr *out, qArgAry *args)
{
        int i = ParseInt((*args)[0]);
        if (i < myArgC) out->PutS("0");
}

void qObjSqlite::EvalRow(qCtx *ctx, qStr *out, qArgAry *args)
{
	out->PutN(myRow);
}


void qObjSqlite::EvalSkipRows(qCtx *ctx, qStr *out, qArgAry *args)
{
}

void qObjSqlite::MakeColMap()
{
	int i;
	for (i=0; i < myArgC; ++i) {
		myColMap[myColNames[i]]=i;
	}
	myMapped = true;
}

void qObjSqlite::EvalCol(qCtx *ctx, qStr *out, qArgAry *args)
{
	const char * name = args->GetAt(0);
	int col = -1;
	if (isdigit(name[0])) {
		col = atoi(name)-1;
	} else {
		if (!myMapped) MakeColMap();
		col = myColMap[name];
	}
	if (col >= 0 && col < myArgC) {
		char *p = myArgV[col];
		if (p) {
		while (isspace(*p)) ++p;
		char *r = p + strlen(p) - 1;
		while (r >= p && isspace(*r)) 
			--r;
		r[1]='\0';
		out->PutS(p);
		}
	}
}

void qObjSqlite::EvalColumn(qCtx *ctx, qStr *out, qArgAry *args)
{
	const char * name = args->GetAt(0);
        int col = -1;
        if (isdigit(name[0])) {
                col = atoi(name)-1;
        } else {
                if (!myMapped) MakeColMap();
                col = myColMap[name];
        }
        if (col >= 0 && col < myArgC) {
                out->PutS(myArgV[col]);
        }
}

void qObjSqlite::EvalEnumCol(qCtx *ctx, qStr *out, qArgAry *args)
{
        if (args->Count() == 0)
                return;

        int   num, type=0, size=0;

        const char *name=0;
        const char *value=0;
        bool  ok = true;

        qCtxTmp sub(ctx);

        sub.MapObj(&num,   "num");
        sub.MapObj(&name,  "name");
        sub.MapObj(&value, "value");
        sub.MapObj(&type,  "type");
        sub.MapObj(&size,  "size");
        sub.MapObj(&ok, (QOBJFUNC) EvalBreak, "break");

        for(num = 1; ok && num <= myArgC; ++num) {
                name = myColNames[num-1];
		value = myArgV[num-1];
                sub.qCtx::Parse(args->GetAt(0), out);
        }
};

#endif
