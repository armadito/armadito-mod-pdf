/*  
	< UHURU PDF ANALYZER is a tool to parses and analyze PDF files in order to detect potentially dangerous contents.>
    Copyright (C) 2015 by Ulrich FAUSTHER <u.fausther@uhuru-solutions.com>
    

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "pdfAnalyzer.h"
#include "filters.h"
#include "miniz.c"
#include <errno.h>


//extern int _errno;

char * FlateDecode(char * stream, struct pdfObject* obj){


	char * dest = NULL;
	char * out = NULL;
	unsigned long src_len = 0;
	//mz_ulong len = 16384;
	unsigned long len = 150000;
	//int dest_len = 16384;
	int res = 0;

	//printf("stream = --%s--\n",stream );
	//printStream(stream,obj->tmp_stream_size);
	//unsigned long streamd_len = 163840; // TODO TOFIX search a way to find the right length
	
	src_len = obj->tmp_stream_size;

	// verify parameters
	if (stream == NULL || src_len == 0){
		#ifdef DEBUG
			printf("Error :: FlateDecode :: Null stream to decode in object %s\n", obj->reference);
		#endif
		return NULL;
	}

	//sleep(10);

	dest = calloc(len,sizeof(char));
	
	//dest[17505] = '\0';


	res = uncompress((unsigned char*)dest,&len,(unsigned char*)stream,src_len);
	//printf("Len = %ld\n",len);
	//printf("Dest = %s\n",dest);

	out = (char*)calloc(len+1,sizeof(char));
	out[len]='\0';
	memcpy(out,dest,len);

	//debugPrint(stream,obj->tmp_stream_size);

	#ifdef DEBUG
		printf("FlateDecode :: status = %d :: len = %ld\n",res,len);
	#endif


	if( res != Z_OK){
		#ifdef DEBUG
			printf("Warning :: Flatedecode failed for object \"%s\" :: %d \n",obj->reference, res);
		#endif
	}


	obj->decoded_stream_size = len;
	obj->tmp_stream_size = len;

	
	//printf("decoded data = %s :: len  = %d\n",out,len);


	//printf("\n\n");
	free(dest);

	return out;
}





char * FlateDecode_old(char * stream, struct pdfObject* obj){

	char * streamd = NULL;
	//unsigned long limit = 1000000;

	int res = 0;
	unsigned long streamd_len = 163840; // TODO TOFIX search a way to find the right length
	unsigned long len = 163840;
	//char * buffer = 0;
	//unsigned long buffer_len = 16384;



	if (stream == NULL ){
		printf("Error :: Null stream to decode in object %s\n", obj->reference);
		return NULL;
	}

	//printf("stream = %s\n",stream);

	//printf("Flatedecode...\n");

	// Get the decoded stream length

	//obj->stream_size --;
	//streamd_len = obj->stream_size;
	//streamd_len = 10000;

	// printf("Stream size = %d...\n",obj->stream_size);

	if(strncmp(obj->reference,"8 0 obj",7) == 0){
		printf("len_stream = %d\n",obj->tmp_stream_size);
		debugPrint(stream,obj->tmp_stream_size-1);
		printf("stream _flatedecode = %s\n",stream);
	}
	

	//debugPrint(stream,obj->tmp_stream_size);

	streamd  = (char*)calloc(streamd_len,sizeof(char));

	

	/*if(strncmp(obj->reference,"13 0 obj",8) == 0){
		printf("data = --%s--\n",stream);
	}*/


	/*
	res = 1;

	while(res != Z_OK){

		buffer = (char*)calloc(buffer_len,sizeof(char));

		res = uncompress(buffer,&buffer_len,stream,obj->tmp_stream_size);

		if(res == -3 || res == -5){
			printf(" hey !! \n");
			buffer_len *= 2;
		}

		if(res == Z_OK){
			streamd = (char*)calloc(buffer_len,sizeof(char));
			streamd = memcpy(streamd,buffer,buffer_len);
			printf("decoded data = %s\n",streamd);
		}


		free(buffer);
		buffer = NULL;

	}
	*/

	//returned_len = obj->tmp_stream_size;

	//printf("heyhey !! :: %d \n",obj->tmp_stream_size);
	//rintf("heyhey !! %s \n",stream);

	res = uncompress((unsigned char*)streamd,&len,(unsigned char*)stream,obj->tmp_stream_size);

	//printf("heyhey :: %ld !!\n",len);

	if( res != Z_OK){
		printf("Warning :: Flatedecode failed for object \"%s\" :: %d \n",obj->reference, res);
	}



	//printf("deflate res res_d = %d\nlen_d = %d\nstream_d = %s--\n",res,streamd_len,streamd);
	//printf("deflate res = %d :: &streamd_len = %ld\n",res,streamd_len);

	/*if(streamd_len > limit){
		printf("Warning :: FlateDecode :: Potentially BufferOverflow in stream in object %s !!\n",obj->reference);
		// TODO :: add a coef to this test
	}*/

	obj->decoded_stream_size = len;
	obj->tmp_stream_size = len;

	streamd[len +1] = '\0';

	
	/*if(strncmp(obj->reference,"34 0 obj",8) == 0){
		printf("decoded data = %s\n",streamd);
	}*/
	
	//printf("decoded data = %s\n",streamd);

	return streamd;
}



