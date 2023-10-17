//
//  rapidjson.h
//  Mp3Player
//
//  Created by henry_xiao on 2023/1/27.
//

#ifndef rapidjson_h
#define rapidjson_h

#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/writer.h>
#include <rapidjson/reader.h>
#include <string>
#include <vector>
#include "RapidjsonWriterEx.hpp"


using RapidjsonWriter = rapidjson::Writer<rapidjson::StringBuffer, rapidjson::UTF8<>, rapidjson::UTF8<>, rapidjson::CrtAllocator, rapidjson::kWriteNanAndInfFlag>;
using RapidjsonPrettyWriter = rapidjson::PrettyWriter<rapidjson::StringBuffer, rapidjson::UTF8<>, rapidjson::UTF8<>, rapidjson::CrtAllocator, rapidjson::kWriteNanAndInfFlag>;

/** rapidjson writer 的用法:
 rapidjson::StringBuffer buf;
 RapidjsonWriter writer(buf);

 writer.StartObject();
 writer.Key("type");
 writer.String(m_type.c_str());

 writer.Key("result");
 writer.String("OK");
 writer.EndObject();

 buf.GetString();
 buf.GetSize();
 */

const rapidjson::Value &getMember(const rapidjson::Value &message, const char *key);
std::string getMemberString(const rapidjson::Value &message, const char *key, const char *defVal = "");
int64_t getMemberInt64(const rapidjson::Value &message, const char *key, int64_t defVal = 0);
inline int getMemberInt(const rapidjson::Value &message, const char *key, int defVal = 0)
    { return (int)getMemberInt64(message, key, defVal); }
bool getMemberBool(const rapidjson::Value &message, const char *key, bool defVal = false);
std::vector<int> getMemberIntArray(const rapidjson::Value &message, const char *key);

#endif /* rapidjson_h */
