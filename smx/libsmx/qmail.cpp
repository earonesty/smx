/*
Copyright 1998-2005 Erik Aronesty. All rights reserved.

        1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

        2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

	3. Original source code, information and documentation is available at www.smxlang.org

THIS SOFTWARE IS PROVIDED 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE SMX AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

/* Open Source:

   Free to use.
   Must distribute code with product.
   Must attribute this code or code based on this to author(s).
   Somebody reword this please

   Authors:

   Erik Aronesty (erik@q32.com)
*/
#include "stdafx.h"

#define _Q_NOFORCE_LIBS

#ifdef WIN32
	#include <winsock.h>
	#include <io.h>
	#include <process.h> 
#endif

#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "ex.h"
#include "opt.h"
#include "sock.h"
#include "base64.h"
#include "qmail.h"
#include "qmail_resource.h"

#ifdef WIN32
  #define Q_APPKEY  "Software\\qFactory\\qMail"
#endif

#define Q_MINTIMEOUT 5
#define Q_ERRTHRESH 8
#define Q_CHUNK 6144			// must be multiple of 48
#define Q_BOUND "---//=+_QMail_0L0_01Q/BD3C=7D.B+C3A5E540---"

#define qmSMTP 1
#define qmSave 4

#ifdef WIN32
// ++ global ++ //
static HKEY gAppKey;
#endif

enum {					// smtp state values
	qsNone = 0,
	qsOpen = 1,
	qsHelo,
	qsEhlo,
	qsMail,
	qsRcpt,
	qsData,
	qsBlock,
	qsEOF,
	qsQuit,
};
// xx end global xx //

// ++ state ++ //

class qMailState
{
public:
	qMailOpts *opts;

// proto state
	int proto;

// proto err code
	int code;

// multipart
	int bodyn;
	char *bodyc;

// more state info
	int		errs;
	char *	cur_buf;
	int     cur_len;
	int		data;

	CStr    buf;
	FILE *  fbody;
	int     len;
	bool    multi;
	bool    enc;

qMailState() {
	proto = 0;
	code = 0;
	bodyn = 0;
	bodyc = NULL;
	errs = 0;
	cur_buf = NULL;
	cur_len = 0;
	data= 0;
	fbody=NULL;
	len = 0;
	multi=false;
	enc=false;
}

};

// string helpers
static char *fslash(char *arg);

// ** smtp functions
static void parse_email(char *email,	// parse email addr
				 char *&user, char *&host, int &port, char *&desc);
static int  smtp_next(qMailState *pstate, CStr &request);
static int  smtp_resp(qMailState *pstate, char *line); // multiline response

// ** misc. registry functions

#ifdef WIN32
	static HKEY GetAppRegistryKey();
	static HKEY OpenRegistryKey(HKEY hKey, LPCTSTR strKey);
	static bool GetRegistryValue(HKEY hKey, LPCTSTR szName, DWORD &dwData);
	static bool GetRegistryValue(HKEY hKey, LPCTSTR szName, CStr &strData);
	static void SetRegistryValue(HKEY hKey, LPCTSTR szName, CStr strData);
	static void SetRegistryValue(HKEY hKey, LPCTSTR szName, DWORD dwData);
#endif

// xx end proto xx //

