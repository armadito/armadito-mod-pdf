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



#include <armaditopdf.h>
#include <armaditopdf/analysis.h>
#include <armaditopdf/utils.h>
#include <armaditopdf/osdeps.h>
#include <armaditopdf/log.h>
#include <armaditopdf/errors.h>



int check_xref_obj(struct pdfDocument * pdf, char * xref_obj_ref){

	int retcode = ERROR_SUCCESS;
	struct pdfObject * obj;

	if(pdf == NULL || xref_obj_ref == NULL){
		return ERROR_INVALID_PARAMETERS;
	}

	obj = getPDFObjectByRef(pdf,xref_obj_ref);
	if(obj == NULL){
		warn_log("check_xref_obj :: obj ref %s not found!\n");
		return ERROR_OBJ_REF_NOT_FOUND;
	}

	if(obj->type != NULL && strcmp(obj->type,"/XRef")== 0){

		// check if the pdf is encrypted.
		if( searchPattern(obj->dico, "/Encrypt",8,strlen(obj->dico)) != NULL  ){
			warn_log("check_xref_obj :: Encrypted content found!\n");
			retcode = ERROR_ENCRYPTED_CONTENT;
		}

	}else{

		retcode = ERROR_INVALID_XREF_OFFSET;
	}

	return retcode;
}


int check_xref_table_entry(struct pdfDocument * pdf, char * entry){

	int retcode = ERROR_SUCCESS;
	int len = 0;
	char * off_s;
	char * gen_s;

	if( pdf == NULL || entry == NULL ){
		return ERROR_INVALID_PARAMETERS;
	}

	len = strlen(entry);

	off_s = getNumber_s(entry, len);
	if (off_s == NULL || strlen(off_s) != 10){
		err_log("check_xref_table_entry :: bad offset format in xref entry! :: offset = %s :: entry = %s\n", off_s,entry);
		return ERROR_INVALID_XREF_FORMAT;
	}
	free(off_s);

	// skip 10 bytes corresponding to obj offset.
	entry += 10;
	len -= 10;

	// check white space between offset and generation number.
	if (*entry != ' '){
		err_log("check_xref_table_entry :: bad xref format!\n");
		return ERROR_INVALID_XREF_FORMAT;
	}

	// skip white space
	entry++;
	len--;

	// Get generation number.
	gen_s = getNumber_s(entry, len);
	if(gen_s == NULL || (gen_s != NULL && strlen(gen_s) != 5) ){
		err_log("check_xref_table_entry :: bad generation number format in xref table!\n");
		return ERROR_INVALID_XREF_FORMAT;
	}

	if (strlen(gen_s) != 5){
		err_log("check_xref_table_entry :: bad generation number format in xref table! :: gen_number = %s\n",gen_s);
		free(gen_s);
		return ERROR_INVALID_XREF_FORMAT;
	}
	free(gen_s);

	// skip 10 bytes corresponding to obj gen number.
	entry += 5;
	len -= 5;

	// check white space between generation number and free flag.
	if (*entry != ' '){
		err_log("check_xref_table_entry :: bad xref format!\n");
		return ERROR_INVALID_XREF_FORMAT;
	}

	// skip white space
	entry++;
	len--;

	if(*entry  != 'n' && *entry != 'f'){
		err_log("check_xref_table_entry :: bad xref format!\n");
		return ERROR_INVALID_XREF_FORMAT;
	}

	// skip flag and white space.
	entry+=2;
	len-=2;

	return ERROR_SUCCESS;
}


