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
