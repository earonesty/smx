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

#include "qstr.h"
#include "qobj.h"
#include "qctx.h"

#include "opt.h"
#include "libs.h"
#include "qscgi.h"

int main(int argc, char* argv[], char* envp[])
{
// Declare the variables needed
	setarg(&argc, argv);
	if (argc > 1) {
		qEnvCGI env(stdin, stdout, 1024);
		qCtxTmp global(&env);
    
		global.MapArgv(argc, argv, "arg");
		global.MapObj(argc, "num-args");
    
		LoadAllLibs(&global);
		qCtxTmp ctx(&global);
		env.SetSessionCtx(&ctx);

		char *p_opt;
		while ((p_opt = getopt("m*odule"))) {
			ctx.Eval("module", p_opt);
		}

		if (getoptbool("e*xpand")) {
#ifdef WIN32
			CStr script_path;
			script_path.Grow(MAX_PATH);
			DWORD nb;
			if (nb = GetCurrentDirectory(MAX_PATH, script_path.GetBuffer())) {
				script_path.Shrink();
				script_path << "<shell>";
				setenv("PATH_TRANSLATED",script_path,0);
			}
			else 
#endif
			{
				setenv("PATH_TRANSLATED","./<shell>",0);
			}

			CStr toParse = argv[1];
			int i;
			for (i = 2; i < argc; ++i) {
				toParse << ' ' << argv[i];
			}
		        qStrReadBuf in(toParse);
			ctx.ParseTry(&in, &env);
		} else {
			bool cgi = getoptbool("c*gi") != 0;
			bool path = getoptbool("p*ath") != 0;
			CStr file = argv[1];

			#ifdef WIN32
			if (!cgi && path) {
				char *fp;
				SearchPath(
				  NULL,      // search path
				  argv[1],  // file name
				  ".cmd", // file extension
				  MAX_PATH, // size of buffer
				  file.Grow(MAX_PATH).GetBuffer(),     // found file name buffer
				  &fp   // file component
				);
			}
			#endif

			if (cgi && !getenv("PATH_TRANSLATED"))
				setenv("PATH_TRANSLATED",file,0);

			FILE *fp = fopen(file, "r");
			if (fp) {
				char c = fgetc(fp);
				if (c == '#') {
					do {
					    c = fgetc(fp);
					} while (c != '\n');
				} else {
					ungetc(c, fp);	
				}
				qStrFileI fin(fp, true);
				qStrBuf buf;
				env.SetSessionCtx(&ctx);

				if (cgi) {
					ctx.ParseTry(&fin, &buf);
					env.PutS("Content-Length: ");
					env.PutN(buf.Length());
					env.PutC('\n');
					env.PrintHeaders();
					env.PutC('\n');
					env.PutS(buf);
				} else {
					ctx.ParseTry(&fin, &env);
				}
			}
		}

    ctx.Clear();
    global.Clear();
    #ifdef unix
    if (isatty(1)) {
      fflush(stdout);
      write(1, "\n", 1);
    }
    #endif
	} else {
		printf("USAGE : smx [-c*gi]  <\"file\">     : expand file [optionally with http headers]\n"
		       "        smx -e*xpand <\"macro\">    : expand macro inline, output to stdout\n"
			  );
	}
 	return 0;
}



