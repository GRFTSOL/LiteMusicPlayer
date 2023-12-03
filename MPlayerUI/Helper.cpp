#include "MPlayerApp.h"
#include "Helper.h"
#include <time.h>


cstr_t httpErrorCodeToStr(int nCode) {
    switch (nCode) {
        // case 2xx:    return "[Miscellaneous successes]";
        case 200: return "OK"; // # HTTP request OK
        case 201: return "Created";
        case 202: return "Request recorded, will be executed later";
        case 203: return "Non-authoritative information";
        case 204: return "Request executed";
        case 205: return "reset document";
        case 206: return "Partial Content";
        // #[Miscellaneous redirections]
        // case 3xx:    return "[Miscellaneous redirections]";
        case 300: return "Multiple documents available";
        case 301: return "Moved permanently (redirect)";
        case 302: return "Moved temporarily (redirect)";
        case 303: return "See other document";
        case 304: return "Not Modified since last retrieval"; // # HTTP request OK
        case 305: return "Use proxy";
        case 306: return "Switch proxy";
        case 307: return "Moved temporarily";
        // #[Miscellaneous client/user errors]
        // case 4xx:    return "[Miscellaneous client/user errors]";
        case 400: return "Bad Request";
        case 401: return "Unauthorized";
        case 402: return "Payment required";
        case 403: return "Forbidden";
        case 404: return "Document Not Found";
        case 405: return "Method not allowed";
        case 406: return "Document not acceptable to client";
        case 407: return "Proxy authentication required";
        case 408: return "Request Timeout";
        case 409: return "Request conflicts with state of resource";
        case 410: return "Document gone permanently";
        case 411: return "length required";
        case 412: return "Precondition failed";
        case 413: return "Request too long";
        case 414: return "Requested filename too long";
        case 415: return "Unsupported media type";
        case 416: return "Requested range not valid";
        case 417: return "Failed";
        //#[Miscellaneous server errors]
        //    case 5xx:    return "[Miscellaneous server errors]";
        case 500: return "Internal server Error";
        case 501: return "Not implemented";
        case 502: return "Received bad response from real server";
        case 503: return "Server busy";
        case 504: return "Gateway timeout";
        case 505: return "HTTP version not supported";
        case 506: return "Redirection failed";
    default:
        if (nCode > 206 && nCode < 300) {
            return "Miscellaneous successes";
        } else if (nCode > 307 && nCode < 400) {
            return "Miscellaneous redirections";
        } else if (nCode > 418 && nCode < 500) {
            return "Miscellaneous client/user errors";
        } else if (nCode > 506 && nCode < 600) {
            return "Miscellaneous server errors";
        } else {
            return "Unknown Http error code";
        }
        // #[Unknown]
        // 'xxx'=>'[Unknown]'
    }
}

void profileGetColorValue(COLORREF &clr, cstr_t szSectName, cstr_t szKeyName) {
    CColor c;

    c.set(clr);
    getColorValue(c, g_profile.getString(szSectName, szKeyName, ""));

    clr = c.get();
}

void profileGetColorValue(CColor &clr, cstr_t szSectName, cstr_t szKeyName) {
    getColorValue(clr, g_profile.getString(szSectName, szKeyName, ""));
}
