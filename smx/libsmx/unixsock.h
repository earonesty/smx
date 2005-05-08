/* COPYRIGHT 1998 Prime Data Corp. 
	All Rights Reserved.  Use of this file without prior written 
	authorization is strictly prohibited.
*/
#ifdef unix
#ifndef unixsock_h
#define unixsock_h
#include "unix.h"

// socket support
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#define SOCKET int
#define SOCKET_ERROR            (-1)
#define WSAGetLastError() (errno)
#define WSAEWOULDBLOCK EINPROGRESS
#define closesocket close
#define ioctlsocket ioctl

#endif //ifndef unixsock_h
#endif //ifdef unix
