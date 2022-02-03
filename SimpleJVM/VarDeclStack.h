#pragma once


struct VarInfo
{
    TypeId                typeId;
    int16_t                nAddress;            // -1 for unavailable.
    int                    nRet;
    bool                bStatic;            // Static variable?
    bool                bMemberVar;
};

//
// CVarDeclStack 
//
class CVarDeclStack
{
public:

    struct VarProperty
    {
        TypeId                    typeId;
        // Local Variable: address related to bp: bp + nAddr
        // Static Variable: offset in static variable.
        uint16_t                    nAddress;
        bool                    bStatic;            // Static variable?
    };

    typedef map<string, VarProperty*>    MapVarProp;

    // block is the name space of variables, for example: { }
    struct StmtBlock
    {
        StmtBlock() { m_pClass = nullptr; m_pMethod = nullptr; }
        CClass                    *m_pClass;    // Only method block has this.
        CMemberMethod            *m_pMethod;
        MapVarProp                mapVarProp;
    };

    typedef vector<StmtBlock *>        StmtBlockStack;

public:
    CVarDeclStack();
    virtual ~CVarDeclStack();

    void reset();
    
    // travel in block
    void enterClass(CClass *pClass);
    void enterMethod(CMemberMethod *pMethod);
    void enterBlock();

    void leaveBlock();

    void cleanStack();

    CClass *getCurClass();
    CMemberMethod *getCurMethod();

    // add new symbol declaration 
    int addVarDecl(cstr_t szName, TypeId typeId, bool bStatic);

    int declareStaticMemberVar(CClass *pClass, cstr_t szName, TypeId typeId);

    // "this" is the first parameter of method call
    void add_this_super_Var();

    VarInfo getVariable(cstr_t szName);

    // For local variable, return offset
    // For member variable, return member variable index.
    int getVariableAddress(cstr_t szName);

    bool isStaticVariable(cstr_t szName);

    int getCurrentBlockSize();

    void initStaticVar(VecStaticVar &vStaticVar);

protected:
    size_t getCurrentMethodLocalVarCount();

/*    SSM_STACK_ADDR getSymbolMemLoc(cstr_t szName);
    SSM_STACK_ADDR getCurFuncStackSize();
*/
protected:
    void freeStmtBlock(StmtBlock *pblock);

protected:
    StmtBlockStack            m_stmtBlockStack;

protected:
    int declareStaticVar(void *pMethodOrClass, cstr_t szVarName, TypeId typeId, int nStmtDeepOfMethod);
    //
    // Static variable space allocate
    //
    struct AddressTypeId
    {
        TypeId        typeId;
        uint16_t        nAddress;
    };
    typedef map<string, AddressTypeId>        MapStaticVars;
    MapStaticVars            m_mapStaticVars;

};
