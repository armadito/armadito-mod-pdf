#ifndef __os_deps_h_
#define __os_deps_h_

#include <stdio.h>

#ifdef WIN32

#define os_strncat strncat_s
#define os_sprintf sprintf_s
#define os_sscanf sscanf_s
#define os_strncpy strncpy_s
FILE * os_fopen(const char * filename, const char * mode);

#else

#define os_fopen fopen
#define os_sprintf snprintf
#TOTRY...
//#define os_sprintf(buffer,sizeOfBuffer, format,...) sprintf(buffer, format,...)
errno_t os_strncat(char *strDest, size_t numberOfElements, const char *strSource, size_t count);
errno_t os_strncpy_s(char *strDest, size_t numberOfElements, const char *strSource, size_t count);

#endif

#endif