int qsmtp(qMailOpts *popts)
{

	Sock *pSock = NULL;
	int rval = 0;

	try
	{
		if (popts->timeout == 0.0F)
			popts->timeout = QM_TIMEOUT;
		else if (popts->timeout < QM_TIMEOUT_MIN)
			popts->timeout = QM_TIMEOUT_MIN;

		if (!popts->port)
			popts->port = QM_SMTP_PORT;

		if (!popts->smtp)
			throw qEx(199, "Host was not specified");

		if (!popts->rcpt)
			throw qEx(198, "Must specify recipient.");

		if (!popts->to && !strchr((const char *)popts->rcpt,';'))
			popts->to = popts->rcpt;

		if (!popts->helo) 
			popts->helo = popts->smtp;

		const char *host = popts->smtp; 
		int port = popts->port;

		pSock = new Sock;
		pSock->SetTimeout(popts->timeout);

		int sock_err = pSock->Open(host, port);

		if (sock_err == Sock::ERR_GETHOST)
			throw qEx(QM_ERR_SOCK_GETHOST, QM_ERR_SOCK_GETHOST_RC, host);
		else if (sock_err == Sock::ERR_CONNECT)
			throw qEx(QM_ERR_SOCK_CONNECT, QM_ERR_SOCK_CONNECT_RC, host, port);
		else if (sock_err == Sock::ERR_TIMEOUT)
			throw qEx(QM_ERR_SOCK_CONNECT, QM_ERR_SOCK_CONNECT_RC, host, port);
		else if (sock_err)
			throw qEx(sock_err, "Connect to host %s, error #%d", host, sock_err);


		qMailState state;

		int   multi;
		char *line, *pcur;
		int   errs;
		CStr  request;

		state.opts = popts;
		state.proto = 1;
		state.code = 0;

		pSock->ReadLine(&line);

		do {
			errs = smtp_next(&state, request);
			if (errs > Q_ERRTHRESH) {
				char *p = request.Length() ? strrchr((const char *) request,'\r') : NULL;
				if (p) *p = '\0';
				throw qEx(QM_ERR_SMTP_THRESH, QM_ERR_SMTP_THRESH_RC, errs, (const char *) request);
			}

			if (pSock->Write(request, request.Length()) < 0)
				throw qEx(QM_ERR_SOCK_WRITE_REQ, QM_ERR_SOCK_WRITE_REQ_RC, host);

			if (state.proto != qsBlock && state.proto != qsQuit) {
				do {
					if (pSock->ReadLine(&line) < 0)
						throw qEx(QM_ERR_SOCK_READ_REQ, QM_ERR_SOCK_READ_REQ_RC, host);
					state.code = strtol(line, &pcur, 10);
					multi = (*pcur++ == '-');
					smtp_resp(&state, pcur);
				} while (multi);
			}
		} while (state.proto);
	} catch (CEx pEx) {
		if (pSock) delete pSock;
		pSock = NULL;
		throw pEx;
	} catch (...)	{
		rval = 999;
	}

	if (pSock) delete pSock;
	return rval;
}

int smtp_resp(qMailState *pstate, char *line)
{
	// todo: parse SIZE attribute
	return 0;
}

void smtp_rcpt_error(char *rcpt)
{
	fprintf(stderr, "ERROR #20: Invalid recipient %s\n", rcpt);
}

char *smtp_rcpt_next(qMailState *state, int first = 0)
{
	if (first) {
		if (*state->opts->rcpt == '@') {
			FILE *fp; int len, off = 0;
			if ((fp = fopen(&(state->opts->rcpt[1]),"r"))) {
				state->opts->rcpt.Grow(Q_CHUNK + 1);
				while ((len = fread(&state->opts->rcpt[off], 1, Q_CHUNK, fp))) {
					off += len;
					state->opts->rcpt.Grow(off + len + 1);
				}
				fclose(fp);
			} else {
				throw qEx(QM_ERR_OPEN_FILE, QM_ERR_OPEN_FILE_RC, (&state->opts->rcpt[1]));
			}
		}
		return strtok(state->opts->rcpt.GetBuffer(),";\n\r");
	} else {
		char *r = strtok(NULL,";\n\r");
		return r;
	}
}

char *multi_delim(CStr &buf, bool last)
{
	buf = "\r\n--";
	buf += Q_BOUND;
	if (last) buf += "--";
	buf += "\r\n";
	return buf.GetBuffer();
}

char *multi_head(CStr &buf, char *file, bool &enc)
{
	char *ext;
	multi_delim(buf, false);
	if (file && *file)
		ext = strrchr(file, '.');
	else
		ext = 0;

	if (ext && !strnicmp(ext,".txt",4)) {
		enc = false;
		buf += "Content-Type: text/plain; charset=us-ascii\r\n";
		buf += "Content-Transfer-Encoding: 7 bit\r\n";
	} else {
		enc = true;
		buf += "Content-Type: application/x-ext-";
		buf += ext + 1;
		buf += "; name=\"";
		if ((ext = strrchr(fslash(file), '/'))) {
			file = ext + 1;	
		}
		buf += file;
		buf += "\"\r\n";
		buf += "Content-Transfer-Encoding: base64\r\n";
	}
	return (buf += "\r\n").GetBuffer();
}

