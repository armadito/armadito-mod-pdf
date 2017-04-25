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




#include <armaditopdf/filters.h>
#include <armaditopdf/miniz.c>
#include <errno.h>
#include <armaditopdf/utils.h>
#include <armaditopdf/osdeps.h>
#include <armaditopdf/log.h>


char * WHITE_RUN_LENGTH_TERMINATING_CODES[] = {
	"00110101",
	"000111",
	"0111",
	"1000",
	"1011",
	"1100",
	"1110",
	"1111",
	"10011",
	"10100",
	"00111",
	"01000",
	"001000",
	"000011",
	"110100",
	"110101",
	"101010",
	"101011",
	"0100111",
	"0001100",
	"0001000",
	"0010111",
	"0000011",
	"0000100",
	"0101000",
	"0101011",
	"0010011",
	"0100100",
	"0011000",
	"00000010",
	"00000011",
	"00011010",
	"00011011",
	"00010010",
	"00010011",
	"00010100",
	"00010101",
	"00010110",
	"00010111",
	"00101000",
	"00101001",
	"00101010",
	"00101011",
	"00101100",
	"00101101",
	"00000100",
	"00000101",
	"00001010",
	"00001011",
	"01010010",
	"01010011",
	"01010100",
	"01010101",
	"00100100",
	"00100101",
	"01011000",
	"01011001",
	"01011010",
	"01011011",
	"01001010",
	"01001011",
	"00110010",
	"00110011",
	"00110100"
};


char * BLACK_RUN_LENGTH_TERMINATING_CODES[] = {
	"0000110111",
	"010",
	"11",
	"10",
	"011",
	"0011",
	"0010",
	"00011",
	"000101",
	"000100",
	"0000100",
	"0000101",
	"0000111",
	"00000100",
	"00000111",
	"000011000",
	"0000010111",
	"0000011000",
	"0000001000",
	"00001100111",
	"00001101000",
	"00001101100",
	"00000110111",
	"00000101000",
	"00000010111",
	"00000011000",
	"000011001010",
	"000011001011",
	"000011001100",
	"000011001101",
	"000001101000",
	"000001101001",
	"000001101010",
	"000001101011",
	"000011010010",
	"000011010011",
	"000011010100",
	"000011010101",
	"000011010110",
	"000011010111",
	"000001101100",
	"000001101101",
	"000011011010",
	"000011011011",
	"000001010100",
	"000001010101",
	"000001010110",
	"000001010111",
	"000001100100",
	"000001100101",
	"000001010010",
	"000001010011",
	"000000100100",
	"000000110111",
	"000000111000",
	"000000100111",
	"000000101000",
	"000001011000",
	"000001011001",
	"000000101011",
	"000000101100",
	"000001011010",
	"000001100110",
	"000001100111"
};


char * WHITE_MAKE_UP_CODES[] = {
	"11011",
	"10010",
	"010111",
	"0110111",
	"00110110",
	"00110111",
	"01100100",
	"01100101",
	"01101000",
	"01100111",
	"011001100",
	"011001101",
	"011010010",
	"011010011",
	"011010100",
	"011010101",
	"011010110",
	"011010111",
	"011011000",
	"011011001",
	"011011010",
	"011011011",
	"010011000",
	"010011001",
	"010011010",
	"011000",
	"010011011"
	
};

char * BLACK_MAKE_UP_CODES[] = {
	"0000001111",
	"000011001000",
	"000011001001",
	"000001011011",
	"000000110011",
	"000000110100",
	"000000110101",
	"0000001101100",
	"0000001101101",
	"0000001001010",
	"0000001001011",
	"0000001001100",
	"0000001001101",
	"0000001110010",
	"0000001110011",
	"0000001110100",
	"0000001110101",
	"0000001110110",
	"0000001110111",
	"0000001010010",
	"0000001010011",
	"0000001010100",
	"0000001010101",
	"0000001011010",
	"0000001011011",
	"0000001100100",
	"0000001100101"
	
};

