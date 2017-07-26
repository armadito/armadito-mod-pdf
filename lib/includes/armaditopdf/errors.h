#ifndef _PDF_ERRORS_H
#define _PDF_ERRORS_H
#endif


/* PDF library errors definitions */
#define ERROR_SUCCESS 0

// Errors flags
#define ERROR_ON_LOAD 				(1 << 8)	/* Errors during loading */
#define ERROR_ON_PARSING 			(2 << 8)	/* Errors during pdf parsing */
#define ERROR_ON_TRAILER_PARSING 	(3 << 8)	/* Errors during trailer parsing */
#define ERROR_ON_XREF_PARSING 		(4 << 8)	/* Errors during xref parsing */
#define ERROR_ON_OBJ_PARSING 		(5 << 8)	/* Errors during obj parsing */
#define ERROR_ON_STRUCT_ANALYSIS	(6 << 8)	/* Errors during struct parsing */
#define ERROR_ON_OBJ_ANALYSIS 		(7 << 8)	/* Errors during obj analysis */


#define ERROR_INVALID_FD			1 		/* (invalid file handle or file descriptor) */
#define ERROR_FILE_NOT_FOUND		2
#define ERROR_ZERO_LENGTH_FILE		3
#define ERROR_INSUFFICENT_MEMORY	4
#define ERROR_FILE_READ				5
#define ERROR_INVALID_FORMAT		6

#define ERROR_INVALID_PARAMETERS	7
#define ERROR_TRAILER_NOT_FOUND 	8
#define ERROR_BAD_TRAILER_FORMAT	9

#define ERROR_BAD_OBJ_FORMAT		10
#define ERROR_BAD_OBJ_REF_FORMAT	11
#define ERROR_OBJ_DICO_NOT_FOUND	12
#define ERROR_OBJ_DICO_OBFUSCATION	13
#define ERROR_INVALID_OBJ_TYPE		14
#define ERROR_INVALID_OBJ_STREAM	15
#define ERROR_NO_STREAM_FILTERS		16
#define ERROR_INVALID_STREAM_FILTERS		17
#define ERROR_STREAM_FLATEDECODE 	18
#define ERROR_STREAM_ASCIIHEXDECODE 19
#define ERROR_STREAM_ASCII85DECODE 	20
#define ERROR_STREAM_LZWDECODE 		21
#define ERROR_STREAM_RLEDECODE 		22
#define ERROR_STREAM_CCITTFAXDECODE 23
#define ERROR_FILTER_NOT_IMPLEMENTED		24
#define ERROR_INVALID_OBJSTM_DICO 	25
#define ERROR_INVALID_OBJSTM_FORMAT 26
#define ERROR_ENCRYPTED_CONTENT 	27

#define ERROR_OBJ_REF_NOT_FOUND 	28
#define ERROR_INVALID_XREF_OFFSET 	29
#define ERROR_INVALID_XREF_FORMAT	30
#define ERROR_NO_JS_FOUND 			31
#define ERROR_NO_XFA_FOUND 			32
#define ERROR_NO_EF_FOUND 			33
#define ERROR_NO_URI_FOUND 			34
#define ERROR_NO_ACTION_FOUND		35
#define ERROR_INVALID_DICO 			36
