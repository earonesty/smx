/* COPYRIGHT 1998 Prime Data Corp. 
	All Rights Reserved.  Use of this file without prior written 
	authorization is strictly prohibited.
*/
// sock.cpp
#ifdef WIN32
	#include <winsock.h>
#endif

#include <string.h>                                                                                                                                                         
#include <ctype.h>                                                                                                                                                         
#include <assert.h>

#include "sock.h"

#define sock_buf_bytes_left() (m_buf_bytes - (m_eol_p - m_buf))

Sock::Sock()
{
	m_timeout_sec = SOCK_DEFAULT_TIMEOUT;
	m_timeout_usec = 0;

	m_sock = 0;
	m_buf_size = SOCK_DEFAULT_BUF_SIZE;
	m_buf = (char *)malloc(SOCK_DEFAULT_BUF_SIZE + 1);
	m_eol_p = m_buf;
	m_buf_bytes = 0;
	m_connected = FALSE;
	*m_host ='\0';
	m_hostAddr = 0;
	m_hostPort = 0;
	m_bindAddr = INADDR_ANY;
	m_bindPort = 0;
}

Sock::~Sock()
{
	if (m_buf) free(m_buf);
	Close();
}

Sock * Sock::Layer(Sock *sock)
{
	assert(sock);

	m_sock = sock->m_sock;
	strcpy(m_host,sock->m_host);
	m_hostAddr = sock->m_hostAddr;
	m_hostPort = sock->m_hostPort;
	m_connected = sock->m_connected;

	sock->m_connected = FALSE;
	delete sock;

	return this;
}

void Sock::Close()
{
	if (m_connected)
	{
		closesocket(m_sock);
		m_connected = FALSE;
	}
}


int Sock::Select()
{
	timeval timeout = {m_timeout_sec,m_timeout_usec};
	fd_set readSet;  FD_ZERO(&readSet);  FD_SET(m_sock, &readSet); 
	fd_set writeSet; FD_ZERO(&writeSet); FD_SET(m_sock, &writeSet);
	fd_set errorSet; FD_ZERO(&errorSet); FD_SET(m_sock, &errorSet);

	if (select(m_sock+1, &readSet, &writeSet, &errorSet, &timeout) == 0)
		return ERR_TIMEOUT;
	else
		return 
			  (FD_ISSET(m_sock, &readSet) ? SOCK_READ : 0)
			| (FD_ISSET(m_sock, &writeSet) ? SOCK_WRITE : 0)
			| (FD_ISSET(m_sock, &errorSet) ? SOCK_ERROR : 0);
}

void Sock::Bind(const char *ip, int port)

{
	m_bindAddr = inet_addr(ip);
	m_bindPort = port;
}

int Sock::Open(const char *host, int port)
{
// get sock-address
	struct hostent *phe;

	if (stricmp(m_host,host))
	{
		if(isdigit(*host))
			m_hostAddr = inet_addr(host);
		else
		{
			if(phe = gethostbyname(host))
				m_hostAddr = *((unsigned long *)phe->h_addr_list[0]);
			else
				return ERR_GETHOST;
		}
		strncpy(m_host, host, 255);
	}

	m_hostPort = port;		

	if (m_connected) 
		Close();

	m_sock = socket(PF_INET, SOCK_STREAM, 0);

	if (m_bindAddr || m_bindPort) {
		sockaddr_in bindAddr;
		ZeroMemory(&bindAddr, sizeof (bindAddr));
		bindAddr.sin_family = AF_INET;
		bindAddr.sin_port = htons(m_bindPort);
		bindAddr.sin_addr.s_addr = ((m_bindAddr == INADDR_NONE) ? INADDR_ANY : m_bindAddr);
		bind(m_sock, (sockaddr *) &bindAddr, sizeof(bindAddr));
	}

	// create socket parameters for http functions
	sockaddr_in sockAddr;
	ZeroMemory(&sockAddr, sizeof (sockAddr));
	sockAddr.sin_family = AF_INET;
	sockAddr.sin_port = htons(port);
	sockAddr.sin_addr.s_addr = m_hostAddr;

	u_long noblock = 1;

	int retval;
	ioctlsocket(m_sock, FIONBIO, &noblock);
	retval = connect(m_sock, (sockaddr *) &sockAddr, sizeof(sockAddr));

   	if ((retval != SOCKET_ERROR) || ((retval==SOCKET_ERROR)&&(WSAGetLastError()==WSAEWOULDBLOCK)))
	{
		fd_set sockSet; timeval timeout = {m_timeout_sec,m_timeout_usec};
		FD_ZERO(&sockSet); FD_SET(m_sock, &sockSet); // Adds descriptor sock to set.
		fd_set errSet;
		FD_ZERO(&errSet); FD_SET(m_sock, &errSet); // Adds descriptor sock to set.
		
		retval = select(m_sock+1, NULL, &sockSet, &errSet, &timeout);
		if (retval == 0)
			return ERR_TIMEOUT;
		else if (retval == -1)
			return ERR_CONNECT;	

		if (FD_ISSET(m_sock, &errSet))
			return ERR_CONNECT;
	}
	else
	{
		return ERR_CONNECT;
	}
	noblock = 0;
	ioctlsocket(m_sock, FIONBIO, &noblock);

	m_connected = TRUE;
	return 0;
}

