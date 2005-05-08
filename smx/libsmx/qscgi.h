class HEADER_ENT {
public:
	CStr Name; 
	CStr Value;
};

class qEnvCGI : public qEnvHttp 
{
	FILE *myIn;
	FILE *myOut;
	int   myBufSize;

	CStr myReply;
	int  myReplyCode;
	
	CLst<HEADER_ENT> *myHeaders;
	CMap<const char *, const char *, CStr *> myHeaderMap;

	qStrBuf myOutBuf;

public:
	qEnvCGI(FILE *in, FILE *out, int myBufSize);

	qStrBuf *GetBuf()
		{return &myOutBuf;}

	CStr GetHeader(const char *name);
	
	virtual int  GetHeaders(qEnvHttpHeaderCB *fHeaderCB) 
		{return 0;}


	CStr MapFullPath(const char *path);
	bool Flush();

	virtual bool SetReply(const char *reply)
	{
		myReply = reply; 
		myReplyCode = atoi(reply); 
		return true;
	}

	virtual bool AppendHeader(const char *name, const char *value)
	{
		HEADER_ENT e;
		e.Name = name; e.Value = value;
		myHeaders = myHeaders->Link(e);
		myHeaderMap.Set(myHeaders->Data().Name, &myHeaders->Data().Value);
		return true;
	}

	virtual bool SetHeader(const char *name, const char *value)
	{
		CStr *str; 
		if (myHeaderMap.Find(name,str)) 
			*str = value; 
		else 
			AppendHeader(name, value);
		return true;
	}

	const char *GetRequestURL()	    { return getenv("SCRIPT_NAME"); }
	const char *GetScriptPath()     { return getenv("PATH_TRANSLATED"); }
	const char *GetQueryString()	{ return getenv("QUERY_STRING"); }
	const char *GetContentType()	{ return getenv("CONTENT_TYPE"); }
	const char *GetRequestMethod()	{ return getenv("REQUEST_METHOD"); }

	int   GetContentLength() 		{ char *l; return (l = getenv("CONTENT_LENGTH")) ? atoi(l) : 0; }

	const char *GetServerSoftware()	{ return getenv("SERVER_SOFTWARE"); }
	const char *GetServerProtocol()	{ return getenv("SERVER_PROTOCOL"); }
	const char *GetServerName()		{ return getenv("SERVER_NAME"); }
	const char *GetServerAddr()		{ return getenv("SERVER_ADDR"); }
	int   GetServerPort() 			{ char *l; return (l = getenv("SERVER_PORT")) ? atoi(l) : 0; }

	const char *GetRemoteAddr()		{ return getenv("REMOTE_ADDR"); }
	const char *GetRemoteHost()		{ return getenv("REMOTE_HOST"); }

// streams
	char GetC()	 {return fgetc(myIn);}
	CStr GetS()  {CStr tmp(myBufSize); tmp.Grow(fread(tmp.GetBuffer(), 1, myBufSize, myIn)); return tmp;}

	void PutS(const char *s)		{fputs(s, myOut);}
	void PutS(const char *s, int n)	{fwrite(s, 1, n, myOut);}
	void PutC(char c)		     	{fputc(c, myOut);}
};
