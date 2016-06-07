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


#include "utils.h"
#include "osdeps.h"
#include "log.h"


/*
getPDFObjectByRef() :: return the object corresponding to the reference given in parameter
parameters:
- struct pdfDocument * pdf (the pdf document pointer).
- char * ref (the reference of the object to search)
returns: (struct pdfObject *)
- the pointer of the pdf object on success
- NULL on error or if not found
*/
struct pdfObject * getPDFObjectByRef(struct pdfDocument * pdf, char * ref){

	struct pdfObject * obj = NULL;

	if(pdf == NULL  || ref == NULL){		
		err_log("getPDFObjectByRef :: invalid parameter\n");
		return NULL;
	}

	obj = pdf->objects;
		
	while(obj != NULL){
	
		if( strncmp(ref,obj->reference,strlen(ref)) == 0 ){
			return obj;
		}
		
		obj = obj->next;	
	}	

	return NULL;
}


/*
getPDFNextObjectByRef() :: return the object within the reference given in parameters (get the next object in the list, starting from obj, with the reference given in parameter)
parameters:
- struct pdfDocument * pdf (the pdf document pointer).
- char * ref (the reference of the object to search)
returns: (struct pdfObject *)
- the pointer of the pdf object on success
- NULL on error or if not found
*/
struct pdfObject * getPDFNextObjectByRef(struct pdfDocument * pdf, struct pdfObject * obj, char * ref){

	struct pdfObject * tmp = NULL;

	if (pdf == NULL || ref == NULL || obj == NULL){
		err_log("getPDFNextObjectByRef :: invalid parameter\n");
		return NULL;
	}

	tmp = obj->next;

	while(tmp != NULL){
	
		if( strncmp(ref,tmp->reference,strlen(ref)) == 0 ){
			return tmp;
		}
		
		tmp = tmp->next;	
	}

	return NULL;
}


/*
searchPattern() :: search a pattern in a stream
parameters:
- char * src (the source stream).
- char * pat (the pattern to search)
- int pat_size (the size of the pattern)
- int size (the size of the src stream)
returns: (char*)
- a pointer to the pattern string on success.
- NULL if not found or on error.
*/
void * searchPattern(char* src, char* pat , int pat_size ,  int size){

	char* res = NULL;
	char * tmp = NULL;
	char * end = NULL;
	int len = 0;
	int len_verif = 0;
	
	
	if( size < pat_size || src == NULL || pat == NULL || pat_size == 0 || size == 0){
		err_log("searchPattern :: invalid parameters\n");
		return NULL;
	}
		
	tmp =  (char*)calloc(pat_size+1,sizeof(char));
	tmp[pat_size] = '\0';
	

	len = size; 
	end = src;
	while(len >= pat_size){
	
		
		res = memchr(end,pat[0],len);		
		if(res == NULL){
			free(tmp);			
			return NULL;
		}


		len_verif = (int)(res-end);
		len_verif = len - len_verif;

		if(len_verif < pat_size){
			free(tmp);
			return NULL;
		}

		memcpy(tmp,res,pat_size);
		
		if( memcmp(tmp,pat,pat_size) == 0){
			free(tmp);
			return res;
		}
		
		end = res +1;

		len=(int)(end-src);
		len = size - len;

	}
	
	free(tmp);
	tmp = NULL;
	
	return NULL;
}