int check_xref_table(struct pdfDocument * pdf, char * xref, unsigned int xref_size){

	int retcode = ERROR_SUCCESS;
	char * tmp;
	int len = 0;
	int i;
	int first_obj_num, num_entries;
	char * first_obj_num_s;
	char * num_entries_s;
	char entry[19] = {0};
	int entry_len = 18;


	if( pdf == NULL || xref == NULL || xref_size == 0){
		err_log("check_xref_table :: invalid parameters\n");
		return ERROR_INVALID_PARAMETERS;
	}

	// skip "xref" keyword
	tmp =  xref + 4;
	len = xref_size - 4;

	// skip white space
	while(tmp[0] == '\r' || tmp[0] == '\n' || tmp[0] == ' '){
		tmp++;
		len --;
	}

	// Get the object number of the first object described in xref table
	first_obj_num_s = getNumber_s(tmp,len);
	if (first_obj_num_s == NULL){
		err_log("check_xref_table :: can't get first object number\n");
		return ERROR_INVALID_XREF_FORMAT;
	}

	// skip number + white space
	tmp += (strlen(first_obj_num_s) +1);
	len -= (strlen(first_obj_num_s) +1);

	free(first_obj_num_s);

	// Get the number of entries in the xref table
	num_entries_s = getNumber_s(tmp,len);
	if (num_entries_s == NULL){
		err_log("check_xref_table :: can't get number of entries in xref table\n");
		return ERROR_INVALID_XREF_FORMAT;
	}
	num_entries = atoi(num_entries_s);

	// skip number
	tmp += strlen(num_entries_s);
	len -= strlen(num_entries_s);
	free(num_entries_s);

	// hint after the number of entries it should be '\r' or '\n' not a space.
	if (*tmp != '\r' && *tmp != '\n'){
		err_log("check_xref_table :: bad xref format!1\n");
		return ERROR_INVALID_XREF_FORMAT;
	}

	// skip white space.
	tmp++;
	len--;

	for(i = 0; i< num_entries ; i++){

		// get entry
		if(len >= entry_len){
			memcpy(entry, tmp,entry_len);
			//printf("entry = %s\n",entry);
			retcode = check_xref_table_entry(pdf, entry);
		}

		tmp+=entry_len;

		// skip eol
		while(tmp[0] == '\r' || tmp[0] == '\n'){
			tmp++;
			len --;
		}
	}

	return retcode;
}


int pdf_check_xref(struct pdfDocument * pdf, unsigned int xref_offset){

	int retcode = ERROR_SUCCESS;
	char * xref;
	char * start;
	int len = 0;

	if(pdf == NULL || xref_offset <= 0){
		err_log("check_xref :: invalid parameters!\n");
		return ERROR_INVALID_PARAMETERS;
	}

	// Goto the xref offset and check the "xref" keyword.
	xref = (char*)calloc(5, sizeof(char));
	xref[4] = '\0';

	if(pdf->fh != NULL){

		fseek(pdf->fh, 0, SEEK_SET);
		fseek(pdf->fh, xref_offset, SEEK_SET);
		fread(xref, 1, 4, pdf->fh);
	}
	else if (pdf->fd >= 0){

		os_lseek(pdf->fd, 0, SEEK_SET);
		os_lseek(pdf->fd, xref_offset, SEEK_SET);
		os_read(pdf->fd, xref, 4);
	}
	else{
		err_log(" check_xref :: invalid file handle or file descriptor\n");
		return ERROR_INVALID_FD;
	}

	if( strcmp(xref, "xref") == 0){

		free(xref);
		// TOFIX: real offset will change if comments have been removed.
		start = pdf->content + xref_offset;
		len = pdf->size - (int)(start - pdf->content);

		xref = getDelimitedStringContent(start, "xref" , "trailer" , len);
		if(xref == NULL){
			return ERROR_INVALID_XREF_FORMAT;
		}

		retcode = check_xref_table(pdf, xref, strlen(xref));
		free(xref);

	}else{

		free(xref);
		start = pdf->content + xref_offset;
		len = pdf->size - (int)(start - pdf->content);

		// get object reference at this offset.
		xref = get_obj_ref(start,len);
		if(xref == NULL){
			return ERROR_INVALID_XREF_OFFSET;
		}

		retcode = check_xref_obj(pdf, xref);
		free(xref);

	}

	return retcode;
}


