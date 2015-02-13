#include "pdfAnalyzer.h"


// This function checks if the header is correct
int checkMagicNumber(struct pdfDocument * pdf){
	
	
	int version_size = 8;
	//int ret;
	
	char * version;
	version = (char*)malloc(8*sizeof(char));
	
	
	// Go to the beginning of the file
	fseek(pdf->fh,0,SEEK_SET);
	
	
	// Read the 8 first bytes Ex: %PDF-1.
	fread(version,1,version_size,pdf->fh);
	
	//printf("pdf header = %s\n",version);
	
	if( strncmp(version,"%PDF-1.1",8) == 0 || strncmp(version,"%PDF-1.2",8) == 0 || strncmp(version,"%PDF-1.3",8) == 0 || strncmp(version,"%PDF-1.4",8) == 0 || strncmp(version,"%PDF-1.5",8) == 0 || strncmp(version,"%PDF-1.6",8) == 0 || strncmp(version,"%PDF-1.7",8) == 0 ){
	
		printf ("PDF Header OK = %s\n",version);
		pdf->version = version;
	
	}else{
		printf ("PDF Header KO : This document is not a PDF file.\n");
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
	
	// Get the size in byte
	fseek(pdf->fh,0,SEEK_END);
	doc_size = ftell(pdf->fh);
	
	printf("Document Size  = %d\n",doc_size);
	
	content = (char*)malloc(doc_size*sizeof(char));
	
	fseek(pdf->fh,0,SEEK_SET);
	
	read_bytes = fread(content,1,doc_size,pdf->fh);
	
	printf("read bytes = %d\n",read_bytes);
	
	//printf("Document content = %s",content);
	
	//read_bytes = strlen(content);
	//printf("read bytes = %d\n",read_bytes);
	
	pdf->content = content;
	
	pdf->size = read_bytes;
	
	return doc_size;
	
}


//This function get the object dictionary
char * getObjectDictionary(struct pdfObject * obj){
	
	char  * dico = NULL;
	char * content =  obj->content;
	char * dico_start = NULL;
	char * dico_end = NULL;
	int len = 0;
	
	
	
	//char* src, char* pat , int pat_size ,  int size
	
	// Search the beginning of the
	dico_start = searchPattern(content,"<<",2,obj->content_size);
	
	if(dico_start == NULL){
		//printf("No dictionary found in object %s!!\n", obj->reference);
		return NULL;
	}
	
	// TODO search other occurencies of "<<" to detect sub dictionaries
	// TODO Found the same number of occurencies of stream ">>"
	
	len = dico_start - obj->content;	
	
	dico_end = searchPattern(dico_start,">>",2,obj->content_size-len);
	
	if(dico_end == NULL){
		//printf("No dictionary found in object %s!!\n", obj->reference);
		return NULL;
	}
	
	len = dico_end - dico_start + 2; // +2 is for >>
	
	dico = (char*)malloc(len*sizeof(char));
	
	strncpy(dico,dico_start,len);
	
	// store the dictionary length
	//obj->dico_len = len;
	
	//printf("dictionary  = %s\n",dico);
	
	
	
	return dico;
	
}


char * getObjectType(struct pdfObject * obj){

	char * type = NULL;
	char * start = NULL;
	char * end = NULL;
	int len = 0;
	//char tmp = 0;
	//char *
	
	start = searchPattern(obj->dico,"/Type",5,strlen(obj->dico));
	
	if(start == NULL ){
		//printf("I got no type !!\n");
		return NULL;
	}
	
	len = (int)(start - obj->dico);
	//printf("len2 = %d :: %d :: %d\n",len, start, obj->dico);
	
	start += 1;
	//printf("len = %d \t start = %d \t obj->dico = %d\n",len, start,obj->dico);
	
	end = memchr(start,'/',strlen(obj->dico)-len);
	
	
	if(end == NULL){
		//printf("Err :: end = NULL\n");
		return NULL;
	}
	
	len = 0;
	do{
		//printf("char = %c\n",end[0]);
		end ++;
		len++;
		
	}while( (end[0] >=97 && end[0] <=122) || (end[0] >=65 && end[0] <=90 ) || (end[0] >=48 && end[0] <= 57) ); // Lower case [97 122] or Upper case [65 90] + digit [48 57]
	
	type = (char*)malloc(len*sizeof(char));
	//printf("len = %d \n",len);
	start += 4;
	strncpy(type,start,len);
	
	
	return type;
}


// This function get the stream content of an object (returns NULL if there is no stream)
char * getObjectStream(struct pdfObject * obj){

	char * stream = NULL;
	char * start = NULL;
	char * end = NULL;
	int len = 0;
	
	start = searchPattern(obj->content,"stream",6,obj->content_size);
	
	if(start == NULL){
		return NULL;
	}
	
	end = searchPattern(start,"endstream",9,obj->content_size);
	
	//len = (int)(end-start);
	
	// Remove white space after "stream" and before "endstream"
	start += 6;
	while(start[0] == '\n' || start[0] == '\r'){
		start ++;
	}
	
	/*end --;
	while(end[0] == '\n' || end[0] == '\r'){
		end --;
	}*/
	
	len = (int)(end-start);;
	
	stream = (char*)malloc(len*sizeof(char));
	
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
		
		end  = end_tmp;
		
		len = 0;
		do{
			//printf("char = %c\n",end[0]);
			end ++;
			len++;
		
		}while( (end[0] >=97 && end[0] <=122) || (end[0] >=65 && end[0] <=90 ) || (end[0] >=48 && end[0] <= 57) || (end[0] == ' ') || (end[0] == '/') ); // Lower case [97 122] or Upper case [65 90] + digit [48 57]
		
	}else{ // a single filter
	
		len = 0;
		do{
			//printf("char = %c\n",end[0]);
			end ++;
			len++;
		
		}while( (end[0] >=97 && end[0] <=122) || (end[0] >=65 && end[0] <=90 ) || (end[0] >=48 && end[0] <= 57) ); // Lower case [97 122] or Upper case [65 90] + digit [48 57]
		
	
	}
	
	
	filters = (char*)malloc(len*sizeof(char));
	//printf("len = %d \n",len);
	start += 6;
	strncpy(filters,start,len);
	
	printf("fitlers = %s \n",filters);
	
	
	return filters;
}