int WHITE_BLACK_MAKE_UP_CODES_VALUES[] = {
	64,
	128,
	192,
	256,
	320,
	384,
	448,
	512,
	576,
	640,
	704,
	768,
	832,
	896,
	960,
	1024,
	1088,
	1152,
	1216,
	1280,
	1344,
	1408,
	1472,
	1536,
	1600,
	1664,
	1728
};

/*
FlateDecode() :: Decode stream FlateDecode filter.
parameters:
- char * stream (the stream to decode).
- struct pdfObject * obj ( the pointer of the object containing the stream).
returns: (char *)
- the decoded stream on success.
- NULL on error.
TODO :: FlateDecode :: check if the stream is conform (Ex: '\r')
*/
char * FlateDecode(char * stream, struct pdfObject* obj){


	char * dest = NULL;
	char * out = NULL;
	unsigned long src_len = 0;
	unsigned long len = 150000;
	int res = 0;
	

	if (obj == NULL || stream == NULL || obj->tmp_stream_size == 0){
		err_log("FlateDecode :: invalid parameter\n");
		return NULL;
	}

	//dbg_log("FlateDecode :: src_len = %d\n", src_len);
	src_len = obj->tmp_stream_size;

	dest = calloc(len, sizeof(char));

	while ((res = uncompress((unsigned char*)dest, &len, (unsigned char*)stream, src_len)) != Z_OK){

		switch (res){

			case Z_DATA_ERROR:
				err_log("Flatedecode :: Z_DATA_ERROR :: the deflate stream is invalid\n");
				/* TODO :: treat the case when there is NULL character in the dictionary. Ex file : Windows Internals Part 2_6th Edition.pdf  */
				/* debug log */
				/*
				dbg_log("Flatedecode :: res = Z_DATA_ERROR :: len = %d :: strlen = %d :: dest = %s\n", len, strlen(dest),dest);
				printStream(dest,len);
				*/
				obj->decoded_stream_size = 0;
				obj->tmp_stream_size = 0;
				goto clean;

			case Z_BUF_ERROR:
				//warn_log("Flatedecode :: Z_BUF_ERROR :: len = %d\n", len);
				while (res == Z_BUF_ERROR){
					free(dest);
					len += 100000; // increase length
					dest = calloc(len, sizeof(char));
					res = uncompress((unsigned char*)dest, &len, (unsigned char*)stream, src_len);
				}

				break;
			default:
				err_log("Flatedecode :: res = %d\n", res);
				obj->decoded_stream_size = 0;
				obj->tmp_stream_size = 0;
				goto clean;
		}

	}

	
	//dbg_log("Flatedecode :: len = %d\n", len);

	out = (char*)calloc(len+1,sizeof(char));
	out[len]='\0';
	memcpy(out,dest,len);

	obj->decoded_stream_size = len;
	obj->tmp_stream_size = len;

clean:

	free(dest);

	return out;
}


/* Decode ASCIIHexDecode Filter */
char * ASCIIHexDecode(char * stream, struct pdfObject * obj){

	char * out = NULL; 
	char* tmp = NULL; 
	//int has_eod_marker = 0;
	int len = 0;
	int i = 0 ,j =0;
	int flag = 0;
	char c = 0;
	char * end = NULL;

	if ( stream == NULL){
		printf("Error :: ASCIIHexDecode :: Invalid parameter :: obj %s\n", obj->reference);
		return NULL;
	}

	len  = strlen(stream);

	tmp = (char*)calloc(3,sizeof(char));
	tmp[2]='\0';

	// If has EOD marker.
	if(stream[len-1] == '>' || (( stream[len-1] < 65 || stream[len-1] > 70 ) && (stream[len-1] <  97 || stream[len-1] > 102 ) && (stream[len-1] <  48 || stream[len-1] > 57 )) ){
		//has_eod_marker = 1;
		//printf("has_eod_marker = %d\n",has_eod_marker);
		stream[len-1]='\0';
		len --;
	}



	obj->tmp_stream_size = len/2;

	// If the length is odd and there is an eod marker (>) padd with a zero.
	out =  (char*)calloc(len+1,sizeof(char));
	out[len] = '\0';

	end = out;
	
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
		j++;
		end ++;

	}

	obj->tmp_stream_size = j;
	obj->decoded_stream_size = j;

	free(tmp);

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


