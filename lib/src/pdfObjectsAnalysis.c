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




#include "pdfAnalysis.h"
#include "pdfParsing.h"
#include "utils.h"
#include "log.h"
#include "osdeps.h"
#include "time.h"



/*
getJavaScript() ::  Get Javascript content in the document.
parameters:
- struct pdfDocument * pdf (pdf document pointer)
returns: (int)
- 1 if js content is found
- 0 if no active content.
- an error code (<0) on error.
*/
int getJavaScript(struct pdfDocument * pdf, struct pdfObject* obj){

	char * js = NULL;
	char * js_obj_ref = NULL;
	char * start = NULL;
	int len = 0;
	int ret = 0;
	struct pdfObject * js_obj = NULL;
	

	if (pdf == NULL || obj == NULL){
		err_log("getJavaScript :: invalid parameters\n");
		return -1;
	}

	if( obj->dico == NULL){		
		return 0;
	}

	if ((start = searchPattern(obj->dico, "/JS", 3, strlen(obj->dico))) == NULL){
		return 0;
	}

	//dbg_log("getJavaScript :: JavaScript Entry in dictionary detected in object %s\n", obj->reference);
	//dbg_log("getJavaScript :: dictionary = %s\n", obj->dico);


	start += 3; // 3 => /JS

	// skip space
	if(start[0] == ' '){
		start ++;
	}

	len = strlen(obj->dico) - (int)(start - obj->dico);	

	// get an indirect reference object containing of js content.
	js_obj_ref = getIndirectRef(start,len);

	// Get javascript
	if(js_obj_ref != NULL){

		if ((js_obj = getPDFObjectByRef(pdf, js_obj_ref)) == NULL){
			err_log("getJavaScript :: Object %s not found\n",js_obj_ref);
			pdf->errors++;
			free(js_obj_ref);
			return -1;
		}

		// Decode object stream
		if (js_obj->filters != NULL){
			if (decodeObjectStream(js_obj) < 0){
				err_log("getJavaScript :: decode object stream failed!\n");
				// if decoding object stream failed then exit.
				pdf->errors++;
				free(js_obj_ref);
				return -2;
			}
		}

		if( js_obj->decoded_stream != NULL){
			js = js_obj->decoded_stream;
		}else{
			js = js_obj->stream;
		}

		free(js_obj_ref);

		if (js != NULL){
			dbg_log("getJavaScript :: Found JS content in object %s\n", js_obj->reference);
			pdf->testObjAnalysis->active_content++;
			pdf->testObjAnalysis->js++;

			// Launch js content analysis			
			if (unknownPatternRepetition(js, strlen(js), pdf, js_obj) < 0){
				err_log("getJavaScript :: get pattern high repetition failed!\n");
				return -1;
			}
			
			if (findDangerousKeywords(js, pdf, js_obj) < 0){
				err_log("getJavaScript :: get dangerous keywords failed!\n");
				return -1;
			}
		}
		else{
			warn_log("getJavaScript :: Empty js content in object %s\n", obj->reference);
		}



	}else{

		// get js content in dictionary string.
		js = getDelimitedStringContent(start,"(",")",len);

		if (js != NULL){

			dbg_log("getJavaScript :: Found JS content in object %s\n", obj->reference);

			pdf->testObjAnalysis->active_content++;
			pdf->testObjAnalysis->js++;

			// Launch js content analysis
			if (unknownPatternRepetition(js, strlen(js), pdf, obj) < 0){
				err_log("getJavaScript :: get pattern high repetition failed!\n");
				free(js);
				return -1;
			}
			
			if (findDangerousKeywords(js, pdf, obj) < 0){
				err_log("getJavaScript :: get dangerous keywords failed!\n");
				free(js);
				return -1;
			}

			free(js);

		}
		else{
			warn_log("getJavaScript :: Empty js content in object %s\n", obj->reference);
		}
	
	}


	return ret;

}


