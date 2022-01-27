#include "SJVM.h"
#include "SyntaxTree.h"
#include "VMStatment.h"

CStmtImage::CStmtImage()
{
    m_stmtIndex.push_back(nullptr);
}

CStmtImage::~CStmtImage()
{

}

Statement *CStmtImage::newStmt(int nLineNo, StmtType st, stmt_index &index)
{
    Statement        *ps = new Statement();

    ps->nLineNo = nLineNo;
    ps->stmtType = st;
    m_stmtNewInMem.push_back(ps);
    index = (int)m_stmtIndex.size();
    m_stmtIndex.push_back(ps);

    return ps;
}

stmt_index CStmtImage::newStmtExpConstInt(int nLineNo, int value)
{
    stmt_index        si;
    Statement        *ps = newStmt(nLineNo, ST_EXPRESSION, si);

    ps->sExpression.expType = E_CONST;
    ps->sExpression.nConstValue = value;
    ps->sExpression.typeId = TID_INT;

    return si;
}

stmt_index CStmtImage::newStmtExpConstBool(int nLineNo, bool bValue)
{
    stmt_index        si;
    Statement        *ps = newStmt(nLineNo, ST_EXPRESSION, si);

    ps->sExpression.expType = E_CONST;
    ps->sExpression.nConstValue = bValue;
    ps->sExpression.typeId = TID_BOOLEAN;

    return si;
}

stmt_index CStmtImage::newStmtExpNull(int nLineNo)
{
    stmt_index        si;
    Statement        *ps = newStmt(nLineNo, ST_EXPRESSION, si);

    ps->sExpression.expType = E_CONST;
    ps->sExpression.nConstValue = 0;
    ps->sExpression.typeId = TID_NULL_VAL;

    return si;
}

stmt_index CStmtImage::newStmtExpConstString(int nLineNo, int nStrJObjIndex)
{
    stmt_index        si;
    Statement        *ps = newStmt(nLineNo, ST_EXPRESSION, si);

    ps->sExpression.expType = E_CONST;
    ps->sExpression.nConstValue = nStrJObjIndex;
    ps->sExpression.typeId = TID_STRING;

    return si;
}

stmt_index CStmtImage::newStmtExpNew(int nLineNo, TypeId typeId, int nMemberIndex, stmt_index siParameter)
{
    stmt_index        si;
    Statement        *ps = newStmt(nLineNo, ST_EXPRESSION, si);

    ps->sExpression.expType = E_NEW;
    ps->sExpression.typeId = typeId;
    ps->sExpression.newObj.nMemberIndex = nMemberIndex;
    ps->sExpression.newObj.siParameter = siParameter;

    return si;
}

stmt_index CStmtImage::newStmtExpVar(int nLineNo, bool bStatic, TypeId typeId, int nAddress, stmt_index siParentVar)
{
    stmt_index        si;
    Statement        *ps = newStmt(nLineNo, ST_EXPRESSION, si);

    ps->sExpression.expType = E_VAR;
    ps->sExpression.typeId = typeId;
    ps->sExpression.var.nAddress = nAddress;
    ps->sExpression.var.bStatic = bStatic;
    ps->sExpression.var.siParentVar = siParentVar;

    return si;
}

stmt_index CStmtImage::newStmtExpFuncCall(int nLineNo, bool bStatic, TypeId typeIdRet, TypeId typeIdMethodClass, stmt_index siParentVar, int nMemberIndex, stmt_index siParameter)
{
    stmt_index        si;
    Statement        *ps = newStmt(nLineNo, ST_EXPRESSION, si);

    ps->sExpression.expType = E_METHOD_CALL;
    ps->sExpression.typeId = typeIdRet;
    ps->sExpression.functionCall.bStatic = bStatic;
    ps->sExpression.functionCall.typeIdSpecialfiedClass = typeIdMethodClass;
    ps->sExpression.functionCall.nMemberIndex = nMemberIndex;
    ps->sExpression.functionCall.siParentVar = siParentVar;
    ps->sExpression.functionCall.siParameter = siParameter;

    return si;
}

