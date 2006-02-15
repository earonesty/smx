/*
Copyright 1998-2005 Erik Aronesty. All rights reserved.

        1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

        2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

	3. Original source code, information and documentation is available at www.smxlang.org

THIS SOFTWARE IS PROVIDED 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE SMX AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

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

	strncpy(gEx.msg,  gExPrefix, QEX_MAXMSG);
	strncpy(gEx.msg + gExPrefixLen, msg, QEX_MAXMSG);

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
