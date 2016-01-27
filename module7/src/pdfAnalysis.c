/*  
	< UHURU PDF ANALYZER is a tool to parses and analyze PDF files in order to detect potentially dangerous contents.>
    Copyright (C) 2015 by Ulrich FAUSTHER <u.fausther@uhuru-solutions.com>
    

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

#include "pdfAnalyzer.h"

// This function print a report of the analysis
int printAnalysisReport(struct pdfDocument * pdf, char * filename){

	printf("\n\n");
	printf("-------------------------------\n");
	printf("-- UHURU PDF ANALYZER REPORT --\n");
	printf("-------------------------------\n\n");

	printf("Filename = %s\n",filename);
	//printf("Execution time = %d sec\n",0);
	printf("PDF version = %s\n",pdf->version);

	//#ifdef DEBUG
	
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

	//#endif

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

	return 0;

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
	int analyzePDF(...)
	This function launch analysis on a PDF file (parsing, analysis and evaluation).
	Returns a suspicion coef on success.
	Returns -1 on error.
	Returns -2 if the file is not supported (bad header or encrypted file).
*/
int analyzePDF(char * filename){


	int ret = 0;
	FILE * f = NULL;
	struct pdfDocument * pdf = NULL;
	time_t start_time, end_time;
	double time_elapsed = 0;
	int res = 0;


	time(&start_time);
	
	// Open file
	if(!(f = os_fopen(filename,"rb"))){
		printf("[-] Error :: analyzePDF :: Error while opening file %s\n",filename);
		return -1;
	}
	
	// Initialize the pdfDocument struct
	if(!(pdf = initPDFDocument())){
		printf("[-] Error :: analyzePDF :: Error while allocating memory for pdfDocument structure\n");
		fclose(f);
		return -1;
	}
	pdf->fh = f;	
	
	// Check the magic number of the file
	res = checkMagicNumber(pdf);

	
	if(pdf->testStruct->bad_header > 0){
		#ifdef DEBUG
		printf("[-] Error :: analyzePDF :: Bad PDF header :: This file is not a PDF file :: %s \n",filename);
		#endif
		printAnalysisReport(pdf,filename);
		freePDFDocumentStruct(pdf);
		return -2;
	}

	// PDF Parsing
	res = parsePDF(pdf);

	if(res < 0 ){

		// If the file is encrypted.
		if (res == -2) {
			printAnalysisReport(pdf,filename);
			freePDFDocumentStruct(pdf);
			return -2;
		}

		freePDFDocumentStruct(pdf);
		return -1;
	}
	
	#ifdef DEBUG
		//printPDFObjects(pdf);
	#endif
		
	
	// Object analysis
	if(pdf->objects != NULL){
		res = getDangerousContent(pdf);
	}


	// Document structure analysis
	res = documentStructureAnalysis(pdf);

	time(&end_time);
	time_elapsed = difftime(end_time,start_time);


	// print all objects references
	//printObjectReferences(pdf);

	// Analysis summary
	calcSuspiciousCoefficient(pdf);
	printAnalysisReport(pdf,filename);
	printf("Execution time : %.2lf sec \n",time_elapsed);
	printf("-------------------------------------------------------\n");
	printf("-------------------------------------------------------\n\n");

	ret = pdf->coef;
		
	
	freePDFDocumentStruct(pdf);
	//fclose(f);
	return ret;

}

/*
	int analyzePDF_fd(...) with file descriptor
	This function launch analysis on a PDF file (parsing, analysis and evaluation).
	Returns a suspicion coef on success.
	Returns -1 on error.
	Returns -2 if the file is not supported (bad header or encrypted file).
*/
int analyzePDF_fd(int fd, char * filename){


	int ret = 0;
	struct pdfDocument * pdf = NULL;
	time_t start_time, end_time;
	double time_elapsed = 0;
	int res = 0;

	// Check parameters
	if (fd < 0) {
		printf("[-] Error :: analyzePDF_fd :: Invalid parameter :: fd = %d\n",fd);
		return -1;
	}

	time(&start_time);

	
	// Initialize the pdfDocument struct
	if(!(pdf = initPDFDocument())){
		printf("[-] Error :: analyzePDF :: Error while allocating memory for pdfDocument structure\n");
		return -1;
	}
	pdf->fh = NULL;
	pdf->fd = fd;
	
	// Check the magic number of the file
	res = checkMagicNumber(pdf);

	
	if(pdf->testStruct->bad_header > 0){
		#ifdef DEBUG
		printf("[-] Error :: analyzePDF :: Bad PDF header :: This file is not a PDF file :: %s \n",filename);
		#endif
		printAnalysisReport(pdf,filename);
		freePDFDocumentStruct(pdf);
		return -2;
	}

	// PDF Parsing
	res = parsePDF(pdf);
	
	if(res < 0 ){

		// If the file is encrypted.
		if (res == -2) {
			printAnalysisReport(pdf,filename);
			freePDFDocumentStruct(pdf);
			return -2;
		}

		freePDFDocumentStruct(pdf);
		return -1;
	}
	
	
	#ifdef DEBUG
		//printPDFObjects(pdf);
	#endif
		
	
	// Object analysis
	if(pdf->objects != NULL){
		res = getDangerousContent(pdf);
	}


	// Document structure analysis
	res = documentStructureAnalysis(pdf);

	time(&end_time);
	time_elapsed = difftime(end_time,start_time);


	// print all objects references
	//printObjectReferences(pdf);

	// Analysis summary
	calcSuspiciousCoefficient(pdf);
	printAnalysisReport(pdf,filename);
	printf("Execution time : %.2lf sec \n",time_elapsed);
	printf("-------------------------------------------------------\n");
	printf("-------------------------------------------------------\n\n");

	ret = pdf->coef;
		
	
	freePDFDocumentStruct(pdf);
	//fclose(f);
	return ret;

}