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


#include <armaditopdf/parsing.h>
#include <armaditopdf/utils.h>
#include <armaditopdf/osdeps.h>
#include <armaditopdf/log.h>
#include <armaditopdf/filters.h>
#include <armaditopdf/errors.h>


char * pdf_get_version_from_data(char * data, unsigned int data_size){

	int minor_off = 7; // minor version offset.
	char * version = NULL;
	int ver_size = 9;

	if(data == NULL || data_size == 0)
		return NULL;

	if(!strcmp(data,"%PDF-1.") == 0 && data[minor_off] >= '1' && data[minor_off] <= '7' ){

		version = (char*)malloc(ver_size*sizeof(char));
		strncpy(version,data,ver_size);

		dbg_log("pdf_get_header :: PDF version = %s\n", version);

		return version;
	}

	err_log("pdf_get_header :: bad pdf version number\n");

	return version;
}


char * pdf_get_version_from_fd(int fd, int * retcode){

	char * data = NULL, *version = NULL;
	unsigned int doc_size, read_bytes;

	doc_size =  os_lseek(fd,0,SEEK_END);
	os_lseek(fd,0,SEEK_SET);

	if(doc_size == 0){
		*retcode = ERROR_ZERO_LENGTH_FILE;
		return NULL;
	}

	doc_size = (doc_size > 1024) ? 1024 : doc_size;

	data = (char*)calloc(doc_size+1, sizeof(char));
	if(data == NULL){
		*retcode = ERROR_INSUFFICENT_MEMORY;
		return NULL;
	}
	data[doc_size] = '\0';

	read_bytes = os_read(fd, data, doc_size);
	os_lseek(fd,0,SEEK_SET);

	if(read_bytes != doc_size){
		free(data);
		*retcode = ERROR_FILE_READ;
		return NULL;
	}

	version = pdf_get_version_from_data(data, doc_size);
	if(version == NULL){
		* retcode = ERROR_INVALID_FORMAT;
	}

	free(data);

	return version;

}


int pdf_get_content(struct pdfDocument * pdf){

	char * content = NULL;
	int doc_size, read_bytes;


	if(pdf == NULL){
		return ERROR_INVALID_PARAMETERS;
	}

	if (pdf->fh == NULL && pdf->fd < 0) {
		err_log("get_get_ontent :: invalid file handle or file descriptor!\n");
		return ERROR_INVALID_FD;
	}

	if (pdf->fh != NULL) {

		fseek(pdf->fh,0,SEEK_END);
		doc_size = ftell(pdf->fh);
		fseek(pdf->fh,0,SEEK_SET);

		if(doc_size == 0)
			return ERROR_ZERO_LENGTH_FILE;

		content = (char*)calloc(doc_size+1, sizeof(char));
		if(content == NULL)
			return ERROR_INSUFFICENT_MEMORY;

		content[doc_size]= '\0';
		read_bytes = fread(content, 1, doc_size, pdf->fh);

		if(read_bytes != doc_size){
			free(content);
			return ERROR_FILE_READ;
		}
	}
	else if(pdf->fd > 0) {

		doc_size =  os_lseek(pdf->fd,0,SEEK_END);
		os_lseek(pdf->fd,0,SEEK_SET);

		if(doc_size == 0)
			return ERROR_ZERO_LENGTH_FILE;

		content = (char*)calloc(doc_size+1, sizeof(char));
		if(content == NULL)
			return ERROR_INSUFFICENT_MEMORY;

		content[doc_size]= '\0';
		read_bytes = os_read(pdf->fd, content, doc_size);
		os_lseek(pdf->fd,0,SEEK_SET);

		if(read_bytes != doc_size){
			free(content);
			return ERROR_FILE_READ;
		}
	}

	pdf->content = content;	
	pdf->size = read_bytes;

	return EXIT_SUCCESS;
	
}


char * get_dico_from_data(char *data, unsigned int data_size){

	char * dico = NULL;
	char * start = NULL;		// dico start pointer
	char * end = NULL;			// dico end pointer
	int inQuotes = 0;
	int inString = 0;
	int sub = 0;
	int flag = 0;
	int len = 0;


	if (data == NULL || data_size == 0){
		err_log("get_dico_from_data :: invalid parameter\n");
		return NULL;
	}

	start = searchPattern(data,"<<",2,data_size);
	if(start == NULL){
		return NULL;
	}

	// search other occurencies of "<<" to detect sub dictionaries
	// check if you find the same number of occurencies of stream ">>"
	len = data_size - (int)(start - data);
	end = start;

	while(len >= 2 && flag == 0){

		// String delimiter
		if(inQuotes == 0 && end[0] == '(' && ((end == start) || (end > start && (end-1)[0] != '\\')) ){
			inString ++;
			len--;
			end ++;
			continue;
		}

		// String delimiter 2
		if(inQuotes == 0 && inString > 0 && end[0] == ')' && ((end == start) || (end > start && (end-1)[0] != '\\')) ){
			inString --;
			len --;
			end++;
			continue;
		}

		// Quotes delimiter
		if(end[0] == '"' && ((end == start) || (end > start && (end-1)[0] != '\\')) ){
			inQuotes = (inQuotes == 0)?1:0;
			len --;
			end++;
			continue;
		}

		// Dico delimiter
		if(inString == 0 && end[0] == '<' && end[1] == '<' && ((end == start) || (end > start && (end-1)[0] != '\\')) ){
			sub ++;
			end+=2;
			len -=2;
			continue;
		}

		// Dico delimiter
		if(inString == 0 && end[0] == '>' && end[1] == '>' && ((end == start) || (end > start && (end-1)[0] != '\\')) ){
			sub --;
			end+=2;
			len -=2;

			if(sub == 0){
				flag ++;
			}

			continue;
		}

		len --;
		end ++;

	}


	// if dico found
	if(flag != 0){

		len = (unsigned int)(end - start);
		dico = (char*)calloc(len+1,sizeof(char));
		if(dico == NULL){
			err_log("get_dico_from_data :: memory allocation faild!\n");
			return NULL;
		}

		dico[len]='\0';
		memcpy(dico,start,len);
	}


	return dico;
}


