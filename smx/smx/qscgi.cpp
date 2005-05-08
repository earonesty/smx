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
		qEnvCGI env(NULL, stdout, 1024);
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
			setenv("PATH_TRANSLATED","<shell>",0);
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

			if (path) {
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

			setenv("PATH_TRANSLATED",file,0);

			FILE *fp = fopen(file, "r");
			if (fp) {
				qEnvCGI env(fp, stdout, 1024);
				
				ctx.SetEnv(&env);
				env.SetSessionCtx(&ctx);

				char *p_opt;
				while ((p_opt =  getopt("m*odule"))) {
					ctx.Eval("module", p_opt);
				}

				if (cgi) {
					ctx.ParseTry(&env, env.GetBuf());
					env.PutS("Content-length: ");
					env.PutN(env.GetBuf()->Length());
					env.PutS("\n\n");
					env.PutS(*env.GetBuf());
				} else {
					ctx.ParseTry(&env, &env);
				}

				fclose(fp);
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




#ifndef linux
#include "shlwapi.h"
#endif

qEnvCGI::qEnvCGI(FILE *script, FILE *out, int newBufSize)
{
	myIn = script;
	myOut = out;
	myBufSize = newBufSize;
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

CStr qEnvCGI::MapFullPath(const char *path) 
{
	CStr path1 = GetHeader("PATH_TRANSLATED");
	CStr path2 = path;
	path1.RTrim();
	CStr path3(path1.Length()+path2.Length()+1);
	PathCombine(path3.GetBuffer(),path1,path2);
	path1.Grow(path3.Length());
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
