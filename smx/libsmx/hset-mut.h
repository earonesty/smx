#ifndef _QHSET_
#define _QHSET_

/////// %hset/%hget object, uses a threadsafe (TS) context

class qObjHCtx : public qObj 
{
protected:
	qCtxTS myCtx;
	CStr   myStr;
	qObjHCtx * HSet(char *path, CStr &val, CMutexLock &lock);

public:
// construct / destruct
	~qObjHCtx() {
		CMutexLock lock = myCtx.Enter(); 
		myCtx.Clear(); 
		myStr.Free();
	}

	qObjHCtx() : qObj() {}
	qObjHCtx(CStr &str) : qObj() {myStr = str;}

// access
	qCtx *GetCtx() {CMutexLock lock = myCtx.Enter(); return &myCtx;}
	CStr  GetStr() {CMutexLock lock = myCtx.Enter(); return  myStr;}

// helpers - used internally and also by %pset, %fset  & other hierarchical storage macros
	bool HGet(char *path, CStr &val);
	bool HExists(char *path);
	bool HDel(CStr &path);
	qObjHCtx *HSet(char *path, CStr &val);

	qObjHCtx *HFind(char *path, CMutexLock &lock);

// override
	void Eval(qCtx *ctx, qStr *out, qArgAry *args);

// mapped macros
	int HGet(qCtx *ctx, qStr *out, qArgAry *args);
	int HExists(qCtx *ctx, qStr *out, qArgAry *args);
	int HSet(qCtx *ctx, qStr *out, qArgAry *args);
	int HOpen(qCtx *ctx, qStr* out, qArgAry *args);
	int HEnum(qCtx *ctx, qStr *out, qArgAry *args);
};

void EvalHEnumValues(const void *data, qCtx *ctx, qStr *out, qArgAry *args);
void EvalHEnumKeys(const void *data, qCtx *ctx, qStr *out, qArgAry *args);

#endif //#ifndef _QHSET_
