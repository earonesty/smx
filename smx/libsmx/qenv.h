/*
Copyright 1998-2005 Erik Aronesty. All rights reserved.

        1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

        2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

	3. Original source code, information and documentation is available at www.smxlang.org

THIS SOFTWARE IS PROVIDED 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE SMX AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

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
	virtual bool PutEnvString(char *value)  {return putenv(strdup(value))==0;}

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
