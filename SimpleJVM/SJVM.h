#pragma once

#include "SyntaxTree.h"
#include "VMStatment.h"

enum SIMPLE_JAVA_COMPILE_ERROR
{
    ERR_C_UNDECLARED_ID        = 3000,
    ERR_C_UNDECLARED_TYPE,
    ERR_C_UNDECLARED_METHOD,
    ERR_C_UNDECLARED_VAR,
    ERR_C_UNDECLARED_MEMBER_VAR,
    ERR_C_PARSE,
    ERR_C_REDEFINITION_ID,
    ERR_C_OP_TYPE_NOT_MATCH,
    ERR_C_OP_REQUIRE_INT_STR_TYPE,
    ERR_C_OP_REQUIRE_INT_TYPE,
    ERR_C_OP_REQUIRE_BOOL_TYPE,
    ERR_C_OP_MUST_TARGET_VAR,
    ERR_C_REDEFINITION_TYPE,
    ERR_C_REDEFINITION_METHOD,
    ERR_C_CONVERT_VALUE,
    ERR_C_UNEXPECTED_ERR,
    ERR_C_FUNC_RETURN_VALUE,
    ERR_C_NEW_WRONG_TYPE,
    ERR_C_NOT_CLASS_TYPE,
    ERR_C_CAN_NOT_CALL_NONE_STATIC_MEMBER,
    ERR_C_REQUIRE_LOGICAL_CONDITION,
    ERR_C_METHOD_CALL_PARAMETER_COUNT,
    ERR_C_CLASS_TYPE_NOT_VAR,
    ERR_C_CLASS_HIERARCHY_INCONSISTENT,
    ERR_C_OVERRIDE_METHOD_RET_DIFF,
    ERR_C_DUMPLICATE_METHOD,
    ERR_C_CONSTRUCTOR_MUST_BE_FIRST_STMT,
    ERR_C_CONSTRUCTOR_OBJ_UNDEFINED,
    ERR_C_CONSTRUCTOR_NO_RETRUN_VALUE,
    ERR_C_RETRUN_TYPE_MISSING,
    ERR_C_NO_THIS_SUPER_IN_STATIC_METHOD,
    ERR_C_BREAK_CONTINUE_MUST_IN_LOOP,

    ERR_R_NO_ENTRY_POINT,
};


enum TypeIdPredefined
{
    TID_INVALID = 0,
    TID_VOID,
    TID_CHAR,
    TID_BOOLEAN,
    TID_SHORT,
    TID_INT,
    TID_NULL_VAL,
    TID_STRING,
};

#define TID_EXCEPTION        CSjobjException::m_sTypeId

inline bool isSimpleType(TypeId typeId) { return typeId < TID_NULL_VAL; }

#define JOBJ_NULL        0

#define SZ_THIS        "this"
#define    SZ_SUPER    "super"
#define SZ_NULL        "null"

// #define SJNI_API            __stdcall
#define SJNI_API

typedef int                    jobject;

typedef vector<TypeId>        VecParamType;
typedef vector<int>            VecInt;
typedef vector<bool>        VecBool;
typedef vector<string>        VecStrings;
typedef map<jobject, int>    MapJobjRef;

typedef map<string, int>    MapStrIndex;

struct Variable {
    int            value;
    TypeId        typeId;
};
typedef vector<Variable>    VecVariable;

struct StaticVar {
    bool    bInitialized;
    TypeId    typeId;
    int        value;
};
typedef vector<StaticVar>    VecStaticVar;

class CJObjectNewer
{
public:
    CJObjectNewer() { }
    virtual ~CJObjectNewer() { }

    virtual class CJObject *newObj() = 0;

};