// This function decode an object stream according to the filters applied
int decodeObjectStream(struct pdfObject * obj){

	
	char * start = NULL;
	char * end = NULL;
	char * filter =NULL; 
	char * stream = NULL;
	int len = 0;
	int i = 0; 
	
	end = obj->filters;
	
	if(obj->filters == NULL){
		printf("Error :: There is no filter implemented\n");
		return -1;
	}
	
	//printf("implemented filters = %s\n",obj->filters);
	
	stream = obj->stream;
	
	while( (start = memchr(end, '/',strlen(obj->filters)-len)) != NULL){
		
		//filter ++;
		end = start;		
		// Scan the filter name
		do{
			end ++;
			i++;
			
		}while((end[0] >=97 && end[0] <=122) || (end[0] >=65 && end[0] <=90 ) || (end[0] >=48 && end[0] <= 57)  );
		
		len = (int)(end-start);
		
		filter = (char*)malloc(len*sizeof(char));
		strncpy(filter,start,len);
		printf("implemented filter_end = %s\n",filter);
		
		//len -= filter - strlen(obj->filter);
		
		
		// Apply decode filter
		
		//TODO
		if(strncmp(filter,"/FlateDecode",len) == 0){
				
		}
		
		//TODO
		if(strncmp(filter,"/ASCIIHexDecode",len) == 0){
			
		}
		
		//TODO
		if(strncmp(filter,"/ASCII85Decode",len) == 0){
			
		}
		
		//TODO
		if(strncmp(filter,"/LZWDecode",len) == 0){
			
		}
		
		//TODO
		if(strncmp(filter,"/RunLengthDecode",len) == 0){
						
		}
		
		//TODO
		if(strncmp(filter,"/CCITTFaxDecode",len) == 0){
			
		}
		
	}
	
	
	// Store the decoded stream
	if(stream != NULL){
		obj->decoded_stream = stream ;
	}
	
	
	return 0;
}