struct LZWdico * initDico_(int code, char * entry, int len){

	struct LZWdico * new_dico = NULL;

	if((new_dico = (struct LZWdico *)calloc(1,sizeof(struct LZWdico))) == NULL){
		printf("[-] Error :: initDico :: Initialization failed\n");
		return NULL;
	}

	new_dico->code = code;

	//dico->entry = entry;
	new_dico->entry = (char*)calloc(len+1,sizeof(char));
	new_dico->entry[len]='\0';
	memcpy(new_dico->entry,entry,len);

	new_dico->entry_len = len;
	new_dico->next = NULL;

	return new_dico;
}


int addEntryInDico(struct LZWdico * dico , int code, char * entry, int len){


	struct LZWdico * tmp = NULL;

	// check parameters
	if(entry == NULL || dico == NULL || len <= 0 || code <= 0){
		err_log("addInDico :: invalid parameters\n");
		return -1;
	}

	tmp = dico;
	
	while(tmp->next != NULL){
		
		tmp = tmp->next;
	}
	
	tmp->next = initDico_(code, entry,len);

	return 1;

}


int addInDico(struct LZWdico * dico , int code, char * entry){


	// check parameters
	if(entry == NULL || dico == NULL){
		printf("Error :: addInDico :: Bad parameter\n");
		return -1;
	}

	while(dico->next != NULL){
		dico = dico->next;
	}

	dico->next = initDico(code, entry);

	return 1;

}


void freeDico(struct LZWdico * dico){

	struct LZWdico * tmp = NULL;

	while(dico != NULL){

		tmp = dico;
		dico = dico->next;

		free(tmp->entry);
		tmp->entry = NULL;
		tmp->code = 0;
		tmp->entry_len = -1;		
		free(tmp);
		tmp = NULL;
	}

	return ;
}


char * getEntryInDico(struct LZWdico * dico , int code){

	struct LZWdico * tmp = NULL;


	if(dico == NULL){
		printf("Error :: getEntryInDico :: Invalid parameter\n");
		return NULL;
	}

	tmp = dico;

	while(tmp != NULL){

		if(tmp->code == code)
			return tmp->entry;

		tmp = tmp->next;
	}

	return NULL;

}


int getEntryLengthInDico(struct LZWdico * dico , int code){

	struct LZWdico * tmp = NULL;

	if(dico == NULL){
		printf("Error :: getEntryLengthInDico :: Invalid parameter\n");
		return -1;
	}

	
	tmp = dico ;

	while(tmp != NULL){

		if(tmp->code == code)
			return tmp->entry_len;

		tmp = tmp->next;
	}

	return -1;

}


/* Debug :: print LZW dictionary */
void printDico(struct LZWdico * dico){

	struct LZWdico * tmp = NULL;

	if(dico == NULL){
		printf("Empty LZW dictionary !!\n");
		return;
	}

	tmp = dico;

	while( tmp != NULL ){

		printf("Code = %d :: entry =  [%s]\n",tmp->code, tmp->entry);
		tmp = tmp->next;

	}

	return;

}


unsigned short readData(char ** data, unsigned int * partial_code, unsigned int * partial_bits, unsigned int code_len ){

	unsigned short code = 0;
	unsigned char unpack = 0;
	

	while(*partial_bits < code_len){

		if(*partial_code != -1){

			os_sscanf(&(*data[0]),"%c",&unpack);
			
			*partial_code = (*partial_code << 8) + unpack;


		}else{

			os_sscanf(&(*data[0]),"%c",&unpack);			
			*partial_code = unpack;

		}

		*data += 1;
		*partial_bits += 8;
	}

	code = *partial_code >> (*partial_bits - code_len);
	*partial_code &= (1 << (*partial_bits - code_len)) -1;
	*partial_bits -= code_len;
	

	return code;

}


