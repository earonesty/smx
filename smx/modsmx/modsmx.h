#ifndef MOD_PSX_H
#define MOD_PSX_H 

#define NEED_CHUNKS

#if !defined(APACHE2) & defined(AP_MODULE_DECLARE_DATA)
#define APACHE2
#endif

#ifdef APACHE2
#define MODULE_VAR_EXPORT AP_MODULE_DECLARE_DATA
#define ap_table_get apr_table_get
#define ap_table_add apr_table_add
#define ap_table_set apr_table_set
#define ap_table_setn apr_table_setn
#define ap_table_do apr_table_do
#define ap_pstrdup apr_pstrdup
#define ap_send_http_header apr_send_http_header
#define ap_pool apr_pool_t
#define ap_destroy_pool apr_pool_destroy
#define ap_pstrcat apr_pstrcat
#define ap_inline APR_INLINE
#define ap_uudecode ap_pbase64decode
#define ap_uuencode ap_pbase64encode
#define AP2STATUS 0,
#else
#define AP2STATUS
#ifndef AP_IOBUFSIZE
#define AP_IOBUFSIZE IOBUFSIZE
#endif
#endif

#ifdef __cplusplus
	extern "C" module MODULE_VAR_EXPORT smx_module;
#else
	module MODULE_VAR_EXPORT smx_module;
#endif

class qStrApacheOut;
class qEnvApache;
class qEnvApacheServer;

class qStrApacheOut : public qStr
{
	request_rec * myReq;
public:
	void SetRequest(request_rec *req) {myReq = req;}
	void PutS(const char *s)		  {ap_rputs(s, myReq);}
	void PutS(const char *s, int n)	  {ap_rwrite(s, n, myReq);}
	void PutC(char c)			      {ap_rputc(c, myReq);}
};

class qEnvApache : public qEnvHttp
{
	request_rec * myReq;

	qStrBuf		  myStrBuf;
	qStrApacheOut myApacheOut;
	qStr         *myCurStr;

	int           myRefs;
	CStr myServerAddr;

	bool          mySetupChunkThing : 1;
	bool          myCtxFree : 1;
	unsigned int  myBufSize : 30;
	CStr          myBuf;

public:
	int			  IsAuth;

	qEnvApache(request_rec *req);
	~qEnvApache();

	qEnvApache *MainEnv() {
		return (qEnvApache *) (
			(myReq->main) ? 
			ap_get_module_config(myReq->main->request_config, &smx_module) : 
			NULL);
	}

	qEnvApache *PrevEnv() {
		return (qEnvApache *) (
			(myReq->prev) ? 
			ap_get_module_config(myReq->prev->request_config, &smx_module) : 
			NULL);
	}

	qCtx *GetCtx() {
		return myCtx;
	}
	void AddRef() {
		++myRefs;
	}
	void Free();
	void Done();

	void SetRequest(request_rec *req) {
		myReq = req;
	}

	request_rec *GetRequest() {
		return myReq;
	}

	CStr GetHeader(const char *name) {
		return ap_table_get(myReq->headers_in,name);
	}

// qEnvHttp overrides
	int GetHeaders(qEnvHttpHeaderCB *fHeaderCB);

	CStr MapFullPath(const char *path);
	CStr MapURL(const char *url);

	bool AppendHeader(const char *name, const char *value);
	bool SetHeader(const char *name, const char *value);
	bool SetReplyCode(int reply);
	CStr GetMimeType(const char *ext);
	

	bool DoFlush()
	{
		if (!IsFlushed()) {
			ap_table_set(myReq->headers_out, "Connection", "close");
			FinishReq(true);
			myApacheOut.SetRequest(myReq);
			myCurStr = &myApacheOut;
		}
		ap_rflush(myReq);
		return true;
	}