char * ASCIIHexDecode(char * stream, struct pdfObject * obj){

	char * out = NULL; 
	//unsigned long stream_len = 0;
	char* tmp = NULL; 
	int has_eod_marker = 0;
	int len = 0;
	int i = 0 ,j =0;
	//char res;
	int flag = 0;
	char c = 0;
	char * end = NULL;

	//printf("ASCIIHexDecode...\n");

	if ( stream == NULL){
		printf("Error :: Null stream to decode in obj %s\n", obj->reference);
		return NULL;
	}

	//printf("stream = %s\n",stream);
	len  = strlen(stream);
	//len= obj->stream_size;
	//printf("len = %d\n",len);

	tmp = (char*)calloc(3,sizeof(char));
	tmp[2]='\0';

	//printf("stream_len-1 = %c\n",stream[len-1]);
	if(stream[len-1] == '>' || (( stream[len-1] < 65 || stream[len-1] > 70 ) && (stream[len-1] <  97 || stream[len-1] > 102 ) && (stream[len-1] <  48 || stream[len-1] > 57 )) ){
		has_eod_marker = 1;
		printf("has_eod_marker = %d\n",has_eod_marker);
		stream[len-1]='\0';
		len --;
	}



	obj->tmp_stream_size = len/2;

	//if(strncmp(obj->reference,"30 0 obj",8)==0)
	//printf("stream = %s\n",stream);

	// If the length is odd and there is an eod marker (>) padd with a zero.
	out =  (char*)calloc(len+1,sizeof(char));
	out[len] = '\0';

	//printf("%s\n", );

	end = out;
	
	//while( i < len-2 || stream[i] != '>' || stream[i+1] != '>'){
	while( i < len-1 ){


		// Ignore white space and non hex characters
		flag  = 0;
		while(flag < 2 && i< len-1){

			if( (stream[i]>=  65 && stream[i]<=70 ) || (stream[i] >=  97 && stream[i]<=102 ) || (stream[i] >=  48 && stream[i]<=57 ) ){ // HExa characters
				tmp[flag] = stream[i];
				flag ++;
			}
			i++;
		}

		c = strtol(tmp,NULL,16);
		memset(end,c,1);
		//out[j] = strtol(tmp,NULL,16);
		//printf("tmp = %s\tres = %c\n",tmp,out[j]);
		j++;
		end ++;

	}

	//printf("out = %s :: %d\n\n",out,j);
	obj->tmp_stream_size = j;
	obj->decoded_stream_size = j;

	//if(strncmp(obj->reference,"11 0 obj",8) == 0){
		//debugPrint(out,obj->tmp_stream_size);
	//}
	free(tmp);
	tmp = NULL;

	return out;


}





