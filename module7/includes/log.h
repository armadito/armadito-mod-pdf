#ifndef _log_h_
#define _log_h_

#include <stdio.h>
#include <stdarg.h>

#define max_level LOG_LEVEL_DEBUG
#define print_report 1	// print the analysis report.

enum log_level {
	LOG_LEVEL_ERROR = 1 << 1,
	LOG_LEVEL_WARNING = 1 << 2,
	LOG_LEVEL_INFO = 1 << 3,
	LOG_LEVEL_DEBUG = 1 << 4,
	LOG_LEVEL_NONE = 1 << 5,
};

void cli_log(enum log_level level, const char * fmt, ...);

#define err_log(fmt, ...) cli_log(LOG_LEVEL_ERROR,(fmt),__VA_ARGS__)
#define warn_log(fmt, ...) cli_log(LOG_LEVEL_WARNING,(fmt),__VA_ARGS__)
#define dbg_log(fmt, ...) cli_log(LOG_LEVEL_DEBUG,(fmt),__VA_ARGS__)
#define info_log(fmt, ...) cli_log(LOG_LEVEL_INFO,(fmt),__VA_ARGS__)

#endif