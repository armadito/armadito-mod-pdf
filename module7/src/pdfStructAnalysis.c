#include "pdfAnalyzer.h"


// This function check the consitency of the Cross-reference table
int checkXRef(struct pdfDocument * pdf){

	int ret = 1;
	struct pdfTrailer * trailer = NULL;
	char * start = NULL;
	//char * end = NULL;
	char * xref = NULL;
	int xref_offset = 0;
	int len = 0;
	int num_entries = 0;
	char * num_entries_a = NULL;
	//int gen_num = 0;
	int obj_num = 0;
	char * obj_num_a = NULL;
	int first_obj_num = 0;
	char * first_obj_num_a = NULL;
	int i = 0;
	char * ref = NULL;
	struct pdfObject * obj = NULL;
	char free_obj = 'n';
	int off = 0;



	if(pdf->trailers == NULL){
		printf("Warning :: checkXRef :: No trailer found in pdfDocument\n");
		return -1;
	}

	trailer = pdf->trailers;

	while(trailer != NULL){

		// Get xref offset
		//printf("Hey !!\n");
		if(trailer->content == NULL){
			printf("Error :: getInfoObject :: Empty trailer content\n");
			trailer = trailer->next;
			continue;
		}
		

		start = searchPattern(trailer->content, "startxref", 9 , strlen(trailer->content));

		if(start == NULL){
			printf("Error :: checkXRef :: XRef offset not found in trailer\n");
			return -1;
		}

		start += 9;

		while(start[0] == '\r' || start[0] == '\n' || start[0] == ' '){
			start ++;
		}

		len =  (int)(start - trailer->content);
		len = strlen(trailer->content) - len;

		xref_offset = getNumber(start,len);

		// TODO test the value of xref_offset ( modify get number to return -1 if error);

		printf("xref offset = %d\n",xref_offset);

		
		if(pdf->fh == NULL){
			printf("Error :: pdf file handle is NULL\n");
			return -1;
		}

		fseek(pdf->fh,0,SEEK_SET);
		fseek(pdf->fh,xref_offset,SEEK_SET);



		// check xref keyword in file
		xref = (char*)calloc(4,sizeof(char));
		fread(xref,1,4,pdf->fh);
		//printf("xref keyword = %s\n",xref);


		


		if(strncmp(xref,"xref",4) == 0){


			printf("Good xref table offset : %d\n",xref_offset);
			start = pdf->content;
			start += xref_offset;

			len = (int)(start - pdf->content);
			len = pdf->size -len ;
			//printf("start = %s\n",start);

			free(xref);
			xref = NULL;

			// Get xref table content from pdf document content
			xref = getDelimitedStringContent( start, "xref" , "trailer" , len);
			//printf("xref table content = \n%s\n",xref);

			// shift xref keyword
			xref += 4;

			//printf("xref[0] = %c\n",xref[0]);

			len = strlen(xref) - 4;


			while(xref[0] == '\r' || xref[0] == '\n' || xref[0] == ' '){
				xref++;
				len --;
			}


			//printf("xref[0] = %c\n",xref[0]);

			// Get the object number of the first object described in xref table
			first_obj_num_a = getNumber_a(xref,len);
			first_obj_num = atoi(first_obj_num_a);


			//printf("first_obj_num = %d\n",first_obj_num);

			len -= strlen(first_obj_num_a);
			xref += strlen(first_obj_num_a);

			// move for white space
			len --;
			xref ++;


			//printf("xref[0] = %c\n",xref[0]);

			// get the number of entries in the xref table
			num_entries_a = getNumber_a(xref,len);
			num_entries = atoi(num_entries_a);

			//printf("num_entries = %d\n",num_entries);

			len -= strlen(num_entries_a);
			xref += strlen(num_entries_a);

			// move for white space
			len --;
			xref ++;


			

			// For each entry of table
			for(i = 0; i< num_entries ; i++){

				//printf("xref[0] = %c :: %d\n",xref[0],xref);
				

				off = getNumber(xref,len);
				xref += 17;

				free_obj = xref[0];
				//printf("free_obj = %c\n",free_obj);


				//printf("offset = %d\n", off);

				obj_num = first_obj_num + i;
				//printf("object number = %d\n",obj_num );


				ref = (char*)calloc(10,sizeof(char));
				sprintf(ref,"%d 0 obj",obj_num);
				//printf("ref = %s at %d\n",ref, off);

				// get object by ref
				if(obj_num > 0){
					obj = getPDFObjectByRef(pdf,ref);	
				}
				

				if( obj_num > 0 && obj == NULL){
					printf("Warning :: checkXRef :: object not found %s\n",ref);
				}else{

					if( free_obj == 'n' && off != obj->offset ){

						printf("Warning :: checkXRef :: Bad offset for object %s :: %d differs from %d\n",ref,off,obj->offset);
						pdf->testStruct->bad_obj_offset ++;
						ret = 0;

					}

				}

				// go to the next entry
				xref += 3;

				//printf("\n");

			}

		}else{

			
			//printf("xref keyword = %s\n",xref);

			start = pdf->content;
			start += xref_offset;

			len = (int)(start - pdf->content);
			len = pdf->size -len ;
			//printf("start = %s\n",start);

			//printf("start[0] = %c\n",start[0]);
			//printf("pdf size = %d\n",pdf->size);
			//printf("len = %d\n",len);


			//  Check if the offset point to a Xref Object type /XRef
			obj_num_a = getNumber_a(start,len);

			if(obj_num_a == NULL){
				pdf->testStruct->bad_xref_offset ++;
				trailer = trailer->next;
				continue;
			}

			//printf("obj num = %s\n",obj_num_a);
			obj_num = atoi(obj_num_a);

			len = strlen(obj_num_a) + 7;
			//printf("len = %d\n",len);

			ref = (char*)calloc(len+1,sizeof(char));
			ref[len] = '\0';

			sprintf(ref,"%d 0 obj",obj_num);
			//printf("xref object = %s\n",ref);


			obj = getPDFObjectByRef(pdf,ref);

			if(obj != NULL){


				if(obj->type == NULL || strncmp(obj->type,"/XRef",5) != 0){

					printf("Warning :: Wrong Xref table (or xref object) offset %d\n",xref_offset);
					printf("type = %s\n",obj->type);
					ret = 0;
					pdf->testStruct->bad_xref_offset ++;

				}else{ // if the xref object is found

					pdf->testStruct->bad_xref_offset = 0;
				}

			}else{

				printf("Warning :: checkXRef :: object not found %s\n",ref);
				//pdf->testStruct->bad_xref_offset ++;

			}

		}

		free(obj_num_a);
		free(ref);
		trailer = trailer->next;

	}


	if(ret != 0){
		printf("The XRef table is OK\n");
	}

	return ret;

}



