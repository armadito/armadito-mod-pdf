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