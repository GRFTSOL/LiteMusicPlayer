#include "BatchMem.h"
#include "SyntaxTree.h"
#include "SJVM.h"

#pragma warning (disable: 4996)

int            g_srcLineNo = 1; // source line number for listing
STreeNode    *_syntaxTree;
CBatchMem    g_memSyntax;

//
// Using CBatchMem to manage small piece of memory, and free them togother.
//

static char *dupStr(const char *str)
{
    if (str == nullptr)
        return nullptr;

    return g_memSyntax.dupStr(str);
}

static char *dupStnStr(const char *str)
{
    return (char *)str;
}

STreeNode *newSTreeNode(STreeNode::Type _t, int _nLineNo)
{
    STreeNode    *p = (STreeNode *)g_memSyntax.alloc(sizeof(STreeNode));
    p->nodeType = _t;
    p->nLineNo = _nLineNo;
    p->pNext = nullptr;
    return p;
}

STreeNode *appendToSibling(PSTreeNode &pHead, STreeNode *pTail)
{
    if (!pHead)
    {
        pHead = pTail;
        return pHead;
    }

    STreeNode *p = pHead;
    while (p->pNext)
        p = p->pNext;
    p->pNext = pTail;

    return pHead;
}

void insertToSibling(PSTreeNode &pHead, STreeNode *pToInsert)
{
    assert(pHead && pToInsert);
    assert(pToInsert->pNext == nullptr);
    pToInsert->pNext = pHead->pNext;
    pHead->pNext = pToInsert;
}

void SJType::operator=(STreeNode *pNodeID)
{
    assert(pNodeID->nodeType == STreeNode::T_STRING);
    nLineNo = pNodeID->nLineNo;
    szTypeName = dupStnStr(pNodeID->szString);
    typeId = 0;
}

STreeNode *newStnString(int nLineNo, cstr_t szString)
{
    STreeNode    *pNode = newSTreeNode(STreeNode::T_STRING, nLineNo);
    pNode->szString = dupStr(szString);
    return pNode;
}

STreeNode *newStnVarDeclare(STreeNode *pVarType, STreeNode *pVarNameList, bool bStatic)
{
    assert(pVarType->nodeType == STreeNode::T_STRING);

    STreeNode    *pNode = newSTreeNode(STreeNode::T_VAR_DECLAR, pVarType->nLineNo);
    StnVarDeclare &sVarDeclare = pNode->sVarDeclare;
    sVarDeclare.bStatic = bStatic;
    sVarDeclare.pVarList = pVarNameList;
    sVarDeclare.type = pVarType;

    deleteStn(pVarType);

    return pNode;
}

STreeNode *newStnVar(STreeNode *pVarIdString, STreeNode *pExp)
{
    assert(pVarIdString->nodeType == STreeNode::T_STRING);

    STreeNode    *pNode = newSTreeNode(STreeNode::T_VAR, pVarIdString->nLineNo);
    StnVar        &sVar = pNode->sVar;
    sVar.szVarName = dupStnStr(pVarIdString->szString);
    sVar.pExpAssign = pExp;
    sVar.typeId = 0;
    sVar.nAddress = -1;

    deleteStn(pVarIdString);

    return pNode;
}

STreeNode *newStnClassDeclare(STreeNode *pClassNameString, STreeNode *pParentClassString, STreeNode *pClassMember)
{
    assert(pClassNameString->nodeType == STreeNode::T_STRING);

    STreeNode    *pNode = newSTreeNode(STreeNode::T_CLASS, pClassNameString->nLineNo);
    StnClass    &sClass = pNode->sClass;
    sClass.szClassName = dupStnStr(pClassNameString->szString);
    sClass.pClassMember = pClassMember;
    if (pParentClassString)
    {
        assert(pParentClassString->nodeType == STreeNode::T_STRING);
        sClass.typeParentClass = pParentClassString;
        deleteStn(pParentClassString);
    }
    else
        sClass.typeParentClass.szTypeName = nullptr;
    sClass.typeIdClass = 0;

    deleteStn(pClassNameString);

    return pNode;
}

void newStnDefConstructorDeclare(cstr_t szName, CMemberMethod *pMethod, int nLineNo)
{
    pMethod->pBodyNode = newStnBlock(nLineNo, nullptr);
}

