/*
Copyright 1998-2005 Erik Aronesty. All rights reserved.

        1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

        2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

	3. Original source code, information and documentation is available at www.smxlang.org

THIS SOFTWARE IS PROVIDED 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE SMX AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/



#ifndef _QSCHED_H
#define _QSCHED_H

#include <limits.h>

#ifdef WIN32
#include <process.h>
#include <winbase.h>
#endif

#include "dparse.h"
#include "qthread.h"

class qSchedElem
{
//	long myRefs;

	CStr myErrs;
	CStr myName;
	CStr myBody;

	bool myPause;
	bool myDuring;

	time_t myFirst;
	time_t myNext;
	long   myOffset;
	time_t myLast;

	int mySec1;
	int mySec2;
	int myMin1;
	int myMin2;
	int myHour;
	int myDayW;
	int myDayM;
	int myMonth;
	int myYear;

	inline bool BitSet(int &Field, int Bits, int Index) {
		return Index >= 0 && Index < Bits && (Field |= (0x1 << Index));
	}

	inline bool BitSetX(int &Field1, int &Field2, int Bits, int Index) {
		if (Index < 32)
			return Index >= 0 && (Field1 |= (0x1 << Index));
		else
			return Index < (Bits-32) && (Field2 |= (0x1 << Index));
	}

	bool SetNumeric(const char *p, const char *t, bool (qSchedElem::*SetBit)(int i));

// since we're parsing...a simple string... use simple errors
	inline bool ErrAdd(const char *p, const char *t) {
		myErrs << "Bad " << t << " at " << CStr (p, min((size_t)3,strlen(p))) << "...\n";
		return true;
	}
	inline int ErrCount() {
		return myErrs.Length();
	}
	inline void ErrClear() {
		myErrs = "";
	}

public:
	qSchedElem() {
		myPause = 0;
		myDuring = 0;

		myFirst = 0;
		myNext = 0;
		myOffset = 0;
		myLast = 0;

		mySec1 = 0;
		mySec2 = 0;
		myMin1 = 0;
		myMin2 = 0;
		myHour = 0;
		myDayW = 0;
		myDayM = 0;
		myMonth = 0;
		myYear = 0;
//		myRefs = 1;
	}
/*
	void AddRef() {
		InterlockedIncrement(&myRefs);
	}

*/
	void Free() {
//		if (InterlockedDecrement(&myRefs) == 0) 
			delete this;
	}

	bool SetMonth(int i) { return BitSet(myMonth, 12,  i); }
	bool SetDayW(int i)  { return BitSet(myDayW,   7,  i); }
	bool SetDayM(int i)  { return BitSet(myDayM,  31,  i); }
	bool SetMin(int i)   { return BitSet(myHour,  24,  i); }
	bool SetHour(int i)  { return BitSetX(myMin1, myMin2, 60,  i); }
	bool SetSec(int i)   { return BitSetX(mySec1, mySec2, 60,  i); }

	bool SetMonth(const char *p);
	bool SetDayW(const char *p);
	bool SetDayM(const char *p)  { return SetNumeric(p, "mm", &qSchedElem::SetDayM); }
	bool SetHour(const char *p)  { return SetNumeric(p, "hh", &qSchedElem::SetHour); }
	bool SetMin(const char *p)   { return SetNumeric(p, "nn", &qSchedElem::SetMin); }
	bool SetSec(const char *p)   { return SetNumeric(p, "ss", &qSchedElem::SetSec); }

	bool SetFlags(const char *mm, const char *dd, const char *ww, const char *hh, const char *nn, const char *ss) {
		ErrClear();
		if (mm)
			SetMonth(mm); 
		if (dd)
			SetDayM(dd);
		if (ww)
			SetDayW(ww);
		if (hh)
			SetHour(hh);
		if (nn)
			SetMin(nn);
		if (ss)
			SetSec(ss);
		return !ErrCount();
	}

	bool SetTimes(int First, int Offset, int Last) {
		myFirst = First;
		if (myFirst == 0)
			time(&myFirst);
		myNext = First;
		myOffset = Offset;
		if (myOffset <=0)
			myLast = myFirst;
		myLast = Last;
		return true;
	}

	bool CheckFlags(time_t t) {
		return true;
	}

	time_t CalcNext() {
		time_t cur;
		time(&cur);

		if (myLast && cur > myLast)
			return 0;

		if (myOffset <= 0)
			return myNext;

		int diff = (cur - myFirst);

		if (diff > 0) {
			int cnt  = (int) diff / myOffset;
			if (diff % myOffset)
				++cnt;
			myNext = myFirst + cnt * myOffset;
		} else {
			myNext = myFirst;
		}

		while (!CheckFlags(myNext) && (!myLast || myNext <= myLast)) {
			myNext += myOffset;
		}

		if (myLast && myNext > myLast)
			return 0;

		return myNext;
	}

	time_t GetNext() const {return myNext;}

	CStr GetName()    {return myName;}
	CStr GetBody()    {return myBody;}

	CStr GetSched()   {
		CStr sched;
		sched <<  "%s(f,"  << myFirst 
			  << ")%s(o,"  << myOffset
			  << ")%s(l,"  << myLast
			  << ")%s(s,"  << mySec1
			  << ")%s(S,"  << mySec2
			  << ")%s(n,"  << myMin1
			  << ")%s(N,"  << myMin2
			  << ")%s(h,"  << myHour
			  << ")%s(w,"  << myDayW
			  << ")%s(d,"  << myDayM
			  << ")%s(m,"  << myMonth
			  << ")%s(y,"  << myYear
			  << ")";
		return sched;
	}

	CStr GetInterval() {
		if (myOffset) {
			qStrBuf buf;
		
			time_t myHH = myOffset % 86400;
			time_t myDD = myOffset - myHH;
			if (myDD) {
				buf.PutN(myDD);
				buf.PutC('d');
			}
			if (myHH) {
				struct tm tx = *gmtime(&myHH);
				FmtTime(&tx, "hh:nn", &buf);
			}
			return buf.GetS();
		} else
			return CStr::Null;
	}

	void SetSched(char typ, int bits) {
		switch(typ) {
			case 'f':myNext = myFirst = bits; break;
			case 'o':myOffset= bits; break;
			case 'l':myLast  = bits; break;
			case 's':mySec1  = bits; break;
			case 'S':mySec2  = bits; break;
			case 'n':myMin1  = bits; break;
			case 'N':myMin2  = bits; break;
			case 'h':myHour  = bits; break;
			case 'w':myDayW  = bits; break;
			case 'd':myDayM  = bits; break;
			case 'm':myMonth = bits; break;
			case 'y':myYear  = bits; break;		// note... this is not used!
		}
	}

	void SetName(const CStr &name)  {myName = name;}
	void SetBody(const CStr &body)  {myBody = body;}

	void SetPause(bool pause)       {myPause = pause;}
	bool GetPause()                 {return myPause;}

	void SetDuring(bool during)       {myDuring = during;}
	bool GetDuring()                 {return myDuring;}
};