/* Get PDF trailers according to PDF (version 1.1 to 1.4) specifications */
int pdf_get_trailers_v1_to_v4(struct pdfDocument * pdf){

	char * content = NULL;
	char * start, *end;
	int len;
	struct pdfTrailer * trailer = NULL;
	int retcode = ERROR_TRAILER_NOT_FOUND;


	if(pdf == NULL || pdf->content == NULL || pdf->size <= 0){
		return ERROR_INVALID_PARAMETERS;
	}

	end = pdf->content;
	len = pdf->size;

	while((start = searchPattern(end,"trailer",7,len))){

		len = pdf->size - (unsigned int)(start - end);
		end = searchPattern(start,"%%EOF",5,len);
		if (end == NULL){
			warn_log("pdf_get_trailer :: missing eof marker!\n");
			retcode = ERROR_BAD_TRAILER_FORMAT;
			end = start + 7; // search another trailer.
			len = pdf->size - (unsigned int)( end - pdf->content);
			continue;
		}

		end += 5; 	// skipping %%EOF string.

		len = (unsigned int)(end - start);
		content = (char*)calloc(len+1,sizeof(char));
		if(content == NULL){
			return ERROR_INSUFFICENT_MEMORY;
		}

		content[len] = '\0';
		memcpy(content,start,len);

		trailer = init_pdf_trailer(content,len);
		if( trailer == NULL){
			free(content);
			return ERROR_INSUFFICENT_MEMORY;
		}

		// Get trailer dictionary.
		trailer->dico = get_dico_from_data(trailer->content, trailer->size);
		if( trailer->dico == NULL){
			free_pdf_trailer(trailer);
			err_log("pdf_get_trailer :: Bad trailer format!\n");
			return ERROR_BAD_TRAILER_FORMAT;
		}

		printf("trailre_dico = %s\n", trailer->dico);

		retcode = add_pdf_trailer(pdf, trailer);
		if(retcode != EXIT_SUCCESS){
			err_log("pdf_get_trailer :: add pdf trailer failed!\n");
			free_pdf_trailer(trailer);
			return retcode;
		}

		len = pdf->size - (unsigned int)( end - pdf->content);

	}

	if(pdf->trailers == NULL)
		return retcode;

	return EXIT_SUCCESS;
}


/* Get PDF trailers according to PDF (version from to 1.5) specifications */
int pdf_get_trailers_from_v5(struct pdfDocument * pdf){

	char * content = NULL;
	char * start = NULL;
	char * end = NULL;
	int len = 0;
	struct pdfTrailer * trailer;
	int retcode = ERROR_TRAILER_NOT_FOUND;


	if(pdf == NULL || pdf->content == NULL || pdf->size <= 0){
		return ERROR_INVALID_PARAMETERS;
	}

	end = pdf->content;
	len = pdf->size;

	while( (start = searchPattern(end,"startxref",9,len) ) != NULL ){

		len = pdf->size - (unsigned int)(start - end);
		end = searchPattern(start,"%%EOF",5,len);
		if (end == NULL){
			warn_log("pdf_get_trailer :: missing eof marker!\n");
			retcode = ERROR_BAD_TRAILER_FORMAT;
			end = start + 9; // search another trailer.
			len = pdf->size - (unsigned int)( end - pdf->content);
			continue;
		}

		end += 5;
		len = (unsigned int)(end - start);
		content = (char*)calloc(len+1,sizeof(char));
		if(content == NULL){
			return ERROR_INSUFFICENT_MEMORY;
		}

		content[len] = '\0';
		memcpy(content,start,len);

		trailer = init_pdf_trailer(content,len);
		if( trailer == NULL){
			free(content);
			return ERROR_INSUFFICENT_MEMORY;
		}

		retcode = add_pdf_trailer(pdf, trailer);
		if(retcode != EXIT_SUCCESS){
			free_pdf_trailer(trailer);
			return retcode;
		}

		len = pdf->size - (unsigned int)( end - pdf->content);

	}

	if(pdf->trailers == NULL)
		return retcode;

	return EXIT_SUCCESS;
}


int pdf_get_trailers(struct pdfDocument * pdf){

	int retcode = EXIT_SUCCESS;
	char minor_ver;

	if(pdf == NULL || pdf->version == NULL){
		return ERROR_INVALID_PARAMETERS;
	}

	minor_ver = pdf->version[7];

	retcode = pdf_get_trailers_v1_to_v4(pdf);

	if(retcode != EXIT_SUCCESS && minor_ver >= '5' && minor_ver <= '7')
		retcode = pdf_get_trailers_from_v5(pdf);

	return retcode;
}


// TODO: add obj_ptr_size for params verifs
char * get_ref_stepback_from_ptr(char * obj_ptr, int ptr_size, int ptr_limit){

	char * tmp;
	char * ref;
	unsigned int ref_size = 0;

	if(obj_ptr == NULL || ptr_limit < 4){ // 4 = min_ref_size - "obj"
		return NULL;
	}

	tmp = obj_ptr;

	if(strncmp(tmp,"obj",3)!=0 ){
		return NULL;
	}

	tmp --;
	ref_size++;
	ptr_limit --;

	// check space
	if(tmp[0] != ' ')
		return NULL;

	// check generation number -TODO: check if it's a digit.
	do{
		tmp --;
		ptr_limit --;
		ref_size++;
	}while(ptr_limit > 0 && tmp[0] >= 48 && tmp[0] <= 57);

	// check space
	if(tmp[0] != ' ')
		return NULL;

	// check object number
	do{
		tmp --;
		ptr_limit --;
		ref_size++;
	}while(ptr_limit > 0 && tmp[0] >= 48 && tmp[0] <= 57);

	// skip '\n' or '\r'
	if(tmp[0] != '\n' && tmp[0]!= '\r')
		return NULL;

	tmp ++;
	ptr_limit ++;
	ref_size--;


	if(ptr_limit == 0 || ref_size == 0)
		return NULL;

	ref_size += 3;

	ref = (char*)calloc(ref_size+1,sizeof(char));
	if(ref == NULL){
		return NULL;
	}

	ref[ref_size]='\0';
	memcpy(ref,tmp,ref_size);

	return ref;
}


char * decode_hexa_obfuscation(char * dico){

	int size;
	int is_space_hexa = 1;
	char * decoded_dico = NULL;
	char * start;
	char * hexa;
	char * hexa_decoded;
	char * tmp_dico;


	if (dico == NULL) {
		err_log("decode_hexa_obfuscation :: invalid parameter\n");
		return NULL;
	}

	hexa = (char*)calloc(4,sizeof(char));
	hexa[3] = '\0';

	hexa_decoded = (char*)calloc(2,sizeof(char));
	hexa_decoded[1] = '\0';

	start = dico;
	size = strlen(dico);

	tmp_dico = (char*)calloc(size+1,sizeof(char));
	tmp_dico[size]= '\0';
	memcpy(tmp_dico, dico, size);

	while(size >= 3 && (start = getHexa(start, size)) != NULL ){

		strncpy(hexa, start,3);

		// #20 = space - #2F = '/' (slash) - #E9 = é - #2C = ','
		if(strcmp(hexa,"#20") != 0 && strcmp(hexa,"#2F") != 0 && strcmp(hexa,"#E9") != 0 && strcmp(hexa,"#2C") != 0){

			printf("hexa = %s\n",hexa);
			is_space_hexa = 0;

			os_sscanf(hexa,"#%x",&hexa_decoded[0]);

			decoded_dico = replaceInString(tmp_dico,hexa,hexa_decoded);
			//printf("decoded_dico = %s\n",decoded_dico);

			if(decoded_dico != NULL && decoded_dico != tmp_dico){
				free(tmp_dico);
				tmp_dico = decoded_dico;
			}

		}

		start += 3;
		size = strlen(dico) - (int)(start - dico);

	}

	if(is_space_hexa == 1){
		free(tmp_dico);
	}

	free(hexa);
	free(hexa_decoded);

	return decoded_dico;
}


