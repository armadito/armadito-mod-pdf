#include "pdfAnalyzer.h"





// spot Javascript :: returns 1 if found and 0 if not.
int getJavaScript(struct pdfDocument * pdf, struct pdfObject* obj){

	char * js = NULL;
	char * js_obj_ref = NULL;
	char * start = NULL;
	//char * end = NULL;
	int len = 0;
	struct pdfObject * js_obj = NULL;


	if( obj->dico == NULL){
		//printf("No dictionary in object %s\n",obj->reference);
		return -1;
	}

	start = searchPattern(obj->dico, "/JS" , 3 ,  strlen(obj->dico));

	if(start == NULL){
		//printf("No Javascript detected in object %s\n",obj->reference);
		return 0;
	}

	printf("JavaScript Entry in dictionary detected in object %s\n", obj->reference);
	//printf("dictionary = %s\n", obj->dico);


	
	start += 3;

	// If there is a space
	//printf("start[0] = %c\n",start[0]);
	if(start[0] == ' '){
		start ++;
	}


	//printf("start[0] = %c\n",start[0]);

	len = strlen(obj->dico) - (int)(start - obj->dico);
	printf("len = %d\n",len);

	// get an indirect reference
	js_obj_ref = getIndirectRef(start,len);


	printf("js_obj_ref = %s\n", js_obj_ref);

	// Get javascript
	if(js_obj_ref != NULL){


		js_obj = getPDFObjectByRef(pdf,js_obj_ref);

		if(js_obj == NULL){
			printf("Error :: getJavaScript :: Object %s not found\n",js_obj_ref);
			return -1;
		}

		if( js_obj->decoded_stream != NULL){
			js = js_obj->decoded_stream;
		}else{
			js = js_obj->stream;
		}


		if(js != NULL){
			printf("Javascript content = %s\n",js);	
		}else{
			printf("Empty js content in object %s\n",js_obj_ref);
		}



	}else{

		// TODO process js script directly
		printf("Warning :: JS object reference is null\n");

		js = getDelimitedStringContent(start,"(",")",len);
		printf("JavaScript content = %s\n",js);



	}


	// TODO Launch analysis on content
	if(js != NULL){
		unknownPatternRepetition(js, strlen(js), obj);
		findDangerousKeywords(js,obj);
	}



	printf("\n\n");
	return 1;

}