char *smtp_data_next(qMailState *state, int first, int &len)
{
	CStr &buf = state->buf;
	len = 0;

	if (first) {
		if (state->opts->subj) {
			buf  = "Subject: "; buf += state->opts->subj; 
			buf += "\r\n";
		}

		if (state->opts->from) {
			buf += "From: "; buf += state->opts->from; buf += "\r\n";
		}

		if (state->opts->to) {
			buf += "To: "; buf += state->opts->to; buf += "\r\n";
		}

		if (state->opts->reply) {
			buf += "Reply-To: "; buf += state->opts->reply; buf += "\r\n";
		}

		int i = 0;
		while (state->opts->other[i++]) {
			buf += "Reply-To: "; buf += state->opts->reply; buf += "\r\n";
		}

		if (state->opts->body.Count() > 1) {
			state->multi = true;
			buf += "MIME-Version: 1.0\r\n";
			buf += "Content-Type: multipart/mixed; boundary=\"";
			buf += Q_BOUND;
			buf += "\"\r\n";
		} else {
			state->multi = false;
			buf += "Content-Type: text/plain; charset=us-ascii\r\n";
		}
		state->bodyn = 0;
		state->bodyc = state->opts->body.GetAt(state->bodyn).GetBuffer();
		buf += "\r\n";
	} else {
		if (state->bodyc && *state->bodyc) {
			if (*state->bodyc == '\'') {
				const char *r = state->bodyc + 1;
				state->bodyc = state->opts->body.GetAt(++state->bodyn).GetBuffer();
				if (state->multi) {
					multi_head(buf, ".txt", state->enc);
					buf += r;
				} else {
					buf = r;
				}
			} else {
				if (!state->fbody) { // open body file
					buf.Grow(Q_CHUNK);
					if (!state->multi && !strcmp(state->bodyc, "-"))
						state->fbody = stdin;
					else if (!(state->fbody = fopen(state->bodyc, "rb")))
						throw qEx(QM_ERR_OPEN_FILE, QM_ERR_OPEN_FILE_RC, state->bodyc);

					if (state->multi) {
						multi_head(buf, state->bodyc, state->enc);
						len = buf.Length();
						return buf.GetBuffer();
					}
				}

				if (state->fbody) { // read body file
					if (!(state->len = fread(buf.GetBuffer(), 1, Q_CHUNK, state->fbody))) {
						buf.Grow(0);
						if (state->fbody != stdin) fclose(state->fbody);
						if (state->multi) {
							// open attach file
							state->bodyc = state->opts->body.GetAt(++state->bodyn).GetBuffer();
							if (state->bodyc && *state->bodyc) {
								if (!(state->fbody = fopen(state->bodyc, "rb")))
									throw qEx(QM_ERR_OPEN_FILE, QM_ERR_OPEN_FILE_RC, (const char *) state->opts->body.GetAt(0));
								multi_head(buf, state->bodyc, state->enc);
							} else {
								multi_delim(buf, true);
							}
						}
					} else {
						buf[state->len] = '\0';
						if (state->enc)  {
							const char *ebuf = mime64(buf.GetBuffer(), state->len);
							buf.Grow(state->len);
							memcpy(buf.GetBuffer(), ebuf, state->len);
							free((void*)ebuf);
							buf[state->len] = '\0';
						} else {
							buf.Grow(state->len);
						}
					}
				}

			}
		} else {
			len = 0;
			return NULL;
		}
	}

	len = buf.Length();
	if (len > 0)
		return buf.GetBuffer();
	else
		return 0;
}