/*
getJSContentInXFA() :: Get and analyze JavaScript content in XFA form description (xml).
parameters:
- struct pdfDocument * pdf (pdf document pointer)
- struct pdfObject* obj
returns: (int)
- 1 if found
- 0 if not found
- an error code (<0) on error.
TODO :: getJSContentInXFA :: Check the keyword javascript
*/
int getJSContentInXFA(char * stream, int size, struct pdfObject * obj, struct pdfDocument * pdf){
	
	
	int len = 0;
	int rest = 0;
	int found = 0;
	char * start = NULL;
	char * end = NULL;
	char * js_content = NULL;
	char * script_balise = NULL;	
	char * tmp = NULL;
	

	if (stream == NULL || size <= 0 || obj == NULL || pdf == NULL){
		err_log("getJSContentInXFA :: invalid parameter\n");
		return -1;
	}

	end = stream;
	rest = size;

	while((start = searchPattern(end,"<script",7,rest)) != NULL && rest > 0){

		
		dbg_log("getJSContentInXFA :: javascript content found in %s\n",obj->reference);
		

		rest = (int)(start-stream);
		rest = size - rest;
		end = start;
		len = 0;
		while(end[0] != '>' && len<=rest ){
			end ++;		
			len ++;
		}

		script_balise = (char*)calloc(len+1,sizeof(char));
		script_balise[len]= '\0';
		if(len > 0)
			memcpy(script_balise,start,len);
		//dbg_log("getJSContentInXFA :: script_balise = %s\n",script_balise);

		// save the script start ptr
		tmp = start;

		// search the </script> balise
		rest = (int)(end-stream);
		rest = size - rest;

		start = searchPattern(end,"</script",8,rest);

		if(start == NULL){
			free(script_balise);
			return -1;
		}

		rest = (int)(start-stream);
		rest = size - rest;

		end = start;
		len = 0;
		while(end[0] != '>' && len<=rest ){
			end ++;		
			len ++;
		}

		len = (int)(end - tmp);
		len ++;


		js_content = (char*)calloc(len+1, sizeof(char));
		js_content[len]='\0';

		memcpy(js_content,tmp,len);

		//dbg_log("getJSContentInXFA :: js_content = %s\n",js_content);
		found = 1;
		
		// Launch js content analysis
		if (unknownPatternRepetition(js_content, len, pdf, obj) < 0){
			err_log("getJSContentInXFA :: get pattern high repetition failed!\n");
			free(script_balise);
			free(js_content);
			return -1;
		}

		if (findDangerousKeywords(js_content, pdf, obj) < 0){
			err_log("getJSContentInXFA :: get dangerous keywords failed!\n");
			free(script_balise);
			free(js_content);
			return -1;
		}
	
		rest = (int)(start-stream);
		rest = size - rest;


		free(script_balise);
		free(js_content);

	}

	if(found == 1){
		pdf->testObjAnalysis->js ++;
	}


	return found;
}


