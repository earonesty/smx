/*
Copyright 1998-2005 Erik Aronesty. All rights reserved.

        1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

        2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

	3. Original source code, information and documentation is available at www.smxlang.org

THIS SOFTWARE IS PROVIDED 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE SMX AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/



#include "stdafx.h"
#include "qstr.h"
#include "qobj.h"
#include "qctx.h"

#include "qpriq.h"
#include "qsched.h"

#include "util.h"

#include <limits.h>

enum {
	I_JAN,
	I_FEB,
	I_MAR,
	I_APR,
	I_MAY,
	I_JUN,
	I_JUL,
	I_AUG,
	I_SEP,
	I_OCT,
	I_NOV,
	I_DEC
};

enum {
	I_SUN,
	I_MON,
	I_TUE,
	I_WED,
	I_THU,
	I_FRI,
	I_SAT
};




bool qSchedElem::SetNumeric(const char *p, const char *t, bool (qSchedElem::*SetBit)(int i)) 
{
	bool err = false;
	char *e;
	while (*p) {
		while (*p && !isdigit(*p))
			++p;
		if (*p) {
			int i = strtol(p, &e, 10);
			if (!(this->*SetBit)(i)) {
				err = ErrAdd(p, t);
			}
		}
	}
	return err;
}

bool qSchedElem::SetDayW(const char *ww) 
{
	bool err = false;
	const char *p = ww;
	while (*p) {
		switch (*p) {
		case 'S':
			if (p[1] == 'u')
				SetDayW(I_SUN), ++p;
			else if (p[1] == 'a')
				SetDayW(I_SAT), ++p;
			else
				err = ErrAdd (p, "dw");
		case 'T':
			if (p[1] == 'u')
				SetDayW(I_TUE), ++p;
			else if (p[1] == 'h')
				SetDayW(I_THU), ++p;
			else
				err = ErrAdd (p, "dw");
		case 'M':
			SetMonth(I_MON);
		case 'W':
			SetMonth(I_WED);
		case 'F':
			SetMonth(I_FRI);
		default:
			err = ErrAdd (p, "dw");
		}
		++p;
	}
	return err;
}

bool qSchedElem::SetMonth(const char *mm) 
{
	bool err = false;
	const char *p = mm;
	while (*p) {
		switch (*p) {
		case 'J':
			if (p[1] == 'a')
				SetMonth(I_JAN), ++p;
			else if (p[1] == 'u')
				if (p[2] == 'n')
					SetMonth(I_JUN), p+=2;
				else if (p[2] == 'l')
					SetMonth(I_JUL), p+=2;
			else if (p[1] == 'e')
				SetMonth(I_JUN), ++p;
			else if (p[1] == 'y')
				SetMonth(I_JUL), ++p;
			else
				err = ErrAdd(p, "mm");
		case 'M':
			if (p[1] == 'y')
				SetMonth(I_MAY), ++p;
			else if (p[1] == 'r')
				SetMonth(I_MAR), ++p;
			else if (p[1] == 'a')
				if (p[2] == 'y')
					SetMonth(I_MAY), p+=2;
				else
					SetMonth(I_MAR), ++p;
			else
				err = ErrAdd (p, "mm");
		case 'A':
			if (p[1] == 'u')
				SetMonth(I_AUG), ++p;
			else if (p[1] == 'p')
				SetMonth(I_APR), ++p;
			else
				err = ErrAdd(p, "mm");
		case 'F':
			SetMonth(I_FEB);
		case 'D':
			SetMonth(I_DEC);
		case 'N':
			SetMonth(I_NOV);
		case 'O':
			SetMonth(I_OCT);
		case 'S':
			SetMonth(I_SEP);
		default:
			err = ErrAdd (p, "mm");
		}
		++p;
	}
	return err;
}

void qSchedPool::RunThread() {
	qStrNull nul;
	qSchedElem *e = NULL;

	while (myHeap) {
		while ((e = RemoveHeap())) {
			time_t now;

			myGodot = e->GetNext();
			now = time(0);

			if (now >= myGodot) {
				if (myGodot >= 0) {
					if (!e->GetPause()) {
						try {
							myCtx.Parse(e->GetBody(), &nul);
						} catch (...) {
							// TODO: LOG EVENTS
						}
					} else
						e->SetDuring(true);
				}
				
				time_t next = e->CalcNext();

				if (next > myGodot) {
					Schedule(e, next, false);
				} else {
					CLock lock = myCrit.Enter();
					myMap.Del(e->GetName());
					myCtx.Eval("pset", "/schedule/"<<e->GetName());
					e->Free();
				}
			} else {
				InsertHeap(myGodot, e);
				myEvent.Wait(min(10000,(int)(1000 * (myGodot-now) - 100)));
			}
		}
		myEvent.Wait(10000);
	}
}

void EvalSetBody(const void *pvelem, qCtx *ctx, qStr *out, qArgAry *args) {
	qSchedElem *elem = (qSchedElem *) pvelem;
	elem->SetBody(args->GetAt(0));
}

void EvalSetSched(const void *pvelem, qCtx *ctx, qStr *out, qArgAry *args) {
	qSchedElem *elem = (qSchedElem *) pvelem;
	if (!args->GetAt(0).IsEmpty() && !args->GetAt(1).IsEmpty()) {
		elem->SetSched(args->GetAt(0)[0], atoi(args->GetAt(1)));
	}
}

void qSchedPool::Load(qCtx *ctx, qStr *out, qArgAry *args) {
	qSchedElem *elem = NULL;

	CStr name  = (*args)[0];
	CStr data  = (*args)[1];

	elem = new qSchedElem;
	elem->SetName(name);

	qCtxTmp tmp(ctx);
	tmp.MapObj(elem, EvalSetBody, "b");
	tmp.MapObj(elem, EvalSetSched,"s");
	tmp.Parse(data, out);

	Schedule(elem, elem->CalcNext(), false);
}

bool qSchedPool::Schedule(qSchedElem * elem, time_t t, bool persist)
{
	if (persist)
		myCtx.Eval("pset", 
			"/schedule/"<<elem->GetName(), 
			CStr() << elem->GetSched() << "%b(" << elem->GetBody() << ")"
			);

	{
		CStr name = elem->GetName();

		Delete(name, false);

		assert(!FindElem(elem));

		InsertHeap(t, elem);
		
		if (myThread.IsDead())
			SpawnThread();

		if (myGodot > t)
			KickThread();
	}

	return true;
}

int qSchedPool::Define(qCtx *ctx, qStr *out, qArgAry *args) {
	qSchedElem *elem = NULL;

	CStr name  = (*args)[0];
	CStr body  = args->GetAt(1);
	CStr arg2 = (*args)[2];
	time_t t;

	if (strchr((const char *)arg2,'/') > 0) {
		time_t s = date_parse(arg2);
		int    o = ParseInt((*args)[3]);
		time_t e;

		CStr arg4 = (*args)[4];

		if (!arg4.IsEmpty())
			e = date_parse(arg4);
		else
			e = 0;

		elem = new qSchedElem;
		elem->SetTimes(s, o, e);
	} else if (strchr((const char *)arg2,':') > 0) {
		elem = new qSchedElem;
		time_t s = date_parse(arg2);
		elem->SetTimes(s, 86400, 0);
		
		CStr ww = (*args)[3];
		CStr dd = (*args)[4];
		CStr mm = (*args)[5];

		elem->SetFlags(mm, dd, ww, 0, 0, 0);
	} else if ((t = atol(arg2)) > 0 || args->Count() == 3) {
		elem = new qSchedElem;
		elem->SetTimes(t, 0, 0);
	}

	if (elem) {
		elem->SetBody(body);
		elem->SetName(name);
		
		time_t t = elem->CalcNext();
		time_t now = time(0);

		if (t > (now - myOKLate)) {
			Schedule(elem, t, true);
		} else { 
			elem->Free();
		}
	}

	return 0;
}

bool qSchedPool::Delete(const char *name, bool pset) {
	CLock lock = myCrit.Enter();

	qSchedElem *elem;
	if (myMap.Find(name, elem)) {

		if (RemoveElem(elem)) {
			elem->Free();
		}

		myMap.Del(name);

		if (pset)
			myCtx.Eval("pset", CStr("/schedule/")<<name);

#ifdef _DEBUG
		assert(!FindElem(elem));
#endif

		return true;
	} else 
		return false;
}

qSchedPool::qSchedPool(qCtx *ctx) : myCtx(ctx) {
	assert(ctx != NULL);


	myCtx.SetParent(ctx);
	myGodot = 0;
	myOKLate = 30;

	myHeap = new qPriQ<time_t, qSchedElem *>(LONG_MAX, 100);

	ctx->MapObj((qObj *) this, "<schedule>");

	ctx->MapObj(this, (QOBJMETH) &qSchedPool::Define,    "schedule-event", "01");
//	ctx->MapObj(this, (QOBJMETH) &qSchedPool::Load,      "load-event");

	ctx->MapObj(this, (QOBJMETH) &qSchedPool::Delete,    "delete-event");
	ctx->MapObj(this, (QOBJMETH) &qSchedPool::Fire,      "do-event");
	ctx->MapObj(this, (QOBJMETH) &qSchedPool::Pause,     "pause-event");
	ctx->MapObj(this, (QOBJMETH) &qSchedPool::Resume,    "resume-event");

	ctx->MapObj(this, (QOBJMETH) &qSchedPool::GetMacro,  "get-event-macro");
	ctx->MapObj(this, (QOBJMETH) &qSchedPool::GetSched,  "get-event-times");

	ctx->MapObj(this, (QOBJMETH) &qSchedPool::Enum,      "enum-events", "1");

	ctx->Eval("penumvalues", "/schedule/", "%load-event(%name%,%value%)");
}

void LoadSched(qCtx *ctx)
{
#ifdef WIN32
	qSchedPool * pool = new qSchedPool(ctx);
#endif
}
