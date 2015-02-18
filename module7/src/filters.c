#include "pdfAnalyzer.h"
#include "miniz.c"


char * FlateDecode(char * stream, struct pdfObject* obj){

	char * streamd = NULL;

	int res = 0;
	unsigned long streamd_len = 100000; // TODO TOFIX search a way to find the right length


	if ( stream == NULL ){
		printf("Error :: Null stream to decode\n");
		return NULL;
	}

	//printf("stream = %s\n",stream);

	//printf("Flatedecode...\n");

	// Get the decoded stream length

	//obj->stream_size --;
	//streamd_len = obj->stream_size;
	//streamd_len = 10000;

	// printf("Stream size = %d...\n",obj->stream_size);	

	streamd  = (char*)calloc(streamd_len,sizeof(char));

	res = uncompress(streamd,&streamd_len,stream,obj->stream_size);

	if( res != Z_OK){
		printf("Warning :: Flatedecode failed for object \"%s\" :: %d \n",obj->reference, res);
	}

	//printf("deflate res res_d = %d\nlen_d = %d\nstream_d = %s--\n",res,streamd_len,streamd);
	printf("deflate res = %d\n",res);

	obj->decoded_stream_size = streamd_len;

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
	char flag = 0;

	//printf("ASCIIHexDecode...\n");

	if ( stream == NULL){
		printf("Error :: Null stream to decode in obj %s\n", obj->reference);
		return NULL;
	}

	//printf("stream = %s\n",stream);
	len  = strlen(stream);
	//len= obj->stream_size;
	printf("len = %d\n",len);

	tmp = (char*)calloc(2,sizeof(char));


	if(stream[len-1] == '>'){
		has_eod_marker = 1;
		printf("has_eod_marker = %d\n",has_eod_marker);
	}

	// If the length is odd and there is an eod marker (>) padd with a zero.


	out =  (char*)calloc(len,sizeof(char));

	
	//while( i < len-2 || stream[i] != '>' || stream[i+1] != '>'){
	while( i < len-3 ){

		// Ignore white space and non hex characters
		flag  = 0;
		while(flag < 2 && i < len-3){

			if( (stream[i]>=  65 && stream[i]<=70 ) || (stream[i] >=  97 && stream[i]<=102 ) || (stream[i] >=  48 && stream[i]<=57 ) ){ // HExa characters
				tmp[flag] = stream[i];
				flag ++;
			}

			i++;


		}

		//tmp[0] = stream[i];
		//tmp[1] = stream[i+1];

		out[j] = strtol(tmp,NULL,16);
		//printf("tmp = %s\tres = %c\n",tmp,out[j]);
		j++;

	}

	//printf("out = %s\n\n",out);


	return out;
}