// This function checks if pages contituting the document are not all empty. => returns 1 if not empty
// TODO :: Improve this function by creating a function getPagesKids which could be called recursively when the Kids objects reffers also to a /Pages object.
int checkEmptyDocument(struct pdfDocument * pdf){


	struct pdfObject * obj = NULL;
	int ret = 0;
	char * start = NULL;
	char * end = NULL;
	char * kids = NULL;
	char * kid_obj_ref = NULL;
	struct pdfObject * kid_obj = NULL;
	char * pageContents = NULL;
	char * pageContent_obj_ref = NULL;
	struct pdfObject * pageContent_obj = NULL;
	//char * contentStream = NULL;
	//char * tmp = NULL;
	int len = 0;
	int len2 = 0;

	if(pdf == NULL){
		printf("Error :: checkEmptyDocument :: NULL pdf object in parameter\n");
		return -1;
	}


	obj = pdf->objects;

	while(obj != NULL){

		if(obj->type != NULL && strncmp(obj->type,"/Pages",6) == 0){

			printf("Found /Pages object :: %s\n",obj->reference);
			printf("Dico = %s\n",obj->dico);

			// get kids pages
			start = searchPattern(obj->dico, "/Kids", 5 , strlen(obj->dico));

			if(start == NULL){
				printf("Warning :: no kids entry in pages dictionary %s\n",obj->reference);
				obj = obj->next; // go to the next object
				continue;
			}

			start += 5;

			len = (int)(start - obj->dico);
			len = strlen(obj->dico) -len;

			//while
			kids = getDelimitedStringContent(start,"[","]",len);
			printf("kids = %s\n",kids);


			len = strlen(kids);
			end = kids;

			while( (kid_obj_ref = getIndirectRefInString(end,len)) != NULL){

				//printf("kid ref = %s :: %d\n",kid_obj_ref,kid_obj_ref);

				printf("kid ref = %s\n",kid_obj_ref);
				

				end = searchPattern(end,kid_obj_ref,strlen(kid_obj_ref)-3,len);

				if(end == NULL){
					printf("Error :: checkEmptyDocument :: end == NULL\n" );
					return -1;
				}

				end += strlen(kid_obj_ref) - 2;

				len = (int)(end - kids);
				len = strlen(kids) - len;
				//printf("kids = %d :: end = %d :: len = %d\n",kids, end, len );

				if((kid_obj = getPDFObjectByRef(pdf,kid_obj_ref)) == NULL){
						printf("Warning :: checkEmptyDocument :: Object not found %s\n", kid_obj_ref);
						continue;
				}

				// check the type of the object
				
				if(kid_obj->dico != NULL && kid_obj->type != NULL && strncmp(kid_obj->type,"/Page",5) == 0){

					start = searchPattern(kid_obj->dico, "/Contents", 9 , strlen(kid_obj->dico));
					printf("Page dico = %s\n",kid_obj->dico);

					if(start == NULL){
						printf("Warning :: checkEmptyDocument :: No page content in object %s\n",kid_obj_ref);
						continue;
					}

					start += 9;

					// skip white spaces
					while(start[0] == ' '){
						start ++;
					}

					printf("start[0] = %c\n",start[0]);

					// if there is serveral content objects
					if(start[0] == '['){

						len2 = (int)(start - kid_obj->dico);
						len2 = strlen(kid_obj->dico) - len2;
						pageContents = getDelimitedStringContent(start,"[","]",len2);

						if(pageContents == NULL){
							printf("Warning :: checkEmptyDocument ::getting Page content array failed !\n");
							continue;
						}

						len2 = strlen(pageContents);
						start = pageContents;

						// get page content objects ref
						while( (pageContent_obj_ref = getIndirectRefInString(start,len2)) != NULL){

							printf(" page content ref = %s\n",pageContent_obj_ref);

							start = searchPattern(start,pageContent_obj_ref,strlen(pageContent_obj_ref)-3,len2);

							if(start == NULL){
								printf("Error :: checkEmptyDocument :: end == NULL\n" );
								return -1;
							}

							start += strlen(pageContent_obj_ref) - 2;

							len2 = (int)(end - pageContents);
							len2 = strlen(pageContents) - len2;


							if((pageContent_obj = getPDFObjectByRef(pdf,pageContent_obj_ref)) == NULL){
									printf("Warning :: checkEmptyDocument :: Object not found %s\n", pageContent_obj_ref);
									continue;
							}

							// get the stream
							if(pageContent_obj->stream != NULL && pageContent_obj->stream_size > 0){
								printf("checkEmptyDocument :: Page %s is not empty\n",kid_obj_ref);
								//return 1;
								ret = 1;
							}else{
								printf("Warning :: checkEmptyDocument :: Empty page content %s\n",pageContent_obj_ref);
							}


						}


					}else{

						len2 = (int)(start - kid_obj->dico);
						len2 = strlen(kid_obj->dico) - len2;
						pageContent_obj_ref = getIndirectRef(start, len2);

						if(pageContent_obj_ref == NULL){
							printf("Warning :: checkEmptyDocument :: Error while getting page content object reference \n");
							continue;
						}

						if((pageContent_obj = getPDFObjectByRef(pdf,pageContent_obj_ref)) == NULL){
								printf("Warning :: checkEmptyDocument :: Object not found %s\n", pageContent_obj_ref);
								continue;
						}

						// get the stream
						if(pageContent_obj->stream != NULL && pageContent_obj->stream_size > 0){
							//return 1;
							printf("checkEmptyDocument :: Page %s is not empty\n",kid_obj_ref);
							ret = 1;

						}else{
							printf("Warning :: checkEmptyDocument :: Empty page content %s\n",pageContent_obj_ref);
						}


					}


				}

				
			}


		}


		obj = obj->next;

	}


	return ret;

}



// This function check ifs the document respects the PDF reference recommendations...
int documentStructureAnalysis(struct pdfDocument * pdf){

	int res = 0;

	printf("-----------------------------------------\n");
	printf("--- DOCUMENT STRUCTURE ANALYSIS ---\n");
	printf("-----------------------------------------\n\n");

	res = checkXRef(pdf);



	res = checkEmptyDocument(pdf);
	if(res == 0){
		pdf->testStruct->empty_page_content ++;
	}



	return 0;
}
