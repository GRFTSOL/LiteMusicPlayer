#pragma once

#define MAJOR_VERSION       1
#define MINOR_VERSION       0
#define BUILD               "ABCDEF"

#define make_str(s) #s

#define VERSION_STR         make_str(MAJOR_VERSION) make_str(MINOR_VERSION) BUILD
