#include "pdfAnalyzer.h"



// Check the trailer content
int checkTrailer(struct pdfDocument * pdf){


	char * start = NULL;
	//char * end = NULL;
	char * xref_obj_ref = NULL;
	//struct pdfObject * xref_obj = NULL;
	struct pdfTrailer * trailer = NULL;
	int len = 0;
	int xref_offset;

	if(pdf->trailers == NULL){
		#ifdef DEBUG
			printf("Warning :: checkXTrailer :: No trailer found in pdfDocument\n");
		#endif
		return -1;
	}



	trailer = pdf->trailers;

	while(trailer != NULL){


		// trailer with a dico 
		if(trailer->dico != NULL){

			// TODO

		}else{

			// get the offset of the XRef object
			start = searchPattern(trailer->content, "startxref", 9 , strlen(trailer->content));
			if(start == NULL){
				#ifdef DEBUG
					printf("Error :: checkTrailer :: XRef offset not found in trailer\n");
				#endif
				return -1;
			}

			start += 9;

			while(start[0] == '\r' || start[0] == '\n' || start[0] == ' '){
				start ++;
			}

			len =  (int)(start - trailer->content);
			len = strlen(trailer->content) - len;

			xref_offset = getNumber(start,len);

			#ifdef DEBUG
				printf("Xref object offset = %d\n",xref_offset);
			#endif

			if(xref_offset <= 0){
				trailer = trailer->next;
				continue;
			}

			// go to xref object offset
			start = pdf->content + xref_offset;


			len = (int)(start - pdf->content);
			len = pdf->size - len;

			// if the offset is higher than th PDF size
			if(xref_offset > pdf->size){
				#ifdef DEBUG
					printf("Warning :: checkTrailer :: Wrong xref object offset %d\n",xref_offset);
				#endif
				trailer = trailer->next;
				continue;
			}

			//printf("xref_obj_ref = %s\n",xref_obj_ref);


		}

		free(xref_obj_ref);
		trailer = trailer->next;


	}



	return 0; 
}