// get Object
int getXFA(struct pdfDocument * pdf, struct pdfObject* obj){

	char * xfa = NULL;
	char * xfa_obj_ref = NULL;
	char * start = NULL;
	char * end = NULL;
	char * obj_list = NULL;
	//char * stream = NULL;
	int len = 0;
	int len2 = 0;
	struct pdfObject * xfa_obj = NULL;


	//printf("Analysing object :: %s\n",obj->reference);

	if( obj->dico == NULL ){
		//printf("No dictionary in object %s\n",obj->reference);
		return -1;
	}

	start = searchPattern(obj->dico, "/XFA" , 4 , strlen(obj->dico));

	if( strncmp(obj->reference,"5 0 obj",7) == 0 && strncmp(obj->reference,"5 0 obj",strlen(obj->reference)) == 0 ){
		printf("dico = %s\n",obj->dico);
	}

	if(start == NULL){
		//printf("No XFA entry detected in object dictionary %s\n",obj->reference);
		return 0;
	}

	printf("XFA Entry in dictionary detected in object %s\n", obj->reference);
	//printf("dictionary = %s\n", obj->dico);

	start += 4;

	// If there is a space // todo put a while
	printf("start[0] = %c\n",start[0]);
	if(start[0] == ' '){
		start ++;
	}

	printf("start[0] = %c\n",start[0]);

	len = strlen(obj->dico) - (int)(start - obj->dico);
	printf("len = %d\n",len);


	// Get xfa object references

	// If its a list get the content
	if(start[0] == '['){

		obj_list =  getDelimitedStringContent(start,"[", "]", len); 
		printf("obj_list = %s\n",obj_list);


		end = obj_list;
		len2 = strlen(obj_list);


		// get XFA object reference in array ::
		while( (xfa_obj_ref = getIndirectRefInString(end, len2)) ){

			printf("xfa_obj_ref = %s\n",xfa_obj_ref);

			end = searchPattern(end, xfa_obj_ref , 4 , len2); // change value 4
			end += strlen(xfa_obj_ref) - 2;

			len2 = (int)(end - obj_list);
			len2 = strlen(obj_list) - len2;

			// get xfa object 
			xfa_obj =  getPDFObjectByRef(pdf, xfa_obj_ref);

			if(xfa_obj == NULL){
				printf("Error :: getXFA :: Object %s not found\n",xfa_obj_ref);
				continue;
			}

			// get xfa content
			if(xfa_obj->decoded_stream != NULL ){
				xfa = xfa_obj->decoded_stream;
			}else{
				xfa = xfa_obj->stream;
			}

			if(xfa != NULL){
				//printf("XFA content = %s\n",xfa);	
				// TODO Analyze xfa content
				unknownPatternRepetition(xfa, strlen(xfa), xfa_obj);
			}else{
				printf("Warning :: Empty XFA content in object %s\n",xfa_obj_ref);
			}


		}

	}else{
				
		len2 = (int)(start - obj->dico);
		len2 = strlen(obj->dico) - len2;


		xfa_obj_ref = getIndirectRefInString(start, len2);

		if(xfa_obj_ref == NULL){
			printf("Error :: getXFA :: get xfa object indirect reference failed\n");
			return -1;
		}

		printf("xfa_obj_ref = %s\n",xfa_obj_ref);

		xfa_obj =  getPDFObjectByRef(pdf, xfa_obj_ref);

		if(xfa_obj == NULL){
			printf("Error :: getXFA :: Object %s not found\n",xfa_obj_ref);
			return -1;
		}

		// get xfa content
		if(xfa_obj->decoded_stream != NULL ){
			xfa = xfa_obj->decoded_stream;
		}else{
			xfa = xfa_obj->stream;
		}

		if(xfa != NULL){
			//printf("XFA content = %s\n",xfa);	
			// TODO Analyze xfa content
			unknownPatternRepetition(xfa, strlen(xfa), xfa_obj);
		}else{
			printf("Warning :: Empty XFA content in object %s\n",xfa_obj_ref);
		}


	}

	
	
	printf("\n\n");
	return 1;

}



// This function get Embedded file content and analyze it ; returns 0 ther is no embedded file
int getEmbeddedFile(struct pdfDocument * pdf , struct pdfObject* obj){

	char * ef = NULL;
	struct pdfObject * ef_obj = NULL;
	char * ef_obj_ref = NULL;
	char * start = NULL;
	//char * end = NULL;
	int len = 0;

	//printf("Analysing object :: %s\n",obj->reference);

	// verif params
	if(obj->dico == NULL || obj->type == NULL){
		return 0;
	}

	// Get by Type or by Filespec (EF entry)

	if( strncmp(obj->type,"/EmbeddedFile",13) == 0){


		printf("Found EmbeddedFile object %s\n",obj->reference);

		if(obj->decoded_stream != NULL ){
			ef = obj->decoded_stream;
		}else{
			ef = obj->stream;
		}

		if(ef != NULL){
			printf("ef content = %s\n",ef);
			// TODO Process to ef stream content analysis.
		}else{
			printf("Warning :: Empty EF stream content in object %s\n",obj->reference);
		}


	}


	
	if( strncmp(obj->type,"/Filespec",9) == 0){

		// Get EF entry in dico
		start = searchPattern(obj->dico, "/EF" , 3 , strlen(obj->dico));

		if(start == NULL){
			//printf("No EF detected in object %s\n",obj->reference);
			return 0;
		}

		printf("Found EmbeddedFile in file specification %s\n",obj->reference);

		start += 3;

		// For white spaces
		while(start[0] == ' '){
			start ++;
		}


		len = (int)(start - obj->dico);
		len = strlen(obj->dico) - len;
		// get indirect ref of the 
		ef_obj_ref = getIndirectRef(start,len);
		printf("ef_obj_ref = %s\n",ef_obj_ref);

		ef_obj = getPDFObjectByRef(pdf,ef_obj_ref);

		if(ef_obj != NULL){


			if(ef_obj->dico == NULL){
				printf("Warning :: No dictionary found in object %s\n",ef_obj_ref);
				return -1;
			}
			// Get the /F entry in the dico
			start = searchPattern(ef_obj->dico, "/F" , 2 , strlen(ef_obj->dico));

			if(start == NULL){
				//printf("No EF detected in object %s\n",obj->reference);
				return 0;
			}

			start += 2;

			// For white spaces
			while(start[0] == ' '){
				start ++;
			}

			len = (int)(start - ef_obj->dico);
			len = strlen(ef_obj->dico) - len;

			ef_obj_ref = getIndirectRef(start,len);
			printf("EF_obj_ref = %s\n",ef_obj_ref);


			ef_obj = getPDFObjectByRef(pdf,ef_obj_ref);

			if(ef_obj != NULL){

				if(ef_obj->decoded_stream != NULL ){
					ef = ef_obj->decoded_stream;
				}else{
					ef = ef_obj->stream;
				}

				if( ef != NULL){
					printf("ef content = %s\n",ef);
					// TODO Process to ef stream content analysis.
				}else{
					printf("Warning :: Empty EF stream content in object %s\n",obj->reference);
				}

			}else{
				printf("Warning :: getEmbeddedFile :: object not found %s\n",ef_obj_ref);
			}



		}else{
			printf("Warning :: getEmbeddedFile :: object not found %s\n",ef_obj_ref);
		}
		

	}

	/*if(ef != NULL){
		ptr2_len > pattern_size
	}*/



	return 1;
}


