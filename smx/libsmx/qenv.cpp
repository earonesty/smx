/* COPYRIGHT 1998 Prime Data Corp. 
	All Rights Reserved.  Use of this file without prior written 
	authorization is strictly prohibited.
*/

#include "stdafx.h"
#include "qctx.h"
#include "qstr.h"
#include "qenv.h"

const char * qEnvHttp::GetName()
{
	return GetScriptPath();
}

