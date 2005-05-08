/* COPYRIGHT 1998 Prime Data Corp. 
	All Rights Reserved.  Use of this file without prior written 
	authorization is strictly prohibited.
*/
#define QM_TIMEOUT     60
#define QM_TIMEOUT_MIN 20
#define QM_ERR_OFF     200
#define QM_SMTP_PORT   25

#ifndef _STRARY_H
#include "strary.h"
#endif

// ++ options & state info ++ //
class qMailOpts
{
public:
	float timeout;		// usu. default

	CStr host;			// smtp user@host-gateway
	int  save;			// save options - only 
	
	CStr pass;			// smtp password

	CStr rcpt;			// smtp rcpt
	CStr to;			// smtp to header
	CStr from;			// smtp from header
	CStr reply;		    // smtp reply-to
	CStrAry other;		// smtp other headers (up to 8)

	CStr    subj;		// smtp subj header
	CStrAry body;		// smtp 'body records *or* filenames

// parsed from host/email (remove this!)
	char *user;			// user name
	char *desc;			// user description?
	int   port;			// port
	const char *helo;			// smtp helo address
	const char *smtp;			// server address

	bool  unreg;		// registration status

	qMailOpts() {
        timeout = 0.0;
        save = 0;
        user = NULL;
        desc = NULL;
        port = 0;
        helo = NULL;
        smtp = NULL;
        unreg = false;
	}
};

// ++ proto ++ //

// ** main() functions
void qinit();
void qterm();

// registry defaults
void getregdef(qMailOpts *popts);
void setregdef(qMailOpts *popts);

// command-line opts
int  parseopts(qMailOpts *popts, int &argc, char **&argv);	

// command execution function
int  qsmtp(qMailOpts *popts);				// smtp client command

// xx end proto xx //
