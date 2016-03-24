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


// This function checks if the header is correct
int checkMagicNumber(struct pdfDocument * pdf){
	
	
	int version_size = 8;
	int ret = 0;

	
	char * version;
	version = (char*)calloc(9,sizeof(char));
	version[8]='\0';
	
	if (pdf->fh == NULL && pdf->fd < 0) {
		printf("[-] Error :: checkMagicNumber :: invalid parameters\n");
		return -1;
	}

	// In this case use file handle.
	if (pdf->fh != NULL) {

		// Go to the beginning of the file
		fseek(pdf->fh,0,SEEK_SET);

		// Read the 8 first bytes Ex: %PDF-1.
		ret = fread(version,1,version_size,pdf->fh);

	}
	else {

		os_lseek(pdf->fd,0,SEEK_SET);
		os_read(pdf->fd, version, version_size);
	}

	//printf("pdf header = %s\n",version);
	
	if( strncmp(version,"%PDF-1.1",8) == 0 || strncmp(version,"%PDF-1.2",8) == 0 || strncmp(version,"%PDF-1.3",8) == 0 || strncmp(version,"%PDF-1.4",8) == 0 || strncmp(version,"%PDF-1.5",8) == 0 || strncmp(version,"%PDF-1.6",8) == 0 || strncmp(version,"%PDF-1.7",8) == 0 ){
	
		printf ("PDF Header OK = %s\n",version);
		pdf->version = version;
	
	}else{
		//printf ("PDF Header KO : This document is not a PDF file.\n");
		pdf->testStruct->bad_header = 1;
		//pdf->version = "none";
		
		//TODO XDP files
		
		return -1;
	}
	
	return 0;
	
}

// This function get the content of the PDF document
int getPDFContent(struct pdfDocument * pdf){

	char * content = NULL;
	int doc_size = 0;
	int read_bytes;


	if (pdf->fh == NULL && pdf->fd < 0) {
		printf("[-] Error :: getPDFContent :: invalid parameters\n");
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
		//printf("Document Size  = %d\n",doc_size);
		//doc_size = _tell(pdf->fd);
		os_lseek(pdf->fd,0,SEEK_SET); // rewind		

	}
	
	
	
	#ifdef DEBUG
		printf("Document Size  = %d\n",doc_size);
	#endif
	
	content = (char*)calloc(doc_size+1,sizeof(char));
	content[doc_size]= '\0';
	if (content == NULL) {
		return -1;
	}
	
	if (pdf->fh != NULL) {
		read_bytes = fread(content, 1, doc_size, pdf->fh);
	}
	else {		
		read_bytes = os_read(pdf->fd, content, doc_size);
	}
	
	//printf("read bytes = %d\n",read_bytes);
	
	//printf("Document content = %s",content);
	
	pdf->content = content;	
	pdf->size = read_bytes;
	
	return read_bytes;
	
}



// This function decode a dictionnary obfuscated with hexa; return the modified dico or the original if there is no obfuscation.
char * hexaObfuscationDecode(char * dico){

	char * start = NULL;
	int len = 0, start_len = 0;
	char * decoded_dico = NULL;
	char * hexa = NULL;
	char * hexa_decoded = NULL;
	int is_space_hexa = 1;
	char * tmp = NULL;

	if (dico == NULL) {
		return NULL;
	}

	len = strlen(dico);
	
	start = searchPattern(dico,"#",1,len);

	if(start == NULL){
		return NULL;
	}

	start_len = (int)(start - dico);
	start_len = len - start_len;
	//printf("dico_len =  %d:: start_len = %d\n",len,start_len );
	//printf("dico = %s\n\n",dico);
	
	//decoded_dico = (char*)calloc(len,sizeof(char));
	hexa = (char*)calloc(4,sizeof(char));
	hexa[3] = '\0';

	//hexa = (char*)calloc(2,sizeof(char));
	hexa_decoded = (char*)calloc(2,sizeof(char));
	hexa_decoded[1] = '\0';



	//memcpy(decoded_dico,dico,len);
	tmp = (char*)calloc(len+1,sizeof(char));
	tmp[len]= '\0';
	memcpy(tmp,dico,len);
	


	while( start != NULL && start_len >= 3){

		
		//printf("FLAG1\n");
		// get the pointer of the hexa code
		start = getHexa(start,start_len);
		//printf("FLAG2\n");
		
		if(start == NULL){
			//start += 3;
			//len -=3;
			//len = 0;
			continue;
		}
			
		
		memcpy(hexa, start, 3);

		// #20 = space - #2F = '/' (slash) - #E9 = é - #2C = ','
		if(memcmp(hexa,"#20",3) != 0 && memcmp(hexa,"#2F",3) != 0 && memcmp(hexa,"#E9",3) != 0 && memcmp(hexa,"#2C",3) != 0){
			is_space_hexa = 0;
		}

		//memcpy(hexa, start, 2);
		//hexa[2]='\0';
		//printf("hexa = %s\n",hexa);
		
		//sscanf(hexa,"%x",&hexa_decoded[0]);
		os_sscanf(hexa,"%x",&hexa_decoded[0]);
		
		//printf("hexa_decoded_s = %s\n",hexa_decoded);

		decoded_dico = replaceInString(tmp,hexa,hexa_decoded);

		if(decoded_dico != NULL){

			free(tmp);
			tmp = NULL;

			tmp = decoded_dico;

			//printf("decoded_dico___  = %s\n\n",tmp);


		}else{
			#ifdef DEBUG
				printf("Error :: hexaObfuscationDecode :: replaceInString returns NULL \n");
			#endif
			return NULL;
		}


		start += 3;
		//start += 3;

		start_len = (int)(start - dico);
		start_len = strlen(dico) -start_len;
		//printf("len = %d\n",len);


	}
	
	free(hexa);
	free(hexa_decoded);

	if(decoded_dico != NULL && is_space_hexa == 0){
		//printf("dico = %s\n",dico);
		//printf("decoded_dico  = %s\n\n",decoded_dico);
		return decoded_dico;
	}
		


	return NULL ; 

}