int pdf_parse_object_dico(struct pdfObject * obj){

	char * dico;
	char * hexa_decoded;
	int retcode = ERROR_SUCCESS;

	if(obj == NULL || obj->content == NULL || obj->content_size <= 0){
		return ERROR_INVALID_PARAMETERS;
	}

	dico = get_dico_from_data(obj->content, obj->content_size);
	if(dico == NULL){
		return ERROR_OBJ_DICO_NOT_FOUND;
	}

	hexa_decoded = decode_hexa_obfuscation(dico);

	if(hexa_decoded != NULL){
		free(dico);
		obj->dico = hexa_decoded;
		retcode = ERROR_OBJ_DICO_OBFUSCATION;
	}else{
		obj->dico = dico;
	}

	return retcode;
}


int pdf_parse_object_type(struct pdfObject * obj){

	char * type;
	char * start_ptr;
	char * end_ptr;
	int type_size = 0;
	int tmp_size = 0;
	int type_flag  = 0;
	int sub = 0;


	if (obj == NULL || obj->dico == NULL){
		err_log("getObjectType :: invalid parameter\n");
		return ERROR_INVALID_PARAMETERS;
	}

	tmp_size = strlen(obj->dico);
	start_ptr = obj->dico;

	// skip first dico delimiter
	while(start_ptr[0] == '<'){
		start_ptr ++;
	}

	// Search /Type keyword (but outbound of a sub dictionary)
	while(tmp_size >= 5 && type_flag == 0){

		// open sub dico
		if(strncmp(start_ptr,"<<",2 ) == 0 ){
			sub ++;
			start_ptr += 2;
			tmp_size = strlen(obj->dico) - (int)(start_ptr - obj->dico);
			continue;
		}

		// close sub dico
		if(strncmp(start_ptr,">>",2 ) == 0 ){
			sub --;
			start_ptr += 2;
			tmp_size = strlen(obj->dico) - (int)(start_ptr - obj->dico);
			continue;
		}

		if(sub == 0 && strncmp(start_ptr,"/Type",5) == 0 && start_ptr[5] != '0' && start_ptr[5] != '1' && start_ptr[5] != '2' ){
			type_flag ++;
			continue;
		}

		start_ptr++;
		tmp_size = strlen(obj->dico) - (int)(start_ptr - obj->dico);

	}

	// if type found
	if(type_flag > 0){

		// skip /Type string and white space.
		start_ptr += 5;

		while(start_ptr[0] == ' '){
			start_ptr ++;
		}

		end_ptr = start_ptr+1;

		while( (end_ptr[0] >= 'a' && end_ptr[0] <= 'z') || (end_ptr[0] >='A' && end_ptr[0] <='Z' ) || (end_ptr[0] >='0' && end_ptr[0] <= '9') ){
			end_ptr ++;
			type_size ++;
		}

		if(type_size == 0)
			return ERROR_INVALID_OBJ_TYPE;

		type_size ++; // add it for '/'
		type = (char*)calloc(type_size+1,sizeof(char));
		type[type_size]= '\0';

		memcpy(type,start_ptr,type_size);
		obj->type = type;

	}

	return ERROR_SUCCESS;
}


int pdf_parse_object_stream(struct pdfObject * obj){


	char * stream;
	char * start;
	char * end;
	int len = 0;

	if (obj == NULL || obj->content == NULL){
		err_log("parse_obj_stream :: invalid parameter\n");
		return ERROR_INVALID_PARAMETERS;
	}

	// TODO: Skip dictionary if any.

	start = searchPattern(obj->content,"stream",6,obj->content_size);
	if(start == NULL){
		return ERROR_SUCCESS;
	}

	len = obj->content_size -(int)(start - obj->content);

	end = searchPattern(start,"endstream",9,len);
	if(end == NULL){
		return ERROR_SUCCESS;
	}

	// Remove white space after "stream" and before "endstream"
	start += 6;
	while(start[0] == '\n' || start[0] == '\r'){
		start ++;
	}

	end --;
	while(end[0] == '\n' || end[0] == '\r'){
		end --;
	}
	end++;

	len = (int)(end-start);
	if(len <= 0 ){
		warn_log("parse_obj_stream :: Empty stream content in object %s\n", obj->reference);
		return ERROR_INVALID_OBJ_STREAM;
	}

	obj->stream_size = len;
	obj->tmp_stream_size = obj->stream_size;

	stream = (char*)calloc(len+1,sizeof(char));
	stream[len]='\0';
	memcpy(stream,start,len);

	obj->stream = stream;

	return ERROR_SUCCESS;
}


char * obj_get_stream_filters(struct pdfObject * obj, int * retcode){

	char * start;
	int len = 0;
	char * filters = NULL;

	if (obj == NULL || obj->dico == NULL){
		err_log("obj_get_stream_filters :: invalid parameter\n");
		*retcode = ERROR_INVALID_PARAMETERS;
		return NULL;
	}

	start = searchPattern(obj->dico,"/Filter",7,strlen(obj->dico));
	if( start == NULL ){
		*retcode = ERROR_NO_STREAM_FILTERS;
		return NULL;
	}

	start+=7;

	// skip white spaces
	while(start[0] == ' ' || start[0] == '\n' || start[0] == '\r')
		start++;

	len = strlen(obj->dico) - (int)(start - obj->dico);

	if(start[0] == '/'){

		filters = get_name_object(start,len);

	}else if(start[0] == '['){

		filters = getDelimitedStringContent(start,"[","]",len);
	}

	if(filters == NULL)
		*retcode = ERROR_INVALID_STREAM_FILTERS;

	return filters;
}


