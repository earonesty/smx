/* COPYRIGHT 1998 Prime Data Corp. 
	All Rights Reserved.  Use of this file without prior written 
	authorization is strictly prohibited.
*/

#include "stdafx.h"
#include "qstr.h"
#include "qobj.h"
#include "qobj-ctx.h"

void qObjCtxP::Eval(qCtx *ctx, qStr *out, qArgAry *args) 
{
	if (myCtx) {
		myCtx->Parse(args->GetAt(0), out); 
	}
}

void qObjClass::Eval(qCtx *ctx, qStr *out, qArgAry *args) 
{
	if (myCtx) {
		if (myBase) {
			myCtx->SetBase(myBase->GetCtx());
		}
		qCtx *tmp = myCtx->GetParent();
		myCtx->SetParent(ctx);
		CStr bod = args->GetAt(0);
		myCtx->qCtx::Parse(bod, out); 
		myCtx->SetParent(tmp);
	}
}

qObjClass::qObjClass(qObjClass *base)
{
	myChild = NULL;
	myCtx = new qCtxBased;
	if (base) {
		myBase = base;
		myBase->AddRef();
		myCtx->SetBase(myBase->GetCtx());
	} else
		myBase = NULL;
}


qObjClass::~qObjClass()
{
	Unchain();
	if (myBase)
		myBase->Free();
	myCtx->Free();
}

