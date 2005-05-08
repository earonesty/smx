/* COPYRIGHT 1998 Prime Data Corp. 
	All Rights Reserved.  Use of this file without prior written 
	authorization is strictly prohibited.
*/

#ifndef _QARG_H
#define _QARG_H

class qObj;
class qCtx;

#include "strary.h"

typedef CStr qArg;

class qArgAry : public CStrAry
{
	char  *myOpts;
	char   myOptsN;
	enum {
		myGrowBy = 16
	};
public:
	qArgAry() {
		myOpts = 0; myOptsN = 0;
	}
	~qArgAry() {
		free(myOpts);
	}
	qArgAry(CStr *args, int n) : CStrAry(args, n) {
		myOpts = 0; myOptsN = 0;
	}
	qArgAry(int n) : CStrAry(n) {
		myOpts = 0; myOptsN = 0;
	}
	
	void Shift(int n) {
		CStrAry::Shift(n);
		if (myOpts) {
			myOpts++;
			myOptsN--;
		}
	}
	void Restore(int n) {
		CStrAry::Restore(n);
		if (myOpts) {
			myOpts--;
			myOptsN++;
		}
	}

	void SetQuot(int n, char value) {
		if (!this)
			return;

		if (n >= myOptsN) {
			myOpts = (char *) realloc(myOpts, n + myGrowBy);
			memset(myOpts + myOptsN, 0, n + myGrowBy - myOptsN);
			myOptsN = n + myGrowBy;
		}

		myOpts[n] = value;
	}

	char GetQuot(char n) {
		return (this && n < myOptsN) ? myOpts[n] : 0;
	}

	char *GetQuots() {
		return this ? myOpts : 0;
	}
};


#endif //#ifndef _QARG_H
