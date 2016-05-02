#ifndef __os_deps_h_
#define __os_deps_h_

#include <stdio.h>

#ifdef _WIN32

#include <io.h>
#define os_strncat strncat_s
#define os_sprintf sprintf_s
#define os_sscanf sscanf_s
#define os_strncpy strncpy_s
#define os_strdup _strdup
#define os_lseek _lseek
#define os_read _read
#define os_fileno _fileno
FILE * os_fopen(const char * filename, const char * mode);

#else

#include <unistd.h>
#define os_fopen fopen
#define os_sprintf snprintf
#define os_sscanf sscanf
#define os_strdup strdup
#define os_lseek lseek
#define os_read read
#define os_fileno fileno
//#define os_sprintf(buffer,sizeOfBuffer, format,...) sprintf(buffer, format,...)
int os_strncat(char *strDest, size_t numberOfElements, const char *strSource, size_t count);
int os_strncpy(char *strDest, size_t numberOfElements, const char *strSource, size_t count);

#endif

#endif