// Print object in a debug file (debug.txt)
void printObjectInFile(struct pdfObject * obj){


	FILE * debug = NULL;

	// Open en file
	if(!(debug = os_fopen("debug.txt","wb+"))){
		printf("open failed\n");
		return ;
	}

	//printf("DEBUG ::: \n");


	fputc('\n',debug);
	fputc('\n',debug);
	fputc('\n',debug);
	fwrite("---------------------------------------------",sizeof(char),45,debug);
	fputc('\n',debug);

	// Reference
	fwrite(obj->reference,sizeof(char),strlen(obj->reference),debug);

	
	

	// Dictionary
	if(obj->dico != NULL){
		fputc('\n',debug);
		fputc('\n',debug);
		fputc('\n',debug);
		fwrite(obj->dico,sizeof(char),strlen(obj->dico),debug);
	}
	

	
	// Type
	if(obj->type != NULL){
		fputc('\n',debug);
		fputc('\n',debug);
		fputc('\n',debug);
		fwrite(obj->type,sizeof(char),strlen(obj->type),debug);
	}
	



	// Filters
	if(obj->filters != NULL){
		fputc('\n',debug);
		fputc('\n',debug);
		fputc('\n',debug);
		fwrite(obj->filters,sizeof(char),strlen(obj->filters),debug);
	}

	

	// Stream 
	if(obj->stream != NULL){
		fputc('\n',debug);
		fputc('\n',debug);
		fputc('\n',debug);
		fwrite(obj->stream, sizeof(char),obj->stream_size,debug);
	}
		
	//printf("\n\n::: Object :::\n\n");


	fputc('\n',debug);
	fputc('\n',debug);
	fputc('\n',debug);

	// Decoded Stream 
	if(obj->decoded_stream != NULL){
		printf("------------------\n");
		fwrite(obj->decoded_stream, sizeof(char),obj->decoded_stream_size,debug);	
	}
	
	//printf("Reference = %s\n",);


	fclose(debug);
	debug = NULL;

	return;
}


// Print object in a debug file (debug.txt)
void printObject(struct pdfObject * obj){

	
	if(obj == NULL){
		return ;
	}



	// Reference
	printf("\n\nObject :: %s\n", obj->reference);
	

	// Dictionary
	if(obj->dico != NULL)
		printf("\tDictionary = %s\n",obj->dico);
	

	
	// Type
	if(obj->type != NULL)
		printf("\tType = %s\n",obj->type);



	// Filters
	if(obj->filters != NULL)
		printf("\tFilters = %s\n",obj->filters);
	

	// Stream 
	if(obj->stream != NULL){
		printf("\tStream = %s\n", obj->stream); // Print in debug file
		printf("\tStream size = %d\n",obj->stream_size);
	}
	
	if(obj->decoded_stream != NULL){
		printf("\tDecoded Stream = %s\n", obj->decoded_stream);	// Print in debug file
		printf("\tDecoded Stream size = %d\n",obj->decoded_stream_size);
	}
		

	return;
}


void printObjectByRef(struct pdfDocument * pdf, char * ref){

	struct pdfObject * obj = NULL;


	if(pdf == NULL || ref == NULL ){
		err_log("printObjectByRef :: invalid parameter\n");
		return;
	}

	obj = pdf->objects;
	
	while(obj != NULL){
	
		if( strncmp(ref,obj->reference,strlen(ref)) == 0 ){
			printObject(obj);
			return;
		}
		
		obj = obj->next;	
	}

	return;


}


/*
printPDFObjects() :: Prints all object described in the PDF Document
parameters:
- struct pdfDocument * pdf (pdf document structure)
returns: (void)
- none.
*/
void printPDFObjects(struct pdfDocument * pdf){


	struct pdfObject * obj =  NULL;

	if(pdf == NULL || pdf->objects == NULL)
		return;

	printf("\n::: Objects Lists :::\n");

	obj = pdf->objects;

	while(obj){

		printObject(obj);
		obj = obj->next;
		printf("------------------------------------\n");
		printf("------------------------------------\n\n");
	}

	return;


}


