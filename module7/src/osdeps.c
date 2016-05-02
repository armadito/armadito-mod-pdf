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