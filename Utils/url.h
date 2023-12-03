#pragma once

#include "UtilsTypes.h"


struct HttpHeader {
    string      name;
    string      value;
};

using ListHttpHeaders = std::list<HttpHeader>;
using VecHttpHeaders = std::vector<HttpHeader>;

// szUrl: scheme://domain:port/path?query_string#fragment_id
// If no port is specified, -1 will be set.
bool urlParse(cstr_t szUrl, string &scheme, string &domain, int &port, string &path);

// rfc:  http://www.faqs.org/rfcs/rfc2396.html
string uriQuote(const char *path);

// Convert %20 etc to blank space...
bool uriIsQuoted(cstr_t str);
string uriUnquote(cstr_t str, bool isForm = false);

struct ArgsPair {
    string                      name;
    string                      value;
};

typedef std::vector<ArgsPair>            VecArgs;

StringView urlGetArgs(const StringView &url);

VecArgs parseArgs(const StringView &args);
string *getArgsValue(VecArgs &args, const StringView &name);