//This function get the object dictionary
char * getObjectDictionary(struct pdfObject * obj, struct pdfDocument * pdf){
	
	char  * dico = NULL;
	char * decoded_dico = NULL;
	char * content =  obj->content;
	char * dico_start = NULL;
	char * end = NULL;
	int inQuotes = 0;
	int inString = 0;
	int sub = 0;
	int flag = 0;
	//char * dico_end = NULL;
	int len = 0;
	//int i = 0;
		
	//char* src, char* pat , int pat_size ,  int size


	
	// Search the beginning of the
	dico_start = searchPattern(content,"<<",2,obj->content_size);


	if(dico_start == NULL){
		//printf("No dictionary found in object %s!!\n", obj->reference);
		return NULL;
	}

	//printf("hey == %s\n",content);
	
	// TODO search other occurencies of "<<" to detect sub dictionaries
	// TODO Found the same number of occurencies of stream ">>"
	
	len = (int)(dico_start - obj->content);
	len = obj->content_size - len;

	//printf("getDico :: len = %d\n",len);

	/////////////////////////////////////////////////

	end = dico_start;
	// Scan the line
	//for(i= 0; i< len ; i++){

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

	
	//dico =  getDelimitedStringContent(dico_start,"<<", ">>", len);

	/*
	if(dico == NULL){
		#ifdef DEBUG
			printf("Warning :: getObjectDictionary :: No dictionary found in object %s\n",obj->reference);
		#endif
		return NULL;
	}*/

	//len = strlen(dico);

	//printf("dico = %s---->\n",dico);
	

	// decode hexa obfuscated dictionaries
	decoded_dico = hexaObfuscationDecode(dico);

	if(decoded_dico != NULL){
		#ifdef DEBUG
			printf("Warning :: getObjectDictionary :: Obfuscated Object dictionary in %s\n", obj->reference);
		#endif
		free(dico);
		pdf->testStruct->obfuscated_object ++;
		return decoded_dico;
	}

	if (decoded_dico != NULL)
		free(decoded_dico);
	return dico;
	
}



// This function get the Object type described in dictionary
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


	pattern = (char*)calloc(pattern_len+1,sizeof(char));


	dico_len = strlen(obj->dico);
	len = dico_len;
	start = obj->dico;

	//printf("Dico = %s \n",obj->dico);
	//printf("len = %d\n",len);
	//printf("start0 = %c\n",start[0]);

	// skip first dico delimiter
	while(start[0] == '<'){
		start ++;
	}


	// Search /Type keyword (but outbound of a sub dictionary)
	while(len >= pattern_len && flag == 0){

		if(start[0] == '<' && start[1] == '<'){
			//printf("sub dico start :: %d\n",sub);
			//printf("start0 = %c :: %d\n",start[0],len);
			sub ++;
			start += 2;
			len = (int)(start - obj->dico);
			len = dico_len -len;			
			continue;
		}

		if(start[0] == '>' && start[1] == '>'){
			//printf("sub dico end :: %d\n",sub);
			//printf("start0 = %c :: %d\n",start[0],len);
			sub --;
			start += 2;
			len = (int)(start - obj->dico);
			len = dico_len -len;
			continue;
		}

		memcpy(pattern,start,5);

		if(sub == 0 && memcmp(pattern,"/Type",5) == 0 && start[5] != '1' && start[5] != '2' ){
			//printf("Found type delimiter :: start[5] = %c\n",start[5]);
			flag ++;		
			continue;
		}
		//printf("pattern = %s\n",pattern);

		//printf("start0 = %c :: %d\n",start[0],len);

		start++;

		len = (int)(start - obj->dico);
		len = dico_len -len;

	}

	free(pattern);
	pattern = NULL;


	//printf("start0 = %c%c\n",start[0],start[1]);
	//printf("sub = %d\n",sub);
	//printf("len = %d\n",len);

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

	//printf("start0 = %c :: end0 = %c\n",start[0],end[0]);


	if(type_len == 0){
		return NULL;
	}

	type_len ++; // add it for '/'
	type = (char*)calloc(type_len+1,sizeof(char));
	type[type_len]= '\0';
	//printf("getObjectType :: len = %d \n",len);
	//start += 4;
	//len = (int)(end-start);
	memcpy(type,start,type_len);
	

	return type;
}


// This function get the stream content of an object (returns NULL if there is no stream)
char * getObjectStream(struct pdfObject * obj){

	char * stream = NULL;
	char * start = NULL;
	char * end = NULL;
	int len = 0;
	//char * tmp = NULL;

	start = searchPattern(obj->content,"stream",6,obj->content_size);

	
	if(start == NULL){
		return NULL;
	}

		
	len = (int)(start - obj->content);
	len = obj->content_size -len;

	//printf("getObjectStream :: len = %d\n",len);

	//printf("start = %d\n",start);
	end = searchPattern(start,"endstream",9,len);
	//printf("end = %d\n",end);

	if(end == NULL){
		return NULL;
	}


	
	//len = (int)(end-start);
	
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
	//printf("end = %d\n",end);

	len = (int)(end-start);
	if(len <= 0 ){
		#ifdef DEBUG
			printf("Warning :: Empty stream content in object %s\n", obj->reference);
		#endif
		return NULL;
	}


	//printf("Stream len = %d\n",len);

	obj->stream_size = len; // -1 for the white space
	obj->tmp_stream_size = obj->stream_size;

	stream = (char*)calloc(len+1,sizeof(char));
	stream[len]='\0';
	
	memcpy(stream,start,len);

	return stream;

}



