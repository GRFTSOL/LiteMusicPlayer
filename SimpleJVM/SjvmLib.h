#pragma once

void initSjvmLib(CSJVM *sjvm);

class CSjobjException : public CJObject
{
    DECLARE_JNI_OBJ(CSjobjException);
public:
    CSjobjException() : CJObject(m_sTypeId, 0) { }

    static void registerJNI();

    static void SJNI_API ExceptionConstructor(jobject jObj, cstr_t szException);

    static jobject SJNI_API toString(jobject jObj);

public:
    string            strException;

};