/*
getXFA() ::  Get XFA form in the document.
parameters:
- struct pdfDocument * pdf (pdf document pointer)
- struct pdfObject* obj
returns: (int)
- 1 if found
- 0 if not found
- an error code (<0) on error.
*/
int getXFA(struct pdfDocument * pdf, struct pdfObject* obj){

	char * xfa = NULL;
	char * xfa_obj_ref = NULL;
	char * start = NULL;
	char * end = NULL;
	char * obj_list = NULL;	
	int len = 0;
	int len2 = 0;
	int ret = 0;
	struct pdfObject * xfa_obj = NULL;

	if (pdf == NULL || obj == NULL){
		err_log("getXFA :: invalid parameters\n");
		return -1;
	}

	if( obj->dico == NULL ){
		//dbg_log("getXFA :: No dictionary in object %s\n",obj->reference);
		return 0;
	}

	start = searchPattern(obj->dico, "/XFA" , 4 , strlen(obj->dico));

	if(start == NULL){
		//dbg_log("getXFA :: No XFA entry detected in object dictionary %s\n",obj->reference);
		return 0;
	}

	dbg_log("getXFA :: XFA Entry in dictionary detected in object %s\n", obj->reference);
	//dbg_log("getXFA :: dictionary = %s\n", obj->dico);

	start += 4;

	// If there is a space // todo put a while
	if(start[0] == ' '){
		start ++;
	}

	len = strlen(obj->dico) - (int)(start - obj->dico);
	
	// Get xfa object references

	// If its a list get the content
	if(start[0] == '['){

		obj_list =  getDelimitedStringContent(start,"[", "]", len); 

		if(obj_list == NULL){
			err_log("getXFA :: Can't get object list in dictionary\n");
			return -1;
		}
		
		end = obj_list;
		len2 = strlen(obj_list);


		// get XFA object reference in array ::
		while( (xfa_obj_ref = getIndirectRefInString(end, len2)) ){

			//dbg_log("getXFA :: xfa_obj_ref = %s\n",xfa_obj_ref);

			end = searchPattern(end, xfa_obj_ref , 4 , len2); // change value 4
			end += strlen(xfa_obj_ref) - 2;

			len2 = (int)(end - obj_list);
			len2 = strlen(obj_list) - len2;

			// get xfa object 
			xfa_obj =  getPDFObjectByRef(pdf, xfa_obj_ref);
			if(xfa_obj == NULL){				
				err_log("getXFA :: Object %s containing xfa not found\n",xfa_obj_ref);				
				continue;
			}

			// Decode object stream
			if (xfa_obj->filters != NULL){
				if (decodeObjectStream(xfa_obj) < 0){
					err_log("getXFA :: decode object stream failed!\n");
					// if decoding object stream failed then continue.
					pdf->errors++;					
					continue;
				}
			}
		

			// get xfa content
			if(xfa_obj->decoded_stream != NULL ){
				xfa = xfa_obj->decoded_stream;
			}else{
				xfa = xfa_obj->stream;
			}

			if(xfa != NULL){

				// dbg_log("getXFA :: xfa content = %s\n",xfa);

				pdf->testObjAnalysis->active_content ++;
				pdf->testObjAnalysis->xfa ++;
				ret++;

				if (getJSContentInXFA(xfa, strlen(xfa), xfa_obj, pdf) < 0){
					err_log("getXFA :: get js content in xfa failed!\n");
				}

				dbg_log("getXFA :: Found XFA content in object %s\n",xfa_obj_ref);
				
				// Analyze xfa content 
				/* This part is commented because only js content in XFA form will be analyzed.*/
				//unknownPatternRepetition(xfa, strlen(xfa),pdf, xfa_obj);
				//findDangerousKeywords(xfa,pdf,xfa_obj);
				
			}else{
				
				warn_log("getXFA :: Empty XFA content in object %s\n",xfa_obj_ref);				
			}


		}

		free(obj_list);

	}else{
				
		len2 = (int)(start - obj->dico);
		len2 = strlen(obj->dico) - len2;


		xfa_obj_ref = getIndirectRefInString(start, len2);
		if(xfa_obj_ref == NULL){
			err_log("getXFA :: get xfa object indirect reference failed\n");			
			return -1;
		}

		//dbg_log("getXFA :: xfa_obj_ref = %s\n",xfa_obj_ref);

		xfa_obj =  getPDFObjectByRef(pdf, xfa_obj_ref);
		if(xfa_obj == NULL){
			err_log("getXFA :: Object %s not found\n",xfa_obj_ref);
			free(xfa_obj_ref);
			return -1;
		}

		// Decode object stream
		if (xfa_obj->filters != NULL){
			if (decodeObjectStream(xfa_obj) < 0){
				err_log("getXFA :: decode object stream failed!\n");
				// if decoding object stream failed then continue.
				pdf->errors++;
				free(xfa_obj_ref);
				return -1;
			}
		}

		// get xfa content
		if(xfa_obj->decoded_stream != NULL ){
			xfa = xfa_obj->decoded_stream;
		}else{
			xfa = xfa_obj->stream;
		}

		if(xfa != NULL){

			dbg_log("Found XFA content in object %s\n",xfa_obj_ref);
			ret++;
			//dbg_log("getXFA :: xfa content = %s\n",xfa);

			pdf->testObjAnalysis->active_content++;
			pdf->testObjAnalysis->xfa++;
			
			if (getJSContentInXFA(xfa, strlen(xfa), xfa_obj, pdf) < 0){
				err_log("getXFA :: get js content in xfa failed!\n");
			}

			// Analyze xfa content
			/* This part is commented because only js content in XFA form will be analyzed.*/
			//unknownPatternRepetition(xfa, strlen(xfa), pdf, xfa_obj);
			//findDangerousKeywords(xfa,pdf,xfa_obj);
			
		}else{
			warn_log("getXFA :: Empty XFA content in object %s\n",xfa_obj_ref);			
		}

		free(xfa_obj_ref);

	}
	
	return ret;

}


