#ifndef _QCORE_
#define _QCORE_

/////// %hset/%hget object, uses a threadsafe (TS) context

class qObjHCtx : public qObj 
{
	qCtxTS myCtx;
	CStr   myStr;
public:
// construct / destruct
	~qObjHCtx() {};
	qObjHCtx() : qObj() {}
	qObjHCtx(CStr &str) : qObj() {myStr = str;}

// access
	qCtx *GetCtx() {return &myCtx;}
	CStr &GetStr() {return  myStr;}

// helpers - used internally and also by %pset, %fset  & other hierarchical storage macros
	bool HGet(char *path, CStr &val);
	bool HDel(CStr &path);
	qObjHCtx *HSet(char *path, CStr &val);
	qObjHCtx *HFind(char *path);

// override
	void Eval(qCtx *ctx, qStr *out, qArgAry *args);

// mapped macros
	int HGet(qCtx *ctx, qStr *out, qArgAry *args);
	int HSet(qCtx *ctx, qStr *out, qArgAry *args);
	int HOpen(qCtx *ctx, qStr* out, qArgAry *args);
	int HEnum(qCtx *ctx, qStr *out, qArgAry *args);
};

void EvalHEnumValues(const void *data, qCtx *ctx, qStr *out, qArgAry *args);
void EvalHEnumKeys(const void *data, qCtx *ctx, qStr *out, qArgAry *args);

#endif //#ifndef _QCORE_