class CMemberMethod
{
public:
    CMemberMethod() {
        nLineNo = -1;
        nVirtualIndex = -1;
        siMethod = SI_INVALID;
        pNativeFunc = nullptr;
        typeIDRet = TID_INVALID;
        pBodyNode = nullptr;
        bStatic = false;
    }

public:
    int                        nLineNo;
    int                        nVirtualIndex;
    VecParamType            vParamType;
    TypeId                    typeIDRet;
    bool                    bStatic;        // Is Static function?
    string                    name;

    STreeNode                *pBodyNode;

    void *                   pNativeFunc;

    stmt_index                siMethod;

public:
    void addParam(TypeId typeId) { vParamType.push_back(typeId); }

    bool isSameNameAndParameter(cstr_t szMethodName, VecParamType &vParameter);

    bool isSameNameAndParameter(CMemberMethod *pMethod);

    bool isSame(CMemberMethod *pMethod);

};

typedef vector<CMemberMethod *>            VecMethod;

class CMemberVar
{
public:
    TypeId                    typeId;
    bool                    bStatic;        // Is Static variable?
    int                        nAddr;            // Static address or index in CJObject.
    string                    name;

};
typedef vector<CMemberVar *>        VecMemberVar;

class CClass
{
public:
    CClass() {
        nLineNo = -1;
        pClassParent = nullptr;
        typeId = TID_INVALID;
        bFreeJObjNewer = false;
        nTotalMemberVarCount = 0;
        pJObojNewer = nullptr;
    }
    virtual ~CClass() {
        if (bFreeJObjNewer)
            delete pJObojNewer;
    }

public:
    int                        nLineNo;
    TypeId                    typeId;
    CClass                    *pClassParent;
    VecMethod                vMethod;
    VecMethod                vVirtualMethod;
    CMemberMethod            methodVarInit;
    VecMemberVar            vMemberVar;
    int                        nTotalMemberVarCount;
    string                    name;
    CJObjectNewer            *pJObojNewer;
    bool                    bFreeJObjNewer;

public:
    void addNativeDefaultConstructor();

    // Return the index of method, -1 for error.
    int addNativeMethod(bool bStatic, cstr_t szMethodName, void *funcPtr, TypeId typeIdReturn, ...);

    int addMethod(CMemberMethod *pMethod);

    int addMemberVar(TypeId typeId, cstr_t szVarName, bool bStatic, int nStaticAddr);

    void updateTotalMemberCount();

    bool isMemberVarStatic(cstr_t szVarName);
    CMemberVar *getMemberVar(cstr_t szVarName);

    CMemberMethod *getMethod(cstr_t szMethodName, VecParamType &vParameter);

    CMemberMethod *getMethod(size_t nMethodIndex) {
        assert(nMethodIndex < vVirtualMethod.size());
        if (nMethodIndex >= vVirtualMethod.size())
            return nullptr;
        return vVirtualMethod[nMethodIndex];
    }

    CMemberMethod *getVarInitMethod() { return &methodVarInit; }

    CMemberMethod *getNoParameterConstructor();
    bool hasConstructor();

    int updateVirtualMethodTable(string &strMsg);

    void updateMemberVarAddress();

};
typedef vector<CClass *>    VecClass;

class CClassTable
{
public:
    CClassTable();
    virtual ~CClassTable();

    int addClass(cstr_t szName, int nLineNo = -1, CJObjectNewer *pJObjNewer = nullptr);

    CClass *getClass(cstr_t szClassName);
    CClass *getClass(TypeId typeId);

    bool isClassDeclared(cstr_t szClassName);

    bool isSubClassOf(TypeId typeChild, TypeId typeParent);

    CMemberMethod *getEntryPoint();
    CMemberMethod *getClassStaticVarInitMethod() { return &m_classStaticVarInitMetod; }

    int standardizeClassMember(string strMsg);

    void updateJObjNewer();

#ifdef _DEBUG
    void print(class CSJVM *pVm);
#endif

    VecClass                m_vClass;
    CMemberMethod            m_classStaticVarInitMetod;

    typedef list<CJObjectNewer*>        ListJObjNewer;

