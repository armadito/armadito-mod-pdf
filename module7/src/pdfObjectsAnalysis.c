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
	//printf("len = %d\n",len);

	// get an indirect reference
	js_obj_ref = getIndirectRef(start,len);


	//printf("js_obj_ref = %s\n", js_obj_ref);

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
			printf("Found JS content in object %s\n",js_obj_ref);
			//printf("Javascript content = %s\n",js);	
			// TODO Launch analysis on content

			//printf("js = %s\n",js);

			
			pdf->testObjAnalysis->active_content ++;
			pdf->testObjAnalysis->js ++;
			unknownPatternRepetition(js, strlen(js),pdf, js_obj);
			findDangerousKeywords(js,pdf,js_obj);
			

		}else{
			printf("Debug :: getJavaScript :: Empty js content in object %s\n",js_obj_ref);
		}



	}else{

		// TODO process js script directly
		//printf("Warning :: getJavaScript :: JS object reference is null\n");

		js = getDelimitedStringContent(start,"(",")",len);
		//printf("JavaScript content = %s\n",js);

		if(js != NULL){
			printf("Debug :: getJavaScript :: Found JS content in object %s\n",obj->reference);

			pdf->testObjAnalysis->active_content ++;
			pdf->testObjAnalysis->js ++;
			unknownPatternRepetition(js, strlen(js),pdf,obj);
			findDangerousKeywords(js,pdf,obj);

		}



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

	/*if( strncmp(obj->reference,"5 0 obj",7) == 0 && strncmp(obj->reference,"5 0 obj",strlen(obj->reference)) == 0 ){
		printf("dico = %s\n",obj->dico);
	}*/

	if(start == NULL){
		//printf("No XFA entry detected in object dictionary %s\n",obj->reference);
		return 0;
	}

	//printf("XFA Entry in dictionary detected in object %s\n", obj->reference);
	//printf("dictionary = %s\n", obj->dico);

	start += 4;

	// If there is a space // todo put a while
	//printf("start[0] = %c\n",start[0]);
	if(start[0] == ' '){
		start ++;
	}

	//printf("start[0] = %c\n",start[0]);

	len = strlen(obj->dico) - (int)(start - obj->dico);
	//printf("len = %d\n",len);


	// Get xfa object references

	// If its a list get the content
	if(start[0] == '['){

		obj_list =  getDelimitedStringContent(start,"[", "]", len); 
		//printf("obj_list = %s\n",obj_list);


		end = obj_list;
		len2 = strlen(obj_list);


		// get XFA object reference in array ::
		while( (xfa_obj_ref = getIndirectRefInString(end, len2)) ){

			//printf("xfa_obj_ref = %s\n",xfa_obj_ref);

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
				printf("Found XFA content in object %s\n",xfa_obj_ref);
				// TODO Analyze xfa content
				unknownPatternRepetition(xfa, strlen(xfa),pdf, xfa_obj);
				findDangerousKeywords(xfa,pdf,obj);
				pdf->testObjAnalysis->active_content ++;
				pdf->testObjAnalysis->xfa ++;
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

		//printf("xfa_obj_ref = %s\n",xfa_obj_ref);

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
			printf("Found XFA content in object %s\n",xfa_obj_ref);
			//printf("XFA content = %s\n",xfa);	
			// TODO Analyze xfa content
			unknownPatternRepetition(xfa, strlen(xfa), pdf, xfa_obj);
			findDangerousKeywords(xfa,pdf,obj);
			pdf->testObjAnalysis->active_content ++;
			pdf->testObjAnalysis->xfa ++;
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
	//char * isDico = NULL;
	//char * end = NULL;
	int len = 0;

	//printf("Analysing object :: %s\n",obj->reference);

	// verif params
	if(obj->dico == NULL || obj->type == NULL){
		return 0;
	}

	// Get by Type or by Filespec (EF entry)

	if( strncmp(obj->type,"/EmbeddedFile",13) == 0){


		

		if(obj->decoded_stream != NULL ){
			ef = obj->decoded_stream;
		}else{
			ef = obj->stream;
		}

		if(ef != NULL){
			printf("Found EmbeddedFile object %s\n",obj->reference);
			//printf("ef content = %s\n",ef);
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


		// The case <</EF <</F 3 0 R>>
		//isDico = searchPattern
		printf("start[0] = %c\n",start[0]);

		if(start[0] == '<' && start[1] == '<'){

			len = (int)(start - obj->dico);
			len = strlen(obj->dico) - len;

			ef_obj_ref = obj->reference;
			ef_obj = obj;



		}else{

			len = (int)(start - obj->dico);
			len = strlen(obj->dico) - len;
			// get indirect ref of the 
			ef_obj_ref = getIndirectRef(start,len);
			//printf("ef_obj_ref = %s\n",ef_obj_ref);
			ef_obj = getPDFObjectByRef(pdf,ef_obj_ref);

		}



		

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
					printf("Found EmbeddedFile object %s\n",ef_obj_ref);
					//printf("ef content = %s\n",ef);
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

	if(ef != NULL){

		pdf->testObjAnalysis->active_content ++;
		pdf->testObjAnalysis->ef ++;
		// Analysi
	}



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
	int res = 0;


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

			// TODO analyze the content
			res = unknownPatternRepetition(info, strlen(info), pdf, info_obj);
			if(res > 0){
				printf("Warning :: getInfoObject :: found potentially malicious /Info object %s\n",info_ref);
				pdf->testObjAnalysis->dangerous_keyword_high ++; // TODO find another variable for this test :: MALICIOUS_INFO_OBJ
			}

			res = findDangerousKeywords(info,pdf,info_obj);
			if(res > 0){
				printf("Warning :: getInfoObject :: found potentially malicious /Info object %s\n",info_ref);
				pdf->testObjAnalysis->dangerous_keyword_high ++;
			}

		}else{
			printf("Warning :: getInfoObject :: Empty dictionary in info object :: %s\n",info_ref);
		}


		trailer = trailer->next;
		free(info_ref);

		printf("\n\n");

	}

	return 1;
}



// This function detects potentially malicious URI.
int analyzeURI(char * uri, struct pdfDocument * pdf, struct pdfObject * obj){


	printf("\tTODO... URI analysis :: %s\n", uri);


	return 0;
}



//This function get the URI defined in the object and analyze it
int getURI(struct pdfDocument * pdf, struct pdfObject * obj){


	char * start = NULL;
	char * end = NULL;
	char * uri = NULL;
	int len = 0;




	if(obj == NULL || pdf == NULL){
		printf("Error :: getURI :: Bad (null) parameters\n");
		return -1;
	}

	if(obj->dico == NULL){
		return 0;
	}

	// get the URI entry in the dico
	//start = searchPattern(obj->dico,"/URI",4,strlen(obj->dico));

	end= obj->dico;
	len = strlen(obj->dico);

	while( (start = searchPattern(end,"/URI",4,len)) != NULL ){

		start += 4;

		// skip white spaces
		while(start[0] == ' '){
			start ++;
		}

		//printf("start0 = %c\n",start[0]);
		end = start;

		if(start[0] != '('){
			continue;
		}


		len = (int)(start - obj->dico);
		len = strlen(obj->dico) - len;

		uri = getDelimitedStringContent(start,"(",")", len);
		//printf("uri = %s\n",uri);

		// Analyze uri
		analyzeURI(uri,pdf,obj);

	}

	return 1;
}


// This function remove whites space in a given stream
char * removeWhiteSpace(char * stream, int size){

	char * out = NULL;
	char * start = NULL;
	char * end = NULL;
	int len = 0;
	int len2 = 0;
	int count = 0;
	int i = 0;
	// count white spaces

	

	for(i = 0; i<size; i++){
		if(stream[i] == '\n' || stream[i] == '\r' || stream[i] == '\n' || stream[i] == ' ' ){
			count ++;
		}

	}

	//printf("There are %d white space in this stream\n",count);

	// calc the new len
	len = size - count;

	out = (char*)calloc(len+1,sizeof(char));
	out[len] = '\0';

	

	start = stream;
	end = start;
	len = 0;

	//printf("end0 = %c\n",end[0]);

	
	
	while( len < (size - count) ){

		
		len2 = len;
		while(end[0] != '\n' && end[0] != '\r' && end[0] != '\n' && end[0] != ' ' && len2 < (size-count)){
			end ++;
			len2 ++;
		}

		len2 = (int)(end-start);
		//memcpy(out,start,len2);
		strncat(out, start, len2);
		len += len2;

		// skip white spaces
		start = end;
		while(start[0] == '\n' || start[0] == '\r' || start[0] == '\n' || start[0] == ' ' ){
			start ++;
		}

		end = start;
	}

	


	return out;
}



// This function detects when a string (unknown) is repeted in the stream with a high frequency.
// TODO remove white spaces to improve results.
int unknownPatternRepetition(char * stream, int size, struct pdfDocument * pdf, struct pdfObject * obj){

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
	char * whithout_space = NULL;
	time_t start_time, end_time;
	double time_elapsed = 0;

	/*
	if(pdf != NULL){
		return 0;
	}*/


	time(&start_time);
	//char * test = "Bonjour je suis une stream de test";


	//printf("\n\nDebug :: unknownPatternRepetition \n");

	// remove white space in stream ? 
	
	//whithout_space =  removeWhiteSpace(test,strlen(test));

	//printf("whithout_space = %s\n",whithout_space);

	
	whithout_space =  removeWhiteSpace(stream,size);
	

	//printf("whithout_space = %s\n",ptr);
	
	ptr = whithout_space;
	ptr_len = strlen(whithout_space);

	ptr_bis = whithout_space;
	ptr2_len = strlen(whithout_space);


	/*
	ptr = stream;
	ptr_len = strlen(stream);

	ptr_bis = stream;
	ptr2_len = strlen(stream);
	*/


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

			//printf("tmp = %s\n",tmp);

			if(strncmp(pattern,tmp,pattern_size) == 0){
				rep ++;
			}

			if(rep > lim_rep){
				printf("Warning :: unknownPatternRepetition :: Found pattern repetition in object %s\n",obj->reference);
				printf("pattern = %s :: rep = %d--\n\n",pattern,rep);
				pdf->testObjAnalysis->pattern_high_repetition ++;
				return rep;
			}

			ptr_bis ++;
			ptr2_len --;

			time(&end_time);

			time_elapsed = difftime(end_time,start_time);
			
			//printf("\ntmp = %s, %.2lf sec \n",tmp,time_elapsed);

			if(time_elapsed > 5){
				printf("Warning :: unknownPatternRepetition :: Time Exceeded while analyzing object %s content\n",obj->reference );
				pdf->testObjAnalysis->time_exceeded++;
				free(whithout_space);
				return 0;
			}


			
		}

		

		//printf("heyhey :: %d :: %d :: %d :: \n",ptr2_len,pattern_size,rep, );

		/*
		if(rep > 100){
			return 100;
		}
		*/

		time(&end_time);

		time_elapsed = difftime(end_time,start_time);

		//printf("heyhoo :: %d\n",time_elapsed);
		//printf("\nExecution time : %.2lf sec \n",time_elapsed);

		if(time_elapsed > 5){
			printf("Warning :: unknownPatternRepetition :: Time Exceeded while analyzing object %s content\n",obj->reference );
			pdf->testObjAnalysis->time_exceeded++;
			free(whithout_space);
			return 0;
		}

	}

	free(whithout_space);


	return 0 ;

}




