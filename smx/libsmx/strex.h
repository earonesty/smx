// strex.h
CString operator +(CString left, int right);
CString operator +(CString left, long right);
CString operator +(CString left, float right);
CString operator +(CString left, double right);
CString ExpandEnviron(LPCTSTR lpSrc);
CStringArray& Array(const char *first,...);
char **strlist(const char *first,...);