int Sock::Write(const char *buf, int len)
{
	return _Write(buf, len);
}

int Sock::_Write(const char *buf, int len)
{
	fd_set sockSet; timeval timeout = {m_timeout_sec,m_timeout_usec};
	FD_ZERO(&sockSet); FD_SET(m_sock, &sockSet); // Adds descriptor sock to set.
	
	int so_far = 0;
	while (so_far < len) {
		if (select(m_sock+1, NULL, &sockSet, NULL, &timeout) == 0)
			return ERR_TIMEOUT;
		int rval = send(m_sock, buf, len, 0);
		if (rval == SOCKET_ERROR)
			return rval;
		so_far += rval;
	}
	return so_far;
}

int Sock::_Read(char *buf, int len)
{
	fd_set sockSet; timeval timeout = {m_timeout_sec,m_timeout_usec};
	FD_ZERO(&sockSet); FD_SET(m_sock, &sockSet); // Adds descriptor sock to set.
	if (!select(m_sock+1, &sockSet, NULL, NULL, &timeout))
		return ERR_TIMEOUT;
	return recv(m_sock, buf, len, 0);
}

void Sock::SetBufSize(long size)
{
	if (m_buf_size != size)
	{
		void * newp = realloc(m_buf, size);
		if (newp)
		{
			m_buf_size = size;
			m_buf = (char *) newp;
		}
	}
}

int Sock::Write(int len)
{
	return this->_Write(m_buf, len);
}

int Sock::Read(int req_len)
{
	int recv_len;
	if ((recv_len = sock_buf_bytes_left()) > 0)
	{
		if (req_len < recv_len) recv_len = req_len;
		memmove(m_buf, m_eol_p, recv_len);
		m_eol_p += recv_len;
		return recv_len;
	}
	else
	{
		return _Read(m_buf, req_len);
	}
}

int Sock::ReadWait(float secs)
{
	if (secs > 0) {
		timeval timeout = {(long) secs, (long) (1000.0 * (secs - (float) ((long) secs)))};
		fd_set sockSet;
		FD_ZERO(&sockSet); FD_SET(m_sock, &sockSet); // adds descriptor sock to set.
		if (!select(m_sock+1, &sockSet, NULL, NULL, &timeout))
		return ERR_TIMEOUT;
	}

	u_long cb; int r;
	if (!(r = ioctlsocket(m_sock, FIONREAD, &cb)))
		return cb;
	else
		return ERR_IOCTL;
}

int Sock::ReadLine(char **line)
{
	char *line_p;
	int sock_rem_bytes = sock_buf_bytes_left();
	if (sock_rem_bytes <= 0 || !(line_p = (char *) memchr(m_eol_p, '\n', sock_rem_bytes)))
	{
		if (sock_rem_bytes > 0)
			memmove(m_buf, m_eol_p, sock_rem_bytes);
		else
			sock_rem_bytes = 0;

		m_eol_p = m_buf + sock_rem_bytes;

		if ((m_buf_bytes = _Read(m_eol_p, m_buf + m_buf_size - m_eol_p)) <= 0)
			return m_buf_bytes;

		if (!(line_p = (char *) memchr(m_eol_p, '\n', m_buf_bytes)))
		{
			*line = m_buf;
			line[m_buf_size] = '\0';
			m_eol_p = m_buf + m_buf_size;
			return m_buf_size;
		}			

		m_buf_bytes += sock_rem_bytes;
		m_eol_p = m_buf;
	}

	*line = m_eol_p; // save base

// move past crlf
	*line_p = '\0'; 
	m_eol_p = line_p + 1;
	if ((*(line_p - 1)) == '\r')
		*(--line_p) = '\0';


	return line_p - *line;
}

long Sock::Ignore(long len)
{
	long ignore_count = 0;
	int recv_len;

	if ((recv_len = sock_buf_bytes_left()) > 0)	{
		if (recv_len < len) {
			len -= recv_len;
			recv_len = 0;
		}
		else {
			len = 0;
			recv_len -= len;
		}
	}

	if (len == IGNORE_ALL)
	{
		while ((recv_len = _Read(m_buf, m_buf_size)) > 0)
			ignore_count += recv_len;
	}
	else
	{
		while (len > 0 && (recv_len = _Read(m_buf, len)) > 0)
		{
			len -= recv_len;
			ignore_count += recv_len;
		}
	}
	m_buf_bytes = 0;
	return ignore_count;
}
