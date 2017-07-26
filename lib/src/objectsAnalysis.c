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




#include <armaditopdf/analysis.h>
#include <armaditopdf/parsing.h>
#include <armaditopdf/utils.h>
#include <armaditopdf/log.h>
#include <armaditopdf/osdeps.h>
#include <armaditopdf/errors.h>
#include <time.h>












/*
getInfoObject() ::  Get the Info object
parameters:
- struct pdfDocument * pdf (pdf document pointer)
returns: (int)
- 1 if found
- 0 if not found
- an error code (<0) on error.
*/
int getInfoObject(struct pdfDocument * pdf){

	int len = 0;
	int res = 0;
	char * info = NULL;
	char * info_ref = 0;
	char * start = NULL;
	struct pdfObject * info_obj = NULL;
	struct pdfTrailer* trailer = NULL;
	
	if (pdf == NULL){
		err_log("getInfoObject :: invalid parameter\n");
		return -1;
	}

	// Get the trailer
	if(pdf->trailers == NULL){		
		warn_log("getInfoObject :: No trailer found in the document!\n");		
		return -1;
	}

	trailer = pdf->trailers;

	while(trailer != NULL){


		if(trailer->content == NULL){
			err_log("getInfoObject :: Empty trailer content\n");
			trailer = trailer->next;
			continue;
		}

		start = searchPattern(trailer->content, "/Info" , 5 , strlen(trailer->content));

		if(start == NULL){
			dbg_log("No /Info entry found in the trailer dictionary\n");			
			return 0;
		}


		start += 5; // skip "/Info"

		len = (int)(start - trailer->content);
		len = strlen(trailer->content) - len;

		info_ref = getIndirectRefInString(start,len);

		//dbg_log("getInfoObject :: info_ref =  %s\n", info_ref);		

		info_obj = getPDFObjectByRef(pdf, info_ref);

		if(info_obj == NULL){
			warn_log("getInfoObject :: Info object not found %s\n", info_ref);
			free(info_ref);
			return 0;
		}

		if(info_obj->dico != NULL){
			info = info_obj->dico;
			// dbg_log("getInfoObject :: info = %s\n",info);			

			// analyze the content
			res = unknownPatternRepetition(info, strlen(info), pdf, info_obj);
			if (res < 0){
				err_log("getJavaScript :: get pattern high repetition failed!\n");

			}else if (res > 0){
				warn_log("getInfoObject :: found potentially malicious /Info object %s\n", info_ref);
				pdf->testObjAnalysis->dangerous_keyword_high++; // TODO set another variable for this test :: MALICIOUS_INFO_OBJ
			}

			res = findDangerousKeywords(info, pdf, info_obj);
			if(res < 0){
				err_log("getJavaScript :: get dangerous keywords failed!\n");

			}else if(res > 0){
				warn_log("Warning :: getInfoObject :: found potentially malicious /Info object %s\n",info_ref);				
				pdf->testObjAnalysis->dangerous_keyword_high ++;
			}

		}else{
			warn_log("Warning :: getInfoObject :: Empty dictionary in info object :: %s\n",info_ref);			
		}


		trailer = trailer->next;
		free(info_ref);

	}

	return 1;
}


/*
analyzeURI() :: detects potentially malicious URI.
parameters:
- char * uri (uri to analyze).
- struct pdfDocument * pdf (pdf document pointer)
- struct pdfObject* obj
returns: (int)
- 1 if found
- 0 if not found
- an error code (<0) on error.
TODO :: analyzeURI :: Path traveral detection.
TODO :: analyzeURI :: Malicious uri detection.
*/
int analyzeURI(char * uri, struct pdfDocument * pdf, struct pdfObject * obj){

	
	//dbg_log("\tTODO... URI analysis :: %s\n", uri);
	if(pdf == NULL || obj == NULL)
		return -1;


	return 0;
}





/*
getActions() ::  Get Suspicious actions in the document.
parameters:
- struct pdfDocument * pdf (pdf document pointer)
returns: (int)
- 1 if dangerous content is found
- 0 if no active content.
- an error code (<0) on error.
TODO :: getActions :: get other potentially dangerous actions (OpenActions - GoToE - GoToR - etc.)
*/
int getActions(struct pdfDocument * pdf, struct pdfObject * obj){

	char * start = NULL;
	int dico_len = 0;

	// Check parameters
	if(pdf == NULL || obj == NULL){
		err_log("getActions :: invalid parameters!\n");
		return -1;
	}

	if(obj->dico == NULL){
		return 0;
	}

	dico_len = strlen(obj->dico);

	// get Launch actions
	start = searchPattern(obj->dico,"/Launch",7,dico_len);
	if(start != NULL){
		warn_log("getActions :: Found /Launch action in object %s\n",obj->reference);
		pdf->testObjAnalysis->dangerous_keyword_high ++;
		return 1;
	}

	return 0;
}


