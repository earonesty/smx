/* COPYRIGHT 1998 Prime Data Corp. 
	All Rights Reserved.  Use of this file without prior written 
	authorization is strictly prohibited.
*/
#include <stdio.h>
#include <stdarg.h>

#include "qlib.h"
#include "str.h"
#include "res.h"
#include "rcex.h"

// global error helpers
CRCException m_ex;
CRCException *RCError(int errNumber, ...)
{
	va_list marker;
	va_start( marker, errNumber );
	m_ex.SetErrorV(errNumber, (void *) marker);
	va_end( marker ); 
	return &m_ex;
}

CRCException *GetLastRCError()
{
	return &m_ex;
}

// constructors
CRCException::CRCException()
{
	m_cause = 0;
}

CRCException::CRCException(int errNumber, ...)
{
	va_list marker;
	va_start( marker, errNumber );
	SetErrorV(errNumber, (void *) marker);
	va_end( marker ); 
}

CRCException::CRCException(CRCException &ex) 
{
	m_cause = ex.m_cause;
	m_desc = ex.m_desc;
}

// methods
void CRCException::SetError(int errNumber, ...)
{
	va_list marker;
	va_start( marker, errNumber );
	SetErrorV(errNumber, (void *) marker);
	va_end( marker ); 
}

void CRCException::PushError(int errNumber, ...)
{
	CRCException temp;
	va_list marker;
	va_start( marker, errNumber );
	temp.SetErrorV(errNumber, (void *) marker);
	va_end( marker );
	m_desc = CStr(temp.m_desc) << m_desc;
}

void CRCException::SetErrorV(int errNumber, void *vargs)
{
	CStr errDesc;
	va_list marker = (va_list) vargs;
	 
	try
	{
		char *resString;
		if (resLoad(ghRes, errNumber, &resString) < 0)
			resString = "Unknown Error.";

		errDesc = CStr("ERROR #") << (int) errNumber << ": " << resString << "\n";

		m_desc.Grow(
			vsprintf(m_desc.Grow(errDesc.Length()+1024).GetBuffer(), errDesc, marker)
		);
	}
	catch(...)
	{
		errDesc = "Error reporting error.";
		errDesc = CStr("ERROR #") << (int) errNumber << ": " << errDesc << '\n';
	}
	m_cause = errNumber;
}
