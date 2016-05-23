/*  
	<ARMADITO PDF ANALYZER is a tool to parse and analyze PDF files in order to detect potentially dangerous contents.>
    Copyright (C) 2015 by Teclib' 
	<ufausther@teclib.com>
    
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

// This function free the memory allocated for an object structure
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

// This function free the memory allocated for a trailer structure
void freePDFTrailerStruct(struct pdfTrailer * trailer){

	struct pdfTrailer * tmp = NULL;

	if(trailer == NULL){
		return ;
	}

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

// This function free the memory allocated for the pdf structure
void freePDFDocumentStruct(struct pdfDocument * pdf){

	
	if(pdf == NULL){
		return ;
	}
	
	// Free objects
	if(pdf->objects != NULL)
		freePDFObjectStruct(pdf->objects);
	
	
	// Free trailer
	if(pdf->trailers != NULL)
		freePDFTrailerStruct(pdf->trailers);
	
	if (pdf->fh != NULL)
		fclose(pdf->fh);

	if (pdf->version != NULL)
		free(pdf->version);

	if (pdf->content != NULL)
		free(pdf->content);

	if (pdf->testStruct != NULL)
		free(pdf->testStruct);

	if (pdf->testObjAnalysis != NULL)
		free(pdf->testObjAnalysis);

	if (pdf != NULL)
		free(pdf);
	

	return ;

}


// return the object within the reference given in parameters
struct pdfObject * getPDFObjectByRef(struct pdfDocument * pdf, char * ref){

	struct pdfObject * obj = NULL;


	if(pdf == NULL ){

		#ifdef DEBUG
			printf("Error :: getPDFObjectByRef :: NULL parameter \"pdf\" \n");
		#endif

		return NULL;
	}

	if(ref == NULL ){
		#ifdef DEBUG
			printf("Error :: getPDFObjectByRef :: NULL parameter \"ref\" \n");
		#endif

		return NULL;
	}

	obj = pdf->objects;
	
	
	while(obj != NULL){
	
		if( strncmp(ref,obj->reference,strlen(ref)) == 0 ){
			return obj;
		}
		
		obj = obj->next;	
	}
	
	//printf("Object \"%s\" not found\n",ref);

	return NULL;
}

// return the object within the reference given in parameters (get the next object in the list, starting from obj, with the reference given in parameter)
struct pdfObject * getPDFNextObjectByRef(struct pdfDocument * pdf, struct pdfObject * obj, char * ref){

	struct pdfObject * tmp = NULL;


	if(pdf == NULL ){

		#ifdef DEBUG
			printf("Error :: getPDFObjectByRef :: NULL parameter \"pdf\" \n");
		#endif

		return NULL;
	}

	if(ref == NULL ){
		#ifdef DEBUG
			printf("Error :: getPDFObjectByRef :: NULL parameter \"ref\" \n");
		#endif

		return NULL;
	}

	if(obj == NULL){
		return NULL;
	}


	tmp = obj->next;

	while(tmp != NULL){
	
		if( strncmp(ref,tmp->reference,strlen(ref)) == 0 ){
			return tmp;
		}
		
		tmp = tmp->next;	
	}
	
	//printf("Object \"%s\" not found\n",ref);

	return NULL;
}


// This function add an object in the list
int addObjectInList(struct pdfObject* obj, struct pdfDocument* pdf){

	struct pdfObject* tmp = NULL;

	if(obj == NULL){
		#ifdef DEBUG
			printf("Error :: addObjectInList :: Object is NULL\n");
		#endif

		return -1;	
	}
	
	if(pdf->objects == NULL){
		pdf->objects = obj;
	}else{
		
		tmp = pdf->objects;

		// Object collision detection
		if(strncmp(tmp->reference,obj->reference,strlen(tmp->reference)) == 0 && strncmp(tmp->reference,obj->reference,strlen(obj->reference)) == 0){
			#ifdef DEBUG
				printf("Warning :: addObjectInList :: Object collision :: %s\n",obj->reference);
			#endif

			pdf->testStruct->object_collision ++;
		}


		while(tmp->next != NULL){

			// Object collision detection
			
			tmp = tmp->next;

			if(strncmp(tmp->reference,obj->reference,strlen(tmp->reference)) == 0 && strncmp(tmp->reference,obj->reference,strlen(obj->reference)) == 0){
				#ifdef DEBUG
					printf("Warning :: addObjectInList :: Object collision :: %s\n",obj->reference);
				#endif

				pdf->testStruct->object_collision ++;
			}	
		}
		tmp->next = obj;
		
	}
	
	return 0;
}



// Init 
struct testsPDFStruct * initTestsPDFStruct(){

	struct testsPDFStruct * testStruct = NULL;

	if( !(testStruct = (struct testsPDFStruct *)calloc(1,sizeof(struct testsPDFStruct)) ) ){
		printf("Error :: initTestsPDFStruct :: memory allocation1 failed\n");		
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

// Init 
struct testsPDFObjAnalysis * initTestsPDFObjAnalysisStruct(){

	struct testsPDFObjAnalysis * testObjAnalysis = NULL;

	if( !(testObjAnalysis = (struct testsPDFObjAnalysis *)calloc(1,sizeof(struct testsPDFObjAnalysis)) ) ){
		printf("Error :: initTestsPDFStruct :: memory allocation1 failed\n");
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


// Init a pdfDocument object structure
struct pdfDocument* initPDFDocument(){

	struct pdfDocument* pdf = NULL;
	
	if( (pdf = (struct pdfDocument *)calloc(1,sizeof(struct pdfDocument))) == NULL ){
		printf("Error :: initPDFDocument :: memory allocation failed\n");
		return NULL;
	}

	if( (pdf->testStruct = initTestsPDFStruct()) == NULL ){
		printf("Error :: initPDFDocument :: memory allocation1 failed\n");
		return NULL;
	}

	if( (pdf->testObjAnalysis = initTestsPDFObjAnalysisStruct()) == NULL ){
		printf("Error :: initPDFDocument :: memory allocation2 failed\n");
		return NULL;
	}


	
	// Initialize entries
	pdf->fh = NULL;
	pdf->fd = -1;
	pdf->content = NULL;
	pdf->objects =NULL;
	pdf->coef = 0;
	pdf->size = 0;
	pdf->version = NULL;
	pdf->trailers = NULL;
	pdf->xref = NULL;

	pdf->errors = 0;
	pdf->scan_time=0;
	return pdf;

}


// Init a pdfObject object structure
struct pdfObject* initPDFObject(){

	struct pdfObject* obj = NULL;
	
	
	if( !(obj = (struct pdfObject*)calloc(1,sizeof(struct pdfObject)) ) ){
		printf("Error :: initPDFDocument :: memory allocation failed\n");
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
	obj->content_size = 0;
	obj->decoded_stream_size = 0;
	obj->errors = 0;
	
	return obj;

}

// Init a pdfDocument object structure
struct pdfTrailer* initPDFTrailer(){

	struct pdfTrailer* trailer = NULL;
	
	if( !(trailer = (struct pdfTrailer *)calloc(1,sizeof(struct pdfTrailer)) ) ){
		printf("Error :: initPDFTrailer :: memory allocation failed\n");
		return NULL;
	}
	
	// Initialize entries
	trailer->offset = 0;
	trailer->content = NULL;
	trailer->dico = NULL;
	trailer->next = NULL;
	
	return trailer;

}

// This function search a pattern in a stream
void * searchPattern(char* src, char* pat , int pat_size ,  int size){

	char* res = NULL;
	
	//int i =0;
	char * tmp = NULL;
	//int diff = 0;
	char * end = NULL;
	int len = 0;
	int len_verif = 0;
	
	
	
	//printf("src = %s\n",src);
	//printf("pat = %s\n",pat);
	//printf("hey! %d :: %d\n",src, size);
	if( size < pat_size || src == NULL || pat == NULL || pat_size == 0 || size == 0){
		//printf("Error :: searchPattern :: Bad arguments\n");
		return NULL;
	}
	
	//tmp =  (char*)malloc(pat_size*sizeof(char));
	tmp =  (char*)calloc(pat_size+1,sizeof(char));
	tmp[pat_size] = '\0';
	//printf("hey2!\n");
	/*
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
		
		//diff = (int)(res - src);
		//i+=diff;
		i=(int)(res-src);

		src = res + 1;

	}*/


	len = size; 
	end = src;
	while(len >= pat_size){
	
		//i = 0;
		res = memchr(end,pat[0],len);
		//printf("res = %s\n",res);
		if(res == NULL){
			free(tmp);
			//printf("Not found\n");
			return NULL;
		}


		len_verif = (int)(res-end);
		len_verif = len - len_verif;

		if(len_verif < pat_size){
			free(tmp);
			return NULL;
		}

		//printf("src  = %d && res = %d\n",src,res);
		//i++;
		

		memcpy(tmp,res,pat_size);
		
		if( memcmp(tmp,pat,pat_size) == 0){
			free(tmp);
			return res;
		}
		
		//diff = (int)(res - src);
		//i+=diff;

		end = res +1;

		len=(int)(end-src);
		len = size - len;

	}
	
	free(tmp);
	
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


	if(pdf == NULL ){
		printf("Error :: printObjectByRef :: NULL parameter \"pdf\" \n");
		return;
	}

	if(ref == NULL ){
		printf("Error :: printObjectByRef :: NULL parameter \"ref\" \n");
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
	
	//printf("Object \"%s\" not found\n",ref);

	return;


}




