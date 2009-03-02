/*
Copyright 1998-2005 Erik Aronesty. All rights reserved.

        1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

        2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

	3. Original source code, information and documentation is available at www.smxlang.org

THIS SOFTWARE IS PROVIDED 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE SMX AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/



#include "stdafx.h"

#include "qctx.h"

#include "smxext.h"
#include "smximpl.h"

void STDCALL smxExObjectImpl::Eval(smxExContext *pCtx, smxExStreamOut *pOut, const char *pArgs[], smxArgType pArgType[], int nNumArgs)
{
	qArgAry ary;
	int i;
	for (i = 0; i < nNumArgs; ++i) {
		ary.Add(pArgs[i]);
		ary.SetQuot(i, pArgType ? (pArgType[i] == smxQuotedString) : false);
	}

	smxExStreamOutWrap wrap(pOut, false);
	myObj->Eval(((smxExContextImpl *)pCtx)->GetCtx(), &wrap);
}


void smxExFuncWrap::Eval(qCtx *ctx, qStr *out, qArgAry *args) {
	smxExContextImpl exCtx(ctx, false);
	smxExStreamOutImpl exOut(out, false);
	myFunc(myData, &exCtx, &exOut, (const char **) (args ? (const char **) *args : NULL), (smxArgType*)args->GetQuots(), args->Count());
}

void smxExObjectWrap::Eval(qCtx *ctx, qStr *out, qArgAry *args) {
	smxExContextImpl exCtx(ctx, false);
	smxExStreamOutImpl exOut(out, false);
	myObj->Eval(&exCtx, &exOut, (const char **) (args ? (const char **) *args : NULL), (smxArgType*)args->GetQuots(), args->Count());
}

class smxExAllocWrap : public qObj {
        void *myMem;

public:
        smxExAllocWrap(int N) {
                myMem = malloc(N);
        }

        ~smxExAllocWrap() {
		if (myMem) free(myMem);
        }

        void Delete() {
                if (this)
                        delete this;
        }

	void * Mem() {
		return myMem;
	}

        void Eval(qCtx *ctx, qStr *out, qArgAry *args) {
        }

        char *GetQmap() {
                return S_QMAP;
        }
};


char gB[4] = {1,1,1,1};

void * STDCALL smxExContextImpl::Alloc(int pInt) {
	char memID[7];
	int i = 3;
	while (i >= 0 && gB[i] >= 125) {
		gB[i] = 1;
		--i;
	}
	++gB[i];
	memID[0] = 'M';
	memID[1] = '<';
	memID[2] = gB[0];
	memID[3] = gB[1];
	memID[4] = gB[2];
	memID[5] = gB[3];
	memID[6] = '\0';

	smxExAllocWrap *pA;
	myCtx->MapObj(pA = new smxExAllocWrap(pInt), memID);
	return pA->Mem();
}

