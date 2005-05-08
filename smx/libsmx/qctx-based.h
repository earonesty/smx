#ifndef _QCTX_BASED_
#define _QCTX_BASED_

#include "qctx.h"

// context "based on another" (inherits from another)

class qCtxBased : public qCtx 
{
	qCtx *myBase;
public:
	qCtxBased() : qCtx() {myBase = 0;}
	bool FindL(qObj **obj, const char *name) \
		{return ((*obj = myMap.Find(name)) != myMap.Nf) ? true : \
		(myBase ? myBase->FindL(obj, name) : false);}
	void SetBase(qCtx* base) {myBase = base;}
};

class qCtxBasedTS : public qCtxBased {
	CCrit myCrit;
public:
	virtual qObj *MapObj(qObj *obj, char *name) \
		{	CLock lock= myCrit.Enter(); \
			return qCtxBased::MapObj(obj, name);
		}

	virtual qObj *MapObjG(qObj *obj, char *name) \
		{	CLock lock= myCrit.Enter(); \
			return qCtxBased::MapObjG(obj, name);
		}

	virtual bool FindL(qObj **obj, const char *name) \
		{	CLock lock= myCrit.Enter(); \
			return qCtxBased::Find(obj, name);
		}
};

#endif //#ifndef _QCTX_BASED_
