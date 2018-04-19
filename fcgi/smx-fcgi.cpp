/*
Copyright 1998-2005 Erik Aronesty. All rights reserved.

        1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

        2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

	3. Original source code, information and documentation is available at www.smxlang.org

THIS SOFTWARE IS PROVIDED 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE SMX AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

// psmx.cpp : Defines the entry point for the console application.
//

#ifdef unix
#define NO_LOAD_SCHED
#endif
#define NO_LOAD_TSET

#include "stdafx.h"

#include "fcgi_config.h"
#include "fcgi_stdio.h"

// PATCH to fix incorrect prototype
#undef fwrite
#define fwrite FCGI_FWRITE
#define FCGI_FWRITE(p, s, n, f) FCGI_fwrite((void *)p, s, n, f)
// REMOVE when FCGI is fixed


#include "qstr.h"
#include "qobj.h"
#include "qctx.h"

#include "opt.h"
#include "libs.h"

#include "qscgi.h"

#ifdef _WIN32
#include <process.h>
#else
extern char **environ;
#endif

void printerr(const char * msg, const char *arg=NULL);

int main(int argc, char* argv[], char* envp[])
{
// Declare the variables needed
	setarg(&argc, argv);
	qEnvCGI env(NULL, stdout, 1024);
	qCtxTmp global(&env);
	LoadAllLibs(&global);

	char *init;
	if (init = getenv("SMX_INIT")) {
		try {
			global.Parse(init);
		} catch (...) {
			// log something here...
		}
	}

	while (FCGI_Accept() >= 0) {
		char * file = getenv("PATH_TRANSLATED");
		if (file) {
			FILE *fp = fopen(file, "r");
			if (fp) {
				qCtxTmp ctx(&global);
				qStrFileI fin(fp, true);
				qEnvCGI env(stdin, stdout, 1024);
				
				ctx.SetEnv(&env);
				env.SetSessionCtx(&ctx);

				qStrBuf out;

				ctx.ParseTry(&fin, &out);

				env.PutS("Content-Length: ");
				env.PutN(out.Length());
				env.PutC('\n');
				env.PrintHeaders();
				env.PutC('\n');
				env.PutS(out);
			} else {
				printerr("Failed to open file: %s", file);
			}
		} else {
			printerr("PATH_TRANSLATED not set");
		}
		FCGI_Finish();
        }
	global.Clear();
	return 0;
}

void printerr(const char *msg, const char *arg) {
        puts("Content-Type: text/html\n\n");
	printf(msg, arg);
	printf("\n\n");	
}

void dumpenv(const char * msg) {
	char **envp = environ;
	puts("Status: 500\n");
	puts("Content-Type: text/html\n\n");
	printf("%s:<p><pre>\n",msg);
	for ( ; *envp != NULL; envp++) {
		printf("%s\n", *envp);
	}
	printf("</pre>\n");
}

qEnvCGI::qEnvCGI(FILE *script, FILE *out, int newBufSize)
{
        myIn = script;
        myOut = out;
        myBufSize = newBufSize;
        myHeaders=NULL;
}

void qEnvCGI::PutS(const char *s)                {if(s) fputs(s, myOut);}
void qEnvCGI::PutS(CStr &s)                      {fwrite(s.Data(), 1, s.Length(), myOut);}
void qEnvCGI::PutS(const char *s, int n) {if (s) fwrite(s, 1, n, myOut);}
void qEnvCGI::PutC(char c)                       {fputc(c, myOut);}
char qEnvCGI::GetC()      {return fgetc(myIn);}
CStr qEnvCGI::GetS()  {CStr tmp(myBufSize); tmp.Grow(fread(tmp.GetBuffer(), 1, myBufSize, myIn)); return tmp;}

/* THESE HAVE TO BE DEFINED HERE TO FORCE THE LINKER TO USE FCGI stuff */

// this won't work unless fcmppos is defined... which it often isnt

int qStrFileI::GetLineNum() {
// painfully derive line number
#ifdef fcmppos
        fpos_t sav;
        fgetpos(myFile, &sav);
        fseek(myFile, 0, SEEK_SET);

        fpos_t p;
        fgetpos(myFile, &p);

        char c;
        int lc = 1;
        while (fcmppos(p,sav) < 0 && (c=fgetc(myFile) ) != EOF) {
                if (c == '\n')
                        ++lc;
        fgetpos(myFile, &p);
        }

        fsetpos(myFile,&sav);

        return lc;
#else
	return 0;
#endif
}

// why won't this link from libsmx?
#if defined(_WINDOWS) || defined(WIN32)
time_t GetFileModified(const FILE *fp)
{
        HANDLE hF = (HANDLE) _get_osfhandle(fp->_file);
        FILETIME ft; FILETIME lt;
        if (GetFileTime(hF,NULL, NULL, &ft)) {
                FileTimeToLocalFileTime(&ft,&lt);
                return ConvWin32Time(lt);
        }
        return 0;
}
#else
time_t GetFileModified(const FILE *fp)
{
        struct stat sb;
        if (!fstat(fileno((FILE *)fp), &sb)) {
                return sb.st_mtime;
        }
        return 0;
}
#endif

