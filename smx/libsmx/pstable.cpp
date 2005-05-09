/*
Copyright 1998-2005 Erik Aronesty. All rights reserved.

        1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

        2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

	3. Original source code, information and documentation is available at www.smxlang.org

THIS SOFTWARE IS PROVIDED 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE SMX AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/



#include "stdafx.h"

#include "qctx.h"

#include "tabfmt.h"
#include "tabpre.h"
#include "tabtd.h"

#include "util.h"

void EvalTabRowDef(const void *pObject, qCtx *pContext, qStr *out, qArgAry *args)
{
	if (args->Count() > 0) {
		CTabFmt *td = (CTabFmt *) pObject;
		int i; for (i = 0; i < args->Count(); ++i) {
			args->SetAt(i, (*args)[i]);
		}
		td->RowDef(*args, args->Count());
	}
}

void EvalTabRowStyle(const void *pObject, qCtx *pContext, qStr *out, qArgAry *args)
{
	CTabTD *td = (CTabTD *) pObject;
	if (args->Count() > 0)
		args->SetAt(0, (*args)[0]);
	if (args->Count() > 1)
		args->SetAt(1, (*args)[1]);
	td->SetRowEx(args->GetAt(0), args->GetAt(1));
}

void EvalTabRowH(const void *pObject, qCtx *pContext, qStr *pStream, qArgAry *args)
{
	if (args->Count() > 0) {
		CTabFmt *td = (CTabFmt *) pObject;
		int i; for (i = 0; i < args->Count(); ++i) {
			args->SetAt(i, (*args)[i]);
		}
		td->RowH(*args, args->Count());
	}
}

void EvalTabSubT(const void *pObject, qCtx *pContext, qStr *pStream, qArgAry *args)
{
	if (args->Count() > 0) {
		int l = atoi((*args)[0]) - 1;
		CTabFmt *td = (CTabFmt *) pObject;
		if (args->Count() > 1) {
			td->Lev = l;
			pContext->Parse(args->GetAt(1), pStream);
			td->Lev = -1;
		}
		td->CalcSub(l);
	}
}

void EvalTabRowS(const void *pObject, qCtx *pContext, qStr *pStream, qArgAry *args)
{
	if (args->Count() > 0) {
		CTabFmt *td = (CTabFmt *) pObject;
		int i; for (i = 0; i < args->Count(); ++i) {
			args->SetAt(i, (*args)[i]);
		}
		td->RowS(*args, args->Count());
	}
}

void EvalTabRow(const void *pObject, qCtx *pContext, qStr *pStream, qArgAry *args)
{
	if (args->Count() > 0) {
		CTabFmt *td = (CTabFmt *) pObject;
		int i; for (i = 0; i < args->Count(); ++i) {
			args->SetAt(i, (*args)[i]);
		}
		if (td->Lev >= 0)
			td->RowS(*args, args->Count());
		else
			td->Row(*args, args->Count());
	}
}

void EvalTabOut(const void *pObject, qCtx *pContext, qStr *pStream, qArgAry *args)
{
	if (args->Count() > 0) {
		CTabFmt *td = (CTabFmt *) pObject;
		td->Out((*args)[0]);
	}
}

// STATE INFO
void EvalTabWid(const void *pObject, qCtx *pContext, qStr *pStream, qArgAry *args)
{
	CTabFmt *td = (CTabFmt *) pObject;
	pStream->PutN(td->Wid());
}

void EvalTabCols(const void *pObject, qCtx *pContext, qStr *pStream, qArgAry *args)
{
	CTabFmt *td = (CTabFmt *) pObject;
	pStream->PutN(td->Cols());
}

void EvalTabCnt(const void *pObject, qCtx *pContext, qStr *pStream, qArgAry *args)
{
	CTabFmt *td = (CTabFmt *) pObject;
	int l = (args->Count() > 1) ? (ParseInt((*args)[0])-1) : td->Lev;
	if (l >= 0)
		pStream->PutN(td->Cnt(l));
	else
		pStream->PutN(td->Cnt());
}

void EvalTabSum(const void *pObject, qCtx *pContext, qStr *pStream, qArgAry *args)
{
	if (args->Count() > 0) {
		CTabFmt *td = (CTabFmt *) pObject; int i = ParseInt((*args)[0]);
		if (i >= 0 && i < td->Cols()) {
			int l = (args->Count() > 1) ? (ParseInt((*args)[1])-1) : td->Lev;
			if (l >= 0)
				pStream->PutN(td->Sum(l, i));
			else
				pStream->PutN(td->Sum(i));
		}
	}
}

void EvalTabDev(const void *pObject, qCtx *pContext, qStr *pStream, qArgAry *args)
{
	if (args->Count() > 0) {
		CTabFmt *td = (CTabFmt *) pObject; int i = ParseInt((*args)[0]);
		if (i >= 0 && i < td->Cols()) {
			int l = (args->Count() > 1) ? (ParseInt((*args)[1])-1) : td->Lev;
			if (l >= 0)
				pStream->PutN(td->Dev(l,i));
			else
				pStream->PutN(td->Dev(i));
		}
	}
}

void EvalTabAvg(const void *pObject, qCtx *pContext, qStr *pStream, qArgAry *args)
{
	if (args->Count() > 0) {
		CTabFmt *td = (CTabFmt *) pObject; int i = ParseInt((*args)[0]);
		if (i >= 0 && i < td->Cols()) {
			int l = (args->Count() > 1) ? (ParseInt((*args)[1])-1) : td->Lev;
			if (l >= 0)
				pStream->PutN(td->Avg(l,i));
			else
				pStream->PutN(td->Avg(i));
		}
	}
}

// PRE-FORMATTED HTML TABLE
void EvalTabPre(const void *data, qCtx *pContext, qStr *pStream, qArgAry *args)
{
	if (args->Count() > 0) {
		CTabPre td(pStream);

		qCtxTmp tmpCtx(pContext);

		tmpCtx.MapObj(&td, &EvalTabRowDef,"rowdef");
		tmpCtx.MapObj(&td, &EvalTabRow,   "row");
		tmpCtx.MapObj(&td, &EvalTabRowH,  "rowh");
		tmpCtx.MapObj(&td, &EvalTabRowS,  "rows");
		tmpCtx.MapObj(&td, &EvalTabSubT,  "subt");
		tmpCtx.MapObj(&td, &EvalTabOut,   "out");

		tmpCtx.MapObj(&td, &EvalTabSum,   "sum");
		tmpCtx.MapObj(&td, &EvalTabCnt,   "cnt");
		tmpCtx.MapObj(&td, &EvalTabWid,   "wid");
		tmpCtx.MapObj(&td, &EvalTabDev,   "dev");
		tmpCtx.MapObj(&td, &EvalTabAvg,   "avg");
		tmpCtx.MapObj(&td, &EvalTabCols,  "cols");

		qStrNull nul;
		tmpCtx.Parse(args->GetAt(0), &nul);
	}
}

void EvalTable(const void *data, qCtx *pContext, qStr *pStream, qArgAry *args)
{
	if (args->Count() > 0) {
		CTabTD td(pStream);

		qCtxTmp tmpCtx(pContext);

		pContext->SetValue("rowdef",&EvalTabRowDef, &td);
		pContext->SetValue("rowstyle",&EvalTabRowStyle, &td);
		pContext->SetValue("row",&EvalTabRow, &td);
		pContext->SetValue("rowh",&EvalTabRowH, &td);
		pContext->SetValue("rows",&EvalTabRowS, &td);
		pContext->SetValue("subt",&EvalTabSubT, &td);
		pContext->SetValue("out",&EvalTabOut, &td);

		pContext->SetValue("sum",&EvalTabSum, &td);
		pContext->SetValue("cnt",&EvalTabCnt, &td);
		pContext->SetValue("wid",&EvalTabWid, &td);
		pContext->SetValue("dev",&EvalTabDev, &td);
		pContext->SetValue("avg",&EvalTabAvg, &td);
		pContext->SetValue("cols",&EvalTabCols, &td);

		qStr *pNul = new qStrNull;

		pContext->Parse(args->GetAt(0), pNul);

		pNul->Delete();
	}
}

void LoadTable(qCtx *ctx)
{
	ctx->MapObj(EvalTable,  "table", "1");
	ctx->MapObj(EvalTabPre, "tabpre", "1");
}
/* END TABLE HOOK */
