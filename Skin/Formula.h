/********************************************************************
    Created  :    2002/01/04    21:39
    FileName :    Formula.h
    Author   :    xhy
    
    Purpose  :    
*********************************************************************/

#if !defined(AFX_FORMULA_H__29B0CFC1_7922_11D5_9E04_02608CAD9330__INCLUDED_)
#define AFX_FORMULA_H__29B0CFC1_7922_11D5_9E04_02608CAD9330__INCLUDED_

//#define EW_OPND_ERROR        0
#define FW_OPND_LONG        1
//#define FW_OPND_DOUBLE        2

#define FW_OPND_VAR_LONG    5

#define FW_OPTR                10
#define FW_OPTR_ADD            11
#define FW_OPTR_SUB            12
#define FW_OPTR_MUL            13
#define FW_OPTR_DIV            14
#define FW_OPTR_LBRACKET    15
#define FW_OPTR_RBRACKET    16
#define FW_OPTR_BEGIN        17
#define FW_OPTR_END            18


typedef struct    FORMULA_WORD{
    int            nWordType;        // '单词类型'
    int            Value;            // 值, 根据word的类型而定，可以是
                                // 数值，也可能是一个待定的字符“变量”
                                // 5, w, h
}FORMULA_WORD, *PFORMULA_WORD;

// ‘变量’的定义
typedef struct FORMULA_VAR{
    int            nVarName;        // variable name
    int            Value;            // variable value
}FORMULA_VAR, *PFORMULA_VAR;

typedef vector<PFORMULA_WORD>        FormulaArray;

class CFormula 
{
public:
    CFormula();
    virtual ~CFormula();

public:
    bool calCualteValue(FORMULA_VAR var[], int &nRetValue);
    bool calCualteValue(int &nRetValue);
    void setFormula(cstr_t szExp);
    void setFormula(int nConstValue);

    void append(cstr_t szFormula);
    void increase(int value);
    void decrease(int value) { increase(-value); }

    void simpleOptimize();

    CFormula& operator =(CFormula &fRight);

    bool getVarValue(int nVar, FORMULA_VAR vars[], int &nRetValue);

    cstr_t getFormula() { return m_strFormula.c_str(); }
    bool empty() { return m_strFormula.empty(); }

protected:
    bool phraseAnalyse(FormulaArray &vWords);

protected:
    string            m_strFormula;

};

#endif // !defined(AFX_FORMULA_H__29B0CFC1_7922_11D5_9E04_02608CAD9330__INCLUDED_)