    ListJObjNewer            m_listJObjNewer;

};

#include "VarDeclStack.h"

class CJObject
{
public:
    TypeId                m_typeId;
    VecVariable            m_vMemberVar;

    CJObject(TypeId typeId, int nMemberVarCount) { 
        m_typeId = typeId;
        Variable    var;
        var.value = 0;
        var.typeId = TID_INVALID;
        m_vMemberVar.resize(nMemberVarCount, var);
    }
    virtual ~CJObject() { }

    int getValue(size_t nPos) {
        assert(nPos < m_vMemberVar.size());
        if (nPos >= m_vMemberVar.size())
            return 0;
        return m_vMemberVar[nPos].value;
    }

    void setValue(size_t nPos, int value) {

        assert(nPos < m_vMemberVar.size());
        if (nPos >= m_vMemberVar.size())
            return;
        m_vMemberVar[nPos].value = value;
    }

    int *getAddress(size_t nPos) {
        assert(nPos < m_vMemberVar.size());
        if (nPos >= m_vMemberVar.size())
            return nullptr;
        return &(m_vMemberVar[nPos].value);
    }

};

class CStrJObject : public CJObject
{
public:
    CStrJObject(cstr_t szStr, bool bCopy = true);
    virtual ~CStrJObject();

    cstr_t getStrValue() { return m_szValue; }

protected:
    cstr_t                m_szValue;
    bool                m_bCopy;

};

typedef map<int, CJObject *>    MapJObject;
typedef vector<CJObject *>    VecJObject;

class CObjectMgr
{
public:
    CObjectMgr() { 
        // Take 0 as invalid address
        m_vJObjects.push_back(nullptr);
        m_nAllocCount = 0;
    }
    virtual ~CObjectMgr();

    void reset();

    int New(CClass *pClass);

    int newString(cstr_t szString, bool bCopy = true);

    CJObject *get(int nPos);

    void initAddConstString(MapStrIndex &mapStr);

protected:
    friend class CSJVMGarbageCoolector;
    friend class CSjobjUnitTest;

    VecJObject                m_vJObjects;
    list<int>                m_vFreeJObjs;
    int                        m_nAllocCount;

};

class CVMStack
{
public:
    void insert(int nPos, int value, TypeId typeId) {
        Variable i; i.value = value; i.typeId = typeId; m_vItems.insert(m_vItems.begin() + nPos, i);
    }

    void push(TypeId typeId, int value) { 
        Variable i; i.value = value; i.typeId = typeId; m_vItems.push_back(i); }
    void pop() { m_vItems.pop_back(); }
    void popN(size_t n) {
        assert(n < m_vItems.size());
        m_vItems.resize(m_vItems.size() - n);
    }

    void push(class CVMParameters &parameters);

    void resize(size_t s) { m_vItems.resize(s); }

    Variable &back() { assert(m_vItems.size() > 0); return m_vItems.back(); }

    int getValue(size_t nOffset)    {
        assert(nOffset < (int)m_vItems.size());
        return m_vItems[nOffset].value;
    }

    Variable &get(size_t nOffset)    {
        assert(nOffset < (int)m_vItems.size());
        return m_vItems[nOffset];
    }

    size_t size() { return m_vItems.size(); }

protected:
    friend class CSJVM;
    VecVariable            m_vItems;

};

class CVMParameters
{
public:
    enum { COUNT_MAX        = 32 };

    CVMParameters() { m_nCount = 0; }

    void push(TypeId typeId, int value) { 
        assert(m_nCount < COUNT_MAX);
        m_vItems[m_nCount].value = value;
        m_vItems[m_nCount].typeId = typeId;
        m_nCount++;
    }

public:
    Variable            m_vItems[COUNT_MAX];
    size_t                m_nCount;

};

class CSJVM
{
public:
    CSJVM(void);
    virtual ~CSJVM(void);

    void reset();

