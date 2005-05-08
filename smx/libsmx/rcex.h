/* COPYRIGHT 1998 Prime Data Corp. 
	All Rights Reserved.  Duplication of this file without prior written 
	authorization is strictly prohibited.
*/
#ifndef _Q_RCEX
#define _Q_RCEX

class CRCException
{
public:
	CRCException();
	CRCException(int errNumber, ...);
	CRCException(CRCException &ex);
	void SetError(int errNumber, ...);
	void SetErrorV(int errNumber, void *argList);
	void PushError(int errNumber, ...);

	int m_cause;
	CStr m_desc;
};

CRCException *RCError(int errNumber, ...);
CRCException * GetLastRCError();
#endif