// This function check the consitency of the Cross-reference table
int checkXRef(struct pdfDocument * pdf){

	int ret = 1;
	struct pdfTrailer * trailer = NULL;
	char * start = NULL;
	//char * end = NULL;
	char * xref = NULL;
	char * xref_orig = NULL;
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
	char * encrypt = NULL;

	#ifdef DEBUG
		printf("\n\nDebug :: checkXRef \n");
	#endif


	if(pdf->trailers == NULL){
		#ifdef DEBUG
			printf("Warning :: checkXRef :: No trailer found in pdfDocument\n");
		#endif

		return -1;
	}



	trailer = pdf->trailers;

	while(trailer != NULL){

		// Get xref offset
		//printf("Hey !!\n");
		if(trailer->content == NULL){
			#ifdef DEBUG
				printf("Error :: getInfoObject :: Empty trailer content\n");
			#endif

			trailer = trailer->next;
			continue;
		}
		

		start = searchPattern(trailer->content, "startxref", 9 , strlen(trailer->content));


		if(start == NULL){
			#ifdef DEBUG
				printf("Error :: checkXRef :: XRef offset not found in trailer\n");
			#endif

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
		
		if(pdf->fh == NULL){
			#ifdef DEBUG
				printf("Error :: checkXRef :: pdf file handle is NULL\n");
			#endif

			return -1;
		}

		fseek(pdf->fh,0,SEEK_SET);
		fseek(pdf->fh,xref_offset,SEEK_SET);


		// check xref keyword in file
		xref = (char*)calloc(5,sizeof(char));
		xref[4] = '\0';
		fread(xref,1,4,pdf->fh);
		//printf("xref keyword = %s\n",xref);




		if(memcmp(xref,"xref",4) == 0){


			//printf("DEBUG :: checkXRef :: Good xref table offset : %d\n",xref_offset);
			start = pdf->content;
			start += xref_offset;

			len = (int)(start - pdf->content);
			len = pdf->size -len ;


			//printf("DEBUG :: start = %s\n",start);

			free(xref);
			xref = NULL;

			// Get xref table content from pdf document content
			xref = getDelimitedStringContent( start, "xref" , "trailer" , len);
			xref_orig = xref;


			if(xref == NULL){
				#ifdef DEBUG
					printf("Error :: checkXRef :: Error while getting the xref table\n");
				#endif

				return -1;
			}
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

			#ifdef DEBUG
				printf("num_entries = %d\n",num_entries);
			#endif


			len -= strlen(num_entries_a);
			xref += strlen(num_entries_a);

			// move for white space
			len --;
			xref ++;


			

			// For each entry of table
			for(i = 0; i< num_entries ; i++){

				//printf("xref[0] = %c :: %d\n",xref[0],xref);
				//printf("xref = %s\n",xref );

				off = getNumber(xref,len);
				xref += 17;

				free_obj = xref[0];
				//printf("free_obj = %c\n",free_obj);


				//printf("offset = %d\n", off);

				obj_num = first_obj_num + i;

				//printf("object number = %d\n",obj_num );

				
				ref = (char*)calloc(12,sizeof(char)); // whhhhyyyyyyyyyyyyyyyyyyyyyy !!!!!!!!!
				
				sprintf(ref,"%d 0 obj",obj_num);
				
				//printf("ref = %s at %d\n",ref, off);

				// get object by ref
				if(obj_num > 0){
					obj = getPDFObjectByRef(pdf,ref);

				}

				if( obj_num > 0 && obj == NULL){
					#ifdef DEBUG
						printf("Warning :: checkXRef :: object not found %s--\n",ref);
					#endif

				}else{

					
					if(obj_num > 0 && free_obj == 'n' && off != obj->offset ){
						
						#ifdef DEBUG
							printf("Warning :: checkXRef :: Bad offset for object %s :: %d differs from %d\n",ref,off,obj->offset);
						#endif

						pdf->testStruct->bad_obj_offset ++;
						ret = 0;

					}

				}


				// go to the next entry
				xref += 3;

				//printf("\n");
				free(ref);

			}

			free(xref_orig);

		}else{

			// if the offset is higher than th PDF size
			if(xref_offset > pdf->size){
				#ifdef DEBUG
					printf("Warning :: Wrong Xref table (or xref object) offset %d\n",xref_offset);				
				#endif

				ret = 0;
				pdf->testStruct->bad_xref_offset ++;
				trailer = trailer->next;
				continue;
			}
			
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
				
				if(obj->type == NULL || memcmp(obj->type,"/XRef",5) != 0){

					#ifdef DEBUG
						printf("Warning :: Wrong Xref table (or xref object) offset %d\n",xref_offset);
					#endif

					//printf("type = %s\n",obj->type);
					ret = 0;
					pdf->testStruct->bad_xref_offset ++;


				}else{ // if the xref object is found

					#ifdef DEBUG
						printf("XRef obj Dico =  %s\n",obj->dico);
					#endif


					// Check if the document is encrypted 
					if( (encrypt = searchPattern(obj->dico, "/Encrypt",8,strlen(obj->dico)) ) != NULL  ){
						pdf->testStruct->encrypted ++;
					}

					pdf->testStruct->bad_xref_offset = 0;
				}

			}else{

				#ifdef DEBUG
					//printf("Warning :: checkXRef :: object not found %s\n",ref);
				#endif

				//pdf->testStruct->bad_xref_offset ++;

			}

			free(ref);

		}

		free(num_entries_a);
		free(first_obj_num_a);
		free(obj_num_a);
		

		trailer = trailer->next;

	}

	/*
	if(ret != 0){
		printf("The XRef table is OK\n");
	}*/

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
	char * content_array = NULL;
	//char * contentStream = NULL;
	//char * tmp = NULL;
	int len = 0;
	int len2 = 0;


	#ifdef DEBUG
		printf("\n\nDebug :: checkEmptyDocument \n");
	#endif

	if(pdf == NULL){
		#ifdef DEBUG
			printf("Error :: checkEmptyDocument :: NULL pdf object in parameter\n");
		#endif
		return -1;
	}


	obj = pdf->objects;

	while(obj != NULL){

		if(obj->type != NULL && strncmp(obj->type,"/Pages",6) == 0){

			#ifdef DEBUG
				printf("Found /Pages object :: %s\n",obj->reference);
			#endif
			//printf("Dico = %s\n",obj->dico);



			// get kids pages
			start = searchPattern(obj->dico, "/Kids", 5 , strlen(obj->dico));

			if(start == NULL){
				#ifdef DEBUG
					printf("Warning :: no kids entry in pages dictionary %s\n",obj->reference);
				#endif
				obj = obj->next; // go to the next object
				continue;
			}

			start += 5;

			len = (int)(start - obj->dico);
			len = strlen(obj->dico) -len;

			//while
			kids = getDelimitedStringContent(start,"[","]",len);
			//printf("kids = %s\n",kids);


			len = strlen(kids);
			end = kids;

			while( (kid_obj_ref = getIndirectRefInString(end,len)) != NULL){

				//printf("kid ref = %s :: %d\n",kid_obj_ref,kid_obj_ref);

				//printf("kid ref = %s\n",kid_obj_ref);
				

				end = searchPattern(end,kid_obj_ref,strlen(kid_obj_ref)-3,len);

				if(end == NULL){
					#ifdef DEBUG
						printf("Error :: checkEmptyDocument :: end == NULL\n" );
					#endif
					return -1;
				}

				end += strlen(kid_obj_ref) - 2;

				len = (int)(end - kids);
				len = strlen(kids) - len;
				//printf("kids = %d :: end = %d :: len = %d\n",kids, end, len );

				if((kid_obj = getPDFObjectByRef(pdf,kid_obj_ref)) == NULL){
						#ifdef DEBUG
							printf("Warning :: checkEmptyDocument :: Object not found %s\n", kid_obj_ref);
						#endif
						continue;
				}

				// check the type of the object
				if(kid_obj->dico != NULL && kid_obj->type != NULL && strncmp(kid_obj->type,"/Page",5) == 0 && strncmp(kid_obj->type,"/Page",strlen(kid_obj->type)) == 0 ){

					
					// Check empty Page
					//checkEmptyPages(pdf,kid_obj);


					/***********************/

					start = searchPattern(kid_obj->dico, "/Contents", 9 , strlen(kid_obj->dico));
					//printf("Page dico = %s\n",kid_obj->dico);

					if(start == NULL){
						#ifdef DEBUG
							printf("Warning :: checkEmptyDocument :: No page content in object %s\n",kid_obj_ref);
						#endif
						continue;
					}

					start += 9;

					// skip white spaces
					while(start[0] == ' '){
						start ++;
					}

					//printf("start[0] = %c\n",start[0]);

					// if there is serveral content objects
					if(start[0] == '['){

						len2 = (int)(start - kid_obj->dico);
						len2 = strlen(kid_obj->dico) - len2;
						pageContents = getDelimitedStringContent(start,"[","]",len2);

						if(pageContents == NULL){
							#ifdef DEBUG
								printf("Warning :: checkEmptyDocument ::getting Page content array failed !\n");
							#endif
							continue;
						}

						len2 = strlen(pageContents);
						start = pageContents;

						// get page content objects ref
						while( (pageContent_obj_ref = getIndirectRefInString(start,len2)) != NULL){

							//printf(" page content ref = %s\n",pageContent_obj_ref);

							start = searchPattern(start,pageContent_obj_ref,strlen(pageContent_obj_ref)-3,len2);

							if(start == NULL){
								#ifdef DEBUG
									printf("Error :: checkEmptyDocument :: end == NULL\n" );
								#endif
								return -1;
							}

							start += strlen(pageContent_obj_ref) - 2;

							len2 = (int)(end - pageContents);
							len2 = strlen(pageContents) - len2;


							if((pageContent_obj = getPDFObjectByRef(pdf,pageContent_obj_ref)) == NULL){
									#ifdef DEBUG
										printf("Warning :: checkEmptyDocument :: Object not found %s\n", pageContent_obj_ref);
									#endif
									continue;
							}



							// get the stream
							if(pageContent_obj->stream != NULL && pageContent_obj->stream_size > 0){
								//printf("checkEmptyDocument :: Page %s is not empty\n",kid_obj_ref);
								//return 1;
								ret = 1;
							}else{

								#ifdef DEBUG
									printf("Warning :: checkEmptyDocument :: Empty page content %s\n",pageContent_obj_ref);
								#endif
							}


						}


					}else{


						//printObjectByRef(pdf,"39 0 obj");

						len2 = (int)(start - kid_obj->dico);
						len2 = strlen(kid_obj->dico) - len2;
						pageContent_obj_ref = getIndirectRef(start, len2);

						if(pageContent_obj_ref == NULL){
							#ifdef DEBUG
								printf("Warning :: checkEmptyDocument :: Error while getting page content object reference \n");
							#endif
							continue;
						}

						if((pageContent_obj = getPDFObjectByRef(pdf,pageContent_obj_ref)) == NULL){
								#ifdef DEBUG
									printf("Warning :: checkEmptyDocument :: Object not found %s\n", pageContent_obj_ref);
								#endif
								free(pageContent_obj_ref);
								continue;
						}
						
						// get the stream
						if(pageContent_obj->stream != NULL && pageContent_obj->stream_size > 0){
							//return 1;
							//printf("checkEmptyDocument :: Page %s is not empty\n",kid_obj_ref);
							ret = 1;

						}else{

							//printf("DEBUG :: %s --\n",pageContent_obj->content);

							// Case when the content is an array ::
							content_array = getDelimitedStringContent(pageContent_obj->content,"[","]",pageContent_obj->content_size);

							//printf("content array = %s\n",content_array);

							if(content_array != NULL){

								start = content_array;
								len2 = strlen(content_array);

								while((pageContent_obj_ref = getIndirectRefInString(start,len2) ) != NULL){

									//printf("page content ref = %s\n",pageContent_obj_ref );

									start = searchPattern(start,pageContent_obj_ref,strlen(pageContent_obj_ref)-3,len2);
									start += strlen(pageContent_obj_ref) - 2;

									len2 = (int)(start - content_array);
									len2 = strlen(content_array) - len2;


									if((pageContent_obj = getPDFObjectByRef(pdf,pageContent_obj_ref)) == NULL){
										#ifdef DEBUG
											printf("Warning :: checkEmptyDocument :: Object not found %s\n", pageContent_obj_ref);
										#endif
										free(pageContent_obj_ref);
										continue;
									}

									if(pageContent_obj->stream != NULL && pageContent_obj->stream_size > 0){
										//return 1;
										//printf("checkEmptyDocument :: Page %s is not empty\n",kid_obj_ref);
										ret = 1;

									}else{
										#ifdef DEBUG
											printf("Warning :: checkEmptyDocument :: Empty page content %s\n",pageContent_obj_ref);
										#endif
									}

								}

							}else{
								
								#ifdef DEBUG
									printf("Warning :: checkEmptyDocument :: Empty page content %s\n",pageContent_obj_ref);
								#endif

							}

						}

						free(pageContent_obj_ref);

					}


				}
				free(kid_obj_ref);
				
			}

			free(kids);
		}


		obj = obj->next;

	}


	return ret;

}


// This function check ifs the document respects the PDF reference recommendations...
int documentStructureAnalysis(struct pdfDocument * pdf){

	int res = 0;

	#ifdef DEBUG
	printf("\n");
	printf("-----------------------------------\n");
	printf("--- DOCUMENT STRUCTURE ANALYSIS ---\n");
	printf("-----------------------------------\n");
	printf("\n");
	#endif

	//res = checkTrailer(pdf);

	res = checkXRef(pdf);

	res = checkEmptyDocument(pdf);
	if(res == 0){
		pdf->testStruct->empty_page_content ++;
	}



	return 0;
}
