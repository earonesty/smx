#ifndef _RCERR_H_
#define _RCERR_H_

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
	CString m_desc;
};

CRCException *RCError(int errNumber, ...);
CRCException * GetLastRCError();


#endif // #ifndef _RCERR_H_