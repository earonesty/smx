/* COPYRIGHT 1998 Prime Data Corp. 
	All Rights Reserved.  Use of this file without prior written 
	authorization is strictly prohibited.
*/

#ifndef _QENV_
#define _QENV_

// working environment information
// probably should use RTTI ... but cant get it
// to work consistently

#ifdef _DEBUG
	#define DEFAULT_MAX_LEVELS 500
#else
	#define DEFAULT_MAX_LEVELS 2000
#endif

#define ENV_BASE 0
#define ENV_HTTP 1
#define ENV_CGI  2
#define ENV_SMTP 3

class qEnv : public qStr {
protected:
	qCtx *myCtx;
	bool myFlushed;
	int myMaxLevels;
	
	virtual bool  DoFlush()					{return false;}

public:
	qEnv()	{
		myCtx = NULL; 
		myFlushed = false;
		myMaxLevels = DEFAULT_MAX_LEVELS; 
	}

	virtual ~qEnv()							{}

	// return NULL or a supported interface
	virtual qEnv *GetInterface(int envNum)  {return envNum == ENV_BASE ? this : NULL;}

	virtual char *GetEnvString(char *name)  {return name ? getenv(name) : 0;}
	virtual bool PutEnvString(char *value)  {return putenv(value)==0;}

	virtual void  SetSessionCtx(qCtx *ctx ) {assert(ctx!=NULL); myCtx = ctx;}
	virtual qCtx *GetSessionCtx()			{return myCtx;}

	virtual bool  EnableCache()				{return true;}

	bool Flush()							{return myFlushed |= DoFlush();}
	bool IsFlushed()						{return myFlushed;}

        virtual const char * GetName()                               {return "<unknown>";}

	int  GetMaxLevels()						{return this ? myMaxLevels : DEFAULT_MAX_LEVELS;}
	void SetMaxLevels(int maxLevels)		{if (this && maxLevels > 0) myMaxLevels = maxLevels;}
};

class qEnvHttpHeaderCB
{
public:
	virtual bool Callback(const char *name, const char *valu)
		{return false;}
};

class qEnvHttp : public qEnv
{

public:
	virtual qEnv *GetInterface(int envNum)  {return envNum == ENV_HTTP ? this : qEnv::GetInterface(envNum);}

// ** required overrides

	virtual bool AppendHeader(const char *name, const char *value)
		{return false;}

	virtual bool SetHeader(const char *name, const char *value)
		{return false;}

	virtual bool SetReplyCode(int reply)
		{return false;}

// turn a virtual into a phyical path
	virtual CStr MapFullPath(const char *path)
		{return path;}
	virtual CStr MapURL(const char *path)
		{return (const char *) NULL;}

// get info about http session
	virtual const char *GetScriptPath()    = 0; // full path to the script
	virtual const char *GetRequestURL()    = 0; // orginal request
	virtual const char *GetQueryString()   = 0; // stuff after the "?"
	virtual const char *GetRequestMethod() = 0;	// GET, POST, HEAD, PUT, etc.
	virtual const char *GetContentType()   = 0;
	virtual int         GetContentLength() = 0;
	virtual const char *GetProtocol()			// http or https
		{return CStr::Null;}

        virtual const char * GetName();

// get all headers sent by the agent
	virtual int  GetHeaders(qEnvHttpHeaderCB *fHeaderCB) = 0;


// get one header sent by the agent, default impl is to loop (may be inefficient)
	virtual CStr GetHeader(const char *name) = 0;

// ** reccomended overrides

// get mime based on extension 
	virtual CStr GetMimeType(const char *extension)	
		{return CStr::Null;}

// optional info
	virtual const char *GetServerSoftware()	// EG: Apache/1.34
		{return CStr::Null;}
	virtual const char *GetServerProtocol()	// EG: HTTP/1.1
		{return CStr::Null;}
	virtual const char *GetURLProtocol()	// EG: HTTP or HTTPS
		{return CStr::Null;}

	virtual const char *GetServerName()		// name of server
		{return CStr::Null;}
	virtual const char *GetServerAddr()		// ip of server
		{return CStr::Null;}
	virtual int   GetServerPort()			// port request came in on
		{return 0;}
	virtual const char *GetServerRoot()		// for posts & puts of data
		{return CStr::Null;}

	virtual const char *GetRemoteHost()		// name of client host
		{return CStr::Null;}
	virtual const char *GetRemoteAddr()		// ip of client host
		{return CStr::Null;}
	virtual const char *GetRemoteUser()		// user (if any)
		{return CStr::Null;}
	virtual void LogError(const char *err)		// user (if any)
		{}
};

#endif //#ifndef _QENV_
