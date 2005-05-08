/* COPYRIGHT 1998 Prime Data Corp. 
	All Rights Reserved.  Use of this file without prior written 
	authorization is strictly prohibited.
*/

#ifndef _QOBJ_CACHE_
#define _QOBJ_CACHE_

#include "qobj-ctx.h"

#include "pstime.h"

#define MIN_CACHE_SIZE 10000

#ifdef _DEBUG
#	define MAX_CACHE_SIZE MIN_CACHE_SIZE
#else
#	define MAX_CACHE_SIZE 10000000
#endif

class qObjTimed;
class qObjCache;

// cache of timed objects
class qObjCache : public qObjTS {
protected:
	qCtxTS    *myCtx;

protected:
	qObjTimed *MapObj(qObj *obj, const char *name, qStr *fp, int size);

public:
	~qObjCache();
	
	qObjCache(qCtx *ctx) {
		myCtx = new qCtxTS();
	}

	qCtx *GetCtx() {
		return myCtx;
	}

// opens file AND returns cached object, but does nothing with file
	qStr *OpenFile(qCtx *ctx, qObjTimed **found, CStr &path);
	qStr *OpenFileFQ(qCtx *ctx, qObjTimed **found, const char *path);

	void EvalModule(qCtx *ctx, qStr *out, qArgAry *args);
#ifdef WIN32
	void EvalActiveX(qCtx *ctx, qStr *out, qArgAry *args);
#endif
};

// timed objects stored in a cache
class qObjTimed : public qObjTS
{
	qTime myTime;
	qObj *myObj;
	qObjTimed *myNext;
	qObjTimed *myPrev;
	qObjCache *myCache;
	CStr myName;

public:
	qObjTimed(qObj *obj, qObjCache *cache, CStr name, int size) {
		myTime = time(NULL); myObj = obj;
		myName = name;
		myCache = cache;
	}

	~qObjTimed() {
		if (myObj)
			myObj->Free();
	}

	CStr &GetName() {
		return myName;
	}
	time_t GetTime() {
		return myTime;
	}
	time_t GetTimeGMT() {
		return myTime + qTime::GetZoneOffset();
	}
	void SetTime(time_t t) {
		myTime = t;
	}
	qObj *GetObj() 	{
		return myObj;
	}
	void Eval(qCtx *ctx, qStr *out, qArgAry *args) 	{
		if (myObj)
			myObj->Eval(ctx, out, args);
	}
};

class qStrCache : public qObjCache {
public:
	qStrCache(qCtx *ctx) : qObjCache(ctx) {}

// should return an object that can be read from (possibly a blocking thing)
	CStr OpenFile(qCtx *ctx, CStr &path);

	CStr OpenFileFQ(qCtx *ctx, const char *path);

//	void EvalInclude(qCtx *ctx, qStr *out, qArgAry *args);
};



#endif //#ifndef _QOBJ_CACHE_
