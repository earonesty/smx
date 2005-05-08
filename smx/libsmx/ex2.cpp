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

#include "ex2.h"

char   gEx2Prefix[] = QEX_PREFIX;
int    gEx2PrefixLen = strlen(gEx2Prefix);

CEx2 &CEx2::Fmt(int ID, const char *Msg, ...)
{
	va_list vargs;
	va_start( vargs, ID );
	return FmtV(ID, Msg, vargs);
}

CEx2 &CEx2::FmtRc(int ID, ...)
{
	va_list vargs;
	va_start( vargs, ID );

	Msg.Grow(QEX_MAXMSG);

	char data[QEX_MAXMSG];
	long len = resLoad(ghRes, ID, data, QEX_MAXMSG);

	if (len <= 0)
		Msg = "Unknown error.";
	else {
		Msg = CStr(data, len);
	}

	return FmtV(ID, Msg, vargs);
}

CEx2 &CEx2::FmtRcF(int ID, ...)
{
	va_list vargs;
	va_start( vargs, ID );

	Msg=gEx2Prefix;

	Msg.Grow(QEX_MAXMSG);
	
	char data[QEX_MAXMSG];
	long len = resLoad(ghRes, ID, data, QEX_MAXMSG);

	if (len <= 0) {
		strcpy(Msg + gEx2PrefixLen, "Unknown error.");
	} else {
		strncpy(Msg + gEx2PrefixLen, data, QEX_MAXMSG - gEx2PrefixLen);
		Msg[QEX_MAXMSG - gEx2PrefixLen - 1] = 0;
	}

	Msg.Shrink();

	return FmtV(ID, Msg, ((char *)vargs)-sizeof(int));
}


CEx2 &CEx2::FmtF(int ID, const char *m, ...)
{
	va_list vargs;
	va_start( vargs, ID );

	Msg<<gEx2Prefix<<m;
	return FmtV(ID, Msg, ((char *)vargs)-sizeof(int));
}


CEx2 &CEx2::FmtV(int ID, const char *msg_p, va_list vargs)
{
	int len = strlen(msg_p);
	this->ID = ID;

	CStr tmp(QEX_MAXMSG);
	Msg.Grow(QEX_MAXMSG);
	strcpy(tmp.GetBuffer(), msg_p);

// support for the "%y" format flag
	int   s;
	char *pm = tmp.GetBuffer();
	char *pa = (char *) vargs;
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
		int add, off = (pmy - (const char *)tmp);
		memcpy(Msg.GetBuffer(), tmp, off);

		add = 0;

#ifdef WIN32
		add = FormatMessage(
			FORMAT_MESSAGE_FROM_SYSTEM, 0, dy, 0, &(Msg[off]), 1024, 0);
#else
    strerror_r(dy, &(Msg[off]), 1024);
#endif

		memcpy(&(Msg[add+off]), pmy+2, len-off-2);
		len = add + len - 2;
		if (len > 0 && Msg[len-1] == '\n') --len;
		if (len > 0 && Msg[len-1] == '\r') --len;
		Msg[len] = '\0';
		strcpy(tmp.GetBuffer(), Msg);
	}
// end support for the "%y" format flag

	Msg.Grow(vsprintf(Msg.GetBuffer(), tmp, vargs));

	va_end( vargs );

	return *this;
}