int obj_decode_stream(struct pdfObject * obj){

	int retcode = ERROR_SUCCESS;
	char * filters, *f;
	char * start, * end;
	char * stream, *tmp;
	int len = 0;
	int stream_size;
	int filter_applied = 0;


	if(obj == NULL || obj->stream == NULL)
		return ERROR_INVALID_PARAMETERS;

	// Check if the stream is encrypted
	if(obj->dico != NULL && searchPattern(obj->dico,"/Encrypt",7,strlen(obj->dico)) != NULL ){
		printf("Encrypted content! %s\n", obj->dico);
		return ERROR_ENCRYPTED_CONTENT;
	}

	filters = obj_get_stream_filters(obj, &retcode);
	if(filters == NULL){
		return retcode;
	}

	dbg_log("obj (%s) filters = %s\n",obj->reference, filters);

	end = filters;

	stream = (char*)calloc(obj->stream_size+1,sizeof(char));
	stream[obj->stream_size]='\0';
	memcpy(stream, obj->stream, obj->stream_size);
	stream_size = obj->stream_size;

	while((start = strchr(end, '/')) != NULL && retcode == ERROR_SUCCESS){

		len = strlen(filters) - (int)(start - end);
		f = get_name_object(start,len);
		end += strlen(f);

		if( strcmp(f,"/FlateDecode") == 0 || strcmp(f,"/Fl") == 0){

			tmp = FlateDecode(stream, &stream_size, obj);
			if( tmp == NULL){
				err_log("obj_decode_stream :: Decoding obj (%s) stream failed!\n", obj->reference);
				retcode = ERROR_STREAM_FLATEDECODE;
			}

			free(stream);
			stream = tmp;
			filter_applied ++;

		}else if( strcmp(f,"/ASCIIHexDecode") == 0 || strcmp(f,"/AHx") == 0){

			tmp = ASCIIHexDecode(stream,&stream_size,obj);
			if( tmp == NULL){
				err_log("obj_decode_stream :: Decoding obj (%s) stream failed!\n", obj->reference);
				retcode = ERROR_STREAM_ASCIIHEXDECODE;
			}

			free(stream);
			stream = tmp;
			filter_applied ++;

		}else if( strcmp(f,"/ASCII85Decode") == 0 || strcmp(f,"/A85") == 0){

			/*tmp = ASCII85Decode(stream,&stream_size,obj);
			if( tmp == NULL){
				err_log("obj_decode_stream :: Decoding obj (%s) stream failed!\n", obj->reference);
				retcode = ERROR_STREAM_ASCII85DECODE;
			}

			free(stream);
			stream = tmp;
			filter_applied ++;
			*/

		}else if( strcmp(f,"/LZWDecode") == 0 || strcmp(f,"/LZW") == 0){

			/*tmp = LZWDecode(stream,&stream_size,obj);
			if( tmp == NULL){
				err_log("obj_decode_stream :: Decoding obj (%s) stream failed!\n", obj->reference);
				retcode = ERROR_STREAM_LZWDECODE;
			}

			free(stream);
			stream = tmp;
			filter_applied ++;
			*/

		}else if( strcmp(f,"/RunLengthDecode") == 0 || strcmp(f,"/RL") == 0){

			err_log("obj_decode_stream :: RunLengthDecode filter not implemented yet!\n");
			retcode = ERROR_FILTER_NOT_IMPLEMENTED;

		}else if( strcmp(f,"/CCITTFaxDecode") == 0 || strcmp(f,"/CCF") == 0){

			/*tmp = CCITTFaxDecode(stream,&stream_size,obj);
			if( tmp == NULL){
				err_log("obj_decode_stream :: Decoding obj (%s) stream failed!\n", obj->reference);
				retcode = ERROR_STREAM_CCITTFAXDECODE;
			}

			free(stream);
			stream = tmp;
			filter_applied ++;
			*/

		}else{

			err_log("obj_decode_stream :: %s filter not implemented!\n");
			retcode = ERROR_FILTER_NOT_IMPLEMENTED;

		}

		free(f);

	}

	free(filters);

	if(filter_applied == 0)
		retcode = ERROR_FILTER_NOT_IMPLEMENTED;


	if(retcode == ERROR_SUCCESS && stream != NULL){
		obj->decoded_stream = stream ;
		obj->decoded_stream_size = stream_size;
	}
	else if (stream != NULL) {
		free(stream);
	}

	return retcode;
}