int pdf_check_trailer(struct pdfDocument * pdf, struct pdfTrailer * trailer){

	int retcode = ERROR_SUCCESS;
	char * start = NULL;
	int len = 0;
	int xref_offset;

	if(pdf == NULL || trailer == NULL){
		err_log("check_trailer :: invalid parameters!\n");
		return ERROR_INVALID_PARAMETERS;
	}

	// check if the pdf is encrypted.
	if(trailer->dico != NULL && searchPattern(trailer->dico, "/Encrypt",8,strlen(trailer->dico)) != NULL ){
		warn_log("check_trailer :: Encrypted content found!\n");
		return ERROR_ENCRYPTED_CONTENT;
	}

	// get the offset of the XRef object
	start = searchPattern(trailer->content, "startxref", 9 , strlen(trailer->content));
	if(start == NULL){
		err_log("check_trailer :: XRef offset not found in trailer\n");
		return ERROR_BAD_TRAILER_FORMAT;
	}

	start += 9;
	while(start[0] == '\r' || start[0] == '\n' || start[0] == ' '){
		start ++;
	}

	len = trailer->size - (int)(start - trailer->content);
	xref_offset = getNumber(start,len);

	if(xref_offset <= 0 || xref_offset > pdf->size ){
		warn_log("check_trailer :: Bad Wrong xref object offset %d\n",xref_offset);
		return ERROR_BAD_TRAILER_FORMAT;
	}

	// if the offset is higher than th PDF size
	if(xref_offset > pdf->size){
		warn_log("check_trailer :: Wrong xref object offset %d\n",xref_offset);
		return ERROR_BAD_TRAILER_FORMAT;
	}

	// check the xref table or object offset.
	retcode = pdf_check_xref(pdf,xref_offset);

	return retcode;
}


int pdf_check_valid_trailer(struct pdfDocument * pdf){

	int retcode = ERROR_SUCCESS;
	struct pdfTrailer * trailer;

	if(pdf == NULL || pdf->trailers == NULL){
		err_log("check_valid_trailer :: invalid parameters!\n");
		return ERROR_INVALID_PARAMETERS;
	}

	trailer = pdf->trailers;

	while(trailer != NULL){

		retcode = pdf_check_trailer(pdf, trailer);
		if(retcode == ERROR_SUCCESS)
			return retcode;

		trailer = trailer->next;
	}

	return retcode;
}