// For native function, body = nullptr
STreeNode *newStnMethodDeclareNode(STreeNode *pMethodNameString, 
                                   STreeNode *pRetTypeString, STreeNode *pParameterList, STreeNode *pBody,
                                   bool bStatic)
{
    assert(pMethodNameString->nodeType == STreeNode::T_STRING);

    STreeNode    *pNode = newSTreeNode(STreeNode::T_MEMBER_METHOD, pMethodNameString->nLineNo);
    StnMemberMethod    &sMethod = pNode->sMemberMethod;
    sMethod.bStatic = bStatic;
    sMethod.pBody = pBody;
    sMethod.pParameterList = pParameterList;
    sMethod.szMethodName = dupStnStr(pMethodNameString->szString);
    sMethod.pMethod = nullptr;

    if (pRetTypeString)
    {
        assert(pRetTypeString->nodeType == STreeNode::T_STRING);
        sMethod.typeRet = pRetTypeString;
        deleteStn(pRetTypeString);
    }
    else
    {
        sMethod.typeRet.typeId = 0;
        sMethod.typeRet.szTypeName = nullptr;
    }

    deleteStn(pMethodNameString);

    return pNode;
}

STreeNode *newStnTry(STreeNode *pTryStmt, STreeNode *pCatchSeq)
{
    STreeNode    *pNode = newSTreeNode(STreeNode::T_TRY, pTryStmt->nLineNo);
    StnTry        &sTry = pNode->sTry;

    sTry.pTryStmt = pTryStmt;
    sTry.pCatch = pCatchSeq;

    return pNode;
}

STreeNode *newStnCatch(STreeNode *pException, STreeNode *pCatchStmt)
{
    STreeNode    *pNode = newSTreeNode(STreeNode::T_CATCH, pException->nLineNo);
    StnCatch    &sCatch = pNode->sCatch;

    sCatch.pException = pException;
    sCatch.pCatchStmt = pCatchStmt;
    sCatch.pNextCatch = nullptr;

    return pNode;
}

STreeNode *appendCatchTail(STreeNode *pCatchSeq, STreeNode *pCatch)
{
    assert(pCatchSeq->nodeType == STreeNode::T_CATCH);
    pCatchSeq->sCatch.pNextCatch = pCatch;

    return pCatchSeq;
}

STreeNode *newStnThrow(STreeNode *pExprThrow)
{
    STreeNode    *pNode = newSTreeNode(STreeNode::T_THROW, pExprThrow->nLineNo);
    StnThrow    &sThrow = pNode->sThrow;

    sThrow.pNewExpr = pExprThrow;

    return pNode;
}

STreeNode *newStnIf(STreeNode *pCondition, STreeNode *pTrueBlock, STreeNode *pFalseBlock)
{
    STreeNode    *pNode = newSTreeNode(STreeNode::T_IF, pCondition->nLineNo);
    StnIf        &sIf = pNode->sIf;

    sIf.pCondition = pCondition;
    sIf.pTrueNode = pTrueBlock;
    sIf.pFalseNode = pFalseBlock;

    return pNode;
}

STreeNode *newStnFor(STreeNode *pInitalization, STreeNode *pTermination, STreeNode *pIncrement, STreeNode *pActionBlock)
{
    STreeNode    *pNode = newSTreeNode(STreeNode::T_FOR, pInitalization->nLineNo);
    StnFor        &sFor = pNode->sFor;

    sFor.pInitalization = pInitalization;
    sFor.pTermination = pTermination;
    sFor.pIncrement = pIncrement;
    sFor.pActionNode = pActionBlock;

    return pNode;
}

STreeNode *newStnWhile(STreeNode *pCondition, STreeNode *pActionBlock)
{
    STreeNode    *pNode = newSTreeNode(STreeNode::T_WHILE, pCondition->nLineNo);
    StnWhile    &sWhile = pNode->sWhile;

    sWhile.pCondition = pCondition;
    sWhile.pActionNode = pActionBlock;

    return pNode;
}

STreeNode *newStnDoWhile(STreeNode *pCondition, STreeNode *pActionBlock)
{
    STreeNode    *pNode = newSTreeNode(STreeNode::T_DO_WHILE, pCondition->nLineNo);
    StnWhile    &sWhile = pNode->sWhile;

    sWhile.pCondition = pCondition;
    sWhile.pActionNode = pActionBlock;

    return pNode;
}

STreeNode *newStnBreak(int nLineNo)
{
    return newSTreeNode(STreeNode::T_BREAK, nLineNo);
}