stmt_index CStmtImage::newStmtExpAssign(int nLineNo, TypeId typeId, stmt_index siVarAddr, stmt_index siExpression)
{
    stmt_index        si;
    Statement        *ps = newStmt(nLineNo, ST_EXPRESSION, si);

    ps->sExpression.expType = E_ASSIGN;
    ps->sExpression.typeId = typeId;
    ps->sExpression.assign.siVarAddress = siVarAddr;
    ps->sExpression.assign.siExpression = siExpression;

    return si;
}

stmt_index CStmtImage::newStmtExpAssignStaticVarInitial(int nLineNo, TypeId typeId, stmt_index siVarAddr, stmt_index siExpression)
{
    stmt_index        si;
    Statement        *ps = newStmt(nLineNo, ST_EXPRESSION, si);

    ps->sExpression.expType = E_ASSIGN_STATIC_VAR_INIT;
    ps->sExpression.typeId = typeId;
    ps->sExpression.assign.siVarAddress = siVarAddr;
    ps->sExpression.assign.siExpression = siExpression;

    return si;
}

stmt_index CStmtImage::newStmtExpOperate(int nLineNo, ExpressionType typeOperator, TypeId typeIdValue, stmt_index siExprLeft, stmt_index siExprRight, TypeId typeIdOperand)
{
    stmt_index        si;
    Statement        *ps = newStmt(nLineNo, ST_EXPRESSION, si);

    ps->sExpression.expType = typeOperator;
    ps->sExpression.typeId = typeIdValue;
    ps->sExpression.opParameter.siExprLeft = siExprLeft;
    ps->sExpression.opParameter.siExprRight = siExprRight;
    ps->sExpression.opParameter.typeIdOperand = typeIdOperand;

    return si;
}

stmt_index CStmtImage::newStmtExpDynCast(int nLineNo, TypeId typeId, stmt_index siVar)
{
    stmt_index        si;
    Statement        *ps = newStmt(nLineNo, ST_EXPRESSION, si);

    ps->sExpression.expType = E_DYNAMIC_CAST;
    ps->sExpression.typeId = typeId;
    ps->sExpression.dynCast.siVar = siVar;

    return si;
}

stmt_index CStmtImage::newStmtTry(int nLineNo, stmt_index siTryStmt, stmt_index siCatch)
{
    stmt_index        si;
    Statement        *ps = newStmt(nLineNo, ST_TRY, si);

    ps->sTry.siTryStmt = siTryStmt;
    ps->sTry.siCatch = siCatch;

    return si;
}

stmt_index CStmtImage::newStmtCatch(int nLineNo, TypeId typeIdException, stmt_index siCatchStmt, stmt_index siNextCatch)
{
    stmt_index        si;
    Statement        *ps = newStmt(nLineNo, ST_CATCH, si);

    ps->sCatch.typeIdException = typeIdException;
    ps->sCatch.siCatchStmt = siCatchStmt;
    ps->sCatch.siNextCatch = siNextCatch;

    return si;
}

stmt_index CStmtImage::newStmtThrow(int nLineNo, stmt_index siNewExpr)
{
    stmt_index        si;
    Statement        *ps = newStmt(nLineNo, ST_THROW, si);

    ps->sThrow.siNewExpr = siNewExpr;

    return si;
}

stmt_index CStmtImage::newStmtIf(int nLineNo, stmt_index siCondition, stmt_index siTrueStmt, stmt_index siFalseStmt)
{
    stmt_index        si;
    Statement        *ps = newStmt(nLineNo, ST_IF, si);

    ps->sIf.siCondition = siCondition;
    ps->sIf.siTrueStmt = siTrueStmt;
    ps->sIf.siFalseStmt = siFalseStmt;

    return si;
}