// Find a potentially dangerous pattern in the given stream; return High = 3 ; Medium = 2 ; Low = 1 ; none = 0
int findDangerousKeywords(char * stream , struct pdfDocument * pdf, struct pdfObject * obj){

	int i = 0;
	char * high_keywords[] = {"HeapSpray","heap","spray","hack","shellcode", "shell", "pointers", "byteToChar", "system32", "payload", "console"};
	int num_high = 10;
	int num_medium = 10;
	char * medium_keywords[] = {"toString", "substring", "split", "eval", "addToolButton", "String.replace", "unescape", "exportDataObject", "StringFromChar", "util.print"};
	char * start = NULL;
	int len = 0;
	int unicode_count = 0;
	char* unicode = NULL;
	int ret = 0;


	//printf(":: Searching Dangerous Keywords in %s\n",obj->reference);

	//printf("\n\n content to analyze = %s\n",stream);

	//stream = "hackbonjour";

	//printf("Test :: %s\n",high_keywords[i]);

	for(i = 0; i< num_high ; i++){

		//printf("Test :: %s\n",high_keywords[i]);
		//if(strnstr(stream,high_keywords[i],strlen(high_keywords[i])) != NULL ){
		
		if(searchPattern(stream,high_keywords[i],strlen(high_keywords[i]),strlen(stream)) != NULL ){
			printf("Warning :: findDangerousKeywords :: High dangerous keyword (%s) found in object %s\n",high_keywords[i], obj->reference);
			pdf->testObjAnalysis->dangerous_keyword_high ++;
			ret = 3;
		}

	}

	// find unicode strings
	//stream = "%ufadeqsdqdqsdqsdqsdqsdqsd";
	start = stream ;
	len = strlen(stream);

	unicode = (char*)calloc(6,sizeof(char));

	printf("heyhey\n");

	while( len >= 6 && (start = getUnicodeInString(start,len)) != NULL && unicode_count < 50 ){


		memcpy(unicode, start, 6);
		printf("Found unicode string %s in object %s\n",unicode,obj->reference);

		unicode_count ++ ;
		start ++;

		len = (int)(start - stream);
		len = strlen(stream) - len;


		printf("len = %d\n",len);



	}

	if(unicode_count > 10){
		printf("Warning :: findDangerousKeywords :: Unicode string found in object %s\n", obj->reference);
		pdf->testObjAnalysis->dangerous_keyword_high ++;
		ret = 3;
	}
	


	for(i = 0; i< num_medium ; i++){

		//printf("Test :: %s\n",medium_keywords[i]);
		//if(strnstr(stream,medium_keywords[i],strlen(medium_keywords[i])) != NULL ){
		if(  searchPattern(stream,medium_keywords[i],strlen(medium_keywords[i]),strlen(stream)) != NULL ){
			printf("Warning :: findDangerousKeywords :: Medium dangerous keyword (%s) found in object %s\n",medium_keywords[i], obj->reference);
			pdf->testObjAnalysis->dangerous_keyword_medium ++;
			ret=  2;
		}

	}

	free(unicode);
	return ret;


}

// This function get all potentially dangerous content (js, embedded files, dangerous pattern) form. and return 1 if any of 0 if not
int getDangerousContent(struct pdfDocument* pdf){

	int res = 0;
	struct pdfObject * obj = NULL;

	if( pdf == NULL || pdf->objects == NULL ){
		printf("Error :: getDangerousContent :: Null Arguments\n");
		return -1;
	}

	printf("\n-------------------------\n");
	printf("---  OBJECT ANALYSIS  ---\n");
	printf("-------------------------\n\n");

	//printf("pdfobjects %d\n",pdf->objects);
	obj = pdf->objects;


	while(obj != NULL){

		//printf("Analysing object :: %s\n",obj->reference);

		getJavaScript(pdf,obj);

		getXFA(pdf,obj);

		getEmbeddedFile(pdf,obj);

		getURI(pdf,obj);

		// next object
		obj = obj->next;

	}

	getInfoObject(pdf);

	return res;

}