/********************************************************************
    Created  :    2002/01/04    21:30
    FileName :    CProfile.h
    Author   :    xhy

    Purpose  :    
*********************************************************************/

#ifndef Utils_Profile_h
#define Utils_Profile_h

#pragma once

#include "UtilsTypes.h"


bool writePrivateProfileInt(
    cstr_t lpAppName,  // section name
    cstr_t lpKeyName,  // key name
    int value,            // int to add
    cstr_t lpFileName  // initialization file
    );

#ifndef WIN32
uint32_t GetPrivateProfileString(cstr_t lpAppName, cstr_t lpKeyName, cstr_t lpDefault, char * lpReturnedString, uint32_t nSize, cstr_t lpFileName);
bool WritePrivateProfileString(cstr_t lpAppName, cstr_t lpKeyName, cstr_t lpString, cstr_t lpFileName);
uint32_t GetPrivateProfileInt(cstr_t lpAppName, cstr_t lpKeyName, int nDefault, cstr_t lpFileName);
#endif

//
// 从*.ini文件中读入设置之类的
// 为了提高效率，使用缓冲，将访问过的key都保存在m_mapAppName2Keys中。
// 在保存的时候，没有使用缓冲，即只要修改，就会更新；因为修改不多，所以直接保存
//
class CProfile {
public:
    CProfile();
    virtual ~CProfile();

protected:
    struct Section {
        string                      strAppName;
        MapStrings                  mapKeys;            // all keyname will be converted to lowercase.
    };
    typedef list<Section*>        ListSections;

    string                      m_strProfile;
    ListSections                m_listAppSect;
    string                      m_strDefAppName;

protected:
    void addKeys(cstr_t szAppName, cstr_t szKeyName, cstr_t szValue);
    bool getKey(cstr_t szAppName, cstr_t szKeyName, MapStrings::iterator &it);

    void setKeyDefaultIfNotExist(cstr_t szAppName, cstr_t szKeyName, cstr_t szValue);

public:
    void init(cstr_t szProfile, cstr_t szDefKey = nullptr);
    void close();

    void doCache();

    void setDefaultIfNotExist(cstr_t szAppName, cstr_t szKeyName, cstr_t szDefault);
    void setDefaultIfNotExist(cstr_t szAppName, cstr_t szKeyName, int nDefault);

    cstr_t getString(cstr_t szAppName, cstr_t szKeyName, cstr_t szDefault);
    bool writeString(cstr_t szAppName, cstr_t szKeyName, cstr_t szValue);

    uint32_t getInt(cstr_t szAppName, cstr_t szKeyName, int nDefault);
    bool writeInt(cstr_t szAppName, cstr_t szKeyName, int value);

    bool getBool(cstr_t szAppName, cstr_t szKeyName, int nDefault)
        { return getInt(szAppName, szKeyName, nDefault) != 0; }
    bool getBool(cstr_t szKeyName, int nDefault)
        { return getInt(m_strDefAppName.c_str(), szKeyName, nDefault) != 0; }
    bool writeBool(cstr_t szAppName, cstr_t szKeyName, bool value)
        { return writeInt(szAppName, szKeyName, value); }
    bool writeBool(cstr_t szKeyName, bool value)
        { return writeInt(m_strDefAppName.c_str(), szKeyName, value); }

    // 无需输入 szAppName 的函数
    cstr_t getString(cstr_t szKeyName, cstr_t szDefault)
        { return getString(m_strDefAppName.c_str(), szKeyName, szDefault); }
    bool writeString(cstr_t szKeyName, cstr_t szValue)
        { return writeString(m_strDefAppName.c_str(), szKeyName, szValue); }

    uint32_t getInt(cstr_t szKeyName, int nDefault)
        { return getInt(m_strDefAppName.c_str(), szKeyName, nDefault); }
    bool writeInt(cstr_t szKeyName, int value)
        { return writeInt(m_strDefAppName.c_str(), szKeyName, value); }

    cstr_t getFile();
    cstr_t getDefAppName();

    //
    // support encrypt keyname and value
    //    only keyname is encrypted in cache
    //  vlaue does not encrypted in cache, and value only encrypted in file.
    //
    string encryptGetString(cstr_t szAppName, cstr_t szKeyName, cstr_t szDefault);
    bool encryptWriteString(cstr_t szAppName, cstr_t szKeyName, cstr_t szValue);

    // 无需输入 szAppName 的函数
    string encryptGetString(cstr_t szKeyName, cstr_t szDefault)
        { return encryptGetString(m_strDefAppName.c_str(), szKeyName, szDefault); }
    bool encryptWriteString(cstr_t szKeyName, cstr_t szValue)
        { return encryptWriteString(m_strDefAppName.c_str(), szKeyName, szValue); }
    int encryptGetInt(cstr_t szKeyName, int value);
    void encryptWriteInt(cstr_t szKeyName, int value);

};


#endif // !defined(Utils_Profile_h)
