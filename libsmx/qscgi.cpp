/*
Copyright 1998-2005 Erik Aronesty. All rights reserved.

        1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

        2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

	3. Original source code, information and documentation is available at www.smxlang.org

THIS SOFTWARE IS PROVIDED 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE SMX AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#ifdef unix
#define NO_LOAD_SCHED
#endif
#define NO_LOAD_TSET

#include "stdafx.h"

#include "qstr.h"
#include "qobj.h"
#include "qctx.h"
#include "libs.h"
#include "qscgi.h"

#ifdef WIN32
#include "shlwapi.h"
#endif

qEnvCGI::qEnvCGI(FILE *script, FILE *out, int newBufSize)
{
	myIn = script;
	myOut = out;
	myBufSize = newBufSize;
	myHeaders=NULL;
}

CStr qEnvCGI::GetHeader(const char *name)
{
	CStr buf = name;
	char *p = buf.GetBuffer();
	if (p) {
		while (*p) {
			if (*p == '-') *p = '_';
			else *p = toupper(*p);
			++p;
		}
		const char *env = getenv(strupr(buf.GetBuffer()));
		return CStr(env);
	} else {
		return 0;
	}
}

#define ISPATHSEP(c) ((c)=='/'||(c) =='\\'||(c) ==':')

CStr qEnvCGI::MapFullPath(const char *path) 
{
	CStr path1 = GetHeader("PATH_TRANSLATED");

	if (!path1.IsEmpty()) {
		CStr strip = GetHeader("PATH_INFO");
		if (strip.IsEmpty()) strip = GetHeader("SCRIPT_NAME");
		if (!strip.IsEmpty()){
		int i = strip.Length();
		int j = path1.Length();

		while (--i > 0 && --j > 0 && strip.Data()[i] == path1.Data()[j]) {
		}

		if ((j + 1) < path1.Length() && ISPATHSEP(path1.Data()[j+1])) {
			path1.Grow(j+1);
		}
		}
	} else {
		path1 = GetHeader("SCRIPT_NAME");
		int i = path1.Length();

		while (i-- > 0) {
			if (ISPATHSEP(path1.Data()[i])) {
				break;
			}
		}
		if (i >= 0) {
			path1.Grow(i);
		}
	}

	path1.RTrim();
	CStr path2 = path;
//	CStr path3(MAX_PATH);
	CStr path3(path1.Length()+path2.Length()+1);
	PathCombine(path3.GetBuffer(),path1,path2);
	path1.Grow(path3.Length()+1000);
	PathCanonicalize(path1.GetBuffer(),path3.GetBuffer());
	PathMakePretty(path1.GetBuffer());
	path1.Shrink();

	return path1;
}

bool qEnvCGI::Flush()
{
	// todo : rebind stream to context parser
	return false;
}

#ifdef WIN32
	#define environ _environ
#else
	extern char **environ;
#endif

int qEnvCGI::GetHeaders(qEnvHttpHeaderCB *CB)
{
	char **pp = environ;
	if (!pp) return 0;
	while(*pp) {
		char *p = *pp;
		if (!strncmp(p,"HTTP_",5)) {
		  p+=5;
		  char *v = p;
		  while (*v && *v != '=') ++v;
		  
		  CStr name(p,v-p);
		  if (*v == '=') ++v;
		  CB->Callback(name, v);
		}
		++pp;
	}
	return 1;
}

void qEnvCGI::PrintHeaders()
{
        CLst<HEADER_ENT> *lst = myHeaders;
        while(lst) {
		PutS(lst->Data().Name);
		PutS(": ");
		PutS(lst->Data().Value);
		PutC('\n');
		lst=lst->Next();
        }
}

void qEnvCGI::PutS(const char *s)                {if(s) fputs(s, myOut);}
void qEnvCGI::PutS(CStr &s)                      {fwrite(s.Data(), 1, s.Length(), myOut);}
void qEnvCGI::PutS(const char *s, int n) {if (s) fwrite(s, 1, n, myOut);}
void qEnvCGI::PutC(char c)                       {fputc(c, myOut);}
char qEnvCGI::GetC()      {return fgetc(myIn);}
CStr qEnvCGI::GetS()  {CStr tmp(myBufSize); tmp.Grow(fread(tmp.GetBuffer(), 1, myBufSize, myIn)); return tmp;}