STreeNode *newStnContinue(int nLineNo)
{
    return newSTreeNode(STreeNode::T_CONTINUE, nLineNo);
}

STreeNode *newStnExpAssign(STreeNode *pTargetVar, STreeNode *pExpression)
{
    STreeNode    *pNode = newSTreeNode(STreeNode::T_EXPRESSION, pTargetVar->nLineNo);
    StnExpression    &sExp = pNode->sExpression;
    sExp.type = E_ASSIGN;

    sExp.assign.pRightExp = pExpression;
    sExp.assign.pVar = pTargetVar;

    return pNode;
}

STreeNode *newStnExpVar(int nLineNo, cstr_t szVarName, STreeNode *pParentVar)
{
    STreeNode    *pNode = newSTreeNode(STreeNode::T_EXPRESSION, nLineNo);
    StnExpression    &sExp = pNode->sExpression;
    sExp.type = E_VAR;
    sExp.var.szVarName = dupStr(szVarName);
    sExp.var.pVarParent = pParentVar;
    sExp.var.nAddress = -1;
    sExp.var.bStatic = false;

    return pNode;
}

STreeNode *newStnExpVar(STreeNode *pVarNameString, STreeNode *pParentVar)
{
    assert(pVarNameString->nodeType == STreeNode::T_STRING);

    STreeNode    *pNode = newStnExpVar(pVarNameString->nLineNo, pVarNameString->szString, pParentVar);

    deleteStn(pVarNameString);

    return pNode;
}

STreeNode *newStnExprConstInt(STreeNode *pString)
{
    assert(pString->nodeType == STreeNode::T_STRING);

    STreeNode    *pNode = newSTreeNode(STreeNode::T_EXPRESSION, pString->nLineNo);
    StnExpression    &sExp = pNode->sExpression;
    sExp.type = E_CONST_INT;
    sExp.nConstInt = atoi(pString->szString);

    deleteStn(pString);

    return pNode;
}

STreeNode *newStnExprConstBool(int nLineNo, bool bTrue)
{
    STreeNode    *pNode = newSTreeNode(STreeNode::T_EXPRESSION, nLineNo);
    StnExpression    &sExp = pNode->sExpression;
    sExp.type = E_CONST_BOOLEAN;
    sExp.bConstBool = bTrue;

    return pNode;
}

STreeNode *newStnExprNull(int nLineNo)
{
    STreeNode    *pNode = newSTreeNode(STreeNode::T_EXPRESSION, nLineNo);
    StnExpression    &sExp = pNode->sExpression;
    sExp.type = E_CONST_NULL;

    return pNode;
}

STreeNode *newStnExprConstString(STreeNode *pString)
{
    assert(pString->nodeType == STreeNode::T_STRING);

    STreeNode    *pNode = newSTreeNode(STreeNode::T_EXPRESSION, pString->nLineNo);
    StnExpression    &sExp = pNode->sExpression;
    sExp.type = E_CONST_STRING;
    sExp.szConstString = dupStnStr(pString->szString);

    deleteStn(pString);

    return pNode;
}

STreeNode *newStnExpNew(STreeNode *pString, STreeNode *pParameterList)
{
    assert(pString->nodeType == STreeNode::T_STRING);

    STreeNode    *pNode = newSTreeNode(STreeNode::T_EXPRESSION, pString->nLineNo);
    StnExpression    &sExp = pNode->sExpression;
    sExp.type = E_NEW;
    sExp.newObj.szNewType = dupStnStr(pString->szString);
    sExp.newObj.pParameter = pParameterList;

    deleteStn(pString);

    return pNode;
}

STreeNode *newStnExpDynamicCast(STreeNode *pIDStr, STreeNode *pVar)
{
    assert(pIDStr->nodeType == STreeNode::T_STRING);

    STreeNode    *pNode = newSTreeNode(STreeNode::T_EXPRESSION, pIDStr->nLineNo);
    StnExpression    &sExp = pNode->sExpression;
    sExp.type = E_DYNAMIC_CAST;
    sExp.dynCast.szCastToType = dupStnStr(pIDStr->szString);
    sExp.dynCast.pVar = pVar;

    deleteStn(pIDStr);

    return pNode;
}