int pdf_extract_objstm(struct pdfDocument * pdf, struct pdfObject * obj){

	int retcode = ERROR_SUCCESS;
	int first = 0, num = 0, len = 0, i = 0, check = 0;
	int obj_off = 0, obj_ref_len = 0, obj_size = 0;
	int stream_size = 0;
	int * obj_offsets;
	char * stream;
	char * start;
	char * obj_data_ptr;
	char * obj_num_s;
	char * obj_off_s;
	char * obj_ref;
	char * obj_content;

	struct pdfObject * comp_obj;

	if( pdf == NULL || obj == NULL ){
		err_log("pdf_extract_objstm :: invalid parameters\n");
		return ERROR_INVALID_PARAMETERS;
	}

	retcode = obj_decode_stream(obj);
	if(retcode != ERROR_SUCCESS && retcode != ERROR_NO_STREAM_FILTERS)
		return retcode;

	if(retcode == ERROR_NO_STREAM_FILTERS){
		stream = obj->stream;
		stream_size = obj->stream_size;
	}else{
		stream = obj->decoded_stream;
		stream_size = obj->decoded_stream_size;
	}

	// Get the number of object embedded in the stream => N in the dictionary
	start = searchPattern(obj->dico, "/N", 2, strlen(obj->dico));
	if ( start == NULL){
		err_log("pdf_extract_objstm :: Entry /N not found in Object stream dictionary %s\n",obj->reference);
		return ERROR_INVALID_OBJSTM_DICO;
	}

	start +=2;
	while(start[0] == ' '){
		start ++;
	}

	len = strlen(obj->dico) - (int)(start - obj->dico);
	num = getNumber(start,len);
	if(num <= 0){
		err_log("pdf_extract_objstm :: Incorrect /N entry in object stream %s\n",obj->reference);
		return ERROR_INVALID_OBJSTM_DICO;
	}

	// Get the byte offset of the first compressed object "/First" entry in dico
	if ((start = searchPattern(obj->dico, "/First", 6, strlen(obj->dico))) == NULL){
		err_log("pdf_extract_objstm :: Entry /First not found in Object stream dictionary %s\n", obj->reference);
		return ERROR_INVALID_OBJSTM_DICO;
	}

	start +=6;
	while (start[0] == ' ')
		start++;

	len = strlen(obj->dico) - (int)(start - obj->dico);
	first = getNumber(start, len);
	if (first <= 0){
		err_log("pdf_extract_objstm :: Incorrect /First entry in object stream %s\n", obj->reference);
		return ERROR_INVALID_OBJSTM_DICO;
	}

	start = stream;
	len = stream_size;

	obj_offsets = (int*)calloc(num, sizeof(int));
	if ( obj_offsets == NULL){
		return ERROR_INSUFFICENT_MEMORY;
	}

	// Get objects number and offset
	for(i = 0 ; i< num; i++){

		// check if the offset if out-of-bound
		check = start - stream;
		if (check >= stream_size){
			err_log("pdf_extract_objstm :: bad offset in object stream %s\n", obj->reference);
			free(obj_offsets);
			return ERROR_INVALID_OBJSTM_FORMAT;
		}

		// Get the object number
		obj_num_s = getNumber_s(start, len);
		if (obj_num_s == NULL){
			err_log("pdf_extract_objstm :: Can't extract object from object stream :: obj_ref = %s\n", obj->reference);
			free(obj_offsets);
			return ERROR_INVALID_OBJSTM_FORMAT;
		}

		len -=  strlen(obj_num_s);
		start += strlen(obj_num_s);
		free(obj_num_s);

		// Move ptr for white space
		while(start[0] == ' '){
			start ++ ;
			len--;
		}

		// Get the object offset
		if ((obj_off_s = getNumber_s(start, len)) == NULL){
			err_log("pdf_extract_objstm :: Can't extract object from object stream :: obj_ref = %s\n", obj->reference);
			free(obj_offsets);
			return ERROR_INVALID_OBJSTM_FORMAT;
		}

		obj_off = atoi(obj_off_s);
		obj_offsets[i] = obj_off;

		len -=  strlen(obj_off_s);
		start += strlen(obj_off_s);
		free(obj_off_s);

		// Move ptr for white space
		while(start[0] == ' '){
			start ++ ;
			len--;
		}

		// calc the length of the object according to the offset of the next object.
	}


	start = stream;
	len = stream_size;

	for(i = 0 ; i< num; i++){

		// Get the object number
		obj_num_s = getNumber_s(start, len);
		if (obj_num_s == NULL){
			err_log("pdf_extract_objstm :: Can't extract object from object stream :: obj_ref = %s\n", obj->reference);
			free(obj_offsets);
			return ERROR_INVALID_OBJSTM_FORMAT;
		}

		// Build the object reference
		obj_ref_len = strlen(obj_num_s) + 6;
		obj_ref = (char*)calloc(obj_ref_len+1,sizeof(char));
		obj_ref[obj_ref_len] = '\0';

		sprintf(obj_ref, "%s 0 obj", obj_num_s);

		// move ptr according to the size of the scanned number.
		len -=  strlen(obj_num_s);
		start += strlen(obj_num_s);

		// skip white space
		while(start[0] == ' '){
			start ++ ;
			len--;
		}

		// Get the object offset
		if ((obj_off_s = getNumber_s(start, len)) == NULL){
			err_log("pdf_extract_objstm :: Can't extract object from object stream :: obj_ref = %s\n", obj->reference);
			free(obj_offsets);
			free(obj_num_s);
			return ERROR_INVALID_OBJSTM_FORMAT;
		}

		obj_off = atoi(obj_off_s);
		obj_offsets[i] = obj_off;

		len -=  strlen(obj_off_s);
		start += strlen(obj_off_s);

		// Move ptr for white space
		while(start[0] == ' '){
			start ++ ;
			len--;
		}

		/*
		Hint:
			- offset in stream = off + first
			- real offset = start + off + first.
		*/

		// offset of the object content = stream ptr + ptr of the first obj + offset of the obj.
		obj_data_ptr = stream + first + obj_off;

		// check if the offset if out-of-bound
		check = obj_data_ptr - stream;
		if (check >= stream_size){
			err_log("pdf_extract_objstm :: bad offset in object stream %s\n", obj->reference);
			free(obj_offsets);
			free(obj_num_s);
			free(obj_off_s);
			return ERROR_INVALID_OBJSTM_FORMAT;
		}

		// calc the length of the object according to the offset of the next object.
		if( i != num-1 ){
			obj_size  = (obj_offsets[i+1] - obj_off);
		}else{
			// calc according to the end of the stream
			obj_size =  stream_size - (int)(obj_data_ptr - stream );
		}

		if (obj_size <= 0){
			err_log("pdf_extract_objstm :: bad object length! :: obj_len = %d\n", obj_size);
			free(obj_offsets);
			free(obj_num_s);
			free(obj_off_s);
			return ERROR_INVALID_OBJSTM_FORMAT;
		}

		obj_content = (char*)calloc(obj_size + 1, sizeof(char));
		obj_content[obj_size] = '\0';
		memcpy(obj_content,obj_data_ptr,obj_size);

		comp_obj = init_pdf_object(obj_ref, obj_content, obj_size, 0);
		if(comp_obj == NULL){
			free(obj_offsets);
			free(obj_num_s);
			free(obj_off_s);
			return ERROR_INSUFFICENT_MEMORY;
		}

		// Continue on error.
		retcode = pdf_parse_obj_content(pdf, comp_obj);
		if(retcode != ERROR_SUCCESS && retcode != ERROR_OBJ_DICO_NOT_FOUND){
			warn_log("pdf_extract_objstm :: Parsing obj (%s) content failed with code: 0x%x\n",obj_ref, retcode);
		}

		retcode = add_pdf_object(pdf, comp_obj);
		if(retcode != ERROR_SUCCESS){
			err_log("pdf_extract_objstm :: Adding obj %s failed!\n");
			free(obj_offsets);
			free(obj_num_s);
			free(obj_off_s);
			free_pdf_object(comp_obj);
			return retcode;
		}

		free(obj_off_s);
		free(obj_num_s);

	}

	free(obj_offsets);
}


int pdf_parse_obj_content(struct pdfDocument * pdf, struct pdfObject * obj){

	char * dico;
	int retcode = 0;

	if(obj == NULL || pdf == NULL){
		err_log("Invalid object or pdf structs\n");
		return ERROR_INVALID_PARAMETERS;
	}

	retcode = pdf_parse_object_dico(obj);

	if(retcode == ERROR_OBJ_DICO_NOT_FOUND){
		return ERROR_SUCCESS;
	}

	if(retcode != ERROR_SUCCESS && retcode != ERROR_OBJ_DICO_OBFUSCATION)
		return retcode;

	if(retcode == ERROR_OBJ_DICO_OBFUSCATION){
		pdf->flags |= FLAG_OBFUSCATED_OBJ;
	}

	retcode = pdf_parse_object_type(obj);
	if(retcode != ERROR_SUCCESS)
		return retcode;

	retcode = pdf_parse_object_stream(obj);
	if(retcode != ERROR_SUCCESS)
		return retcode;

	if( obj->type != NULL && strcmp(obj->type,"/ObjStm") == 0 ){

		retcode = pdf_extract_objstm(pdf, obj);
		if(retcode != ERROR_SUCCESS)
			return retcode;
	}

	return ERROR_SUCCESS;
}


