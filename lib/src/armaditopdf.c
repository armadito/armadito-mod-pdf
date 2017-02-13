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
#include "pdfParsing.h"
#include "pdfAnalysis.h"
#include "osdeps.h"
#include "log.h"
#include <time.h>






/*
printAnalysisReport() :: print a report of the analysis (debug only).
parameters:
- struct pdfDocument * pdf
returns:
- none.
*/
// TODO :: printAnalysisReport :: filter report informations by log level.
void printAnalysisReport(struct pdfDocument * pdf){


	if (!print_report || pdf == NULL){
		return;
	}

	printf("\n\n");
	printf("----------------------------------\n");
	printf("-- ARMADITO PDF ANALYZER REPORT --\n");
	printf("----------------------------------\n\n");

	printf("Filename = %s\n",pdf->fname);
	if (pdf->version)
		printf("PDF version = %s\n",pdf->version);

	printf("size = %d bytes\n", pdf->size);
	
	printf("\n\n");
	printf("::: PDF Document Structure Tests :::\n\n");


	printf("bad_header  = %d\n", pdf->testStruct->bad_header);
	printf("encrypted  = %d\n", pdf->testStruct->encrypted);
	printf("empty_page_content  = %d\n", pdf->testStruct->empty_page_content);
	printf("object_collision  = %d\n", pdf->testStruct->object_collision);
	printf("bad_trailer  = %d\n", pdf->testStruct->bad_trailer);
	printf("bad_xref_offset  = %d\n", pdf->testStruct->bad_xref_offset);
	printf("bad_obj_offset  = %d\n", pdf->testStruct->bad_obj_offset);
	printf("obfuscated_object  = %d\n", pdf->testStruct->obfuscated_object);
	printf("multiple_headers  = %d\n", pdf->testStruct->multiple_headers);
	printf("postscript_comments = %d\n", pdf->testStruct->comments);
	printf("malicious_comments = %d\n", pdf->testStruct->malicious_comments);

	printf("\n\n");
	printf("::: PDF Object Analysis Tests :::\n\n");

	printf("active_content = %d\n", pdf->testObjAnalysis->active_content);
		printf(" - js content = %d\n", pdf->testObjAnalysis->js);
		printf(" - xfa content = %d\n", pdf->testObjAnalysis->xfa);
		printf(" - ef content = %d\n", pdf->testObjAnalysis->ef);
	printf("shellcode = %d\n", pdf->testObjAnalysis->shellcode); 
	printf("pattern_high_repetition = %d\n", pdf->testObjAnalysis->pattern_high_repetition); 
	printf("dangerous_keyword_high = %d\n", pdf->testObjAnalysis->dangerous_keyword_high); 
	printf("dangerous_keyword_medium = %d\n", pdf->testObjAnalysis->dangerous_keyword_medium); 
	printf("dangerous_keyword_low = %d\n", pdf->testObjAnalysis->dangerous_keyword_low); 
	printf("time_exceeded = %d\n", pdf->testObjAnalysis->time_exceeded);


	printf("\n\n");
	printf("::: Suspicious Coefficient :::\n\n");
	printf("errors = %d\n", pdf->errors);

	if(pdf->testStruct->bad_header > 0)
		printf("Coef = BAD_HEADER\n");
	else
		if(pdf->testStruct->large_file > 0)
			printf("Coef = %d (LARGE_FILE)\n",pdf->coef);
		else
			if(pdf->testStruct->encrypted > 0)
				printf("Coef = Encrypted_PDF\n");
			else
				printf("Coef = %d\n",pdf->coef);


	printf("-------------------------------------------------------\n");
	//printf("-------------------------------------------------------\n");
	printf("Execution time : %.2lf sec \n",pdf->scan_time);
	printf("-------------------------------------------------------\n");
	printf("-------------------------------------------------------\n\n");

	return;

}


