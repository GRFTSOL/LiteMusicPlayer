#pragma once

#include "UtilsTypes.h"


// szUrl: scheme://domain:port/path?query_string#fragment_id
// If no port is specified, -1 will be set.
bool urlParse(cstr_t szUrl, string &scheme, string &domain, int &port, string &path);

// COMMENT:
// rfc:  http://www.faqs.org/rfcs/rfc2396.html
void uriQuote(const char *szLocal, string &strInet);
void uriQuote(const char *szLocal, char *szInet, int nLenMax);

// Convert %20 etc to blank space...
bool uriIsQuoted(cstr_t str);
string uriUnquote(cstr_t str, bool isForm = false);

struct ArgsPair {
    string                      name;
    string                      value;
};

typedef std::vector<ArgsPair>            VecArgs;

VecArgs parseArgs(const string &args);
string *getArgsValue(VecArgs &args, const string &name);
