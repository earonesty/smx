#include "stdafx.h"
#include "qctx.h"

#ifdef unix
#define NO_LOAD_SCHED
#endif
#define NO_LOAD_TSET

#include "libs.h"
#include "../smx/qscgi.h"

int main()
{
        qEnvCGI env(NULL, stdout, 1024);
        qCtxTmp global(&env);

	global.MapObj(9,       "test-map-int");
	global.Parse("%test-map-int%");

        LoadAllLibs(&global);
        qCtxTmp ctx(&global);
        env.SetSessionCtx(&ctx);

	ctx.Parse("%iadd(1,1)");

	return 0;
}

qEnvCGI::qEnvCGI(FILE *script, FILE *out, int newBufSize)
{
        myIn = script;
        myOut = out;
        myBufSize = newBufSize;
}

CStr qEnvCGI::GetHeader(const char *name)
{
	return "";
}

CStr qEnvCGI::MapFullPath(const char *path)
{
        return path;
}

bool qEnvCGI::Flush()
{
        return false;
}

