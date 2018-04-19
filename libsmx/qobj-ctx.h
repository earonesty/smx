/*
Copyright 1998-2005 Erik Aronesty. All rights reserved.

        1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

        2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

	3. Original source code, information and documentation is available at www.smxlang.org

THIS SOFTWARE IS PROVIDED 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE SMX AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

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
