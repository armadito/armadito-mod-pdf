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


#include <armaditopdf/structs.h>
#include <armaditopdf/log.h>
#include <armaditopdf/osdeps.h>
#include <armaditopdf/errors.h>



/*
freePDFObjectStruct() :: free the allocated memory PDF object structure.
parameters:
- struct pdfObject * pdf (the pdf object list pointer).
returns:
- none
*/
void freePDFObjectStruct(struct pdfObject * obj){

	struct pdfObject * tmp = NULL;

	if(obj == NULL){
		return ;
	}

	while(obj != NULL){

		tmp = obj;
		obj = obj->next;

		// free all elements		
		free(tmp->reference);
		free(tmp->content);
		free(tmp->dico);
		free(tmp->type);
		free(tmp->stream);
		free(tmp->filters);
		free(tmp->decoded_stream);
		
		free(tmp);
		tmp = NULL;

	}

	return;

}


/*
freePDFTrailerStruct() :: free the allocated memory PDF trailer structure.
parameters:
- struct pdfTrailer * pdf (the pdf trailer list pointer).
returns:
- none
*/
void free_pdf_trailer(struct pdfTrailer * trailer){

	struct pdfTrailer * tmp = NULL;

	while(trailer!= NULL){
		
		tmp = trailer;
		trailer = trailer->next;

		free(tmp->dico);
		free(tmp->content);
				
		free(tmp);
		tmp = NULL;

	}

	return;

}


/*
freePDFDocumentStruct() :: free the allocated memory PDF document structure.
parameters:
- struct pdfDocument * pdf (the pdf document pointer).
returns:
- none
*/
void freePDFDocumentStruct(struct pdfDocument * pdf){

	
	if(pdf == NULL){
		return ;
	}
	
	if (pdf->fname != NULL){
		free(pdf->fname);
		pdf->fname = NULL;
	}
	
	// Free objects
	if (pdf->objects != NULL){
		freePDFObjectStruct(pdf->objects);		
	}
	
	// Free trailer
	if (pdf->trailers != NULL){
		free_pdf_trailer(pdf->trailers);
	}
	
	if (pdf->fh != NULL){
		fclose(pdf->fh);
		pdf->fh = NULL;
	}

	if (pdf->version != NULL){
		free(pdf->version);
		pdf->version = NULL;
	}		

	if (pdf->content != NULL){
		free(pdf->content);
		pdf->content = NULL;
	}

	if (pdf->testStruct != NULL){
		free(pdf->testStruct);
		pdf->testStruct = NULL;
	}

	if (pdf->testObjAnalysis != NULL){
		free(pdf->testObjAnalysis);
		pdf->testObjAnalysis = NULL;
	}

	
	free(pdf);
	pdf = NULL;
	

	return ;

}


/*
addObjectInList() :: add an object in the pdf document object list
parameters:
- struct pdfObject * obj (pdf object pointer)
- struct pdfDocument * pdf (pdf document pointer)
returns: (int)
- 0 on success.
- an error code (<0) on error.
*/
int addObjectInList(struct pdfObject* obj, struct pdfDocument* pdf){

	struct pdfObject* tmp = NULL;

	if(obj == NULL || pdf == NULL){		
		err_log("addObjectInList :: invalid parameter\n");
		return -1;	
	}
	
	if(pdf->objects == NULL){
		pdf->objects = obj;
	}else{
		
		tmp = pdf->objects;

		// Object collision detection
		if(strncmp(tmp->reference,obj->reference,strlen(tmp->reference)) == 0 && strncmp(tmp->reference,obj->reference,strlen(obj->reference)) == 0){
			
			warn_log("addObjectInList :: Object collision :: %s\n", obj->reference);
			pdf->testStruct->object_collision ++;
		}


		while(tmp->next != NULL){

			// Object collision detection			
			tmp = tmp->next;

			if(strncmp(tmp->reference,obj->reference,strlen(tmp->reference)) == 0 && strncmp(tmp->reference,obj->reference,strlen(obj->reference)) == 0){
				warn_log("addObjectInList :: Object collision :: %s\n", obj->reference);
				pdf->testStruct->object_collision ++;
			}	
		}
		tmp->next = obj;
		
	}
	
	return 0;
}


/*
initTestsPDFStruct() :: Initialize PDF Tests structure.
parameters:
- none
returns: (struct testsPDFStruct *)
- the testsPDFStruct pointer on success.
- NULL on error.
*/
struct testsPDFStruct * initTestsPDFStruct(){

	struct testsPDFStruct * testStruct = NULL;

	if( !(testStruct = (struct testsPDFStruct *)calloc(1,sizeof(struct testsPDFStruct)) ) ){
		err_log("initTestsPDFStruct :: memory allocation failed\n");
		return NULL;
	}

