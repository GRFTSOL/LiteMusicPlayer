#include "RequestParser.hpp"
#include "Request.hpp"
#include "Utils/Utils.h"


namespace HttpServer {

static const std::string HEADER_CONTENT_LENGTH = "Content-Length";
static const int MAX_REQ_BODY_LENGTH = 1024 * 1024 * 50;

static inline bool is_char(int c) { return c >= 0 && c <= 127; }

static inline bool is_ctl(int c) { return (c >= 0 && c <= 31) || (c == 127); }

static inline bool is_tspecial(int c) {
    switch (c) {
        case '(':
        case ')':
        case '<':
        case '>':
        case '@':
        case ',':
        case ';':
        case ':':
        case '\\':
        case '"':
        case '/':
        case '[':
        case ']':
        case '?':
        case '=':
        case '{':
        case '}':
        case ' ':
        case '\t':
            return true;
        default:
            return false;
    }
}

static inline bool is_digit(int c) { return c >= '0' && c <= '9'; }

RequestParser::RequestParser() : m_state(method_start) {
    reset();
}

void RequestParser::reset() {
    m_state = method_start;
    m_contentLength = -1;
}

RequestParser::ResultType RequestParser::consumeBody(Request &req, char *begin, char *end) {
    if (m_contentLength < 0 || m_contentLength > MAX_REQ_BODY_LENGTH) {
        return BAD;
    }

    int remainedSize = (int)(m_contentLength - req.body.size());
    if (remainedSize <= end - begin) {
        req.body.append(begin, begin + remainedSize);
        return GOT_BODY;
    } else {
        req.body.append(begin, end);
        return CONTINUE;
    }
}

RequestParser::ResultType RequestParser::consume(Request& req, char input) {
    switch (m_state) {
        case method_start:
            if (!is_char(input) || is_ctl(input) || is_tspecial(input)) {
                return BAD;
            } else {
                m_state = method;
                req.method.push_back(input);
                return CONTINUE;
            }
        case method:
            if (input == ' ') {
                m_state = uri;
                return CONTINUE;
            } else if (!is_char(input) || is_ctl(input) || is_tspecial(input)) {
                return BAD;
            } else {
                req.method.push_back(input);
                return CONTINUE;
            }
        case uri:
            if (input == ' ') {
                m_state = http_version_h;
                return CONTINUE;
            } else if (is_ctl(input)) {
                return BAD;
            } else {
                req.uri.push_back(input);
                return CONTINUE;
            }
        case http_version_h:
            if (input == 'H') {
                m_state = http_version_t_1;
                return CONTINUE;
            } else {
                return BAD;
            }
        case http_version_t_1:
            if (input == 'T') {
                m_state = http_version_t_2;
                return CONTINUE;
            } else {
                return BAD;
            }
        case http_version_t_2:
            if (input == 'T') {
                m_state = http_version_p;
                return CONTINUE;
            } else {
                return BAD;
            }
        case http_version_p:
            if (input == 'P') {
                m_state = http_version_slash;
                return CONTINUE;
            } else {
                return BAD;
            }
        case http_version_slash:
            if (input == '/') {
                req.versionMajor = 0;
                req.versionMinor = 0;
                m_state = http_version_major_start;
                return CONTINUE;
            } else {
                return BAD;
            }
        case http_version_major_start:
            if (is_digit(input)) {
                req.versionMajor =
                    req.versionMajor * 10 + input - '0';
                m_state = http_version_major;
                return CONTINUE;
            } else {
                return BAD;
            }
        case http_version_major:
            if (input == '.') {
                m_state = http_version_minor_start;
                return CONTINUE;
            } else if (is_digit(input)) {
                req.versionMajor =
                    req.versionMajor * 10 + input - '0';
                return CONTINUE;
            } else {
                return BAD;
            }
        case http_version_minor_start:
            if (is_digit(input)) {
                req.versionMinor =
                    req.versionMinor * 10 + input - '0';
                m_state = http_version_minor;
                return CONTINUE;
            } else {
                return BAD;
            }
        case http_version_minor:
            if (input == '\r') {
                m_state = expecting_newline_1;
                return CONTINUE;
            } else if (is_digit(input)) {
                req.versionMinor =
                    req.versionMinor * 10 + input - '0';
                return CONTINUE;
            } else {
                return BAD;
            }
        case expecting_newline_1:
            if (input == '\n') {
                m_state = header_line_start;
                return CONTINUE;
            } else {
                return BAD;
            }
        case header_line_start:
            if (input == '\r') {
                m_state = expecting_newline_3;
                return CONTINUE;
            } else if (!req.headers.empty() &&
                       (input == ' ' || input == '\t')) {
                m_state = header_lws;
                return CONTINUE;
            } else if (!is_char(input) || is_ctl(input) || is_tspecial(input)) {
                return BAD;
            } else {
                req.headers.push_back(Header());
                req.headers.back().name.push_back(input);
                m_state = header_name;
                return CONTINUE;
            }
        case header_lws:
            if (input == '\r') {
                m_state = expecting_newline_2;
                return CONTINUE;
            } else if (input == ' ' || input == '\t') {
                return CONTINUE;
            } else if (is_ctl(input)) {
                return BAD;
            } else {
                m_state = header_value;
                req.headers.back().value.push_back(input);
                return CONTINUE;
            }
        case header_name:
            if (input == ':') {
                m_state = space_before_header_value;
                return CONTINUE;
            } else if (!is_char(input) || is_ctl(input) || is_tspecial(input)) {
                return BAD;
            } else {
                req.headers.back().name.push_back(input);
                return CONTINUE;
            }
        case space_before_header_value:
            if (input == ' ') {
                m_state = header_value;
                return CONTINUE;
            } else {
                return BAD;
            }
        case header_value:
            if (input == '\r') {
                if (m_contentLength == -1 && req.headers.back().name.size() == HEADER_CONTENT_LENGTH.size() &&
                        strcasecmp(req.headers.back().name.c_str(), HEADER_CONTENT_LENGTH.c_str()) == 0) {
                    m_contentLength = atoi(req.headers.back().value.c_str());
                }
                m_state = expecting_newline_2;
                return CONTINUE;
            } else if (is_ctl(input)) {
                return BAD;
            } else {
                req.headers.back().value.push_back(input);
                return CONTINUE;
            }
        case expecting_newline_2:
            if (input == '\n') {
                m_state = header_line_start;
                return CONTINUE;
            } else {
                return BAD;
            }
        case expecting_newline_3:
            return (input == '\n') ? GOT_HEADER : BAD;
        default:
            return BAD;
    }
}

std::string RequestParser::resultTypeToString(ResultType result) {
    static std::string values[] = { "GOT_HEADER", "GOT_BODY", "BAD", "CONTINUE" };
    assert(result >= 0 && (int)result < CountOf(values));
    return values[result];
}

} // namespace HttpServer

