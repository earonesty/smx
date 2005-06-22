/*
Copyright 1998-2005 Erik Aronesty. All rights reserved.

        1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

        2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

	3. Original source code, information and documentation is available at www.smxlang.org

THIS SOFTWARE IS PROVIDED 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE SMX AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

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
	CMap<const char *, CStr *>myHeaderMap;

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

	void PutS(const char *s)		{if(s) fputs(s, myOut);}
	void PutS(const char *s, int n)	{if (s) fwrite(s, 1, n, myOut);}
	void PutC(char c)		     	{fputc(c, myOut);}
};
