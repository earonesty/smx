/*
Copyright 1998-2005 Erik Aronesty. All rights reserved.

        1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

        2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

	3. Original source code, information and documentation is available at www.smxlang.org

THIS SOFTWARE IS PROVIDED 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE SMX AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

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

int qEnvCGI::GetHeaders(qEnvHttpHeaderCB*)
{
}

void qEnvCGI::PrintHeaders()
{
}