/*
documentStructureAnalysis() :: check the consitency of the Cross-reference table
parameters:
- struct pdfDocument * pdf (pdf document pointer)
returns: (int)
- 0 on success.
- an error code (<0) on error.
*/
int checkXRef(struct pdfDocument * pdf){

	int ret = 1;
	int xref_offset = 0;
	int len = 0;
	int num_entries = 0;
	int obj_num = 0;
	int first_obj_num = 0;
	int i = 0;
	int ref_size = 12; // xxxx 0 obj
	int off = 0;
	int gen = 0; // generation number

	char * off_s = NULL;
	char * gen_s = NULL;
	char * start = NULL;
	char * xref = NULL;
	char * xref_orig = NULL;
	char * num_entries_a = NULL;
	char * obj_num_a = NULL;
	char * first_obj_num_a = NULL;
	char * ref = NULL;

	char free_obj;

	struct pdfObject * obj = NULL;
	struct pdfTrailer * trailer = NULL;
	

	if(pdf == NULL){
		err_log("checkXRef :: invalid parameter\n");
		return -1;
	}


	if(pdf->trailers == NULL){
		err_log("checkXRef :: No trailer found in pdfDocument\n");
		return -1;
	}

	trailer = pdf->trailers;

	while(trailer != NULL){

		// Get xref offset
		if(trailer->content == NULL){
			err_log("checkXRef :: Empty trailer content\n");
			trailer = trailer->next;
			continue;
		}
		
		start = searchPattern(trailer->content, "startxref", 9 , strlen(trailer->content));
		if(start == NULL){
			dbg_log("checkXRef :: XRef offset not found in trailer\n");
			return -1;
		}

		start += 9; // 9 => startxref.
		while(start[0] == '\r' || start[0] == '\n' || start[0] == ' '){
			start ++;
		}

		len =  (int)(start - trailer->content);
		len = strlen(trailer->content) - len;

		xref_offset = getNumber(start,len);
		if (xref_offset < 0){
			err_log("checkXRef :: get xref offset failed!\n");
			return -1;
		}
		
		// Goto the xref offset and check the "xref" keyword.		
		xref = (char*)calloc(5, sizeof(char));
		xref[4] = '\0';

		if(pdf->fh != NULL){

			fseek(pdf->fh, 0, SEEK_SET);
			fseek(pdf->fh, xref_offset, SEEK_SET);
			fread(xref, 1, 4, pdf->fh);

		}
		else if (pdf->fd >= 0){

			os_lseek(pdf->fd, 0, SEEK_SET);
			os_lseek(pdf->fd, xref_offset, SEEK_SET);
			os_read(pdf->fd, xref, 4);
			
		}
		else{
			err_log("checkXRef :: invalid file handle or file descriptor\n");
			free(xref);
			return -1;
		}

		
		//dbg_log("checkXRef :: xref keyword = %s\n",xref);


		if(memcmp(xref,"xref",4) == 0){


			//dbg_log("checkXRef :: Good xref table offset : %d\n",xref_offset);
			start = pdf->content;
			start += xref_offset;

			len = (int)(start - pdf->content);
			len = pdf->size -len ;

			free(xref);

			// Get xref table content from pdf document content
			xref_orig = getDelimitedStringContent( start, "xref" , "trailer" , len);
			xref = xref_orig;


			if(xref == NULL){
				err_log("checkXRef :: Error while getting the xref table\n");				
				return -1;
			}

			//dbg_log("checkXRef :: xref table content = \n%s\n",xref);

			// shift "xref" keyword
			xref += 4;

			len = strlen(xref) - 4;

			while(xref[0] == '\r' || xref[0] == '\n' || xref[0] == ' '){
				xref++;
				len --;
			}


			// Get the object number of the first object described in xref table
			first_obj_num_a = getNumber_s(xref,len);
			if (first_obj_num_a == NULL){
				err_log("checkXRef :: can't get first object number\n");
				//ret = unexpected_error;
				// goto_clean;
				free(xref_orig);
				return bad_xref_format;				
			}

			first_obj_num = atoi(first_obj_num_a);


			//dbg_log("checkXRef :: first_obj_num = %d\n",first_obj_num);

			len -= strlen(first_obj_num_a);
			xref += strlen(first_obj_num_a);

			// move for white space
			len --;
			xref ++;


			// get the number of entries in the xref table
			num_entries_a = getNumber_s(xref,len);
			if (num_entries_a == NULL){
				err_log("checkXRef :: can't get number of entries\n");
				free(xref_orig);
				free(first_obj_num_a);
				return bad_xref_format;
			}

			num_entries = atoi(num_entries_a);

			//dbg_log("checkXRef :: num_entries = %d\n",num_entries);

			len -= strlen(num_entries_a);
			xref += strlen(num_entries_a);

			// move for white space
			// Check xref format.
			

			// hint after the number of entries it should be '\r' or '\n' not a space.
			if (*xref != '\r' && *xref != '\n'){
				err_log("checkXref :: bad xref format!\n");
				free(xref_orig);
				free(num_entries_a);
				free(first_obj_num_a);
				return bad_xref_format;
			}

			// skip white spaces
			while (*xref == ' ' || *xref == '\r' || *xref == '\n'){
				len--;
				xref++;
			}
			

			// For each entry of table
			for(i = 0; i< num_entries ; i++){


				// TODO :: check offset length. :: use get_Number_s first.
				off_s = getNumber_s(xref, len);
				if (off_s == NULL || strlen(off_s) != 10){					
					err_log("chackXref :: bad offset format in xref table! :: offset = %s :: xref = %s\n", off_s,xref);
					ret = bad_xref_format;
					goto clean;
				}

				free(off_s);
				off_s = NULL;

				off = getNumber(xref,len);

				// skip 10 bytes corresponding to obj offset.
				xref += 10;
				len -= 10;

				// check white space between offset and generation number.
				if (*xref != ' '){
					err_log("chackXref :: bad xref format!\n");
					ret = bad_xref_format;
					goto clean;
				}

				// skip white space between offset and generation number..
				xref++;
				len--;


				// Get generation number.
				gen_s = getNumber_s(xref, len);
				if(gen_s == NULL){
					err_log("checkXref :: bad generation number format in xref table!\n");
					ret = bad_xref_format;
					goto clean;
				}

				if (strlen(gen_s) != 5){
					err_log("checkXref :: bad generation number format in xref table! :: gen_number = %s\n",gen_s);
					ret = bad_xref_format;
					free(gen_s);
					goto clean;
				}

				free(gen_s);
				//gen = getNumber(xref, len);

				// skip 10 bytes corresponding to obj gen number.
				xref += 5;
				len -= 5;

				// check white space between generation number and free flag.
				if (*xref != ' '){
					err_log("chackXref :: bad xref format!\n");
					ret = bad_xref_format;
					goto clean;
				}

				// skip white space
				xref++;
				len--;

				
				//xref += 17;

				free_obj = xref[0];

				obj_num = first_obj_num + i;

				dbg_log("checkXRef :: object number = %d :: offset = %d :: free = %c\n",obj_num,off,free_obj);

				ref = (char*)calloc(ref_size+1,sizeof(char));
				
				os_sprintf(ref,ref_size+1,"%d 0 obj",obj_num);
				
				//dbg_log("checkXRef :: ref = %s at %d\n",ref, off);

				// get object by ref
				if(obj_num > 0){
					obj = getPDFObjectByRef(pdf,ref);
				}

				if( obj_num > 0 && obj == NULL){

					warn_log("checkXRef :: object not found %s--\n",ref);

				}else{

					
					if(obj_num > 0 && free_obj == 'n' && obj != NULL && off != obj->offset ){
						
						
						//warn_log("checkXRef :: Bad offset for object %s :: %d differs from %d\n",ref,off,obj->offset);

						// the object could be defnied more than once
						if(pdf->testStruct->object_collision > 0){

							while( (obj = getPDFNextObjectByRef(pdf,obj,ref)) != NULL ){

								if(obj_num > 0 && free_obj == 'n' && off != obj->offset ){
						
									//warn_log("Warning :: checkXRef :: Bad offset for object %s :: %d differs from %d\n",ref,off,obj->offset);
									
									pdf->testStruct->bad_obj_offset ++;
									ret = 0;
								}else{
									ret = 1;
								}
							}

						}else{
							
							//warn_log("checkXRef :: Bad offset for object %s :: %d differs from %d\n",ref,off,obj->offset);

							pdf->testStruct->bad_obj_offset ++;
							ret = 0;
						}
						

					}

				}

				// skip "free object" flag. ( 'f' or 'n' )
				xref++;
				len--;

				// skip white spaces.				
				while (*xref == '\r' || *xref == '\n' || *xref == ' '){
					xref += 1;
					len -= 1;
				}
				//dbg_log("xref = %s\n", xref);

				free(ref);
				ref = NULL;

			}

			free(xref_orig);
			xref_orig = NULL;
			

		}else{

			// get the xref object.

			// if the offset is higher than th PDF size
			if(xref_offset > pdf->size){
				warn_log("checkXRef :: Wrong Xref table (or xref object) offset %d\n",xref_offset);
				ret = 0;
				pdf->testStruct->bad_xref_offset ++;
				trailer = trailer->next;
				free(xref);
				continue;
			}
			
			//dbg_log("checkXRef :: xref keyword = %s\n",xref);

			start = pdf->content;
			start += xref_offset;


			len = (int)(start - pdf->content);
			len = pdf->size -len ;

			//  Check if the offset point to a Xref Object type /XRef
			obj_num_a = getNumber_s(start,len);

			if(obj_num_a == NULL){
				pdf->testStruct->bad_xref_offset ++;
				trailer = trailer->next;
				free(xref);
				continue;
			}
			
			obj_num = atoi(obj_num_a);

			len = strlen(obj_num_a) + 7;

			

			ref = (char*)calloc(len+1,sizeof(char));
			ref[len] = '\0';

			os_sprintf(ref,len,"%d 0 obj",obj_num);
			//dbg_log("xref object = %s\n",ref);

			free(obj_num_a);
			obj_num_a = NULL;

			obj = getPDFObjectByRef(pdf,ref);



			if(obj != NULL){
				
				if(obj->type == NULL || memcmp(obj->type,"/XRef",5) != 0){

					warn_log("checkXRef :: Wrong Xref table (or xref object) offset %d\n",xref_offset);					

					//dbg_log("checkXRef ::type = %s\n",obj->type);

					ret = 0;
					pdf->testStruct->bad_xref_offset ++;


				}else{ // if the xref object is found

					//dbg_log("checkXRef :: XRef obj Dico =  %s\n",obj->dico);

					// Check if the document is encrypted 
					if( searchPattern(obj->dico, "/Encrypt",8,strlen(obj->dico)) != NULL  ){
						pdf->testStruct->encrypted ++;
					}

					pdf->testStruct->bad_xref_offset = 0;
				}

			}else{

				warn_log("checkXRef :: checkXRef :: object not found %s\n",ref);
				//pdf->testStruct->bad_xref_offset ++;

			}

			free(ref);
			ref = NULL;
			free(xref);

		}

	clean:

		if (off_s != NULL){
			free(off_s);
			off_s = NULL;
		}

		if (xref_orig != NULL){
			free(xref_orig);
			xref_orig = NULL;
		}

		if (num_entries_a != NULL){
			free(num_entries_a);
			num_entries_a = NULL;
		}
		
		if (first_obj_num_a != NULL){
			free(first_obj_num_a);
			first_obj_num_a = NULL;
		}		


		trailer = trailer->next;

	}



	return ret;

}