char * getStreamFilters(struct pdfObject * obj){

	char * start = NULL;
	char * end = NULL;
	char * end_tmp = NULL;
	int len = 0;
	char * filters = NULL;
	


	
	start = searchPattern(obj->dico,"/Filter",7,strlen(obj->dico));
	//printf("getStreamFilters :: start = %d\n",start);
	
	if( start == NULL ){
		//printf("I got no filter !!\n");
		return NULL;
	}
	
	len = (int)(start - obj->dico);
	start += 1;
	//printf("len = %d \t start = %d \t obj->dico = %d\n",len, start,obj->dico);
	
	
	
	end = memchr(start,'/',strlen(obj->dico)-len);
	end_tmp = memchr(start,'[',strlen(obj->dico)-len);
	
	if(end == NULL){
		return NULL;
	}

	
	// if a bracket is before the first / that means it's an array of filters
	if( end_tmp != NULL && end_tmp < end ){

		//printf("getStreamFilters :: heyhey = %d\n",end);
		
		end  = end_tmp;
		start = end;
		
		len = 0;
		while(end[0] != ']'){
			end ++;
			len ++;
		}

		len +=1;
		//printf("getStreamFilters :: len = %d\n",len);
		//printf("end debug :: %c",end[0]);


	}else{ // a single filter
	
		start = end;
		len = 0;
		do{
			//printf("char = %c\n",end[0]);
			end ++;
			len++;
		
		}while( (end[0] >=97 && end[0] <=122) || (end[0] >=65 && end[0] <=90 ) || (end[0] >=48 && end[0] <= 57) ); // Lower case [97 122] or Upper case [65 90] + digit [48 57]
		
	
	}
	
	
	//filters = (char*)malloc(len*sizeof(char));
	filters = (char*)calloc(len+1,sizeof(char));
	filters[len] = '\0';
	//printf("len = %d \n",len);

	//start += 6;
	//len = (int)(end - start);
	//printf("getStreamFilters :: len = %d \n",len);

	os_strncpy(filters,len+1,start,len);
	
	//printf("filters = %s \n",filters);
	
	
	return filters;

}



// This function decode an object stream according to the filters applied
int decodeObjectStream(struct pdfObject * obj){

	
	char * start = NULL;
	char * end = NULL;
	char * filter =NULL; 
	char * stream = NULL;
	char * tmp = NULL;
	int len = 0;
	unsigned int i =0;
	int filter_applied = 0;
	
	
	
	if(obj->filters == NULL){
		#ifdef DEBUG
			printf("Error :: decodeObjectStream :: There is no filter implemented\n");
		#endif
		return -1;
	}
	
	//printf("implemented filters = %s\n",obj->filters);
	if(obj->stream == NULL){
		#ifdef DEBUG
			printf("Error :: decodeObjectStream :: Null Stream in object %s\n", obj->reference);
		#endif
		return -1;
	}


	//stream = obj->stream;
	end = obj->filters;

	stream = (char*)calloc(obj->stream_size+1,sizeof(char));
	stream[obj->stream_size]='\0';
	memcpy(stream,obj->stream,obj->stream_size);




	//while( (start = memchr(end, '/',strlen(obj->filters)-len)) != NULL ){
	while( (start = strchr(end, '/')) != NULL ){

		//printf("Searching filter in :: %d :: %s\n",end,end);
		//printf("Searching filter in :: %d :: %s\n",start,start);
		
		//filter ++;
		i = 0;
		end = start;		
		// Scan the filter name
		do{
			end ++;
			i++;
		}while( i < strlen(obj->filters) && ((end[0] >=97 && end[0] <=122) || (end[0] >=65 && end[0] <=90 ) || (end[0] >=48 && end[0] <= 57)));
		
		len = (int)(end-start);
		
		//filter = (char*)malloc(len*sizeof(char));
		filter = (char*)calloc(len+1,sizeof(char));
		filter[len] = '\0';

		//printf("filter_len = %d\n",len);

		os_strncpy(filter,len+1,start,len);
		//printf("implemented filter_end = %s\n",filter);
		
		//len -= filter - strlen(obj->filter);
		
		
		// Apply decode filter
		
		//TODO
		if((strncmp(filter,"/FlateDecode",12) == 0 && strncmp(filter,"/FlateDecode",strlen(filter)) == 0) || (strncmp(filter,"/Fl",3) == 0 && strncmp(filter,"/Fl",strlen(filter)) == 0)){
				#ifdef DEBUG
					printf("Decode Fladetecode :: %s\n",obj->reference);
				#endif
				tmp = FlateDecode(stream, obj);
				free(stream);
				stream  = tmp;

				/*if(strncmp(obj->reference,"8 0 obj",8) == 0)
					printf("stream _flatedecode = %s\n",stream);*/
				filter_applied ++;
		}else{

			if((strncmp(filter,"/ASCIIHexDecode",15) == 0 && strncmp(filter,"/ASCIIHexDecode",strlen(filter)) == 0) || (strncmp(filter,"/AHx",4) == 0 && strncmp(filter,"/AHx",strlen(filter)) == 0)){
				#ifdef DEBUG
					printf("Decode ASCIIHexDecode :: %s\n",obj->reference);
				#endif
				tmp = ASCIIHexDecode(stream, obj);
				free(stream);
				stream  = tmp;
				filter_applied ++;
		
			}else{

				
				if((strncmp(filter,"/ASCII85Decode",14) == 0 && strncmp(filter,"/ASCII85Decode",strlen(filter)) == 0) || (strncmp(filter,"/A85",4) == 0 && strncmp(filter,"/A85",strlen(filter)) == 0)){
					//if(strncmp(obj->reference,"44 0 obj",8) == 0)
					#ifdef DEBUG
						printf("Decode ASCII85Decode :: %s \n",obj->reference);
					#endif
					tmp = ASCII85Decode(stream, obj);

					free(stream);
					stream  = tmp;
					filter_applied ++;
				}else{

					
					if((strncmp(filter,"/LZWDecode",10) == 0 && strncmp(filter,"/LZWDecode",strlen(filter)) == 0) || (strncmp(filter,"/LZW",4) == 0 && strncmp(filter,"/LZW",strlen(filter)) == 0)){
						#ifdef DEBUG
							printf("Decode LZWDecode :: %s\n",obj->reference);
						#endif
						tmp = LZWDecode(stream,obj);
						free(stream);
						stream  = tmp;
						//printf("LZWDecode stream = %s\n",stream );
						filter_applied ++;
					}else{

						//TODO
						if((strncmp(filter,"/RunLengthDecode",16) == 0 && strncmp(filter,"/RunLengthDecode",strlen(filter)) == 0) || (strncmp(filter,"/RL",3) == 0 && strncmp(filter,"/RL",strlen(filter)) == 0) ){
							#ifdef DEBUG
								printf("Warning :: Filter RunLengthDecode not implemented :: %s\n",obj->reference );
							#endif
							filter_applied = 0;
										
						}else{

							

							if((strncmp(filter,"/CCITTFaxDecode",15) == 0 && strncmp(filter,"/CCITTFaxDecode",strlen(filter)) == 0) || (strncmp(filter,"/CCF",4) == 0 && strncmp(filter,"/CCF",strlen(filter)) == 0)){
								//if(strncmp(obj->reference,"11 0 obj",8) == 0)
								#ifdef DEBUG
									printf("Decode CCITTFaxDecode :: %s \n",obj->reference);
								#endif
								tmp = CCITTFaxDecode(stream,obj);
								free(stream);
								stream  = tmp;

								filter_applied ++;
								
							}else{
								#ifdef DEBUG
									printf("Filter %s  in object %s not implemented\n",filter,obj->reference);
								#endif
								
							}

						}


					}

				}


			}

		}
		
		free(filter);
		
		
	}
	
	
	// Store the decoded stream
	if(stream != NULL && filter_applied > 0){
		obj->decoded_stream = stream ;
	}else{
		if(stream != NULL){
			free(stream);
			stream = NULL;	
		}
		
	}
	
	return 0;
}



