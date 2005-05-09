/*
Copyright 1998-2005 Erik Aronesty. All rights reserved.

        1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

        2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

	3. Original source code, information and documentation is available at www.smxlang.org

THIS SOFTWARE IS PROVIDED 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE SMX AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

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