/*
removeWhiteSpace() ::  Remove all whites spaces in a given stream
parameters:
- char * stream
- int size.
returns: (char *)
- the new string without white spaces on success.
- NULL on error.
*/
char * removeWhiteSpace(char * stream, int size){

	char * out = NULL;
	char * start = NULL;
	char * end = NULL;
	int len = 0;
	int len_saved = 0;
	int len2 = 0;
	int count = 0;
	int i = 0;
	

	if (stream == NULL || size <= 0){
		err_log("removeWhiteSpace :: invalid parameters\n");
		return NULL;
	}

	// count white spaces
	for(i = 0; i<size; i++){
		if(stream[i] == '\n' || stream[i] == '\r' || stream[i] == ' ' ){
			count ++;
		}
	}	

	// calc the new len
	len = size - count;
	len_saved = len;
	out = (char*)calloc(len+1,sizeof(char));
	out[len] = '\0';


	start = stream;
	end = start;
	len = 0;
	
	while( len < (size - count) ){

		
		len2 = len;
		while(end[0] != '\n' && end[0] != '\r' && end[0] != ' ' && len2 < (size-count)){
			end ++;
			len2 ++;
		}

		len2 = (int)(end-start);
		//memcpy(out,start,len2);		
		os_strncat(out,len_saved+1,start, len2);
		len += len2;

		// skip white spaces
		start = end;
		while(start[0] == '\n' || start[0] == '\r' || start[0] == ' ' ){
			start ++;
		}

		end = start;
	}

	return out;
}


/*
unknownPatternRepetition() ::  Detect when a string (unknown) is repeated in the stream with a high frequency.
parameters:
- 
returns: (int)
- 1 if found
- 0 if not found
- error code (<0) on error.
*/
int unknownPatternRepetition(char * stream, int size, struct pdfDocument * pdf, struct pdfObject * obj){

	//int ret = 0;
	int ptr_len = 0;
	int ptr2_len = 0;
	int pattern_size = 5;
	int rep = 0;
	int lim_rep = 150;
	int time_exceeded = 6;
	char * pattern = NULL;
	char * ptr = NULL;
	char * ptr_bis = NULL;	
	char * tmp = NULL;
	char * whithout_space = NULL;
	time_t start_time, end_time;
	double time_elapsed = 0;


	if(pdf == NULL || obj == NULL || stream == NULL || size <= 0 ){
		err_log("unknownPatternRepetition :: invalid parameter\n");
		return -1;
	}

	time(&start_time);

	// remove white space.
	whithout_space =  removeWhiteSpace(stream,size);
	if (whithout_space == NULL){
		err_log("unknownPatternRepetition :: removing spaces failed !!\n");
		return -1;
	}
	
	//dbg_log("unknownPatternRepetition :: whithout_space = %s\n",without_space);
	
	ptr = whithout_space;
	ptr_len = strlen(whithout_space);


	// get pattern
	while( ptr_len > pattern_size && (pattern = getPattern(ptr,pattern_size,ptr_len)) != NULL ){

		rep = 0;
		//dbg_log("unknownPatternRepetition ::pattern = %s :: ptr_len = %d:: ptr = %d\n",pattern,ptr_len, ptr);
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
				
				warn_log("unknownPatternRepetition :: Found pattern repetition in object %s :: pattern = %s\n",obj->reference,pattern);							
				pdf->testObjAnalysis->pattern_high_repetition ++;
				free(whithout_space);
				free(pattern);
				free(tmp);
				return rep;
			}

			ptr_bis ++;
			ptr2_len --;

			time(&end_time);

			time_elapsed = difftime(end_time,start_time);
			
			//dbg_log("\n unknownPatternRepetition :: tmp = %s, %.2lf sec \n",tmp,time_elapsed);

			if(time_elapsed > time_exceeded){
				
				warn_log("unknownPatternRepetition :: Time Exceeded while analyzing object %s content\n",obj->reference);
				
				pdf->testObjAnalysis->time_exceeded++;
				free(whithout_space);
				free(tmp);
				free(pattern);
				return 0;
			}


			free(tmp);


			
		}


		time(&end_time);

		time_elapsed = difftime(end_time,start_time);

		if (time_elapsed > time_exceeded){
			warn_log("unknownPatternRepetition :: Time Exceeded while analyzing object %s content\n", obj->reference);

			pdf->testObjAnalysis->time_exceeded++;
			free(whithout_space);
			free(pattern);
			return 0;
		}

		free(pattern);

	}

	free(whithout_space);


	return 0 ;

}


