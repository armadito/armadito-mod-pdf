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


#include "pdfParsing.h"
#include "utils.h"
#include "osdeps.h"
#include "log.h"
#include "filters.h"



/*
checkMagicNumber() :: check the presence of a PDF header and fill the pdf struct version field.
parameters:
- struct pdfDocument * pdf (pdf document pointer)
returns: (int)
- 0 on success.
- an error code (<0) on error.

// TODO :: checkMagicNumber :: search the header in the 1024 first bytes.
// TODO :: checkMagicNumber :: Thread XDP files.
*/
int checkMagicNumber(struct pdfDocument * pdf){
	
	int version_size = 8;
	int ret = 0;
	char * version = NULL;

	
	if (pdf->fh == NULL && pdf->fd < 0) {
		err_log("checkMagicNumber :: invalid parameters!\n");
		return -1;
	}

	version = (char*)calloc(version_size+1, sizeof(char));
	version[version_size] = '\0';
	
	if (pdf->fh != NULL) {

		// Go to the beginning of the file
		fseek(pdf->fh,0,SEEK_SET);

		// Read the 8 first bytes Ex: %PDF-1.x
		ret = fread(version,1,version_size,pdf->fh);
		if (ret != version_size){
			err_log("checkMagicNumber :: read file failed!\n");
			free(version);
			return -1;
		}

	}
	else {

		os_lseek(pdf->fd,0,SEEK_SET);
		os_read(pdf->fd, version, version_size);
	}

	dbg_log("checkMagicNumber :: pdf header = %s\n",version);
	
	// PDF version from 1.1 to 1.7
	if( strncmp(version,"%PDF-1.",7) == 0 && version[7] >= '1' && version[7] <= '7' ){
	
		pdf->version = version;
	
	}else{
		
		free(version);
		pdf->testStruct->bad_header = 1;
		return -2;
	}
	
	return 0;
	
}


/*
getPDFContent() :: Get the content of the PDF document
parameters:
- struct pdfDocument * pdf (pdf document pointer)
returns: (int)
- the size of the document on success.
- an error code (<=0) on error.

// TODO :: getPDFContent :: set max_size limit.
*/
int getPDFContent(struct pdfDocument * pdf){

	char * content = NULL;
	int doc_size = 0;
	int read_bytes = 0;


	if (pdf->fh == NULL && pdf->fd < 0) {
		err_log("getPDFContent :: invalid parameters!\n");
		return -1;
	}

	if (pdf->fh != NULL) {

		// Get the size in byte
		fseek(pdf->fh,0,SEEK_END);
		doc_size = ftell(pdf->fh);
		fseek(pdf->fh,0,SEEK_SET); // rewind

	}
	else {

		doc_size =  os_lseek(pdf->fd,0,SEEK_END);		
		//doc_size = _tell(pdf->fd);
		os_lseek(pdf->fd,0,SEEK_SET); // rewind		

	}
	
	dbg_log("getPDFContent :: Document Size  = %d\n",doc_size);
	
	if ((content = (char*)calloc(doc_size + 1, sizeof(char))) == NULL) {
		err_log("getPDFContent :: content allocation failed!\n");
		return -1;
	}	
	content[doc_size]= '\0';
	
	
	if (pdf->fh != NULL) {
		read_bytes = fread(content, 1, doc_size, pdf->fh);
	}
	else {		
		read_bytes = os_read(pdf->fd, content, doc_size);
	}

	if (read_bytes != doc_size){
		warn_log("getPDFContent :: read_byte (%d)  != doc_size (%d)\n",read_bytes,doc_size);
	}
	
	pdf->content = content;	
	pdf->size = read_bytes;
	
	return read_bytes;
	
}


/*
hexaObfuscationDecode() :: Decode a dictionnary obfuscated with hexa; return the modified dico or the original if there is no obfuscation.
parameters:
- char * dico (object dictionnary).
returns: (char*)
- the modified dictionary.
- NULL if there is no hexa obfuscation or on error.
*/
char * hexaObfuscationDecode(char * dico){

	char * start = NULL;
	char * decoded_dico = NULL;
	char * hexa = NULL;
	char * hexa_decoded = NULL;
	char * tmp = NULL;
	int len = 0, start_len = 0;	
	int is_space_hexa = 1;
	

	if (dico == NULL) {
		err_log("hexaObfuscationDecode :: invalid parameter\n");
		return NULL;
	}
	
	// TODO :: len= obj->dico_len;
	len = strlen(dico);
	
	start = searchPattern(dico,"#",1,len);
	if(start == NULL){
		return NULL;
	}

	start_len = (int)(start - dico);
	start_len = len - start_len;
		
	hexa = (char*)calloc(4,sizeof(char));
	hexa[3] = '\0';
	
	hexa_decoded = (char*)calloc(2,sizeof(char));
	hexa_decoded[1] = '\0';

	// temporary decoded dictionary
	tmp = (char*)calloc(len+1,sizeof(char));
	tmp[len]= '\0';
	memcpy(tmp,dico,len);
	

	while( start != NULL && start_len >= 3){
		
		// get the pointer of the hexa code
		start = getHexa(start,start_len);		
		if(start == NULL){
			continue;
		}
					
		memcpy(hexa, start, 3);

		// #20 = space - #2F = '/' (slash) - #E9 = é - #2C = ','
		if(memcmp(hexa,"#20",3) != 0 && memcmp(hexa,"#2F",3) != 0 && memcmp(hexa,"#E9",3) != 0 && memcmp(hexa,"#2C",3) != 0){
			is_space_hexa = 0;
		}
				
		os_sscanf(hexa,"%x",&hexa_decoded[0]);			

		decoded_dico = replaceInString(tmp,hexa,hexa_decoded);
		if(decoded_dico != NULL){

			free(tmp);
			tmp = decoded_dico;

		}else{			
			err_log("hexaObfuscationDecode :: replaceInString returns NULL \n");
			free(hexa);
			free(hexa_decoded);
			free(tmp);
			return NULL;
		}


		start += 3;		

		start_len = (int)(start - dico);
		start_len = strlen(dico) -start_len;

	}
	
	free(hexa);
	free(hexa_decoded);


	if(decoded_dico != NULL && is_space_hexa == 0){		
		dbg_log("hexaObfuscationDecode :: decoded_dico  = %s\n",decoded_dico);
		return decoded_dico;
	}

	// if not returned, then free decoded_dico.
	free(tmp);

	return NULL ; 

}