STreeNode *newStnExpOp(ExpressionType expType, STreeNode *pExpressionLeft, STreeNode *pExpressionRight)
{
    STreeNode    *pNode = newSTreeNode(STreeNode::T_EXPRESSION, pExpressionLeft->nLineNo);
    StnExpression    &sExp = pNode->sExpression;
    sExp.type = expType;
    sExp.opParameter.pExpressionLeft = pExpressionLeft;
    sExp.opParameter.pExpressionRight = pExpressionRight;

    return pNode;
}

STreeNode *newStnExpFuncCall(int nLineNo, cstr_t szMethodName, STreeNode *pVarToCall, STreeNode *pParameterList)
{
    STreeNode    *pNode = newSTreeNode(STreeNode::T_EXPRESSION, nLineNo);
    StnExpression    &sExp = pNode->sExpression;
    sExp.type = E_METHOD_CALL;
    sExp.functionCall.szFuncName = dupStr(szMethodName);
    sExp.functionCall.pVarParent = pVarToCall;
    sExp.functionCall.pParameter = pParameterList;

    return pNode;
}

STreeNode *newStnReturn(int nLineNo, STreeNode *pExpression)
{
    STreeNode    *pNode = newSTreeNode(STreeNode::T_RETURN, nLineNo);
    StnReturn    &sReturn = pNode->sReturn;

    sReturn.pExpression = pExpression;

    return pNode;
}

STreeNode *newStnBlock(int nLineNo, STreeNode *pChildList)
{
    STreeNode    *pNode = newSTreeNode(STreeNode::T_BLOCK, nLineNo);
    StnBlock    &sBlock = pNode->sBlock;

    sBlock.pChildList = pChildList;

    return pNode;
}

void printSapces(int nIndent, int nLineNo)
{
    printf("line: %03d  ", nLineNo);
    for (int i = 0; i < nIndent; i++)
        printf("  ");
}