	// return true if you already sent the headers
	int FinishReq(bool flush) 
	{
		if (IsFlushed()) {
			ap_discard_request_body(myReq);
			return OK;
		}

		if (myReq->status >= 300 && myReq->status < 400) {
/*			const char *loc = ap_table_get(myReq->headers_out, "Location");
			if (!strchr(loc,':')) {
				request_rec *rr = ap_sub_req_lookup_uri(loc, myReq);
				if (rr) {
					int rval = ap_run_sub_req(rr);
					ap_log_transaction(rr);
					return rval;
				}
			}		*/
			int code = myReq->status;
			myReq->status = HTTP_OK;
			return code;
		} else {
			if (!myReq->content_type || !*myReq->content_type) {
				myReq->content_type = ap_pstrdup(myReq->pool, "text/html");
			}

			if (!flush) {
				myReq->clength = myStrBuf.Length();
				char buf[48];
				_itoa((int) myReq->clength, buf, 10);
				ap_table_set(myReq->headers_out, "Content-Length", buf);
			}

#ifndef APACHE2
			ap_send_http_header(myReq);
#endif

			if (!mySetupChunkThing)
				ap_discard_request_body(myReq);
			else {
				while (ap_get_client_block(myReq, myBuf.GetBuffer(), myBufSize) > 0) 
					continue;
			}

			if (myReq->header_only)
				return true;

			ap_rwrite(myStrBuf, myStrBuf.Length(), myReq);

			return OK;
		}
	}
	
	const char *GetScriptPath()		{return myReq->filename;}

	const char *GetRequestURL()	    {return myReq->unparsed_uri;}
	const char *GetQueryString()	{return myReq->args;}
	const char *GetContentType()	{return ap_table_get(myReq->headers_in,"Content-Type");}
	const char *GetRequestMethod()	{
		return myReq->method;
	}
	int         GetContentLength()	{return IsFlushed() ? ((int) myReq->bytes_sent) : myStrBuf.Length();}


	const char *GetServerName()		{return ap_get_server_name(myReq);}
	int   GetServerPort()			{return ap_get_server_port(myReq);}
	const char *GetServerSoftware()	{return ap_get_server_version();}
	const char *GetServerProtocol()	{return myReq->protocol;}
	const char *GetServerAddr();

	const char *GetRemoteAddr()		{return myReq->connection->remote_ip;}
#ifdef APACHE2
	const char *GetRemoteHost()		{return (char *) ap_get_remote_host(myReq->connection, myReq->per_dir_config, REMOTE_NAME, NULL);}
#else
	const char *GetRemoteHost()		{return (char *) ap_get_remote_host(myReq->connection, myReq->per_dir_config, REMOTE_NAME);}
#endif
#ifdef APACHE2
	const char *GetRemoteUser()		{return myReq->user ? myReq->user : "";}
#else
	const char *GetRemoteUser()		{return myReq->connection->user ? myReq->connection->user : "";}
#endif

public:

// Streams Override

	void PutS(const char *s)	    {myCurStr->PutS(s);}
	void PutS(const char *s, int n)	{myCurStr->PutS(s,n);}
	void PutC(char c)			    {myCurStr->PutC(c);}

protected:

// Read Streams
	bool SetupRead() {
		if (!mySetupChunkThing) {
			if (ap_setup_client_block(myReq, REQUEST_CHUNKED_ERROR))
				return false;
			else 
				mySetupChunkThing = true;
		}
		return true;
	}

	char GetC() {
		if (!SetupRead())
			return EOF;
		char c[2]; 
		return ap_get_client_block(myReq, c, 1) > 0 ? c[0] : EOF;
	}

	CStr GetS() {
		if (!SetupRead())
			return CStr::Null;
		int l;
		l = ap_get_client_block(myReq, myBuf.GetBuffer(), myBufSize);
		return l > 0 ? CStr((const char *) myBuf,l) : CStr::Null;

	}
};

class qEnvApacheServer : public qEnvHttp
{
protected:
	server_rec * myServer;
	bool         myReady;
	ap_pool    * myPool;
	qCtxTS     * myCtx;
	qStrCache  * myCache;	    /* string cache        */
    CStr         myVName;       /* virtual server name */