	testStruct->bad_header = 0;
	testStruct->encrypted = 0;
	testStruct->empty_page_content = 0;
	testStruct->object_collision = 0;
	testStruct->bad_trailer = 0;
	testStruct->bad_xref_offset = 0;
	testStruct->bad_obj_offset = 0;
	testStruct->obfuscated_object = 0;
	testStruct->multiple_headers = 0;
	testStruct->large_file = 0;
	testStruct->comments = 0;
	testStruct->malicious_comments = 0;

	return testStruct;
}


/*
initTestsPDFObjAnalysisStruct() :: Initialize PDF Tests structure.
parameters:
- none
returns: (struct testsPDFObjAnalysis *)
- the testsPDFObjAnalysis pointer on success.
- NULL on error.
*/
struct testsPDFObjAnalysis * initTestsPDFObjAnalysisStruct(){

	struct testsPDFObjAnalysis * testObjAnalysis = NULL;

	if( !(testObjAnalysis = (struct testsPDFObjAnalysis *)calloc(1,sizeof(struct testsPDFObjAnalysis)) ) ){
		err_log("initTestsPDFObjAnalysisStruct :: memory allocation failed\n");
		return NULL;
	}

	testObjAnalysis->active_content = 0;
	testObjAnalysis->shellcode = 0;
	testObjAnalysis->pattern_high_repetition = 0;
	testObjAnalysis->dangerous_keyword_high = 0;
	testObjAnalysis->dangerous_keyword_medium = 0;
	testObjAnalysis->dangerous_keyword_low = 0;
	testObjAnalysis->time_exceeded = 0;

	testObjAnalysis->js = 0;
	testObjAnalysis->xfa = 0;
	testObjAnalysis->ef = 0;

	return testObjAnalysis;
}


struct pdfDocument * init_pdf_document(int fd, FILE * fh, char * filename, char * version){

	struct pdfDocument * pdf;

	pdf = (struct pdfDocument *)calloc(1,sizeof(struct pdfDocument));
	if( pdf == NULL){
		return NULL;
	}

	pdf->fd = fd;
	pdf->fh = fh;
	pdf->fname =  os_strdup(filename);
	pdf->version = version;

	return pdf;

}


void free_pdf_document(struct pdfDocument * pdf ){

	if(pdf == NULL)
		return ;

	if (pdf->fname != NULL){
		free(pdf->fname);
		pdf->fname = NULL;
	}

	if (pdf->version != NULL){
		free(pdf->version);
		pdf->version = NULL;
	}

	if (pdf->content != NULL){
		free(pdf->content);
		pdf->content = NULL;
	}

	if (pdf->testStruct != NULL){
		free(pdf->testStruct);
		pdf->testStruct = NULL;
	}

	if (pdf->testObjAnalysis != NULL){
		free(pdf->testObjAnalysis);
		pdf->testObjAnalysis = NULL;
	}

	free_all_pdf_objects(pdf->objects);
	free_pdf_trailer(pdf->trailers);

	free(pdf);
	pdf = NULL;
	

	return ;
}


struct pdfTrailer * init_pdf_trailer(char * content, unsigned int size){

	struct pdfTrailer * trailer;

	trailer = (struct pdfTrailer *)calloc(1,sizeof(struct pdfTrailer));
	if(trailer == NULL){
		return NULL;
	}

	trailer->content = content;
	trailer->size = size;

	return trailer;
}


/* DEPRECATED
initPDFDocument() :: Initialize pdfDocument structure.
parameters:
- none
returns: (struct pdfDocument *)
- the pdfDocument pointer on success.
- NULL on error.
*/
struct pdfDocument* initPDFDocument(){

	struct pdfDocument* pdf = NULL;
	int err = 0;
	
	if( (pdf = (struct pdfDocument *)calloc(1,sizeof(struct pdfDocument))) == NULL ){
		err_log("initPDFDocument :: memory allocation failed\n");
		err++;
		goto clean;		
	}

	if( (pdf->testStruct = initTestsPDFStruct()) == NULL ){
		err_log("initPDFDocument :: testsPDFstruct initialization failed!\n");
		err++;
		goto clean;
	}

	if( (pdf->testObjAnalysis = initTestsPDFObjAnalysisStruct()) == NULL ){
		err_log("initPDFDocument :: testsPDFObjAnalysisStruct initialization failed!\n");
		err++;
		goto clean;		
	}


	// Initialize entries
	pdf->fh = NULL;
	pdf->fd = -1;
	pdf->fname = NULL;
	pdf->content = NULL;
	pdf->objects =NULL;
	pdf->coef = 0;
	pdf->size = 0;
	pdf->version = NULL;
	pdf->trailers = NULL;
	pdf->xref = NULL;
	pdf->errors = 0;
	pdf->scan_time=0;


clean:
	if (err != 0){
		if (pdf != NULL){
			freePDFDocumentStruct(pdf);
			pdf = NULL;
		}
	}