stmt_index CStmtImage::newStmtFor(int nLineNo, stmt_index siInitalization, stmt_index siTermination, stmt_index siIncrement, stmt_index siActionStmt)
{
    stmt_index        si;
    Statement        *ps = newStmt(nLineNo, ST_FOR, si);

    ps->sFor.siInitalization = siInitalization;
    ps->sFor.siTermination = siTermination;
    ps->sFor.siIncrement = siIncrement;
    ps->sFor.siStmt = siActionStmt;

    return si;
}

stmt_index CStmtImage::newStmtWhile(int nLineNo, stmt_index siCondition, stmt_index siActionStmt)
{
    stmt_index        si;
    Statement        *ps = newStmt(nLineNo, ST_WHILE, si);

    ps->sWhile.siCondition = siCondition;
    ps->sWhile.siStmt = siActionStmt;

    return si;
}

stmt_index CStmtImage::newStmtDoWhile(int nLineNo, stmt_index siCondition, stmt_index siActionStmt)
{
    stmt_index        si;
    Statement        *ps = newStmt(nLineNo, ST_DO_WHILE, si);

    ps->sWhile.siCondition = siCondition;
    ps->sWhile.siStmt = siActionStmt;

    return si;
}

stmt_index CStmtImage::newStmtBreak(int nLineNo)
{
    stmt_index        si;
    newStmt(nLineNo, ST_BREAK, si);
    return si;
}

stmt_index CStmtImage::newStmtContinue(int nLineNo)
{
    stmt_index        si;
    newStmt(nLineNo, ST_CONTINUE, si);
    return si;
}

stmt_index CStmtImage::newStmtReturn(int nLineNo, stmt_index siExpression)
{
    stmt_index        si;
    Statement        *ps = newStmt(nLineNo, ST_RETURN, si);

    ps->sRetrun.siExpression = siExpression;

    return si;
}

stmt_index CStmtImage::newStmtBlock(int nLineNo, stmt_index &siChildBlock)
{
    stmt_index        si;
    Statement        *ps = newStmt(nLineNo, ST_BLOCK, si);

    ps->sBlock.siFirstChild = siChildBlock;

    return si;
}

stmt_index CStmtImage::newStmtStackPush(int nLineNo, TypeId typeId)
{
    stmt_index        si;
    Statement        *ps = newStmt(nLineNo, ST_PUSH_STACK, si);

    ps->sStackPush.typeIdVar = typeId;

    return si;
}

Statement *CStmtImage::getStmt(stmt_index si)
{
    assert(si > SI_INVALID && si < m_stmtIndex.size());
    if (si > SI_INVALID && si < m_stmtIndex.size())
        return m_stmtIndex[si];

    return nullptr;
}

stmt_index *CStmtImage::getTail(stmt_index *psi)
{
    assert(psi);

    if (*psi == SI_INVALID)
        return psi;

    Statement        *ps = getStmt(*psi);
    while (ps)
    {
        psi = &(ps->siNextStmt);
        if (ps->siNextStmt == SI_INVALID)
            break;
        ps = getStmt(*psi);
    }

    return psi;
}

stmt_index CStmtImage::getNext(stmt_index si)
{
    if (si == SI_INVALID)
        return si;

    Statement        *ps = getStmt(si);
    if (ps)
        return ps->siNextStmt;

    return si;
}


#ifdef _DEBUG

void printSapces(int nIndent)
{
    for (int i = 0; i < nIndent; i++)
        printf("  ");
}

