#include "pdfAnalyzer.h"



// This function free the memory allocated for the pdf structure
void freePDFDocumentStruct(struct pdfDocument * pdf){

	struct pdfObject * obj_tmp;
	struct pdfTrailer * trailer_tmp;
	
	if(pdf == NULL){
		return ;
	}
	
	//Close file handle
	fclose(pdf->fh);
	
	// Free objects
	if(pdf->objects){
	
		//tmp = pdf->objects;
		while(pdf->objects){
			obj_tmp = pdf->objects;
			pdf->objects = pdf->objects->next;
			free(obj_tmp);
			obj_tmp = NULL;
		}
	}
	
	// Free trailer
	if(pdf->trailers){
	
		while(pdf->trailers){
			trailer_tmp = pdf->trailers;
			pdf->trailers = pdf->trailers->next;
			free(trailer_tmp);
			trailer_tmp = NULL;
		}
	}
	
	// Free xref
	if(pdf->xref){
		
	}
	
	free(pdf);
	

	return ;

}


// return the object within the reference given in parameters
struct pdfObject * getPDFObjectByRef(struct pdfDocument * pdf, char * ref){

	struct pdfObject * obj = NULL;


	if(pdf == NULL || ref == NULL){
		printf("Bad argument in function get PDFObjectByRef\n");
		return NULL;
	}

	obj = pdf->objects;
	
	
	while(obj != NULL){
	
		if( strncmp(ref,obj->reference,strlen(ref)) == 0 ){
			return obj;
		}
		
		obj = obj->next;	
	}
	
	printf("Object \"%s\" not found\n",ref);

	return NULL;
}


// This function add an object in the list
int addObjectInList(struct pdfObject* obj, struct pdfDocument* pdf){

	struct pdfObject* tmp = NULL;

	if(obj == NULL){
		printf("Error :: addObjectInList :: Object is NULL\n");
		return -1;	
	}
	
	if(pdf->objects == NULL){
		pdf->objects = obj;
	}else{
		
		tmp = pdf->objects;
		while(tmp->next != NULL){
			tmp = tmp->next;	
		}
		tmp->next = obj;
		
	}
	
	return 0;
}


// Init a pdfDocument object structure
struct pdfDocument* initPDFDocument(){

	struct pdfDocument* pdf = NULL;
	
	if( !(pdf = (struct pdfDocument *)malloc(sizeof(struct pdfDocument)) ) ){
		printf("Error :: initPDFDocument :: memory allocation failed\n");
		return NULL;
	}
	
	
	// Initialize entries
	pdf->fh = NULL;
	pdf->content = NULL;
	pdf->objects =NULL;
	pdf->coef = 0;
	pdf->size = 0;
	pdf->version = NULL;
	pdf->trailers = NULL;
	pdf->xref = NULL;
	
	return pdf;

}


// Init a pdfObject object structure
struct pdfObject* initPDFObject(){

	struct pdfObject* obj = NULL;
	
	
	if( !(obj = (struct pdfObject*)malloc(sizeof(struct pdfObject)) ) ){
		printf("Error :: initPDFDocument :: memory allocation failed\n");
		return NULL;
	}
	
	// Initialize entries
	obj->reference = NULL;
	obj->content = NULL;
	obj->dico = NULL;
	obj->type = NULL;
	obj->stream = NULL;
	obj->decoded_stream = NULL;
	obj->offset = 0;
	obj->next = NULL;
	obj->stream_size = 0;
	obj->content_size = 0;
	
		
	
	return obj;

}

// Init a pdfDocument object structure
struct pdfTrailer* initPDFTrailer(){

	struct pdfTrailer* trailer = NULL;
	
	if( !(trailer = (struct pdfTrailer *)malloc(sizeof(struct pdfTrailer)) ) ){
		printf("Error :: initPDFTrailer :: memory allocation failed\n");
		return NULL;
	}
	
	// Initialize entries
	trailer->offset = 0;
	trailer->content = NULL;
	trailer->next = NULL;
	
	return trailer;

}

// This function search a pattern in a stream
void * searchPattern(char* src, char* pat , int pat_size ,  int size){

	char* res = NULL;
	
	int i =0;
	char * tmp = NULL;
	int diff = 0;
	
	
	
	//printf("src = %s\n",src);
	//printf("pat = %s\n",pat);
	//printf("hey! %d :: %d\n",src, size);
	if( size < pat_size || src == NULL || pat == NULL || pat_size == 0 || size == 0){
		printf("Err :: hey!\n");
		return NULL;
	}
	
	tmp =  (char*)malloc(pat_size*sizeof(char));
	//printf("hey2!\n");
	
	while(i < size - pat_size ){
	
		//i = 0;
		res = memchr(src,pat[0],size-i);
		//printf("src  = %d && res = %d\n",src,res);
		//i++;
		
		if(res == NULL){
			//printf("Not found\n");
			return NULL;
		}
		
		
		memcpy(tmp,res,pat_size);
		
		if( memcmp(tmp,pat,pat_size) == 0){
			return res;
		}
		
		diff = res - src;
		i+=diff;
		
		src = res + 1;
		
	}
	
	
	
	return NULL;
}