// Get object information (types, filters, streams, etc.)
int getObjectInfos(struct pdfObject * obj){

	char * dico = NULL;
	char * type = NULL;
	char * stream = NULL;
	char * filters = NULL;
	int res = 0;
	

	if(obj == NULL){
		printf("Error :: getObjectInfos :: null object\n");
		return -1;
	}
	
	// Get the dictionary
	dico = getObjectDictionary(obj);
	if(dico == NULL){
		return 0;
	}
	
	printf("dictionary = %s\n\n",dico);
	obj->dico = dico;
	
	// Get the type
	type = getObjectType(obj);
	if(type != NULL){
		printf("/Type = %s\n\n",type);
		obj->type = type;
	}
	
	// Get stream content
	stream  = getObjectStream(obj);
	if(stream != NULL){
		printf("stream = %s\n\n",stream);
		obj->stream = stream;
	}
	
	// Get stream filters
	filters = getStreamFilters(obj);
	
	if(filters != NULL){
		obj->filters = filters;
		res = decodeObjectStream(obj);
		printf("res = %d\n",res);
	}
	
		
	return 1;
	

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

	
	while( (startobj_ptr = strstr(endobj_ptr,"obj")) != NULL){
	
		//printf("search offset = %d\n",endobj_ptr);
		gen_num_len = 0;
		obj_num_len = 0;
		
		startobj_ptr -= 2; // point to the generation number
		//printf("Generation number = %c\n",startobj_ptr[0]);
		while(startobj_ptr[0] >= 48 && startobj_ptr[0] <= 57  ){
		
			//printf("Warning :: Treat this case 1 :: \n");
			
			startobj_ptr--;
			//printf("Warning :: Treat this case 1 :: %c\n", startobj_ptr[0] );
			gen_num_len++;
		}
	
		startobj_ptr -= 1; // point to the object number
		//printf("object number = %c\n",startobj_ptr[0]);
		while(startobj_ptr[0] >= 48 && startobj_ptr[0] <= 57  ){
		
			startobj_ptr--;
			//printf("Warning :: Treat this case 2 :: %c\n",startobj_ptr[0] );
			obj_num_len ++;
			
		}
		
		startobj_ptr++;
		
		
		//gen_num = (char*)malloc(gen_num_len*sizeof(char));
		
		//obj_num = (char*)malloc(obj_num_len*sizeof(char));
		
		ref_len = gen_num_len + 1 + obj_num_len + 1 + 3 ; // 1 = space and 3 = obj
		ref = (char*)malloc(ref_len*sizeof(char));
		
		strncpy(ref,startobj_ptr,ref_len);
		printf("object reference = %s :: %d\n",ref,ref_len);
		
		//startobj_ptr++;
		
		offset = (int)(startobj_ptr -pdf->content);
		
	
		printf("object offset = %d\n",offset);
		//printf("start obj ptr = %d\n",startobj_ptr);
		
		//endobj_ptr = strstr(startobj_ptr,"endobj");
		//printf("end obj ptr 1 = %d\n",endobj_ptr);
		tmp = (int)(pdf->size - (startobj_ptr - pdf->content));
		//printf("size of block = %d\n",tmp);
		endobj_ptr = searchPattern(startobj_ptr,"endobj",6,tmp);
		//printf("end obj ptr = %d\n",endobj_ptr);
		
		if(endobj_ptr == NULL){
			//printf(":: Error :: startobj_ptr = %d\n", startobj_ptr);
			return -1;
		}
		//printf("end obj ptr = %c\n",endobj_ptr[0]);
		
		
		endobj_ptr += 6;
		//printf("end obj ptr = %c\n",endobj_ptr[0]);
		
	
		len = (int)(endobj_ptr - startobj_ptr);
		printf("object len = %d\n",len);
	
		content = (char*)malloc(len*sizeof(char));
		
		memcpy (content, startobj_ptr,len);
		
		// Create a object
		
		printf("\nobj content --\n%s--\n\n\n",content);
		
		obj = initPDFObject();
		
		//printf("obj_content size = %d\n",obj->content_size);
		if( obj != NULL){
			obj->reference = ref;
			obj->content = content;
			obj->offset = offset;
			obj->content_size = len;
			
			getObjectInfos(obj);
			
		}
		
		
		// Add in object list.
		addObjectInList(obj,pdf);
		printf("------------------------------------\n\n");
	
	
	}
	
	//printf("content ptr = %d\n",pdf->content);
	//printf("startobj ptr = %s\n",startobj_ptr);
	
	
	return 0;
}


// Get pdf trailer according to PDF
int getPDFTrailers_1(struct pdfDocument * pdf){

	char * trailer_content = NULL;
	char * start = NULL; 
	char * end = NULL;
	int len = 0;
	struct pdfTrailer * trailer;
	
	start = searchPattern(pdf->content,"trailer",7,pdf->size);
	
	if(start == NULL ){
		return -1;
	}
	
	len = (int)(start - pdf->content);
	//len = obj->content_size - len;
	
	end = searchPattern(start,"%%EOF",5,pdf->size-len);
	end += 5;
	
	if(end == NULL){
		return -1;
		
	}
	len = (int)(end - start);
	
	trailer_content = (char*)malloc(len*sizeof(char));
	
	memcpy(trailer_content,start,len);
	
	if(!(trailer = initPDFTrailer())){
		printf("Error :: while initilaizing pdfTrailer structure\n");
		return -1;
	}
	
	pdf->trailers = trailer;
	
	printf("trailer content = %s\n",trailer_content);
	
	
	// TODO get several trailers

	return 0;
	
}


// Get pdf trailer according to PDF version starting from 1.5
int getPDFTrailers_2(struct pdfDocument * pdf){

	char * trailer_content = NULL;
	char * start = NULL; 
	char * end = NULL;
	int len = 0;
	struct pdfTrailer * trailer;
	
	start = searchPattern(pdf->content,"startxref",9,pdf->size);
	
	if(start == NULL ){
		return -1;
	}
	
	len = (int)(start - pdf->content);
	//len = obj->content_size - len;
	
	end = searchPattern(start,"%%EOF",5,pdf->size-len);
	end += 5;
	
	if(end == NULL){
		return -1;
	}
	
	len = (int)(end - start);
	
	trailer_content = (char*)malloc(len*sizeof(char));
	
	memcpy(trailer_content,start,len);
	
	if(!(trailer = initPDFTrailer())){
		printf("Error :: while initilaizing pdfTrailer structure\n");
		return -1;
	}
	
	pdf->trailers = trailer;
	
	printf("trailer content = %s\n",trailer_content);
	
	
	// TODO get several trailers

	return 0;
	
}