void CStmtImage::printStmtCodeTree(stmt_index si, int &nIndent)
{
    Statement    *ps = getStmt(si);
    while (ps)
    {
        switch (ps->stmtType)
        {
        case ST_BLOCK:
            {
                StmtBlock    &sBlock = ps->sBlock;
                printSapces(nIndent); printf("{\n");
                nIndent++;
                if (sBlock.siFirstChild != SI_INVALID)
                    printStmtCodeTree(sBlock.siFirstChild, nIndent);
                nIndent--;
                printSapces(nIndent); printf("}\n");
            }
            break;
        case ST_TRY:
            {
                StmtTry    &sTry = ps->sTry;
                printSapces(nIndent); printf("try\n");
                printStmtCodeTree(sTry.siTryStmt, nIndent);

                printStmtCodeTree(sTry.siCatch, nIndent);
            }
            break;
        case ST_CATCH:
            {
                StmtCatch    &sCatch = ps->sCatch;
                printSapces(nIndent); printf("catch(\n");

                if (sCatch.siNextCatch != SI_INVALID)
                    printStmtCodeTree(sCatch.siNextCatch, nIndent);
            }
            break;
        case ST_THROW:
            {
                StmtThrow    &sThrow = ps->sThrow;
                printSapces(nIndent); printf("throw ");
                printStmtCodeTree(sThrow.siNewExpr, nIndent);
            }
            break;
        case ST_IF:
            {
                StmtIf    &sIf = ps->sIf;
                printSapces(nIndent); printf("if (");
                printStmtCodeTree(sIf.siCondition, nIndent);
                printf(")\n");

                if (sIf.siTrueStmt != SI_INVALID)
                {
                    nIndent++;
                    printStmtCodeTree(sIf.siTrueStmt, nIndent);
                    nIndent--;
                }

                if (sIf.siFalseStmt != SI_INVALID)
                {
                    nIndent++;
                    printStmtCodeTree(sIf.siFalseStmt, nIndent);
                    nIndent--;
                }
            }
            break;
        case ST_FOR:
            {
                StmtFor    sFor = ps->sFor;
                printSapces(nIndent); printf("for (");
                printStmtCodeTree(sFor.siInitalization, nIndent);
                printf(";");
                printStmtCodeTree(sFor.siTermination, nIndent);
                printf(";");
                printStmtCodeTree(sFor.siIncrement, nIndent);
                printf(")\n");

                nIndent++;
                if (sFor.siStmt != SI_INVALID)
                    printStmtCodeTree(sFor.siStmt, nIndent);
                nIndent--;
            }
            break;
        case ST_WHILE:
            {
                StmtWhile    sWhile = ps->sWhile;
                printSapces(nIndent); printf("while (");
                printStmtCodeTree(sWhile.siCondition, nIndent);
                printf(")\n");

                nIndent++;
                if (sWhile.siStmt != SI_INVALID)
                    printStmtCodeTree(sWhile.siStmt, nIndent);
                nIndent--;
            }
            break;
        case ST_DO_WHILE:
            {
                StmtWhile    sWhile = ps->sWhile;
                printSapces(nIndent); printf("do\n");
                nIndent++;
                if (sWhile.siStmt != SI_INVALID)
                    printStmtCodeTree(sWhile.siStmt, nIndent);
                nIndent--;

                printSapces(nIndent); printf("while (");
                printExpression(sWhile.siCondition);
                printf(");\n");
            }
            break;
        case ST_BREAK:
            printf("break;");
            break;
        case ST_CONTINUE:
            printf("continue;");
            break;
        case ST_RETURN:
            {
                StmtReturn    &sReturn = ps->sRetrun;
                printSapces(nIndent); printf("return ");
                if (sReturn.siExpression != SI_INVALID)
                    printStmtCodeTree(sReturn.siExpression, nIndent);
                printf(";\n");
            }
            break;
        case ST_PUSH_STACK:
            {
                StmtStackPush    &sStackPush = ps->sStackPush;
                printSapces(nIndent); printf("LocalVarDeclare, type: %d\n", sStackPush.typeIdVar);
            }
            break;
        case ST_EXPRESSION:
            {
                printExpression(si);
            }
            break;
        default:
            assert(0 && "Invalid statement type");
            return;
        }

        if (ps->siNextStmt == SI_INVALID)
            break;
        si = ps->siNextStmt;
        ps = getStmt(ps->siNextStmt);

    }
}


