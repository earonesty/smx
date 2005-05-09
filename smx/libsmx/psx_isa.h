/*
Copyright 1998-2005 Erik Aronesty. All rights reserved.

        1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

        2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

	3. Original source code, information and documentation is available at www.smxlang.org

THIS SOFTWARE IS PROVIDED 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE SMX AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#define MIN_BUF_SIZE 1024

#define HTTP_STATUS_OK              200     // OK
#define HTTP_STATUS_CREATED         201     // created
#define HTTP_STATUS_ACCEPTED        202     // accepted
#define HTTP_STATUS_NO_CONTENT      204     // no content
#define HTTP_STATUS_REDIRECT        301     // moved permanently
#define HTTP_STATUS_TEMP_REDIRECT   302     // moved temporarily
#define HTTP_STATUS_NOT_MODIFIED    304     // not modified
#define HTTP_STATUS_BAD_REQUEST     400     // bad request
#define HTTP_STATUS_AUTH_REQUIRED   401     // unauthorized
#define HTTP_STATUS_FORBIDDEN       403     // forbidden
#define HTTP_STATUS_NOT_FOUND       404     // not found
#define HTTP_STATUS_SERVER_ERROR    500     // internal server error
#define HTTP_STATUS_NOT_IMPLEMENTED 501     // not implemented
#define HTTP_STATUS_BAD_GATEWAY     502     // bad gateway
#define HTTP_STATUS_SERVICE_NA      503     // service unavailable

class qIISServer
{
	qCtxTS *myGlobalCtx;


	CStr myInitFile;
	bool myInitDone;
	
	CStr myUserRealm;
	CStr myUserMacro;
	CStr myDeniedMacro;

public:
	qIISServer();
	~qIISServer();

	void Init(qCtx *ctx);

	bool IsReady() {
		return myInitDone;
	}

	char *GetName() {return "qScript IIS Server Extension 1.0";}

	CStr GetUserDomain() {
		return myUserRealm;
	}

	CStr GetDeniedMacro() {
		return myDeniedMacro;
	}

	CStr GetUserMacro() {
		return myUserMacro;
	}

	qCtx *GetCtx()  {return myGlobalCtx;}
};

class qIISExtension
{
public:
	BOOL GetVersion(HSE_VERSION_INFO* pVer);
	DWORD HttpProc(EXTENSION_CONTROL_BLOCK* pCtxt);
	BOOL Terminate(DWORD dwFlags);
};

class qIISFilter
{
public:
	BOOL GetVersion(PHTTP_FILTER_VERSION pVer);
	DWORD FilterProc(PHTTP_FILTER_CONTEXT pfc, DWORD dwNotificationType, LPVOID pvNotification);
	BOOL  Terminate(DWORD dwFlags);
	DWORD OnAuthentication(PHTTP_FILTER_CONTEXT pfc, PHTTP_FILTER_AUTHENT pAuthent);
};


const char * PROTO_HTTPS = "https";
const char * PROTO_HTTP  = "http";

class qEnvIIS : public qEnvHttp
{
protected:
	qIISServer* myServer;

	int   myReply;
	const char *myReplyString;

	CStr myRawBuf;

	CStr myContentType;
	CStrAry myHeaderN;
	CStrAry myHeaderV;

public:
	qEnvIIS(qIISServer* ext) {
		myServer = ext; 
	}

// http env impl

	CStr GetMimeType(const char *pszPath);

	CStr GetHeader(const char *pszName);

	int GetHeaders(qEnvHttpHeaderCB *pCB);

	bool AppendHeader(const char *name, const char *value) {
		myHeaderN.Add(name);
		myHeaderV.Add(value);
		return true;
	}

	bool SetHeader(const char *name, const char *value) {
		int i;
		for (i = 0; i < myHeaderN.Count(); ++i) {
			if (!stricmp(myHeaderN[i],name)) {
				myHeaderV[i] = value;
				return true;
			}
		}
		return AppendHeader(name, value);
	}

	bool SetReplyCode(int reply) {
		myReply = reply;
		myReplyString = LookupReplyString(myReply);
		return true;
	}

	CStr MapFullPath(const char *path);

// mandatory overrides
	virtual CStr GetServerVariable(char *name) = 0;

	virtual void MapRootPath(char *path, DWORD &len) = 0;

// optional overrides

	virtual const char *GetRequestURL()		{return GetServerVariable("PATH_INFO");}
	virtual const char *GetScriptPath()		{return GetServerVariable("PATH_TRANSLATED");}

	virtual const char *GetQueryString()	{return GetServerVariable("QUERY_STRING");}
	virtual const char *GetContentType()	{return GetServerVariable("CONTENT_TYPE");}
	virtual const char *GetRequestMethod()	{return GetServerVariable("REQUEST_METHOD");}

	virtual int   GetContentLength()		{return atoi(GetServerVariable("CONTENT_LENGTH"));}

	const char *GetServerSoftware()	{return GetServerVariable("SERVER_SOFTWARE");}
	const char *GetServerProtocol()	{return GetServerVariable("SERVER_PROTOCOL");}
	const char *GetURLProtocol()	{
		const char *tmp = GetServerVariable("HTTPS");
		if (tmp && tolower(tmp[0]) == 'o' && tolower(tmp[1]) == 'n') {
			return PROTO_HTTPS;
		} else {
			return PROTO_HTTP;
		}
	}
	const char *GetServerName()		{return GetServerVariable("SERVER_NAME");}
	
	const char *GetServerAddr()		{
		return GetServerVariable("SERVER_NAME");
		//CStr name = GetServerVariable("SERVER_NAME");
		//if (!is_ip(name)) {
		//	//struct hostent * gethostbyname();
		//}
	}

	int   GetServerPort()			{return atoi(GetServerVariable("SERVER_PORT"));}

	const char *GetRemoteAddr()		{return GetServerVariable("REMOTE_ADDR");}
	const char *GetRemoteHost()		{return GetServerVariable("REMOTE_HOST");}
	const char *GetRemoteUser()		{return GetServerVariable("REMOTE_USER");}
};

class qStrIISExtOut : public qStr
{
	EXTENSION_CONTROL_BLOCK* myECB;
public:
	void SetECB(EXTENSION_CONTROL_BLOCK *ecb) {
		myECB = ecb;
	}

	void PutS(const CStr &s) {
		DWORD w=s.Length(); 
		BOOL ok = myECB->WriteClient(myECB->ConnID, s.Data(), &w, HSE_IO_SYNC);
	}

	void PutS(const char *s) {
		DWORD w=strlen(s); 
		myECB->WriteClient(myECB->ConnID, (void *) s, &w, HSE_IO_SYNC);
	}

	void PutS(const char *s, int n)	{
		myECB->WriteClient(myECB->ConnID, (void *) s, (DWORD*)&n, HSE_IO_SYNC);
	}

	void PutC(char c) {
		DWORD w=1; 
		myECB->WriteClient(myECB->ConnID, &c, &w, HSE_IO_SYNC);
	}
};

class qEnvIISExt : public qEnvIIS
{
	DWORD myBufSize;
	DWORD myRemBytes;
	DWORD myCOff;

	EXTENSION_CONTROL_BLOCK* myECB;

	CStr myFormBuf;
	CStr myTmp;
	qStr         * myCurStr; // points to one of the 2 streams below
	qStrBuf		   myStrBuf;
	qStrIISExtOut  myIISExtOut;

	bool SendHeader(const char *status, CStr head) {
		HSE_SEND_HEADER_EX_INFO exInfo;
		
		exInfo.pszStatus = status;
		exInfo.cchStatus = strlen(status);
		exInfo.pszHeader = head;
		exInfo.cchHeader = head.Length();
		exInfo.fKeepConn = 0;

		return 0 != myECB->ServerSupportFunction(
                myECB->ConnID
                , HSE_REQ_SEND_RESPONSE_HEADER_EX
                , &exInfo , NULL, NULL );
	}


public:
	qEnvIISExt(qIISServer *server, EXTENSION_CONTROL_BLOCK* ecb, int bufs) 
		: qEnvIIS(server) {

		myECB = ecb; 
		myIISExtOut.SetECB(ecb);

		myBufSize = bufs; 
		myCOff = 0; 
		myRemBytes = ecb->cbTotalBytes - ecb->cbAvailable;

		myCurStr = &myStrBuf;
	}

	DWORD HttpProc();

	bool DoFlush()
	{
		if (!IsFlushed()) {
			SetHeader("Connection", "close");
			FinishReq(true);
			myCurStr = &myIISExtOut;
		}
		return true;
	}

	int FinishReq(bool flush) 
	{
		if (IsFlushed())
			return TRUE;

		if (myReply == 401) {
			myStrBuf.Clear();
			myStrBuf.PutS(GetSessionCtx()->Eval(myServer->GetDeniedMacro()));
			myECB->dwHttpStatusCode = HTTP_STATUS_AUTH_REQUIRED;
			flush = true;
		}

		CStr head;

		int i; for(i=0;i<myHeaderN.Count();++i) {
			head << myHeaderN[i] << ':' << myHeaderV[i] << "\r\n";
		}

		if (!flush) {
			head << "Content-Length: ";
			head << myStrBuf.Length();
			head << "\r\n";
		}

		myReplyString = LookupReplyString(myReply);
		myECB->dwHttpStatusCode = myReply;
		head << "\r\n";
		BOOL ok = myECB->ServerSupportFunction(myECB->ConnID, HSE_REQ_SEND_RESPONSE_HEADER, (void *) myReplyString ,0,(DWORD*)(char *)head);

		myCurStr = &myIISExtOut;

		if (stricmp(GetRequestMethod(), "HEAD")) {
			try {
				PutS(myStrBuf);
			} catch (...) {
				PutS("Error writing to output stream!");
			}
		}

		return true;
	}

// http env impl overrides

	CStr GetServerVariable(char *name);

	void MapRootPath(char *path, DWORD &len) {
		myECB->ServerSupportFunction(myECB->ConnID, HSE_REQ_MAP_URL_TO_PATH,path,&len,0);
	}

	const char *GetRequestURL()		{return myECB->lpszPathInfo;}
	const char *GetScriptPath()		{return myECB->lpszPathTranslated;}

	const char *GetQueryString()	{return myECB->lpszQueryString;}
	const char *GetContentType()	{return myECB->lpszContentType;}
	const char *GetRequestMethod()	{return myECB->lpszMethod;}
	int   GetContentLength()		{return myECB->cbTotalBytes;}

// get/put streams
	char GetC();
	CStr GetS();
	void Fill();	// fills ECB

// write streams
	void PutS(const char *s)	    {myCurStr->PutS(s);}
	void PutS(const char *s, int n)	{myCurStr->PutS(s,n);}
	void PutC(char c)			    {myCurStr->PutC(c);}
};


class qEnvIISFilt : public qEnvIIS
{
	PHTTP_FILTER_CONTEXT myFC;

public:
	qEnvIISFilt(qIISServer* serv, PHTTP_FILTER_CONTEXT ecb)
		: qEnvIIS(serv)
	{
		myFC = ecb;
	}

	DWORD FilterProc();

// http env impl

	CStr GetServerVariable(char *name);

	void MapRootPath(char *path, DWORD &len) {
	}

// ideas:
		//SF_REQ_TYPE SF_REQ_GET_PROPERTY
		//SF_PROPERTY_INSTANCE_NUM_ID
		//GetVar...("INSTANCE_ID")
};
