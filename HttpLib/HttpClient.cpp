//
//  HttpClient.cpp
//

#include "HttpClient.hpp"
#include <glog/logging.h>


string getHttpRequestString(cstr_t method, const string &domain, int port, const string &path, const ListHttpHeaders &headers, const string *body) {
    string out;

    out.append(method); out.push_back(' ');
    out.append(uriQuote(path.c_str())); out.push_back(' ');
    out.append("HTTP/1.1\r\n");

    if (port == 0 || port == 80) {
        out.append("Host: " + domain + "\r\n");
    } else {
        out.append("Host: " + domain + ":" + std::to_string(port) + "\r\n");
    }

    for (auto &header : headers) {
        out.append(header.name + ": " + header.value);
        out.append("\r\n");
    }

    if (body) {
        out.append("Content-Length: " + std::to_string(body->size()));
        out.append("\r\n\r\n");
        out.append(*body);
    } else {
        out.append("\r\n");
    }

    return out;
}

static http_parser_settings _parserSettings = { nullptr };
static StringView KeepAlive("Keep-Alive");

HttpClient::HttpClient() {
    if (_parserSettings.on_header_field == nullptr) {
        _parserSettings.on_header_field = onHeaderFieldCb;
        _parserSettings.on_header_value = onHeaderValueCb;
        _parserSettings.on_body = onBodyCb;
        _parserSettings.on_message_complete = onMessageCompleteCb;
    }
}

void HttpClient::stopConnection() {
    uv_close((uv_handle_t *)&_tcp, onClose);
    // _isOngoing = false;
}

void HttpClient::onAlloc(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
    buf->base = (char *)malloc(suggested_size);
    buf->len = suggested_size;
}

void HttpClient::onClose(uv_handle_t *handle) {
    HttpClient *thiz = static_cast<HttpClient *>(handle->data);
    thiz->_isOngoing = false;
}

void HttpClient::onWrite(uv_write_t *req, int status) {
    delete req;
}

void HttpClient::onRead(uv_stream_t *tcp, ssize_t nread, const uv_buf_t *buf) {
    HttpClient *thiz = static_cast<HttpClient *>(tcp->data);

    if (nread > 0) {
        http_parser_execute(&thiz->_httpParser, &_parserSettings, buf->base, nread);
    } else {
        /* EOF */
        thiz->stopConnection();
    }

    free(buf->base);
}

void HttpClient::onConnect(uv_connect_t *connection, int status) {
    HttpClient *thiz = static_cast<HttpClient *>(connection->data);
    assert(thiz);

    if (status != 0) {
        // Failed.
        thiz->stopConnection();
        return;
    }

    uv_stream_t *stream = connection->handle;
    uv_buf_t http = {
#ifdef WIN32
        thiz->_toWriteData.size(),
        thiz->_toWriteData.data(),
#else
        thiz->_toWriteData.data(),
        thiz->_toWriteData.size(),
#endif
    };

    uv_write(new uv_write_t, stream, &http, 1, onWrite);
    uv_read_start(stream, onAlloc, onRead);
}

void HttpClient::onAddressResolved(uv_getaddrinfo_t *req, int status, struct addrinfo *res) {
    HttpClient *thiz = static_cast<HttpClient *>(req->data);
    assert(thiz);

    if (status < 0) {
        LOG(ERROR) << "Failed to resolve address.";
        thiz->_isOngoing = false;
        return;
    }

    thiz->_tcp.data = thiz;
    thiz->_req.data = thiz;
    thiz->_tcp.close_cb = onClose;

    uv_tcp_init(uv_default_loop(), &thiz->_tcp);
    uv_tcp_connect(&thiz->_req, &thiz->_tcp, res->ai_addr, onConnect);
}

int HttpClient::request(cstr_t method, const string &url, const ListHttpHeaders &headers, const string *body, HttpResponseCallback callback) {

    if (_isOngoing) {
        return ERR_BUSY;
    }

    string scheme, domain, path;
    int port = 0;
    if (!urlParse(url.c_str(), scheme, domain, port, path)) {
        LOG(ERROR) << "Invalid http url format: " << url;
        return ERR_HTTP_BAD_URL;
    }

    if (scheme != "http") {
        LOG(ERROR) << "Only http schema is supported: " << url;
        return ERR_HTTP_BAD_URL;
    }

    if (path.empty()) {
        path = "/";
    }

    if (port == -1) {
        port = 80;
    }

    _isOngoing = true;
    _callback = callback;

    _responseBody.clear();
    _responseHeaders.clear();
    _headerName.clear();
    _headerValue.clear();
    _hasValue = false;

    _toWriteData = getHttpRequestString(method, domain, port, path, headers, body);

    http_parser_init(&_httpParser, HTTP_RESPONSE);
    _httpParser.data = this;

    struct addrinfo hints;
    hints.ai_family = PF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = 0;

    uv_getaddrinfo_t *resolver = new uv_getaddrinfo_t;
    resolver->data = this;
    uv_getaddrinfo(uv_default_loop(), resolver, onAddressResolved,
                   domain.c_str(), std::to_string(port).c_str(), &hints);

    return ERR_OK;
}

int HttpClient::onHeaderFieldCb(http_parser *parser, const char *at, size_t length) {
    HttpClient *client = (HttpClient *)parser->data;

    if (client->_hasValue) {
        client->_responseHeaders.push_back({ client->_headerName, client->_headerValue });
        client->_headerName.clear();
        client->_headerValue.clear();
        client->_hasValue = false;
    }

    client->_headerName.append(at, length);

    return 0;
}

int HttpClient::onHeaderValueCb(http_parser *parser, const char *at, size_t length) {
    HttpClient *client = (HttpClient *)parser->data;

    client->_hasValue = true;
    client->_headerValue.append(at, length);

    return 0;
}

int HttpClient::onBodyCb(http_parser *parser, const char *at, size_t length) {
    HttpClient *client = (HttpClient *)parser->data;
    client->_responseBody.append(at, length);

    return 0;
}

int HttpClient::onMessageCompleteCb(http_parser *parser) {
    HttpClient *client = (HttpClient *)parser->data;

    if (client->_callback) {
        client->_callback(client->_httpParser.status_code, client->_responseHeaders, client->_responseBody);
    }

    client->stopConnection();

    return 1;
}
