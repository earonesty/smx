/*
Copyright 1998-2005 Erik Aronesty. All rights reserved.

        1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

        2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

	3. Original source code, information and documentation is available at www.smxlang.org

THIS SOFTWARE IS PROVIDED 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE SMX AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/


// sock.h
#ifndef _SOCK_H_
#define _SOCK_H_

#ifdef WIN32
	#include <winsock.h>
#endif

#ifdef unix
	#include "unixsock.h"
#endif

#ifdef _DEBUG
#define SOCK_DEFAULT_BUF_SIZE 128
#define SOCK_DEFAULT_TIMEOUT 10
#define SOCK_MAX_HOSTNAME 255
#else
#define SOCK_DEFAULT_BUF_SIZE 8192
#define SOCK_DEFAULT_TIMEOUT 10
#define SOCK_MAX_HOSTNAME 1024
#endif

#define SOCK_READ 1
#define SOCK_WRITE 2
#define SOCK_ERROR 4

class Sock
{
protected:
	BOOL m_connected;
			   
	long m_timeout_sec;
	long m_timeout_usec;
	long m_buf_size;

	SOCKET m_sock;
	char *m_buf;
	char *m_eol_p;
	int m_buf_bytes;

public:
// assignment actually plugs one Sock 
// into another's connection
	Sock * Layer(Sock *sock);
	Sock * operator =(Sock *sock) {return Layer(sock);}

	virtual int _Read(char *buf, int len);
	virtual int _Write(const char *buf, int len);

	Sock();
	virtual ~Sock();

	enum
	{
		ERR_IOCTL = -5,
		ERR_GETHOST = -4,
		ERR_CONNECT = -3,
		ERR_TIMEOUT = -2,
		IGNORE_ALL = -1
	};

	unsigned long m_hostAddr;
	char m_host[SOCK_MAX_HOSTNAME];
	int m_hostPort;

	unsigned long m_bindAddr;
	int m_bindPort;

	// methods

	// returns ZERO for OK
	virtual void Bind(const char *ip, int port = 0);
	virtual int Open(const char *host, int port);
	virtual void Close();
	
	// single-socket version... returns SOCK_READ|SOCK_WRITE|SOCK_ERROR
	virtual int Select();

	virtual int Write(int len);
	virtual int Write(const char * buf, int len);
	virtual int Write(const char * buf) {return Write(buf, strlen(buf));}

	virtual int Read(int len);
	virtual int ReadLine(char **line);
	

	int ReadWait(float timeout = 0.0F);

	long Ignore(long len);
	char *GetBuf() { return m_buf; }
	long GetBufSize() { return m_buf_size; }
	void SetBufSize(long size);
	

	// accessors
	BOOL Connected() {return m_connected;}
	float GetTimeout() {return (float) (m_timeout_sec + (m_timeout_usec / 1000.0));}

	void SetTimeout(float secs) 
		{m_timeout_sec = (long) secs; 
		 m_timeout_usec = (long) (1000.0 * (secs - (float) ((long) secs))); }

	char *GetHost() {return m_host;}
	int GetPort() {return m_hostPort;}
};

inline bool SockInit() {
	#if MAC
		GUSISetup(GUSIwithSIOUXSockets);
		GUSISetup(GUSIwithInternetSockets);
	#elif WIN32
		WSAData wsaData;
		WORD wVersionRequested = MAKEWORD(1, 1); 

	// WSA startup
		int wsa_err;
		if (wsa_err = WSAStartup(wVersionRequested, &wsaData)) 
			return false;
	#endif

	return true;
}

#endif // #ifndef _SOCK_H_
