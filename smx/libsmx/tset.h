/* COPYRIGHT 1998 Prime Data Corp. 
	All Rights Reserved.  Use of this file without prior written 
	authorization is strictly prohibited.
*/

#ifndef _QTSET_
#define _QTSET_

/////// %tset/%hget object, uses a threadsafe (TS) context

class qObjTCtx : public qObjTS 
{
protected:
	qCtxTS *myCtx;
	CStr    myStr;

	qObjTCtx * TSet(char *path, CStr &val, qObjTSRef &ref);

// construct / destruct
	~qObjTCtx() {
		if (myCtx)
			myCtx->Free(); 
		myStr.Free();
	}

public:
	qObjTCtx() {myCtx = new qCtxTS();}
	qObjTCtx(CStr &str) {myStr = str; myCtx=NULL;}

// access
	qCtx *GetCtx() {return myCtx;}
	CStr  GetStr() {return myStr;}
	void SetStr(CStr str) {myStr = str;}

// helpers - used internally and also by %pset, %fset  & other hierarchical storage macros
	bool TGet(const char *path, CStr &val);
	bool TExists(const char *path);
	bool TDel(CStr &path);
	qObjTCtx *TSet(char *path, CStr &val);

	qObjTCtx *TFind(const char *path, qObjTSRef &lock);

// override
	void Eval(qCtx *ctx, qStr *out, qArgAry *args);

// mapped macros (_ is to resolve gcc problem)
	void _TGet(qCtx *ctx, qStr *out, qArgAry *args);
	void _TExists(qCtx *ctx, qStr *out, qArgAry *args);
	void _TSet(qCtx *ctx, qStr *out, qArgAry *args);
	void TOpen(qCtx *ctx, qStr* out, qArgAry *args);

	int TEnum(qCtx *ctx, qStr *out, qArgAry *args);
};

void EvalTEnumValues(const void *data, qCtx *ctx, qStr *out, qArgAry *args);
void EvalTEnumKeys(const void *data, qCtx *ctx, qStr *out, qArgAry *args);

#endif //#ifndef _QTSET_
