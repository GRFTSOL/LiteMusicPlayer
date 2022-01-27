#pragma once

#include "../Utils/Utils.h"
#include "BatchMem.h"

typedef uint16_t        TypeId;
struct STreeNode;

extern int            g_srcLineNo; // source line number for listing
extern STreeNode    *_syntaxTree;
extern CBatchMem    g_memSyntax;

// Syntax tree node = Stn_

struct SJType {
    uint16_t            nLineNo;
    TypeId              typeId;
    char                *szTypeName;

    void operator=(STreeNode *pNodeID);

};

struct StnClass {
    char                *szClassName;
    TypeId              typeIdClass;

    SJType              typeParentClass;

    STreeNode           *pClassMember;
};

struct StnVarDeclare {
    bool            bStatic;
    SJType            type;
    STreeNode        *pVarList;
};

struct StnVar {
    char *           szVarName;
    int                nAddress;
    TypeId            typeId;
    STreeNode        *pExpAssign;        // Assignment expression node
};

struct StnMemberMethod {
    bool            bStatic;
    char *           szMethodName;
    SJType            typeRet;            // 
    class CMemberMethod    *pMethod;
    STreeNode        *pParameterList;    // StnVar
    STreeNode        *pBody;
};

struct StnTry {
    STreeNode        *pTryStmt;
    STreeNode        *pCatch;
};

struct StnCatch {
    STreeNode        *pException;
    STreeNode        *pCatchStmt;
    STreeNode        *pNextCatch;
};

struct StnThrow {
    STreeNode        *pNewExpr;
};

struct StnIf {
    STreeNode        *pCondition;
    STreeNode        *pTrueNode;
    STreeNode        *pFalseNode;
};

struct StnWhile {
    STreeNode        *pCondition;
    STreeNode        *pActionNode;
};

struct StnFor {
    STreeNode        *pInitalization;
    STreeNode        *pTermination;
    STreeNode        *pIncrement;
    STreeNode        *pActionNode;
};

enum ExpressionType {
    E_CONST,
    E_CONST_INT,
    E_CONST_BOOLEAN,
    E_CONST_NULL,
    E_CONST_STRING,
    E_NEW,
    E_DYNAMIC_CAST,
    E_VAR,
    E_METHOD_CALL,
    E_ASSIGN,
    E_ASSIGN_STATIC_VAR_INIT,
    OP_LT_EQ,        // <=
    OP_GT_EQ,        // >=
    OP_LITTLE,        // <
    OP_GREATER,        // >
    OP_EQUAL,        // ==
    OP_NOT_EQUAL,    // !=
    OP_OP_INC,        // ++
    OP_OP_DEC,        // --
    OP_PLUS_ASSIGN, // +=
    OP_MINUS_ASSIGN,// -=
    OP_PLUS,        // +
    OP_MINUS,        // -
    OP_MULT,        // *
    OP_DIV,            // /
    OP_BOOL_AND,    // &&
    OP_BOOL_OR,        // ||
};

struct StnExpression {

    ExpressionType        type;
    TypeId                typeIdValExp;

    union {
        int                    nConstInt;            // E_CONST_INT
        bool                bConstBool;
        char *               szConstString;        // E_CONST_STRING
        struct 
        {
            char *           szNewType;
            int                nMemberIndex;
            STreeNode        *pParameter;        // Parameter of construction
        }                    newObj;                // E_NEW
        struct 
        {
            char *           szCastToType;
            STreeNode        *pVar;
        }                    dynCast;            // E_DYNAMIC_CAST
        struct {
            char *           szVarName;
            int                nAddress;
            bool            bStatic;
            STreeNode        *pVarParent;
        }                    var;                // E_VAR
        struct {
            char *               szFuncName;
            bool                bStatic;
            TypeId                typeIdSpecialfiedClass;    // SI_INVALID, indicate call from typeId of pVarParent.
            int                    nMethodIndex;
            STreeNode            *pVarParent;
            STreeNode            *pParameter;
        }                    functionCall;        // E_METHOD_CALL
        struct 
        {
            STreeNode        *pVar;
            STreeNode        *pRightExp;
        }                    assign;                // E_ASSIGN, E_ASSIGN_STATIC_VAR
        struct {
            STreeNode            *pExpressionLeft;
            STreeNode            *pExpressionRight;
        }                    opParameter;        // OP_XXXX
    };
};

struct StnReturn {
    STreeNode        *pExpression;
};

struct StnBlock {
    STreeNode        *pChildList;
};