// Get object information (types, filters, streams, etc.)
int getObjectInfos(struct pdfObject * obj, struct pdfDocument * pdf){

	char * dico = NULL;
	char * type = NULL;
	char * stream = NULL;
	char * filters = NULL;
	//int res = 0;
	

	if(obj == NULL){
		#ifdef DEBUG
			printf("Error :: getObjectInfos :: null object\n");
		#endif
		return -1;
	}
	

	// Get the dictionary
	dico = getObjectDictionary(obj,pdf);
	if(dico == NULL){
		return 0;
	}
	//printf("dictionary = %s\n\n",dico);
	obj->dico = dico;
	
	/*if(strncmp(obj->reference,"17 0 obj",8)== 0){
		type = getObjectType_2(obj);
		printf("DEBUG :: /Type = %s\n\n",type);
	
	}*/
	
	// Get the type
	type = getObjectType(obj);
	if(type != NULL){
		//printf("/Type = %s\n\n",type);
		obj->type = type;
	}

	
	
	

	// Get stream content
	stream  = getObjectStream(obj);

	if(stream != NULL){
		//printf("stream = %s--\n\n",stream);
		obj->stream = stream;
	}

	// Get stream filters
	filters = getStreamFilters(obj);
	
	if(filters != NULL){
		obj->filters = filters;
		//decodeObjectStream(obj); // to uncomment
		//printf("res = %d\n",res);
	}

	// debug
	
	/*if(strncmp(obj->reference,"19 0 obj",sizeof(obj->reference)) == 0){
		printObject(obj);
	}*/
	
		
	return 1;
	

}