int pdf_parse_objects(struct pdfDocument * pdf){

	int retcode = EXIT_SUCCESS;
	int size, obj_size;
	char * obj_ptr;
	char * endobj_ptr;
	char * content;
	char * obj_ref;
	unsigned int offset = 0;
	struct pdfObject * obj;


	if(pdf == NULL || pdf->content == NULL || pdf->size <= 0){
		return ERROR_INVALID_PARAMETERS;
	}

	endobj_ptr = pdf->content;
	size = pdf->size;

	while((obj_ptr = searchPattern(endobj_ptr, "obj", 3,size) ) != NULL){


		// get the object reference
		offset = (int)(obj_ptr - pdf->content);

		obj_ref = get_ref_stepback_from_ptr(obj_ptr, size, offset);
		if(obj_ref == NULL){
			return ERROR_BAD_OBJ_REF_FORMAT;
		}

		// get the object real offset.
		offset -= (strlen(obj_ref) - 3);

		// get the object content.
		size = (int)(pdf->size - (obj_ptr - pdf->content));
		endobj_ptr = searchPattern(obj_ptr,"endobj",6,size);
		if(endobj_ptr == NULL){
			err_log("invalid object (%s): no endobj pattern found!\n", obj_ref);
			free(obj_ref);
			return ERROR_BAD_OBJ_FORMAT;
		}

		endobj_ptr += 6; // skip "endobj" string.

		obj_size = (int)(endobj_ptr - obj_ptr);
		content = (char*)calloc(obj_size+1,sizeof(char));
		if(content == NULL){
			free(obj_ref);
			return ERROR_INSUFFICENT_MEMORY;
		}
		content[obj_size]='\0';
		memcpy(content, obj_ptr,obj_size);

		// create object and add it in pdf struct.
		obj = init_pdf_object(obj_ref, content, obj_size, offset);
		if(obj == NULL){
			free(obj_ref);
			free(content);
			return ERROR_INSUFFICENT_MEMORY;
		}

		// parse object content
		retcode = pdf_parse_obj_content(pdf, obj);
		if(retcode != ERROR_SUCCESS){
			err_log("Parsing obj (%s) content failed with code: %d\n", obj->reference,retcode);
		}

		// add object in doc list.
		retcode = add_pdf_object(pdf, obj);
		if(retcode != ERROR_SUCCESS){
			err_log("adding object %s failed!\n");
			return retcode;
		}

		size = (int)(pdf->size - (endobj_ptr - pdf->content));

	}


	return retcode;
}


int pdf_get_javascript(struct pdfDocument * pdf, struct pdfObject* obj){

	char * js;
	char * js_obj_ref = NULL;
	char * start = NULL;
	int len = 0;
	int retcode = ERROR_SUCCESS;
	struct pdfObject * js_obj = NULL;


	if (pdf == NULL || obj == NULL){
		err_log("get_JavaScript :: invalid parameters\n");
		return ERROR_INVALID_PARAMETERS;
	}

	if( obj->dico == NULL){
		return ERROR_NO_JS_FOUND;
	}

	if ((start = searchPattern(obj->dico, "/JS", 3, strlen(obj->dico))) == NULL){
		return ERROR_NO_JS_FOUND;
	}

	dbg_log("getJavaScript :: JavaScript Entry in dictionary detected in object %s\n", obj->reference);
	//dbg_log("getJavaScript :: dictionary = %s\n", obj->dico);

	start += 3; // 3 => /JS

	// skip space
	while(start[0] == '\n' || start[0] == '\r' || start[0] == ' '){
		start ++;
	}

	len = strlen(obj->dico) - (int)(start - obj->dico);

	// get an indirect reference object containing of js content.
	js_obj_ref = getIndirectRef(start,len);

	// Get javascript
	if(js_obj_ref != NULL){

		js_obj = getPDFObjectByRef(pdf, js_obj_ref);

		if (js_obj == NULL){
			err_log("get_JavaScript :: Object %s not found\n",js_obj_ref);
			pdf->errors++;
			free(js_obj_ref);
			return ERROR_OBJ_REF_NOT_FOUND;
		}

		free(js_obj_ref);

		retcode = obj_decode_stream(js_obj);
		if(retcode != ERROR_SUCCESS){
			err_log("get_JavaScript :: decode obj (%s) stream failed!\n",js_obj->reference);
			return retcode;
		}

		if(js_obj->decoded_stream != NULL){
			js = js_obj->decoded_stream;
			len = js_obj->decoded_stream_size;
		}
		else if(js_obj->stream != NULL){
			js = js_obj->stream;
			len = js_obj->stream_size;
		}else{
			js = NULL;
			len = 0;
			warn_log("getJavaScript :: Empty js content in object %s\n", obj->reference);
		}

		pdf->flags |= FLAG_ACTIVE_CONTENTS;
		retcode = add_pdf_active_content(pdf,AC_JAVASCRIPT,obj->reference, js, len);
		if(retcode != ERROR_SUCCESS){
			err_log("get_JavaScript :: Add active content failed!\n");
		}

	}else{

		// get js content in dictionary string.
		js = getDelimitedStringContent(start,"(",")",len);

		if (js != NULL){

			dbg_log("getJavaScript :: Found JS content in object %s\n", obj->reference);

			pdf->flags |= FLAG_ACTIVE_CONTENTS;
			retcode = add_pdf_active_content(pdf,AC_JAVASCRIPT,obj->reference, js, strlen(js));
			if(retcode != ERROR_SUCCESS){
				err_log("get_JavaScript :: Add active content failed!\n");
			}
			free(js);

		}
		else{
			warn_log("getJavaScript :: Empty js content in object %s\n", obj->reference);
		}
	
	}


	return retcode;
}


/*
get_js_from_data() :: Get JavaScript content in XFA form description (xml).
TODO :: getJSContentInXFA :: Check the keyword javascript in script tag
*/
int get_js_from_data(char * data, int data_size, struct pdfObject * obj, struct pdfDocument * pdf){
	
	
	int len_tmp = 0;
	int js_size = 0;
	int retcode= ERROR_SUCCESS;
	char * start = NULL;
	char * end = NULL;
	char * js_content = NULL;
	char * tmp = NULL;


	if (data == NULL || data_size <= 0 || obj == NULL || pdf == NULL){
		err_log("get_js_from_data :: invalid parameter\n");
		return ERROR_INVALID_PARAMETERS;
	}

	end = data;
	len_tmp = data_size;

	while((start = searchPattern(end,"<script",7,len_tmp)) != NULL && len_tmp > 0){

		dbg_log("get_js_from_data :: javascript content found in %s\n",obj->reference);

		tmp = start; // save the script start ptr
		end = start+7;
		len_tmp = data_size - (int)(end-data);

		// case: <script....>
		// skip white space
		while(end[0] != '>' && len_tmp>0 ){
			end ++;
			len_tmp --;
		}

		// search the </script> balise
		start = searchPattern(end,"</script",8,len_tmp);
		if(start == NULL){
			dbg_log("get_js_from_data :: End of js script balise not found... continue\n");
			continue;
		}

		end = start+8;
		len_tmp = data_size - (int)(end-data);
		while(end[0] != '>' && len_tmp>0 ){
			end ++;		
			len_tmp --;
		}

		js_size = (int)(end - tmp)+1;
		js_content = (char*)calloc(js_size+1, sizeof(char));
		js_content[js_size]='\0';
		memcpy(js_content,tmp,js_size);

		//dbg_log("get_js_from_data :: js_content = %s\n",js_content);

		retcode = add_pdf_active_content(pdf,AC_JAVASCRIPT,obj->reference, js_content, js_size);
		if(retcode != ERROR_SUCCESS){
			err_log("get_JavaScript :: Add active content failed!\n");
			return retcode;
		}

		free(js_content);

	}

	return retcode;
}


