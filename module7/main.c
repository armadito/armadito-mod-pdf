#include "pdfAnalyzer.h"
//#include "src/includes/pdfParsing.h"


// This function analyze a pdf file given in parameters.

void Helper(){
	
	printf("UHURU PDF ANALYZER :: No file in parameter\n");
	printf("Command : ./pdfAnalyzer [filename]\n\n");
}


// This function extracts objects embedded in onject streams
int extractObjectFromObjStream(struct pdfDocument * pdf){

	return 0;
}
 


int analyze(char * filename){


	int ret = 0;
	FILE * f = NULL;
	struct pdfDocument * pdf = NULL;
	
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
	
	// Get the content of the document
	getPDFContent(pdf);
	
	// Get objects described in pdf document
	getPDFObjects(pdf);
	
	// TODO Get Trailer
	getPDFTrailers_1(pdf);
	
	
	printf("DEBUG :: version %s\n",pdf->version);
	
	// TODO Get XRef
	
	freePDFDocumentStruct(pdf);
	//fclose(f);
	return ret;

}


int main (int argc, char ** argv){

	int ret;
	
	printf("UHURU PDF ANALYZER\n\n");
	
	if(argc < 2){
		Helper();
		return (-1);
	}
	
	printf ("Analyzing file : %s\n",argv[1]);
	
	ret = analyze(argv[1]);
	

	return ret;
}