struct LZWdico * initDico(int code, char * entry){

	struct LZWdico * dico = NULL;

	if((dico = (struct LZWdico *)calloc(1,sizeof(struct LZWdico))) == NULL){
		printf("Error :: initDico :: Initialization failed\n");
		return NULL;
	}

	dico->code = code;
	dico->entry = entry;
	dico->next = NULL;

	return dico;
}


int addInDico(struct LZWdico * dico , int code, char * entry){

	//struct LZWdico * tmp = NULL;


	// check parameters
	if(entry == NULL || dico == NULL){
		printf("Error :: addInDico :: Bad parameters \n");
		return -1;
	}

	//tmp = dico;

	//tmp = dico->next;
	//printf("hey0 :: %d code \n",code);
	while(dico->next != NULL){
		dico = dico->next;
		//if(code ==  2089)
	}
	//printf("hey0\n");

	

	//printf("hey !!\n");

	dico->next = initDico(code, entry);

	/*
	if((tmp->next = (struct LZWdico *)calloc(1,sizeof(struct LZWdico))) == NULL){
		printf("Error :: initDico :: Initialization failed\n");
		return -1;
	}
	
	tmp->next->code = code ;
	tmp->next->entry = entry;
	(tmp->next)->next = NULL;
	*/

	return 1;

}

/*
char isInDico(struct LZWdico * dico , int code, char * entry){



	return 0;
}
*/

void freeDico(struct LZWdico * dico){

	struct LZWdico * tmp = NULL;

	if(dico == NULL){
		return;
	}

	while(dico != NULL){

		tmp = dico;
		dico = dico->next;
		
		free(tmp->entry);
		free(tmp);
		tmp = NULL;
	}

	return ;
}



char * getEntryInDico(struct LZWdico * dico , int code){

	//char * entry = NULL;

	// checks parameters

	while(dico != NULL){

		if(dico->code == code){
			return dico->entry;
		}

		dico = dico->next;
	}

	return NULL;

}


void printDico(struct LZWdico * dico){

	if(dico == NULL){
		printf("Empty Dico !!\n");
		return;
	}

	printf("LZW DICO\n");

	while( dico != NULL ){

		printf("Code == %d :: entry :: %s\n",dico->code, dico->entry );

		dico = dico->next;

	}

	return;

}



unsigned short readData(char ** data, int * partial_code, int * partial_bits, int code_len ){

	unsigned short code = 0;
	unsigned char unpack = 0;
	


	//printf("partial_bits (before) = %d\n",*partial_bits);
	//printf("partial_code (before) = %d\n",*partial_code);
	//printf("data :: %c\n",*data[0]);

	

	while(*partial_bits < code_len){

		if(*partial_code != -1){

			os_sscanf(&(*data[0]),"%c",&unpack);
			

			//printf("Debug :: unpack :: %d :: %d\n",unpack,unpack_test);
			//unpack = *data[0] - '\0';
			//printf("unpack1 = %d :: data0 = %c\n",unpack,*data[0]);
			*partial_code = (*partial_code << 8) + unpack;


		}else{

			os_sscanf(&(*data[0]),"%c",&unpack);
			//printf("unpack2 = %d\n",unpack);
			*partial_code = unpack;

		}

		*data += 1;
		*partial_bits += 8;
	}

	code = *partial_code >> (*partial_bits - code_len);
	*partial_code &= (1 << (*partial_bits - code_len)) -1;
	*partial_bits -= code_len;
	

	// *partial_bits += 10;
	// *partial_code += 10;
	// *data += 1;


	//printf("partial_bits (after) = %d\n",*partial_bits);
	//printf("partial_code (after) = %d\n",*partial_code);
	//printf("data (after) :: %c\n",*data[0]);


	return code;

}

