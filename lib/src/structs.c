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


void free_active_contents(struct pdfActiveContent * ac){

	struct pdfActiveContent * tmp = NULL;

	while(ac!= NULL){

		tmp = ac;
		ac = ac->next;

		free(tmp->src);
		free(tmp->data);
		free(tmp);
		tmp = NULL;

	}

	return;
}


struct pdfActiveContent * init_active_content(enum acType type, char * src, char * data, unsigned int size){

	struct pdfActiveContent * ac;

	ac = (struct pdfActiveContent *)calloc(1,sizeof(struct pdfActiveContent));
	if( ac == NULL){
		return NULL;
	}

	ac->type = type;
	ac->src = strdup(src);
	ac->size = size;
	ac->data = (char*)calloc(size,sizeof(char));
	memcpy(ac->data,data, size);

	ac->next = NULL;

	return ac;
}


int add_pdf_active_content(struct pdfDocument * pdf, enum acType type, char * src, char * data, unsigned int size){

	struct pdfActiveContent * ac;
	struct pdfActiveContent * tmp;

	if(pdf == NULL || data == NULL || size == 0){
		err_log("add_active_content :: invalid parameters\n");
		return ERROR_INVALID_PARAMETERS;
	}

	ac = init_active_content(type, src, data, size);
	if(ac == NULL){
		err_log("add_active_content :: can't allocate memory!\n");
		return ERROR_INSUFFICENT_MEMORY;
	}

	if(pdf->activeContents == NULL){
		pdf->activeContents = ac;
		return ERROR_SUCCESS;
	}

	tmp = pdf->activeContents;

	while(tmp->next != NULL){
		tmp = tmp->next;
	}

	tmp->next = ac;

	return ERROR_SUCCESS;
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

	free_all_pdf_objects(pdf->objects);
	free_pdf_trailer(pdf->trailers);
	free_active_contents(pdf->activeContents);

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


struct pdfObject * init_pdf_object(char * ref, char * content, int obj_size, int offset){

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

		tmp = tmp->next;

		// obj ref collision detection
		if(strcmp(tmp->reference,obj->reference) == 0){
			warn_log("add_pdf_object :: Object reference collision :: %s\n", obj->reference);
			pdf->flags |= FLAG_OBJECT_COLLISION;
		}
	}

	tmp->next = obj;

	return EXIT_SUCCESS;
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