int pdf_get_xfa(struct pdfDocument * pdf, struct pdfObject* obj){

	char * xfa = NULL;
	char * xfa_obj_ref = NULL;
	char * start = NULL;
	char * end = NULL;
	char * tmp = NULL;
	char * obj_list = NULL;	
	int len = 0;
	int len2 = 0;
	int ret = 0;
	int retcode = ERROR_SUCCESS;
	struct pdfObject * xfa_obj = NULL;

	if (pdf == NULL || obj == NULL){
		err_log("get_xfa :: invalid parameters\n");
		return ERROR_INVALID_PARAMETERS;
	}

	if( obj->dico == NULL ){
		return ERROR_NO_XFA_FOUND;
	}

	start = searchPattern(obj->dico, "/XFA" , 4 , strlen(obj->dico));
	if(start == NULL){
		return ERROR_NO_XFA_FOUND;
	}

	dbg_log("get_xfa :: XFA Entry in dictionary detected in object %s\n", obj->reference);

	start += 4;

	// skip white space
	while(start[0] == ' ' || start[0]=='\n' || start[0]=='\r'){
		start ++;
	}

	len = strlen(obj->dico) - (int)(start - obj->dico);

	// If its a list get the content
	if(start[0] == '['){

		obj_list =  getDelimitedStringContent(start,"[", "]", len);
		if(obj_list == NULL){
			err_log("get_xfa :: Can't get object list in dictionary\n");
			return ERROR_INVALID_DICO;
		}
		
		end = obj_list;
		len2 = strlen(obj_list);

		// get XFA object reference in array ::
		while( (xfa_obj_ref = getIndirectRefInString(end, len2)) ){

			//dbg_log("get_xfa :: xfa_obj_ref = %s\n",xfa_obj_ref);

			end = searchPattern(end, xfa_obj_ref , strlen(xfa_obj_ref)-4 , len2);
			if(end == NULL){
				err_log("get_xfa :: unexpected error !!\n");
				free(obj_list);
				return ERROR_NO_XFA_FOUND;
			}

			end += strlen(xfa_obj_ref)-2;
			len2 = strlen(obj_list) - (int)(end - obj_list);


			// get xfa object
			xfa_obj =  getPDFObjectByRef(pdf, xfa_obj_ref);
			if(xfa_obj == NULL){
				err_log("get_xfa :: Object %s containing xfa not found\n",xfa_obj_ref);
				free(xfa_obj_ref);
				continue;
			}

			free(xfa_obj_ref);

			retcode = obj_decode_stream(xfa_obj);
			if(retcode != ERROR_SUCCESS && retcode != ERROR_NO_STREAM_FILTERS){
				err_log("get_xfa :: decode obj (%s) stream failed!\n",xfa_obj->reference);
				continue;
			}

			if(xfa_obj->decoded_stream != NULL){
				xfa = xfa_obj->decoded_stream;
				len = xfa_obj->decoded_stream_size;
			}
			else if(xfa_obj->stream != NULL){
				xfa = xfa_obj->stream;
				len = xfa_obj->stream_size;
			}else{
				xfa = NULL;
				len = 0;
				warn_log("get_xfa :: Empty xfa content in object %s\n", obj->reference);
			}

			pdf->flags |= FLAG_ACTIVE_CONTENTS;
			retcode = get_js_from_data(xfa, len, xfa_obj, pdf);
			if(retcode != ERROR_SUCCESS){
				err_log("get_xfa :: Get javascript from xfa content failed!\n");
				free(obj_list);
				return retcode;
			}

			retcode = add_pdf_active_content(pdf,AC_XFA,obj->reference, xfa, len);
			if(retcode != ERROR_SUCCESS){
				err_log("get_JavaScript :: Add active content failed!\n");
				return retcode;
			}

		}

		free(obj_list);

	}else{

		len2 = strlen(obj->dico) -(int)(start - obj->dico);

		xfa_obj_ref = getIndirectRefInString(start, len2);
		if(xfa_obj_ref == NULL){
			err_log("get_xfa :: get xfa object indirect reference failed\n");
			return ERROR_INVALID_DICO;
		}

		// get xfa object 
		xfa_obj =  getPDFObjectByRef(pdf, xfa_obj_ref);
		if(xfa_obj == NULL){
			err_log("get_xfa :: Object %s containing xfa not found\n",xfa_obj_ref);
			free(xfa_obj_ref);
			return ERROR_OBJ_REF_NOT_FOUND;
		}

		free(xfa_obj_ref);

		retcode = obj_decode_stream(xfa_obj);
		if(retcode == ERROR_SUCCESS || retcode == ERROR_NO_STREAM_FILTERS){

			if(xfa_obj->decoded_stream != NULL){
				xfa = xfa_obj->decoded_stream;
				len = xfa_obj->decoded_stream_size;
			}
			else if(xfa_obj->stream != NULL){
				xfa = xfa_obj->stream;
				len = xfa_obj->stream_size;
			}else{
				xfa = NULL;
				len = 0;
				warn_log("get_xfa :: Empty xfa content in object %s\n", obj->reference);
			}

			pdf->flags |= FLAG_ACTIVE_CONTENTS;
			retcode = get_js_from_data(xfa, len, xfa_obj, pdf);
			if(retcode != ERROR_SUCCESS){
				err_log("get_xfa :: Get javascript from xfa content failed!\n");
				return retcode;
			}
			
		}else{

			err_log("get_xfa :: decode obj (%s) stream failed!\n",xfa_obj->reference);
			// TODO: treat error code.
		}
	}
	
	return ERROR_SUCCESS;
}


