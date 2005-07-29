/*
Copyright 1998-2005 Erik Aronesty. All rights reserved.

        1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

        2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

	3. Original source code, information and documentation is available at www.smxlang.org

THIS SOFTWARE IS PROVIDED 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE SMX AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/



#include "stdafx.h"

#include "qstr.h"
#include "qobj.h"
#include "qctx.h"

#include "pstime.h"
#include "dparse.h"

#include "util.h"

void EvalTime(const void *data, qCtx *ctx, qStr *out, qArgAry *args) {
	if (args->Count() > 0) {
		time_t tmt = time(0);
		struct tm * tms = gmtime(&tmt);
		CStr p = (*args)[0];
		FmtTime(tms, p, out);
	} else {
		time_t tmt;
		out->PutN((int)time(&tmt));
	}
}

void EvalLocalTime(const void *data, qCtx *ctx, qStr *out, qArgAry *args) {
	if (args->Count() > 0) {
		time_t tmt = time(0);
		struct tm * tms = localtime(&tmt);
		CStr p = (*args)[0];
		FmtTime(tms, p, out);
	} else {
		time_t tmt;
		out->PutN((int)time(&tmt));
	}
}

char *s_wdays[] = 
{
	"Sunday",
	"Monday",
	"Tuesday",
	"Wednesday",
	"Thursday",
	"Friday",
	"Saturday"
};

char *s_wday3[] = 
{
	"Sun",
	"Mon",
	"Tue",
	"Wed",
	"Thu",
	"Fri",
	"Sat"
};

char *s_mons[] = 
{
	"January",
	"February",
	"March",
	"April",
	"May",
	"June",
	"July",
	"August",
	"September",
	"October",
	"November",
	"December"
};

char *s_mon3[] = 
{
	"Jan",
	"Feb",
	"Mar",
	"Apr",
	"May",
	"Jun",
	"Jul",
	"Aug",
	"Sep",
	"Oct",
	"Nov",
	"Dec"
};

void EvalFmtlTime(const void *data, qCtx *ctx, qStr *out, qArgAry *args) {
	if (args->Count() >= 1) {
		time_t tmt = ParseInt((*args)[0]);
		struct tm * tms = localtime(&tmt);

		CStr p;
	
		if (args->Count() == 1)
			p = "m/d/yy hh:nn aa";
		else
			p = (*args)[1];

		if (p && tms) {
			FmtTime(tms, p, out);
		}
	}
}

void EvalFmtgTime(const void *data, qCtx *ctx, qStr *out, qArgAry *args) {
	if (args->Count() >= 1) {
		time_t tmt = ParseInt((*args)[0]);
		struct tm * tms = gmtime(&tmt);

		CStr p;
	
		if (args->Count() == 1)
			p = "m/d/yy hh:nn aa";
		else
			p = (*args)[1];

		if (p && tms) {
			FmtTime(tms, p, out);
		}
	}
}

void EvalMakeGTime(const void *gtime, qCtx *ctx, qStr *out, qArgAry *args) {
	if (args->Count() > 1) {
		struct tm tms;
		memset(&tms, 0, sizeof(tms));

		tms.tm_year = ParseInt((*args)[0]) - 1900;
		tms.tm_mon = ParseInt((*args)[1]);
		tms.tm_mday = ParseInt((*args)[2]);
		tms.tm_hour = ParseInt((*args)[3]);
		tms.tm_min = ParseInt((*args)[4]);
		tms.tm_sec = ParseInt((*args)[5]);
		tms.tm_isdst = -1;

		if (tms.tm_mon) 
			--tms.tm_mon;

		time_t tmt = mktime( &tms );

		if (gtime && (tmt != -1)) 
			tmt -= _timezone;

		out->PutN(tmt);
	}
}

void FmtTime(struct tm * tms, const char *p, qStr *out) {
	while (p && *p) {
		if (p[0] == 'y' && p[1] == 'y') {
			if (p[2] == 'y' && p[3] == 'y') { 
				p += 4; 
				out->PutN(tms->tm_year+1900);
			} else {
				p += 2; 
				if (tms->tm_year % 100 < 10)
					out->PutC('0');
				out->PutN((tms->tm_year+1900) % 100);
			}
		} else if (p[0] == 'h') {
			int hour = tms->tm_hour%12 == 0 ? 12 : tms->tm_hour % 12;
			if (p[1] == 'h') {
				p += 2;
				if (hour < 10)
					out->PutC('0');
				out->PutN(hour);
			} else {
				p += 1;
				out->PutN(hour);
			}
		} else if (p[0] == 'H') {
			int hour = tms->tm_hour;
			if (p[1] == 'H') {
				p += 2;
				if (hour < 10)
					out->PutC('0');
				out->PutN(hour);
			} else {
				p += 1;
				out->PutN(hour);
			}
		} else if (p[0] == 'n') {
			if (p[1] == 'n') {
				p += 2;
				if ((tms->tm_min) < 10)
					out->PutC('0');
				out->PutN(tms->tm_min);
			} else {
				p += 1;
				out->PutN(tms->tm_min);
			}
		} else if (p[0] == 'm') {
			if (p[1] == 'm') {
				if (p[2] == 'm') {
					if (p[3] == 'm') {
						p += 4;
						out->PutS(s_mons[tms->tm_mon]);
					} else {
						p += 3;
						out->PutS(s_mon3[tms->tm_mon]);
					}
				} else {
					p += 2;
					if ((tms->tm_mon+1) < 10)
						out->PutC('0');
					out->PutN(tms->tm_mon+1);
				}
			} else {
				p += 1;
				out->PutN(tms->tm_mon+1);
			}
		} else if (p[0] == 's') {
			if (p[1] == 's') {
				p += 2;
				if ((tms->tm_sec) < 10)
					out->PutC('0');
				out->PutN(tms->tm_sec);
			} else {
				p += 1;
				out->PutN(tms->tm_sec);
			}
		} else if (p[0] == 'w') {
			if (p[1] == 'w') {
				if (p[2] == 'w') {
					if (p[3] == 'w') {
						p += 4;
						out->PutS(s_wdays[tms->tm_wday]);
					} else {
						p += 3;
						out->PutS(s_wday3[tms->tm_wday]);
					}
				} else {
					p += 2;
					out->PutN(tms->tm_wday);
				}
			} else {
				p += 1;
				out->PutN(tms->tm_wday);
			}
		} else if (p[0] == 'd') {
			if (p[1] == 'd') {
				p += 2;
				if (tms->tm_mday < 10)
					out->PutC('0');
				out->PutN(tms->tm_mday);
			} else {
				p += 1;
				out->PutN(tms->tm_mday);
			}
		} else if (p[0] == 'a') {
			if (p[1] == 'a') {
				p += 2;
				if (tms->tm_hour > 11)
					out->PutS("pm",2);
				else
					out->PutS("am",2);
			} else {
				p += 1;
				if (tms->tm_hour > 11)
					out->PutS("p",1);
				else
					out->PutS("a",1);
			}
		}  else if (p[0] == 'A') {
			if (p[1] == 'A') {
				p += 2;
				if (tms->tm_hour > 11)
					out->PutS("PM",2);
				else
					out->PutS("AM",2);
			} else {
				p += 1;
				if (tms->tm_hour > 11)
					out->PutS("P",1);
				else
					out->PutS("A",1);
			}
		} else 
			out->PutC(*p++);
	}
}

void EvalDparse(const void *data, qCtx *ctx, qStr *out, qArgAry *args) {
	CStr date = (*args)[0];
	time_t t = date_parse(date);
	if (t == -1)
		t = 0;
	if (t) {
		out->PutN(t);
	}
}

void LoadTime(qCtx *ctx) {
//string
	ctx->MapObj(EvalTime,	   "time");
	ctx->MapObj(EvalLocalTime, "localtime");
	ctx->MapObj(EvalFmtlTime,  "fmtltime");
	ctx->MapObj(EvalFmtgTime,  "fmtgtime");
	ctx->MapObj((void *)1, EvalMakeGTime, "makegtime");
	ctx->MapObj((void *)0, EvalMakeGTime, "makeltime");
	ctx->MapObj(EvalDparse,    "dparse");
}
