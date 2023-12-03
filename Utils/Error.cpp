#include "Utils.h"
#include "Error.h"
#include "IdString.h"


static char g_szCustomErrorDesc[512];

RegErr2Str *g_err2StrHeader = nullptr;

static IdToString __ErrID2Str[] = {
    { ERR_FALSE, "Failed." },
    { ERR_OK, "Succeed." },
    { ERR_INITDB_FAILED, "Failed to init Database." },
    { ERR_NO_MEM, "Can't allocate more memory." },
    { ERR_OUTOF_BUFF, "Insufficient buffer." },
    { ERR_NOT_IMPLEMENTED, "Not implemented." },
    { ERR_EOF, "Reached the end of the file." },
    { ERR_BAD_MSG, "Corrupted message." },
    { ERR_PARSE_XML, "Failed to parse XML." },

    { ERR_FILE_NOT_EXIST, "File does not exist." },
    { ERR_PARSE_JSON, "Failed to parse JSON string." },

    { ERR_EXIST, "Exist" },
    { ERR_NOT_FOUND, "Not found." },

    { ERR_NET_HOST_NOT_FOUND, "Internet connection may be down, no such host is known." },
    { ERR_NOT_RECIVED_ALL, "The data packet didn't receive the rest data." },

    { ERR_NOT_SUPPORT_FILE_FORMAT, "The file may not be supported." },
    { ERR_NOLYRICS_OPENED, "No lyrics file was opened." },
    { ERR_EMPTY_LYRICS, "The lyrics are empty."},

    { ERR_OPEN_FILE, "Failed to open file."},
    { ERR_READ_FILE, "Failed to read file."},
    { ERR_WRITE_FILE, "Failed to write file." },
    { ERR_SEEK_FILE, "Failed to seek file." },
    { ERR_NOT_FIND_LRC3V2, "Not found Lyrics3v2 tag" },
    { ERR_BAD_LRC3V2_FORMAT, "Invalid Lyrics3v2 format." },
    { ERR_BAD_FILE_FORMAT, "Invalid file format." },

    { ERR_DISABLE_PACK_XML, "XML packet was disabled, please upgrade to the latest version." },
    { ERR_PACK_BAD_MD5, "Invalid Signature, please contact us for more info." },
    { ERR_INVALID_PACK_VER, "Invalid package version, please contact us for more info." },

    { ERR_NOT_SUPPORT_ENCODING, "Not supported character encoding, please contact us for more info." },

    { ERR_NOT_FOUND_ID3V2, "Not found id3v2 tag." },
    { ERR_NOT_SUPPORT_ID3V2_VER, "Not supported id3v2 tag version." },
    { ERR_INVALID_ID3V2_FRAME, "Invalid id3v2 tag frame." },
    { ERR_BUSY, "Busy, try again later." },

    { ERR_HTTP_BAD_REQUEST, "Invalid HTTP request." },        // 无效的HTTP请求
    { ERR_HTTP_BAD_FORMAT, "Invalid Http format." },        // 无效的HTTP格式
    { ERR_HTTP_BAD_HTTP_PRO_TYPE, "Unrecognized HTTP protocol type." },        // 无法识别的HTTP协议类型
    { ERR_HTTP_INVLIAD_TRUNK, "Invalid HTTP trunck size." },        // 解析HTTP return 数据失败，无法识别的trunck大小
    { ERR_HTTP_BAD_URL, "Invalid HTTP URL format." },        // 无效的HTTP URL格式
    { ERR_HTTP_CODE_ERROR, "NOT dealed HTTP return code." },        // 未处理的HTTP return 代码
    { ERR_HTTP_HEAD_NOT_END, "Can't read the whole HTTP head." },        // 无法完整的读取HTTP头
    { ERR_HTTP_DATA_NOT_FINISHED, "Can't read all the data of the HTTP message." },        // 无法完全读取HTTP消息的数据段
    { ERR_HTTP_302, "HTTP status code: 302, Redirect the request." },        // HTTP状态代码：302，重定向请求
    { ERR_HTTP_400, "HTTP status code: 400, The request could not be understood by the server due to malformed syntax." },        // HTTP状态代码：400，服务器无法识别的请求
    { ERR_HTTP_401, "HTTP status code: 401, The request requires user authentication." },        // HTTP状态代码：401，此请求需要用户名和密码验证
    { ERR_HTTP_403, "HTTP status code: 403, Forbidden." },        // HTTP状态代码：403，访问被禁止
    { ERR_HTTP_404, "HTTP status code: 404, The server has not found anything matching the Request-URI." },        // HTTP状态代码：404，请求的URI在服务器端不能找到
    { ERR_HTTP_407, "HTTP status code: 407, Proxy Authentication Required." },        // HTTP状态代码：407，使用代理时用户名和密码验证失败！
    { ERR_HTTP_500, "HTTP status code: 500, Internal Server Error." },        // HTTP状态代码：500，服务器内部错误

    { 0, nullptr}
};

static RegErr2Str __addErr2Str(__ErrID2Str);

void setCustomErrorDesc(cstr_t szErrorDesc) {
    strcpy_safe(g_szCustomErrorDesc, CountOf(g_szCustomErrorDesc), szErrorDesc);
}

RegErr2Str::RegErr2Str(struct IdToString err2Str[]) {
    m_err2Str = err2Str;
    m_pNext = nullptr;

    if (!g_err2StrHeader) {
        g_err2StrHeader = this;
    } else {
        RegErr2Str *p = g_err2StrHeader;
        while (p->m_pNext) {
            p = p->m_pNext;
        }

        p->m_pNext = this;
    }
}

RegErr2Str::~RegErr2Str() {
    assert(g_err2StrHeader != nullptr);

    if (g_err2StrHeader) {
        if (g_err2StrHeader == this) {
            g_err2StrHeader = m_pNext;
        } else {
            RegErr2Str *p = g_err2StrHeader;
            while (p) {
                if (p->m_pNext == this) {
                    p->m_pNext = m_pNext;
                    return;
                }
                p = p->m_pNext;
            }
            assert(0 && "Can't reach here");
        }
    }
}

cstr_t Error2Str::c_str() {
    if (m_nError == ERR_CUSTOM_ERROR) {
        return g_szCustomErrorDesc;
    }

    if (m_nError >= ERR_MAX) {
        return m_osError.Description();
    }

    cstr_t szError;
    RegErr2Str *it;

    for (it = g_err2StrHeader; it != nullptr; it = it->m_pNext) {
        IdToString *pid2str = it->m_err2Str;
        szError = idToString(pid2str, m_nError, nullptr);
        if (szError) {
            return szError;
        }
    }

#ifndef _WIN32
    if (m_nError >= ERR_C_ERRNO_BASE && m_nError - ERR_C_ERRNO_BASE <= 100) {
        // C error no, translate it.
        m_osError.doFormatMessage(m_nError - ERR_C_ERRNO_BASE);
        return m_osError.Description();
    }
#endif

    return "Not translated error code.";
}