    int compileFiles(VecStrings &vFiles);
    int compile(cstr_t szFile);

    int run();

    int garbageCollect();

    void finishedInitSysModule();

    const string &lastError() const { return m_strLastErr; }

public:
    //
    // Interfaces for Simple JNI
    //

    int registerNativeAPI(cstr_t szClassName, cstr_t szFuncName, void *funcPtr, TypeId retValue, ...);

    CJObject *newJObject(TypeId typeId, jobject &jobj);
    jobject newString(cstr_t szString, bool bCopy) { return m_jobjMgr.newString(szString, bCopy); }
    CJObject *getJObject(jobject jobj) { return m_jobjMgr.get(jobj); }

    int addClass(cstr_t szClassName, int nLineNo = -1, CJObjectNewer *pJObjNewer = nullptr) { return m_classTable.addClass(szClassName, nLineNo, pJObjNewer); }
    CClass *getClass(cstr_t szClassName) { return m_classTable.getClass(szClassName); }
    CClassTable &getClassTable() { return m_classTable; }

    int callMethod(TypeId typeId, int nMethodIndex);

    void throwException(cstr_t szException);

    int jniCallMethod(TypeId typeId, jobject thiz, int nMethodIndex, CVMParameters &parameter);
    int jniCallMethod(TypeId typeId, jobject thiz, int nMethodIndex);
    int jniCallMethod(TypeId typeId, jobject thiz, int nMethodIndex, TypeId tidParam1, jobject param1);
    int jniCallMethod(TypeId typeId, jobject thiz, int nMethodIndex, TypeId tidParam1, jobject param1, TypeId tidParam2, jobject param2);

    void jniAddJObjRef(jobject jobj);
    void jniReleaseJObjRef(jobject jobj);

protected:
    //
    // Methods for compiling Simple JAVA
    //

    //
    // Build symbol info and check for syntax error
    //

    // Build class name, class method, class variable symbols.
    int preBuildSymbolInfo();
    int preBSI_AddClassDefinition();
    int preBSI_AddClassInformation();
    int preBSI_AddClassMember(STreeNode *pTree, CClass *pClass);
    int preBSI_AddMemberVarDecl(STreeNode *pTree, CClass *pClass = nullptr);

    int buildSymbolInfo();
    int BSI_Tree(STreeNode *pTree);
    int BSI_ClassDefinition(STreeNode *pTree);
    int BSI_MemberVariable(STreeNode *pTree, CClass *pClass);
    int BSI_MethodDeclare(STreeNode *pTree, CClass *pClass);
    int BSI_ConstructorCheck(CMemberMethod *pMethod, STreeNode *pBody, CClass *pClass);
    int BSI_LocalVariable(STreeNode *pTree);
    int BSI_Expression(STreeNode *pTree);
    int BSI_ExpressionFunctionCall(STreeNode *pTree);

    int BSI_GetCalledMethod(CClass *pClass, cstr_t szMethodName, VecParamType &vParameters, CMemberMethod **ppMethod, int nLineNo);

    int canCovertRightToLeftType(TypeId typeIdLeft, TypeId typeIdRight);

protected:
    //
    // Build code statement
    //
    int buildCodeStmt();

    int bCS_ClassDefinition(STreeNode *pTree);
    int bCS_Tree(STreeNode *pTree, stmt_index &siOut);
    int bCS_ClassVariable(STreeNode *pTree, CClass *pClass);
    int bCS_VarDeclaration(STreeNode *pTree, stmt_index    &siOut);
    int bCS_Expression(STreeNode *pTree, stmt_index &siOut);

protected:
    //
    // execute statement
    //
    int execute(stmt_index siEntry);

protected:
    int executeExpression(stmt_index siExpr);
    int *exeExprGetVarAddr(stmt_index siExprVar);
    StaticVar *exeExprGetStaticVar(stmt_index siExprVar);

    int exeNewJObject(StmtExpression &expr);