/*
getNumber() :: Return a number (int) in a string or stream at a given pointer
parameters:
- char * ptr (the pointer of the string)
- int size (the size of the string)
returns: (int)
- A digit number.
- An error code (<0) on error.
*/
int getNumber(char* ptr, int size){

	int num;
	char * num_a = NULL;
	char * end = NULL;
	int len = 0;

	end = ptr;

	if (ptr == NULL || size <= 0){
		err_log("getNumber :: invalid parameters\n");
		return -1;
	}

	while( (end[0] >= 48 && end[0] <= 57) &&  len < size ){
		len ++;
		end ++;
	}

	if(len == 0){
		return -1;
	}

	num_a = (char*)calloc(len+1,sizeof(char));
	num_a[len]='\0';
	memcpy(num_a,ptr,len);

	num = atoi(num_a);
	free(num_a);
	num_a = NULL;

	if (num < 0)
		return -1;

	return num;
}


/*
getNumber_s() :: Return a number (in ascii string) in a string or stream at a given pointer
parameters:
- char * ptr (the pointer of the string)
- int size (the size of the string)
returns: (char *)
- A digit string.
- NULL on error.
*/
char* getNumber_s(char* ptr, int size){
	
	char * num_a = NULL;
	char * end = NULL;
	int len = 0;

	if (ptr == NULL || size <= 0) {
		err_log("getNumber_s :: invalid parameters\n");
		return NULL;
	}

	end = ptr;

	while( len < size && (end[0] >= 48 && end[0] <= 57)  ){
		len ++;
		end ++;
	}

	if(len == 0){
		return NULL;
	}

	num_a = (char*)calloc(len+1,sizeof(char));
	num_a[len]='\0';
	memcpy(num_a,ptr,len);

	return num_a;

}


/*
getIndirectRef() :: Get the indirect reference string at a given pointer
parameters:
- char * ptr (the pointer of the string)
- int size (the size of the string)
returns: (char *)
- the indirect reference string. (Ex: 1 0 R)
- NULL on error.
*/
char * getIndirectRef(char * ptr, int size){

	char * ref = NULL;
	char * obj_num = NULL; // object number
	char * gen_num = NULL; // generation number	
	char * end = NULL;
	int len = 0;

	if (ptr == NULL || size <= 0) {
		err_log("getIndirectRef :: invalid parameters\n");
		return NULL;
	}

	end = ptr;
	len = size;

	if(size < 5){
		return NULL;
	}

	// Get the object number
	if ((obj_num = getNumber_s(end, len)) == NULL)
		return NULL;

	end += strlen(obj_num);
	len -=  strlen(obj_num);

	// Move ptr for white space
	end ++ ;

	gen_num = getNumber_s(end,len);
	if(gen_num == NULL)
		return NULL;

	end += strlen(gen_num);
	len -=  strlen(gen_num);


	// Move ptr for white space
	end ++ ;
	
	// Check the presence of R => 12 0 R 
	if(end[0] != 'R'){
		return NULL;
	}

	len = strlen(obj_num) + strlen(gen_num) + 5 ;
	ref = (char*)calloc(len+1,sizeof(char));
	ref[len] = '\0';

	os_strncat(ref,len+1, obj_num, strlen(obj_num));
	os_strncat(ref,len+1, " ", strlen(obj_num));
	os_strncat(ref,len+1, gen_num, strlen(gen_num));
	os_strncat(ref,len+1, " obj", 4);

	free(gen_num);
	free(obj_num);

	return ref;

}


