#ifndef SMX_SQLITE_H
#define SMX_SQLITE_H

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

#endif //#ifndef SMX_SQLITE_H
