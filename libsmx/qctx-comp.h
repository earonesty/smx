/*
Copyright 1998-2005 Erik Aronesty. All rights reserved.

        1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

        2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

	3. Original source code, information and documentation is available at www.smxlang.org

THIS SOFTWARE IS PROVIDED 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE SMX AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#ifndef _QCTX_COMP_
#define _QCTX_COMP_

#include "qctx.h"

#define T_LP '('
#define T_RP ')'
#define T_LC '{'
#define T_RC '}'

class qCtxComp;

extern char CMAGIC_V1[5];

class qCtxComp : protected qCtx{

	CStr  myOutBuf;
	bool  myRn;

private:
	inline void ParseC(qStr *in, qStr *out, char c);
	inline bool ParseFunc(qStr *in, qStr *out);
	void ParseCompArgs(qStr *in, qArgAry *out, char *qmap);
	void ParseCompArgs(qStr *in, qArgAry *out);
	bool ParseCompArg(qStr *in, qStrBuf &out, char c);

	void OutC(char c)  {myOutBuf << c;}
	void OutS(const CStr &s) {myOutBuf << s;}
	void OutFunc(qStr *out, CStr &name, qArgAry *ary);
	void OutFlush(qStr *out);

public:

   ~qCtxComp() {}

   qCtxComp(qCtx *p, bool rand = false) {
	   myRn = rand;
	   Construct(p, p->GetEnv());
   }

// macro parsing
	void   Parse(qStr *in, qStr *out);
};

#endif // #ifndef _QCTX_COMP_


void  RunCompiled(qCtx *ctx, qStr *in, qStr *out);
