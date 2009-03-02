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

#include "util.h"

#define CC_MAP_COUNT 22
#define CC_NAME_COUNT 10

enum CC_TYPES
{
	cc_unknown,
	cc_enroute,
	cc_diners,
	cc_amex,
	cc_visa,
	cc_mc,
	cc_discover,
	cc_jcb,
	cc_switch,
	cc_bankcard
};

char * CC_NAMES[CC_NAME_COUNT] = 
{
	"unknown",
	"enroute",
	"diners",
	"amex",
	"visa",
	"mc",
	"discover",
	"jcb",
	"switch",
	"bankcard"
};

char * CC_PREFIX[CC_MAP_COUNT] = 
{
	"1800", 
	"2014", 
	"2131", 
	"2149", 
	"300", 
	"301", 
	"302",
	"303", 
	"304", 
	"305", 
	"34", 
	"36", 
	"37",
	"38", 
	"4", 
	"51", 
	"52", 
	"53", 
	"54", 
	"55", 
	"56", 
	"6011" 
};


CC_TYPES CC_MAP[CC_MAP_COUNT] = 
{
	cc_jcb,
	cc_enroute,
	cc_jcb,
	cc_enroute,
	cc_diners,
	cc_diners,
	cc_diners,
	cc_diners,
	cc_diners,
	cc_diners,
	cc_amex,
	cc_diners,
	cc_amex,
	cc_diners,
	cc_visa,
	cc_mc,
	cc_mc,
	cc_mc,
	cc_mc,
	cc_mc,
	cc_bankcard,
	cc_discover 
};


int GetMask(CStr CardName);


bool ConfirmChecksum(CStr CardNumber)
{
	int  CheckSum;            // Holds the value of the operation              
	bool Flag;                // used to indicate when ready                   
	int  Counter;             // index counter                                 
	int  Number;              // used to convert each digit to integer         

   /**************************************************************************
   function is extracting each digit of the number and subjecting it to the
   checksum formula established by the credit card companies.  It works from
   the end to the front.
   **************************************************************************/

   // get the starting value for our counter 
   Counter = CardNumber.Length() - 1;
   CheckSum = 0;
   Number = 0;
   Flag = false;

   while ( Counter >= 0 )
   {
      // get the current digit 
      Number = CardNumber.Data()[Counter] - '0';
      if ( Flag ) // only do every other digit 
      {
         Number = Number * 2;
         if ( Number >= 10 ) 
			 Number = Number - 9;
      }
      CheckSum = CheckSum + Number;
      Flag = !Flag;
      Counter = Counter - 1;
   }
   return ( ( CheckSum % 10 ) == 0 );
}

#define ShiftMask(n) (1 << (n-12))

int GetMask(int cc_type)
{
   if ( cc_type == cc_mc ) 
	   return ShiftMask( 16 );
   if ( cc_type == cc_visa ) 
	   return ( ShiftMask( 13 ) | ShiftMask( 16 ) );
   if ( cc_type == cc_amex ) 
	   return ShiftMask( 15 );
   if ( cc_type == cc_diners ) 
	   return ShiftMask( 14 );
   if ( cc_type == cc_discover ) 
	   return ShiftMask( 16 );
   if ( cc_type == cc_jcb ) 
	   return ShiftMask( 16 )  | ShiftMask( 15 );
   if ( cc_type == cc_switch ) 
	   return ShiftMask( 16 )  | ShiftMask( 18 ) | ShiftMask( 19 );
   if ( cc_type == cc_bankcard ) 
	   return ShiftMask( 16 );

   return ( ShiftMask( 13 ) | ShiftMask( 14 ) | ShiftMask( 15 ) | ShiftMask( 16 ) );
}

int GetCardType(const char *b, int len)
{
	const char * p, *p2;
	int l = 0, h = CC_MAP_COUNT-1, m;
	while (h >= l) {
		m = (h+l)/2;
		p = b;
		p2 = CC_PREFIX[m];
		while (*p2 && *p2 == *p)
			++p2, ++p;
		if (!*p2)
			return CC_MAP[m];
		if (*p > *p2)
			l  = m + 1;
		else
			h  = m - 1;
	}
	if (len == 16 && *b == '3')
		return cc_jcb;
	if (len == 18 || len == 19)
		return cc_switch;
	return cc_unknown;
}

CStr GetCardName(int cc_index)
{
	if (cc_index >=0 && cc_index < CC_NAME_COUNT)
		return CC_NAMES[cc_index];
	else
		return CC_NAMES[cc_unknown];
}


CStr CleanCard(CStr CardNumber)
{
	return ReplaceStr(ReplaceStr(ReplaceStr(CardNumber, "-", ""), " ", ""),".","");
}

CStr VerifyCard(CStr CardNumber)
{
	CardNumber = CleanCard(CardNumber);
	
	if (CardNumber.Length() == 0) 
		return 0;

	const char * b = CardNumber;
	const char * p;

	p = b;
	while (*p) {
		if (!isdigit(*p))
			return 0;
		++p;
	}

	int l = p - b;

   // check the length
	if ( l > 28 )
		return 0;

	int type = GetCardType(b, l);

	int mask = GetMask( type );

    if ( (l < 12) || ( (ShiftMask(l) && mask) == 0 ) )
		return 0;

   // check the checksum computation
	bool doChecksum;
	if ( type == cc_enroute )
		doChecksum = false;
	else
		doChecksum = true;

   if ( doChecksum && ( !ConfirmChecksum( CardNumber ) ) )
		return 0;

   return GetCardName(type);
}

void EvalVerifyCard(const void *data, qCtx *ctx, qStr *out, qArgAry *args) 
{
	out->PutS(VerifyCard((*args)[0]));
}

void EvalCardPrefix(const void *data, qCtx *ctx, qStr *out, qArgAry *args) 
{
	CStr tmp = CleanCard((*args)[0]);
	out->PutS(tmp, min(tmp.Length(),ParseInt((*args)[1])));
}

void LoadCard(qCtx *ctx) {
	ctx->MapObj(NULL, EvalVerifyCard, "verify-card");
	ctx->MapObj(NULL, EvalCardPrefix, "card-prefix");
}