// This function extract object in an object stream and add them in the object list
int extractObjectFromObjStream(struct pdfDocument * pdf, struct pdfObject *obj){


	int first = 0;
	int num = 0;
	//char * num_a = NULL;
	//char * first_a = NULL;
	char * stream  = NULL;
	char * start = NULL;
	char * end = NULL;
	int len = 0;
	int i = 0;
	//int obj_num = 0;
	char * obj_num_a = NULL;
	int off = 0;
	//char * off_next = 0;
	//char * obj_num_next_a = 0;
	char * off_a = NULL;
	char * obj_ref = NULL;
	int obj_ref_len = 0;
	char * obj_content = NULL;
	int * obj_offsets;
	int obj_len = 0;
	struct pdfObject * comp_obj  =NULL;
	int stream_len = 0;




	// verif params
	if( pdf == NULL || obj == NULL ){
		#ifdef DEBUG
			printf("Error :: extractObjectFromObjStream :: NULL Params\n");
		#endif
		return -1;
	}

	if( obj->decoded_stream != NULL ){

		stream = obj->decoded_stream;
		stream_len = obj->decoded_stream_size;

	}else{
		stream = obj->stream;
		stream_len = obj->stream_size;
	}

	if(stream == NULL){
		#ifdef DEBUG
			printf("Error :: extractObjectFromObjStream :: No stream in the Object stream %s\n",obj->reference);
		#endif
		return -1;
	}

	if( obj->dico == NULL){
		#ifdef DEBUG
			printf("Error :: extractObjectFromObjStream :: No dictionary in the Object stream %s\n",obj->reference);
		#endif
		return -1;
	}


	#ifdef DEBUG
		printf("::: extractObjectFromObjStream ::: %s\n",obj->reference);
	#endif


	if(strlen(stream) == 0){
		#ifdef DEBUG
			printf("Error :: extractObjectFromObjStream :: Null stream in object %s\n",obj->reference );
		#endif
		return -1;

	}

	//printf("stream = %s :: \n\n",stream);

	// stream verification 
	//printf("strm len = %d\n",strlen(obj->dico));

	/*
	if(strncmp(obj->reference,"15 0 obj",8) == 0){
			printf("stream = %s\n\n",stream);
	}
	*/

	// Get the number of object embedded in the stream => N in the dictionary
	start = searchPattern(obj->dico,"/N",2,strlen(obj->dico));
	//printf("start = %d\n",start);	

	if( start == NULL){
		#ifdef DEBUG
			printf("Error :: extractObjectFromObjStream :: Entry /N not found in Object stream dictionary %s\n",obj->reference);
		#endif
		return -1;
	}

	
	end = start;
	//printf("end[0] = %c\n",end[0]);
	start +=2;


	// if there is a space after /N
	if(start[0] == ' '){
		start ++ ;
	}

	end = start;

	len = strlen(obj->dico) - (int)(start - obj->dico);
	num = getNumber(start,len);
	//printf("num = %d\n",num);

	/*len = 0;
	do{
		//printf("end[0] = %c\n",end[0]);
		len ++;
		end ++;
	}while(end[0] >= 48 && end[0] <= 57);

	//printf("len = %d\n",len);

	num_a = (char*)calloc(len,sizeof(char));
	memcpy(num_a,start,len);

	num = atoi(num_a);
	printf("num = %d\n",num);
	*/


	if(num <= 0){
		#ifdef DEBUG
			printf("Error:: Incorrect /N entry in object stream %s\n",obj->reference);
		#endif
		return -1;
	}


	// Get the byte offset of the first compressed object "/First" entry in dico
	start = searchPattern(obj->dico,"/First",6,strlen(obj->dico));
	//printf("start = %d\n",start);	

	if( start == NULL){
		#ifdef DEBUG
			printf("Error :: Entry /First not found in Object stream dictionary %s\n",obj->reference);
		#endif
		return -1;
	}

	
	end = start;
	//printf("end[0] = %c\n",end[0]);
	start +=6;


	// if there is a space after /First (TODO Replace with a while to prevent several white spaces)
	if(start[0] == ' '){
		start ++ ;
	}

	end = start;
	len = 0;
	len = strlen(obj->dico) - (int)(start - obj->dico);
	first = getNumber(start,len);
	//printf("first = %d\n",first);


	if(first <= 0){
		#ifdef DEBUG
			printf("Error:: Incorrect /First entry in object stream %s\n",obj->reference);
		#endif
		return -1;
	}


	
	start = stream;
	//printf("start[0] = %c\n",start[0]);


	//len = strlen(stream);
	//printf("%d\n",strlen(stream));

	

	len = stream_len;



	obj_offsets = (int*)calloc(num,sizeof(int));


	// Get objects number and offset
	for(i = 0 ; i< num; i++){

		//printf("hey!!\n");
		//printf("start[0] = %c\n",start[0]);
		// Get the object number
		obj_num_a = getNumber_a(start,len);

		//printf("Hey :: %d :: %d\n",start,len);

		len -=  strlen(obj_num_a);
		start += strlen(obj_num_a);

		//printf("obj_num = %s\n",obj_num_a);


		// Move ptr for white space
		start ++ ;

		// Get the offset
		off_a = getNumber_a(start,len);
		off = atoi(off_a);

		obj_offsets[i] = off;


		len -=  strlen(off_a);
		start += strlen(off_a);

		//printf("off = %s\n",off_a);

		// Move ptr for white space
		start ++ ;
		//printf("start[0] = %c\n",start[0]);

		free(off_a);
		free(obj_num_a);
		// calc the length of the object according to the offset of the next object.		
	}

	#ifdef DEBUG
		printf("\n\n");
	#endif



	start = stream;
	len = strlen(stream);

	// // Get objects content
	for(i = 0 ; i< num; i++){


		// init object
		comp_obj = initPDFObject();

		if(comp_obj == NULL){
			#ifdef DEBUG
				printf("PDF object initilizing failed\n");
			#endif
			return -1;
		}

		// Get the object number
		obj_num_a = getNumber_a(start,len);





		// "X O obj"
		// Build the object reference
		obj_ref_len = strlen(obj_num_a) + 6;
		obj_ref = (char*)calloc(obj_ref_len+1,sizeof(char));
		obj_ref[obj_ref_len] = '\0';
		
		os_strncat(obj_ref, obj_ref_len+1, obj_num_a, strlen(obj_num_a));
		os_strncat(obj_ref,obj_ref_len+1, " 0 obj", 6);
		//printf("obj_ref = %s\n",obj_ref);
		comp_obj->reference =  obj_ref;


		// move ptr according to the size of the scanned number.
		len -=  strlen(obj_num_a);
		start += strlen(obj_num_a);


		//printf("obj_num = %s\n",obj_num_a);


		// Move ptr for white space
		start ++ ;

		// Get the offset
		off_a = getNumber_a(start,len);
		off = atoi(off_a);

		len -=  strlen(off_a);
		start += strlen(off_a);
		//printf("off = %s\n",off_a);

		// Move ptr for white space
		start ++ ;


		// offset in stream = off + first
		// real offset = start + off + first.

		// calc the length of the object according to the offset of the next object.
		if( i != num-1 ){
			obj_len  = ( obj_offsets[i+1] - off);
			//printf("obj_len = %d\n",obj_len);
		}else{
			// calc according to the end of the stream
			//obj_len =  strlen(stream) - ( (stream + first + off) - stream ) ;
			obj_len =  stream_len - ( (stream + first + off) - stream ) ;
			//printf("obj_len = %d\n",obj_len);
		}
		
		obj_content = (char*)calloc(obj_len+1,sizeof(char));

		
		obj_content[obj_len] = '\0';

		// offset of the object content
		end = stream + first + off;

		
		// Get content
		memcpy(obj_content,end,obj_len);
		

		//printf("extractObjectFromObjStream :: obj_content = %s\n", obj_content);


		comp_obj->content = obj_content;
		comp_obj->content_size = obj_len;

		//  Get object informations
		getObjectInfos(comp_obj,pdf);


		addObjectInList(comp_obj,pdf);

		free(off_a);
		free(obj_num_a);

		
	}
	free(obj_offsets);

	return 0;
}