// This function calc the suspicious coefficient according to the tests results
// TODO Improve  this fucntion by calc the coef with the operation coef += test_result * test_coef
int calcSuspiciousCoefficient(struct pdfDocument * pdf){

	// check parameters
	if(pdf == NULL){
		return -1;
	}

	// PDF Document Structure tests
	/*
	EMPTY_PAGE_CONTENT 99
	OBJECT_COLLISION 10
	BAD_TRAILER 40
	BAD_XREF_OFFSET 30
	BAD_OBJ_OFFSET 20
	OBFUSCATED_OBJECT 50 
	MULTIPLE_HEADERS 50
	*/

	pdf->coef = 0;

	if(pdf->testStruct->encrypted > 0 ){
		pdf->coef = -2;
		return -2;
	}

	if(pdf->testStruct->empty_page_content > 0){
		pdf->coef = EMPTY_PAGE_CONTENT;
		return 0;
	}

	if(pdf->testStruct->object_collision > 0 && ( pdf->testStruct->bad_obj_offset > 0 || pdf->testStruct->bad_xref_offset > 0 )){
		pdf->coef += OBJECT_COLLISION_AND_BAD_XREF;
	}else{

		if(pdf->testStruct->object_collision > 0){
			pdf->coef += OBJECT_COLLISION;
		}

		if(pdf->testStruct->bad_obj_offset > 0){
			pdf->coef += BAD_OBJ_OFFSET;
		}

		if( pdf->testStruct->bad_xref_offset > 0){
			pdf->coef += BAD_XREF_OFFSET;
		}
	}

	if(pdf->testStruct->bad_trailer > 0){
		pdf->coef += BAD_TRAILER;
	}

	if(pdf->testStruct->multiple_headers > 0){
		pdf->coef += MULTIPLE_HEADERS;
	}

	if(pdf->testStruct->obfuscated_object > 0){
		pdf->coef += OBFUSCATED_OBJECT;
	}

	if(pdf->testStruct->malicious_comments > 0){
		pdf->coef += MALICIOUS_COMMENTS;
	}


	// PDF Objects Analysis tests
	/*
	ACTIVE_CONTENT 40
	SHELLCODE 40
	PATTERN_HIGH_REPETITION 40
	DANGEROUS_KEYWORD_HIGH 90
	DANGEROUS_KEYWORD_MEDIUM 40
	DANGEROUS_KEYWORD_LOW 20
	TIME_EXCEEDED 20
	*/


	if(pdf->testObjAnalysis->active_content > 0){
		pdf->coef += ACTIVE_CONTENT;
	}

	if(pdf->testObjAnalysis->shellcode > 0){
		pdf->coef += SHELLCODE;
	}

	if(pdf->testObjAnalysis->pattern_high_repetition > 0){
		pdf->coef += PATTERN_HIGH_REPETITION;
	}

	if(pdf->testObjAnalysis->dangerous_keyword_high > 0){
		pdf->coef += DANGEROUS_KEYWORD_HIGH;
	}

	if(pdf->testObjAnalysis->dangerous_keyword_medium > 0){
		pdf->coef += DANGEROUS_KEYWORD_MEDIUM;
	}

	if(pdf->testObjAnalysis->dangerous_keyword_low > 0){
		pdf->coef += DANGEROUS_KEYWORD_LOW;
	}

	if(pdf->testObjAnalysis->time_exceeded > 0){
		pdf->coef += TIME_EXCEEDED;
	}

	
	return 0;

}


/* 
	analyzePDF_ex() :: Analyze pdf extension function 
	parameters: 
		- int fd (file descriptor of the file to analyze)
		- char * filename (file name of the file).
	returns:
		- the suspicious coefficient (>=0) on success.
		- an error code (<0) on error.
*/
int analyzePDF_ex(int fd, char * filename){

	int ret = 0;
	struct pdfDocument * pdf = NULL;
	time_t start_time =0, end_time = 0;
	double time_elapsed = 0;
	int res = 0;
	FILE * fh = NULL;


	if (fd < 0 && filename == NULL){
		err_log("analyzePDF_ex :: invalid parameters!",0);
		return -1;
	}

	dbg_log("analyzePDF_ex :: Analyzing file :: [%s]\n", filename);

	// open the file if fd is invalid	
	if (fd < 0 && !(fh = os_fopen(filename, "rb"))){
		err_log("analyzePDF_ex :: Can't open file %s\n", filename);
		return -1;
	}
	

	// Initialize pdfDocument struct
	if (!(pdf = initPDFDocument())){
		err_log("analyzePDF_ex :: pdfDocument initialization failed!\n");

		if(fh != NULL)
			fclose(fh);

		return -1;
	}

	pdf->fh = fh;
	pdf->fd = fd;
	pdf->fname = os_strdup(filename);

	// start time initialization.
	time(&start_time);	

	// Parse pdf document content.	
	if ((ret = parsePDF(pdf)) < 0){
		err_log("analyzePDF_ex :: parsing PDF document failed\n");
		goto clean;
	}

	/* this is for debug purpose only */
	// printPDFObjects(pdf);
	// printObjectReferences(pdf);


	// PDF objects analysis.
	if ((ret = getDangerousContent(pdf)) < 0){
		err_log("analyzePDF_ex :: get dangerous content failed\n");
		goto clean;
	}
	

	// Document structure analysis
	if((ret = documentStructureAnalysis(pdf))< 0){
		err_log("analyzePDF_ex :: document structure Analysis failed\n");
		goto clean;
	}


clean:

	time(&end_time);
	time_elapsed = difftime(end_time, start_time);

	pdf->scan_time = time_elapsed;

	// calc supicious coefficient of the document.
	calcSuspiciousCoefficient(pdf);

	// print report. (debug only)
	printAnalysisReport(pdf);

	if (ret >= 0){
		ret = pdf->coef;
		dbg_log("[armaditoPDF] Coef = %d\n", ret);
	}
	
	if (pdf != NULL){
		freePDFDocumentStruct(pdf);
	}


	return ret;


}