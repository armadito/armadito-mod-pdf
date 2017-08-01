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





char * getVersion(){
	return a6o_pdf_ver;
}


/*
printAnalysisReport() :: print a report of the analysis (debug only).
parameters:
- struct pdfDocument * pdf
returns:
- none.
*/
// TODO :: printAnalysisReport :: filter report informations by log level.
void pdf_print_report(struct pdfDocument * pdf){


	if (!print_report || pdf == NULL){
		return;
	}

	printf("\n\n");
	printf("=========================\n");
	printf("== ARMADITO PDF REPORT ==\n");
	printf("=========================\n");


	printf("Filename = %s\n",pdf->fname);
	printf("PDF version = %s\n",pdf->version);
	printf("size = %d bytes\n", pdf->size);
	printf("Execution time : %.2lf sec \n",pdf->scan_time);

	// PDF Document Structure Tests
	printf("-----------------------------\n");
	printf("encrypted = %s\n",(pdf->flags & FLAG_ENCRYPTED_CONTENT ? "yes":"no"));
	printf("object_collision  = %s\n", (pdf->flags & FLAG_OBJECT_COLLISION ? "yes":"no"));
	printf("bad_trailer  = %s\n", (pdf->flags & FLAG_BAD_PDF_TRAILER ? "yes":"no"));
	printf("bad_xref_offset  = %s\n", (pdf->flags & FLAG_BAD_XREF_OFFSET ? "yes":"no"));
	printf("bad_obj_offset  = %s\n", (pdf->flags & FLAG_BAD_OBJ_OFFSET ? "yes":"no"));
	//printf("multiple_headers  = %s\n", (pdf->flags & FLAG_MULTIPLE_HEADERS ? "yes":"no"));

	// PDF Object Analysis Tests
	printf("------------------------------\n");
	printf("obfuscated_object  = %s\n", (pdf->flags & FLAG_OBFUSCATED_OBJ ? "yes":"no"));
	printf("postscript_comments = %s\n", (pdf->flags & FLAG_POSTSCRIPT_COMMENTS ? "yes":"no"));
	//printf("malicious_comments = %s\n", (pdf->flags & FLAG_MALICIOUS_COMMENTS ? "yes":"no"));
	printf("active_content = %s\n", (pdf->flags & FLAG_ACTIVE_CONTENTS? "yes":"no"));
		//printf(" - js content = %d\n", pdf->testObjAnalysis->js);
		//printf(" - xfa content = %d\n", pdf->testObjAnalysis->xfa);
		//printf(" - ef content = %d\n", pdf->testObjAnalysis->ef);
	//printf("pattern_high_repetition = %d\n", pdf->testObjAnalysis->pattern_high_repetition);
	printf("dangerous_keyword_high = %s\n", (pdf->flags & FLAG_DANG_KEY_HIGH? "yes":"no"));
	printf("dangerous_keyword_medium = %s\n", (pdf->flags & FLAG_DANG_KEY_MED ? "yes":"no"));
	printf("dangerous_keyword_low = %s\n", (pdf->flags & FLAG_DANG_KEY_LOW ? "yes":"no"));

	printf("------------------------------\n");
	printf("Coef = %d\n",pdf->coef);
	printf("------------------------------\n");
	printf("------------------------------\n");

	return;
}


// This function calc the suspicious coefficient according to the tests results
// TODO Improve  this fucntion by calc the coef with the operation coef += test_result * test_coef
int calc_suspicious_coef(struct pdfDocument * pdf){

	if(pdf == NULL){
		pdf->coef = -1;
		err_log("calc_coef :: invalid parameters!\n");
		return ERROR_INVALID_PARAMETERS;
	}

	// PDF Document Structure tests
	pdf->coef = 0;

	if( pdf->flags & FLAG_ENCRYPTED_CONTENT ){
		pdf->coef = -2;
		return ERROR_ENCRYPTED_CONTENT;
	}

	if( pdf->flags & FLAG_OBJECT_COLLISION){
		pdf->coef += OBJECT_COLLISION;
	}

	if( pdf->flags & FLAG_BAD_PDF_TRAILER ){
		pdf->coef += BAD_TRAILER;
	}

	if( pdf->flags & FLAG_BAD_XREF_OFFSET ){
		pdf->coef += BAD_XREF_OFFSET;
	}

	// PDF Objects Analysis tests

	if( pdf->flags & FLAG_OBFUSCATED_OBJ ){
		pdf->coef += OBFUSCATED_OBJECT;
	}

	if( pdf->flags & FLAG_POSTSCRIPT_COMMENTS ){
		pdf->coef += MALICIOUS_COMMENTS;
	}

	if( pdf->flags & FLAG_ACTIVE_CONTENTS ){
		pdf->coef += ACTIVE_CONTENT;
	}

	if( pdf->flags & FLAG_DANG_KEY_HIGH ){
		pdf->coef += DANGEROUS_KEYWORD_HIGH;
	}

	if( pdf->flags & FLAG_DANG_KEY_MED ){
		pdf->coef += DANGEROUS_KEYWORD_MEDIUM;
	}

	if( pdf->flags & FLAG_DANG_KEY_LOW ){
		pdf->coef += DANGEROUS_KEYWORD_LOW;
	}

	return ERROR_SUCCESS;
}

#if 0
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
#endif


int pdf_initialize(void){

	// TODO: Set debug log level.
	// TODO: Load configuration.
	return ERROR_SUCCESS;
}


struct pdfDocument * pdf_load_fd(int fd, char * filename, int * retcode){

	char * version;
	struct pdfDocument * pdf = NULL;

	if(retcode)
		*retcode = EXIT_SUCCESS;

	if(fd < 0){
		dbg_log("invalid file descriptor (%d)!\n",fd);
		*retcode = ERROR_ON_LOAD | ERROR_INVALID_FD;
		return NULL;
	}

	version = pdf_get_version_from_fd(fd, retcode);
	if(version == NULL){
		*retcode |= ERROR_ON_LOAD;
		return NULL;
	}

	pdf = init_pdf_document(fd, NULL, filename, version);

	if(pdf == NULL){
		*retcode = ERROR_ON_LOAD | ERROR_INSUFFICENT_MEMORY;
	}

	return pdf;
}


void pdf_unload(struct pdfDocument * pdf){

	free_pdf_document(pdf);

	return;
}