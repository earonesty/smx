/* COPYRIGHT 1998 Prime Data Corp. 
	All Rights Reserved.  Duplication of this file without prior written 
	authorization is strictly prohibited.
*/
#ifndef _EX2_H_
#define _EX2_H_

#ifndef _INC_STDIO
	#include <stdio.h>
#endif

#ifndef _QLIB_H_
	#include "qlib.h"
#endif

#ifndef _STR_H
	#include "str.h"
#endif

#ifndef _RES_H
	#include "res.h"
#endif

#define QEX_PREFIX "ERROR #%d: "
#define QEX_MAXMSG 2048

class CEx2 {
public:
	int  ID;
	CStr Msg;

	CEx2() {ID = 0;}
	CEx2(const CEx2 &ex) {ID = ex.ID; Msg = ex.Msg;}
	CEx2(int ID, char *Msg, ...) {FmtF(ID, Msg);}

	CEx2 &Fmt (int ID, const char *Msg, ...);
	CEx2 &FmtF(int ID, const char *Msg, ...);
	CEx2 &FmtV(int ID, const char *Msg, va_list vargs);
	CEx2 &FmtRc(int ID, ...);
	CEx2 &FmtRcF(int ID, ...);

	CEx2 &Push(const CEx2 &ex) {
		Msg =  ex.Msg + "\n" + Msg; return *this;
	}
};

extern char    gEx2Prefix[];
extern int     gEx2PrefixLen;

#endif //#ifndef _EX_H_

/* CEx2 : just like CEx, but with ref counting (no thread-local needed):

	try {
		... 
		throw(qEx(4, "Error on port #%d. %y", port, GetLastError())) ;
		...
	} catch (CEx *ex) {
	... 
	}

	
	your catch handler then gets an id 
		and a nicely formatted message for printing

	the format string is standard printf 
	... with the addition of a "%y" code
		which takes a GetLastError() code and translates it to a 
		error description
	
	
	qExV is for building new wrapper Ex functions

	rcEx assumes the id is a resource-string-id and uses it 
		to grab a message from the resource fork
		windows resources, however, aren't very portable

	rcExF adds "ERROR %d : " to the fromt of the message

*/
