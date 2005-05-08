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