class qSchedPool : public qObjTS
{
	qCtxTmp  myCtx;

	CCrit myCrit;
	qPriQ<time_t, qSchedElem *> *myHeap;
	CMapStr<qSchedElem *> myMap;

	time_t myGodot;
	time_t myOKLate;

	qThread myThread;
	CEvent myEvent;

	bool Schedule(qSchedElem * elem, time_t t, bool persist);

	void Load(qCtx *ctx);

	void RunThread();

	static int RunThread(void *me) {
		((qSchedPool*)me)->RunThread();
		return 0;
	}

	int Define(qCtx *ctx, qStr *out, qArgAry *args);
	void Load(qCtx *ctx, qStr *out, qArgAry *args);

	bool Delete(const char *name, bool pset);

//	void Delete(qCtx *ctx, qStr *out, qArgAry *args) {
//		CStr name = (*args)[0];
//		Delete(name, true);
//	}
	void Fire(qCtx *ctx, qStr *out, qArgAry *args) {
		qSchedElem *elem;
		if (myMap.Find((*args)[0], elem))
			myCtx.Parse(elem->GetBody(), out);
	}
	void Pause(qCtx *ctx, qStr *out, qArgAry *args) {
		qSchedElem *elem;
		if (myMap.Find((*args)[0], elem))
			elem->SetPause(true);
	}
	void Resume(qCtx *ctx, qStr *out, qArgAry *args) {
		qSchedElem *elem;
		if (myMap.Find((*args)[0], elem)) {
			if (elem->GetDuring()) {
				myCtx.Parse(elem->GetBody(), out);
				elem->SetDuring(false);
			}
			elem->SetPause(false);
		}
	}
	void GetMacro(qCtx *ctx, qStr *out, qArgAry *args) {
		qSchedElem *elem;
		if (myMap.Find((*args)[0], elem))
			out->PutS(elem->GetBody());
	}
	void GetSched(qCtx *ctx, qStr *out, qArgAry *args) {
		qSchedElem *elem;
		if (myMap.Find((*args)[0], elem))
			out->PutS(elem->GetSched());
	}