// Prints all object described in the PDF Document
void printPDFObjects(struct pdfDocument * pdf){


	struct pdfObject * obj =  NULL;

	if(pdf->objects == NULL)
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


// This function return a number in a string or stream at a given pointer
int getNumber(char* ptr, int size){

	int num;
	char * num_a;
	char * end;
	int len = 0;

	end = ptr;

	while( (end[0] >= 48 && end[0] <= 57) &&  len < size ){
		len ++;
		end ++;
	}

	if(len == 0){
		return -1;
	}

	/*
	do{
		//printf("end[0] = %c\n",end[0]);
		len ++;
		end ++;
	}while( (end[0] >= 48 && end[0] <= 57) || len >= size );
	*/


	num_a = (char*)calloc(len+1,sizeof(char));
	num_a[len]='\0';
	memcpy(num_a,ptr,len);
	//printf("num_a = %s\n",num_a);

	num = atoi(num_a);
	//printf("num = %d\n",num);
	free(num_a);

	return num;
}

// This function return a number (in ascii string) in a string or stream at a given pointer
char* getNumber_a(char* ptr, int size){

	//int num;
	char * num_a = NULL;
	char * end = NULL;
	int len = 0;

	if (ptr == NULL || size <= 0) {
		printf("[-] Error :: getNumber_a :: invalid parameters!\n");
		return NULL;
	}

	end = ptr;

	/*
	do{
		//printf("end[0] = %c\n",end[0]);
		len ++;
		end ++;
	}while( (end[0] >= 48 && end[0] <= 57) &&  len < size );
	*/

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

	//num = atoi(num_a);
	//printf("num = %d\n",num);


	return num_a;

}

// This function get the indirect reference in a string at a given pointer
char * getIndirectRef(char * ptr, int size){

	char * ref = NULL;
	char * obj_num = NULL; // object number
	char * gen_num = NULL; // generation number

	//char * start = NULL;
	char * end = NULL;
	int len = 0;


	end = ptr;
	len = size;

	if(size < 5){
		return NULL;
	}

	// Get the object number
	obj_num = getNumber_a(end,len);
	

	if(obj_num == NULL)
		return NULL;

	//printf("obj_num = %s\n",obj_num);

	end += strlen(obj_num);
	len -=  strlen(obj_num);

	// Move ptr for white space
	end ++ ;

	//printf("end[0] = %c\n",end[0]);

	gen_num = getNumber_a(end,len);
	if(gen_num == NULL)
		return NULL;
	//printf("gen_num = %s\n",gen_num);

	end += strlen(gen_num);
	len -=  strlen(gen_num);


	// Move ptr for white space
	end ++ ;
	
	// Check the presence of R => 12 0 R 
	if(end[0] != 'R'){
		return NULL;
	}
	//printf("end[0] = %c\n",end[0]);

	len = strlen(obj_num) + strlen(gen_num) + 5 ;
	ref = (char*)calloc(len+1,sizeof(char));
	ref[len] = '\0';

	os_strncat(ref,len+1, obj_num, strlen(obj_num));
	os_strncat(ref,len+1, " ", strlen(obj_num));
	os_strncat(ref,len+1, gen_num, strlen(gen_num));
	os_strncat(ref,len+1, " obj", 4);
	//printf("ref = %s\n",ref);	

	free(gen_num);
	free(obj_num);

	return ref;

}

// this function get a string delimited by a given character/pattern (take into account sub delimiters) Ex : << foo << bar >> >>
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

	if (src == NULL || src_len <= 0){
		printf("[-] Error :: getDelimitedStringContent :: bad parameters\n");
		return NULL;
	}

	tmp = (char*)calloc(strlen(delimiter1) +1,sizeof(char));
	tmp2 = (char*)calloc(strlen(delimiter2) +1,sizeof(char));

	tmp[strlen(delimiter1)] = '\0';
	tmp2[strlen(delimiter2)] = '\0';


	start = src;
	//printf("start = %d\n",start);
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
	//printf("start = %d\n",start);

	end = start + strlen(delimiter1);


	//free(tmp);
	//tmp = NULL;


	//tmp2 = (char*)calloc(strlen(delimiter2),sizeof(char));
	memcpy(tmp2,start,strlen(delimiter2));


	//while( strncmp(tmp,delimiter2,strlen(delimiter2)) != 0  && sub = 0){
	while( sub > 0  && len <= src_len-2){ // TODO :: why? src_len-2 or src_len -1;

		//printf("hey\n");
		memcpy(tmp,end,strlen(delimiter1));
		memcpy(tmp2,end,strlen(delimiter2));
		echap = end-1;


		if( memcmp(tmp,delimiter1,strlen(delimiter1)) == 0 && echap[0]!='\\'){
			//printf("clic\n");
			//printf("echap = %c\n",echap[0]);
			sub ++;
			end += strlen(delimiter1);
			len += strlen(delimiter1);
		}else{

			if( memcmp(tmp2,delimiter2,strlen(delimiter2)) == 0 && echap[0]!='\\'){
				//printf("clac\n");
				//printf("echap = %c\n",echap[0]);
				sub --;
				end += strlen(delimiter2);
				len += strlen(delimiter2);

			}else{
				end ++;
				len++;
			}
		}

		//end ++;
		//len ++;
	}



	//end += (strlen(delimiter2)-1);

	//printf("end = %d :: %c\n",end,end[0]);
	//printf("len = %d\n",len);
	
	
	if( sub > 0){
		#ifdef DEBUG
		printf("Warning :: getDelimitedStringContent :: Odd number of delimiters :: %d :: src = %s :: delimiter1 = %s :: delimiter2 = %s\n",sub,src,delimiter1,delimiter2);
		#endif

		free(tmp);
		free(tmp2);
		tmp = NULL;
		tmp2 = NULL;
		return NULL;
	}
	

	if(len > src_len){
		#ifdef DEBUG
		printf("Error :: getDelimitedStringContent :: delimiter2 not found :: len > src_len\n");
		#endif

		free(tmp);
		free(tmp2);
		tmp = NULL;
		tmp2 = NULL;
		return NULL;
	}
	//printf("len = %d\n",len);

	//printf("hey \n");
	len = (int)(end - start);
	
	content = (char*)calloc(len+1,sizeof(char));
	content[len] = '\0';

	memcpy(content,start,len);
	//printf("content = %s\n", );


	free(tmp);
	free(tmp2);
	tmp = NULL;
	tmp2 = NULL;


	return content;
}