/* Decode LZWDecode Filter */
char * LZWDecode(char* stream, struct pdfObject * obj){

	char * out = NULL;
	char * ptr_data = NULL;
	char * entry = NULL;
	char * w = NULL;
	char * w_entry0  = NULL ; //to add in dico
	char * tmp = NULL;	
	char * out_init_ptr = NULL;

	int stream_len = 0;
	int i = 0, size = 0;	
	unsigned int code_len  = 9;
	unsigned int partial_code = -1;
	unsigned int partial_bits = 0;
	int next_code = FIRST_CODE; //#define FIRST_CODE 258
	int first = 1;
	int out_len = 0;
	int entry_len= 0;
	int w_entry0_len = 0;
	int w_len = 0;
	unsigned short code = 0;
	//unsigned short last_code = 0;
	struct LZWdico* dico = NULL;


	if (stream == NULL || obj == NULL || obj->tmp_stream_size <= 0){
		err_log("LZWDecode :: invalid parameters\n");
		return NULL;
	}


	stream_len = obj->tmp_stream_size;
	out_len = 2 * stream_len;
	ptr_data = stream;


	out_init_ptr = (char*)calloc(out_len+1,sizeof(char));
	out_init_ptr[out_len] = '\0';

	out = out_init_ptr;

	for(i= 0 ; i < stream_len ; i++){		

		// maybe if next_code + 1 == 1 << code_len )
		if( next_code +1== (1<< code_len)){
			// Increase code length.			
			code_len ++;			
		}


		code = readData(&ptr_data, &partial_code, &partial_bits, code_len);
		
		/*Print Debug infos*/
		/*
		printf("ptr_data = %c :: code = %d :: next_code = %d\n",ptr_data[0], code,next_code);
		printf("code = %d :: next_code = %d\n", code,next_code);
		printf("code = %d :: last_code = %d :: next_code = %d :: code_len = %d\n", code,last_code,next_code,code_len);
		printf("last_code = %d \n",last_code);
		printf("i = %d\n",i);
		printf("code = %d :: last_code = %d :: next_code = %d :: code_len = %d :: w = (%s) :: w_len = %d  :: w_entry0 = (%s) \n", code,last_code,next_code,code_len,w,w_len,w_entry0);
		*/

		// free allocated memory.
		if(w_entry0!=NULL){
			free(w_entry0);
			w_entry0 = NULL;
		}

		if(entry!=NULL){
			free(entry);
			entry = NULL;
			entry_len = 0;
		}


		if(code == CLEAR_TABLE){	// Clear table code

			// CLEAR_TABLE marker reached !!
			//printf("clear_table!!");
			code_len = 9;			
			next_code = FIRST_CODE;
			//last_code = code;
			w_len = 0;
			free(w);
			w= NULL;

			//w = entry;
			/*w_len = entry_len;
			w = (char*)calloc(w_len + 1, sizeof(char));
			w[w_len] = '\0';
			memcpy(w, entry, entry_len);
			*/

			freeDico(dico);
			dico = NULL;			
			continue;

		}else if(code == EOD_MARKER){

			// EOD_MARKER reached !! 
			i = stream_len;
			continue;


		}else if(code >= next_code){

			// finish :: bad sequence :: http://marknelson.us/1989/10/01/lzw-data-compression/
			/***************************************************************
		        * We got a code that's not in our dictionary.  This must be due
		        * to the string + char + string + char + string exception.
		        * Build the decoded string using the last character + the
		        * string from the last code.
		    ***************************************************************/

		    /* Debug print */
			/*
			printf("BAD/END SEQUENCE code (%d) > next_code (%d) :: last_code = %d :: size = %d\n",code, next_code, last_code, size);			
		    printf("BAD/END SEQUENCE w = (%s) :: w_len (%d) :: last_code = %d ::\n",w, w_len, last_code);			
		    */

			// Translate the value of last_code => LAST_value => w. or get_entry(last_code)
			//w_entry0 = (char*)calloc(w_len+2,sizeof(char));
			//free(entry); entry= NULL;

			// entry = w + w[0]
			entry_len = w_len+2;
			entry = (char*)calloc(entry_len+1,sizeof(char));
			entry[entry_len]= '\0';
			if(w_len > 0 && w != NULL)
				memcpy(entry,w,w_len);

			// not sure about this line...(to fix)
			//w_entry0[w_len]= w[0];
			//entry[w_len]= w[0];
			if (w != NULL){
				tmp = entry;
				//dbg_log("[1]tmp = %s :: w = %s :: w_len = %d :: entry = %s\n", tmp, w, w_len, entry);
				tmp += w_len;
				//dbg_log("[2]tmp = %s :: w = %s :: w_len = %d :: entry = %s\n", tmp, w, w_len, entry);
				memcpy(tmp, w, 1);
			}
			


		}else if(code < EOD_MARKER){ /// <257
	

			// first occurrence
			if(first == 1){

				if(w!= NULL)
					free(w);

				w = (char*)calloc(2,sizeof(char));
				w[1] = '\0';
				w[0] = (char)((int)'\0' + code);

				// write result in output.
				memcpy(out,w,1);
				w_len = 1;
				out++;
				size++;
				first = 0;
				//last_code = code;
				continue;
			}	

			if(code == 0){
				//printf("[CAUTION] :: NULL character !!!!! ==============>\n");
			}

			// Write entry in result output
			entry_len = 1;
			entry = (char*)calloc(entry_len+1,sizeof(char));
			entry[entry_len] = '\0';
			entry[0] = (char)((int)'\0' + code); // unsigned int to char.
			


		}else if(code > EOD_MARKER){

			
			if((entry_len = getEntryLengthInDico(dico,code)) > 0){	// if code is in the dictionary.				
				

				entry = (char*)calloc(entry_len+1,sizeof(char));
				entry[entry_len] = '\0';
				
				if ((tmp = getEntryInDico(dico,code)) == NULL) {					
					break;
				}
				
				memcpy(entry,tmp,entry_len);				

			}else{

				
				//printf("[!] NOT FOUND IN THE DICO :: code = %d :: entry_len =%d\n",code,entry_len);											
				entry_len = w_len + 1;
				entry = (char*)calloc(entry_len+1,sizeof(char));
				entry[entry_len]= '\0';				

				if(w != NULL && w_len > 0)
					memcpy(entry,w,w_len);
				tmp = entry;
				tmp += w_len;
				memcpy(tmp,w,1);				

			}


			

		}else{

			//printf("[WARNING] :: This case is not treated !! =======\n");

		}

		
		//printf("out_init_ptr = %d :: out = %d :: diff = %d :: size = %d\n",out_init_ptr, out, (out-out_init_ptr), size);
		// Write entry in result output
		
		// TODO :: if size > stream_len
		if(entry != NULL && entry_len > 0)
			memcpy(out,entry,entry_len);
		
		out  += entry_len;
		size += entry_len;
		
		// w_entry0=  w + entry[0] in dico
		// add w_entry0 in dico
		//free(w_entry0); w_entry0= NULL;	
		w_entry0_len = w_len+1;
		w_entry0 = (char*)calloc(w_entry0_len+1,sizeof(char));
		w_entry0[w_entry0_len]= '\0';
		if(w != NULL && w_len > 0)
			memcpy(entry,w,w_len);		
		
		tmp = w_entry0;
		tmp += w_len;
		memcpy(tmp,entry,1);


		// save new entry in the dico : w + entry[0]
		if(dico == NULL){
			dico = initDico_(next_code,w_entry0,w_entry0_len);			
		}			
		else{		
			addEntryInDico(dico,next_code,w_entry0,w_entry0_len);			
		}

		
		// save entry (last_code entry)		
		if(w != NULL){
			free(w);			
			w = NULL;
			w_len = 0;
		}

		//w = entry;
		w_len = entry_len;
		w = (char*)calloc(w_len+1,sizeof(char));			
		w[w_len]='\0';
		if(entry != NULL && entry_len > 0)
			memcpy(w,entry,entry_len);		
		
		next_code ++;
		/* save last character and code for use in unknown code word case */
		//last_code = code;

	} // end_for

	obj->tmp_stream_size = size;
	obj->decoded_stream_size = size;

	out_init_ptr[size]='\0';

	freeDico(dico);
	free(w);
	free(entry);
	free(w_entry0);

	return out_init_ptr;

}