#define iserror(c) (c >= 400)
int smtp_next(qMailState *state, CStr &request)
{
// control
	switch (state->proto) {
	case qsOpen:
		state->errs = 0;
		state->proto = qsEhlo; 
		break;
	case qsEhlo:
		state->proto = iserror(state->code) ? qsHelo : qsMail; 
		break;
	case qsHelo:
		state->proto = iserror(state->code) ? (++state->errs, qsHelo) : qsMail; 
		break;
	case qsMail:
		state->cur_buf = smtp_rcpt_next(state, 1);
		state->cur_len = state->cur_buf ? strlen(state->cur_buf) : 0;
		state->proto = iserror(state->code) ? (++state->errs, qsMail) : qsRcpt; 
		break;

	case qsRcpt:
		if (iserror(state->code)) 
			smtp_rcpt_error(state->cur_buf);
		else 
			state->errs = 0;

		state->cur_buf = smtp_rcpt_next(state);
		if (!state->cur_buf) 
			state->proto = qsData;
		state->cur_len = state->cur_buf ? strlen(state->cur_buf) : 0;
		break;
	case qsData:
		state->proto = iserror(state->code) ? (++state->errs, qsData) : qsBlock;
		state->cur_buf = NULL;
		state->cur_buf = smtp_data_next(state, 1, state->cur_len);
		break;
	case qsBlock:
		if (iserror(state->code))
			++state->errs;
		else
			state->cur_buf = smtp_data_next(state, 0, state->cur_len);
		if (!state->cur_buf) 
			state->proto = qsEOF;
		break;
	case qsEOF:
		state->proto = qsQuit;
		break;
	case qsQuit:
		state->proto = 0;
		break;
	}

// format request
	switch (state->proto) {
	case qsEhlo:
		request.Format("EHLO %s\r\n", state->opts->helo); break;
	case qsHelo:
		request.Format("HELO %s\r\n", state->opts->helo); break;
	case qsMail:
    if (state->opts->from) {
      request = "MAIL FROM:<" << state->opts->from << ">\r\n";
    } else {
      request = "MAIL FROM:<>\r\n";
    }
    break;
	case qsRcpt:
  		if (state->cur_buf)
        request.Format("RCPT TO:<%s>\r\n", state->cur_buf);
      break;
	case qsData:
		request = "DATA\r\n"; break;
	case qsBlock:
		request = CStr(state->cur_buf, state->cur_len); break;
	case qsEOF:
		request = "\r\n.\r\n"; break;
	case qsQuit:
		request = "QUIT\r\n"; break;
	}

	return state->errs;
}

/*
static char *back_slash(char *arg)
{
	char *p = arg;
	while(*p)
	{
		if (*p=='/') *p='\\';
		++p;
	}
	return arg;
}
*/

static char *fslash(char *arg)
{
	char *p = arg;
	while(*p)
	{
		if (*p=='\\') *p='/';
		++p;
	}
	return arg;
}


void parse_email(char *email, char *&user, char *&host, int &port, char *&desc)
{
	char *p = strchr(email, '@');
	if (p) {
		if (p != email) {
			*p = '\0';
			user = email;
			host = p + 1;

			if ((p = strchr(host, '('))) {
				*p = 0;
				desc = ++p;
				if ((p = strchr(desc, ')'))) {
				}
			}

			if ((p = strchr(host, ':')))
			{
				port = atoi(p + 1);
				if (port <= 0)
					throw qEx(QM_ERR_BADPORT, QM_ERR_BADPORT_RC, host);
				*p = 0;
			}
		}
	}
}

//#define RESDEBUG

void qinit()
{

#ifndef WIN32
	char *argv[1] = {NULL};
#endif

#ifdef RESDEBUG
	printf("qmail/qinit pre: ghRes %d\n", ghRes);
#endif

	RES_INIT;

#ifdef RESDEBUG
	printf("qmail/qinit post: ghRes %d\n", ghRes);
#endif

#if MAC
	GUSISetup(GUSIwithSIOUXSockets);
	GUSISetup(GUSIwithInternetSockets);
#elif WIN32
	WSAData wsaData;
	WORD wVersionRequested = MAKEWORD(1, 1); 

// WSA startup
	int wsa_err;
	if (wsa_err = WSAStartup(wVersionRequested, &wsaData)) 
		throw qEx(QM_ERR_WSASTARTUP, QM_ERR_WSASTARTUP_RC, wsa_err);
#endif
}