/*
pdf_get_embedded_file() ::  Get Embedded file content
*/
int pdf_get_embedded_file(struct pdfDocument * pdf , struct pdfObject* obj){

	char * ef = NULL;
	int ef_size = 0;
	int retcode = ERROR_SUCCESS;

	//dbg_log("getEmbeddedFile :: Analysing object :: %s\n",obj->reference);
	if (pdf == NULL || obj == NULL){
		err_log("get_embedded_file :: invalid parameter\n");
		return ERROR_INVALID_PARAMETERS;
	}
	
	if(obj->dico == NULL || obj->type == NULL){
		return ERROR_NO_EF_FOUND;
	}

	// Get by Type or by Filespec (EF entry)
	/* TOIMPROVE: Type value is optional (see pdf reference 1.7 table 3.42 p.185) */
	if( strncmp(obj->type,"/EmbeddedFile",13) == 0){

		// decode embedded file stream
		retcode = obj_decode_stream(obj);
		if(retcode != ERROR_SUCCESS && retcode != ERROR_NO_STREAM_FILTERS){
			err_log("get_embedded_file :: decode obj (%s) stream failed!\n",obj->reference);
			return retcode;
		}

		if(obj->decoded_stream != NULL ){
			ef = obj->decoded_stream;
			ef_size = obj->decoded_stream_size;
		}else{
			ef = obj->stream;
			ef_size = obj->stream_size;
		}

		pdf->flags |= FLAG_ACTIVE_CONTENTS;
		retcode = add_pdf_active_content(pdf,AC_EMBEDDED_FILE,obj->reference, ef, ef_size);
		if(retcode != ERROR_SUCCESS){
			err_log("get_embedded_file :: Add active content failed!\n");
			return retcode;
		}
	}

	return ERROR_SUCCESS;
}


/*
getEmbeddedFile() ::  Get the URI defined in the object
*/
int pdf_get_uri(struct pdfDocument * pdf, struct pdfObject * obj){


	char * start;
	char * end;
	char * uri;
	int len = 0;
	int retcode = ERROR_SUCCESS;

	if(obj == NULL || pdf == NULL){
		err_log("get_uri :: invalid parameter\n");
		return ERROR_INVALID_PARAMETERS;
	}

	if(obj->dico == NULL){
		return ERROR_NO_URI_FOUND;
	}

	// get the URI entry in the dico
	end= obj->dico;
	len = strlen(obj->dico);

	while( (start = searchPattern(end,"/URI",4,len)) != NULL ){

		start += 4;

		// skip white spaces
		while(start[0] == ' ' || start[0] == '\n' || start[0] == '\r' ){
			start ++;
		}

		end = start;

		if(start[0] != '('){
			continue;
		}

		len = strlen(obj->dico) -(int)(start - obj->dico);
		uri = getDelimitedStringContent(start,"(",")", len);

		if (uri != NULL) {

			retcode = add_pdf_active_content(pdf,AC_URI,obj->reference, uri, strlen(uri));
			if(retcode != ERROR_SUCCESS){
				err_log("get_uri :: Add active content failed!\n");
				free(uri);
				return retcode;
			}

			free(uri);
		}
	}

	return ERROR_SUCCESS;
}


/*TODO :: getActions :: get other potentially dangerous actions (OpenActions - GoToE - GoToR - etc.)*/
int pdf_get_actions(struct pdfDocument * pdf, struct pdfObject * obj){

	char * start = NULL;

	if(pdf == NULL || obj == NULL){
		err_log("get_actions :: invalid parameters!\n");
		return ERROR_INVALID_PARAMETERS;
	}

	if(obj->dico == NULL){
		return ERROR_SUCCESS;
	}

	// get Launch actions
	start = searchPattern(obj->dico,"/Launch",7,strlen(obj->dico));
	if(start != NULL){
		warn_log("get_actions :: Found /Launch action in object %s\n",obj->reference);
		pdf->flags |= FLAG_DANG_KEY_HIGH;
		return ERROR_SUCCESS;
	}

	return ERROR_SUCCESS;
}


int pdf_get_active_contents(struct pdfDocument * pdf){

	int retcode = ERROR_SUCCESS;
	struct pdfObject * obj;

	if(pdf == NULL || pdf->objects == NULL){
		err_log("get_active_contents :: invalid parameter\n");
		retcode = ERROR_INVALID_PARAMETERS;
		return retcode;
	}

	obj = pdf->objects;

	while(obj != NULL){

		retcode = pdf_get_javascript(pdf, obj);
		if(retcode != EXIT_SUCCESS && retcode != ERROR_NO_JS_FOUND){
			err_log("get_active_contents :: get javascript in obj (%s) failed with error: %02x\n",obj->reference, retcode);
			// TODO: treat errors.
		}

		retcode = pdf_get_xfa(pdf, obj);
		if(retcode != EXIT_SUCCESS && retcode != ERROR_NO_XFA_FOUND){
			err_log("get_active_contents :: get xfa in obj (%s) failed with error: %02x\n",obj->reference, retcode);
			// TODO: treat errors.
		}

		retcode = pdf_get_embedded_file(pdf, obj);
		if(retcode != EXIT_SUCCESS && retcode != ERROR_NO_EF_FOUND){
			err_log("get_active_contents :: get embedded file in obj (%s) failed with error: %02x\n",obj->reference, retcode);
			// TODO: treat errors.
		}

		retcode = pdf_get_uri(pdf, obj);
		if(retcode != EXIT_SUCCESS && retcode != ERROR_NO_URI_FOUND){
			err_log("get_active_contents :: get uri in obj (%s) failed with error: %02x\n",obj->reference, retcode);
			// TODO: treat errors.
		}

		retcode = pdf_get_actions(pdf, obj);
		if(retcode != EXIT_SUCCESS && retcode != ERROR_NO_ACTION_FOUND){
			err_log("get_active_contents :: get actions in obj (%s) failed with error: %02x\n",obj->reference, retcode);
			// TODO: treat errors.
		}

		obj = obj->next;
	}

	//print_actives_contents(pdf);

	return ERROR_SUCCESS;
}


int pdf_parse(struct pdfDocument * pdf){

	int retcode = ERROR_SUCCESS;

	if(pdf == NULL){
		err_log("pdf_parse :: invalid parameter\n");
		retcode = ERROR_ON_PARSING | ERROR_INVALID_PARAMETERS;
		return retcode;
	}

	retcode = pdf_get_content(pdf);
	if(retcode != ERROR_SUCCESS){
		retcode |= ERROR_ON_PARSING;
		return retcode;
	}

	retcode = pdf_get_trailers(pdf);
	if(retcode != ERROR_SUCCESS){
		retcode |= ERROR_ON_TRAILER_PARSING;
		return retcode;
	}

	retcode = pdf_parse_objects(pdf);
	if(retcode != ERROR_SUCCESS){
		retcode |= ERROR_ON_OBJ_PARSING;
		return retcode;
	}

	retcode = pdf_check_document_struct(pdf);
	if(retcode != ERROR_SUCCESS){
		retcode |= ERROR_ON_STRUCT_ANALYSIS;
		return retcode;
	}

	retcode = pdf_get_active_contents(pdf);
	if(retcode != ERROR_SUCCESS){
		retcode |= ERROR_ON_OBJ_PARSING;
		return retcode;
	}
	//printPDFObjects(pdf);
	return retcode;
}
