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
	obj->decoded_stream_size = 0;
	
		
	
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
		printf("Error :: searchPattern :: Bad arguments\n");
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



// Print object in a debug file (debug.txt)
void printObject(struct pdfObject * obj){


	FILE * debug = NULL;

	// Open en file
	if(!(debug = fopen("debug.txt","wb+"))){
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
		fwrite(obj->decoded_stream, sizeof(char),obj->decoded_stream_size,debug);	
	}
	
	

	




	//printf("Reference = %s\n",);


	fclose(debug);
	debug = NULL;

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


	num_a = (char*)calloc(len,sizeof(char));
	memcpy(num_a,ptr,len);
	//printf("num_a = %s\n",num_a);

	num = atoi(num_a);
	//printf("num = %d\n",num);


	return num;
}

// This function return a number (in ascii string) in a string or stream at a given pointer
char* getNumber_a(char* ptr, int size){

	//int num;
	char * num_a = NULL;
	char * end = NULL;
	int len = 0;

	end = ptr;

	/*
	do{
		//printf("end[0] = %c\n",end[0]);
		len ++;
		end ++;
	}while( (end[0] >= 48 && end[0] <= 57) &&  len < size );
	*/

	while( (end[0] >= 48 && end[0] <= 57) &&  len < size ){
		len ++;
		end ++;
	}


	if(len == 0){
		return NULL;
	}



	num_a = (char*)calloc(len,sizeof(char));
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
	ref = (char*)calloc(len,sizeof(char));

	strncat(ref, obj_num, strlen(obj_num));
	strncat(ref, " ", strlen(obj_num));
	strncat(ref, gen_num, strlen(gen_num));
	strncat(ref, " obj", 4);
	//printf("ref = %s\n",ref);	

	return ref;

}

// this function get a string delimited by a given character/pattern (take into account sub delimiters) Ex : << foo << bar >> >>
char * getDelimitedStringContent(char * src, char * delimiter1, char * delimiter2, int src_len){

	char * content = NULL;
	int sub = 1;
	char * start = NULL;
	char * end = NULL;
	int len = 0;
	char * tmp = NULL;
	char * tmp2 = NULL;
	char * echap = NULL; // bug fix when Ex: (string = "parenthesis =\) " )  ;where delimiters are "(" and ")"

	tmp = (char*)calloc(strlen(delimiter1),sizeof(char));
	tmp2 = (char*)calloc(strlen(delimiter2),sizeof(char));


	start = src;
	//printf("start = %d\n",start);
	strncpy(tmp,start,strlen(delimiter1));

	// find start point
	while(strncmp(tmp,delimiter1,strlen(delimiter1)) != 0){

		start ++;
		strncpy(tmp,start,strlen(delimiter1));

	}
	//printf("start = %d\n",start);

	end = start + strlen(delimiter1);

	//free(tmp);
	//tmp = NULL;


	tmp2 = (char*)calloc(strlen(delimiter2),sizeof(char));
	strncpy(tmp2,start,strlen(delimiter2));


	//while( strncmp(tmp,delimiter2,strlen(delimiter2)) != 0  && sub = 0){
	while( sub > 0  && len <= src_len){


		strncpy(tmp,end,strlen(delimiter1));
		strncpy(tmp2,end,strlen(delimiter2));
		echap = end-1;


		if( strncmp(tmp,delimiter1,strlen(delimiter1)) == 0 && echap[0]!='\\'){
			//printf("clic\n");
			//printf("echap = %c\n",echap[0]);
			sub ++;
			end += strlen(delimiter1);
		}else{

			if( strncmp(tmp2,delimiter2,strlen(delimiter2)) == 0 && echap[0]!='\\'){
				//printf("clac\n");
				//printf("echap = %c\n",echap[0]);
				sub --;
				end += strlen(delimiter2);

			}else{
				end ++;
			}
		}


		//end ++;
		//len ++;


	}

	//end += (strlen(delimiter2)-1);

	//printf("end = %d :: %c\n",end,end[0]);
	//printf("len = %d\n",len);
	


	len = (int)(end - start);
	//printf("len = %d\n",len);

	if(len > src_len){
		printf("Error :: getDelimitedStringContent :: len > src_len\n");
		return NULL;
	}

	content = (char*)calloc(len,sizeof(char));

	strncpy(content,start,len);
	//printf("content = %s\n", );





	return content;
}

// This function get a indirect reference in a string starting in "ptr" :: return NULL if there is not
char * getIndirectRefInString(char * ptr, int size){

	char * ref = NULL;
	char * start = NULL;
	//char * end = NULL;
	int len = 0;

	if(size < 5){
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
	int white_spaces = 0;
	//int tmp = 0;
	//int tmp_len = 0;


	if(len < size){
		return NULL;
	}

	//tmp = len;

	pattern = (char*)calloc(size,sizeof(char));

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
	
	start = stream;
	len = size ;


	while( unicode == NULL && len > 6){

		start = searchPattern(stream, "%u", 2, len);
		

		if(start == NULL){
			//printf("No unicode detected\n");
			return NULL;
		}

		end = start +2 ;

		len = 0;
		while( (end[0] >= 65 && end[0] <=70) || (end[0] >= 97 && end[0] <= 102) || (end[0] >= 48 && end[0] <= 57) || len != 4 ){
			len ++;
			end ++ ;
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