char * LZWDecode(char* stream, struct pdfObject * obj){

	char * out = NULL;
	int stream_len = 0;
	int i = 0;
	struct LZWdico* dico = NULL;
	int code_len  = 9;
	char * ptr_data = NULL;
	//int test = 0; 
	unsigned short code = 0;
	//char * result = NULL;
	int size = 0;
	char * entry = NULL;
	char * w = NULL;

	int partial_code = -1;
	int partial_bits = 0;
	int next_code = 258;
	int first = 1;
	char * w_entry0  = NULL ; //to add in dico


	//printf("\n\n:: LZWDecode ::\n");

	//printf("stream = %s\n",stream);

	stream_len = obj->tmp_stream_size;
	//printf("Stream_len = %d\n",stream_len);

	/*
	printf("scan0 = %c\n",stream[0]);
	printf("scan1 = %c\n",stream[1]);
	printf("scan2 = %c\n",stream[2]);
	printf("scan3 = %c\n",stream[3]);
	printf("\n");
	*/

	//sscanf(&stream[0],"%c",&test);
	//printf("test = %d\n",test);

	ptr_data = stream;

	// test
	/*
	printf("ptr_data = %c\n",ptr_data[0]);
	code = readData(&ptr_data, &partial_code, &partial_bits, code_len );
	printf("code = %d\n\n",code);


	printf("ptr_data = %c\n",ptr_data[0]);
	code = readData(&ptr_data, &partial_code, &partial_bits, code_len );
	printf("code = %d\n\n",code);
	*/

	//printf("ptr_data = %c\n",ptr_data[0]);
	//code = readData(&ptr_data, &partial_code, &partial_bits, code_len );
	//printf("code = %d\n\n",code);

	


	out = (char*)calloc(stream_len+1,sizeof(char));
	out[stream_len] = '\0';

	//result = (char*)calloc(2,sizeof(char));
	//result[1] = '\0';


	for(i= 0 ; i < stream_len ; i++){

		//printf("hooo\n");

		// maybe if next_code + 1 == 1 << code_len )
		if( next_code +1 == (1<< code_len)){
			//printf("\t<::> INCREASING CODE len = %d ++\n",code_len);
			code_len ++;
		}

		//printf("hey1\n" );
		code = readData(&ptr_data, &partial_code, &partial_bits, code_len );
		//printf("hey !!\n");

		
		//printf("ptr_data = %c :: code = %d :: next_code = %d\n",ptr_data[0], code,next_code);
		//printf("code = %d :: next_code = %d\n", code,next_code);
		

		

		
		// finish :: bad sequence
		if(code >= next_code){
			//printf("BAD/END SEQUENCE code (%d) > next_code (%d)\n",code, next_code);
			i = stream_len;
			continue;
		}
		

		//code


		if(code == CLEAR_TABLE){

			//printf("CLEAR_TABLE marker reached !!\n");
			code_len = 9;
			next_code = EOD_MARKER + 1;
			continue;

		}else{

			if( code == EOD_MARKER ){

				//printf("EOD_MARKER reached !!\n");
				i = stream_len;
				continue;

			}else{

				if(code > EOD_MARKER ){

					// store in the dico
					

					// if the code is in the dico
					if(getEntryInDico(dico,code) != NULL){

						
						// entry  = entry in dico [c]
						entry = getEntryInDico(dico,code);
						//printf("IN DICO !!!! => %s\n", entry);


						// write entry in result
						os_strncat(out,stream_len+1,entry,strlen(entry));
						size += strlen(entry);

						// add entry in dico
						w_entry0 = (char*)calloc(strlen(w)+2,sizeof(char));
						w_entry0[strlen(w)+1]= '\0';
						
						os_strncat(w_entry0,strlen(w)+2,w,strlen(w));
						os_strncat(w_entry0,strlen(w)+2,entry,1);

						if(dico == NULL){
							dico = initDico(next_code,w_entry0);
						}else{
							if( addInDico(dico,next_code,w_entry0) != 1){
								printf("Error :: addInDico failed !! %d :: %s \n",next_code,w_entry0);
							}else{
								//printf("Adding entry %d :: %s\n",next_code,w_entry0);
							}
						}

						//printf("IN DICO => entry = %s :: w_entry0 = %s\n",entry,w_entry0);

						w = entry;
						next_code ++;
						
					}else{

						
						//entry = w + w[0]
						entry = (char*)calloc(strlen(w)+2,sizeof(char));
						entry[strlen(w)+1]= '\0';
						os_strncat(entry,strlen(w)+2,w,strlen(w));
						os_strncat(entry,strlen(w)+2,w,1);


						// write entry in result
						os_strncat(out,stream_len+1,entry,strlen(entry));
						size += strlen(entry);

						
						// add w + entry[0] in dico
						w_entry0 = (char*)calloc(strlen(w)+2,sizeof(char));
						w_entry0[strlen(w)+1]= '\0';
						os_strncat(w_entry0,strlen(w)+2,w,strlen(w));
						os_strncat(w_entry0,strlen(w)+2,entry,1);
						


						if(dico == NULL){
							dico = initDico(next_code,w_entry0);
						}else{
							if( addInDico(dico,next_code,w_entry0) != 1){
								printf("Error :: LZWDecode :: addInDico failed !! %d :: %s\n",next_code,w_entry0);
							}else{


								//printf("Adding entry %d :: %s\n",next_code,w_entry0);
							}
						}

						

						//printf("STORE DICO => entry = %s :: w_entry0 = %s\n",entry,w_entry0);

						w = entry;
						next_code ++;

						//printf(" STORE IN DICO !!! ==> %s \n",w_entry0);
						
						

					}

					// if the code is 

				}else{

					
					// first occurrence
					if(first == 1){
						w = (char*)calloc(2,sizeof(char));
						w[1] = '\0';

						w[0] = (char)((int)'\0' + code);
						//sscanf(&code,"%c",&w[0]);
						//printf("RESULT1 = %d , %s\n",code, w);
						os_strncat(out,stream_len+1,w,1);
						size++;
						first = 0;
						continue;
					}
				

					//sscanf(&code,"%c",&result[0]);
					//printf("RESULT = %s\n",result);
					//strncat(out,result,1);

					entry = (char*)calloc(2,sizeof(char));
					entry[1] = '\0';

					entry[0] = (char)((int)'\0' + code);
					//sscanf(&code,"%c",&entry[0]);
					//printf("RESULT = %s\n",entry);
					os_strncat(out,stream_len+1,entry,1);
					size ++;


					w_entry0 = (char*)calloc(strlen(w)+2,sizeof(char));
					w_entry0[strlen(w)+1]= '\0';

					//free(w);
					

					//w_entry0 = w + entry[0];
					os_strncat(w_entry0,strlen(w)+2,w,strlen(w));
					os_strncat(w_entry0,strlen(w)+2,entry,1);



					//entry = (char*)calloc(2,sizeof(len));
					if(dico == NULL){
						dico = initDico(next_code,w_entry0);
						//printf("Adding entry %d :: %s\n",next_code,w_entry0);
					}else{
						if( addInDico(dico,next_code,w_entry0) != 1){
							#ifdef DEBUG
								printf("Error :: addInDico failed !! %d :: %s \n",next_code,w_entry0);
							#endif
						}
					}

					//printf("=> entry = %s :: w_entry0 = %s\n",entry,w_entry0);
					
					w = entry;
					next_code ++;
					
					//first = 0;

				}

				//printf(" => entry = %s :: w_entry0 = %s\n",entry,w_entry0);



			}

		}



		//printf("scan = %c\n",stream[i]);

		//printf("\n\n");



	} // end_for


	//printf("heyhey :: size = %d\n",size);
	

	out[size+1]='\0';

	//if(strncmp(obj->reference,"44 0 obj",8) == 0)
		//printf("out = %s\n",out);

	//printf("size = %d\n",size);

	obj->tmp_stream_size = size;
	obj->decoded_stream_size = size;

	//if(strncmp(obj->reference,"44 0 obj",8) == 0)
	//printDico(dico);

	freeDico(dico);
	//free(w);

	return out;

}


