/*
Copyright 1998-2005 Erik Aronesty. All rights reserved.

        1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

        2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

	3. Original source code, information and documentation is available at www.smxlang.org

THIS SOFTWARE IS PROVIDED 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE SMX AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

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