void qterm()
{
#ifdef WIN32
	WSACleanup();

	if (gAppKey) 
        RegCloseKey(gAppKey);
#endif

    if (ghRes) 
        resTerm(ghRes);
}

#ifdef WIN32

void setregdef(qMailOpts *popts)
{
	CStr sBuf;
	// save registry/defaults
	if (gAppKey = GetAppRegistryKey())
	{
		SetRegistryValue(gAppKey, "Timeout", (DWORD) (popts->timeout * 1000));
		if (popts->host.Length())
			SetRegistryValue(gAppKey, "Smtp", popts->host);
	}
}

void getregdef(qMailOpts *popts)
{
// read qmail registry - defaults
	if (gAppKey = GetAppRegistryKey())
	{
		CStr sBuf;
		DWORD dwBuf;
		if ( GetRegistryValue(gAppKey, "Timeout", dwBuf))
			popts->timeout = (float) (dwBuf / 1000.0f);
		else if (popts->timeout == 0.0f)
			popts->timeout = Q_MINTIMEOUT;

		if (GetRegistryValue(gAppKey, "Smtp", sBuf))
			popts->host = sBuf;

		if (GetRegistryValue(gAppKey, "From", sBuf))
			popts->from = sBuf;
	}

// read microsoft preferences
	if (!popts->host.Length() || !popts->from) {
		HKEY hk1; CStr str; DWORD dw;
		CStr host;
		if (RegOpenKeyEx(HKEY_CURRENT_USER, 
			"software\\microsoft\\office\\8.0\\outlook\\internet account manager", 0, KEY_READ, &hk1) == ERROR_SUCCESS) 
		{
			if (GetRegistryValue(hk1, "default mail account", str)) {
				HKEY hk2;
				str = "accounts\\" << str;
				if (RegOpenKeyEx(hk1, (const char *) str, 0, KEY_QUERY_VALUE, &hk2) == ERROR_SUCCESS) {
					if (!popts->host.Length()) {
				 		if (GetRegistryValue(hk2, "POP3 User Name", str)) {
							host = str;
							host += '@';
						}
						if (GetRegistryValue(hk2, "SMTP Server", str)) {
							host += str;
						}
						if (GetRegistryValue(hk2, "SMTP Port", dw)) {
							host += ':';
							host += (int) dw;
						}
						popts->host = host;
					}
					
					if (popts->timeout == 0) {
						if (GetRegistryValue(hk2, "SMTP Timeout", dw))
							popts->timeout = (float) dw;
					}

					if (!popts->from.Length()) {
						if (GetRegistryValue(hk2, "SMTP Display Name", str))
							popts->from = str;
						if (GetRegistryValue(hk2, "SMTP Email Address", str)) {
							if (popts->from) {
								popts->from += '<';
								popts->from += str;
								popts->from += '>';
							} else 
								popts->from = str;
						}
					}
					RegCloseKey(hk2);
				}
			}
			RegCloseKey(hk1);
		}
	}
}

#endif

