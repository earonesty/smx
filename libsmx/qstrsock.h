/*
Copyright 1998-2005 Erik Aronesty. All rights reserved.

        1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

        2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

	3. Original source code, information and documentation is available at www.smxlang.org

THIS SOFTWARE IS PROVIDED 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE SMX AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/



#ifndef _QSTRSOCK_H
#define _QSTRSOCK_H

#include "qstr.h"

class qStrSockI : public qStr {
	SOCKET mySock;
	bool myFree;

public:
	qStrSockI() {
		mySock = INVALID_SOCKET;
	}

	~qStrSockI() {
		if (myFree && (mySock != INVALID_SOCKET))
			closesocket(mySock);
	}

	qStrSockI(SOCKET s, bool close = false) {
		SetSock(s, close);
	}

	void SetSock(SOCKET s, bool close = false) {
		mySock = s; 
		myFree = close;
	}

	char GetC() {
		char c;
		int n = recv(mySock, &c, 1, 0);
		if (n <= 0) c = EOF;
		return c;
	}

	CStr GetS() {
		CStr tmp(1024); 
		int n;
		if ((n=recv(mySock, tmp.GetBuffer(), 1024, 0)) > 0)
			tmp.Grow(n); 
		else
			tmp.Grow(0);
		return tmp;
	}

	int GetS(char *b, int n) {
		return recv(mySock, b, n, 0);
	}

	void PutS(const char *s) {assert(false);};
	void PutC(char c) {assert(false);};
	void PutS(const char *s, int len) {assert(false);};
};

#endif //#ifndef _QSTRSOCK_H
