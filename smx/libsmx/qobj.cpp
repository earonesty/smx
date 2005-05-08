/*
Copyright 1998-2005 Erik Aronesty. All rights reserved.

	1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

	2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE SMX AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#include "stdafx.h"
#include "qstr.h"
#include "qobj.h"
#include "qctx.h"

#include "util.h"

void qStrNLTrim::PutS(const char *s)
{
	if (s) {
		char *p;
		while ((p = strchr(s, '\n'))) {
			if (myNL)
				myStr->PutS(s, p - s);
			else
				myStr->PutS(s, p - s + 1);
			s = p + 1;
			while(isspace(*s)) ++s;
			myNL = *s == 0;
		}
		if (myNL) {

			while(isspace(*s)) ++s;
		}
		if (*s != 0)
			myNL = false;
		myStr->PutS(s);
	}
}

void qStrNLTrim::PutS(const char *s, int n)
{
	if (s) {
		const char *p;
		const char *e = s + n;
		while ((p = (const char *) memchr(s, '\n', n))) {
			if (myNL)
				myStr->PutS(s, p - s);
			else
				myStr->PutS(s, p - s + 1);
			s = p + 1;
			while(s < e && isspace(*p)) ++s;
			n = e - s;
			myNL = (n == 0);
		}
		if (myNL) {
			while(s < e && isspace(*s)) ++s;
		}
		n = e - s;
		if (n > 0)

			myNL = false;
		myStr->PutS(s, n);
	}
}

qObjTSRef qObjTS::GetRef()
{
	return qObjTSRef(this);
}


void qObjArgv::Eval(qCtx *ctx, qStr *out, qArgAry *args)
{
	int index = ParseInt((*args)[0]);
	if (index >= 0 && index < myArgc)
		out->PutS(myArgv[index]);
}
