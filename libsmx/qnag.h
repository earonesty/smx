/*
Copyright 1998-2005 Erik Aronesty. All rights reserved.

        1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

        2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

	3. Original source code, information and documentation is available at www.smxlang.org

THIS SOFTWARE IS PROVIDED 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE SMX AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

// qnag.h
// link with qnag.lib

#ifndef _qnag_h_
	#ifdef _DEBUG
		#pragma comment(lib, "qnagd.lib")
	#else
		#pragma comment(lib, "qnag.lib")
	#endif

#define QN_MAX_INP 554
#define QN_MAX_OUT 42

enum qnResult { 
	qnRegOK   = 0,
	qnNeedNag = 1,
	qnNotReg  = 2
};

/****** qNagK ---- Generates Key ---- For distibution
	foo = "AppName\\version-key"
	inp = temp-buffer (size = QN_MAX_INP)
	out = outp-buffer (size = QN_MAX_OUT)
	set = ascii volume serial number eg: 4423-64AE		******/
bool     qNagK(char *foo, char *inp, char *out, char *ser=0);

/****** qNag  ---- Tests Key ---- Whether it's nag-time
	foo = "AppName\\version-key"
	imax = numer of uses between nags
	show = auto-show msgbox								******/
qnResult qNag(char *foo, char imax, bool show=true);

#endif //#ifndef _qnag_h_
