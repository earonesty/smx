/* COPYRIGHT 1998 Prime Data Corp. 
	All Rights Reserved.  Use of this file without prior written 
	authorization is strictly prohibited.
*/

void LoadCGI(qCtx *ctx);
void LoadCore(qCtx *ctx);
void LoadString(qCtx *ctx);
void LoadTime(qCtx *ctx);
void LoadMath(qCtx *ctx);
void LoadSql(qCtx *ctx);
void LoadFile(qCtx *ctx);
void LoadProto(qCtx *ctx);
void LoadTable(qCtx *ctx);
void LoadPSet(qCtx *ctx);
void LoadHSet(qCtx *ctx);
void LoadProcess(qCtx *ctx);
void LoadIO(qCtx *ctx);
void LoadCard(qCtx *ctx);
void LoadSched(qCtx *ctx);
void LoadTSet(qCtx *ctx);

static void LoadAllLibs(qCtx *ctx)
{ 
	LoadCore(ctx);
	LoadMath(ctx);
	LoadString(ctx);
	LoadTime(ctx);
	LoadHSet(ctx);
#ifndef NO_LOAD_FILE
	LoadFile(ctx);
#endif
#ifndef NO_LOAD_SQL
	LoadSql(ctx);
#endif
#ifndef NO_LOAD_CGI
	LoadCGI(ctx);
	LoadTable(ctx);
#endif
#ifndef NO_LOAD_PROTO
	LoadProto(ctx);
#endif
#ifndef NO_LOAD_PSET
	LoadPSet(ctx);
#endif
#ifndef NO_LOAD_PROCESS
	LoadProcess(ctx);
#endif
#ifndef NO_LOAD_IO
	LoadIO(ctx);
#endif
#ifndef NO_LOAD_CARD
	LoadCard(ctx);
#endif
#ifndef NO_LOAD_SCHED
	LoadSched(ctx);
#endif
#ifndef NO_LOAD_TSET
	LoadTSet(ctx);
#endif
}

