#ifndef _PDF_ERRORS_H
#define _PDF_ERRORS_H
#endif


/* PDF library errors definitions */
#define ERROR_SUCCESS 0

// Errors flags
#define ERROR_ON_LOAD 				(1 << 4)	/* Errors during loading */
#define ERROR_ON_PARSING 			(2 << 4)	/* Errors during pdf parsing */
#define ERROR_ON_TRAILER_PARSING 	(3 << 4)	/* Errors during trailer parsing */
#define ERROR_ON_XREF_PARSING 		(4 << 4)	/* Errors during xref parsing */
#define ERROR_ON_OBJ_PARSING 		(5 << 4)	/* Errors during obj parsing */


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