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
	CStr myFoot;

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