// This function get all objects defined in the document
int getPDFObjects(struct pdfDocument * pdf){

	char * startobj_ptr;
	char * endobj_ptr;

	char * content;
	int len = 0;
	char * ref = NULL;
	int gen_num_len = 0;
	int obj_num_len = 0;
	int ref_len =0;
	int tmp = 0;
	struct pdfObject* obj = NULL;
	int offset = 0;
		
	
	//startobj_ptr = startobj_ptr - pdf->content;
	//printf("content ptr = %d\n\n",pdf->content);
	//printf("content ptr = %s\n",pdf->content);
	
	//startobj_ptr = (char*)malloc(gen_num_len*sizeof(char));
	
	endobj_ptr = pdf->content;
	//tmp = pdf->size;
	//len = pdf->size;


	
	
	while( (startobj_ptr = strstr(endobj_ptr,"obj")) != NULL){
	//while( (startobj_ptr = searchPattern(endobj_ptr,"obj",3,len) ) != NULL) {		
	
		//printf("search offset = %d\n",endobj_ptr);
		gen_num_len = 0;
		obj_num_len = 0;
		
		startobj_ptr -= 2; // point to the generation number
		//printf("Generation number = %c\n",startobj_ptr[0]);

		// Check the generation number pointer
		if(startobj_ptr[0] < 48 || startobj_ptr[0] > 57){
			//printf("This is not a generation number:: %c \n",startobj_ptr[0]);
			endobj_ptr = startobj_ptr+3;
			continue;
		}



		while(startobj_ptr[0] >= 48 && startobj_ptr[0] <= 57  ){
		
			//printf("Warning :: Treat this case 1 :: \n");
			startobj_ptr--;
			//printf("Warning :: Treat this case 1 :: %c\n", startobj_ptr[0] );
			gen_num_len++;
		}


	
		startobj_ptr -= 1; // point to the object number

		// Check the object number pointer
		if(startobj_ptr[0] < 48 || startobj_ptr[0] > 57){
			//printf("This is not a generation number:: %c \n",startobj_ptr[0]);
			endobj_ptr = startobj_ptr + gen_num_len + 4;
			continue;
		}

		//printf("object number = %c\n",startobj_ptr[0]);
		while(startobj_ptr[0] >= 48 && startobj_ptr[0] <= 57  ){
		
			startobj_ptr--;
			//printf("Warning :: Treat this case 2 :: %c\n",startobj_ptr[0] );
			obj_num_len ++;
			
		}
		
		startobj_ptr++;
		
		ref_len = gen_num_len + 1 + obj_num_len + 1 + 3 ; // 1 = space and 3 = obj


		ref = (char*)calloc(ref_len+1,sizeof(char));
		ref[ref_len] = '\0';
		
		os_strncpy(ref,ref_len+1,startobj_ptr,ref_len);
		//printf("object reference = %s :: %d\n",ref,ref_len);
		
		//startobj_ptr++;
		
		offset = (int)(startobj_ptr - pdf->content);
		
	
		//printf("object offset = %d\n",offset);

		//printf("start obj ptr = %d\n",startobj_ptr);
		
		//endobj_ptr = strstr(startobj_ptr,"endobj");
		//printf("end obj ptr 1 = %d\n",endobj_ptr);
		tmp = (int)(pdf->size - (startobj_ptr - pdf->content));
		//printf("size of block = %d\n",tmp);
		endobj_ptr = searchPattern(startobj_ptr,"endobj",6,tmp);


		//printf("end obj ptr = %d\n",endobj_ptr);
		
		if(endobj_ptr == NULL){
			// invalid object no "endobj" pattern found... Malformed PDF.
			//printf(":: Error :: startobj_ptr = %d\n", startobj_ptr);
			return -1;
		}
		//printf("end obj ptr = %c\n",endobj_ptr[0]);
		
		
		endobj_ptr += 6;
		//printf("end obj ptr = %c\n",endobj_ptr[0]);
		
	
		len = (int)(endobj_ptr - startobj_ptr);
		//printf("object len = %d\n",len);
	
		//content = (char*)malloc(len*sizeof(char));
		content = (char*)calloc(len+1,sizeof(char));
		content[len]='\0';
		
		memcpy (content, startobj_ptr,len);
		
		// Create a object
		
		//printf("\nobj content --\n%s--\n\n\n",content);
		
		obj = initPDFObject();		
		
		//printf("obj_content size = %d\n",len);
		//printf("obj_content = %s\n",content);
		if( obj != NULL){
			obj->reference = ref;
			obj->content = content;
			obj->offset = offset;
			obj->content_size = len;
			
			getObjectInfos(obj,pdf);
			
		}


		

		// Extract object emebedded in object stream
		if(obj->type != NULL && strncmp(obj->type,"/ObjStm",7) == 0 ){

			// Decode object stream			
			if(obj->filters != NULL){		
				decodeObjectStream(obj);
			}

			extractObjectFromObjStream(pdf,obj);
		}
		
		
		
		// Add in object list.
		addObjectInList(obj,pdf);
		//printf("------------------------------------\n\n");


	
	
	}
	
	//printf("content ptr = %d\n",pdf->content);
	//printf("startobj ptr = %s\n",startobj_ptr);
	
	
	return 0;
}



// Get pdf trailer according to PDF
int getPDFTrailers_1(struct pdfDocument * pdf){

	char * content = NULL;
	char * decoded_content = NULL;
	char * encrypt = NULL;
	char * start = NULL; 
	char * end = NULL;
	int len = 0;
	struct pdfTrailer * trailer;


	end = pdf->content;
	len = pdf->size;

	while( (start = searchPattern(end,"trailer",7,len)) ){

		//printf("hoo\n");

		len = (int)(start - end);
		len = pdf->size -len ;
		end = searchPattern(start,"%%EOF",5,len);
		end += 5;

		len = (int)(end - start);
		content = (char*)calloc(len+1,sizeof(char));
		content[len] = '\0';

		memcpy(content,start,len);
	
		if(!(trailer = initPDFTrailer())){
			#ifdef DEBUG
				printf("Error :: getPDFTrailers_1 ::  pdfTrailer structure initilizing failed\n");
			#endif
			return -1;
		}
		
		//printf("hoo :: %s\n",content);
		// check is the trailer dictionary is no hexa obfuscated
		decoded_content = hexaObfuscationDecode(content);
		//printf("hoo\n");


		if(decoded_content != NULL){
			#ifdef DEBUG
				printf("Warning :: getPDFTrailers_1 :: Obfuscated trailer dictionary !!\n");
			#endif
			pdf->testStruct->obfuscated_object ++ ;
			trailer->content = decoded_content;
		}else{
			trailer->content = content;	
		}

		// check if the file is encrypted
		if( (encrypt = searchPattern(trailer->content,"/Encrypt",8,len)) != NULL){
			#ifdef DEBUG
				printf("Warning :: getPDFTrailers_1 :: This PDF Document is encrypted !\n");
			#endif
			pdf->testStruct->encrypted = 1;
		}

		addTrailerInList(pdf,trailer);

		//pdf->trailers = trailer;
		//printf("trailer content = %s\n",trailer->content);

		len = (int)( end - pdf->content);
		len = pdf->size - len;

		//printf("\n");

	}

	return 0;
	
}


