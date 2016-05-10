/*  
	<ARMADITO PDF ANALYZER is a tool to parse and analyze PDF files in order to detect potentially dangerous contents.>
    Copyright (C) 2015 by Teclib' 
	<ufausther@teclib.com>
    
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "osdeps.h"
#include <string.h>


#ifdef _WIN32

FILE * os_fopen(const char * filename, const char * mode) {

	FILE * f = NULL;

	fopen_s(&f, filename,mode);

	return f;


}



#else

// Linux part for compatibility.
int os_strncat(char *strDest, size_t numberOfElements, const char *strSource, size_t count) {
	

	if( strncat(strDest, strSource, count) == NULL){
		return -1;
	}
	

	return 0;

}

int os_strncpy(char *strDest, size_t numberOfElements, const char *strSource, size_t count) {

	
	if( strncpy(strDest , strSource, count)  == NULL){
		return -1;
	}

	return 0;
}

	

#endif