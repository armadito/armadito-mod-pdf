/***

Copyright (C) 2015, 2016 Teclib'

This file is part of Armadito module PDF.

Armadito module PDF is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Armadito module PDF is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Armadito module PDF.  If not, see <http://www.gnu.org/licenses/>.

***/

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