/*
Copyright 1998-2005 Erik Aronesty. All rights reserved.

        1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

        2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

	3. Original source code, information and documentation is available at www.smxlang.org

THIS SOFTWARE IS PROVIDED 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE SMX AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

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
