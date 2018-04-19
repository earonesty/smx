/*
Copyright 1998-2005 Erik Aronesty. All rights reserved.

        1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

        2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

	3. Original source code, information and documentation is available at www.smxlang.org

THIS SOFTWARE IS PROVIDED 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE SMX AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#ifndef _QSTRPIPE_H
#define _QSTRPIPE_H

#ifndef _QSTR_H
	#include "qstr.h"
#endif

#ifndef _PROCESS_INC
	#ifdef WIN32
		#include <process.h>
		#include <io.h>
		#include <winsock.h>
	#else
		#include <unistd.h>
	#endif
	#include <fcntl.h>
#endif

#ifdef _WIN32 
#define pipe _pipe
#endif

#ifdef _DEBUG_BUF
	#define DEF_BUF_CB  32
	#define DEF_BUF_SIG 8
#else
	#define DEF_BUF_CB  8192
	#define DEF_BUF_SIG 32
#endif

#ifdef USE_EV_PIPE

class qStrPipe : public qStr {
	bool   myClosed;

	CEvent myEventR;
	CEvent myEventW;

	char * myBuf[2];
	int    myBufCB;

	int    myBufXR;
	int    myBufXW;

	char  *myReadP;
	char  *myReadE;

	char  *myWriteP;
	char  *myWriteE;

	int    myCharW;
	int    myMinSig;

	void PutWait(int len) {
		if ( (myWriteP + len) > myWriteE ) {
			if (myBufXW != myBufXR) {
				while (myReadP < myReadE) {
					myEventR.Wait();
				}
				myBufXR = (1-myBufXR);
				myReadP = myBuf[myBufXR];
				myReadE = myWriteP;
				myEventW.Signal();

			}	
			myBufXW = (1-myBufXW);
			myWriteP = myBuf[myBufXW];
			myWriteE = myWriteP + myBufCB;
		}
	}
	
	void PutSignal()
	{
		bool sig = (myReadP >= myReadE);

		if (myBufXW == myBufXR && myWriteP > myReadE)
			myReadE = myWriteP;
		else if (sig) {
			myBufXR = (1-myBufXR);
			myReadP = myBuf[myBufXR];
			myReadE = myWriteP;
		}

		if (sig) {
			myEventW.Signal();
		}
	}

public:
	qStrPipe(int bufCB = DEF_BUF_CB, int minSig = DEF_BUF_SIG) {
		myBufCB  = bufCB;
		myMinSig = minSig;

		myCharW = 0;
		
		myBuf[0] = (char *) malloc(myBufCB);
		myBuf[1] = (char *) malloc(myBufCB);

		myWriteP = myReadP = myReadE = myBuf[0];
		myWriteE = myBuf[0] + myBufCB;

		myBufXR = myBufXW = 0;

		myClosed = false;
	}

	~qStrPipe() {
		Close(false);
		free(myBuf[0]);
		free(myBuf[1]);
	}

	char GetC() {
		while (!myClosed) {
			if (myReadP < myReadE)
				return *myReadP++; 
			else {
				myEventR.Signal();
				myEventW.Wait();
			}
		}
		return -1;
	}

	virtual CStr GetS() {
		while (!myClosed) {
			if (myReadP < myReadE) {
				int  sLen = myReadE-myReadP;
				CStr sRet(myReadP, sLen); 
				myReadP += sLen;
				myEventR.Signal();
				return sRet;
			} else {
				myEventW.Wait();
			}
		}
		return NULL;
	}

	virtual void PutS(const char *s, int len) {
		PutWait(len);

		strncpy(myWriteP, s, len);
		myWriteP += len;

		PutSignal();
	}

	void PutC(char c) {
		PutWait(1);

		*myWriteP++ = c;

		if (myCharW++ > myMinSig && myReadP >= myReadE) {
			myCharW = 0;
			PutSignal();
		}
	}

	void Clear() {
		myReadP = myReadE;
	}

	void Close(bool WaitForRead) {
		if (!myClosed && WaitForRead) {
			PutWait(myBufCB);
			while (myReadP < myReadE) {
				myEventR.Wait();
			}
		}
		myClosed = true;
	}
};

#else
class qStrPipe : public qStr {
	enum { READ, WRITE }; 
	int    myPipes[2];
public:
	qStrPipe(qStrPipe &copy) {
		myPipes[READ] = _dup(copy.myPipes[READ]);
		myPipes[WRITE] = _dup(copy.myPipes[WRITE]);
	}

	qStrPipe(int bufCB = DEF_BUF_CB, int minSig = DEF_BUF_SIG) {
#ifdef WIN32
		pipe(myPipes, bufCB, _O_BINARY);
#else
		pipe(myPipes);
#endif    
	}

	~qStrPipe() {
		CloseWrite();
		CloseRead();
	}


	long PrepChildRead() {
		if (!myPipes[WRITE])
			return 0;

		int hDup;
		if (!(hDup = _dup(myPipes[WRITE]))) {
			return 0;
		} else {
			close(myPipes[WRITE]);
			myPipes[WRITE] = hDup;
			return myPipes[READ];
		}
	}


	long PrepChildWrite() {
		if (!myPipes[READ])
			return 0;

		int hDup;
		if (!(hDup = _dup(myPipes[READ]))) {
			return 0;
		} else {
			close(myPipes[READ]);
			myPipes[READ] = hDup;
			return myPipes[WRITE];
		}
	}


	char GetC() {
		char c;
		if (myPipes[READ]) {
			#ifdef WIN32
				//if (WaitForSingleObject((HANDLE)_get_osfhandle(myPipes[READ]),3000) == WAIT_OBJECT_0) 
			#endif
			if (read( myPipes[READ], &c, 1 ) > 0)
				return c;
		}
		return -1;
	}

	virtual CStr GetS() {
		if (myPipes[READ]) {
			CStr sRet(DEF_BUF_CB); 
			#ifdef WIN32
				//if (WaitForSingleObject((HANDLE)_get_osfhandle(myPipes[READ]),3000) == WAIT_OBJECT_0) 
			#endif
			{
				int ret = read( myPipes[READ],sRet.GetBuffer(), DEF_BUF_CB );
				if (ret > 0)
					return sRet.Grow(ret);
			} 
		}
		return (char *) NULL;
	}

	void PutS(const char *s, int len) {
		if (myPipes[WRITE])
			write(myPipes[WRITE],s,len);
	}

	void PutC(char c) {
		if (myPipes[WRITE])
			write(myPipes[WRITE],&c,1);
	}

	void Clear() {
		// todo??
	}

	void CloseWrite() {
		if (myPipes[WRITE])
			close(myPipes[WRITE]);
		myPipes[WRITE] = 0;
	}

	void CloseRead() {
		if (myPipes[READ])
			close(myPipes[READ]);
		myPipes[READ] = 0;
	}

	int GetReadH() {
		return myPipes[READ];
	}

	int GetWriteH() {
		return myPipes[WRITE];
	}
};

#endif

#endif //#ifndef _QSTRPIPE_H
