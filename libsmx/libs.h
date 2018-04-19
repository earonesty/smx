/*
Copyright 1998-2005 Erik Aronesty. All rights reserved.

        1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

        2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

	3. Original source code, information and documentation is available at www.smxlang.org

THIS SOFTWARE IS PROVIDED 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE SMX AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

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
void LoadImage(qCtx *ctx);

static void LoadAllLibs(qCtx *ctx)
{ 
	LoadCore(ctx);
	LoadMath(ctx);
	LoadString(ctx);
	LoadTime(ctx);
	LoadHSet(ctx);
#ifndef NO_LOAD_IMAGE
	LoadImage(ctx);
#endif
#ifndef NO_LOAD_PSET
	LoadPSet(ctx);
#endif
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