char * getTuple(char * data, int len){

	char * tuple = NULL;
	//int num = 0;
	int i = 0;
	int size = 5; // tuple size

	tuple = (char*)calloc(size+1,sizeof(char));
	tuple[size]='\0';

	if(data == NULL){
		return NULL;
	}

	if(len == 0){
		return NULL;
	}

	if(len >=5){

		memcpy(tuple,data,5);

	}else{

		memcpy(tuple,data,len);

		// padd with "u"
		for(i = 0; i < (5-len); i++){
			os_strncat(tuple,size+1,"u",1);
		}

	}

	return tuple;
}


char * ASCII85Decode(char * stream, struct pdfObject * obj){


	unsigned long stream_len = 0;
	int i = 0,j=0;
	char * out = NULL, *out_tmp = NULL;
	char * data_ptr = NULL;
	char * tuple = NULL;
	//char * tuple_s = NULL;
	int len = 0;
	int * tuple_a = NULL;	
	unsigned long base = 0;

	//int test = 0;
	int num = 0; 
	char * tmp = NULL;
	char * invert = NULL;



	//if(strncmp(obj->reference,"44 0 obj",8) == 0)
	//	printf("\n\n::: ASCII85Decode :::\n");

	stream_len = obj->tmp_stream_size;
	len = stream_len;

	//printf("stream = %s\n", stream);
	//printf("stream_len = %d\n", stream_len);
	data_ptr = stream;

	// remove white spaces if any
	//stream_len *= 2;
	out = (char*)calloc(stream_len+1,sizeof(char));
	out[stream_len] = '\0';
	//out_tmp = out;
	//tuple = (char*)calloc(6,sizeof(char));
	//tuple_s = (char*)calloc(6,sizeof(char));
	tuple_a = (int*)calloc(6,sizeof(int));
	//res = (int*)calloc(6,sizeof(int));
	//tuple[5]='\0';


	while(len > 0 && j < stream_len-1){

		base = 0;

		tuple = getTuple(data_ptr,len);
		if(tuple == NULL){
			printf("ERROR :: Cannot get Tuple!!\n" );
			len = 0;
			continue;
		}

		data_ptr += 5;

		len = (int)(data_ptr - stream);
		len = stream_len - len;

		//printf("tuple = %s\n",tuple);


		// tuple (char) into ascii (dec)
		
		/*sscanf(&tuple[0],"%c",&tuple_a[0]);
		sscanf(&tuple[1],"%c",&tuple_a[1]);
		sscanf(&tuple[2],"%c",&tuple_a[2]);
		sscanf(&tuple[3],"%c",&tuple_a[3]);
		sscanf(&tuple[4],"%c",&tuple_a[4]);*/
		

		for(i = 0; i < 5; i++){
			tuple_a[i] = tuple[i] - '\0'; // char to int			
		}


		
		for(i = 0; i< 5; i++){

			
			//sscanf(&tuple[i],"%d",&tuple_a[i]);
			//printf("%d => %d",tuple_a[i],tuple_a[i]-33);

			// substract 33 to tuple 
			tuple_a[i] -= 33;

			// change in base 85
			//tuple_a[i] += base;

			base *= 85;
			base += tuple_a[i];

			//printf(" => %ld\n",base);

			//change in binary

		}

		//sscanf(&base,"%s",tuple_s);
		//sprintf(tuple_s,"%d",base);

		tmp = (char*)&base; // change unsigned long into char *
		//tmp[4] = '\0';

		//printf("\tres = --%s--\t j = %d \n",tmp,j);




		//printf("num = %d\n",strlen(tuple_s));
		//strncat(out,tmp,strlen(tmp));
		//num = strlen(tmp);
		num = 5;
		//printf("strlen_tmp =  %d\n",strlen(tmp));
		//printf("stream_len =  %d\n",stream_len);



		//invert = tmp + num -1;
		invert = tmp + num -2;
		//printf("\tinvert = --%s--\n",invert);
		//printf("out = %s\n",out);
		//printf("invert= %s :: len = %d\n",invert,len);

		// invert 
		for(i = num-2; i >= 0 ; i--){

			//printf(":: len = %d num = %d\n",len, num);
			//memcpy(out_tmp, invert, 1);
			j++;
			if (j<stream_len)
				os_strncat(out,stream_len,invert,1);
			//out_tmp++;
			//printf("out = %s\n",out);
			//printf("hey :: %c :: %s\n",invert[0],out);

			invert --;

		}
		//strncat(out,invert,num);	
		//j += num;

		//printf("j = %d\n",j);

		//sscanf(&code,"%c",&entry[0]);
		//printf("tuple_a = %s\n",tuple_a);
		
		free(tuple);
	}

	obj->decoded_stream_size = j;
	obj->tmp_stream_size = j;

	//printf("out = %s--\n",out);

	free(tuple_a);

	//free(tuple_s);


	return out;

}