/*
getEmbeddedFile() ::  Get Embedded file content and analyze it.
parameters:
- struct pdfDocument * pdf (pdf document pointer)
- struct pdfObject* obj
returns: (int)
- 1 if found
- 0 if not found
- an error code (<0) on error.
*/
int getEmbeddedFile(struct pdfDocument * pdf , struct pdfObject* obj){

	char * ef = NULL;
	struct pdfObject * ef_obj = NULL;
	char * ef_obj_ref = NULL;
	char * start = NULL;
	//char * subdir = NULL;
	int len = 0;
	int ret = 0;

	//dbg_log("getEmbeddedFile :: Analysing object :: %s\n",obj->reference);
	if (pdf == NULL || obj == NULL){
		err_log("getEmbeddedFile :: invalid parameter\n");
		return -1;
	}
	
	if(obj->dico == NULL || obj->type == NULL){
		return 0;
	}

	// Get by Type or by Filespec (EF entry)

	if( strncmp(obj->type,"/EmbeddedFile",13) == 0){


		// Decode object stream	
		if (obj->filters != NULL){

			if (decodeObjectStream(obj) < 0){
				err_log("getEmbeddedFile :: decode object stream failed!\n");
				// if decoding object stream failed then exit.
				pdf->errors++;
				return -2;
			}
		}

		if(obj->decoded_stream != NULL ){
			ef = obj->decoded_stream;
		}else{
			ef = obj->stream;
		}

		if(ef != NULL){
			dbg_log("getEmbeddedFile :: Found EmbeddedFile object %s\n",obj->reference);			
			//dbg_log("getEmbeddedFile :: ef content = %s\n",ef);
			// TODO Process to ef stream content analysis.
			//unknownPatternRepetition(ef, strlen(xfa), pdf, xfa_obj);
			//findDangerousKeywords(xfa,pdf,obj);
			
		}else{
			warn_log("getEmbeddedFile :: Empty EF stream content in object %s\n",obj->reference);			
		}


	}


	// looking for Filespec object is not always necessary for Embedded file detection
	/*if( strncmp(obj->type,"/Filespec",9) == 0){

		// Get EF entry in dico
		start = searchPattern(obj->dico, "/EF" , 3 , strlen(obj->dico));

		if(start == NULL){
			//dbg_log("getEmbeddedFile :: No EF detected in object %s\n",obj->reference);
			return 0;
		}

		dbg_log("getEmbeddedFile :: Found EmbeddedFile in file specification %s\n",obj->reference);
		

		start += 3;

		// For white spaces
		while(start[0] == ' '){
			start ++;
		}


		// The case <</EF <</F 3 0 R>>
		if(start[0] == '<' && start[1] == '<'){

			//printf("Debug:: start = %s\n", start);

			len = (int)(start - obj->dico);
			len = strlen(obj->dico) - len;

			// get Embedded file sub dictionary
			if( (subdir = getDelimitedStringContent(start, "<<", ">>", len)) == NULL){
				warn_log("getEmbeddedFile :: get EF subdirectory failed!\n");
				ef_obj= NULL;
				ef_obj_ref = NULL;
			}

			//printf("Debug :: subdir = %s\n", subdir);


			ef_obj_ref = obj->reference;
			ef_obj = obj;

			free(subdir);
			subdir=NULL;

		}else{

			len = (int)(start - obj->dico);
			len = strlen(obj->dico) - len;
			// get indirect ref of the 
			ef_obj_ref = getIndirectRef(start,len);
			//dbg_log("getEmbeddedFile :: ef_obj_ref = %s\n",ef_obj_ref);
			ef_obj = getPDFObjectByRef(pdf,ef_obj_ref);



		}


		if(ef_obj != NULL){


			if(ef_obj->dico == NULL){
				warn_log("getEmbeddedFile :: No dictionary found in object %s\n",ef_obj_ref);				
				return -1;
			}
			// Get the /F entry in the dico
			start = searchPattern(ef_obj->dico, "/F" , 2 , strlen(ef_obj->dico));

			//printf("Debug :: start = %s :: dico_len = %d\n",start, strlen(ef_obj->dico));

			if(start == NULL){
				dbg_log("No EF detected in object %s\n",obj->reference);
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
			dbg_log("getEmbeddedFile :: EF_obj_ref = %s\n",ef_obj_ref);

			ef_obj = getPDFObjectByRef(pdf,ef_obj_ref);

			if(ef_obj != NULL){

				// Decode object stream
				if (ef_obj->filters != NULL){
					if (decodeObjectStream(ef_obj) < 0){
						err_log("getEmbeddedFile :: decode object stream failed!\n");
						// if decoding object stream failed then exit.
						pdf->errors++;
						return -2;
					}
				}

				if(ef_obj->decoded_stream != NULL ){
					ef = ef_obj->decoded_stream;
				}else{
					ef = ef_obj->stream;
				}

				if( ef != NULL){
					dbg_log("getEmbeddedFile :: Found EmbeddedFile object %s\n",ef_obj_ref);					
					dbg_log("getEmbeddedFile :: ef content = %s\n",ef);
					// TODO Process to ef stream content analysis.
				}else{
					warn_log("getEmbeddedFile :: Empty EF stream content in object %s\n",obj->reference);					
				}

			}else{
				warn_log("getEmbeddedFile :: object not found %s\n",ef_obj_ref);				
			}



		}else{
			warn_log("getEmbeddedFile :: object not found %s\n",ef_obj_ref);			
		}
		

	}*/

	if(ef != NULL){

		pdf->testObjAnalysis->active_content ++;
		pdf->testObjAnalysis->ef ++;

		// TODO :: launch ef content analysis.
	}

	return ret;
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
				continue;
			}

			if (res > 0){
				warn_log("getInfoObject :: found potentially malicious /Info object %s\n", info_ref);
				pdf->testObjAnalysis->dangerous_keyword_high++; // TODO set another variable for this test :: MALICIOUS_INFO_OBJ
			}

			res = findDangerousKeywords(info, pdf, info_obj);
			if(res < 0){
				err_log("getJavaScript :: get dangerous keywords failed!\n");
				continue;
			}

			if(res > 0){
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

		if (getActions(pdf, obj) < 0){
			err_log("getDangerousContent :: get dangerous actions failed!\n");
			return -1;
		}

		if (getJavaScript(pdf, obj) < 0){
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