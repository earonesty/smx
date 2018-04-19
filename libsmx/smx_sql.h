/*
Copyright 1998-2005 Erik Aronesty. All rights reserved.

        1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

        2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

	3. Original source code, information and documentation is available at www.smxlang.org

THIS SOFTWARE IS PROVIDED 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE SMX AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#ifndef SMX_SQL_H
#define SMX_SQL_H

class qObjSql : public qObj {
public:
        virtual void Execute(qCtx *ctx, qStr *out, const char *sql, CStr &body, CStr &head, CStr &foot)=0;
        virtual void EvalColName(qCtx *ctx, qStr *out, qArgAry *args)=0;
        virtual void EvalColType(qCtx *ctx, qStr *out, qArgAry *args)=0;
        virtual void EvalRow(qCtx *ctx, qStr *out, qArgAry *args)=0;
        virtual void EvalSkipRows(qCtx *ctx, qStr *out, qArgAry *args) {};
        virtual void EvalCol(qCtx *ctx, qStr *out, qArgAry *args)=0;
        virtual void EvalColumn(qCtx *ctx, qStr *out, qArgAry *args)=0;
        virtual void EvalEnumCol(qCtx *ctx, qStr *out, qArgAry *args)=0;
};

#endif //#ifndef SMX_SQL_H
