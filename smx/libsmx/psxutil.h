/* COPYRIGHT 1998 Prime Data Corp. 
	All Rights Reserved.  Use of this file without prior written 
	authorization is strictly prohibited.
*/

const char *LookupReplyString(int status);
qEnvHttp *GetHttpEnv(qCtx *ctx);
void EvalBreak(bool *ok, qCtx *ctx, qStr *out, qArgAry *args);