    int callNativeAPI(CMemberMethod *pMethod);

    int getStaticVarValue(size_t nAddress);
    int getJObjectMemberVarValue(int nJobject, int nMember);

    int *getJObjectMemberVarAddr(int nJobject, int nMember);

    void jniCallMethodPushThis(TypeId &typeId, jobject thiz)
    {
        if (typeId == TID_INVALID)
        {
            CJObject    *pObj = m_jobjMgr.get(thiz);
            assert(pObj);
            if (!pObj)
            {
                throwException("Call method of this is null.");
                return;
            }
            typeId = pObj->m_typeId;
        }

        if (thiz != JOBJ_NULL)
            m_stack.push(typeId, thiz);
    }

protected:

    void formatLastError(int nLine, int nError, cstr_t szStr, int nLineSrc);

public:
    // get TypeId by class name, or system predefined name.
    TypeId getTypeIdByName(cstr_t szTypeName);

    cstr_t getTypeNameStrByID(TypeId id);

protected:
    // for debug
#ifdef _DEBUG
    void print();
    friend class CClassTable;
#endif

    class CAutoLoopState {
    public:
        CAutoLoopState(bool &bState) {
            m_pState = &bState;
            m_bOldState = bState;
            bState = true;
        }
        ~CAutoLoopState() { *m_pState = m_bOldState; }

        bool        *m_pState;
        bool        m_bOldState;

    };

    friend class CSjobjUnitTest;
    friend class CSJVMGarbageCoolector;

protected:
    enum EXE_ACTION
    {
        EXE_GO_ON,
        EXE_LOOP_CONTINUE,
        EXE_LOOP_BREAK,
        EXE_FUNC_RETURN,
        EXE_EXCEPTION,
    };

    // All classes
    CClassTable                    m_classTable;
    size_t                        m_nSysClassCount;

    // Statement image
    CStmtImage                    m_stmtImage;

    // Stack of virtual machine
    CVMStack                    m_stack;
    size_t                        m_bp;        // base pointer of current function call.
    int                            m_valueMethodReturn;

    int                            m_nCallStackDeep;
    MapJobjRef                    m_mapJobjRef;

    // Static variables
    VecStaticVar                m_vStaticVars;
    size_t                        m_nSysStaticVarsCount;

    EXE_ACTION                    m_exeAction;
    TypeId                        m_typeIdException;
    jobject                        m_jobjException;

    // JObject manager
    CObjectMgr                    m_jobjMgr;

    bool                        m_bFinishedInitSysModule;

protected:
    //
    // compile for current file.
    //
    STreeNode                    *m_syntaxTree;

    CVarDeclStack                m_varDeclStack;

    string                        m_strLastErr;

    MapStrIndex                    m_mapStrings;
    bool                        m_bsiInLoop;

};

extern CSJVM            *g_sjvm;

#define DECLARE_JNI_OBJ(_Class)                                \
    public:                                                \
    static void registerClassType(CSJVM *sjvm);                    \
    static cstr_t            CLASS_NAME;                    \
    static TypeId            m_sTypeId;                    \
    class CObjNewer : public CJObjectNewer                \
    {                                                    \
    public:                                                \
        CObjNewer() : CJObjectNewer() { }        \
        CJObject *newObj() {                                \
            return new _Class();                        \
        }                                                \
    };


#define IMPLENENT_JNI_OBJ(_Class, _ClassName)            \
    TypeId    _Class::m_sTypeId        = 0;                \
    cstr_t    _Class::CLASS_NAME        = _ClassName;        \
    void _Class::registerClassType(CSJVM *sjvm) {                    \
    int nRet;                                            \
    nRet = sjvm->addClass(_Class::CLASS_NAME, -1, new CObjNewer());    \
    assert(nRet == ERR_OK);                                \
    m_sTypeId = sjvm->getTypeIdByName(_Class::CLASS_NAME);\
}

#include "SjvmLib.h"