char * getTuple(char * data, int len){

	char * tuple = NULL;	
	int i = 0;
	int size = 5; // tuple size

	if(data == NULL || len == 0){
		return NULL;
	}


	tuple = (char*)calloc(size+1,sizeof(char));
	tuple[size]='\0';

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


/* Decode ASCII85Decode Filter */
char * ASCII85Decode(char * stream, struct pdfObject * obj){


	long stream_len = 0;
	unsigned long base = 0;
	int i = 0, j=0, num = 0, len = 0;
	char * out = NULL;
	//char *out_tmp = NULL;
	char * data_ptr = NULL;
	char * tuple = NULL;
	char * tmp = NULL;
	char * invert = NULL;
	int * tuple_a = NULL;	
	

	if(stream == NULL){
		printf("[-] Error :: ASCII85Decode :: Invalid parameter!\n");
		return NULL;
	}


	stream_len = obj->tmp_stream_size;
	len = stream_len;

	data_ptr = stream;

	// TODO! remove white spaces if any
	out = (char*)calloc(stream_len+1,sizeof(char));
	out[stream_len] = '\0';
	tuple_a = (int*)calloc(6,sizeof(int));

	while(len > 0 && j < stream_len-1){

		base = 0;

		tuple = getTuple(data_ptr,len);
		if(tuple == NULL){
			printf("ERROR :: ASCII85Decode :: Can't get Tuple!!\n" );
			len = 0;
			continue;
		}

		data_ptr += 5;

		len = (int)(data_ptr - stream);
		len = stream_len - len;

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

			// substract 33 to tuple 
			tuple_a[i] -= 33;

			// change in base 85
			base *= 85;
			base += tuple_a[i];

		}


		tmp = (char*)&base; // change unsigned long into char *


		num = 5;
		
		//invert = tmp + num -1;
		invert = tmp + num -2;
		
		// invert 
		for(i = num-2; i >= 0 ; i--){

			j++;
			if (j<stream_len)
				os_strncat(out,stream_len,invert,1);

			invert --;

		}
		
		free(tuple);
	}

	obj->decoded_stream_size = j;
	obj->tmp_stream_size = j;


	free(tuple_a);

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

	for( i = 0; i < table_size ; i ++){

		if( strncmp(table[i],bits,strlen(bits)) == 0 && strncmp(table[i],bits,strlen(table[i])) == 0 ){
			
			return WHITE_BLACK_MAKE_UP_CODES_VALUES[i];
		}
	}

	return -1;
}