void CStmtImage::printExpression(stmt_index siExpr)
{
    assert(siExpr != SI_INVALID);
    Statement        *ps = getStmt(siExpr);
    assert(ps->stmtType == ST_EXPRESSION);
    StmtExpression    &expr = ps->sExpression;

    switch (expr.expType)
    {
    case E_CONST:
        printf("%d", expr.nConstValue);
        return;
    case E_NEW:
        printf("new %d", expr.typeId);
        return;
    case E_DYNAMIC_CAST:
        printf("DynamicCast to: %d", expr.typeId);
        printExpression(expr.dynCast.siVar);
        return;
    case E_VAR:
        {
            if (expr.var.bStatic)
            {
                printf("(static var)");
            }

            if (expr.var.siParentVar != SI_INVALID)
            {
                // A Jobject member varaible
                printExpression(expr.var.siParentVar);
                printf("(member var) %d ", expr.var.nAddress);
                return;
            }

            // Variable is in stack
            printf("(local var) %d ", expr.var.nAddress);

            return;
        }
    case E_METHOD_CALL:
        {
            // Variable is in stack
            printf("Function: %d ", expr.var.nAddress);
            Statement        *pStmt;

            if (!expr.functionCall.bStatic)
            {
                assert(expr.functionCall.siParentVar != SI_INVALID);
                printExpression(expr.functionCall.siParentVar);
            }

            // execute parameter expressions
            stmt_index        siParameter = expr.functionCall.siParameter;

            printf("Parameter: ");
            // push parameter in stack
            while (siParameter != SI_INVALID)
            {
                pStmt = getStmt(siParameter);
                assert(pStmt);
                printExpression(siParameter);

                siParameter = getNext(siParameter);
            }

            // Call method
            if (expr.functionCall.typeIdSpecialfiedClass != TID_INVALID)
                printf("CallSpecialfiedMethod: %d, %d", expr.functionCall.typeIdSpecialfiedClass, expr.functionCall.nMemberIndex);
            else
                printf("CallJObjectMethod: %d", expr.functionCall.nMemberIndex);

            return;
        }
    case E_ASSIGN:
    case E_ASSIGN_STATIC_VAR_INIT:
        {
            printExpression(expr.assign.siVarAddress);
            printf(" = ");
            printExpression(expr.assign.siExpression);
            return;
        }
        break;
    default:
        break;
    }

    printExpression(expr.opParameter.siExprLeft);

    switch (expr.expType)
    {
    case OP_LT_EQ:        // <=
        printf("<="); return;
    case OP_GT_EQ:        // >=
        printf(">="); return;
    case OP_LITTLE:        // <
        printf("<"); return;
    case OP_GREATER:    // >
        printf(">"); return;
    case OP_EQUAL:        // ==
        printf("=="); return;
    case OP_NOT_EQUAL:    // !=
        printf("!="); return;
    case OP_BOOL_AND:    // &&
        printf("&&"); return;
    case OP_BOOL_OR:    // ||
        printf("||"); return;
    case OP_PLUS:        // +
        printf("+"); return;
    case OP_MINUS:        // -
        printf("-"); return;
    case OP_MULT:        // *
        printf("*"); return;
    case OP_DIV:        // /
        printf("/"); return;
    case OP_OP_INC:        // ++
        printf("++"); return;
    case OP_OP_DEC:        // --
        printf("--"); return;
    case OP_PLUS_ASSIGN:        // +=
        printf("+="); return;
    case OP_MINUS_ASSIGN:        // -=
        printf("-="); return;
    default:
        assert(0 && "Undefined expression type.");
    }

    if (expr.opParameter.siExprRight != SI_INVALID)
        printExpression(expr.opParameter.siExprRight);

    return ;
}

#endif