// Get pdf trailer according to PDF version starting from 1.5
int getPDFTrailers_2(struct pdfDocument * pdf){

	char * content = NULL;
	char * start = NULL; 
	char * end = NULL;
	int len = 0;
	struct pdfTrailer * trailer;


	end = pdf->content;
	len = pdf->size;

	while( (start = searchPattern(end,"startxref",9,len) ) != NULL ){

		len = (int)(start - end);
		len = pdf->size -len ;
		end = searchPattern(start,"%%EOF",5,len);
		end += 5;
		

		len = (int)(end - start);
		content = (char*)calloc(len+1,sizeof(char));
		content[len] = '\0';

		memcpy(content,start,len);
	
		if(!(trailer = initPDFTrailer())){
			#ifdef DEBUG
				printf("Error :: getPDFTrailers_2 :: pdfTrailer structure initilizing failed\n");
			#endif
			return -1;
		}
		
		// TODO improve
		trailer->content = content;

		addTrailerInList(pdf,trailer);

		//pdf->trailers = trailer;
		//printf("trailer content = %s\n",content);

		len = (int)( end - pdf->content);
		len = pdf->size - len ;

		//printf("\n\n");

	}

	/*
	
	start = searchPattern(pdf->content,"startxref",9,pdf->size);
	
	if(start == NULL ){
		return -1;
	}
	
	len = (int)(start - pdf->content);
	//len = obj->content_size - len;
	
	end = searchPattern(start,"%%EOF",5,pdf->size-len);
	
	
	if(end == NULL){
		return -1;
	}
	
	len = (int)(end - start);
	
	content = (char*)calloc(len,sizeof(char));
	
	memcpy(content,start,len);
	
	if(!(trailer = initPDFTrailer())){
		printf("Error :: while initilaizing pdfTrailer structure\n");
		return -1;
	}
	
	trailer->content = content;
	pdf->trailers = trailer;

	
	printf("trailer content = %s\n",content);
	*/
	
	// TODO get several trailers

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



// This function remove all PostScript comments in the pdf document
int removeComments(struct pdfDocument * pdf){


	char * new_content = NULL;
	char * tmp = NULL;
	int content_len = 0;
	int tmp_len = 0;
	char * uncomment = NULL;
	char * start = NULL;
	char * end = NULL;
	char * line = NULL;
	char * comment = NULL;
	int len = 0;
	int line_size = 0;	
	int uncomment_len =0;
	//char white_space = 0;
	char * white_spaces = NULL;
	int white_spaces_len = 0;
	char * ptr = NULL;
	char * tmp_spaces = NULL;

	int inStream = 0;
	int inString = 0; // todo multi line string
	int inQuotes = 0;
	int after_header = 0; // line juste after header tag

	int len_tmp = 0;

	int i = 0;

	char * mal_comments[] = {"endobj","obj","endstrem","stream","trailer", "startxref", "xref"};
	int mal_comments_num = 7;



	len = pdf->size;
	
	//content = pdf->content;

	if(pdf->content == NULL || pdf->size <= 0){
		#ifdef DEBUG
			printf("Warning :: removeComments :: pdf content is NULL \n");
		#endif
		return -1;
	}


	start = pdf->content;
	end = start;

	
	//printf("size = %d\n",pdf->size);
	//printf("start = %d\n",start);
	//printf("end[0] = %c\n",end[0]);
	


	//for each line
	while(len > 0){

		//printf("len = %d\n",len);
		len_tmp = len;

		start = end;

		//printf("heyhey\n");

		// scan line
		while( (len_tmp > 0) && (end[0] != '\r') && (end[0] != '\n') && (end[0] != '\f')){
			end ++;
			len_tmp --;
		}

		// If the end of file is reached
		if(len_tmp == 0){
			len = 0;
			//printf("End of file  reached\n");
		}

		//white_space = end[0];

		
		// line
		line_size = (int)(end-start);
		line = (char*)calloc(line_size+1,sizeof(char));
		line[line_size] = '\0';

		memcpy(line,start,line_size);
		//printf("New line = %s :: line_size = %d :: white_space %d\n", line,line_size,white_space);

		//printf("line = %s\n",line);

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
			//printf("End of file  reached\n");
		}

		//printf("end[0] = %c\n",end[0]);

		white_spaces = (char*)calloc(white_spaces_len +1,sizeof(char));
		white_spaces[white_spaces_len]='\0';
		if(len_tmp > 0)
			memcpy(white_spaces,tmp_spaces,white_spaces_len);
		//printf("white_spaces = %s--\n",white_spaces);


		//-----------------------------------------------------
		// Remove comment in line
		//-----------------------------------------------------

		uncomment_len = 0;

		// line after the heaer flag
		//after_header = (after_header == 1)?2:0;
		//printf("DEBUG1 :: line  =  %s :: %d\n",line,after_header );
		if(after_header == 1)
			after_header = 2;
		else
			after_header  =0;
		//printf("DEBUG2 :: line  =  %s :: %d\n",line,after_header );
			
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
					//printf("pdf end of file :: EOF marker !!\n");
					i = line_size;
					continue;
				}else{

					if(line_size - i >= 8 && memcmp(ptr,"\%PDF-1.",7) == 0){
						after_header = 1;
						i = line_size;
						//printf("PDF Header !!\n");
						continue;
					}else{

						// header line immediatly followed by
						if(after_header == 2  &&  ((i==0) || ((line_size - i) >= 5 && (unsigned char)ptr[i+1]>=128  && (unsigned char)ptr[i+2]>=128 && (unsigned char)ptr[i+3]>=128 && (unsigned char)ptr[i+4]>=128)) ){  
							after_header = 1;
							i = line_size;
							continue;
						}else{
							//printf("heyheyhey::\n");
							
							uncomment = (char*)calloc(i+1,sizeof(char));
							uncomment[i]='\0';
							memcpy(uncomment,line,i);
							uncomment_len = i;

							// comment
							if(inStream == 0){
								comment = (char*)calloc((line_size - i)+1,sizeof(char));
								comment[line_size -i] = '\0';
								memcpy(comment,ptr+i,line_size -i );
								//printf("Debug :: removeComments :: comment = %s :: %d :: %d\n",comment,line_size,i);
							}
							
							

							i = line_size;
							continue;

						}

						

					}

				}
				
				

			}

		}

		


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

			if(inStream == 0){
				
				#ifdef DEBUG
					printf("DEBUG :: Comment found :: %s\n",uncomment);
				#endif
				pdf->testStruct->comments ++;

				// Check for a malicious comment :: comments puts to defeat parser (with keywords like "endobj", "obj", "stream", "endstream" )
				for(i = 0; i< mal_comments_num ; i++){

					if(searchPattern(comment,mal_comments[i],strlen(mal_comments[i]),strlen(comment)) != NULL ){
						#ifdef DEBUG
							printf("Warning :: removeComments :: potentially malicious comment :: (%s) found in pdf Document\n", comment);
						#endif
						pdf->testStruct->malicious_comments ++;
						break;
					}

				}

			}
				
		}

		//printf("uncomment = %s :: uncomment_len = %d \n", uncomment,uncomment_len);


		//look if I'm in a stream content before adding the uncommented line
		if(uncomment_len >=9 && searchPattern(uncomment,"endstream",9,uncomment_len) != NULL){
			//printf("removeComments :: Out Stream = 0\n");
			inStream = 0;

		}else{
			if( uncomment_len >=6 && searchPattern(uncomment,"stream",6,uncomment_len) != NULL){
				//printf("removeComments :: In Stream = 1\n");
				inStream = 1;
			}
		}


		//printf("to write = %s \n", uncomment);


		//printf("content_len = %d :: uncomment_len = %d\n",content_len,uncomment_len );



		if(inStream == 0){

			//printf("I can write the uncommented line\n");
			//write uncommented line
			if(content_len > 0){

				tmp = (char*)calloc(content_len+1,sizeof(char));
				tmp_len = content_len;
				tmp[content_len]='\0';
				memcpy(tmp,new_content,content_len);
			}
			
						

			free(new_content);
			new_content = NULL;

			if(white_spaces_len > 0)
				content_len += (uncomment_len + white_spaces_len);
			else
				content_len += uncomment_len;
			//content_len += (uncomment_len + white_spaces_len);
			//printf("content_len = %d :: uncomment_len = %d\n",content_len,uncomment_len );

			//content_len ++; // due to white space
			new_content = (char*)calloc(content_len+1,sizeof(char)); // + 2 due to white space
			new_content[content_len]='\0';

			ptr=new_content;

			if(tmp != NULL)
				memcpy(ptr,tmp,content_len-uncomment_len-1);

			//printf("New content1 = %s\n",new_content);

			ptr += tmp_len;

			//ptr = new_content + (content_len - uncomment_len);
			memcpy(ptr,uncomment,uncomment_len);
			//printf("New content2 = %s\n",new_content);

			//ptr = new_content + content_len - 1;
			ptr += uncomment_len;

			if(white_spaces_len > 0)
				memcpy(ptr,white_spaces,white_spaces_len);
			//ptr[0]=white_space;

			//memset(new_content+content_len-1,'\n',1);
			//strncat(new_content,uncomment,uncomment_len);
			//printf("New content = %s\n",new_content);


		}else{

			//printf("stream content line\n");
			//write uncommented line
			
			if(content_len > 0){
				tmp = (char*)calloc(content_len+1,sizeof(char));
				tmp_len = content_len;
				tmp[content_len]='\0';
				memcpy(tmp,new_content,content_len);
			}
			

			free(new_content);
			new_content = NULL;

			if(white_spaces_len > 0)
				content_len += (line_size + white_spaces_len);
			else
				content_len += line_size;
			//printf("content_len = %d :: line_len = %d\n",content_len,line_size );

			//content_len ++; // due to white space
			new_content = (char*)calloc(content_len+1,sizeof(char)); // + 2 due to white space
			new_content[content_len]='\0';


			ptr=new_content;

			if(tmp != NULL)
				memcpy(ptr,tmp,tmp_len);

			//printf("New content1 = %s\n",new_content);

			ptr += tmp_len;

			//ptr = new_content + (content_len - uncomment_len);
			memcpy(ptr,line,line_size);
			//printf("New content2 = %s\n",new_content);

			//ptr = new_content + content_len - 1;
			ptr += line_size;

			
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
		line = NULL;
		uncomment = NULL;
		comment = NULL;
		
		//printf("len = %d\n",len );	

		//printf("\n\n");
		

	} // end while

	#ifdef DEBUG
		printf("Debug :: removeComments :: Old size :: %d\n",pdf->size);
		printf("Debug :: removeComments :: New size :: %d\n",content_len);
	#endif

	//printf("new content = \n");
	//printStream(new_content,content_len);


	free(pdf->content);
	pdf->content = NULL;


	// Set the uncommented content
	pdf->content = new_content;
	pdf->size = content_len;

	return 0;
}