/*
getDelimitedStringContent() :: get a string delimited by a given character/pattern (take into account sub delimiters) Ex : << foo << bar >> >>
parameters:
- char * src
- char * delimiter1
- char * delimiter2
- int src_len
returns: (char *)
- string between delimiters
- NULL on error.
*/
char * getDelimitedStringContent(char * src, char * delimiter1, char * delimiter2, int src_len){

	char * content = NULL;
	int sub = 1;
	char * start = NULL;
	char * end = NULL;
	int len = 0;
	int lim = src_len;
	char * tmp = NULL;
	char * tmp2 = NULL;
	char * echap = NULL; // bug fix when Ex: (string = "parenthesis =\) " )  ;where delimiters are "(" and ")"	
	//int found = NULL;

	if (src == NULL || src_len <= 0 || delimiter1 == NULL || delimiter2 == NULL){
		err_log("getDelimitedStringContent :: invalid parameters\n");
		return NULL;
	}

	tmp = (char*)calloc(strlen(delimiter1) +1,sizeof(char));
	tmp2 = (char*)calloc(strlen(delimiter2) +1,sizeof(char));

	tmp[strlen(delimiter1)] = '\0';
	tmp2[strlen(delimiter2)] = '\0';


	start = src;


	memcpy(tmp,start,strlen(delimiter1));

	// find start point
	while (memcmp(tmp, delimiter1, strlen(delimiter1)) != 0 && lim > 0){

		start ++;
		lim--;
		if (lim < 0)
			memcpy(tmp,start,strlen(delimiter1));

	}

	if (lim <= 0){
		free(tmp);
		free(tmp2);
		return NULL;
	}
		

	len = (int)(start - src);

	end = start + strlen(delimiter1);

	memcpy(tmp2,start,strlen(delimiter2));

	
	while( sub > 0  && len <= src_len-2){ // TODO :: why? src_len-2 or src_len -1;

		memcpy(tmp,end,strlen(delimiter1));
		memcpy(tmp2,end,strlen(delimiter2));
		echap = end-1;


		if( memcmp(tmp,delimiter1,strlen(delimiter1)) == 0 && echap[0]!='\\'){

			sub ++;
			end += strlen(delimiter1);
			len += strlen(delimiter1);
		}else{

			if( memcmp(tmp2,delimiter2,strlen(delimiter2)) == 0 && echap[0]!='\\'){

				sub --;
				end += strlen(delimiter2);
				len += strlen(delimiter2);

			}else{
				end ++;
				len++;
			}
		}

	}
	
	
	if( sub > 0){
		
		warn_log("getDelimitedStringContent :: Odd number of delimiters :: %d :: src = %s :: delimiter1 = %s :: delimiter2 = %s\n",sub,src,delimiter1,delimiter2);
		
		free(tmp);
		free(tmp2);
		tmp = NULL;
		tmp2 = NULL;
		return NULL;
	}
	

	if(len > src_len){

		err_log("getDelimitedStringContent :: delimiter2 (%s) not found :: len > src_len\n", delimiter2);		

		free(tmp);
		free(tmp2);
		tmp = NULL;
		tmp2 = NULL;
		return NULL;
	}

	len = (int)(end - start);
	
	content = (char*)calloc(len+1,sizeof(char));
	content[len] = '\0';

	memcpy(content,start,len);

	free(tmp);
	free(tmp2);
	tmp = NULL;
	tmp2 = NULL;

	return content;
}


/*
getIndirectRefInString() :: search an object indirect reference in a string starting in "ptr"
parameters:
- char * ptr
- int size
returns: (char *)
- string between delimiters
- NULL on error.
*/
char * getIndirectRefInString(char * ptr, int size){

	char * ref = NULL;
	char * start = NULL;
	int len = 0;

	if( ptr == NULL || size <= 0){
		err_log("getIndirectRefInString :: invalid parameter!\n");
		return NULL;
	}

	start = ptr;
	len = size;

	while(ref == NULL && len >= 5 ){

		ref = getIndirectRef(start, len);
		start ++;
		len --;

	}

	return ref;
}


// get a pattern of length (size)  in ptr and skip white spaces
char * getPattern(char * ptr, int size, int len){

	char * pattern = NULL;
	int i = 0;
	//int white_spaces = 0;
	//int tmp = 0;
	//int tmp_len = 0;


	if(len < size){
		return NULL;
	}

	//tmp = len;

	pattern = (char*)calloc(size+1,sizeof(char));
	pattern[size]='\0';

	for(i=0; i< size ; i++){

		/*
		// Skip white spaces
		while(ptr[0] == '\n' || ptr[0] == '\r' || ptr[0] == ' '){
			ptr ++;
			white_spaces ++;
			len--;
			if( (size - i)  > len )
				return NULL;
		}*/

		/*
		len --;
		if( (size - i)  > len )
				return NULL;
		*/

		pattern[i] = ptr[0];

		ptr++;
	}


	return pattern;

}


