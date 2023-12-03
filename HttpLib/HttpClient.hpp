//
//  HttpClient.hpp
//

#ifndef HttpClient_hpp
#define HttpClient_hpp

#include <functional>
#include "uv.h"
#include "../../Utils/UtilsTypes.h"
#include "../../Utils/url.h"
#include "../../Utils/Error.h"
#include "http_parser.h"


using HttpResponseCallback = std::function<void(int statusCode, const ListHttpHeaders &headers, const string &body)>;


class HttpClient {
public:
    HttpClient();

    int request(cstr_t method, const string &url, const ListHttpHeaders &headers, const string *body, HttpResponseCallback callback);

    int get(const string &url, const ListHttpHeaders &headers, HttpResponseCallback callback)
        { return request("GET", url, headers, nullptr, callback); }
    int post(const string &url, const ListHttpHeaders &headers, const string &body, HttpResponseCallback callback)
        { return request("POST", url, headers, &body, callback); }

    void stopConnection();

protected:
    // uv callbacks
    static void onRead(uv_stream_t *tcp, ssize_t nread, const uv_buf_t *buf);
    static void onAlloc(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf);
    static void onClose(uv_handle_t *handle);
    static void onWrite(uv_write_t *req, int status);
    static void onConnect(uv_connect_t *connection, int status);
    static void onAddressResolved(uv_getaddrinfo_t *req, int status, struct addrinfo *res);

    // http_parser callbacks
    static int onHeaderFieldCb(http_parser *parser, const char *at, size_t length);
    static int onHeaderValueCb(http_parser *parser, const char *at, size_t length);
    static int onBodyCb(http_parser *parser, const char *at, size_t length);
    static int onMessageCompleteCb(http_parser *parser);

protected:
    HttpResponseCallback            _callback;
    uv_connect_t                    _req;
    uv_tcp_t                        _tcp;

    bool                            _isOngoing = false;

    // request data
    string                          _toWriteData;

    // response data
    ListHttpHeaders                 _responseHeaders;
    string                          _responseBody;

    http_parser                     _httpParser;
    string                          _headerName, _headerValue;
    bool                            _hasValue = false;

};

#endif /* HttpClient_hpp */
