#ifndef HTTP_REQUEST_PARSER_HPP
#define HTTP_REQUEST_PARSER_HPP

#include "Request.hpp"


namespace HttpServer {

struct Request;

class RequestParser {
public:
    RequestParser();

    void reset();
    bool hasBody() { return m_contentLength >= 0; }
    bool isBodyFinished(Request &req) { return req.body.size() >= m_contentLength; }

    enum ResultType {
        GOT_HEADER,
        GOT_BODY,
        BAD,
        CONTINUE,
    };

    static std::string resultTypeToString(ResultType result);

    ResultType parse(Request &req, char *begin, char *end) {
        if (m_state == expecting_body) {
            return consumeBody(req, begin, end);
        }

        while (begin != end) {
            ResultType result = consume(req, *begin++);
            if (result == GOT_HEADER) {
                if (m_contentLength >= 0) {
                    m_state = expecting_body;
                    consumeBody(req, begin, end);
                }
                req.method = req.method;
                return result;
            } else if (result == BAD) {
                return result;
            }
        }

        return CONTINUE;
    }

private:
    ResultType consumeBody(Request &req, char *begin, char *end);

    /// Handle the next character of input.
    ResultType consume(Request &req, char input);

    /// The current state of the parser.
    enum State {
        method_start,
        method,
        uri,
        http_version_h,
        http_version_t_1,
        http_version_t_2,
        http_version_p,
        http_version_slash,
        http_version_major_start,
        http_version_major,
        http_version_minor_start,
        http_version_minor,
        expecting_newline_1,
        header_line_start,
        header_lws,
        header_name,
        space_before_header_value,
        header_value,
        expecting_newline_2,
        expecting_newline_3,
        expecting_body,
    };

    State                       m_state;
    int                         m_contentLength;
};

} // namespace HttpServer

#endif // HTTP_REQUEST_PARSER_HPP