/*
getObjectDictionary() :: Get the object dictionary
parameters:
- struct pdfObject * obj (pdf object pointer)
- struct pdfDocument * pdf (pdf document pointer)
returns: (char*)
- a pointer to the dictionary string on success.
- NULL on error.
*/
char * getObjectDictionary(struct pdfObject * obj, struct pdfDocument * pdf){
	
	char  * dico = NULL;
	char * decoded_dico = NULL;
	char * content;
	char * dico_start = NULL;
	char * end = NULL;
	int inQuotes = 0;
	int inString = 0;
	int sub = 0;
	int flag = 0;
	int len = 0;

	if (obj == NULL || pdf == NULL){
		err_log("getObjectDictionary :: invalid parameter\n");
		return NULL;
	}

	content = obj->content;
	
	// Search the beginning of the
	dico_start = searchPattern(content,"<<",2,obj->content_size);

	if(dico_start == NULL){
		//dbg_log("getObjectDictionnary :: No dictionary found in object %s!!\n", obj->reference);
		return NULL;
	}

	// search other occurencies of "<<" to detect sub dictionaries
	// check if you find the same number of occurencies of stream ">>"
	
	len = (int)(dico_start - obj->content);
	len = obj->content_size - len;

	end = dico_start;

	while(len >= 2 && flag == 0){


		// String delimiter
		if(inQuotes == 0 && end[0] == '(' && ((end == dico_start) || (end > dico_start && (end-1)[0] != '\\')) ){
			inString ++;
			len--;
			end ++;
			continue;
		}

		// String delimiter 2
		if(inQuotes == 0 && inString > 0 && end[0] == ')' && ((end == dico_start) || (end > dico_start && (end-1)[0] != '\\')) ){
			inString --;
			len --;
			end++;
			continue;
		}

		// Quotes delimiter
		if(end[0] == '"' && ((end == dico_start) || (end > dico_start && (end-1)[0] != '\\')) ){
			inQuotes = (inQuotes == 0)?1:0;
			len --;
			end++;
			continue;
		}

		// Dico delimiter
		if(inString == 0 && end[0] == '<' && end[1] == '<' && ((end == dico_start) || (end > dico_start && (end-1)[0] != '\\')) ){
			sub ++;
			end+=2;
			len -=2;
			continue;
		}

		// Dico delimiter
		if(inString == 0 && end[0] == '>' && end[1] == '>' && ((end == dico_start) || (end > dico_start && (end-1)[0] != '\\')) ){
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


	// No dico found
	if(flag == 0){
		return NULL;
	}

	len = (int)(end - dico_start);

	dico = (char*)calloc(len+1,sizeof(char));
	dico[len]='\0';

	memcpy(dico,dico_start,len);
	

	// decode hexa obfuscated dictionaries
	decoded_dico = hexaObfuscationDecode(dico);

	if(decoded_dico != NULL){		
		warn_log("getObjectDictionary :: Obfuscated Object dictionary in object %s\n", obj->reference);
		free(dico);
		pdf->testStruct->obfuscated_object ++;
		return decoded_dico;
	}


	return dico;
	
}


/*
getObjectType() :: Get the object type defined in dictionary "/Type"
parameters:
- struct pdfObject * obj (pdf object pointer)
returns: (char *)
- the type of the object if any.
- NULL if not or on error.
*/
char * getObjectType(struct pdfObject * obj){

	char * type = NULL;
	char * start = NULL;
	char * end = NULL;
	char * pattern = NULL;
	int pattern_len = 5; // "/Type"
	int len = 0;
	int flag  = 0;
	int dico_len = 0;
	int sub = 0;
	int type_len =0;


	if (obj == NULL || obj->dico == NULL){
		err_log("getObjectType :: invalid parameter\n");
		return NULL;
	}

	pattern = (char*)calloc(pattern_len+1,sizeof(char));


	dico_len = strlen(obj->dico);
	len = dico_len;
	start = obj->dico;

	// skip first dico delimiter
	while(start[0] == '<'){
		start ++;
	}


	// Search /Type keyword (but outbound of a sub dictionary)
	while(len >= pattern_len && flag == 0){

		if(start[0] == '<' && start[1] == '<'){
			sub ++;
			start += 2;
			len = (int)(start - obj->dico);
			len = dico_len -len;			
			continue;
		}

		if(start[0] == '>' && start[1] == '>'){
			sub --;
			start += 2;
			len = (int)(start - obj->dico);
			len = dico_len -len;
			continue;
		}

		memcpy(pattern,start,5);

		if(sub == 0 && memcmp(pattern,"/Type",5) == 0 && start[5] != '1' && start[5] != '2' ){
			//dbg_log("getObjectType :: Found type delimiter :: start[5] = %c\n",start[5]);
			flag ++;		
			continue;
		}

		start++;

		len = (int)(start - obj->dico);
		len = dico_len -len;

	}

	free(pattern);


	// If no type found
	if(flag == 0){
		return NULL;
	}

	// skip /Type string
	start += 5;

	// White space
	while(start[0] == ' ' ){
		start ++;
	}

	end = start+1;
	
	type_len = 0;

	while( (end[0] >=97 && end[0] <=122) || (end[0] >=65 && end[0] <=90 ) || (end[0] >=48 && end[0] <= 57) ){ // Lower case [97 122] or Upper case [65 90] + digit [48 57]
		end ++;
		type_len ++;
	}

	if(type_len == 0){
		return NULL;
	}

	type_len ++; // add it for '/'
	type = (char*)calloc(type_len+1,sizeof(char));
	type[type_len]= '\0';

	memcpy(type,start,type_len);
	
	return type;
}


/*
getObjectType() :: Get the stream content of an object (returns )
parameters:
- struct pdfObject * obj (pdf object pointer)
returns: (char *)
- the stream content.
- NULL if there is no stream or on error.
*/
char * getObjectStream(struct pdfObject * obj){

	char * stream = NULL;
	char * start = NULL;
	char * end = NULL;
	int len = 0;

	if (obj == NULL){
		err_log("getObjectStream :: invalid parameter\n");
		return NULL;
	}

	start = searchPattern(obj->content,"stream",6,obj->content_size);
	if(start == NULL){
		return NULL;
	}
		
	len = (int)(start - obj->content);
	len = obj->content_size -len;

	
	end = searchPattern(start,"endstream",9,len);
	
	if(end == NULL){
		return NULL;
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
		warn_log("getObjectStream :: Empty stream content in object %s\n", obj->reference);		
		return NULL;
	}

	obj->stream_size = len; // -1 for the white space
	obj->tmp_stream_size = obj->stream_size;

	stream = (char*)calloc(len+1,sizeof(char));
	stream[len]='\0';
	
	memcpy(stream,start,len);

	return stream;

}


/*
getStreamFilters() :: Get stream's filters
parameters:
- struct pdfObject * obj (pdf object pointer)
returns: (char *)
- the filters applied to the stream in on success.
- NULL on error.
*/
char * getStreamFilters(struct pdfObject * obj){

	char * start = NULL;
	char * end = NULL;
	char * end_tmp = NULL;
	int len = 0;
	char * filters = NULL;
	

	if (obj == NULL || obj->dico == NULL){
		err_log("getStreamFilters :: invalid parameter\n");
		return NULL;
	}

	
	start = searchPattern(obj->dico,"/Filter",7,strlen(obj->dico));		
	if( start == NULL ){
		// No filter found.
		return NULL;
	}
	
	len = (int)(start - obj->dico);
	start += 1;
	
	end = memchr(start,'/',strlen(obj->dico)-len);
	end_tmp = memchr(start,'[',strlen(obj->dico)-len);
	
	if(end == NULL){
		return NULL;
	}
	
	// if a bracket is before the first / that means it's an array of filters
	if( end_tmp != NULL && end_tmp < end ){

		len = strlen(obj->dico) - ((int)(end_tmp - obj->dico));

		filters = getDelimitedStringContent(end_tmp,"[","]",len);
		if (filters == NULL){
			// malformed dictionary.
			obj->errors ++;
			err_log("getStreamFilters :: malformed dictionary in object %s!\n",obj->reference);
			return NULL;
		}
				

	}else{ // a single filter
	
		start = end;
		len = 0;
		do{
			
			end ++;
			len++;
		
		}while( (end[0] >=97 && end[0] <=122) || (end[0] >=65 && end[0] <=90 ) || (end[0] >=48 && end[0] <= 57) ); // Lower case [97 122] or Upper case [65 90] + digit [48 57]
		
		filters = (char*)calloc(len + 1, sizeof(char));
		filters[len] = '\0';

		os_strncpy(filters, len + 1, start, len);
	
	}
	
	
	
	/*debuf print*/
	//dbg_log("getStreamFilters :: filters = %s \n",filters);
	
	
	return filters;

}


/*
decodeObjectStream() :: decode an object stream according to the filters applied
parameters:
- struct pdfObject * obj (pdf object pointer)
returns: (int)
- 0 on success.
- an error code (<0) on error.
// TODO :: decodeObjectStream :: check if the stream is encrypted. (/Encrypt in the dico)
*/
int decodeObjectStream(struct pdfObject * obj){

	
	char * start = NULL;
	char * end = NULL;
	char * filter =NULL; 
	char * stream = NULL;
	char * tmp = NULL;
	int len = 0;
	unsigned int i =0;
	int filter_applied = 0;
	
	if (obj == NULL){
		err_log("decodeObjectStream :: invalid parameter\n");
		return -1;
	}
	
	if(obj->filters == NULL){		
		err_log("decodeObjectStream :: There is no stream filter in object %s\n", obj->reference);
		return -1;
	}
	
	dbg_log("decodeObjectStream :: obj= %s :: implemented filters = %s\n", obj->reference,obj->filters);

	if(obj->stream == NULL){		
		err_log("decodeObjectStream :: NULL stream in object %s\n", obj->reference);
		return -1;
	}


	end = obj->filters;

	stream = (char*)calloc(obj->stream_size+1,sizeof(char));
	stream[obj->stream_size]='\0';
	memcpy(stream,obj->stream,obj->stream_size);


	//while( (start = memchr(end, '/',strlen(obj->filters)-len)) != NULL ){
	while( (start = strchr(end, '/')) != NULL ){
				
		i = 0;
		end = start;		
		// Scan the filter name
		do{
			end ++;
			i++;
		}while( i < strlen(obj->filters) && ((end[0] >=97 && end[0] <=122) || (end[0] >=65 && end[0] <=90 ) || (end[0] >=48 && end[0] <= 57)));
		
		len = (int)(end-start);
				
		filter = (char*)calloc(len+1,sizeof(char));
		filter[len] = '\0';

		os_strncpy(filter,len+1,start,len);
		//dbg_log("decodeObjectStream :: implemented filter_end = %s\n",filter);
		

		// Apply decode filter
		if((strncmp(filter,"/FlateDecode",12) == 0 && strncmp(filter,"/FlateDecode",strlen(filter)) == 0) || (strncmp(filter,"/Fl",3) == 0 && strncmp(filter,"/Fl",strlen(filter)) == 0)){
			

			dbg_log("decodeObjectStream :: Decode Fladetecode :: %s\n",obj->reference);

			if ((tmp = FlateDecode(stream, obj)) == NULL){
				err_log("decodeObjectStream :: FlateDecode failed!\n");
				dbg_log("decodeObjectStream :: dico = %s\n",obj->dico);
				free(stream);
				free(filter);
				obj->errors++;
				return -1;
			}

			free(stream);
			stream  = tmp;
			filter_applied ++;

		}else if((strncmp(filter,"/ASCIIHexDecode",15) == 0 && strncmp(filter,"/ASCIIHexDecode",strlen(filter)) == 0) || (strncmp(filter,"/AHx",4) == 0 && strncmp(filter,"/AHx",strlen(filter)) == 0)){


			dbg_log("decodeObjectStream :: Decode ASCIIHexDecode :: %s\n",obj->reference);
								
			if ((tmp = ASCIIHexDecode(stream, obj)) == NULL){
				err_log("decodeObjectStream :: ASCIIHexDecode failed!\n");
				free(stream);
				free(filter);
				obj->errors++;
				return -1;
			}

			free(stream);
			stream  = tmp;
			filter_applied ++;
		
		}else if ((strncmp(filter, "/ASCII85Decode", 14) == 0 && strncmp(filter, "/ASCII85Decode", strlen(filter)) == 0) || (strncmp(filter, "/A85", 4) == 0 && strncmp(filter, "/A85", strlen(filter)) == 0)){


			dbg_log("decodeObjectStream :: Decode ASCII85Decode :: %s \n",obj->reference);
										
			if ((tmp = ASCII85Decode(stream, obj)) == NULL){
				err_log("decodeObjectStream :: ASCII85Decode failed!\n");
				free(stream);
				free(filter);
				obj->errors++;
				return -1;
			}

			free(stream);
			stream  = tmp;
			filter_applied ++;



		}else if ((strncmp(filter, "/LZWDecode", 10) == 0 && strncmp(filter, "/LZWDecode", strlen(filter)) == 0) || (strncmp(filter, "/LZW", 4) == 0 && strncmp(filter, "/LZW", strlen(filter)) == 0)){

#if 0
			dbg_log("decodeObjectStream :: Decode LZWDecode :: %s \n", obj->reference);

			if ((tmp = LZWDecode(stream, obj)) == NULL){
				err_log("decodeObjectStream :: LZWDecode failed!\n");
				free(stream);
				free(filter);
				obj->errors++;
				return -1;
			}

			free(stream);
			stream  = tmp;
			filter_applied ++;
#else
			// to fix.
			warn_log("decodeObjectStream :: Filter LZWDecode not implemented (to fix) :: %s\n", obj->reference);
			free(stream);
			free(filter);
			obj->errors++;
			return -1;
#endif



		}else if ((strncmp(filter, "/RunLengthDecode", 16) == 0 && strncmp(filter, "/RunLengthDecode", strlen(filter)) == 0) || (strncmp(filter, "/RL", 3) == 0 && strncmp(filter, "/RL", strlen(filter)) == 0)){

			// to implement.
			warn_log("decodeObjectStream :: Filter RunLengthDecode not implemented :: %s\n",obj->reference);
			free(stream);
			free(filter);
			obj->errors++;
			return -1;
		

		}else if ((strncmp(filter, "/CCITTFaxDecode", 15) == 0 && strncmp(filter, "/CCITTFaxDecode", strlen(filter)) == 0) || (strncmp(filter, "/CCF", 4) == 0 && strncmp(filter, "/CCF", strlen(filter)) == 0)){


			dbg_log("decodeObjectStream :: Decode CCITTFaxDecode :: %s \n", obj->reference);

			if ((tmp = CCITTFaxDecode(stream, obj)) == NULL){
				err_log("decodeObjectStream :: CCITTFaxDecode failed!\n");
				free(stream);
				free(filter);
				obj->errors++;
				return -1;
			}
						
			free(stream);
			stream  = tmp;
			filter_applied ++;
								
		}else{
						
			warn_log("decodeObjectStream :: Filter %s in object %s not implemented :: %s\n",filter,obj->reference);
			free(stream);
			free(filter);
			obj->errors++;
			return -1;
					
		}
		
		free(filter);
		
		
	}
	
	
	// Store the decoded stream
	if(stream != NULL && filter_applied > 0){
		obj->decoded_stream = stream ;
	}
	else if (stream != NULL) {
		
		free(stream);
		
	}
	
	return 0;
}


/*
getObjectInfos() :: Get object information (types, filters, streams, etc.)
parameters:
- struct pdfObject * obj (pdf object pointer)
- struct pdfDocument * pdf (pdf document pointer)
returns: (int)
- 0 if there is no dictionary and 1 on success.
- an error code (<0) on error.
*/
int getObjectInfos(struct pdfObject * obj, struct pdfDocument * pdf){

	char * dico = NULL;
	char * type = NULL;
	char * stream = NULL;
	char * filters = NULL;	
	

	if(obj == NULL || pdf == NULL){		
		err_log("getObjectInfos :: invalid parameter\n");
		return -1;
	}
	

	// Get the object dictionary	
	if ((dico = getObjectDictionary(obj, pdf)) == NULL){
		return 0;
	}

	//dbg_log("getObjectInfos :: dictionary = %s\n\n",dico);
	obj->dico = dico;	
	
	// Get the object type	
	if ((type = getObjectType(obj)) != NULL){		
		obj->type = type;
	}

	// Get object stream content
	if ((stream = getObjectStream(obj)) != NULL){		
		obj->stream = stream;
	}

	// Get stream's filters
	if ((filters = getStreamFilters(obj)) != NULL){
		obj->filters = filters;
		//decodeObjectStream(obj); // to improve analysis time, the stream will be decoded during object analysis
	}
		
	return 1;
}


/*
extractObjectFromObjStream() :: extract objects embeeded in an object stream and add them in the objects list
parameters:
- struct pdfObject * obj (pdf object pointer)
- struct pdfDocument * pdf (pdf document pointer)
returns: (int)
- 0 on success.
- an error code (<0) on error.
*/
int extractObjectFromObjStream(struct pdfDocument * pdf, struct pdfObject *obj){


	int first = 0;
	int num = 0;
	int len = 0;
	int i = 0;
	int off = 0;
	int obj_ref_len = 0;
	int * obj_offsets;
	int obj_len = 0;
	int stream_len = 0;
	int ret = 0;
	int check = 0;

	char * stream  = NULL;
	char * start = NULL;
	char * end = NULL;
	char * obj_num_a = NULL;
	char * off_a = NULL;
	char * obj_ref = NULL;	
	char * obj_content = NULL;
	
	struct pdfObject * comp_obj  =NULL;
	

	if( pdf == NULL || obj == NULL ){		
		err_log("extractObjectFromObjStream :: invalid parameters\n");		
		return -2;
	}

	// first decode the stream if there is a filter applied.
	if (obj->filters != NULL){
		
		if (decodeObjectStream(obj) < 0){
			err_log("extractObjectFromObjStream :: decode object stream failed!\n");
			// if decoding object stream failed then exit.
			pdf->errors++;
			return -2;
		}
	}


	// avoid parsing object stream not well decoded.
	if (obj->filters != NULL && obj->decoded_stream == NULL){
		err_log("extractObjectFromObjStream :: object not decoded successfully!\n");
		return -2;
	}


	if( obj->decoded_stream != NULL ){

		stream = obj->decoded_stream;
		stream_len = obj->decoded_stream_size;
		//debugPrint(stream,);
		dbg_log("extractObjectFromObjStream :: stream = %d\n",stream_len);
		


	}else{

		stream = obj->stream;
		stream_len = obj->stream_size;

	}

	if(stream == NULL || stream_len <= 0){
		err_log("extractObjectFromObjStream :: NULL stream in object %s\n",obj->reference);
		return -2;
	}

	if( obj->dico == NULL){		
		err_log("extractObjectFromObjStream :: No dictionary in the Object stream %s\n", obj->reference);
		return -2;
	}

	dbg_log("extractObjectFromObjStream :: %s\n",obj->reference);


	// Get the number of object embedded in the stream => N in the dictionary
	if ((start = searchPattern(obj->dico, "/N", 2, strlen(obj->dico))) == NULL){
		err_log("extractObjectFromObjStream :: Entry /N not found in Object stream dictionary %s\n",obj->reference);		
		return -1;
	}
	
	start +=2;


	// if there is a space after /N
	if(start[0] == ' '){
		start ++ ;
	}

	len = strlen(obj->dico) - (int)(start - obj->dico);

	num = getNumber(start,len);
	if(num <= 0){		
		err_log("extractObjectFromObjStream :: Incorrect /N entry in object stream %s\n",obj->reference);		
		return -1;
	}


	// Get the byte offset of the first compressed object "/First" entry in dico
	if ((start = searchPattern(obj->dico, "/First", 6, strlen(obj->dico))) == NULL){
		err_log("extractObjectFromObjStream :: Entry /First not found in Object stream dictionary %s\n", obj->reference);
		return -1;
	}

	
	start +=6; // 6 => /First


	// skip white spaces after /First
	while (*start == ' '){
		start++;
	}

	len = strlen(obj->dico) - (int)(start - obj->dico);

	if ((first = getNumber(start, len)) <= 0){
		err_log("extractObjectFromObjStream :: Incorrect /First entry in object stream %s\n", obj->reference);
		return -1;
	}
	
	start = stream;
	len = stream_len;



	if ((obj_offsets = (int*)calloc(num, sizeof(int))) == NULL){
		err_log("extractObjectFromObjStream :: memory allocation failed!\n");
		return -1;
	}
	

	// Get objects number and offset
	for(i = 0 ; i< num; i++){

		// check if the offset if out-of-bound
		check = start - stream;
		if (check >= stream_len){
			//dbg_log("extractObjectFromObjStream :: check = %d :: stream_len = %d\n",check,stream_len);
			err_log("extractObjectFromObjStream :: bad offset in object stream %s\n", obj->reference);
			ret = -1;
			obj->errors++;
			pdf->errors++;			
			goto clean;
		}
		
		// Get the object number
		if ((obj_num_a = getNumber_s(start, len)) == NULL){
			err_log("extractObjectFromObjStream :: Can't extract object from object stream :: obj_ref = %s\n", obj->reference);
			free(obj_offsets);
			return -1;
		}

		len -=  strlen(obj_num_a);
		start += strlen(obj_num_a);

		// Move ptr for white space
		start ++ ;

		// Get the offset
		if ((off_a = getNumber_s(start, len)) == NULL){
			err_log("extractObjectFromObjStream :: Can't extract object from object stream :: obj_ref = %s\n", obj->reference);
			free(obj_num_a);
			free(obj_offsets);
			obj_num_a = NULL;
			return -1;			
		}

		off = atoi(off_a);
		obj_offsets[i] = off;

		len -=  strlen(off_a);
		start += strlen(off_a);
	
		// Move ptr for white space
		start ++ ;

		free(off_a);
		off_a = NULL;

		free(obj_num_a);
		// calc the length of the object according to the offset of the next object.
	}

	start = stream;
	len = strlen(stream);

	// Get objects content
	for(i = 0 ; i< num; i++){

		// init object
		if ((comp_obj = initPDFObject()) == NULL){
			err_log("extractObjectFromObjStream :: PDF object initilizing failed\n");			
			ret = -1;
			goto clean;
		}

		// Get the object number		
		if ((obj_num_a = getNumber_s(start, len)) == NULL){
			err_log("extractObjectFromObjStream :: Can't get object number :: obj_ref = %s\n", obj->reference);
			ret = -1;
			if (comp_obj != NULL){
				freePDFObjectStruct(comp_obj);
			}
			goto clean;			
		}

		// "X O obj"
		// Build the object reference
		obj_ref_len = strlen(obj_num_a) + 6;
		obj_ref = (char*)calloc(obj_ref_len+1,sizeof(char));
		obj_ref[obj_ref_len] = '\0';
		
		os_strncat(obj_ref, obj_ref_len+1, obj_num_a, strlen(obj_num_a));
		os_strncat(obj_ref,obj_ref_len+1, " 0 obj", 6);
		//dbg_log("extractObjectFromObjStream :: obj_ref = %s\n",obj_ref);
		comp_obj->reference =  obj_ref;


		// move ptr according to the size of the scanned number.
		len -=  strlen(obj_num_a);
		start += strlen(obj_num_a);

		// Move ptr for white space
		start ++ ;

		// Get the offset		
		if ((off_a = getNumber_s(start, len)) == NULL){
			err_log("extractObjectFromObjStream :: getNumber_s failed!\n");
			ret = -1;
			goto clean;
		}
		off = atoi(off_a);

		len -=  strlen(off_a);
		start += strlen(off_a);

		// Move ptr for white space
		start ++ ;


		// offset in stream = off + first
		// real offset = start + off + first.

		// calc the length of the object according to the offset of the next object.
		if( i != num-1 ){
			obj_len  = ( obj_offsets[i+1] - off);
		}else{
			// calc according to the end of the stream
			//obj_len =  strlen(stream) - ( (stream + first + off) - stream ) ;
			obj_len =  stream_len - ( (stream + first + off) - stream ) ;
		}
		
		if (obj_len <= 0){
			err_log("extractObjectFromObjStream :: bad object length! :: obj_len = %d\n", obj_len);
			ret = -1;
			goto clean;
		}
		

		// offset of the object content = stream ptr + ptr of the first obj + offset of the obj.
		end = stream + first + off;

		// check if the offset if out-of-bound
		check = end - stream;
		if (check >= stream_len){
			//dbg_log("extractObjectFromObjStream :: check = %d :: stream_len = %d\n",check,stream_len);
			err_log("extractObjectFromObjStream :: bad offset in object stream %s\n", obj->reference);
			ret = -1;
			obj->errors++;
			pdf->errors++;
			if (comp_obj != NULL){
				freePDFObjectStruct(comp_obj);
			}
			goto clean;
		}

		obj_content = (char*)calloc(obj_len + 1, sizeof(char));
		obj_content[obj_len] = '\0';

		// Get content
		memcpy(obj_content,end,obj_len);
		
		comp_obj->content = obj_content;
		comp_obj->content_size = obj_len;


		//  Get object informations		
		if (getObjectInfos(comp_obj, pdf) < 0){
			warn_log("extractObjectFromObjStream :: getObjectInfos failed for object %s\n",comp_obj->reference);
			pdf->errors++;
		}

		// Add object in list.
		if (addObjectInList(comp_obj, pdf) < 0){
			err_log("extractObjectFromObjStream :: Add object in list failed!\n");
			ret = -1;
			goto clean;
		}

		free(off_a);
		off_a = NULL;
		free(obj_num_a);
		obj_num_a = NULL;

		
	}

clean:
	if (obj_offsets != NULL){
		free(obj_offsets);
	}

	if (off_a != NULL){
		free(off_a);
	}

	if (obj_num_a != NULL){
		free(obj_num_a);
	}

	if (ret != 0){
		pdf->errors++;
	}

	return ret;
}


/*
getPDFObjects() :: get all objects defined in the document
parameters:
- struct pdfDocument * pdf (pdf document pointer)
returns: (int)
- 0 on success.
- an error code (<0) on error.
// TODO :: getPDFObjects :: use function searchPattern instead of strstr to get objects
*/
int getPDFObjects(struct pdfDocument * pdf){

	char * startobj_ptr = NULL;
	char * endobj_ptr = NULL;
	char * content = NULL;
	char * ref = NULL;
	int len = 0;
	int size;
	int gen_num_len = 0;
	int obj_num_len = 0;
	int ref_len =0;
	int tmp = 0;
	int offset = 0;
	struct pdfObject* obj = NULL;
	

	if (pdf == NULL){
		err_log("getPDFObjects :: invalid parameter\n");
		return -1;
	}
		
	
	endobj_ptr = pdf->content;
	size = pdf->size;
	
	while((startobj_ptr = searchPattern(endobj_ptr, "obj", 3,size) ) != NULL){	
	
		gen_num_len = 0;
		obj_num_len = 0;
		
		startobj_ptr -= 2; // go to the generation number
		

		// Check the generation number pointer
		if(startobj_ptr[0] < 48 || startobj_ptr[0] > 57){
			//dbg_log("getPDFObjects :: This is not a generation number:: %c \n",startobj_ptr[0]);
			endobj_ptr = startobj_ptr+3;
			continue;
		}


		// get the generation number length
		while(startobj_ptr[0] >= 48 && startobj_ptr[0] <= 57  ){
			startobj_ptr--;			
			gen_num_len++;
		}

		startobj_ptr -= 1; // point to the object number

		// Check the object number pointer
		if(startobj_ptr[0] < 48 || startobj_ptr[0] > 57){
			//printf("This is not a generation number:: %c \n",startobj_ptr[0]);
			endobj_ptr = startobj_ptr + gen_num_len + 4;
			continue;
		}

		// get the object number length
		while(startobj_ptr[0] >= 48 && startobj_ptr[0] <= 57  ){		
			startobj_ptr--;			
			obj_num_len ++;			
		}
		
		startobj_ptr++;
		
		ref_len = gen_num_len + 1 + obj_num_len + 1 + 3 ; // 1 = space and 3 = obj


		ref = (char*)calloc(ref_len+1,sizeof(char));
		ref[ref_len] = '\0';
		
		os_strncpy(ref,ref_len+1,startobj_ptr,ref_len);
				
		// get the real offset of the object.
		offset = (int)(startobj_ptr - pdf->content);
		
		/*debug print*/
		//dbg_log("getPDFObjects :: object reference = %s :: offset = %d\n", ref, offset);				
				
		tmp = (int)(pdf->size - (startobj_ptr - pdf->content));
		
		endobj_ptr = searchPattern(startobj_ptr,"endobj",6,tmp);
		if(endobj_ptr == NULL){
			// invalid object no "endobj" pattern found... Malformed PDF.
			err_log("getPDFObjects :: invalid object no \"endobj\" pattern found :: startobj_ptr = %d\n", startobj_ptr);
			pdf->errors++;
			free(ref);
			return -2;
		}
		
		// 6 => endobj
		endobj_ptr += 6;
	
		len = (int)(endobj_ptr - startobj_ptr);		
		content = (char*)calloc(len+1,sizeof(char));
		content[len]='\0';
		
		memcpy (content, startobj_ptr,len);
		
		// Create and initialize pdf object
		if (!(obj = initPDFObject())){
			err_log("getPDFObjects :: pdf object creation failed!\n");
			free(ref);
			free(content);
			return -1;
		}

		obj->reference = ref;
		obj->content = content;
		obj->offset = offset;
		obj->content_size = len;


		// get objects informations
		if (getObjectInfos(obj, pdf) < 0){
			err_log("getPDFObjects :: get Object infos failed!\n");
			return -1;
		}
		

		// Extract object embedded in object stream
		if(obj->type != NULL && strncmp(obj->type,"/ObjStm",7) == 0 ){

			if (extractObjectFromObjStream(pdf, obj) < -1){
				err_log("getPDFObjects :: extract object from object stream failed!\n");
				freePDFObjectStruct(obj);
				return -1;
			}
		}		
				
		// Add in object list.
		if (addObjectInList(obj, pdf) < 0){
			err_log("getPDFObjects :: Add object in list failed!\n");
			return -1;
		}

		// calc the length left.
		size = (int)(pdf->size - (endobj_ptr - pdf->content));
	
	}

	
	
	return 0;
}


/*
getPDFTrailers() :: Get pdf trailer according to PDF documentation (before version 1.5)
parameters:
- struct pdfDocument * pdf (pdf document pointer)
returns: (int)
- 0 on success.
- an error code (<0) on error.
*/
int getPDFTrailers(struct pdfDocument * pdf){

	char * content = NULL;
	char * decoded_content = NULL;
	char * start = NULL; 
	char * end = NULL;
	int len = 0;
	struct pdfTrailer * trailer = NULL;

	if (pdf == NULL){
		err_log("getPDFTrailers :: invalid parameter\n");
		return -1;
	}


	end = pdf->content;
	len = pdf->size;

	while(len > 7 && (start = searchPattern(end,"trailer",7,len)) ){

		len = (int)(start - end);
		len = pdf->size -len ;
		end = searchPattern(start,"%%EOF",5,len);
		if (end == NULL){
			warn_log("getPDFTrailers :: missing end of trailer!\n");
			continue;
		}

		end += 5;

		len = (int)(end - start);
		content = (char*)calloc(len+1,sizeof(char));
		content[len] = '\0';

		memcpy(content,start,len);
	
		if(!(trailer = initPDFTrailer())){			
			err_log("getPDFTrailers ::  pdfTrailer structure initilizing failed\n");
			free(content);
			return -1;
		}
				
		// check if the trailer dictionary is hexa obfuscated
		decoded_content = hexaObfuscationDecode(content);

		if(decoded_content != NULL){			
			warn_log("getPDFTrailers :: Obfuscated trailer dictionary !!\n");
			pdf->testStruct->obfuscated_object ++ ;
			free(content);
			trailer->content = decoded_content;
		}else{
			trailer->content = content;	
		}

		// check if the file is encrypted
		if( searchPattern(trailer->content,"/Encrypt",8,len) != NULL){
			warn_log("getPDFTrailers :: This PDF Document is encrypted !\n");
			pdf->testStruct->encrypted = 1;
		}

		if (addTrailerInList(pdf, trailer) < 0){
			err_log("getPDFTrailers :: add trailer failed!\n");
			return -1;
		};
		
		/* debug print*/
		//dbg_log("trailer content = %s\n",trailer->content);

		len = (int)( end - pdf->content);
		len = pdf->size - len;


	}

	return 0;
	
}


/*
getPDFTrailers_ex() :: Get pdf trailer according to PDF documentation (starting from version 1.5)
parameters:
- struct pdfDocument * pdf (pdf document pointer)
returns: (int)
- 0 on success.
- an error code (<0) on error.
*/
int getPDFTrailers_ex(struct pdfDocument * pdf){

	char * content = NULL;
	char * start = NULL; 
	char * end = NULL;
	int len = 0;
	struct pdfTrailer * trailer;

	if (pdf == NULL){
		err_log("getPDFTrailers_ex :: invalid parameter\n");
		return -1;
	}


	end = pdf->content;
	len = pdf->size;

	while( (start = searchPattern(end,"startxref",9,len) ) != NULL ){

		len = (int)(start - end);
		len = pdf->size -len ;
		end = searchPattern(start,"%%EOF",5,len);
		if (end == NULL){
			warn_log("getPDFTrailers_ex :: missing end of trailer!\n");
			continue;
		}
		end += 5;
		

		len = (int)(end - start);
		content = (char*)calloc(len+1,sizeof(char));
		content[len] = '\0';

		memcpy(content,start,len);
	
		if(!(trailer = initPDFTrailer())){			
			err_log("Error :: getPDFTrailers_ex :: pdfTrailer structure initilizing failed\n");			
			free(content);
			return -1;
		}
		
		// TODO improve
		trailer->content = content;

		addTrailerInList(pdf,trailer);
		
		/* debug print*/
		//dbg_log("getPDFTrailers_ex :: trailer content = %s\n",content);

		len = (int)( end - pdf->content);
		len = pdf->size - len ;

		if (len <= strlen("startxref")){
			break;
		}

	}	

	return 0;
	
}


// This function remove the comment in the src stream
// Returns NULL if there is no comment in this line
char *removeCommentLine(char * src, int size, int * ret_len){

	char * start= NULL;
	char * out = NULL;
	int len = 0;


	// No comment in this line
	if((start = searchPattern(src,"%",1,size)) == NULL){
		return NULL;
	}

	if(start[1] == '%'){
		return NULL;
	}


	len = (int)(start - src);
	/*if(len == 0){

	}*/

	* ret_len = len;

	out = (char*)calloc(len+1,sizeof(char));
	out[len]='\0';

	memcpy(out,src,len);

	return out;
}


/*
removeComments() :: remove all PostScript comments in the pdf document
parameters:
- struct pdfDocument * pdf (pdf document pointer)
returns: (int)
- 0 on success.
- an error code (<0) on error.
// TODO :: removeComments :: split this function (implement function get_line, etc.)
*/
int removeComments(struct pdfDocument * pdf){


	char * new_content = NULL;
	char * tmp = NULL;
	char * uncomment = NULL;
	char * start = NULL;
	char * end = NULL;
	char * line = NULL;
	char * comment = NULL;
	char * white_spaces = NULL;
	char * ptr = NULL;
	char * tmp_spaces = NULL;
	int content_len = 0;
	int tmp_len = 0;	
	int len = 0;
	int line_size = 0;
	int line_num = 0;
	int uncomment_len =0;
	int white_spaces_len = 0;
	int inStream = 0;
	int inString = 0;
	int inQuotes = 0;
	int after_header = 0; // line juste after header tag
	int after_eof = 0; // case when there is bytes after %%EOF. // due to file: CVE_2010-2883_PDF_851D895614645756999BD9F6E002C127.pdf
	int len_tmp = 0;
	int i = 0;

	//char white_space = 0;

	char * mal_comments[] = {"endobj","obj","endstrem","stream","trailer", "startxref", "xref"};
	int mal_comments_num = 7;


	if (pdf == NULL){
		err_log("removeComments :: invalid parameter!\n");
		return -1;
	}

	if (pdf->size > LARGE_FILE_SIZE){
		warn_log("removeComments :: skipping removeComment because of the file size (%d bytes)\n", pdf->size);
		return 0;
	}

	if (pdf->content == NULL || pdf->size <= 0){
		err_log("removeComments :: null pdf content!\n");
		return -1;
	}


	// variable initialization.
	len = pdf->size;
	start = pdf->content;
	end = start;

	
	//for each line
	while(len > 0){
		
		len_tmp = len;
		start = end;

		// scan line
		while( (len_tmp > 0) && (end[0] != '\r') && (end[0] != '\n') && (end[0] != '\f')){
			end ++;
			len_tmp --;
		}

		// If the end of file is reached
		if(len_tmp == 0){
			len = 0;
		}

		//white_space = end[0];
		
		// line
		line_size = (int)(end-start);
		line = (char*)calloc(line_size+1,sizeof(char));
		line[line_size] = '\0';

		memcpy(line,start,line_size);
		line_num++;
		//dbg_log("removeComments :: line num = %d\n", line_num);

		/* debug print */
		//printf("New line = %s :: line_size = %d :: white_space %d\n", line,line_size,white_space);
		//dbg_log("removeComments :: line = %s\n",line);

		if (strcmp(line, "trailer") == 0){
			dbg_log("removeComments :: line = %s\n", line);
		}

		// calc whites spaces
		white_spaces_len = 0;

		tmp_spaces = end;
		while((len_tmp > 0) && ((end[0] == '\r') || (end[0] == '\n') || (end[0] == '\f'))){
			end ++;
			white_spaces_len ++;
			len_tmp --;
		}

		if(len_tmp == 0){
			len = 0;
		}

		if(white_spaces_len > 0){
			white_spaces = (char*)calloc(white_spaces_len +1,sizeof(char));
			white_spaces[white_spaces_len]='\0';
			if(len_tmp > 0 )
				memcpy(white_spaces,tmp_spaces,white_spaces_len);	
		}
		


		//------------------------
		// Remove comment in line
		//------------------------

		uncomment_len = 0;

		// line after the header flag
		//after_header = (after_header == 1)?2:0;
		if(after_header == 1)
			after_header = 2;
		else
			after_header  =0;
			
		ptr = line;

		// Scan the line
		for(i= 0; i< line_size ; i++){


			// String delimiter
			if(inStream == 0 && ptr[i] == '(' && inQuotes == 0 && ((i == 0) || (i > 0 && ptr[i-1] != '\\')) ){
				inString ++;
			}

			// String delimiter 2
			if(inStream == 0 && ptr[i] == ')' && inQuotes == 0 && inString > 0 && ((i == 0) || (i > 0 && ptr[i-1] != '\\'))){
				inString --;
			}

			// Quotes delimiter
			if(inStream == 0 && ptr[i] == '"' && ((i == 0) || (i > 0 && ptr[i-1] != '\\'))){
				inQuotes = (inQuotes == 0)?1:0;
			}				

			// If % is not in string
			if(ptr[i] == '%' && inString == 0 && ((i == 0) || (i > 0 && ptr[i-1] != '\\'))){
				// remove conmment
				
				// %%EOF
				// %PDF-version
				if(line_size -i >= 5 && memcmp(ptr+i,"%%EOF",5) == 0){
					dbg_log("removeComments :: pdf end of file :: EOF marker !!\n");
					after_eof = 1;
					i = line_size;
					continue;
				}else{

					if(line_size - i >= 8 && memcmp(ptr,"%PDF-1.",7) == 0){
						after_header = 1;
						i = line_size;
						dbg_log("removeComments :: PDF Header found !!\n");
						continue;
					}else{

						// header line immediatly followed by %[bin]
						if(after_header == 2  &&  ((i==0) || ((line_size - i) >= 5 && (unsigned char)ptr[i+1]>=128  && (unsigned char)ptr[i+2]>=128 && (unsigned char)ptr[i+3]>=128 && (unsigned char)ptr[i+4]>=128)) ){  
							after_header = 1;
							i = line_size;
							continue;
						}else{
							
							if(i>0){

								uncomment = (char*)calloc(i+1,sizeof(char));
								uncomment[i]='\0';
								memcpy(uncomment,line,i);
								uncomment_len = i;

							}
							

							// comment
							if(inStream == 0){
								comment = (char*)calloc((line_size - i)+1,sizeof(char));
								comment[line_size -i] = '\0';
								memcpy(comment,ptr+i,line_size -i );
								//dbg_log("removeComments :: comment = %s :: %d :: %d\n",comment,line_size,i);
							}

							i = line_size;
							continue;

						}

					}

				}
				
			}

		} // end for(i > line_len)


		inString = 0;

		
		//--------------------------------------------------------------

		
		// remove comments in line
		//uncomment = removeCommentLine(line,line_size,&uncomment_len);
		//printf("uncomment = %s :: len = %d \n", uncomment,uncomment_len);

		// 
		if(uncomment == NULL){
			uncomment = (char*)calloc(line_size+1,sizeof(char));
			uncomment[line_size]='\0';
			//uncomment = line;
			memcpy(uncomment,line,line_size);
			uncomment_len = line_size;
			
		}else{

			if(inStream == 0 && after_eof == 0){
				
				
				dbg_log("removeComments :: Comment found :: %s\n",uncomment);
				dbg_log("removeComments :: line :: %s\n", line);
				dbg_log("removeComments :: line num = %d :: after_oef = %d\n", line_num,after_eof);
				
				pdf->testStruct->comments ++;

				// Check for a malicious comment :: comments puts to defeat parser (with keywords like "endobj", "obj", "stream", "endstream" )
				for(i = 0; i< mal_comments_num ; i++){

					if(searchPattern(comment,mal_comments[i],strlen(mal_comments[i]),strlen(comment)) != NULL ){						
						warn_log("removeComments :: potentially malicious comment :: (%s) found in pdf Document\n", comment);						
						pdf->testStruct->malicious_comments ++;
						break;
					}

				}

			}
				
		}

		//printf("uncomment = %s :: uncomment_len = %d \n", uncomment,uncomment_len);


		//look if I'm in a stream content before adding the uncommented line
		if(uncomment_len >=9 && searchPattern(uncomment,"endstream",9,uncomment_len) != NULL){			
			inStream = 0;
			

		}else{
			if( uncomment_len >=6 && searchPattern(uncomment,"stream",6,uncomment_len) != NULL){
				//printf("removeComments :: In Stream = 1\n");
				inStream = 1;
			}
		}



		if(inStream == 0 && after_eof == 0){

			//write uncommented line
			if(content_len > 0){

				tmp = (char*)calloc(content_len+1,sizeof(char));
				tmp_len = content_len;
				tmp[content_len]='\0';
				memcpy(tmp,new_content,content_len);
			}
			
						

			

			if(white_spaces_len > 0)
				content_len += (uncomment_len + white_spaces_len);
			else
				content_len += uncomment_len;
			//content_len += (uncomment_len + white_spaces_len);
			//printf("content_len = %d :: uncomment_len = %d\n",content_len,uncomment_len );

			//content_len ++; // due to white space
			free(new_content);
			new_content = (char*)calloc(content_len+1,sizeof(char)); // + 2 due to white space
			new_content[content_len]='\0';

			ptr=new_content;

			if(tmp != NULL)
				memcpy(ptr,tmp,content_len-uncomment_len-1);			

			ptr += tmp_len;

			//ptr = new_content + (content_len - uncomment_len);
			memcpy(ptr,uncomment,uncomment_len);			

			//ptr = new_content + content_len - 1;
			ptr += uncomment_len;

			if(white_spaces_len > 0)
				memcpy(ptr,white_spaces,white_spaces_len);
			


		}else{
			
			//write uncommented line			
			if(content_len > 0){
				tmp = (char*)calloc(content_len+1,sizeof(char));
				tmp_len = content_len;
				tmp[content_len]='\0';
				memcpy(tmp,new_content,content_len);
			}
			

			

			if(white_spaces_len > 0)
				content_len += (line_size + white_spaces_len);
			else
				content_len += line_size;			

			//content_len ++; // due to white space
			free(new_content);
			new_content = (char*)calloc(content_len+1,sizeof(char)); // + 2 due to white space
			new_content[content_len]='\0';


			ptr=new_content;

			if(tmp != NULL)
				memcpy(ptr,tmp,tmp_len);

			ptr += tmp_len;

			//ptr = new_content + (content_len - uncomment_len);
			if(line_size > 0)
				memcpy(ptr,line,line_size);			

			//ptr = new_content + content_len - 1;
			ptr += line_size;

			if (white_spaces_len > 0)
				memcpy(ptr,white_spaces,white_spaces_len);
			//ptr[0]=white_space;

		}


		//end ++;
		//printf("\n");
		len = (int)(end - pdf->content);
		len = pdf->size - len;


				
		free(tmp);
		free(line);
		free(uncomment);
		free(comment);
		free(white_spaces);
		//white_spaces == NULL;
		tmp = NULL;
		uncomment = NULL;
		comment = NULL;

		//printf("\n\n");
		

	} // end while(len > 0)

	
	dbg_log("removeComments :: Old size :: %d\n",pdf->size);
	dbg_log("removeComments :: New size :: %d\n", content_len);

	//printf("new content = \n");
	//printStream(new_content,content_len);


	free(pdf->content);
	pdf->content = NULL;


	// Set the uncommented content
	pdf->content = new_content;
	pdf->size = content_len;

	return 0;
}


/*
parsePDF() :: parse the PDF document (extract objects, trailers and xref).
parameters:
- struct pdfDocument * pdf (pdf document pointer)
returns: (int)
- 0 on success.
- an error code (<0) on error.
*/
int parsePDF(struct pdfDocument * pdf){

	int ret = 0;

	if(pdf == NULL){
		err_log("parsePDF :: Invalid parameter!\n");
		return -1;
	}
	
	// Check the magic number of the file
	// return -1 if unexpected error or -2 if bad header.
	if ((ret = checkMagicNumber(pdf)) < 0){
		err_log("parsePDF :: invalid header for file %s\n", pdf->fname);
		return ret;
	}


	// Get the content of the document
	if (getPDFContent(pdf) <= 0) {
		err_log("parsePDF :: get PDF content failed!\n");
		return -1;
	}

	// remove PostScript comments for a better parsing. (to fix :: improve comment removing).
	/*
	if (removeComments(pdf) < 0) {
		err_log("parsePDF :: removing comments failed!\n");
		return -1;
	}
	*/

	
	// Get Trailers (before version 1.5)
	if (getPDFTrailers(pdf) < 0) {
		err_log("parsePDF :: getting PDF trailer v1 failed!\n");
		return -1;
	}
	
	// Get Trailers extension function (from version 1.5)
	if (pdf->trailers == NULL){		
		if (getPDFTrailers_ex(pdf) < 0) {
			err_log("parsePDF :: getting PDF trailer v2 failed!\n");
			return -1;
		}
	}

	// if no trailer found in the document.
	if(pdf->trailers == NULL){
		warn_log("parsePDF :: no trailer found in the document!\n");
		pdf->testStruct->bad_trailer++;
	}
	
		
	// if the document is encrypted
	if( pdf->testStruct->encrypted > 0){		
		return -2;
	}

	// Get all objects defined in pdf document
	if (getPDFObjects(pdf) < 0) {
		// malformed PDF.
		err_log("parsePDF :: get PDF object failed!\n");
		return -1;
	}
	

	return 0;

}


