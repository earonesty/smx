/*
Copyright 1998-2005 Erik Aronesty. All rights reserved.

        1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

        2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

	3. Original source code, information and documentation is available at www.smxlang.org

THIS SOFTWARE IS PROVIDED 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE SMX AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/


#include "stdafx.h"

#undef UCHAR
#undef DWORD
#undef BOOL
#include <sql.h>
#include <sqlext.h>

#include "qstr.h"
#include "qobj.h"
#include "qctx.h"

#include "util.h"

#include <assert.h>

#ifdef WIN32
	#include <process.h>
#endif

#include "smx_sql.h"

#ifdef HAVE_SQLITE3_H
	#include "smx_sqlite.h"
#endif

class CDbLib;
class CDbConn;
class CDbStmt;
class CDbCol;

#define DEFAULT_MAX_BUFFER 12000
#ifdef _DEBUG
#define DEFAULT_TIMEOUT 15
#else
#define DEFAULT_TIMEOUT 120
#endif

extern CDbLib *myDBL;

void EvalSql(const void *data, qCtx *ctx, qStr *out, qArgAry *args);
void EvalSqlTrans(const void *data, qCtx *ctx, qStr *out, qArgAry *args);
void EvalSqlMax(const void *data, qCtx *ctx, qStr *out, qArgAry *args);
void EvalSqlTimeout(const void *data, qCtx *ctx, qStr *out, qArgAry *args);
void EvalSqlGrp(const void *data, qCtx *ctx, qStr *out, qArgAry *args);
void EvalSqlQ(const void *data, qCtx *ctx, qStr *out, qArgAry *args);
void ParseDSN(char *cstr, char *&dsn, char *&uid, char *&pwd, char *&etc);

CDbLib *GetDbLib(qCtx *ctx);

class CDbHandle
{
	CStr myErrBuf;
	CStr myErrState;
	SQLINTEGER myErrCode;
	long myErrRes;

	SQLHANDLE   myHandle;
	SQLSMALLINT myType;
	int         myRefs;
	
	SQLRETURN _Close() {
		if (myHandle) {
			SQLRETURN nRet;
			
			if (myType == SQL_HANDLE_DBC) {
				nRet = SQLDisconnect(myHandle);
			} else if (myType == SQL_HANDLE_STMT) {
				nRet = SQLCloseCursor(myHandle);
			} else {
				nRet = SQL_SUCCESS;
			}

			return SQLFreeHandle(myType, myHandle); 
			myHandle = 0; 
		} 
		return SQL_SUCCESS;
	}

public:
	CDbHandle(SQLHANDLE handle, SQLSMALLINT type) 
		{ myHandle = handle; myType = type; myErrRes = 0; myRefs = 1; }
	~CDbHandle() 
		{ assert(myRefs == 0); }
	void Free()    
		{ if (--myRefs == 0) { _Close(); delete this;} }
	bool LastRef()    
		{ return (myRefs == 1); }
	void AddRef()  
		{ ++myRefs; }
	operator SQLHANDLE () 
		{return myHandle;}
	void Close() 
		{ if (--myRefs == 0) { _Close(); delete this;} else _Close(); }

	void ClearError() {
		myErrRes = 0;
	}
	bool SetError(int nResult, bool bFree = true) {
		if (!myErrRes 
			&& !SQL_SUCCEEDED(nResult)) {
				myErrBuf.Grow(1024);
				myErrState.Grow(6);
				SQLSMALLINT tlen = 0;
				if (!SQLGetDiagRec(myType, myHandle, 1, (SQLCHAR*) myErrState.GetBuffer(), &myErrCode, (SQLCHAR*) (myErrBuf.GetBuffer())+8, myErrBuf.Length()-8, &tlen)) {
					myErrBuf[0] = '(';
					memcpy(myErrBuf+1, (const char *)myErrState, 5);
					myErrBuf[6] = ')';
					myErrBuf[7] = ' ';
					myErrBuf.Grow(min(tlen+8,myErrBuf.Length()-8));
					myErrRes = nResult;
				} else {
					myErrState = "00000";
					myErrBuf = "(00000) No error.";
					myErrRes = nResult;
				}
				if (bFree) {
					SQLFreeHandle(myType, myHandle);
					myHandle = 0;
				}
				return false;
		}
		return true;
	}
	CStr GetErrorMsg() {
		return myErrBuf;
	}
	CStr GetErrorState() {
		return myErrState;
	}
	long GetErrorCode() {
		return myErrCode;
	}
};


class CDbStmt
{
	CDbConn  *myDbConn;

	char   *myMem;			// allocated for indptr and buffer
	long    myColN;			// bound count of columns
	long    myRow;			// current
	bool    myDone;
	bool    myHasData;      // ever had 1 row?
	long    myRowsetMax;    // maximum #rows bound per column
	long    myRowsetSize;	// current #rows bound per column
	long    myBufIndex;	// index into buffer

//	CMap<const char *, const char *, CDbCol *> myColMap;
	CMapStr<CDbCol *> myColMap;

	void FreeMem();

	void Init() {
		myDone = false; 
		myCols = NULL; 
		myMem = NULL; 
		myRow = -1; 
		myRowsetMax = 0;
		myRowsetSize = 0;
		myHasData = false;
		myBufIndex = 0;
		myColN = 0;
	}

public:
	CDbCol *myCols;			// list of columns
	CDbHandle *myHSTMT;

	~CDbStmt();

	CDbStmt(CDbConn *conn, const SQLHSTMT hstmt)  { 
		Init();
		myDbConn = conn;
		myHSTMT = hstmt ? new CDbHandle(hstmt, SQL_HANDLE_STMT) : 0; 
	}
	CDbStmt(const CDbStmt &stmt)         { 
		Init();
		myHSTMT = NULL; 
		myDbConn = NULL; 
		operator = (stmt); 
	}
	CDbStmt()                      { 
		Init();
		myHSTMT = NULL; 
		myDbConn = NULL; 
	}

	void Close() 				   { if (myHSTMT) myHSTMT->Close(); myHSTMT = NULL; }

	bool Bind();
	int  ColCount()				   { return myColN; }
	int  RowCount()				   { return myRow;  }
	CDbCol *Column(int index);
	CDbCol *Column(const char * index);
	int  BufIndex()                { return myBufIndex; }
	bool Next();
	bool Skip(int n);
	bool Done()					   { return myDone; }
	bool HasData()				   { return myHasData; }

	void Finish() {
		if (!myDone) {
			if (myHSTMT && *myHSTMT) {
				SQLRETURN nReturn;
				do {
					nReturn = SQLMoreResults(*myHSTMT);
				} while (SQL_SUCCEEDED(nReturn));
			}
			myDone = true;
		}
	}

	CStr    GetErrorMsg()			 
		{ return (myHSTMT != NULL)? myHSTMT->GetErrorMsg() : CStr("Unknown error."); }

	bool SetError(int nResult, bool bFree = true)
		{ return myHSTMT ? myHSTMT->SetError(nResult, bFree) : false; }

	operator bool () 
		{ return myHSTMT ? *myHSTMT != 0 : false; }

	CDbStmt &operator = (const CDbStmt &stmt) {
		myDbConn = stmt.myDbConn;

		if (myHSTMT) 
			myHSTMT->Free(); 
		myHSTMT = stmt.myHSTMT; 
		if (myHSTMT) 
			myHSTMT->AddRef(); 
		FreeMem(); 
		myRow = -1; 
		return *this;
	}
	CDbConn *GetDbConn() {
		return myDbConn;
	}
};

class CDbConn
{
public:
	CDbHandle     *myHDBC;
	CDbLib        *myDbLib;

   ~CDbConn();

    CDbConn(CDbLib *lib, const SQLHDBC hdbc)  { myDbLib = lib; myHDBC = hdbc ? new CDbHandle(hdbc, SQL_HANDLE_DBC) : 0; }
    CDbConn(const CDbConn &dbc)        { myHDBC = NULL; operator = (dbc); }
    CDbConn()                    { myDbLib = NULL; myHDBC = NULL; }

	void Close()			     {
		if (myHDBC) {
			myHDBC->Close(); 
			myHDBC = NULL; 
		}
	}

	CDbStmt Execute(const char *sql);
	
	CStr    GetErrorMsg()			 
		{ return myHDBC->GetErrorMsg(); }
	bool SetError(int nResult, bool bFree = true)
		{ return myHDBC->SetError(nResult, bFree); }

	operator bool () 
		{ return myHDBC ? (*myHDBC) != 0 : false; }
	CDbConn &operator = (const CDbConn &dbc) { 
		if (myHDBC != dbc.myHDBC) {
			myDbLib = dbc.myDbLib;
			if (myHDBC) 
				myHDBC->Free(); 
			myHDBC = dbc.myHDBC; 
			if (myHDBC) 
				myHDBC->AddRef(); 
		}
		return *this;
	}
	void BeginTrans() {
		//SQLRETURN nResult =
    SQLSetConnectAttr(*myHDBC, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER)SQL_AUTOCOMMIT_OFF, SQL_IS_UINTEGER);
	}
	void EndTrans(short completionType) {
		//SQLRETURN nResult =
    SQLEndTran(SQL_HANDLE_DBC, *myHDBC, completionType);
		SQLSetConnectAttr(*myHDBC, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER)SQL_AUTOCOMMIT_ON, SQL_IS_UINTEGER);
	}
	CDbLib *GetDbLib() {
		return myDbLib;
	}
	operator SQLHANDLE () 
		{return *myHDBC;}
};

class CDbPool : public qObjTS
{
	CDbLib			  *myLib;

	CMapStr<CDbConn>  myConnMap;
	CLst<CDbStmt>     *myTransSt;

public:

	CDbPool(CDbLib *lib) {
		myTransSt = NULL;
		myLib = lib;
	}

	~CDbPool() {
		EndTrans(SQL_ROLLBACK);
	}

	void EndTrans(short completionType) {
		if (myConnMap.Count()) {
			MAPPOS pos; CDbConn conn; const char *dsn;
			for ( pos = myConnMap.First(); myConnMap.Next(&pos, &dsn, &conn); ) {
				conn.EndTrans(completionType);
			}
			while (myTransSt)
				myTransSt = myTransSt->DeleteTop();
		}
	}

	CDbConn Connect(char *dsn);

	void AddStmt(const CDbStmt stmt) {
		if (stmt.myHSTMT && *stmt.myHSTMT) {
			myTransSt = myTransSt->Link(stmt);
		}
	}
};

class CDbLib : public qObjTS
{
	SQLHENV        myHENV;	// Environment Handle
	CCrit          myCrit;
	int			   myMaxBuffer;
	SQLULEN    myTimeout;

public:

    CDbLib();

   ~CDbLib();

	CDbConn Connect(char *dsn);

	void SetMaxBuffer(int n) {myMaxBuffer = n;}
	int GetMaxBuffer() {return myMaxBuffer;}

	void SetTimeout(SQLULEN timeout) {myTimeout = timeout;}
	SQLULEN GetTimeout() {return myTimeout;}

	operator SQLHANDLE () 
		{return myHENV;}
};

void _itoa2(int i, char *a)
{
	*a++ = '0' + (((int)(i/10)) % 10);
	*a++ = '0' + (i % 10);
}

void _itoa4(int i, char *a)
{
	*a++ = '0' + (((int)(i/1000)) % 10);
	*a++ = '0' + (((int)(i/100)) % 10);
	*a++ = '0' + (((int)(i/10)) % 10);
	*a++ = '0' + (i % 10);
}

#define QSQL_BASE          (SQL_NULL_DATA - 100)
#define QSQL_CONVERTED     (QSQL_BASE - 1)
#define QSQL_TRIMMED	   (QSQL_BASE - 2)
#define QSQL_IND(C)		   (((C) == QSQL_CONVERTED) || ((C) == QSQL_TRIMMED))
#define SQL_HAS_DATA(C)    (((C)>0)||QSQL_IND(C))

class CDbCol
{
	char	    *myBuf;
	SQLLEN  *myIndPtr;
	CDbStmt     *myStmt;

public:

	CStr  Name;

	SQLSMALLINT Type;
	SQLULEN Size;
	SQLLEN  DispSize;
	SQLSMALLINT Digits;
	SQLSMALLINT Null;

	void SetStmt(CDbStmt *stmt)     {myStmt = stmt;}
	void SetBuf(void *mem)			{myBuf = (char *) mem;}
	void SetIndPtr(void *mem)		{myIndPtr = (SQLLEN *) mem;}

	char *GetBuf() {
		return myBuf + (DispSize+1) * myStmt->BufIndex();
	}

	inline SQLLEN &GetInd() {
		return myIndPtr[myStmt->BufIndex()];
	}

	char *ConvBuf() {
		char *buf = GetBuf();

		if (GetInd() == SQL_NULL_DATA) {
			*buf = '\0';
		} else if (Type == SQL_C_TYPE_TIMESTAMP && GetInd() != QSQL_CONVERTED) {
			char *p = buf + DispSize;



			char *m = p;
			--p;
			while (*p == '0') {
				--p;
				if (*p == '.')
					m = p--;
				else if (*p == ':') {
					m = p + 3;
					break;
				}
			}
			if (*p == ' ')
				m = p;
			*m = '\0';
			GetInd() = QSQL_CONVERTED;
		}

		return buf;
	}

	char *RTrim();

	void Free() {Name.Free();}
};

inline char *CDbCol::RTrim()
{
	char *p = ConvBuf();

	if (GetInd() > 0) {
		int n  = min(GetInd(),DispSize);
		if (n > 0) {
			char *l = p + n - 1;
			if (isspace(*l)) {
				do {
					--l;
				} while (l >= p && isspace(*l));
			}
			*++l = '\0';
		}  else
			p[DispSize]='\0';

		GetInd() = QSQL_TRIMMED;
	} else
		p[DispSize]='\0';

	return p;
}

CDbLib::CDbLib()
{
	myHENV = 0;
	
	SetTimeout(DEFAULT_TIMEOUT);

	SQLSetEnvAttr(0, SQL_ATTR_CONNECTION_POOLING, (SQLPOINTER) SQL_CP_ONE_PER_HENV, 0);

	if (SQLAllocHandle(SQL_HANDLE_ENV,SQL_NULL_HANDLE,&myHENV) != SQL_SUCCESS) {
	  myHENV = 0;
	} else {
		if (SQLSetEnvAttr(myHENV, SQL_ATTR_ODBC_VERSION,
			(SQLPOINTER) SQL_OV_ODBC3, SQL_IS_INTEGER) != SQL_SUCCESS) {
			SQLFreeHandle(SQL_HANDLE_ENV, myHENV);
			myHENV = 0;
	   }
	}

	myMaxBuffer = DEFAULT_MAX_BUFFER;
}

CDbLib::~CDbLib()
{
	if (myHENV) {
		try {
			SQLFreeHandle(SQL_HANDLE_ENV, myHENV);
		} catch (...) {
			smx_log_str(1, "error freeing dblib");
		}
	}
}

typedef SQLCHAR FAR * LPSQLC;

CDbConn CDbLib::Connect(char *dsn)
{
	char *uid, *pwd, *etc;

	ParseDSN(dsn, dsn, uid, pwd, etc);

	qObjTSRef ts = GetRef();
	if (myHENV) {
		TRACE("sql: spawning connection\n");
		
		SQLHDBC hdbc;
		SQLRETURN nResult;

		if ((nResult = SQLAllocHandle(SQL_HANDLE_DBC, myHENV, &hdbc) != SQL_SUCCESS)) {
			return CDbConn(this, (SQLHDBC)NULL);
		}

		if (etc && *etc) {
			CStr connIn; 
			connIn = CStr("DSN=")<<dsn<<";UID="<<uid<<";PWD="<<pwd<<';'<<etc;
			CStr connOut(1024);
			short totLen;
			nResult = SQLDriverConnect(hdbc, NULL, (LPSQLC) connIn.SafeP(), connIn.Length(), (LPSQLC) connOut.SafeP(), connOut.Length(), &totLen, SQL_DRIVER_NOPROMPT);
		} else {
			nResult = SQLConnect(hdbc, (LPSQLC) dsn, SQL_NTS, (LPSQLC) uid, SQL_NTS, (LPSQLC) pwd, SQL_NTS);
		}

		// if failed to connect, free the allocated hdbc before return

		if (!SQL_SUCCEEDED(nResult)) {
			CDbConn conn(this, hdbc);
			conn.SetError(nResult);
			return conn;
		}

		return CDbConn(this, hdbc);
	} else {
		return CDbConn(this, (SQLHDBC)NULL);
	}
}

CDbConn::~CDbConn()
{
	if (myHDBC) {
		try {
			myHDBC->Free(); 
			myHDBC = NULL;
		} catch (...) {
			smx_log_str(1, "error freeing dbconn");
		}
	}
}


void CDbStmt::FreeMem()
{
	if (myCols) {
		if (myRow != -1) { // unbound
			int i;
			for (i = 0; i < myColN; ++i) {
				myCols[i].Free();
			}
			myRow = -1;
		}
		delete[] myCols;
		myCols = 0;
	} else
		myRow = -1;

	if (myMem) {
		free(myMem);
		myMem = 0;
	}
}

bool CDbStmt::Bind()
{
	assert (myRow == -1) ;

	// call SQLNumResultCols to calculate the number of columns
	SQLSMALLINT nCols;
	SQLRETURN nResult;

	if (!myHSTMT)
		return false;


	do {
		nResult = SQLNumResultCols(*myHSTMT, &nCols);
		if (nResult != SQL_ERROR && nCols == 0) {
			SQLRETURN nResultMore = SQLMoreResults(*myHSTMT);
			if (!SQL_SUCCEEDED(nResultMore))
				break;
		} else
			break;
	} while (nResult != SQL_ERROR && nCols == 0);

	if (nResult == SQL_ERROR || nCols == 0) {
	  // Close the open result set.
		myHSTMT->Close();
		myHSTMT = NULL;
		return false;
	} else {
		myCols = new CDbCol[(myColN = nCols)+1];
	}


	CDbLib *dbLib = GetDbConn()->GetDbLib();
	SQLSMALLINT nColName;
	SQLSMALLINT i;
	int nAlloc;

	myColMap.Clear();

	nAlloc = 0;
	for(i = 0; myCols && i < myColN; i++) {
		myCols[i].Name.Grow(3);
		SQLDescribeCol(*myHSTMT, i+1, (LPSQLC) (const char *) myCols[i].Name, 2, 
					&nColName, &myCols[i].Type, &myCols[i].Size,
					&myCols[i].Digits, &myCols[i].Null);
		
		if (nColName) {
			myCols[i].Name.Grow(nColName);

			SQLDescribeCol(*myHSTMT, i+1, (LPSQLC) (const char *) myCols[i].Name, nColName+1, &nColName, 
						&myCols[i].Type, &myCols[i].Size,
						&myCols[i].Digits, &myCols[i].Null);
		} else {
			SQLColAttribute(*myHSTMT, i+1, SQL_DESC_CATALOG_NAME, 
				0, 0, &nColName, 0);
			if (nColName) {
				myCols[i].Name.Grow(nColName);
				SQLColAttribute(*myHSTMT, i+1, SQL_DESC_CATALOG_NAME, 
					(LPSQLC) (const char *) myCols[i].Name, nColName+1, &nColName, 0);
			} else  {
				myCols[i].Name.Grow(1);
				*(myCols[i].Name.GetBuffer()) = '\0';
			}
		}

	
		SQLColAttribute(*myHSTMT, i+1, SQL_DESC_DISPLAY_SIZE, 0, 0, 0, 
					&myCols[i].DispSize);

		smx_log(SMXLOGLEVEL_DEBUG, "allocate size %d for %s", myCols[i].DispSize, myCols[i].Name.Data());

		myColMap.Set(myCols[i].Name,&myCols[i]);

		if (myCols[i].DispSize > DEFAULT_MAX_BUFFER) {
			myCols[i].DispSize = DEFAULT_MAX_BUFFER;
		}

		nAlloc += (myCols[i].DispSize + 1);
	}

	SQLLEN rowSetSize = max((long unsigned int) 1, (dbLib->GetMaxBuffer() / (nAlloc + (myColN * sizeof(SQLLEN)))));

	nResult = SQLSetStmtAttr(*myHSTMT, SQL_ATTR_ROW_ARRAY_SIZE, (void *) rowSetSize,   SQL_IS_INTEGER);
	nResult = SQLGetStmtAttr(*myHSTMT, SQL_ATTR_ROW_ARRAY_SIZE,   &rowSetSize,   SQL_IS_INTEGER,  NULL);
	nResult = SQLSetStmtAttr(*myHSTMT, SQL_ATTR_ROWS_FETCHED_PTR, &myRowsetSize, SQL_IS_POINTER);
	

	myRowsetMax = myRowsetSize = rowSetSize;
	myRow = 0;
	myBufIndex = 0;

	myMem = (char *) malloc(nAlloc * (myRowsetSize+1) + sizeof(SQLLEN) * (myColN+1) * (myRowsetSize+1));
	
	char *pB = myMem;
	SQLLEN *pI = (SQLLEN *) (myMem + nAlloc * (myRowsetSize+1));
	for(i = 0; i < myColN; i++) {
		myCols[i].SetBuf(pB);
		myCols[i].SetIndPtr(pI);
		myCols[i].SetStmt(this);

		SQLBindCol(*myHSTMT, (UWORD)(i+1), SQL_C_CHAR, myCols[i].GetBuf(), myCols[i].DispSize + 1, &myCols[i].GetInd());

		pB += (myCols[i].DispSize + 1) * myRowsetSize;
		pI += myRowsetSize;
	}
	myBufIndex = myRowsetSize;
	return true;
}

CDbStmt::~CDbStmt() 
{
	if (myHSTMT) {
		myHSTMT->Free();
		myHSTMT = 0;
	}

	FreeMem();
}

		
CDbStmt CDbConn::Execute(const char *sql) 
{
	SQLHSTMT hstmt = (SQLHSTMT) NULL;
	SQLRETURN nResult;

	try {
		if ((nResult = SQLAllocHandle(SQL_HANDLE_STMT, *myHDBC, &hstmt)) != SQL_SUCCESS) {
			return CDbStmt(this, (SQLHSTMT)NULL);
		}
	} catch (...) {
		return CDbStmt(this, (SQLHSTMT)NULL);
	}

	if (!sql)
		return CDbStmt(this, (SQLHSTMT)NULL);

	while (isspace(*sql))
		++sql;

	bool doStatic = false, doDynamic = false;
	
	if (!strncmp(sql, "static", 6) && isspace(sql[6]))	 {
		sql += 6;
		doStatic = true;
	} else if (!strncmp(sql, "dynamic", 7) && isspace(sql[7]))	 {
		sql += 7;
		doDynamic = true;
	}

	SQLSetStmtAttr(hstmt, SQL_ATTR_QUERY_TIMEOUT, (SQLPOINTER) myDbLib->GetTimeout(), SQL_IS_UINTEGER);

	if (doStatic) {
		SQLSetStmtAttr(hstmt, SQL_ATTR_CURSOR_SCROLLABLE, (void *) SQL_NONSCROLLABLE, SQL_IS_UINTEGER);
		SQLSetStmtAttr(hstmt, SQL_ATTR_CURSOR_SENSITIVITY, (void *) SQL_INSENSITIVE, SQL_IS_UINTEGER);
	}

	if (doDynamic) {
		SQLSetStmtAttr(hstmt, SQL_ATTR_CURSOR_SCROLLABLE, (void *) SQL_SCROLLABLE, SQL_IS_UINTEGER);
		SQLSetStmtAttr(hstmt, SQL_ATTR_CURSOR_SENSITIVITY, (void *) SQL_SENSITIVE, SQL_IS_UINTEGER);
	}

	nResult = SQLExecDirect(hstmt, (LPSQLC) sql, SQL_NTS);

	CDbStmt stmt(this, hstmt);

	if (!SQL_SUCCEEDED(nResult) && !(nResult == SQL_NO_DATA)) {
		stmt.SetError(nResult, true);
	}

	return stmt;
}

bool CDbStmt::Next()
{
	SQLRETURN nReturn;

	if (myDone)
		return false;

	if (myRow == -1) {
		Bind();
		if (myRow == -1) {
			myDone = true;
			if (myHSTMT && *myHSTMT)
				SQLMoreResults(*myHSTMT);
			return false;
		}
	}
	if (myBufIndex < (myRowsetSize-1)) {
		++myBufIndex;
		++myRow;
		return true;
	} else {
		if (myHSTMT) {
			smx_log(SMXLOGLEVEL_DEBUG, "fetch again with same bound memory");
			nReturn = SQLFetch(*myHSTMT);
			if (SQL_SUCCEEDED(nReturn)) {
				++myRow;
				myHasData = true;
				myBufIndex = 0;
				return true;
			} else {
				nReturn = SQLMoreResults(*myHSTMT);
				if (SQL_SUCCEEDED(nReturn)) {
					smx_log(SMXLOGLEVEL_DEBUG, "reallocate for more results");
					FreeMem();
					myRow = -1;
					return Next();
				} else
					myDone = true;
			}
		}
		return false;
	}
}

bool CDbStmt::Skip(int n)
{
	SQLRETURN nReturn;
	if (n < 0) 
		return false;

	if (n == 0) 
		return true;

	if (myDone)
		return false;

	if (myRow == -1) {
		Bind();
		if (myRow == -1) {
			myDone = true;

			return false;
		}
	}
	if (myBufIndex < (myRowsetSize-n)) {
		myBufIndex+=n;
		myRow+=n;
		return true;
	} else {
		if (myHSTMT) {
			nReturn = SQLFetchScroll(*myHSTMT,SQL_FETCH_RELATIVE,n-myRowsetSize+myBufIndex);
			if (SQL_SUCCEEDED(nReturn)) {
				myBufIndex = 0;
				myRow+=n;
				return true;
			} else {
				SetError(nReturn, false);
				if (memcmp(myHSTMT->GetErrorState().Data(),"HY106",5)) {
					myDone = true;
				} else {
					while (n-- > 0 && Next()) {
						;
					}
					return true;
				}

			}
		}
		return false;
	}
}

CDbCol *CDbStmt::Column(int index)
{
	if (index < myColN && index >= 0) {
		return &myCols[index];
	} else {
		smx_log(SMXLOGLEVEL_DEBUG, "index %d is >= %d", index, myColN);
	}
	return 0;
}

CDbCol *CDbStmt::Column(const char * index)
{
	CDbCol *col;
	if (myColMap.Find(index, col)) {
		return col;
	}
	return NULL;
}

CDbLib *myDBL;

class qObjODBC : public qObjSql {
	CDbStmt myStmt;
	CDbConn myConn;
	CDbCol *GetEvalCol(qCtx *ctx, qArgAry *args);
public:
	qObjODBC(char *dsn, qCtx *ctx, const void *data);
	void Open(char *dsn, qCtx *ctx, const void *data);
	void Execute(qCtx *ctx, qStr *out, const char *sql, CStr &body, CStr &head, CStr &foot);
	void EvalColName(qCtx *ctx, qStr *out, qArgAry *args);
	void EvalColType(qCtx *ctx, qStr *out, qArgAry *args);
	void EvalRow(qCtx *ctx, qStr *out, qArgAry *args);
	void EvalSkipRows(qCtx *ctx, qStr *out, qArgAry *args);
	void EvalCol(qCtx *ctx, qStr *out, qArgAry *args) ;
	void EvalColumn(qCtx *ctx, qStr *out, qArgAry *args) ;
	void EvalEnumCol(qCtx *ctx, qStr *out, qArgAry *args) ;
};

void qObjODBC::Execute(qCtx *ctx, qStr *out, const char *sql, CStr &body, CStr &head, CStr &foot)
{
	if (!myConn) {
		ctx->Throw(out, 302, myConn.GetErrorMsg());
		return;
	}

	myStmt = myConn.Execute(sql);

        if (!myStmt) {
                ctx->Throw(out, 302, myStmt.GetErrorMsg());
                return;
        }

	if ( ! ( body.IsEmpty() && head.IsEmpty() && foot.IsEmpty() ) ) {
		if (myStmt.Bind()) {
			bool ok = myStmt.Next();
			ctx->MapObj(&ok, (QOBJFUNC) EvalBreak, "break");
			if (!head.IsEmpty()) ctx->Parse(head, out);
			ok = ok && !myStmt.Done();
                        while (ok) {
                        	ctx->Parse(body, out);
				if (ok)
                                	ok = myStmt.Next();
                        }
			if (!foot.IsEmpty()) ctx->Parse(foot, out);
		}
	}
}

qObjODBC::qObjODBC(char *dsn, qCtx *ctx, const void *data)
{
	Open(dsn,ctx,data);
}

void qObjODBC::Open(char *dsn, qCtx *ctx, const void *data)
{
	myConn = data ? ((CDbPool*)data)->Connect(dsn) : GetDbLib(ctx)->Connect(dsn);
}

CDbCol *qObjODBC::GetEvalCol(qCtx *ctx, qArgAry *args)
{
	CDbCol *col = 0;

	if (args->Count() > 0) {
	CStr index = (*args)[0];
	index.Trim();
	if (!index.IsEmpty()) {
		if (isdigit(index[0])) {
			col = myStmt.Column(atoi(index)-1);
		} else {
			strlwr(index.GetBuffer());
			col = myStmt.Column((const char *)index);
		}
	}
        }
	return col;
}

void qObjODBC::EvalSkipRows(qCtx *ctx, qStr *out, qArgAry *args)
{
	int val = ParseInt((*args)[0]);
	myStmt.Skip(val);
}

void qObjODBC::EvalRow(qCtx *ctx, qStr *out, qArgAry *args)
{
	assert(myStmt.RowCount()!=-1);
	if (myStmt.RowCount() == -1)
		return;
	out->PutN(myStmt.RowCount());
}

void qObjODBC::EvalCol(qCtx *ctx, qStr *out, qArgAry *args) 
{
	assert(myStmt.RowCount()!=-1);
	if (myStmt.RowCount() == -1)
		return;
	CDbCol *col;
	if ((col = GetEvalCol(ctx, args))) {
		int ind = col->GetInd();
		// smx_log(SMXLOGLEVEL_DEBUG, "ind for %s is %d", (const char *) col->Name, ind);
		if (myStmt.HasData() && SQL_HAS_DATA(ind)) {
			col->ConvBuf();
			col->RTrim();
			if (col->GetBuf() && *col->GetBuf())
				out->PutS(col->GetBuf());
		}
	}
}

void qObjODBC::EvalColumn(qCtx *ctx, qStr *out, qArgAry *args) 
{
	assert(myStmt.RowCount()!=-1);
	if (myStmt.RowCount() == -1)
		return;
	CDbCol *col;
	if ((col = GetEvalCol(ctx, args))) {
		int ind = col->GetInd();
		if (myStmt.HasData() && SQL_HAS_DATA(ind)) {
			col->ConvBuf();
			if (ind >= 0) {
				int n = min((SQLLEN)ind, col->DispSize);
				out->PutS(col->GetBuf(), n);
			} else {
				out->PutS(col->GetBuf());
			}
		}
	}
}

void qObjODBC::EvalEnumCol(qCtx *ctx, qStr *out, qArgAry *args) 
{
	assert(myStmt.RowCount()!=-1);
	if (myStmt.RowCount() == -1)
		return;
	if (args->Count() == 0)
		return;

	int   num, type, size;

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

	CDbCol *col;
	for(num = 1; ok && num <= myStmt.ColCount(); ++num) {
		col = myStmt.Column(num-1);
		name = col->Name;
		if (myStmt.HasData() && SQL_HAS_DATA(col->GetInd())) {
			col->ConvBuf();
			col->RTrim();
			value = col->GetBuf();
		} else {
			value = 0;
		}
		type = col->Type;
		size = col->Size;
		sub.qCtx::Parse(args->GetAt(0), out);
	}
}

void qObjODBC::EvalColName(qCtx *ctx, qStr *out, qArgAry *args) 
{
	assert(myStmt.RowCount()!=-1);
	if (myStmt.RowCount() == -1)
		return;
	CDbCol *col;
	if ((col = GetEvalCol(ctx, args))) {
		out->PutS(col->Name);
	}
}

void qObjODBC::EvalColType(qCtx *ctx, qStr *out, qArgAry *args) 
{
	assert(myStmt.RowCount()!=-1);
	if (myStmt.RowCount() == -1)
		return;

	CDbCol *col;
	if ((col = GetEvalCol(ctx, args))) {
		out->PutN(col->Type);
	}
}


void EvalSqlQ(const void *data, qCtx *ctx, qStr *out, qArgAry *args)
{
	CStr val = (*args)[0];
	char *p = val.GetBuffer();
	CStr quo(val.Length() * 2 + 2);
	char *v = quo.GetBuffer();
	*v++ = '\'';
	if (p) {
		while (*p) {
			if (*p == '\'')
				*v++ = '\'';
			*v++ = *p;
			++p;
		}
	}
	*v++ = '\'';
	quo.Grow(v-(const char *)quo);
	out->PutS(quo);
}

void ParseDSN(char *cstr, char *&dsn, char *&uid, char *&pwd, char *&etc)
{
	dsn = cstr;
	while (isspace(*dsn)) ++dsn;

	uid = strchr(dsn,';');
	pwd = 0;
	etc = 0;
	if (uid) {
		*uid++ = '\0';
		while (isspace(*uid)) ++uid;
		pwd = strchr(uid,';');
		if (pwd) {
			*pwd++ = 0; 
			while (isspace(*pwd)) ++pwd;
			etc = strchr(pwd,';');
			while (etc && etc[1] == ';') {
				char *p = etc;
				while (p[1]) {
					p[0] = p[1];
					++p;
				}
				etc = strchr(etc+1, ';');
			}
			if (etc) {
				*etc++ = 0;
				while (isspace(*etc)) ++etc;
			}
		}
	}
}

void EvalSqlTrans(const void *data, qCtx *ctx, qStr *out, qArgAry *args)
{
	int errCode = 0; CStr errMsg;

	VALID_ARGC("sql-trans", 1,1);


	qObjTSRef ts = myDBL->GetRef();

	try {
		CDbPool *dbPool = new CDbPool(myDBL);
		CDbLib  *dbLib = GetDbLib(ctx);

		qObjTSRef ts2 = dbLib->GetRef();

		qCtxTmp tmpCtx(ctx);
		bool ok = true;

		tmpCtx.AddTry();
		tmpCtx.MapObj(dbPool, EvalSql, "sql", "00111");
		tmpCtx.MapObj(&ok , (QOBJFUNC) EvalBreak, "rollback");

		try {
			tmpCtx.Parse(args->GetAt(0), out);
		} catch(qCtxEx ex) {
			ok = false;
			errCode = ex.GetID();
			errMsg = ex.GetMsg();
		} catch (qCtxExAbort ex) {
			throw ex;
		} catch(...) {
			ok = false;
			errCode = 999;
			errMsg = "SqlTrans Unhandled exception.";
		}

		dbPool->EndTrans(ok ? SQL_COMMIT : SQL_ROLLBACK);
		dbPool->Free();

		tmpCtx.RemTry();
	} catch (qCtxExAbort ex) {
		throw ex;
	} catch(...) {
		ctx->Throw(out, 303, "Unknown SQL Library error during transaction.\nTransaction was rolled back");
	}
	if (errCode > 0)
		ctx->Throw(out, errCode, errMsg << "\nTransaction was rolled back.");
}

void EvalSql(const void *data, qCtx *ctx, qStr *out, qArgAry *args)
{
	int errCode = 0; CStr errMsg;
	
	VALID_ARGC("sql", 2 ,5);

	CDbLib *dbLib = GetDbLib(ctx);

	CDbConn conn;
	CStr dsn = (*args)[0];
	dsn.Trim();

	qObjSql *handler;

	CStr sql = (*args)[1];
	sql.Trim();

	if (dsn.IsEmpty()) {
		ctx->Throw(out, 358, "No datasource specified");
		return;
	}

	try {
	    qObjTSRef ts;
#ifdef HAVE_SQLITE3_H
            if(!strnicmp(dsn,"sqlite:",7)) {
		const char *p = dsn+7;
		while (isspace(*p)) ++p;
		handler = new qObjSqlite(p, ctx);
            } else 
#endif // HAVE_SQLITE3_H	
	    {
		ts = dbLib->GetRef();
		handler = new qObjODBC(dsn, ctx, data);
	    }

	    qCtxTmp loopCtx(ctx);
            loopCtx.MapObj(handler, (QOBJMETH) &qObjSql::EvalColName, "column-name");
            loopCtx.MapObj(handler, (QOBJMETH) &qObjSql::EvalColName, "col-name");
            loopCtx.MapObj(handler, (QOBJMETH) &qObjSql::EvalColType, "column-type");
            loopCtx.MapObj(handler, (QOBJMETH) &qObjSql::EvalColType, "col-type");
            loopCtx.MapObj(handler, (QOBJMETH) &qObjSql::EvalEnumCol, "enumcolumn","1");
            loopCtx.MapObj(handler, (QOBJMETH) &qObjSql::EvalEnumCol, "enumcol","1");
            loopCtx.MapObj(handler, (QOBJMETH) &qObjSql::EvalColumn,  "column");
            loopCtx.MapObj(handler, (QOBJMETH) &qObjSql::EvalCol,     "col");
            loopCtx.MapObj(handler, (QOBJMETH) &qObjSql::EvalRow,     "num-rows");
            loopCtx.MapObj(handler, (QOBJMETH) &qObjSql::EvalSkipRows,"skip-rows");

            CStr sql = (*args)[1];
            sql.Trim();

	    ctx->MapObj(sql, "sql-lastquery");

            handler->Execute(&loopCtx, out, sql, args->GetAt(2), args->GetAt(3), args->GetAt(4));
	    delete handler;
	} catch(qCtxEx ex) {
		errCode = ex.GetID();
		errMsg = ex.GetMsg();
	    	delete handler;
	} catch (qCtxExAbort ex) {
		throw ex;
	    	delete handler;
	} catch (...) {
		errCode = 303;
		errMsg  = "Unknown SQL Library error.";
	    	delete handler;
	}

	if (errCode)
		ctx->Throw(out, errCode, errMsg);
}


void EvalSqlMax(const void *data, qCtx *ctx, qStr *out, qArgAry *args)
{

	CDbLib *dbLib = GetDbLib(ctx);
	if (args->GetAt(0)) {
		int val = ParseInt((*args)[0]);
		if (val < 4192)
			val = 4192;
		dbLib->SetMaxBuffer(val);
	} else {
		out->PutN(dbLib->GetMaxBuffer());
	}
}

void EvalSqlTimeout(const void *data, qCtx *ctx, qStr *out, qArgAry *args)
{
	CDbLib *dbLib = GetDbLib(ctx);
	if (args->GetAt(0)) {
		int val = ParseInt((*args)[0]);
		dbLib->SetTimeout(val);
	} else {
		out->PutN((int) dbLib->GetTimeout());
	}
}

CDbLib *GetDbLib(qCtx *ctx)
{
	CDbLib *dbLib;
	if (!ctx->Find((qObj **)&dbLib, "<dblib>"))
		return myDBL;
	else 
		return dbLib;
}

void LoadSql(qCtx *ctx) {
//sql
	myDBL = new CDbLib();
	ctx->MapObj(myDBL,     "<dblib>");

	ctx->MapObj((void *) false, EvalSql,       "sql", "00111");
	ctx->MapObj(EvalSqlMax,    "sql-maxbuf");
	ctx->MapObj(EvalSqlTimeout,"sql-timeout");
	ctx->MapObj(EvalSqlGrp,    "sqlg", "001111111111111");
	ctx->MapObj(EvalSqlQ,      "sqlq");
	ctx->MapObj(EvalSqlQ,      "sql-quote");
	ctx->MapObj(EvalSqlTrans,  "sql-trans", "1");
}


CDbConn CDbPool::Connect(char *dsn) 
{
	CDbConn conn;
	if (!myConnMap.Find(dsn,conn)) {
		CStr tmp = dsn;
		conn = myLib->Connect(tmp.GetBuffer());
		conn.BeginTrans();
		myConnMap.Set(dsn,conn);
	}
	return conn;
}
