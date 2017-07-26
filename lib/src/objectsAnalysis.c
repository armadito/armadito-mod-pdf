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
pdf_get_JavaScript() ::  Get Javascript content in the document.
*/
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

		retcode = add_pdf_active_content(pdf,AC_JAVASCRIPT,obj->reference, js, len);
		if(retcode != ERROR_SUCCESS){
			err_log("get_JavaScript :: Add active content failed!\n");
		}

	}else{

		// get js content in dictionary string.
		js = getDelimitedStringContent(start,"(",")",len);

		if (js != NULL){

			dbg_log("getJavaScript :: Found JS content in object %s\n", obj->reference);

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
get_js_from_data() :: Get and analyze JavaScript content in XFA form description (xml).
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


/*
pdf_get_xfa() ::  Get XFA form in the document.
TODO: treat the case when dico is
	/XFA [(xdp:xdp) 10 0 R
			(template) 11 0 R
			(datasets) 12 0 R
			(config) 13 0 R
			(/xdp:xdp) 14 0 R ]

*/
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

			retcode = get_js_from_data(xfa, len, xfa_obj, pdf);
			if(retcode != ERROR_SUCCESS){
				err_log("get_xfa :: Get javascript from xfa content failed!\n");
				free(obj_list);
				return retcode;
			}

			/*retcode = add_pdf_active_content(pdf,AC_XFA,obj->reference, xfa, len);
			if(retcode != ERROR_SUCCESS){
				err_log("get_JavaScript :: Add active content failed!\n");
				return retcode;
			}*/

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

		retcode = add_pdf_active_content(pdf,AC_EMBEDDED_FILE,obj->reference, ef, ef_size);
		if(retcode != ERROR_SUCCESS){
			err_log("get_embedded_file :: Add active content failed!\n");
			return retcode;
		}
	}

	return ERROR_SUCCESS;
}


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
getEmbeddedFile() ::  Get the URI defined in the object and analyze it
parameters:
- struct pdfDocument * pdf (pdf document pointer)
- struct pdfObject* obj
returns: (int)
- 1 if found
- 0 if not found
- an error code (<0) on error.
*/
int getURI(struct pdfDocument * pdf, struct pdfObject * obj){


	char * start = NULL;
	char * end = NULL;
	char * uri = NULL;
	int len = 0;

	if(obj == NULL || pdf == NULL){
		err_log("getURI :: invalid parameter\n");		
		return -1;
	}

	if(obj->dico == NULL){
		return 0;
	}

	// get the URI entry in the dico
	end= obj->dico;
	len = strlen(obj->dico);

	while( (start = searchPattern(end,"/URI",4,len)) != NULL ){

		start += 4;

		// skip white spaces
		while(start[0] == ' '){
			start ++;
		}

		end = start;

		if(start[0] != '('){
			continue;
		}


		len = (int)(start - obj->dico);
		len = strlen(obj->dico) - len;

		uri = getDelimitedStringContent(start,"(",")", len);

		// Analyze uri
		if (uri != NULL) {
			analyzeURI(uri, pdf, obj);
			free(uri);
		}
		

	}

	return 1;
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
		}*/
		
		if (getURI(pdf, obj) < 0){
			err_log("getDangerousContent :: get uri failed!\n");
			return -1;
		}

		// next object
		obj = obj->next;
		
	}
	
	if (getInfoObject(pdf) < 0){
		err_log("getDangerousContent :: get info object failed!\n");
		return -1;
	}
	

	return res;
}