int getRunLengthCodeInTable(char ** table, char * bits, int table_size){

	int i = 0;

	for( i = 0; i < table_size ; i ++){
		if( strncmp(table[i],bits,strlen(bits)) == 0 && strncmp(table[i],bits,strlen(table[i])) == 0 ){
			//printf("found :: %s : %s\n", table[i],bits);
			return i;
		}
	}

	return -1;
}


int getMakeUpCodeInTable(char ** table, char *bits, int table_size){

	int i =0;
	int code = -1;

	for( i = 0; i < table_size ; i ++){

		if( strncmp(table[i],bits,strlen(bits)) == 0 && strncmp(table[i],bits,strlen(table[i])) == 0 ){

			code = WHITE_BLACK_MAKE_UP_CODES_VALUES[i];
			return code;
		}
	}

	return -1;
}



char * CCITTFaxDecode(char* stream, struct pdfObject * obj){

	char * out = NULL;

	char * bitstream = NULL;
	int len = 0;
	int i = 0, j= 0;
	int max_bits = 13;
	char * select = NULL;
	int code_len = 0;
	int color = 1; // white = 1 ; black = 0
	int run = 0;
	
	char white = '1';
	char black = '0';
	char * binary_mode= NULL;

	char * tmp = NULL; // tmp buffer
	char * result = NULL;
	int result_size = 0;

	
	


	len = obj->tmp_stream_size;
	//bitstream = (char*)calloc(len,sizeof(char));
	//memcpy(bitstream,stream,len);

	//printf("\n\n::: CCITTFaxDecode ::: %s\n",obj->reference);

	//printf("stream + 1 = %s\n", stream+1);

	if(stream == NULL){
		#ifdef DEBUG
			printf("Error :: CCITTFaxDecode :: NULL parameters while decoding stream in object %s\n",obj->reference);
		#endif
		return NULL;
	}


	//printf("len = %d\n",len);

	binary_mode = toBinary(stream,len);
	bitstream = binary_mode;


	//printf("bitstream = %s\n",bitstream);
	len = 8*len;

	//printf("\n\n");

	while(len > 0){

		code_len = 0;

		if(max_bits > len)
			max_bits = len;

		for(i =1; i<= max_bits; i++){



			// select 1 to max_bits code
			select = (char*)calloc(i+1,sizeof(char));
			select[i] = '\0';

			memcpy(select,bitstream,i);

			//printf("select = %s :: len = %d color = %d\n",select, i,color);

			// check code table
			if(color == 1){ //white

				if( (run = getMakeUpCodeInTable(WHITE_MAKE_UP_CODES,select,27)) != -1 ){

					code_len = i;
					i = max_bits;
					//printf("run_white1 = %d\n",run);

					// write bits
					free(tmp);
					tmp = (char*)calloc(result_size+1,sizeof(char));
					tmp[result_size] = '\0';
					memcpy(tmp,result,result_size);

					result_size += run;
					free(result);
					result = NULL;
					result = (char*)calloc(result_size+1,sizeof(char));
					result[result_size] = '\0';

					os_strncat(result,result_size+1,tmp,result_size-run);

					for(j = 0; j < run; j++){
						result[result_size-run+j] = white;
					}

					//printf("result = %s\n",result );



				}else{


					if((run = getRunLengthCodeInTable( WHITE_RUN_LENGTH_TERMINATING_CODES,select,63)) != -1){

						color = (color == 1) ? 0:1; // switch color bit
						code_len = i;
						i = max_bits;
						//printf("run_white2 = %d\n",run);

						// write bits
						free(tmp);
						tmp = (char*)calloc(result_size+1,sizeof(char));
						tmp[result_size] = '\0';
						memcpy(tmp,result,result_size);

						result_size += run;
						free(result);
						result = NULL;
						result = (char*)calloc(result_size+1,sizeof(char));
						result[result_size] = '\0';

						os_strncat(result,result_size+1,tmp,result_size-run);

						for(j = 0; j < run; j++){
							result[result_size-run+j] = white;
						}

						//printf("result = %s\n",result );


					}else{

						//printf("select = %s\n",select);
						if(strncmp("000000000001",select,12) == 0){ // EOL
							//printf("Warning :: CCITTFaxDecode :: EOL White reached !\n" );
							//printf("run_white = EOL \n" );
							code_len = i;
							i = max_bits;

						}

					}

				}
				

			}else{	// black

				if( (run = getMakeUpCodeInTable( BLACK_MAKE_UP_CODES,select,27)) != -1 ){

					code_len = i;
					i = max_bits;
					//printf("run_black1 = %d\n",run);

					// write bits	
					free(tmp);
					tmp = (char*)calloc(result_size+1,sizeof(char));
					tmp[result_size] = '\0';
					memcpy(tmp,result,result_size);

					result_size += run;
					free(result);
					result = NULL;
					result = (char*)calloc(result_size+1,sizeof(char));
					result[result_size] = '\0';

					os_strncat(result,result_size+1,tmp,result_size-run);

					for(j = 0; j < run; j++){
						result[result_size-run+j] = black;
					}

					//printf("result = %s\n",result );

				}else{

					if((run = getRunLengthCodeInTable( BLACK_RUN_LENGTH_TERMINATING_CODES,select,63)) != -1){

						color = (color == 1) ? 0:1; // switch color bit
						code_len = i;
						i = max_bits;
						//printf("run_black2 = %d\n",run);

						// write bits
						free(tmp);
						tmp = (char*)calloc(result_size+1,sizeof(char));
						tmp[result_size] = '\0';
						memcpy(tmp,result,result_size);

						result_size += run;
						free(result);
						result = NULL;
						result = (char*)calloc(result_size+1,sizeof(char));
						result[result_size] = '\0';

						os_strncat(result,result_size+1,tmp,result_size-run);

						for(j = 0; j < run; j++){
							result[result_size-run+j] = black;
						}

						//printf("result = %s\n",result );



					}else{

						if(strncmp("000000000001",select,12) == 0){ // EOL
							//printf("Warning :: CCITTFaxDecode :: EOL Black reached !\n" );
							//printf("run_white = EOL \n" );
							code_len = i;
							i = max_bits;
						}else{

							// Extended code!!

						}

					}

				}

			}
			
			free(select);
			select = NULL;



		}//end for

		if(code_len > 0){

				// remove the threated code (bits) in the bitstream
				bitstream += code_len;
				len -= code_len;

				//printf("run = %d\n",run);

		}else{

			bitstream ++;
			len --;
			//printf("Warning :: CCITTFaxDecode :: code not found ::  bitstream length =  %d\n",len );

		}


	}

	len = 0;
	out = binarytoChar(result,result_size,&len);
	//printf("return len = %d\n",len);

	obj->tmp_stream_size = len;
	obj->decoded_stream_size = len;

	

	//printf("\n\n\n");
	free(tmp);
	free(result);
	free(binary_mode);

	return out;
}