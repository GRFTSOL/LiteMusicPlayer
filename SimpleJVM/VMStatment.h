#pragma once


typedef uint16_t                    stmt_index;
#define SI_INVALID                0

enum StmtType
{
    ST_UNKNOWN,
    ST_BLOCK,
    ST_TRY,
    ST_CATCH,
    ST_THROW,
    ST_IF,
    ST_FOR,
    ST_WHILE,
    ST_DO_WHILE,
    ST_BREAK,
    ST_CONTINUE,
    ST_RETURN,
    ST_PUSH_STACK,

    ST_EXPRESSION,
};

// 
// struct StmtVariableAddress {
//     int                    *pResultAddress;
// 
//     bool                bStaticAddr;
//     TypeId                typeId;            // 
//     int                    nAddress;
//     stmt_index            siParentVar;
// };

struct StmtBlock {
    stmt_index            siFirstChild;
};

struct StmtTry {
    stmt_index            siTryStmt;
    stmt_index            siCatch;
};

struct StmtCatch {
    TypeId                typeIdException;
    stmt_index            siCatchStmt;
    stmt_index            siNextCatch;
};

struct StmtThrow {
    stmt_index            siNewExpr;
};

struct StmtIf {
    stmt_index            siCondition;
    stmt_index            siTrueStmt;
    stmt_index            siFalseStmt;
};

struct StmtFor {
    stmt_index            siInitalization;
    stmt_index            siTermination;
    stmt_index            siIncrement;
    stmt_index            siStmt;
};

struct StmtWhile {
    stmt_index            siCondition;
    stmt_index            siStmt;
};

struct StmtDoWhile {
    stmt_index            siCondition;
    stmt_index            siStmt;
};

struct StmtReturn {
    stmt_index            siExpression;
};

struct StmtStackPush {
    // push variable in stack, and save its typeId
    TypeId                typeIdVar;
};

struct StmtExpression {
    ExpressionType        expType;
    TypeId                typeId;                // E_NEW and typeId of this expression
    union {
        int                    nConstValue;    // E_CONST_INT, E_CONST_STRING
        struct {
            stmt_index        siParameter;    // Parameter of construction method
            int                nMemberIndex;
        }                    newObj;            // E_NEW
        struct {
            stmt_index        siVar;
        }                    dynCast;        // E_DYNAMIC_CAST
        struct {                        // E_VAR
            int                nAddress;
            bool            bStatic;
            stmt_index        siParentVar;
        }                    var;
        struct {
            bool            bStatic;
            TypeId            typeIdSpecialfiedClass;    // SI_INVALID, indicate call from typeId of pVarParent.
            int                nMemberIndex;
            stmt_index        siParentVar;
            stmt_index        siParameter;
        }                    functionCall;        // E_METHOD_CALL
        struct {
            stmt_index            siVarAddress;
            stmt_index            siExpression;
        }                    assign;                // E_ASSIGN
        struct {
            stmt_index        siExprLeft;
            stmt_index        siExprRight;
            TypeId            typeIdOperand;        // The type of operand(left, right)
        }                    opParameter;        // OP_XXXX
    };
};

struct StmtNewStackVar {
};

struct StmtFunctionCall {
    int                    nRetValue;        // Return Value

    TypeId                typeIdRet;        // Return type
    TypeId                typeIdClass;    // Method class, if (typeIdClass == TID_INVALID), call dynamic method of siParentVar
    int                    nMethodId;        // Method Id
    stmt_index            siParentVar;
    stmt_index            siParmeterExpStart;    // First Parameter: siParmeterExpStart, Second: siParmeterExpStart->siNextStmt
};

struct Statement
{
    uint16_t                nLineNo;
    stmt_index            siNextStmt;

    StmtType            stmtType;        // Type of the union.
    union {
        // StmtVariableAddress        sVar;
        StmtTry                    sTry;
        StmtCatch                sCatch;
        StmtThrow                sThrow;
        StmtIf                    sIf;
        StmtFor                    sFor;
        StmtWhile                sWhile;
        StmtDoWhile                sDoWhile;
        StmtReturn                sRetrun;
        StmtBlock                sBlock;
        StmtExpression            sExpression;
        StmtNewStackVar            sNewStackVar;
        StmtFunctionCall        sFunctionCall;
        StmtStackPush            sStackPush;
    };
};

typedef Statement*        PStatement;

class CStmtImage
{
public:
    CStmtImage();
    virtual ~CStmtImage();

    Statement *newStmt(int nLineNo, StmtType st, stmt_index &index);

public:
    stmt_index newStmtExpConstInt(int nLineNo, int value);
    stmt_index newStmtExpConstBool(int nLineNo, bool bValue);
    stmt_index newStmtExpNull(int nLineNo);
    stmt_index newStmtExpConstString(int nLineNo, int nStrJObjIndex);
    stmt_index newStmtExpNew(int nLineNo, TypeId typeId, int nMemberIndex, stmt_index siParameter);
    stmt_index newStmtExpVar(int nLineNo, bool bStatic, TypeId typeId, int nAddress, stmt_index siParentVar);
    stmt_index newStmtExpFuncCall(int nLineNo, bool bStatic, TypeId typeIdRet, TypeId typeIdMethodClass, stmt_index siParentVar, int nMemberIndex, stmt_index siParameter);
    stmt_index newStmtExpAssign(int nLineNo, TypeId typeId, stmt_index siVarAddr, stmt_index siExpression);
    stmt_index newStmtExpAssignStaticVarInitial(int nLineNo, TypeId typeId, stmt_index siVarAddr, stmt_index siExpression);
    stmt_index newStmtExpOperate(int nLineNo, ExpressionType typeOperator, TypeId typeIdValue, stmt_index siExprLeft, stmt_index siExprRight, TypeId typeIdOperand);
    stmt_index newStmtExpDynCast(int nLineNo, TypeId typeId, stmt_index siVar);

    stmt_index newStmtTry(int nLineNo, stmt_index siTryStmt, stmt_index siCatch);
    stmt_index newStmtCatch(int nLineNo, TypeId typeIdException, stmt_index siCatchStmt, stmt_index siNextCatch);
    stmt_index newStmtThrow(int nLineNo, stmt_index siNewExpr);
    stmt_index newStmtIf(int nLineNo, stmt_index siCondition, stmt_index siTrueStmt, stmt_index siFalseStmt);
    stmt_index newStmtFor(int nLineNo, stmt_index siInitalization, stmt_index siTermination, stmt_index siIncrement, stmt_index siActionStmt);
    stmt_index newStmtWhile(int nLineNo, stmt_index siCondition, stmt_index siActionStmt);
    stmt_index newStmtDoWhile(int nLineNo, stmt_index siCondition, stmt_index siActionStmt);
    stmt_index newStmtBreak(int nLineNo);
    stmt_index newStmtContinue(int nLineNo);
    stmt_index newStmtReturn(int nLineNo, stmt_index siExpression);
    stmt_index newStmtBlock(int nLineNo, stmt_index &siChildBlock);

    stmt_index newStmtStackPush(int nLineNo, TypeId typeId);

    Statement *getStmt(stmt_index si);
    stmt_index *getTail(stmt_index *psi);
    stmt_index getNext(stmt_index si);

#ifdef _DEBUG
    void printStmtCodeTree(stmt_index si, int &nIndent);
    void printExpression(stmt_index siExpr);
#endif

protected:
    string                m_memStmtOnDisk;
    list<Statement *>    m_stmtNewInMem;
    vector<Statement *>    m_stmtIndex;

};