	void Enum(qCtx *ctx, qStr *out, qArgAry *args) {

		qCtxTmp sub(ctx);

		int  next;
		CStr name;
		CStr sched;
		CStr interval;
		CStr body;

		sub.MapObj(&next, "date");
		sub.MapObj(&name, "id");
		sub.MapObj(&body, "macro");
		sub.MapObj(&body, "interval");
		sub.MapObj(&sched,"schedule");

		MAPPOS pos; qSchedElem *elem;
		for(pos = myMap.First(); myMap.Next(&pos, &name, &elem); ) {
			next     = elem->GetNext();
			sched    = elem->GetSched();
			interval = elem->GetInterval();
			body     = elem->GetBody();
			sub.Parse(args->GetAt(0), out);
		}
	}

	int FindElem(qSchedElem *elem) {
		CLock lock = myCrit.Enter();

		qSchedElem *cur = NULL;
		int i = 0; time_t next;
		while(myHeap->getnext(++i, &next, &cur)) {
			if (cur == elem) {
				return i;
			}
		}
		return 0;
	}

	bool RemoveElem(qSchedElem *elem) {
		CLock lock = myCrit.Enter();
		int i = FindElem(elem);
		if (i > 0) {
			myHeap->remove(i);
			return true;
		} else
			return false;
	}

	qSchedElem *RemoveHeap() {
		CLock lock = myCrit.Enter();
		if (myHeap && !myHeap->empty()) {
			qSchedElem * e = myHeap->remove();

#ifdef _DEBUG
			assert(!FindElem(e) );
#endif
			return e;
		} else
			return NULL;
	}

	void InsertHeap(time_t t, qSchedElem *e) {
		CLock lock = myCrit.Enter();

#ifdef _DEBUG
		assert(!FindElem(e) );
#endif
		myMap.Set(e->GetName(), e);

		myHeap->insert(-t, e);
	}

public:

	qSchedPool(qCtx *ctx);

	qCtx *GetCtx() {
		return &myCtx;
	}

	~qSchedPool() {
		qPriQ<time_t, qSchedElem *> *spare = myHeap;

		{
			CLock lock = myCrit.Enter();
			while (!myHeap->empty()) {
				qSchedElem *elem = myHeap->remove();
				myMap.Del(elem->GetName());
				elem->Free();
			}
			myHeap = NULL;
			KickThread();
		}
		myThread.Wait(500);
		myThread.Kill();
		
		spare->destruct();
	}

	void KickThread() {
		myEvent.Pulse();
	}

	void SpawnThread() {
		myThread.Run(RunThread, this);
	}

	bool Schedule(const char *mm, const char *dd, const char *ww, const char *hh, const char *nn, const char *ss, bool Persist = true) {
		qSchedElem * pElem;
		pElem->SetFlags(mm, dd, ww, hh, nn, ss);
		return Schedule(pElem, pElem->CalcNext(), Persist);
	}

	bool Schedule(int First, int Offset, int Last, bool Persist = true) {
		qSchedElem * pElem = new qSchedElem;
		pElem->SetTimes(First, Offset, Last);
		return Schedule(pElem, pElem->CalcNext(), Persist);
	}

	int Display(qCtx *ctx, qStr *out, qArgAry *args) {
		qPriQ<time_t, qSchedElem *> copy(*myHeap);
		qSchedElem * elem;
		int  tnext = 0;
		int  count;
		CStr name;
		CStr sched;
		CStr body;

		qCtxTmp tmp(ctx);
		tmp.MapObj(&tnext, "next");
		tmp.MapObj(&name,  "name");
		tmp.MapObj(&body,  "body");
		tmp.MapObj(&sched, "schedule");

		count = 1;
		while (!copy.empty()) {
			elem  = copy.remove();
			tnext = elem->GetNext();
			name  = elem->GetName();
			sched  = elem->GetSched();
			body  = elem->GetBody();
			tmp.Parse(args->GetAt(0), out);
			++count;
		}
		return count;
	}
};

#endif // #ifndef _QSCHED_H