/*
checkEmptyDocument() :: check if pages in the document are not all empty. => returns 1 if not empty
parameters:
- struct pdfDocument * pdf (pdf document pointer)
returns: (int)
- 1 if an non empty page is found on success.
- 0 if the document is empty.
- an error code (<0) on error.

TODO :: checkEmptyDocument :: Improve this function by creating a function getPagesKids which could be called recursively when the Kids objects reffers also to a /Pages object.
*/
int checkEmptyDocument(struct pdfDocument * pdf){

	int ret = 0;
	int len = 0;
	int len2 = 0;
	
	char * start = NULL;
	char * end = NULL;
	char * kids = NULL;
	char * kid_obj_ref = NULL;
	char * content_array = NULL;
	char * pageContents = NULL;
	char * pageContent_obj_ref = NULL;

	struct pdfObject * pageContent_obj = NULL;
	struct pdfObject * obj = NULL;
	struct pdfObject * kid_obj = NULL;
	
	
	if(pdf == NULL){
		err_log("checkEmptyDocument :: invalid parameter\n");		
		return -1;
	}


	obj = pdf->objects;

	while(obj != NULL){

		if(obj->type != NULL && strncmp(obj->type,"/Pages",6) == 0){

			dbg_log("checkEmptyDocument :: Found /Pages object :: %s\n", obj->reference);
			//dbg_log("Dico = %s\n",obj->dico);

			// get kids pages
			start = searchPattern(obj->dico, "/Kids", 5 , strlen(obj->dico));
			if(start == NULL){
				warn_log("checkEmptyDocument :: no kids entry in pages dictionary %s\n",obj->reference);				
				obj = obj->next; // go to the next object
				continue;
			}

			start += 5;

			len = (int)(start - obj->dico);
			len = strlen(obj->dico) -len;

			//while
			kids = getDelimitedStringContent(start,"[","]",len);
			dbg_log("kids = %s\n",kids);

			if (kids == NULL){
				// on error, got to next obj.
				goto next;
			}

			len = strlen(kids);
			end = kids;

			// get kids pages object references.
			while( (kid_obj_ref = getIndirectRefInString(end,len)) != NULL){

				
				dbg_log("checkEmptyDocument :: kid ref = %s\n",kid_obj_ref);

				end = searchPattern(end,kid_obj_ref,strlen(kid_obj_ref)-3,len);
				if(end == NULL){
					err_log("checkEmptyDocument :: end == NULL\n" );
					free(kids);
					free(kid_obj_ref);
					return -1;
				}

				end += strlen(kid_obj_ref) - 2;

				len = (int)(end - kids);
				len = strlen(kids) - len;

				if((kid_obj = getPDFObjectByRef(pdf,kid_obj_ref)) == NULL){
					warn_log("checkEmptyDocument :: Object %s not found\n", kid_obj_ref);						
					continue;
				}

				// check the type of the object
				if(kid_obj->dico != NULL && kid_obj->type != NULL && strncmp(kid_obj->type,"/Page",5) == 0 && strncmp(kid_obj->type,"/Page",strlen(kid_obj->type)) == 0 ){

					
					// TODO :: checkEmptyDocument :: write function (checkEmptyPage) to split this function.
					// checkEmptyPages(pdf,kid_obj);

					start = searchPattern(kid_obj->dico, "/Contents", 9 , strlen(kid_obj->dico));
					//dbg_log("checkEmptyDocument :: Page dico = %s\n",kid_obj->dico);

					if(start == NULL){
						warn_log("Warning :: checkEmptyDocument :: No page content in object %s\n",kid_obj_ref);						
						continue;
					}

					start += 9; // /Contents  => 9 strlen(Contents)

					// skip white spaces
					while(start[0] == ' '){
						start ++;
					}


					// if there is serveral content objects
					if(start[0] == '['){

						len2 = (int)(start - kid_obj->dico);
						len2 = strlen(kid_obj->dico) - len2;
						pageContents = getDelimitedStringContent(start,"[","]",len2);

						if(pageContents == NULL){
							warn_log("checkEmptyDocument :: getting Page content array failed !\n");							
							continue;
						}

						len2 = strlen(pageContents);
						start = pageContents;

						// get page content objects ref
						while( (pageContent_obj_ref = getIndirectRefInString(start,len2)) != NULL){

							dbg_log("checkEmptyDocument :: page content ref = %s\n",pageContent_obj_ref);

							start = searchPattern(start,pageContent_obj_ref,strlen(pageContent_obj_ref)-3,len2);

							if(start == NULL){
								err_log("checkEmptyDocument :: can't get page content object reference\n");
								free(pageContents);
								free(kids);
								free(kid_obj_ref);
								free(pageContent_obj_ref);
								return -1;
							}

							start += strlen(pageContent_obj_ref) - 2;

							len2 = strlen(pageContents) - (int)(end - pageContents);


							if((pageContent_obj = getPDFObjectByRef(pdf,pageContent_obj_ref)) == NULL){
								warn_log("Warning :: checkEmptyDocument :: Object %s not found \n", pageContent_obj_ref);
								free(pageContent_obj_ref);
								continue;
							}

							// get the stream
							if(pageContent_obj->stream != NULL && pageContent_obj->stream_size > 0){
								//dbg_log("checkEmptyDocument :: Page %s is not empty\n",kid_obj_ref);
								//return 1; // TODO ::  when you find a non-empty page then return. + time
								/* Do no forget to free the allocated variables
								free(pageContent_obj_ref);
								free(kid_obj_ref);
								free(kids);
								*/
								if (pageContent_obj_ref != NULL) {
									free(pageContent_obj_ref);
								}
								if (kid_obj_ref != NULL) {
									free(kid_obj_ref);
								}
								if (kids != NULL) {
									free(kids);
								}
								free(pageContents);
								dbg_log("checkEmptyDocument :: non empty page found\n");

								return 1;

								
							}else{

								warn_log("checkEmptyDocument :: Empty page content %s\n",pageContent_obj_ref);								
							}

							free(pageContent_obj_ref);

						}

						free(pageContents);


					}else{


						len2 = (int)(start - kid_obj->dico);
						len2 = strlen(kid_obj->dico) - len2;
						pageContent_obj_ref = getIndirectRef(start, len2);

						if(pageContent_obj_ref == NULL){
							warn_log("checkEmptyDocument :: Error while getting page content object reference \n");							
							continue;
						}

						if((pageContent_obj = getPDFObjectByRef(pdf,pageContent_obj_ref)) == NULL){
							warn_log("checkEmptyDocument :: Object not found %s\n", pageContent_obj_ref);
							free(pageContent_obj_ref);
							continue;
						}


						// get the stream
						if(pageContent_obj->stream != NULL && pageContent_obj->stream_size > 0){
							
							//dbg_log("checkEmptyDocument :: Page %s is not empty\n",kid_obj_ref);
							free(kids);
							free(kid_obj_ref);
							free(pageContent_obj_ref);
							return 1;

						}else{

							// Case when the content is an array ::
							content_array = getDelimitedStringContent(pageContent_obj->content,"[","]",pageContent_obj->content_size);

							//dbg_log("checkEmptyDocument :: content array = %s\n",content_array);
							if(content_array != NULL){

								start = content_array;
								len2 = strlen(content_array);
								dbg_log("checkEmptyDoc :: content = %s\n", start);
								dbg_log("checkEmptyDoc :: content_len = %d\n",len2);

								free(pageContent_obj_ref);

								while((pageContent_obj_ref = getIndirectRefInString(start,len2) ) != NULL){

									dbg_log("checkEmptyDocument :: page content ref = %s\n",pageContent_obj_ref );

									start = searchPattern(start,pageContent_obj_ref,strlen(pageContent_obj_ref)-3,len2);
									if (start == NULL){
										err_log("checkEmptyDocument :: can't retrieve object reference in dico\n");
										free(pageContent_obj_ref);
										break;
									}
									start += strlen(pageContent_obj_ref) - 2;

									len2 = (int)(start - content_array);
									len2 = strlen(content_array) - len2;


									if((pageContent_obj = getPDFObjectByRef(pdf,pageContent_obj_ref)) == NULL){
										warn_log("checkEmptyDocument :: Object not found %s\n", pageContent_obj_ref);										
										free(pageContent_obj_ref);
										continue;
									}

									if(pageContent_obj->stream != NULL && pageContent_obj->stream_size > 0){
										
										//dbg_log("checkEmptyDocument :: Page %s is not empty\n",kid_obj_ref);
										free(kid_obj_ref);
										free(content_array);
										free(pageContent_obj_ref);
										free(kids);
										return 1;

										// TODO :: 
										// ret = 1;
										// goto cleaning.
										
										

									}else{
										warn_log("checkEmptyDocument :: Empty page content %s\n",pageContent_obj_ref);										
									}

									free(pageContent_obj_ref);

								}

								free(content_array);

							}else{
																
								warn_log("checkEmptyDocument :: Empty page content %s\n",pageContent_obj_ref);
								free(pageContent_obj_ref);

							}



						}

					}


				}
				free(kid_obj_ref);
				
			}

			free(kids);
		}

		next:
		obj = obj->next;

	}


	return ret;

}


/*
documentStructureAnalysis() :: check if the document respects the PDF reference recommendations
parameters:
- struct pdfDocument * pdf (pdf document pointer)
returns: (int)
- 0 on success.
- an error code (<0) on error.

TODO :: documentStructureAnalysis :: check trailers.
*/
int documentStructureAnalysis(struct pdfDocument * pdf){

	int ret = 0;

	dbg_log("documentStructureAnalysis ::\n");

	// TODO.
	//res = checkTrailer(pdf);

	if ((ret = checkXRef(pdf)) < 0){
		err_log("documentStructureAnalysis :: check reference table failed!\n");
		return ret;
	}

	if ((ret = checkEmptyDocument(pdf)) < 0){
		err_log("documentStructureAnalysis :: check document pages content failed!\n");
		return ret;
	}

	// TODO :: check if there is no error during pdf parsing.
	if(ret == 0){
		pdf->testStruct->empty_page_content ++;
	}


	return ret;
}