/*
getUnicodeInString() :: Return the first unicode string if present in the stream given in parameters
parameters:
- char * stream
- int size
returns: (char*)
- the unicode string if found.
- NULL if not found or on error.
*/
char * getUnicodeInString(char * stream, int size){

	char * unicode = NULL;
	char * start = NULL;
	char * end = NULL;
	int len = 0;

	if (stream  == NULL || size <= 0) {
		err_log("getUnicodeInString :: invalid parameters\n");
		return NULL;
	}
	
	len = size ;
	end = stream;


	while( unicode == NULL && len > 6){

		start = searchPattern(end, "%u", 2, len);
		if(start == NULL){
			//printf("No unicode detected\n");
			return NULL;
		}

		end = start +2 ;

		len = 0;
		while( ((end[0] >= 65 && end[0] <=70) || (end[0] >= 97 && end[0] <= 102) || (end[0] >= 48 && end[0] <= 57)) && len != 4 ){
			len ++;
			end ++;
		}

		if(len == 4){			
			unicode = start;
			return unicode;
		}


		len = (int)(start - stream);
		len = size - len;

		start ++;

	}

	return NULL;
}


/*
replaceInString() ::  replace all occurrences of the pattern in the stream by another pattern
parameters:
- char * src (the source entry).
- char * toReplace (the string to replace).
- char * pat (the pattern which replace the string).
returns: (char*)
- the new string with the pattern replaced.
- NULL if not found or on error.
// TODO :: replaceString :: replace all occurrences. :: in function replaceAll.
*/
char * replaceInString(char * src, char * toReplace , char * pat){

	char * dest = NULL;
	char * start = NULL;
	char * end = NULL;
	int len = 0;
	int len_alloc = 0;
	int off = 0;

	if (src == NULL || toReplace == NULL || pat == NULL){
		err_log("replaceInString :: invalid parameter\n");
		return NULL;
	}

	// TODO: calc the number of occurrencies of the pattern to replace

	// get the positions
	start = searchPattern(src,toReplace,strlen(toReplace),strlen(src));

	if(start == NULL){
		err_log("String to replace (%s) not found in src \n",toReplace);
		return src;
	}


	// calc the new length = len - diff(pat et pat2)
	len = strlen(src) - (strlen(toReplace) - strlen(pat));
	len_alloc = len;

	dest = (char*)calloc(len+1,sizeof(char));
	dest[len] = '\0';


	// get the position
	off = (int)(start - src);

	memcpy(dest, src, off);

	// replace
	os_strncat(dest,len_alloc+1,pat,strlen(pat));

	end = start + strlen(toReplace);

	len = strlen(src) - off - strlen(toReplace);
	
	os_strncat(dest,len_alloc+1,end,len);
	
	return dest;
}


/*
getHexa() ::  return a pointer to the first hexa string (#F6) or NULL if any;
parameters:
- char * dico (object dictionnary).
- int size, the size of the dico
returns: (char*)
- the hex string found
- NULL if not found or on error.
*/
char * getHexa(char * dico, int size){

	char *  start = NULL;
	char * end = NULL;
	char * hexa = NULL;
	int len = 0;

	len = size ;
	start = dico;
	end = dico;

	if (dico == NULL || size <= 0){
		err_log("getHexa :: invalid parameter\n");
		return NULL;
	}

	while( hexa == NULL && len >= 3  ){

		start = searchPattern(end,"#",1,len);		
		if(start == NULL){
			return NULL;
		}

		end = start +1 ;

		// test the two next characters
		if( ((end[0] >= 65 && end[0] <=70) || (end[0] >= 97 && end[0] <= 102) || (end[0] >= 48 && end[0] <= 57)) && ((end[1] >= 65 && end[1] <=70) || (end[1] >= 97 && end[1] <= 102) || (end[1] >= 48 && end[1] <= 57)) ){
			//dbg_log("getHexa :: hex found\n");
			return start;
		}		

		len = (int)(end - dico);
		len = size - len;


	}


	return NULL;
}