int parsePDF(struct pdfDocument * pdf){

	if(pdf == NULL){
		printf("[-] Error :: parsePDF :: Invalid parameter!\n");
		return -1;
	}

	#ifdef DEBUG
	printf("\n");
	printf("------------------------------\n");
	printf("---  PDF DOCUMENT PARSING  ---\n");
	printf("------------------------------\n\n");
	#endif

	// Check the magic number of the file
	checkMagicNumber(pdf);
	
	if(pdf->testStruct->bad_header > 0){
		#ifdef DEBUG
		printf("[-] Error :: parsePDF :: Bad PDF header :: This file is not a PDF file ::\n");
		#endif
		return -2;
	}


	// Get the content of the document
	if (getPDFContent(pdf) <= 0) {
		printf("[-] Error :: parsePDF :: getPDF content failed\n");
		return -1;
	};


	// File too large
	if(pdf->size > LARGE_FILE_SIZE ){
		printf("Warning :: parsePDF :: Large file :: pdf size =  %d octets ==> Skipping the (RemoveComment) function\n",pdf->size);
		//pdf->testStruct->large_file ++;
		//return -3;
	}else{
		// Remove comments
		removeComments(pdf);
	}


	// Get Trailers
	getPDFTrailers_1(pdf);
	if(pdf->trailers == NULL){
		getPDFTrailers_2(pdf);

		if(pdf->trailers == NULL)
			pdf->testStruct->bad_trailer ++;
	}
	
	
	// if the document is encrypted
	if( pdf->testStruct->encrypted > 0 ){		
		return -2;
	}

	// Get objects described in pdf document
	if (getPDFObjects(pdf) < 0) {
		// malformed PDF.
		//return -1;
	}
	

	return 0;

}


