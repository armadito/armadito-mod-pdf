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


    
#include "armaditopdf.h"
#include "osdeps.h"
#include "log.h"


void Helper(){
	
	printf("ARMADITO PDF ANALYZER :: invalid parameter\n");
	printf("Command : ./pdfAnalyzer [filename]\n\n");

	return;
}


void Banner(){
	
	printf("----------------------------\n");
	printf("-- ARMADITO PDF ANALYZER  --\n");
	printf("----------------------------\n\n");

	return;
}


int main (int argc, char ** argv){

	int ret;
	FILE * f = NULL;
	int fd = -1;
		
	Banner();	

	if(argc < 2){
		Helper();
		return (-1);
	}

	/* Scan a directory */
	//ret = os_scan_dir(argv[1], 1, (dirent_scan_cb)analyzePDF_ex, NULL);


	// analysis with opened file descriptor.	
	if(!(f = os_fopen(argv[1],"rb"))){
		err_log("Can't open file %s\n", argv[1]);
		return -1;
	}


	fd = os_fileno(f);
	ret = analyzePDF_ex(fd, argv[1]);
	fclose(f);

	//system("pause");
	
	return ret;
}