	bool         myNoMagic;     /* parse without %expand% ? */
	bool         myInInit;      /* don't cause recursion in initialization */
	CStr         myInitMacro;   /* initialization macro */
        CStr         myUserMacro;   /* user authentication macro */
	CStr         myBadMacro;    /* 404 macro */
	CStr         myWrapMacro;   /* contents of page passed to it */
	CStr         myPageMacro;   /* called before each page hit */
	
	CLst<qEnvApacheServer *> *myVirts;
	
public:


	qEnvApacheServer(qEnvApacheServer *parent) {
		myCtx = new qCtxTS(parent ? parent->myCtx : NULL);
		myCtx->SetEnv(this);
		SetSessionCtx(myCtx);
//		GetCtx()->MapObj(new qObjCtxP(GetCtx()), "server-context");

#ifdef APACHE2
    apr_pool_create_ex(&myPool,parent ? parent->GetPool() : NULL,NULL,NULL);
#else
		myPool = ap_make_sub_pool(parent ? parent->GetPool() : NULL);
#endif

		if (!myCtx->Find((qObj**)&myCache , "<cache>")) {
			assert(!parent);
			myCache  = new qStrCache(myCtx);
			myCtx->MapObj(myCache, "<cache>");
		}

		myServer = NULL;
		myReady  = false;
		myVirts  = NULL;
		myInInit = false;
	}

	bool EnableCache() {return false;}	// never parse forms, or cache modules in a reusable context
	
	qEnvApacheServer * NewServer() {
		qEnvApacheServer *virt = new qEnvApacheServer(this);
		myVirts = myVirts->Link(virt);
		return virt;
	}

	~qEnvApacheServer() {
		while (myVirts)
			myVirts = myVirts->DeleteTop();
		myCtx->Free();
		ap_destroy_pool(myPool);
	}

	void Initialize(request_rec *r);
	void ReInitialize();

// set conf
	void SetInitMacro(const char *macro)	{myInitMacro = macro;}
	void SetName(const char *name)			{myVName = name;}
	void SetServer(server_rec *s)			{myServer = s;}

// accessors
	const char *GetUserMacro()	{return myUserMacro;}
	const char *GetBadMacro()	{return myBadMacro;}
	const char *GetPageMacro()	{return myPageMacro;}
	const char *GetWrapMacro()	{return myWrapMacro;}
	bool        GetNoMagic()    	{return myNoMagic;}
	const char *GetName()		{return myVName;}
	ap_pool *	GetPool()	{return myPool;}


	qCtx *		GetCtx()		{return myCtx;}
	qStrCache *	GetCache()		{return myCache;}
	bool		IsReady()		{return myReady;}

// handle env-http 
	const char *GetRequestURL()		{return CStr::Null;}
	const char *GetQueryString()	{return CStr::Null;}
	const char *GetRequestMethod()	{return CStr::Null;}
	const char *GetContentType()	{return CStr::Null;}
	int         GetContentLength()	{return 0;}
    CStr        GetHeader(const char *name)  {return CStr::Null;}
	int         GetHeaders(qEnvHttpHeaderCB *fHeaderCB) {return 0;}



	const char *GetScriptPath()		{
		return myServer ?
				(myServer->defn_name ? 
				 myServer->defn_name : 
				 myServer->next->defn_name ? 
			     myServer->next->defn_name : 0) 
			    : 0;
	}

	const char *GetServerName()		{ 
			return myServer ? myServer->server_hostname : NULL; 
	}

	int         GetServerPort()		{ 
			return myServer ? myServer->port : 0; 
	}

	virtual CStr MapFullPath(const char *path);

  // possibly log output of init modules/scripts?
	void PutS(const char *s, int n)
    {}

};

#ifdef APACHE2
class qModApache
{
  public:
	qModApache();
	~qModApache();
};
#endif

#endif /*MOD_PSX_H*/