// print objects references
void printObjectReferences(struct pdfDocument* pdf){

	if(pdf->objects == NULL)
		return;


	while(pdf->objects != NULL){
		dbg_log("object = %s\n",pdf->objects->reference);

		pdf->objects = pdf->objects->next;
	}

	return;

}


void debugPrint(char * stream, int len){

	FILE * debug = NULL;

	if(stream == NULL || len <= 0){
		err_log("debugPrint :: invalid parameter\n");
		return;
	}

	// Open en file
	if(!(debug = os_fopen("debug","wb+"))){
		err_log("debugPrint :: open failed\n");
		return ;
	}

	// Reference
	fwrite(stream,sizeof(char),len,debug);


	fclose(debug);
	debug = NULL;


	return;
}


// This function convert a string into binary.
char * toBinary(char * stream, int size){


	char * binary = NULL;
	int len = 0;
	//char * byte = NULL;
	int bit = 0;
	int i = 0, j = 0;
	char bit_s = 0;
	int off = 0;



	len = 8*size;

	binary = (char*)calloc(len+1,sizeof(char));
	binary[len] = '\0';


	len = 0;
	for(i = 0; i < size; i++){

		//printf("stream[i] = %c\n",stream[i]);

		for(j = 0; j < 8; j++){

			bit = stream[i] & (1 << (7-j));
			bit = bit >> (7-j);
			//printf("%d ",bit);

			//bit_s = bit + ()
			bit_s = bit - '\0' + 48;
			//off = i*10+j;

			binary[off] = bit_s;
			off ++;
		}
		//byte = strtol(stream[i],NULL,2) = ;
		
		//printf("\n\n");

	}


	//printf("binary = %s\n",binary);


	return binary;

}


// Converts a binary string to a char string
char * binarytoChar(char * binary, int size, int * returned_size){

	char * string = NULL;
	int i =0,j=0;
	int len = 0;
	int off = 0;
	char * byte = NULL;
	char res = 0;
	int mod = 0;


	len = size/8;
	mod = size%8;
	if(mod != 0){
		warn_log("binarytoChar :: len not a multiple of 8 :: padding with zero :: size %d :: len = %d :: mod8 = %d\n",size,len,mod);
		//TODO Padd with 0
	}

	byte = (char*)calloc(9,sizeof(char));
	byte[8]='\0';

	//printf("len = %d :: size = %d\n",len, size);
	string = (char*)calloc(len+1,sizeof(char));
	string[len] = '\0';

	for(i= 0; i<len; i++){

		for(j=0;j<8;j++){		
			//printf("%c",binary[j]);
			byte[j]=binary[j];
		}
		res = strtol(byte,NULL,2);
		//printf("%s ==> %c\n\n",byte,res);
		string[off] = res;
		off ++;
		binary+=8;

	}

	*returned_size = len;

	//printf("string = %s\n",string);

	free(byte);

	return string;
}


// This function print a stream with a null characters
void printStream(char * stream, int size){

	int len = 0;
	int rsize = 0;
	int nul = 0;
	char * ptr = NULL;

	if(stream == NULL || size <= 0){
		return;
	}

	ptr = stream;

	while (len < size){

		if(ptr[0] == '\0'){
			printf("<NUL>");
			nul ++;

		}else{
			printf("%c",ptr[0]);
			nul = 0;
		}

		len ++;
		ptr ++;

		if (nul >= 5){
			dbg_log("printStream :: len = %d\n",len);
			return;
		}
		

	}

	printf("\n");

	return ;
	


}