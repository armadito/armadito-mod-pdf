#include "pdfAnalyzer.h"
//#include "src/includes/pdfParsing.h"


// This function analyze a pdf file given in parameters.

void Helper(){
	
	printf("UHURU PDF ANALYZER :: No file in parameter\n");
	printf("Command : ./pdfAnalyzer [filename]\n\n");
}



// This function print a report of the analysis
int analysisReport(struct pdfDocument * pdf, char * filename){

	printf("\n\n");
	printf("-------------------------------\n");
	printf("-- UHURU PDF ANALYZER REPORT --\n");
	printf("-------------------------------\n\n");

	printf("Filename = %s\n",filename);
	printf("Execution time = %d sec\n",0);
	printf("PDF version = %s\n",pdf->version);

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

	if(pdf->testStruct->large_file > 0)
		printf("Coef = LARGE_FILE\n");
	else
		if(pdf->testStruct->encrypted > 0)
			printf("Coef = Encrypted_PDF\n");
		else
			printf("Coef = %d\n",pdf->coef);


	printf("-------------------------------------------------------\n");
	printf("-------------------------------------------------------\n\n");

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





int analyze(char * filename){


	int ret = 0;
	FILE * f = NULL;
	struct pdfDocument * pdf = NULL;
	time_t start_time, end_time;
	double time_elapsed = 0;
	int res = 0;


	time(&start_time);
	
	// Open file
	if(!(f = fopen(filename,"rb"))){
		printf("Error while opening file %s\n",filename);
		return -1;
	}
	
	// Initialize the pdfDocument struct	
	if(!(pdf = initPDFDocument())){
		printf("Error while allocating memory for pdfDocument structure\n");
		fclose(f);
		return -1;
	}
	pdf->fh = f;	
	
	// Check the magic number of the 
	checkMagicNumber(pdf);

	if(pdf->testStruct->bad_header > 0){
		printf("Uhuru PDF analyzer :: Bad PDF header :: This file is not a PDF file :: %s \n",filename);
		return -2;
	}


	res = parsePDF(pdf);

	#ifdef DEBUG
		printPDFObjects(pdf);
	#endif

		
	if(res <= -2 ){		
		analysisReport(pdf,filename);
		freePDFDocumentStruct(pdf);
		return -2;
	}
	

	// Object analysis
	if(pdf->objects != NULL){
		getDangerousContent(pdf);
	}


	// Document structure analysis
	documentStructureAnalysis(pdf);

	time(&end_time);
	time_elapsed = difftime(end_time,start_time);


	// print all objects references
	//printObjectReferences(pdf);




	// Analysis summary
	calcSuspiciousCoefficient(pdf);
	analysisReport(pdf,filename);
	printf("\nExecution time : %.2lf sec \n",time_elapsed);

	//ret = pdf->coef;
	
	
	
	freePDFDocumentStruct(pdf);
	//fclose(f);
	return ret;

}


int main (int argc, char ** argv){

	int ret;
	

	printf("\nUHURU PDF ANALYZER\n\n");
	
	if(argc < 2){
		Helper();
		return (-1);
	}
	
	printf ("Analyzing file : %s\n",argv[1]);
	
	ret = analyze(argv[1]);
	

	return ret;
}

