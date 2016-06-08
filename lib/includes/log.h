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

#ifndef _log_h_
#define _log_h_

#include <stdio.h>
#include <stdarg.h>

#define max_level LOG_LEVEL_WARNING
#define print_report 1	// print the analysis report.

enum log_level {
	LOG_LEVEL_ERROR = 1 << 1,
	LOG_LEVEL_WARNING = 1 << 2,
	LOG_LEVEL_INFO = 1 << 3,
	LOG_LEVEL_DEBUG = 1 << 4,
	LOG_LEVEL_NONE = 1 << 5,
};

void cli_log(enum log_level level, const char * fmt, ...);

//#define err_log(fmt, ...) cli_log(LOG_LEVEL_ERROR,(fmt),__VA_ARGS__)

#ifdef _WIN32

#define err_log(fmt, ...) cli_log(LOG_LEVEL_ERROR,(fmt),__VA_ARGS__)
#define warn_log(fmt, ...) cli_log(LOG_LEVEL_WARNING,(fmt),__VA_ARGS__)
#define dbg_log(fmt, ...) cli_log(LOG_LEVEL_DEBUG,(fmt),__VA_ARGS__)
#define info_log(fmt, ...) cli_log(LOG_LEVEL_INFO,(fmt),__VA_ARGS__)

#else

#define err_log(fmt, ...) cli_log(LOG_LEVEL_ERROR,(fmt),##__VA_ARGS__)
#define warn_log(fmt, ...) cli_log(LOG_LEVEL_WARNING,(fmt),##__VA_ARGS__)
#define dbg_log(fmt, ...) cli_log(LOG_LEVEL_DEBUG,(fmt),##__VA_ARGS__)
#define info_log(fmt, ...) cli_log(LOG_LEVEL_INFO,(fmt),##__VA_ARGS__)

#endif


#endif