struct STreeNode
{
    int            nLineNo;
    STreeNode    *pNext;

    enum Type
    {
        T_STRING,
        T_CLASS,
        T_VAR_DECLAR,
        T_VAR,
        T_MEMBER_METHOD,
        T_TRY,
        T_CATCH,
        T_THROW,
        T_IF,
        T_FOR,
        T_WHILE,
        T_DO_WHILE,
        T_BREAK,
        T_CONTINUE,

        T_EXPRESSION,

        T_RETURN,
        T_BLOCK,
        T_EMPTY,
    };

    Type                nodeType;
    union {
        char *           szString;
        StnClass        sClass;
        StnVarDeclare    sVarDeclare;
        StnVar            sVar;
        StnMemberMethod    sMemberMethod;
        StnTry            sTry;
        StnCatch        sCatch;
        StnThrow        sThrow;
        StnIf            sIf;
        StnWhile        sWhile;
        StnFor            sFor;
        StnExpression    sExpression;

        StnReturn        sReturn;
        StnBlock        sBlock;
    };

    STreeNode(Type _t, int _nLineNo) {
        nodeType = _t;
        nLineNo = _nLineNo;
        pNext = nullptr;
    }

};

typedef STreeNode *    PSTreeNode;

STreeNode *appendToSibling(PSTreeNode &pHead, STreeNode *pTail);
void insertToSibling(PSTreeNode &pHead, STreeNode *pToInsert);

inline void deleteStn(STreeNode *p) { }

STreeNode *newStnString(int nLineNo, cstr_t szString);

STreeNode *newStnVarDeclare(STreeNode *pVarType, STreeNode *pVarNameList, bool bStatic);

STreeNode *newStnVar(STreeNode *pVarIdString, STreeNode *pExp = nullptr);

STreeNode *newStnClassDeclare(STreeNode *pClassNameString, STreeNode *pParentClassString, STreeNode *pClassMember);

void newStnDefConstructorDeclare(cstr_t szName, CMemberMethod *pMethod, int nLineNo);

// For native function, body = nullptr
STreeNode *newStnMethodDeclareNode(STreeNode *pMethodNameString, 
                    STreeNode *pRetTypeString, STreeNode *pParameterList, STreeNode *pBody,
                    bool bStatic = false);

STreeNode *newStnTry(STreeNode *pTryStmt, STreeNode *pCatchSeq);

STreeNode *newStnCatch(STreeNode *pException, STreeNode *pCatchStmt);

STreeNode *appendCatchTail(STreeNode *pCatchSeq, STreeNode *pCatch);

STreeNode *newStnThrow(STreeNode *pExprThrow);

STreeNode *newStnIf(STreeNode *pCondition, STreeNode *pTrueBlock, STreeNode *pFalseBlock);

STreeNode *newStnFor(STreeNode *pInitalization, STreeNode *pTermination, STreeNode *pIncrement, STreeNode *pActionBlock);

STreeNode *newStnWhile(STreeNode *pCondition, STreeNode *pActionBlock);

STreeNode *newStnDoWhile(STreeNode *pCondition, STreeNode *pActionBlock);

STreeNode *newStnBreak(int nLineNo);

STreeNode *newStnContinue(int nLineNo);

STreeNode *newStnExpVar(STreeNode *pVarNameString, STreeNode *pParentVar);
STreeNode *newStnExpVar(int nLineNo, cstr_t szVarName, STreeNode *pParentVar);

STreeNode *newStnExprConstInt(STreeNode *pString);

STreeNode *newStnExprConstBool(int nLineNo, bool bTrue);

STreeNode *newStnExprNull(int nLineNo);

STreeNode *newStnExprConstString(STreeNode *pString);

STreeNode *newStnExpNew(STreeNode *pString, STreeNode *pParameterList);

STreeNode *newStnExpAssign(STreeNode *pTargetVar, STreeNode *pExpression);

STreeNode *newStnExpDynamicCast(STreeNode *pIDStr, STreeNode *pVar);

STreeNode *newStnExpOp(ExpressionType expType, STreeNode *pExpressionLeft, STreeNode *pExpressionRight);

STreeNode *newStnExpFuncCall(int nLineNo, cstr_t szMethodName, STreeNode *pVarToCall, STreeNode *pParameterList);

STreeNode *newStnReturn(int nLineNo, STreeNode *pExpression);

STreeNode *newStnBlock(int nLineNo, STreeNode *pChildList);

void printSyntaxTree(STreeNode *t, int &nIndent);