int parseopts(qMailOpts *popts, int &argc, char **&argv)
{
// arg init
	setarg(&argc, argv);

	char *popt;
	char *pend;

	if ((popt = getopt("ti*meout"))) {
		popts->timeout = (float) strtod(popt, &pend);
		if (popts->timeout < Q_MINTIMEOUT) 
			popts->timeout = Q_MINTIMEOUT;
	}

	if (popts->host) {
		pend = 0;
		parse_email(popts->host.GetBuffer(), popts->user, pend, popts->port, popts->desc);
		if (pend) 
			popts->smtp = pend;
		else
			popts->smtp = popts->host;
	}

 	if ((popt = getopt("u*sername"))) {
		pend = 0; popts->user = 0;
		parse_email(popt, popts->user, pend, popts->port, popts->desc);
		if (pend) popts->smtp = pend;
		if (!popts->user) 
			popts->user = popt;
	}

	if (((popt = getopt("h*ost")))) {
		pend = 0;
		popts->host = popt;
		parse_email(popt, popts->user, pend, popts->port, popts->desc);
		popts->smtp = pend ? pend : popt;

	}
	
	if (popts->smtp) {
		if ((popt = getopt("p*assword")))
			popts->pass = popt;

		if ((popt = getopt("to")))
			popts->to = popt;

		if ((popt = getopt("f*rom")))
			popts->from = popt;
		
		if ((popt = getopt("r*eply")))
			popts->reply = popt;

		int o = 0;
		while ((o < 7) && (popt = getopt("o*ther")))
			popts->other.SetAt(o++,popt);
		popts->other.SetAt(o++,0);


		int i = 0;
		while ((i < 7) && (popt = getopt("b*ody")))
			popts->body.SetAt(i++,popt);
		
		char *p = getarg(3);
		if (p && *p) {
			p = strtok(p, ";");
			while (p) {
				popts->body.SetAt(i++,p);
				p = strtok(NULL, ";");
			}
		}

		popts->body.SetAt(i++,0);

		popts->subj = getarg(2);

		popt = getarg(1);

		if (popt) {
			popts->rcpt = popt;
			if (!popts->from) 
				popts->from = "";
			if (!popts->to && !strchr((const char *) popts->rcpt,';'))
				popts->to = popts->rcpt;
			if (!popts->helo) 
				popts->helo = popts->smtp;
		}
	}
 
	popts->save = getoptbool("s*ave");

	if (argc > 1) {
		fprintf(stderr, QM_ERR_UNK_OPT_RC, argv[1]);
		fprintf(stderr, "\n");
		return QM_ERR_UNK_OPT;
	} else if (!popts->smtp) {
		fprintf(stderr, QM_ERR_NEED_HOST_RC);
		fprintf(stderr, "\n");
		return QM_ERR_NEED_HOST;
	} else if (!popts->rcpt && !popts->save) {
		fprintf(stderr, QM_ERR_NEED_RCPT_RC);
		fprintf(stderr, "\n");
		return QM_ERR_NEED_RCPT;
	}

	return 0;
}


#ifdef WIN32

// registry functions

HKEY GetAppRegistryKey()
{
	HKEY hAppKey = NULL;
	RegCreateKey(HKEY_LOCAL_MACHINE, Q_APPKEY, &hAppKey);
	return hAppKey;
}

HKEY OpenRegistryKey(HKEY hKey, LPCTSTR strKey)
{
	if (RegOpenKey(hKey, strKey, &hKey) != ERROR_SUCCESS)
		return 0;
	return hKey;
}

bool GetRegistryValue(HKEY hKey, LPCTSTR szName, DWORD &dwData)
{
	DWORD dwSize = sizeof(DWORD), dwType = REG_DWORD, dwValue;
	if (RegQueryValueEx(hKey, szName, NULL, &dwType, (LPBYTE) &dwValue, &dwSize) == ERROR_SUCCESS) {
		if (dwSize == 4) dwData = dwValue;
		return true;
	} else
		return false;
}

bool GetRegistryValue(HKEY hKey, LPCTSTR szName, CStr &strData)
{
	DWORD dwSize = 0, dwType = REG_SZ;
	RegQueryValueEx(hKey, szName, NULL, &dwType, NULL, &dwSize);
	LPBYTE buf = (LPBYTE) strData.Grow(dwSize + 1).GetBuffer();

	if (dwType && (RegQueryValueEx(hKey, szName, NULL, &dwType, buf, &dwSize) == ERROR_SUCCESS))
	{
		if (dwSize && buf[(int)dwSize-1] == '\0')
			strData.Grow(dwSize-1);
		else
			strData.Grow(dwSize);
		return true;
	} else {
		strData.Grow(0);
	}
	return false;
}

void SetRegistryValue(HKEY hKey, LPCTSTR szName, CStr strData)
{
	RegSetValueEx(hKey, szName, 0, REG_SZ, (LPBYTE) (LPCTSTR) strData, strData.Length());
}

void SetRegistryValue(HKEY hKey, LPCTSTR szName, DWORD data)
{
	RegSetValueEx(hKey, szName, 0, REG_DWORD, (LPBYTE) &data, sizeof(DWORD));
}
#endif