	return pdf;

}


/*
initPDFObject() :: Initialize pdfObject object structure.
parameters:
- none
returns: (struct pdfObject *)
- the pdfObject pointer on success.
- NULL on error.
*/
struct pdfObject* initPDFObject(){

	struct pdfObject* obj = NULL;
	
	
	if( !(obj = (struct pdfObject*)calloc(1,sizeof(struct pdfObject)) ) ){
		err_log("initPDFObject :: memory allocation failed\n");
		return NULL;
	}
	
	// Initialize entries
	obj->reference = NULL;
	obj->content = NULL;
	obj->dico = NULL;
	obj->type = NULL;
	obj->stream = NULL;
	obj->filters = NULL;
	obj->decoded_stream = NULL;
	obj->offset = 0;
	obj->next = NULL;
	obj->stream_size = 0;
	obj->tmp_stream_size = 0;
	obj->content_size = 0;		// TODO: change to obj->size
	obj->decoded_stream_size = 0;
	obj->errors = 0;
	
	return obj;

}


struct pdfObject* init_pdf_object(char * ref, char * content, int obj_size, int offset){

	struct pdfObject * obj;

	obj = (struct pdfObject *)calloc(1,sizeof(struct pdfObject));
	if(obj == NULL){
		return NULL;
	}

	obj->reference = ref;
	obj->content = content;
	obj->content_size = obj_size;
	obj->offset = offset;

	return obj;
}

void free_pdf_object(struct pdfObject * obj){

	if(obj == NULL)
		return;

	if(obj->content != NULL){
		free(obj->content);
		obj->content = NULL;
	}

	if(obj->reference != NULL){
		free(obj->reference);
		obj->reference = NULL;
	}

	if(obj->dico != NULL){
		free(obj->dico);
		obj->dico = NULL;
	}

	if(obj->stream != NULL){
		free(obj->stream);
		obj->stream = NULL;
	}

	if(obj->type != NULL){
		free(obj->type);
		obj->type = NULL;
	}

	if(obj->filters != NULL){
		free(obj->filters);
		obj->filters = NULL;
	}

	if(obj->decoded_stream != NULL){
		free(obj->decoded_stream);
		obj->decoded_stream = NULL;
	}

	free(obj);
	obj=NULL;

	return;
}

void free_all_pdf_objects(struct pdfObject * obj){

	struct pdfObject * tmp;

	if(obj == NULL)
		return ;

	while(obj != NULL){
		tmp = obj;
		obj = obj->next;
		free_pdf_object(tmp);
	}

	return;
}

int add_pdf_object(struct pdfDocument * pdf, struct pdfObject * obj){

	struct pdfObject * tmp;

	if(pdf == NULL || obj == NULL){
		err_log("add_pdf_object :: invalid parameters\n");
		return ERROR_INVALID_PARAMETERS;
	}

	if(pdf->objects == NULL){
		pdf->objects = obj;
		return EXIT_SUCCESS;
	}

	tmp = pdf->objects;

	while(tmp->next != NULL){
		// TODO: Check object reference collision.
		tmp = tmp->next;
	}

	tmp->next = obj;

	return EXIT_SUCCESS;
}



/* DEPRECATED
initPDFTrailer() :: Initialize pdf trailer structure
parameters:
- none
returns: (struct pdfTrailer *)
- the pdfTrailer pointer on success.
- NULL on error.
*/
struct pdfTrailer* initPDFTrailer(){

	struct pdfTrailer* trailer = NULL;
	
	if( !(trailer = (struct pdfTrailer *)calloc(1,sizeof(struct pdfTrailer)) ) ){
		err_log("initPDFTrailer :: memory allocation failed\n");
		return NULL;
	}
	
	// Initialize entries
	trailer->offset = 0;
	trailer->content = NULL;
	trailer->dico = NULL;
	trailer->next = NULL;
	
	return trailer;

}


int add_pdf_trailer(struct pdfDocument * pdf, struct pdfTrailer * trailer){

	struct pdfTrailer * tmp =  NULL;

	if(pdf == NULL || trailer == NULL){
		err_log("add_pdf_trailer :: invalid parameters\n");
		return ERROR_INVALID_PARAMETERS;
	}


	if(pdf->trailers == NULL){
		pdf->trailers = trailer;
	}else{

		tmp = pdf->trailers;
		while(tmp->next != NULL){
			tmp = tmp->next;
		}
		tmp->next = trailer;

	}

	return EXIT_SUCCESS;
}