// This function get a indirect reference in a string starting in "ptr" :: return NULL if there is not
char * getIndirectRefInString(char * ptr, int size){

	char * ref = NULL;
	char * start = NULL;
	//char * end = NULL;
	int len = 0;

	if( ptr == NULL){
		printf("[-] Error :: getIndirectRefInString :: invalid parameter!\n");
		return NULL;
	}

	start = ptr;
	len = size;
	//printf("len = %d\n",len);

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


// This function return the first unicode string if present in the stream given in parameters
char * getUnicodeInString(char * stream, int size){

	char * unicode = NULL;
	char * start = NULL;
	char * end = NULL;
	int len = 0;
	
	//start = stream;
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

// This function replace all occurrences of the pattern in the stream, returns the new string with a new length.
// TODO improve by replacing all occurrences
char * replaceInString(char * src, char * toReplace , char * pat){

	char * dest = NULL;
	char * start = NULL;
	char * end = NULL;
	int len = 0;
	int len_alloc = 0;
	int off = 0;

	if(src == NULL || toReplace == NULL || pat == NULL)
		return NULL;


	//printf("\nreplace in string\n");
	//printf("src = %s\n",src );

	// calc the number of occurrencies of the pattern to replace

	// get the positions
	start = searchPattern(src,toReplace,strlen(toReplace),strlen(src));

	if(start == NULL){
		#ifdef DEBUG
		printf("Error :: String to replace (%s) not found in src \n",toReplace);
		#endif

		return src;
	}


	// calc the new length = len - diff(pat et pat2)
	len = strlen(src) - (strlen(toReplace) - strlen(pat));
	len_alloc = len;

	//printf("src_len = %d :: len = %d\n",strlen(src),len);

	dest = (char*)calloc(len+1,sizeof(char));
	dest[len] = '\0';


	// get the position
	off = (int)(start - src);
	//printf("off = %d\n",off);


	memcpy(dest, src, off);
	//strncpy(dest, src, off-1);
	//printf("dest = %s\n",dest);

	// replace
	os_strncat(dest,len_alloc+1,pat,strlen(pat));
	//printf("dest = %s\n",dest);

	end = start + strlen(toReplace);
	//printf("end0 = %c\n",end[0]);



	//len = (int)(end - src);
	//len = strlen(src) - len ;
	//printf("len = %d\n",len);

	len = strlen(src) - off - strlen(toReplace);
	//printf("off = %d\n",off);
	//printf("Debug :: len = %d :: len")
	os_strncat(dest,len_alloc+1,end,len);
	//printf("dest = %s\n",dest);	


	return dest;
}


//  this function return a pointer to the first hexa string (#F6) or NULL not;
char * getHexa(char * dico, int size){

	char *  start = NULL;
	char * end = NULL;
	char * hexa = NULL;
	int len = 0;

	len = size ;
	start = dico;
	end = dico;

	while( hexa == NULL && len >= 3  ){

		//printf("heyhey :: end = %s :: len = %d :: size =  %d \n",end,len,size);
		
		start = searchPattern(end,"#",1,len);

		//printf("oooh\n");
		
		if(start == NULL){
			return NULL;
		}

		end = start +1 ;

		//len = 0;
		//printf("end0 =  %c :: end1 =  %c\n",end[0],end[1]);

		// test the two next characters
		if( ((end[0] >= 65 && end[0] <=70) || (end[0] >= 97 && end[0] <= 102) || (end[0] >= 48 && end[0] <= 57)) && ((end[1] >= 65 && end[1] <=70) || (end[1] >= 97 && end[1] <= 102) || (end[1] >= 48 && end[1] <= 57)) ){
			//printf("hex found\n");
			return start;
		}

		//start ++;

		len = (int)(end - dico);
		len = size - len;


	}


	return NULL;
}

// add a trailer in the list of trailers
int addTrailerInList(struct pdfDocument * pdf, struct pdfTrailer * trailer){

	struct pdfTrailer * tmp =  NULL;


	if(pdf == NULL || trailer == NULL){		
		printf("Error :: addTrailerInPDF :: Bad arguments pdf and trailer \n");

		return -1;
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

	return 0;
}

void printObjectReferences(struct pdfDocument* pdf){

	if(pdf->objects == NULL)
		return;


	while(pdf->objects != NULL){
		printf("object = %s\n",pdf->objects->reference);

		pdf->objects = pdf->objects->next;
	}

	return;

}

void debugPrint(char * stream, int len){

	FILE * debug = NULL;

	if(stream == NULL || len <= 0){
		printf("Error :: debugPrint :: NULL parameters \n");

		return;
	}

	// Open en file
	if(!(debug = os_fopen("debug","wb+"))){
		printf("open failed\n");
		return ;
	}

	//printf("DEBUG ::: \n");

	//printf("stream in debug = %s\n",stream);
	//fputc('\n',debug);
	//fputc('\n',debug);
	//fputc('\n',debug);
	//fwrite("---------------------------------------------",sizeof(char),45,debug);
	//fputc('\n',debug);

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
		printf("Warning :: binarytoChar :: len not a multiple of 8 :: padding with zero :: size %d :: len = %d :: mod8 = %d\n",size,len,mod);
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
	char * ptr = NULL;

	if(stream == NULL || size <= 0){
		return;
	}

	ptr = stream;

	while (len < size){

		if(ptr[0] == '\0'){
			printf("<NUL>");
		}else{
			printf("%c",ptr[0]);
		}

		len ++;
		ptr ++;

	}

	printf("\n");

	return ;
	


}