/*
findDangerousKeywords() ::  Find a potentially dangerous pattern in the given stream; return High = 3 ; Medium = 2 ; Low = 1 ; none = 0
parameters:
- char * stream
- struct pdfDocument * pdf
- struct pdfObject * obj
returns: (int)
- 1 if found
- 0 if not found
- error code (<0) on error.
*/
int findDangerousKeywords(char * stream , struct pdfDocument * pdf, struct pdfObject * obj){

	int i = 0;
	char * high_keywords[] = {"HeapSpray","heap","spray","hack","shellcode", "shell", "pointers", "byteToChar", "system32", "payload", "console"};
	int num_high = 10;
	int num_medium = 9;
	int num_low = 1;
	char * medium_keywords[] = {"substring", "split", "eval", "addToolButton", "String.replace", "unescape", "exportDataObject", "StringFromChar", "util.print"};
	char * low_keywords[] = {"toString"};
	char * start = NULL;
	int len = 0;
	int unicode_count = 0;
	char* unicode = NULL;
	int ret = 0;


	if (pdf == NULL || obj == NULL || stream == NULL){
		err_log("findDangerousKeywords :: invalid parameters\n");
		return -1;
	}

	for(i = 0; i< num_high ; i++){

		if(searchPattern(stream,high_keywords[i],strlen(high_keywords[i]),strlen(stream)) != NULL ){			
			warn_log("findDangerousKeywords :: High dangerous keyword (%s) found in object %s\n",high_keywords[i], obj->reference);			
			pdf->testObjAnalysis->dangerous_keyword_high ++;
			ret = 3;
		}

	}

	// find unicode strings
	//stream = "%ufadeqsdqdqsdqsdqsdqsdqsd";
	start = stream ;
	len = strlen(stream);

	unicode = (char*)calloc(6,sizeof(char));

	while( len >= 6 && (start = getUnicodeInString(start,len)) != NULL && unicode_count < 50 ){

		memcpy(unicode, start, 6);

		warn_log("findDangerousKeywords :: Found unicode string %s in object %s\n", unicode, obj->reference);		

		unicode_count ++ ;
		start ++;

		len = (int)(start - stream);
		len = strlen(stream) - len;

	}

	if(unicode_count > 10){

		warn_log("findDangerousKeywords :: Unicode string found in object %s\n", obj->reference);		
		pdf->testObjAnalysis->dangerous_keyword_high ++;
		ret = 3;
	}

	for(i = 0; i< num_medium ; i++){

		if(  searchPattern(stream,medium_keywords[i],strlen(medium_keywords[i]),strlen(stream)) != NULL ){

			warn_log("findDangerousKeywords :: Medium dangerous keyword (%s) found in object %s\n", medium_keywords[i], obj->reference);			
			pdf->testObjAnalysis->dangerous_keyword_medium ++;			
			ret=  2;
		}

	}


	for(i = 0; i< num_low ; i++){

		if(  searchPattern(stream,low_keywords[i],strlen(low_keywords[i]),strlen(stream)) != NULL ){
		
			warn_log("findDangerousKeywords :: Low dangerous keyword (%s) found in object %s\n", low_keywords[i], obj->reference);			
			pdf->testObjAnalysis->dangerous_keyword_low ++;			
			ret = 1;
		}

	}

	free(unicode);
	return ret;


}


/*
getDangerousContent() :: get all potentially dangerous content (actions, js, embedded files, dangerous pattern, forms, url, etc.)
parameters:
- struct pdfDocument * pdf (pdf document pointer)
returns: (int)
- 1 if dangerous content is found
- 0 if no active content.
- an error code (<0) on error.
*/
int getDangerousContent(struct pdfDocument* pdf){

	int res = 0;
	struct pdfObject * obj = NULL;

	if( pdf == NULL || pdf->objects == NULL ){		
		err_log("getDangerousContent :: invalid parameters\n");		
		return -1;
	}

	//dbg_log("getDangerousContent :: start function...\n");

	obj = pdf->objects;

	while(obj != NULL){

		dbg_log("getDangerousContent :: Analysing object %s\n",obj->reference);

		/*if (getActions(pdf, obj) < 0){
			err_log("getDangerousContent :: get dangerous actions failed!\n");
			return -1;
		}

		/*if (getJavaScript(pdf, obj) < 0){
			err_log("getDangerousContent :: get javascript content failed!\n");
			return -1;
		}

		if (getXFA(pdf, obj) < 0){
			err_log("getDangerousContent :: get xfa content failed!\n");
			return -1;
		}

		if (getEmbeddedFile(pdf, obj) < 0){
			err_log("getDangerousContent :: get embedded file failed!\n");
			return -1;
		}
		
		if (getURI(pdf, obj) < 0){
			err_log("getDangerousContent :: get uri failed!\n");
			return -1;
		}*/

		// next object
		obj = obj->next;
		
	}
	
	if (getInfoObject(pdf) < 0){
		err_log("getDangerousContent :: get info object failed!\n");
		return -1;
	}
	

	return res;
}