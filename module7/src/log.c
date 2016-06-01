#include "log.h"

char * lvl_tostring(enum log_level level){

	switch (level){
	case LOG_LEVEL_ERROR:
		return "<error>";
	case LOG_LEVEL_WARNING:
		return "<warning>";
	case LOG_LEVEL_INFO:
		return "<info>";
	case LOG_LEVEL_DEBUG:
		return "<debug>";
	default:
		return "";
	}

}

void cli_log(enum log_level level, const char * fmt, ...){
	
	va_list ap;
	FILE * fd = stdout;

	if (level > max_level)
		return;
	
	printf("%s ", lvl_tostring(level));
	
	va_start(ap, fmt);
	vprintf(fmt, ap);
	va_end(ap);

	
	return;
}