void printSyntaxTree(STreeNode *t, int &nIndent)
{
    while (t != nullptr)
    {
        int        nIndentOld = nIndent;
        switch (t->nodeType)
        {
        case STreeNode::T_STRING:
            printf(" %s ", t->szString);
            break;
        case STreeNode::T_CLASS:
            nIndent++;
            printSapces(nIndent, t->nLineNo);
            if (t->sClass.typeParentClass.szTypeName)
                printf("class %s { \n", t->sClass.szClassName);
            else
                printf("class %s extends %s { \n", t->sClass.szClassName, t->sClass.typeParentClass.szTypeName);
            if (t->sClass.pClassMember)
                printSyntaxTree(t->sClass.pClassMember, nIndent);
            printSapces(nIndent, t->nLineNo);
            printf("}\n");
            break;
        case STreeNode::T_VAR_DECLAR:
            {
                printSapces(nIndent, t->nLineNo);
                printf("%s    ", t->sVarDeclare.type.szTypeName);
                STreeNode    *pVar = t->sVarDeclare.pVarList;
                while (pVar) {
                    printf("%s(%d), ", pVar->sVar.szVarName, pVar->sVar.nAddress);
                    if (pVar->sVar.pExpAssign)
                        printSyntaxTree(pVar->sVar.pExpAssign, nIndent);
                    pVar = pVar->pNext;
                }
                printf(";\n");
            }
            break;
        case STreeNode::T_VAR:
            printf("%s(%d), ", t->sVar.szVarName, t->sVar.nAddress);
            if (t->sVar.pExpAssign)
            {
                printf(" = ");
                printSyntaxTree(t->sVar.pExpAssign, nIndent);
            }
            break;
        case STreeNode::T_MEMBER_METHOD:
            {
                nIndent++;
                printSapces(nIndent, t->nLineNo);
                if (t->sMemberMethod.bStatic) printf("static ");
                if (t->sMemberMethod.pBody == nullptr) printf("native ");
                printf("%s %s(\n", t->sMemberMethod.typeRet.szTypeName, t->sMemberMethod.szMethodName);
                if (t->sMemberMethod.pParameterList)
                    printSyntaxTree(t->sMemberMethod.pParameterList, nIndent);
                printSapces(nIndent, t->nLineNo);
                printf(") {\n");
                nIndent++;
                if (t->sMemberMethod.pBody)
                    printSyntaxTree(t->sMemberMethod.pBody, nIndent);
                nIndent--;
                printSapces(nIndent, t->nLineNo);
                printf("}\n");
            }
            break;
        case STreeNode::T_TRY:
            {
                printSapces(nIndent, t->nLineNo);
                printf("try\n");
                if (t->sTry.pTryStmt)
                    printSyntaxTree(t->sTry.pTryStmt, nIndent);
                assert(t->sTry.pCatch);
                printSyntaxTree(t->sTry.pCatch, nIndent);
            }
            break;
        case STreeNode::T_CATCH:
            {
                printSapces(nIndent, t->nLineNo);
                printf("catch\n");
                if (t->sCatch.pException)
                    printSyntaxTree(t->sCatch.pException, nIndent);
                assert(t->sCatch.pCatchStmt);
                printSyntaxTree(t->sCatch.pCatchStmt, nIndent);

                if (t->sCatch.pNextCatch)
                    printSyntaxTree(t->sCatch.pNextCatch, nIndent);
            }
            break;
        case STreeNode::T_THROW:
            {
                printSapces(nIndent, t->nLineNo);
                printf("throw");
                if (t->sThrow.pNewExpr)
                    printSyntaxTree(t->sThrow.pNewExpr, nIndent);
            }
            break;
        case STreeNode::T_IF:
            {
                printSapces(nIndent, t->nLineNo);
                printf("if (");
                if (t->sIf.pCondition)
                    printSyntaxTree(t->sIf.pCondition, nIndent);
                printf(") {\n");
                if (t->sIf.pTrueNode)
                {
                    nIndent++;
                    printSyntaxTree(t->sIf.pTrueNode, nIndent);
                    nIndent--;
                    printSapces(nIndent, t->nLineNo);
                    printf("}\n");
                }
                if (t->sIf.pFalseNode)
                {
                    nIndent++;
                    printSapces(nIndent, t->nLineNo);
                    printf("else {\n");
                    printSyntaxTree(t->sIf.pFalseNode, nIndent);
                    nIndent--;
                    printSapces(nIndent, t->nLineNo);
                    printf("}\n");
                }
            }
            break;
        case STreeNode::T_FOR:
            {
                printSapces(nIndent, t->nLineNo);
                printf("for (");
                if (t->sFor.pInitalization) printSyntaxTree(t->sFor.pInitalization, nIndent);
                printf(";");
                if (t->sFor.pTermination) printSyntaxTree(t->sFor.pTermination, nIndent);
                printf(";");
                if (t->sFor.pIncrement) printSyntaxTree(t->sFor.pIncrement, nIndent);
                printf(") {\n");
                if (t->sFor.pActionNode)
                {
                    nIndent++;
                    printSyntaxTree(t->sFor.pActionNode, nIndent);
                    nIndent--;
                }
                printSapces(nIndent, t->nLineNo);
                printf("}\n");
            }
            break;
        case STreeNode::T_WHILE:
            {
                printSapces(nIndent, t->nLineNo);
                printf("while (");
                if (t->sWhile.pCondition)
                    printSyntaxTree(t->sWhile.pCondition, nIndent);
                printf(") {\n");
                if (t->sWhile.pActionNode)
                {
                    nIndent++;
                    printSyntaxTree(t->sWhile.pActionNode, nIndent);
                    nIndent--;
                }
                printSapces(nIndent, t->nLineNo);
                printf("}\n");
            }
            break;
        case STreeNode::T_DO_WHILE:
            {
                printSapces(nIndent, t->nLineNo);
                printf("do {");
                if (t->sWhile.pActionNode)
                {
                    nIndent++;
                    printSyntaxTree(t->sWhile.pActionNode, nIndent);
                    nIndent--;
                }
                printSapces(nIndent, t->nLineNo);
                printf("}\n");
                printSapces(nIndent, t->nLineNo);
                printf("while (");
                if (t->sWhile.pCondition)
                    printSyntaxTree(t->sWhile.pCondition, nIndent);
                printf(")\n");
            }
            break;
        case STreeNode::T_EXPRESSION:
            {
                if (t->sExpression.type == E_CONST_INT)
                {
                    printf("%d", t->sExpression.nConstInt);
                    return;
                }
                else if (t->sExpression.type == E_CONST_BOOLEAN)
                {
                    printf("%s", t->sExpression.bConstBool ? "true" : "false");
                    return;
                }
                else if (t->sExpression.type == E_CONST_NULL)
                {
                    printf("null");
                    return;
                }
                else if (t->sExpression.type == E_CONST_STRING)
                {
                    printf("%s", t->sExpression.szConstString);
                    return;
                }
                else if (t->sExpression.type == E_NEW)
                {
                    printf("new %s\n", t->sExpression.newObj.szNewType);
                    return;
                }
                else if (t->sExpression.type == E_DYNAMIC_CAST)
                {
                    printf("DynamicCast %s\n", t->sExpression.dynCast.szCastToType);
                    return;
                }
                else if (t->sExpression.type == E_VAR)
                {
                    STreeNode    *pParent = t->sExpression.var.pVarParent;
                    if (pParent)
                    {
                        assert(pParent->sExpression.type == E_VAR);
                        printSyntaxTree(pParent, nIndent);
                        printf(".");
                    }
                    printf("%s(%d%s)", t->sExpression.var.szVarName, t->sExpression.var.nAddress, t->sExpression.var.bStatic ? " Static" : "");
                    return;
                }
                else if (t->sExpression.type == E_METHOD_CALL)
                {
                    printSapces(nIndent, t->nLineNo);
                    if (t->sExpression.functionCall.pVarParent)
                    {
                        printSyntaxTree(t->sExpression.functionCall.pVarParent, nIndent);
                        printf(".");
                    }
                    printf("%s(", t->sExpression.functionCall.szFuncName);

                    if (t->sExpression.functionCall.pParameter)
                    {
                        printSyntaxTree(t->sExpression.functionCall.pParameter, nIndent);
                    }
                    printf(");\n");
                    return;
                }
                else if (t->sExpression.type == E_ASSIGN || t->sExpression.type == E_ASSIGN_STATIC_VAR_INIT)
                {
                    printSapces(nIndent, t->nLineNo);
                    if (t->sExpression.assign.pVar)
                        printSyntaxTree(t->sExpression.assign.pVar, nIndent);
                    printf(" = ");
                    if (t->sExpression.assign.pRightExp)
                        printSyntaxTree(t->sExpression.assign.pRightExp, nIndent);
                    printf("\n");
                    return;
                }

                if (t->sExpression.opParameter.pExpressionLeft)
                    printSyntaxTree(t->sExpression.opParameter.pExpressionLeft, nIndent);
                switch (t->sExpression.type) {
                    case OP_LT_EQ:        printf("<=");    break;    // <=
                    case OP_GT_EQ:        printf(">=");    break;    // >=
                    case OP_LITTLE:        printf("<");    break;    // <
                    case OP_GREATER:    printf(">");    break;    // >
                    case OP_EQUAL:        printf("==");    break;    // ==
                    case OP_NOT_EQUAL:    printf("!=");    break;    // !=
                    case OP_OP_INC:        printf("++");    break;    // ++
                    case OP_OP_DEC:        printf("--");    break;    // --
                    case OP_PLUS:        printf("+");    break;    // +
                    case OP_MINUS:        printf("-");    break;    // -
                    case OP_MULT:        printf("*");    break;    // *
                    case OP_DIV:        printf("/");    break;    // /
                    case OP_BOOL_AND:    printf("&&");    break;    // &&
                    case OP_BOOL_OR:    printf("||");    break;    // ||
                    case OP_PLUS_ASSIGN:printf("+=");    break;    // +=
                    case OP_MINUS_ASSIGN:printf("-=");    break;    // -=
                    default:
                        assert(0 && "Invalid expression Type."); break;
                }
                if (t->sExpression.opParameter.pExpressionRight)
                    printSyntaxTree(t->sExpression.opParameter.pExpressionRight, nIndent);
            }
            break;
        case STreeNode::T_RETURN:
            {
                printSapces(nIndent, t->nLineNo);
                printf("return ");
                if (t->sReturn.pExpression)
                {
                    printSyntaxTree(t->sReturn.pExpression, nIndent);
                }
                printf("\n");
            }
            break;
        case STreeNode::T_BLOCK:
            {
                printSapces(nIndent, t->nLineNo);
                printf("{\n");
                if (t->sBlock.pChildList)
                {
                    nIndent++;
                    printSyntaxTree(t->sBlock.pChildList, nIndent);
                    nIndent--;
                }
                printSapces(nIndent, t->nLineNo);
                printf("}\n");
            }
            break;
        default:
            printf("Unknown Node type: %d, at line: %d\n", t->nodeType, t->nLineNo);
            break;
        }

        t = t->pNext;
        nIndent = nIndentOld;
    }

}
