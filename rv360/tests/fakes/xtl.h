#ifndef RV360_TEST_XTL_H
#define RV360_TEST_XTL_H

// Host shim limited to load.cpp's CRT dependencies. No GPU emulation.
#include <stdio.h>
#include <string.h>
#include <strings.h>
#define MAX_PATH 260
#define _stricmp strcasecmp
#define _snprintf snprintf
FILE *rv_test_open(const char *filename, const char *mode);
#define fopen rv_test_open

#endif
