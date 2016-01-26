#include "osdeps.h"


#ifdef WIN32

FILE * os_fopen(const char * filename, const char * mode) {

	FILE * f = NULL;

	fopen_s(&f, filename,mode);

	return f;


}



#else

// Linux part for compatibility.
errno_t os_strncat(char *strDest, size_t numberOfElements, const char *strSource, size_t count) {

	return (strncat(strDest, strSource, count));

}

errno_t os_strncpy_s(char *strDest, size_t numberOfElements, const char *strSource, size_t count) {

	return strncpy(strDest , strSource, count);
}

	

#endif