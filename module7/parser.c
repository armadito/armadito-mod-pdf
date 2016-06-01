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


    
#include "pdfAnalyzer.h"



void Banner(){
	
	printf("-------------------------\n");
	printf("-- ARMADITO PDF ANALYZER  --\n");
	printf("-------------------------\n\n");

	return;
}


void Helper(){
	
	printf("ARMADITO PDF ANALYZER :: No file in parameter\n");
	printf("Command : ./pdfAnalyzer [filename]\n\n");

	return;
}

void Commands(){

	Banner();
	
	printf("Commands list:\n");
	printf("- avscan :: launch a complete analysis and display report\n");
	printf("- decode [obj_ref] :: decode object stream\n");
	printf("- dump [obj_ref] :: dump object stream\n");
	printf("- exit :: exit the parser.\n");
	printf("- object [obj_ref] :: display object infos\n");
	printf("- quit :: exit the parser.\n");
	printf("\n");
	printf("objects actions:\n");
	printf("- decode [obj_ref] :: decode object stream\n");
	printf("- object [obj_ref] :: display object infos\n");
	printf("\n");

	return;
}




int main (int argc, char ** argv){

	int ret;
	FILE * f = NULL;
	int fd = -1;
	struct pdfDocument * pdf = NULL;
	struct pdfObject * obj = NULL;
	char cmd[512] = {0};
	char params[512] = {0};	

	
	#ifdef DEBUG
	Banner();
	#endif

	if(argc < 2){
		Helper();
		return (-1);
	}
	
	//printf ("Analyzing file : %s\n",argv[1]);
	if(!(f = os_fopen(argv[1],"rb"))){
		printf("[-] Error :: main :: Error while opening file %s\n",argv[1]);
		return -1;
	}
	

	// Initialize the pdfDocument struct
	if(!(pdf = initPDFDocument())){
		printf("[-] Error :: analyzePDF :: Error while allocating memory for pdfDocument structure\n");
		fclose(f);
		return -1;
	}
	pdf->fh = f;

	if ( parsePDF(pdf)< 0){
		printf("[-] Error :: parsing error\n");
		return -2;
	}
	
	

	while(1){

		printf("enter a command:\nUHPDF>");
		scanf("%s",&cmd);
		//scanf("%[^\t\r\n]",&cmd);
		
		if(strncmp(cmd,"quit",4) == 0 || strncmp(cmd,"exit",4) == 0 ){

			break;

		}else if(strncmp(cmd,"help",4) == 0){

			Commands();

		}else if(strncmp(cmd,"avscan",6) == 0){

			printf("[TODO] :: av scan\n");

		}else if(strncmp(cmd,"info",4) == 0){

			printf("[TODO] :: display infos.\n");

		}else if(strncmp(cmd,"obj",3) == 0){

			//printf("[TODO] :: display object. %s \n",params);
			printf("Enter an object reference: UHPDF>");
			scanf("%10s",params);
			//printf("params = %s\n",params );
			sprintf(ref, "%s 0 obj",params );
			printf("object = %s\n",ref );
			
			//printf("Decoding object :: %s\n","83 0 obj");
			obj = getPDFObjectByRef(pdf,ref);

			if(obj == NULL){
				printf("[-] Error :: Object [%s] not found!\n",ref);				
				continue;
			}			

			printf("Display object :: %s\n","83 0 obj");

			printObject(obj);
			


		}else if(strncmp(cmd,"decode",6) == 0){

			printf("[TODO] :: display object.\n");

			printf("Enter object reference: UHPDF>");
			scanf("%s",params);
			//printf("params = %s\n",params );
			sprintf(ref, "%s 0 obj",params );
			printf("object = %s\n",ref );

			obj = getPDFObjectByRef(pdf,ref);

			if(obj == NULL){
				printf("[-] Error :: Object [%s] not found!\n",ref);				
				continue;
			}
			decodeObjectStream(obj);			


		}
		else{
			printf("Command [%s] not found. See Help (command: help)!\n",cmd);
		}



	}

	//fclose(f);
	freePDFDocumentStruct(pdf);

	//system("pause");
	
	return ret;
}