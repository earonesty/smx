/* COPYRIGHT 1998 Prime Data Corp. 
	All Rights Reserved.  Use of this file without prior written 
	authorization is strictly prohibited.
*/

#include "stdafx.h"

#include "qctx.h"

#include "psxext.h"
#include "psximpl.h"

void STDCALL psxExObjectImpl::Eval(psxExContext *pCtx, psxExStreamOut *pOut, const char *pArgs[], psxArgType pArgType[], int nNumArgs)
{
	qArgAry ary;
	int i;
	for (i = 0; i < nNumArgs; ++i) {
		ary.Add(pArgs[i]);
		ary.SetQuot(i, pArgType ? (pArgType[i] == psxQuotedString) : false);
	}

	psxExStreamOutWrap wrap(pOut, false);
	myObj->Eval(((psxExContextImpl *)pCtx)->GetCtx(), &wrap);
}


void psxExFuncWrap::Eval(qCtx *ctx, qStr *out, qArgAry *args) {
	psxExContextImpl exCtx(ctx, false);
	psxExStreamOutImpl exOut(out, false);
	myFunc(myData, &exCtx, &exOut, (const char **) (args ? (const char **) *args : NULL), (psxArgType*)args->GetQuots(), args->Count());
}

void psxExObjectWrap::Eval(qCtx *ctx, qStr *out, qArgAry *args) {
	psxExContextImpl exCtx(ctx, false);
	psxExStreamOutImpl exOut(out, false);
	myObj->Eval(&exCtx, &exOut, (const char **) (args ? (const char **) *args : NULL), (psxArgType*)args->GetQuots(), args->Count());
}
