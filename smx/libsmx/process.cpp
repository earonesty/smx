/*
Copyright 1998-2005 Erik Aronesty. All rights reserved.

        1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

        2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

	3. Original source code, information and documentation is available at www.smxlang.org

THIS SOFTWARE IS PROVIDED 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE SMX AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#include "stdafx.h"

#include "qctx.h"
#include "qobj-ctx.h"
#include "qstr-pipe.h"

#include "strary.h"

#include "util.h"

#ifdef WIN32
#include <process.h>
#else
#include <sys/types.h>
#include <sys/wait.h>
#endif

#ifdef WIN32
	#define os_sleep(s) Sleep(s);
#endif

#ifndef STDCALL
  #define STDCALL _stdcall
#endif

// sleep for specified seconds
void EvalSleep(const void *data, qCtx *ctx, qStr *out, qArgAry *args) 
{
	if (args->Count() >= 1) {
		int ms = ParseInt((*args)[0]);
		if (ms > 0) {
			os_sleep(ms);
		}
	}
}

#ifdef WIN32
class qObjThread : public qObjTS
{
	HANDLE  myTH;
	CStr    myCode;
	CCrit   myLock;
	qCtx   *myCtx;

	qObjThread(qCtx *parent, CStr name, CStr code) {
		myCtx = new qCtxRef(parent);
		myCode = code;

		myCtx->qCtx::MapObj(this, (QOBJMETH) &qObjThread::EvalKill, "kill");
		myCtx->qCtx::MapObj(this, (QOBJMETH) &qObjThread::EvalWait, "wait");

		parent->MapObjLet(this, name);

		unsigned tid;
		myTH = (HANDLE) _beginthreadex(0, 0, &qObjThread::Run, this, CREATE_SUSPENDED, &tid);
		ResumeThread(myTH);
	}

public:

	~qObjThread() {
		if (myTH) {
			DWORD rVal = WaitForSingleObject(myTH, 10);
			if (rVal != WAIT_OBJECT_0)
				TerminateThread(myTH, ~0);
			CloseHandle(myTH);
		}
		myCtx->Clear(true);
		myCtx->Free();
	}

	static qObjThread * Spawn(qCtx *parent, CStr name, CStr code) {
		return new qObjThread(parent, name, code);
	}

	static unsigned STDCALL Run(void *parm) {
		((qObjThread *)parm)->Run();
		return 0;
	}

	void Run() {
		qStrNull nul;
		try {
			myCtx->Parse(myCode, &nul);
		} catch (...) {
		}
		myTH = 0;
	}

	void Eval(qCtx *ctx, qStr *out, qArgAry *args) {
		myCtx->Parse(args->GetAt(0), out);
	}

	char * GetQmap() {
		static char qm[2] = "1";
		return qm;
	}

	int EvalKill(qCtx *ctx, qStr *out, qArgAry *args) {
		CLock lock = myLock.Enter();
		if (myTH) {
			TerminateThread(myTH, ~0);
			CloseHandle(myTH);
			myTH = 0;
		}
		return 0;
	}

	int EvalWait(qCtx *ctx, qStr *out, qArgAry *args) {
		if (myTH) {
			double nMS = ParseDbl((*args)[0]);
			if (nMS == 0.0) nMS = -1;
			DWORD rVal = WaitForSingleObject(myTH, (int) nMS);
			if (rVal == WAIT_OBJECT_0)
				out->PutC('T');
		}
		return 0;
	}
};

void EvalSpawn(const void *data, qCtx *ctx, qStr *out, qArgAry *args)
{
	if (ctx->GetSafeMode()) {ctx->Throw(out, 301, "function not available"); return;}

	if (args->Count() > 1) {
		CStr name = (*args)[0];
		CStr exec = args->GetAt(1);
		if (!exec.IsEmpty()) {
			if (!name.IsEmpty() && !exec.IsEmpty()) {
				qObjThread::Spawn(ctx, name, exec);
			}
		}
	}
}
#endif


void EvalExec(const void *data, qCtx *ctx, qStr *out, qArgAry *args)
{
	if (ctx->GetSafeMode()) {ctx->Throw(out, 301, "function not available"); return;}

	CStr exec = (*args)[0];
	if (exec.IsEmpty())
		return;

	qStrPipe pIn;
	qStrPipe pOut;
	qStrPipe pErr;

	int pH = 0;

#ifdef WIN32
	// WINDOWS DUP's beforehand (essentially)
	
	STARTUPINFO si; memset(&si, 0, sizeof(si)); si.cb = sizeof(si);
    si.dwFlags     = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_HIDE;
	si.hStdInput   = (HANDLE) _get_osfhandle(pIn.PrepChildRead());
	si.hStdOutput  = (HANDLE) _get_osfhandle(pOut.PrepChildWrite());
	si.hStdError   = (HANDLE) _get_osfhandle(pErr.PrepChildWrite());

	PROCESS_INFORMATION pi;
	if (CreateProcess(NULL, exec.GetBuffer(), NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
		pH = (int) pi.dwProcessId;
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	} else {
		ctx->ThrowF(out, 505, "Execution error. %y", GetLastError());
		return;
	}

#else

    if ((pH = fork()) == -1) {
      ctx->ThrowF(out, 505, "Fork error. %y", GetLastError());
  		return;
    }

    CStr sh = getenv("SHELL");
    if (!sh) sh = "/bin/sh";
    
    if (!pH) {
  	  dup2(pIn.PrepChildRead()    ,0);
 		  dup2(pOut.PrepChildWrite()  ,1);
 		  dup2(pErr.PrepChildWrite()  ,2);

      const char *argv[4] = {sh, "-c", exec, NULL};
  		execv(const_cast<char *>((const char *)sh), const_cast<char *const*>(argv));
      exit(errno);
      }
#endif /* WIN32 */


	/* close our copies */

	 pIn.CloseRead();
	pOut.CloseWrite();
	pErr.CloseWrite();

	/* this read/write should be done in parallel, using something 
	   that can implement os independent "select" on multiple 
	   qStr objects --- */

	ctx->Parse(args->GetAt(1), &pIn);
	pIn.CloseWrite();

	out->PutS(pOut);
	pOut.CloseRead();

	qStrBuf err;
	err.qStr::PutS(pErr);
	pErr.CloseRead();
      