// This function get the Info obejct
int getInfoObject(struct pdfDocument * pdf){

	char * info = NULL;
	char * info_ref = 0;
	struct pdfObject * info_obj = NULL;
	char * start = NULL;
	//char * end = NULL;
	int len = 0;
	struct pdfTrailer* trailer = NULL;


	// Get the trailer
	if(pdf->trailers == NULL){
		printf("Warning :: getInfoObject :: No trailer found !");
		return -1;
	}

	trailer = pdf->trailers;

	while(trailer != NULL){


		//printf("trailer content = %s\n",trailer->content );
		//printf("trailer size = %d\n",strlen(trailer->content));

		if(trailer->content == NULL){
			printf("Error :: getInfoObject :: Empty trailer content\n");
			trailer = trailer->next;
			continue;
		}

		start = searchPattern(trailer->content, "/Info" , 5 , strlen(trailer->content));

		if(start == NULL){
			printf("No /Info entry found in the trailer dictionary\n");
			return 0;
		}


		start += 5;

		len = (int)(start - trailer->content);
		len = strlen(trailer->content) - len;

		info_ref = getIndirectRefInString(start,len);

		printf("info_ref =  %s\n", info_ref);

		info_obj = getPDFObjectByRef(pdf, info_ref);

		if(info_obj == NULL){
			printf("Warning :: getInfoObject :: Info object not found %s\n", info_ref);
			return 0;
		}

		if(info_obj->dico != NULL){
			info = info_obj->dico;
			printf("info = %s\n",info);
		}else{
			printf("Warning :: getInfoObject :: Empty dictionary in info object :: %s\n",info_ref);
		}


		trailer = trailer->next;

		printf("\n\n");

	}





	return 1;
}


// This function get all potentially dangerous content (js, embedded files, dangerous pattern) form. and return 1 if any of 0 if not
int getDangerousContent(struct pdfDocument* pdf){

	int res = 0;
	struct pdfObject * obj = NULL;

	if( pdf == NULL || pdf->objects == NULL ){
		printf("Error :: getDangerousContent :: Null Arguments\n");
		return -1;
	}

	//printf("pdfobjects %d\n",pdf->objects);
	obj = pdf->objects;


	while(obj != NULL){

		//printf("Analysing object :: %s\n",obj->reference);

		getJavaScript(pdf,obj);

		//getXFA(pdf,obj);

		//getEmbeddedFile(pdf,obj);

		// next object
		obj = obj->next;

	}

	getInfoObject(pdf);

	return res;

}




