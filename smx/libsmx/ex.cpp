/* COPYRIGHT 1998 Prime Data Corp. 
	All Rights Reserved.  Use of this file without prior written 
	authorization is strictly prohibited.
*/
#ifdef WIN32
	#include <windows.h>
#endif
#ifdef unix
	#include "unix.h"
#endif

#include <stdarg.h>

#include "ex.h"

TLS CEx gEx = {0,""};

char   gExPrefix[] = QEX_PREFIX;
int    gExPrefixLen = strlen(gExPrefix);

CEx *qEx(int id, char *msg, ...)
{
	va_list vargs;
	va_start( vargs, msg );
	return qExV(id, msg, vargs);
}

CEx *qExF(int id, char *msg, ...)
{
	va_list vargs;
	va_start( vargs, msg );

	strcpy(gEx.msg,  gExPrefix);
	strcpy(gEx.msg + gExPrefixLen, msg);

	return qExV(id, gEx.msg, ((char *)vargs)-sizeof(int));
}

CEx *qExRc(int id, ...)
{
	va_list vargs;
	va_start( vargs, id );

	char *data;
	long len = resLoad(ghRes, id, &data);

	if (len <= 0)
		strcpy(gEx.msg, "Unknown error.");
	else {
		strncpy(gEx.msg, data, QEX_MAXMSG);
		gEx.msg[QEX_MAXMSG-1] = 0;
	}

	return qExV(id, gEx.msg, vargs);
}

CEx *qExRcF(int id, ...)
{
	va_list vargs;
	va_start( vargs, id );

	strcpy(gEx.msg,  gExPrefix);

	char *data;
	long len = resLoad(ghRes, id, &data);

	if (len <= 0) {
		strcpy(gEx.msg + gExPrefixLen, "Unknown error.");
	} else {
		strncpy(gEx.msg + gExPrefixLen, data, QEX_MAXMSG - gExPrefixLen);
		gEx.msg[QEX_MAXMSG - gExPrefixLen - 1] = 0;
	}
	return qExV(id, gEx.msg, ((char *)vargs)-sizeof(int));
}

CEx *qExV(int id, char *msg_p, va_list vargs)
{
	int len = strlen(msg_p);
	gEx.id = id;

	char msg[QEX_MAXMSG+1];
	strcpy(msg, msg_p);

// support for the "%y" format flag
	int   s;
	char *pm = msg;
	char *pa = ((char *)vargs);
	char *py = 0;
	char *pmy = 0;
	DWORD dy = 0;

	while (*pm) {
		if (*pm == '%') {
			++pm;
			if (!py && (*pm == 'y')) {
				pmy = pm-1;
				py = pa;
				dy = *((DWORD *) py);
				pa += sizeof(DWORD);
			} else {
				while (*pm && !isalpha(*pm) && (*pm != 'L') && (*pm != 'h') && (*pm != 'I')) ++ pm;
				s = 0;
				switch (*pm) {
					case 'c': 
						s = sizeof(char); break;
					case 'C': 
						s = sizeof(wint_t); break;
					case 'd':case 'i':case 'o':case 'u':case 'x':case 'X':
						s = sizeof(int); break;
					case 'e':case 'E':case 'f':case 'g':case 'G':
						s = sizeof(double); break;
					case 'n':case 'p':case 'S':case 's': 
						s = sizeof(void *); break;
				}
				if (py) {
					memcpy(py, pa, s);
					py += s;
				}
				pa += s;
			}
		}
		pm++;
	}

	if (pmy) {
		int add, off = pmy - msg;

		memcpy(gEx.msg, msg, off);

		add = 0;

#ifdef WIN32
		add = FormatMessage(
			FORMAT_MESSAGE_FROM_SYSTEM, 0, dy, 0, &(gEx.msg[off]), 1024, 0);
#endif

		memcpy(&(gEx.msg[add+off]), pmy+2, len-off-2);
		len = add + len - 2;
		if (len > 0 && gEx.msg[len-1] == '\n') --len;
		if (len > 0 && gEx.msg[len-1] == '\r') --len;
		gEx.msg[len] = '\0';
		strcpy(msg, gEx.msg);
	}
// end support for the "%y" format flag

	vsprintf(gEx.msg, msg, vargs);

	va_end( vargs );
	return &gEx;
}


CEx *qExLast()
{
	return &gEx;
}
