/* COPYRIGHT 1998 Prime Data Corp. 
	All Rights Reserved.  Use of this file without prior written 
	authorization is strictly prohibited.
*/

#ifndef _QCTX_BASED_
#define _QCTX_BASED_

#include "qctx.h"

// context "based on another" (inherits from another)

class qCtxBased : public qCtxRef 
{
	qCtx *myBase;

public:

	qCtxBased() {
		myBase = NULL; 
	}

	~qCtxBased() {
	}

	virtual bool FindL(qObj **obj, const char *name) {
		if (myMap.Find(name, *obj) && *obj)
			return true;
		else if (myBase) {
			if (myBase->FindL(obj, name))
				return true;
			else if (myBase->GetParent())
				return myBase->GetParent()->FindL(obj, name);
			else
				return false;
		} else {
			*obj = 0;
			return false;
		}
	}

	virtual bool FindL(qObj **obj, const CStr &name) {
		if (myMap.Find(name, *obj) && *obj)
			return true;
		else if (myBase) {
			if (myBase->FindL(obj, name))
				return true;
			else if (myBase->GetParent())
				return myBase->GetParent()->FindL(obj, name);
			else
				return false;
		} else {
			*obj = 0;
			return false;
		}
	}

	void SetBase(qCtx* base) {
		myBase = base;
	}
};

/*
class qCtxBasedTS : public qCtxTS {
	qCtx *myBase;

	~qCtxBasedTS() {
		assert(myRefs == 0);
		if (myBase)
			myBase->Free(); 
	}

public:

	qCtxBasedTS() {
		myBase = 0; 
		myRefs = 1;
	}

	virtual bool FindL(qObj **obj, const char *name) {
		if (myMap.Find(name, *obj) && *obj)
			return true;
		else if (myBase)
			return myBase->FindL(obj, name);
		else {
			*obj = 0;
			return false;
		}
	}

	virtual bool FindL(qObj **obj, const CStr &name) {
		if (myMap.Find(name, *obj) && *obj)
			return true;
		else if (myBase)
			return myBase->FindL(obj, name);
		else {
			*obj = 0;
			return false;
		}
	}

	void SetBase(qCtxBased* base) {
		if (myBase) 
			myBase->Free();
		base->AddRef(); 
		myBase = base;
	}

	void SetBase(qCtx* base) {
		if (myBase) 
			myBase->Free();
		base->AddRef(); 
		myBase = base;
	}
};
*/

/// object that contains its own "based" subcontext/namespace

class qObjClass : public qObjTS
{
	qCtxBased *myCtx;
	qObjClass *myBase;
	qCtx      *myChild;

public:
// override virtuals
	 qObjClass(qObjClass *base = NULL);
	~qObjClass();

	qCtx *GetCtx()       {return myCtx;}
	virtual void Eval(qCtx *ctx, qStr *out, qArgAry *args);

	void Free(bool force = false) {
		if (GetRefs() == 2)
			Unchain();
		qObjTS::Free(force);
	}

	void Unchain() {
		if (myChild) {
			qCtx *ppX = myChild;
			qCtx *pX = ppX->GetParent();
			while (pX && pX != myCtx) {
				ppX = pX;
				pX = pX->GetParent();
			}
			if (pX)
				ppX->myParent = pX->GetParent();
			else
				ppX->myParent = NULL;

			myChild = NULL;
			myCtx->myParent = NULL;
		}
	}

	void Chain(qCtx *ctx) {
		Unchain();
		if (ctx) {
			myChild = ctx;
			myCtx->myParent = myChild->GetParent();
			myChild->myParent = myCtx;
		}
	}
};

/// object that points to a context
class qObjCtxP : public qObj 
{
protected:
	qCtx      *myCtx;
	bool       myFree;

public:
// override virtuals
	 qObjCtxP(qCtx *ctx = NULL, bool free = false)
		{ myCtx = ctx; myFree = free; }
	~qObjCtxP()							{ if (myFree) myCtx->Free(); }
	qCtx *GetCtx()						{ return myCtx; }
	void SetFree(bool free)				{ myFree = free; }

	virtual void Eval(qCtx *ctx, qStr *out, qArgAry *args);
};
#endif //#ifndef _QCTX_BASED_
