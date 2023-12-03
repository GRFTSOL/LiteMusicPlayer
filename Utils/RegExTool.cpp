#include "UtilsTypes.h"
#include <regex>


string regExSub(cstr_t szRegEx, cstr_t szInput, cstr_t szWithWhat) {
    std::regex re(szRegEx, std::regex_constants::icase);

    return std::regex_replace(string(szInput), re, string(szWithWhat));
}
