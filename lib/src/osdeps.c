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



#include "osdeps.h"
#include <string.h>


#ifdef _WIN32

FILE * os_fopen(const char * filename, const char * mode) {

	FILE * f = NULL;

	fopen_s(&f, filename,mode);

	return f;


}


int os_scan_dir(char * path, int recurse, dirent_scan_cb dirent_cb, void * data){

	char * rpath = NULL, *entryPath = NULL;
	char * escapedPath = NULL;
	int ret = 0;
	int size = 0;
	HANDLE fh = NULL;
	WIN32_FIND_DATAA fdata;
	WIN32_FIND_DATAA tmp;
	int fd = -1;
	
	if (path == NULL || dirent_cb == NULL){
		err_log("scan_dir :: invalid parameter\n");
		return -1;
	}

	dbg_log("scan_dir :: path = %s\n", path);

	// Check if it is a directory // TODO :: os_scan_dir :: scan a file.
	if (!(GetFileAttributesA(path) & FILE_ATTRIBUTE_DIRECTORY)) {
		err_log("scan_dir :: (%s) is not a directory\n", path);
		return -2;
	}

	size = strlen(path) + 3;
	rpath = (char*)calloc(size + 1, sizeof(char));
	rpath[size] = '\0';
	sprintf_s(rpath, size, "%s\\*", path);

	dbg_log("scan_dir :: rpath = %s\n",rpath);

	/*
	FindFirstFile note
	Be aware that some other thread or process could create or delete a file with this name between the time you query for the result and the time you act on the information. If this is a potential concern for your application, one possible solution is to use the CreateFile function with CREATE_NEW (which fails if the file exists) or OPEN_EXISTING (which fails if the file does not exist).
	*/
	fh = FindFirstFile(rpath, &fdata);
	if (fh == INVALID_HANDLE_VALUE) {
		warn_log("scan_dir :: FindFirstFileA call failed :: err= [%d]\n", GetLastError());
		goto clean;
	}

	while (fh != INVALID_HANDLE_VALUE && FindNextFile(fh, &tmp) != FALSE) {

		// exclude paths "." and ".."
		if (!strcmp(tmp.cFileName, ".") || !strcmp(tmp.cFileName, ".."))
			continue;

		// build the entry complete path.
		size = strlen(path) + strlen(tmp.cFileName) + 2;

		entryPath = (char*)calloc(size + 1, sizeof(char));
		entryPath[size] = '\0';
		sprintf_s(entryPath, size, "%s\\%s", path, tmp.cFileName);
		dbg_log("scan_dir :: cfilename = %s\n", &tmp.cFileName);
		dbg_log("scan_dir :: entryPath = %s\n", entryPath);


		// If it is a directory and we do recursive scan
		if ((GetFileAttributesA(entryPath) & FILE_ATTRIBUTE_DIRECTORY) && recurse >= 1) {

			ret = scan_dir(entryPath, recurse, dirent_cb, data);
			if (ret != 0){
				free(entryPath);
				break;
			}
		}
		else {
			
			ret = (*dirent_cb)(fd,entryPath);			
		}

		free(entryPath);
		entryPath = NULL;
	}


	// TODO :: scan_dir :: get stats.

clean:
	if (rpath != NULL){
		free(rpath);
		rpath = NULL;
	}
	FindClose(fh);

	return ret;

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


int os_scan_dir(char * path, int recurse, dirent_scan_cb dirent_cb, void * data){

	int ret = 0;

	return ret;
}
	

#endif