#ifdef unix
  int pstat;
	pH = waitpid(pH, (int *)&pstat, 0);
  if (WEXITSTATUS(pstat) > 0) {
    if (!err.Length()) {

      ctx->ThrowF(out, 505, "Execution error. %y", WEXITSTATUS(pstat));
      return;
    } else if (WEXITSTATUS(pstat) > 64) {
      int l = err.Length();
      char *msg = err.GetS().GetBuffer();
      if (msg[l-1]=='\n') msg[l-1] = '\0';
      if (!strncmp(msg, sh, sh.Length())) {
        msg += sh.Length();
        if (!strncmp(msg, ": line ", 7)) {
          msg += 7;
          while (*msg && *msg !=':') ++msg;
          ++msg;
        }
        while (*msg==' ') ++msg;
      }
      ctx->ThrowF(out, 505, "Execution error. %s", msg);
      return;
    }
  }
#endif

	qCtxTmp tmpCtx(ctx);
	tmpCtx.MapObj(err, "err");
	tmpCtx.Parse(args->GetAt(2), out);
}

void EvalMutex(const void *data, qCtx *ctx, qStr *out, qArgAry *args)
{
	if (ctx->GetSafeMode()) {ctx->Throw(out, 301, "function not available"); return;}

	if (args->Count() > 1) {
		CStr name = (*args)[0];
		if (!name.IsEmpty()) {
			CStr body = args->GetAt(1);
			CStr tbody = args->GetAt(3);

#ifdef WIN32
			int  time = ParseInt((*args)[2]);
			HANDLE hM = 0;
			if (!time) 
				time = INFINITE;
			try {
				fslash(name.GetBuffer());
				hM = CreateMutex(NULL, false, "PSX~" << name);
				if (hM) {

					DWORD dwRes = WaitForSingleObject(hM, time);
					if (dwRes == WAIT_OBJECT_0 || dwRes == WAIT_ABANDONED) {
						ctx->Parse(body, out);
						ReleaseMutex(hM);
					} else if (dwRes == WAIT_TIMEOUT) {

						if (tbody) 
							ctx->Parse(tbody, out);
					} else {
						ctx->ThrowF(out, 502, "Wait for mutex failed. %y", GetLastError());
					}

					ReleaseMutex(hM);
				} else {
					ctx->ThrowF(out, 502, "Create mutex failed. %y", GetLastError());
				}
			} catch (qCtxEx ex) {
				if (hM) ReleaseMutex(hM);
				ctx->Throw(out, ex.GetID(), ex.GetMsg());
			} catch (qCtxExAbort ex) {
				if (hM) ReleaseMutex(hM);
				throw ex;
			} catch (...) {
				if (hM) ReleaseMutex(hM);
				ctx->Throw(out, 501, "Unknown exception during mutex");
			}
#else
//				!!!TODO!!!  Unix Systemwide Mutex
#endif
		}
	}
}

void LoadProcess(qCtx *ctx) {
	ctx->MapObj(EvalSleep,   "sleep");
	ctx->MapObj(EvalMutex,   "mutex","0101");
  
#ifdef WIN32  
	ctx->MapObj(EvalSpawn,   "spawn", "01");
#endif
  
	ctx->MapObj(EvalExec,     "exec");
}