/* Decode CCITTFaxDecode Filter */
char * CCITTFaxDecode(char* stream, struct pdfObject * obj){

	char * out = NULL;
	char * bitstream = NULL;
	char * select = NULL;
	char * binary_mode= NULL;
	char * tmp = NULL; // tmp buffer
	char * result = NULL;
	int i = 0, j= 0, len = 0, max_bits = 13, code_len = 0, result_size = 0;
	int color = 1; // white = 1 ; black = 0
	int run = 0;
	char white = '1';
	char black = '0';


	if(stream == NULL){
		err_log("Error :: CCITTFaxDecode :: NULL parameters while decoding stream in object %s\n",obj->reference);
		return NULL;
	}

	len = obj->tmp_stream_size;

	binary_mode = toBinary(stream,len);
	bitstream = binary_mode;


	len = 8*len;


	while(len > 0){

		code_len = 0;

		if(max_bits > len)
			max_bits = len;

		for(i =1; i<= max_bits; i++){



			// select 1 to max_bits code
			select = (char*)calloc(i+1,sizeof(char));
			select[i] = '\0';

			memcpy(select,bitstream,i);		

			// check code table
			if(color == 1){ //white

				if( (run = getMakeUpCodeInTable(WHITE_MAKE_UP_CODES,select,27)) != -1 ){

					code_len = i;
					i = max_bits;					

					// write bits
					free(tmp);
					tmp = (char*)calloc(result_size+1,sizeof(char));
					tmp[result_size] = '\0';

					if(result != NULL && result_size > 0)
						memcpy(tmp,result,result_size);

					result_size += run;
					free(result);
					result = (char*)calloc(result_size+1,sizeof(char));
					result[result_size] = '\0';

					os_strncat(result,result_size+1,tmp,result_size-run);

					for(j = 0; j < run; j++){
						result[result_size-run+j] = white;
					}				



				}else{


					if((run = getRunLengthCodeInTable( WHITE_RUN_LENGTH_TERMINATING_CODES,select,63)) != -1){

						//color = ((color == 1) ? 0:1); // switch color bit
						color = 0; // white to black
						code_len = i;
						i = max_bits;

						// write bits
						free(tmp);
						tmp = (char*)calloc(result_size+1,sizeof(char));
						tmp[result_size] = '\0';
						if(result != NULL && result_size > 0)
							memcpy(tmp,result,result_size);

						result_size += run;
						free(result);
						result = (char*)calloc(result_size+1,sizeof(char));
						result[result_size] = '\0';

						os_strncat(result,result_size+1,tmp,result_size-run);

						for(j = 0; j < run; j++){
							result[result_size-run+j] = white;
						}


					}else{

						if(strncmp("000000000001",select,12) == 0){ // EOL

							// Warning :: CCITTFaxDecode :: EOL White reached !							
							code_len = i;
							i = max_bits;

						}

					}

				}
				

			}else{	// black => color = 0

				if( (run = getMakeUpCodeInTable( BLACK_MAKE_UP_CODES,select,27)) != -1 ){

					code_len = i;
					i = max_bits;

					// write bits	
					free(tmp);
					tmp = (char*)calloc(result_size+1,sizeof(char));
					tmp[result_size] = '\0';
					if(result != NULL && result_size > 0)
						memcpy(tmp,result,result_size);

					result_size += run;
					free(result);
					result = (char*)calloc(result_size+1,sizeof(char));
					result[result_size] = '\0';

					os_strncat(result,result_size+1,tmp,result_size-run);

					for(j = 0; j < run; j++){
						result[result_size-run+j] = black;
					}


				}else{

					if((run = getRunLengthCodeInTable( BLACK_RUN_LENGTH_TERMINATING_CODES,select,63)) != -1){

						//color = (color == 1) ? 0:1; // switch color bit
						color = 1; // black to white.
						code_len = i;
						i = max_bits;

						// write bits
						free(tmp);
						tmp = (char*)calloc(result_size+1,sizeof(char));
						tmp[result_size] = '\0';
						if(result != NULL && result_size > 0)
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



					}else{

						if(strncmp("000000000001",select,12) == 0){ // EOL
							// Warning :: CCITTFaxDecode :: EOL Black reached !
							code_len = i;
							i = max_bits;
						}else{

							// Extended code!!

						}

					}

				}

			}
			
			free(select);


		}//end for

		if(code_len > 0){

				// remove the threated code (bits) in the bitstream
				bitstream += code_len;
				len -= code_len;

		}else{

			bitstream ++;
			len --;
			//printf("Warning :: CCITTFaxDecode :: code not found ::  bitstream length =  %d\n",len );

		}


	}

	len = 0;
	if(result != NULL)
		out = binarytoChar(result,result_size,&len);

	obj->tmp_stream_size = len;
	obj->decoded_stream_size = len;

	free(tmp);
	free(result);
	free(binary_mode);

	return out;
}