// This function detects when a string (unknown) is repeted in the stream with a high frequency.
// TODO remove white spaces to improve results.
int unknownPatternRepetition(char * stream, int size, struct pdfObject * obj){

	//int ret = 0;
	char * pattern = NULL;
	int pattern_size = 5;
	//char * start = NULL;
	//char * end = NULL;
	//char * len = 0;
	int rep = 0;
	int lim_rep = 100;
	//int off = 0;
	char * ptr = NULL;
	char * ptr_bis = NULL;
	int ptr_len = 0;
	int ptr2_len = 0;
	char * tmp = NULL;


	printf("\n\nDebug :: unknownPatternRepetition \n");


	// remove white space in stream ? 
	//ptr = removeWhiteSpace(stream,size);



	ptr = stream;
	ptr_len = strlen(stream);

	ptr_bis = stream;
	ptr2_len = strlen(stream);



	/*
	printf("ptr[0] = %c\n", ptr[0] );
	printf("ptr[1] = %c\n", ptr[1] );
	printf("ptr[2] = %c\n", ptr[2] );
	printf("ptr[3] = %c\n", ptr[3] );
	printf("ptr[4] = %c\n", ptr[4] );
	*/



	// get pattern
	while( ptr_len > pattern_size && (pattern = getPattern(ptr,pattern_size,ptr_len)) != NULL ){

		rep = 0;
		//printf("pattern = %s :: ptr_len = %d:: ptr = %d\n",pattern,ptr_len, ptr);
		ptr ++;
		ptr_len --;

		// search occurrences
		ptr_bis = ptr+5;
		ptr2_len = ptr_len-5;

		while( ptr2_len > pattern_size && (tmp = getPattern(ptr_bis,pattern_size,ptr2_len)) != NULL){

			if(strncmp(pattern,tmp,pattern_size) == 0){
				rep ++;
			}

			if(rep > lim_rep){
				printf("Warning :: unknownPatternRepetition :: Found pattern repetition in object %s\n",obj->reference);
				printf("pattern = %s :: rep = %d--\n\n",pattern,rep);
				return rep;
			}

			ptr_bis ++;
			ptr2_len --;
			
		}

		/*
		if(rep > 100){
			return 100;
		}
		*/

	}


	return 0 ;

}








// Find a potentially dangerous pattern in the given stream; return High = 3 ; Medium = 2 ; Low = 1 ; none = 0
int findDangerousKeywords(char * stream , struct pdfObject * obj){

	int i = 0;
	char * high_keywords[] = {"HeapSpray","heap","spray","hack","shellcode", "shell", "Execute", "pointers", "byteToChar", "system32", "payload", "console"};
	int num_high = 12;
	int num_medium = 10;
	char * medium_keywords[] = {"toString", "substring", "split", "eval", "addToolButton", "String.replace", "unscape", "exportDataObject", "StringFromChar", "util.print"};
	char * start = NULL;
	int len = 0;
	int unicode_count = 0;
	char* unicode = NULL;



	//stream = "hackbonjour";

	//printf("Test :: %s\n",high_keywords[i]);

	for(i = 0; i< num_high ; i++){

		//printf("Test :: %s\n",high_keywords[i]);
		//if(strnstr(stream,high_keywords[i],strlen(high_keywords[i])) != NULL ){
		if(searchPattern(stream,high_keywords[i],strlen(high_keywords[i]),strlen(stream)) != NULL ){
			printf("Warning :: findDangerousKeywords :: High dangerous keyword (%s) found in object %s\n",high_keywords[i], obj->reference);
			return 3;
		}

	}

	// find unicode strings
	//stream = "%ufadeqsdqdqsdqsdqsdqsdqsd";
	start = stream ;
	len = strlen(stream);

	unicode = (char*)calloc(6,sizeof(char));

	while( len >= 6 && (start = getUnicodeInString(start,len)) != NULL ){

		memcpy(unicode, start, 6);
		printf("Found unicode string %s\n",unicode);

		unicode_count ++ ;
		start ++;

		len = (int)(start - stream);
		len = strlen(stream) - len;
		//printf("len = %d\n",len);

	}

	if(unicode_count > 10){
		printf("Warning :: findDangerousKeywords :: Unicode string found in object %s\n", obj->reference);

	}
	





	for(i = 0; i< num_medium ; i++){

		//printf("Test :: %s\n",medium_keywords[i]);
		//if(strnstr(stream,medium_keywords[i],strlen(medium_keywords[i])) != NULL ){
		if(  searchPattern(stream,medium_keywords[i],strlen(medium_keywords[i]),strlen(stream)) != NULL ){
			printf("Warning :: findDangerousKeywords :: Medium dangerous keyword (%s) found in object %s\n",medium_keywords[i], obj->reference);
			return 2;
		}

	}



	return 0;


}