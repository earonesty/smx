/*
Copyright 1998-2005 Erik Aronesty. All rights reserved.

        1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

        2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

	3. Original source code, information and documentation is available at www.smxlang.org

THIS SOFTWARE IS PROVIDED 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE SMX AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#include "stdafx.h"


#include "qstr.h"
#include "qobj-ctx.h"
#include "sock.h"
#include "qmail.h"
#include "util.h"
#include "ex.h"

class qObjProto : public qObj
{
public:
	qObjProto() {
#ifdef WIN32
		qinit();
#endif
	}
	~qObjProto() {
#ifdef WIN32
		qterm();
#endif
	}
#ifdef WIN32
	void EvalQMail(qCtx *ctx, qStr *out, qArgAry *args);
#endif

	void EvalSmtpMail(qCtx *ctx, qStr *out, qArgAry *args);

	void EvalWhois(qCtx *ctx, qStr *out, qArgAry *args);
	void EvalRWhois(qCtx *ctx, qStr *out, qArgAry *args);
	void EvalTCPConn(qCtx *ctx, qStr *out, qArgAry *args);
	void EvalXTCP(qCtx *ctx, qStr *out, qArgAry *args);
};

void qObjProto::EvalSmtpMail(qCtx *ctx, qStr *out, qArgAry *args)
{
	if (args->Count() >= 4) {
		qMailOpts qmop;

		qmop.host = (*args)[0];
		qmop.smtp = qmop.host;
		qmop.user = (*args)[1].GetBuffer();
		qmop.from = (*args)[2];
		qmop.subj = (*args)[4];

		qmop.subj = ReplaceStr(qmop.subj, "\n", "");
		qmop.subj = ReplaceStr(qmop.subj, "\r", "");
		qmop.from = ReplaceStr(qmop.from, "\n", "");
		qmop.from = ReplaceStr(qmop.from, "\r", "");
		qmop.host = ReplaceStr(qmop.host, "\n", "");
		qmop.host = ReplaceStr(qmop.host, "\r", "");

		qmop.body.Add("'" << (*args)[5]);

		if (qsUnreg)
			qmop.unreg = true;

		int i = 6;
		while (args->GetAt(i)) {
			qmop.body.Add((*args)[i++]);
		}

		// FIX QMAIL LACK OF MULTIPLE -TO- HEADERS!

		CStr rcpt = (*args)[3];
		ReplaceStr(rcpt, "\n", "");
		ReplaceStr(rcpt, "\r", "");

		char * rx = rcpt.GetBuffer();
		char * tok = rx;
		char * p   = strchr(tok, ';'); if (p) *p = '\0';
		while (tok) {
			while (isspace(*tok))
				++tok;
			if (*tok) {
				qmop.rcpt = tok;
				qmop.to = tok;

				try {
					int errVal = qsmtp(&qmop);
					if (errVal)
						ctx->ThrowF(out, errVal+600, "Mail error #%d.", errVal);
				} catch (CEx ex) {
					ctx->Throw(out, ex.id+600, ex.msg); 
				}
			}
			if (p) {
				tok = p + 1;
				p   = strchr(tok, ';');
				if (p) *p = '\0';
			} else 
				tok = NULL;
		}
	} else {
		ctx->Throw(out, 655, "USAGE: %smtp-mail(host, user, from, to, subj, body...)"); 
	}

	return;
}

#ifdef WIN32
void qObjProto::EvalQMail(qCtx *ctx, qStr *out, qArgAry *args) 
{
	if (args->Count() >= 1) {
		qMailOpts qmop;

		int argc = args->Count();
		const char **argv = *args;

		try {
			parseopts(&qmop, argc, const_cast<char **&>(argv));
		} catch (CEx ex) {
			ctx->Throw(out, ex.id+600, ex.msg);
		}
		
		if (qsUnreg) {
			qmop.unreg = true;
		}

		try {
			int errVal = qsmtp(&qmop);
			if (errVal)
				ctx->ThrowF(out, errVal+600, "Mail error #%d.", errVal);
		} catch (CEx ex) {
			ctx->Throw(out, ex.id+600, ex.msg); 
		}
	}
	return;
}
#endif

#define DEFAULT_WHOIS_HOST     "whois.internic.net"
#define DEFAULT_WHOIS_IP_HOST  "whois.arin.net"
#define DEFAULT_RWHOIS_HOST    "rwhois.internic.net"
#define IPPORT_RWHOIS          4321

#define PROTO_READ_LINE() {\
	if ((sock_rval = sock.ReadLine(&line)) < 0) {\
		if (sock_rval == Sock::ERR_TIMEOUT)\
			ctx->ThrowF(out, 705, "Time out while reading from host %s:%d, %y", (const char *) serv, port, GetLastError());\
		else if (sock_rval)\
			ctx->ThrowF(out, 706, "Error reading from host %s:%d, %y", (const char *) serv, port, GetLastError());\
		return;\
	}\
}

#define PROTO_OPEN_SOCK() {\
	sock_rval = sock.Open(serv, port);\
	if (sock_rval) {\
		if (sock_rval == Sock::ERR_GETHOST)\
			ctx->ThrowF(out, 701, "Error resolving hostname %s:%d, %y", (const char *) serv, port, GetLastError());\
		else if (sock_rval == Sock::ERR_CONNECT)\
			ctx->ThrowF(out, 702, "Error connecting to host %s:%d, %y", (const char *) serv, port, GetLastError());\
		else if (sock_rval == Sock::ERR_TIMEOUT)\
			ctx->ThrowF(out, 703, "Error connecting to host %s:%d, %y", (const char *) serv, port, GetLastError());\
		else\
			ctx->ThrowF(out, 704, "Error connecting to host %s:%d, %y", (const char *) serv, port, GetLastError());\
		return;\
	}\
}

#define PROTO_OPEN_SOCK2(sock) {\
	sock_rval = sock.Open(serv, port);\
	if (sock_rval) {\
		if (sock_rval == Sock::ERR_GETHOST)\
			ctx->ThrowF(out, 701, "Error resolving hostname %s:%d, %y", (const char *) serv, port, GetLastError());\
		else if (sock_rval == Sock::ERR_CONNECT)\
			ctx->ThrowF(out, 702, "Error connecting to host %s:%d, %y", (const char *) serv, port, GetLastError());\
		else if (sock_rval == Sock::ERR_TIMEOUT)\
			ctx->ThrowF(out, 703, "Error connecting to host %s:%d, %y", (const char *) serv, port, GetLastError());\
		else\
			ctx->ThrowF(out, 704, "Error connecting to host %s:%d, %y", (const char *) serv, port, GetLastError());\
		return;\
	}\
}


void ProtoParseHosts(CStrAry &hosts, CStr &hostStr) {
	char *p = hostStr.GetBuffer();
	p = strtok(p, ";\n");
	while (p) {
		if (*p && strchr(p, '.')) {
			hosts.Add(p);
		}
		p = strtok(NULL, ";\n");
	}
}

char *stristr(char *b, char *s) {
	if (!*s)
		return b;
	if (!s[1])
		return strchr(b, *s);
	if (!*b || !b[1])
		return NULL;

	int l = strlen(s) - 1;
	while (*b) {
		if ((tolower(*b) == tolower(*s))
		 && (!strnicmp(b+1,s+1,l))
		 )
			return b;
		++b;
	}
	return NULL;
}


void EvalXTCPSend(const void *obj, qCtx *ctx, qStr *out, qArgAry *args)
{
	Sock *s = (Sock *) obj;
	CStr str = (*args)[0];
	int len = s->Write(str, str.Length());
	if (len < 0 ) {
		if (len == Sock::ERR_TIMEOUT) {
			ctx->ThrowF(out, 702, "Timeout while waiting to write data from host %s:%d, %y", s->GetHost(), s->GetPort());
		} else {
			ctx->ThrowF(out, 702, "Error while writing data from host %s:%d, %y", s->GetHost(), s->GetPort(), GetLastError());
		}
	}
}

void EvalXTCPRecv(const void *obj, qCtx *ctx, qStr *out, qArgAry *args)
{
	Sock *s = (Sock *) obj;

	int len;
	char *line = NULL;
	if (args->Count() > 0)  {
		len = ParseInt((*args)[0]);
		len = s->Read(len);
		line = s->GetBuf();
	} else {
		len = s->ReadLine(&line);
	}

	if (len > 0)
		out->PutS(line, len);
	else if (len < 0 ) {
		if (len == Sock::ERR_TIMEOUT) {
			ctx->ThrowF(out, 702, "Timeout while waiting to read data from host %s:%d, %y", s->GetHost(), s->GetPort());
		} else {
			ctx->ThrowF(out, 702, "Error while reading data from host %s:%d, %y", s->GetHost(), s->GetPort(), GetLastError());
		}
	}

}

class qObjTcp : public qObjCtxP {

	void EvalTCPRecv(qCtx *ctx, qStr *out, qArgAry *args)
	{
		int len;
		char *line = NULL;
		CStr lcmd = (*args)[0];
		if (!lcmd.IsEmpty())  {
			if ((lcmd[0] = '*')) {
				len = sock.Read(SOCK_DEFAULT_BUF_SIZE);
				while (len > 0) {
					out->PutS(sock.GetBuf(), len);
					len = sock.Read(SOCK_DEFAULT_BUF_SIZE);
				}
			} else {
				int max = atoi(lcmd);
				int sofar = 0;
				while (len > 0 && sofar < max) {
					sofar += (len = sock.Read(min(SOCK_DEFAULT_BUF_SIZE,max)));
					if (len)
						out->PutS(sock.GetBuf(), len);
				}
			}
		} else {
			len = sock.ReadLine(&line);
			if (len > 0)
				out->PutS(line, len);
		}

		if (len < 0 ) {
			if (len == Sock::ERR_TIMEOUT) {
				ctx->ThrowF(out, 702, "Timeout while waiting to read data from host %s:%d, %y", sock.GetHost(), sock.GetPort());
			} else {
				ctx->ThrowF(out, 702, "Error while reading data from host %s:%d, %y", sock.GetHost(), sock.GetPort(), GetLastError());
			}
		}
	}

	void EvalTCPSend(qCtx *ctx, qStr *out, qArgAry *args)
	{
		CStr str = (*args)[0];
		int len = sock.Write(str, str.Length());
		if (len < 0 ) {
			if (len == Sock::ERR_TIMEOUT) {
				ctx->ThrowF(out, 702, "Timeout while waiting to write data from host %sock:%d, %y", sock.GetHost(), sock.GetPort());
			} else {
				ctx->ThrowF(out, 702, "Error while writing data from host %sock:%d, %y", sock.GetHost(), sock.GetPort(), GetLastError());
			}
		}
	}

	void EvalTCPClose(qCtx *ctx, qStr *out, qArgAry *args)
	{
		sock.Close();
	}


	void Eval(qCtx *ctx, qStr *out, qArgAry *args) 
	{
		if (myCtx) {
			myCtx->SetEnv(ctx->GetEnv());
			myCtx->SetParent(ctx);
			myCtx->Parse(args->GetAt(0), out); 
		}
	}

	char *GetQmap() {
		static char qm[2] = "1";
		return qm;
	}
public:
	qObjTcp();
	Sock sock;
};

qObjTcp::qObjTcp() : qObjCtxP(new qCtxRef(NULL), true) {
	myCtx->MapObj(this, (QOBJMETH) (&qObjTcp::EvalTCPSend),"send");
	myCtx->MapObj(this, (QOBJMETH) (&qObjTcp::EvalTCPRecv),"recv");
	myCtx->MapObj(this, (QOBJMETH) (&qObjTcp::EvalTCPClose),"close");
}



void qObjProto::EvalTCPConn(qCtx *ctx, qStr *out, qArgAry *args) 
{
	if (args->Count() < 1) {
		ctx->Throw(out, 655, "USAGE: %tcp(name,host[,timeout]])");
		return;
	}


	CStr name = (*args)[0];

	CStr serv = (*args)[1];

	double timeout = ParseDbl((*args)[2]);

	if (serv.IsEmpty()) {
		ctx->Throw(out, 656, "%tcp : host unspecified");
		return;
	}
	if (name.IsEmpty()) {
		ctx->Throw(out, 656, "%tcp : connection name unspecified");
		return;
	}

	int port = 0;

	char * p = strchr((const char *)serv, ':');

	if (p) {
		port = atoi(p+1);
		*p = '\0';
		++p;

	}

	if (!port)
		port = IPPORT_TELNET;

	qObjTcp *conn = new qObjTcp();

	int sock_rval;

	PROTO_OPEN_SOCK2(conn->sock);

	if (timeout > 1)
		conn->sock.SetTimeout((float) timeout);

	ctx->MapObjLet(conn, name);

	return;
}

void qObjProto::EvalXTCP(qCtx *ctx, qStr *out, qArgAry *args) 
{
	if (args->Count() < 1) {
		ctx->Throw(out, 655, "USAGE: %connect-tcp(host,body[,timeout]])"); 
		return;
	}

	CStrAry hosts;

	CStr serv = (*args)[0];

	CStr bodyStr = args->GetAt(1);

	double timeout = ParseDbl((*args)[2]);

	if (serv.IsEmpty())
		return;

	int port = 0;

	char * p = strchr((const char *)serv, ':');

	if (p) {
		port = atoi(p+1);
		*p = '\0';
		++p;
	}

	if (!port)
		port = IPPORT_TELNET;


	int sock_rval; Sock sock;

	CStr body, dom, ref;

	qCtxTmp tmpCtx(ctx);

	if (!bodyStr.IsEmpty()) {
		tmpCtx.MapObj(&sock, EvalXTCPSend,"send");
		tmpCtx.MapObj(&sock, EvalXTCPRecv,"recv");
	}

	PROTO_OPEN_SOCK();

	if (timeout > 1)
		sock.SetTimeout((float) timeout);

	tmpCtx.Parse(bodyStr, out);

	sock.Close();

	return;
}

void qObjProto::EvalWhois(qCtx *ctx, qStr *out, qArgAry *args) 
{
	if (args->Count() < 1) {
		ctx->Throw(out, 655, "USAGE: %whois(host[;host;host...][,body[,server[,bindip]]])"); 
		return;
	}

	CStrAry hosts;
	CStr hostStr = (*args)[0];
	CStr bodyStr = (*args)[1];
	CStr serv = (*args)[2];
	CStr bindip = (*args)[3];

	ProtoParseHosts(hosts, hostStr);

	if (hosts.Count() == 0)
		return;

	int port;

	if(serv.IsEmpty())

		serv = DEFAULT_WHOIS_HOST;
	
	port = IPPORT_WHOIS;

	int sock_rval, i;
	Sock sock;

	if(!bindip.IsEmpty())
		sock.Bind(bindip);

	CStr body, dom, ref, reg, url;

	qCtxTmp tmpCtx(ctx);

	if (!bodyStr.IsEmpty()) {
		tmpCtx.MapObj(&body, "results");
		tmpCtx.MapObj(&body, "body");

		tmpCtx.MapObj(&dom,  "domain");
		tmpCtx.MapObj(&ref,  "refer");

		tmpCtx.MapObj(&dom,  "domain-name");
		tmpCtx.MapObj(&reg,  "registrar");
		tmpCtx.MapObj(&ref,  "whois-server");
		tmpCtx.MapObj(&url,  "referral-url");
	}

	for (i = 0; i < hosts.Count(); ++i) {
		PROTO_OPEN_SOCK();

		body = CStr();
		ref = CStr();
		dom = hosts[i];
		sock.Write(hosts[i]<<"\r\n", hosts[i].Length()+2);

		do {
			if ((sock_rval = sock.Read(SOCK_DEFAULT_BUF_SIZE)) < 0) {
				if (sock_rval == Sock::ERR_TIMEOUT)
					ctx->ThrowF(out, 705, "Time out while reading from host %s:%d, %y", (const char *) serv, port, GetLastError());
				else if (sock_rval)
					ctx->ThrowF(out, 706, "Error reading from host %s:%d, %y", (const char *) serv, port, GetLastError());
				return;
			}
			if (!bodyStr.IsEmpty())
				body << CStr(sock.GetBuf(), sock_rval);
			else 
				out->PutS(sock.GetBuf(), sock_rval);
		}
		while( sock_rval > 0);	//  If something was received

		if (!bodyStr.IsEmpty()) {
			const char *p;
			if ((p = stristr(body.GetBuffer(),"Whois Server:"))) {
				p += 14;
				while (isspace(*p)) ++p;
				char *e = strchr(p, '\n');
				ref = CStr(p, e-p);
			}
			if ((p = stristr(body.GetBuffer(),"Registrar:"))) {
				p += 14;
				while (isspace(*p)) ++p;
				char *e = strchr(p, '\n');
				reg = CStr(p, e-p);
			}
			if ((p = stristr(body.GetBuffer(),"Referral URL:"))) {
				p += 14;
				while (isspace(*p)) ++p;
				char *e = strchr(p, '\n');
				url = CStr(p, e-p);

			}
			tmpCtx.Parse(bodyStr, out);
		}
		sock.Close();
	}

	return;
}

void qObjProto::EvalRWhois(qCtx *ctx, qStr *out, qArgAry *args) 
{
	if (args->Count() < 1) {
		ctx->Throw(out, 655, "USAGE: %rwhois(host[;host;host...][,body[,server]])"); 
		return;
	}

	CStrAry hosts;
	CStr hostStr = (*args)[0];
	CStr bodyStr = (*args)[1];
	CStr serv = (*args)[2];

	ProtoParseHosts(hosts, hostStr);

	if (hosts.Count() == 0)
		return;

	int port;

	if (serv.IsEmpty())
		serv = DEFAULT_RWHOIS_HOST;

	port = IPPORT_RWHOIS;

	int sock_rval, i;
	Sock sock;

	qCtx *tmpCtx = NULL;
	CStr body;

	if (!bodyStr.IsEmpty()) {
		tmpCtx = new qCtxTmp(ctx);
		tmpCtx->MapObj(&body, "results");
	}

	PROTO_OPEN_SOCK();

	sock.Write("-holdconnect on\r\n");

	char *line = NULL;
	bool done;
	
	PROTO_READ_LINE();
	if (line && (*line != '%')) {
		ctx->ThrowF(out, 709, "RWhoIs not supported at host %s:%d", (const char *) serv, port);
		return;
	}

	PROTO_READ_LINE();
	if (line && stricmp(line,"%ok")) {
		CStr err;
		if (strnicmp(line,"%info",5)) {
			err = line;
		}
		PROTO_READ_LINE();
		while (line && (*line != '%')) {
			err << line;
			err << '\n';
			PROTO_READ_LINE();
		}
		ctx->ThrowF(out, 709, "RWhoIs at %s:%d failed (%s)", (const char *) serv, port, (const char *) err);
		return;
	}

	for (i = 0; i < hosts.Count(); ++i) {
		sock.Write("domain ", 7);
		sock.Write(hosts[i], hosts[i].Length());
		sock.Write("\r\n", 2);
		done = false;
		if (line && !stricmp(line,"%ok")) {
			PROTO_READ_LINE();
			while (line && (*line != '%')) {
				out->PutS(line);
				PROTO_READ_LINE();
				out->PutC('\n');
			}
		}
	}


	sock.Write("-quit\r\n",14);

	return;
}

void LoadProto(qCtx *ctx) {
	qObjProto *qm = new qObjProto;
	ctx->MapObj(qm,	"<proto>");

	ctx->MapObj(qm , (QOBJMETH) &qObjProto::EvalSmtpMail, "smtp-mail");
#ifdef WIN32
	ctx->MapObj(qm , (QOBJMETH) &qObjProto::EvalQMail,    "qmail");
	ctx->MapObj(qm , (QOBJMETH) &qObjProto::EvalWhois,    "whois");
	ctx->MapObj(qm , (QOBJMETH) &qObjProto::EvalRWhois,   "rwhois");
	ctx->MapObj(qm , (QOBJMETH) &qObjProto::EvalXTCP,     "connect-tcp", "01");
	ctx->MapObj(qm , (QOBJMETH) &qObjProto::EvalTCPConn,  "tcp");
#endif
  
//	ctx->MapObj(qm , (QOBJMETH) &qm->EvalSSL,      "connect-ssl");
}
