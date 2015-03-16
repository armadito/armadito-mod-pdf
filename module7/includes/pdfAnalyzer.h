#ifndef pdfAnalyzer
#define pdfAnalyzer

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
//#include <regex.h>

// FILTERS
/*#define FlateDecode 10
#define AsciiHexDecode 11
#define Ascii86Decode 12
#define
*/


// Define Tests Coefficients

//#define bad_header 
//#define encrypted
#define EMPTY_PAGE_CONTENT 99
#define OBJECT_COLLISION 10
#define OBJECT_COLLISION_AND_BAD_XREF 70
#define BAD_TRAILER 40
#define BAD_XREF_OFFSET 30
#define BAD_OBJ_OFFSET 20
#define OBFUSCATED_OBJECT 50 
#define MULTIPLE_HEADERS 50
#define MALICIOUS_COMMENTS 50

#define ACTIVE_CONTENT 40
#define SHELLCODE 40
#define PATTERN_HIGH_REPETITION 40
#define DANGEROUS_KEYWORD_HIGH 90
#define DANGEROUS_KEYWORD_MEDIUM 40
#define DANGEROUS_KEYWORD_LOW 20
#define TIME_EXCEEDED 20

#define LARGE_FILE_SIZE 1500000

//#define DEBUG 1

// PDF object structure
struct pdfObject{
	
	char * reference; // reference of the object Ex : 12 0 obj
	char * content; // The content of the object obj...endobj
	char * dico;	// The dictionary (if any)
	char * type;	// The type of the object (if any)
	char * stream;	// The content stream. stream...endstream
	char * filters;
	char * decoded_stream;
	int offset;	// offset (in byte) in the file
	int stream_size;	// Size in byte of the object's stream
	int tmp_stream_size; // temp size of the stream (between two decoding process)
	int decoded_stream_size;	// Size in byte of the object's decoded stream
	int content_size;	// size in byte of the object's content

	int errors;		// errors in parsing
	//enum efilter * filter;
	//filter* stream_filter[];
	
	struct pdfObject* next;	// next object in the list.
	
};

// PDF Trailer structure
struct pdfTrailer{

	int offset;	// offset in the document
	char * content; // content of the trailer
	char * dico;
	struct pdfTrailer* next;	// next trailer in the document
	
};


// PDF Cross-reference table structure
struct pdfXRef{

	int offset;	// offset in the document
	char * content; // content of the XRef
	struct pdfXRef* next;	// next trailer in the document
	
};


// Suit of tests according to the PDF structure specifications.
struct testsPDFStruct{

	int bad_header;	// when the PDF header is incorrect
	int encrypted;	//  when the document is encrypted
	int empty_page_content;	// when all pages are empty of content
	int object_collision;	// when two objects have the same reference in the document.
	int bad_trailer;	// when the trailer is in an incorrect form
	int bad_xref_offset; // when the offset of the xref table is incorrect;
	int bad_obj_offset; // When at least an object's offset in the reference table is incorrect
	int obfuscated_object;	// when an object dictionary is obfuscated within hexa
	int multiple_headers; // when several headers are found in the document.
	int large_file;
	int comments;	// If PostScript comments are found in pdf.
	int malicious_comments; // Malicious comments found (potentially defeat pdf parsers).

};


struct testsPDFObjAnalysis{

	int active_content;	// presence of js, embedded files, or forms.
	int shellcode;	// presence of shellcode in an object stream content
	int pattern_high_repetition;	// high scale repetition of a pattern in a stream content
	int dangerous_keyword_high;	// potentially dangerous keyword (high level)
	int dangerous_keyword_medium;	// potentially dangerous keyword (medium level)
	int dangerous_keyword_low;	// potentially dangerous keyword (lowlevel)
	int time_exceeded;	// when the analysis of an object stream exceed a given duration.	

	int js; // number of js content
	int xfa; // number of xfa objects
	int ef; // number of ef objects


};

/*
struct testCVEs{

};
*/




// PDF Document structure
struct pdfDocument{
	
	FILE * fh;	// File handle of the document
	char * content;
	struct pdfObject * objects;	// List of objects
	int coef;	// Suspicious coefficient
	int size;	// size in bytes of the PDF
	char * version;	// PDF specification version
	struct pdfTrailer* trailers;
	struct pdfXRef* xref;
	struct testsPDFStruct * testStruct;
	struct testsPDFObjAnalysis * testObjAnalysis;
	int errors; // treatment errors

};


/* Functions */
void Helper();
int analyze(char * filename);
int calcSuspiciousCoefficient(struct pdfDocument * pdf);
int analysisReport(struct pdfDocument * pdf, char * filename);
void debugPrint(char * stream, int len); // print in a debug file


/***** Utils functions *****/
void freePDFDocumentStruct(struct pdfDocument * pdf);
void freePDFObjectStruct(struct pdfObject * obj);
void freePDFTrailerStruct(struct pdfTrailer * trailer);
void * searchPattern(char* src, char* pat , int pat_size ,  int size);
int addObjectInList(struct pdfObject* obj, struct pdfDocument* pdf);
int addTrailerInList(struct pdfDocument * pdf, struct pdfTrailer * trailer);
struct pdfObject * getPDFObjectByRef(struct pdfDocument * pdf, char * ref);
struct pdfDocument* initPDFDocument();
struct pdfObject* initPDFObject();
struct pdfTrailer* initPDFTrailer();
struct testsPDFStruct * initTestsPDFStruct();
struct testsPDFObjAnalysis * initTestsPDFObjAnalysisStruct();
void printObject(struct pdfObject * obj);
void printObjectByRef(struct pdfDocument * pdf, char * ref);
void printObjectInFile(struct pdfObject * obj);
void printPDFObjects(struct pdfDocument * pdf);
int getNumber(char* ptr, int size);
char* getNumber_a(char* ptr, int size);
char * getIndirectRef(char * ptr, int size);
char * getDelimitedStringContent(char * src, char * delimiter1, char * delimiter2, int src_len);
char * getIndirectRefInString(char * ptr, int size);
char * getPattern(char * ptr, int size, int len);
char * getUnicodeInString(char * stream, int size);
char * getHexa(char * dico, int size);
char * replaceInString(char * src, char * toReplace , char * pat);
char * toBinary(char * stream, int size);
char * binarytoChar(char * binary, int size, int * returned_size);
void printStream(char * stream, int size);



/***** pdf Parsing functions *****/
int parsePDF(struct pdfDocument * pdf);
int checkMagicNumber(struct pdfDocument * pdf);	// Check
int getPDFContent(struct pdfDocument * pdf);	// check
char * getObjectDictionary(struct pdfObject * obj, struct pdfDocument * pdf);
char * getObjectType(struct pdfObject * obj);
char * getObjectStream(struct pdfObject * obj);
char * getStreamFilters(struct pdfObject * obj);
int extractObjectFromObjStream(struct pdfDocument * pdf, struct pdfObject *obj);
int getObjectInfos(struct pdfObject * obj, struct pdfDocument * pdf);
int getPDFObjects(struct pdfDocument * pdf);
int getPDFTrailers_1(struct pdfDocument * pdf);
int getPDFTrailers_2(struct pdfDocument * pdf);
int decodeObjectStream(struct pdfObject * obj);
char * hexaObfuscationDecode(char * dico);
char *removeCommentLine(char * src, int size, int * ret_len);
int removeComments(struct pdfDocument * pdf);


/***** filters functions *****/
char * FlateDecode(char * stream, struct pdfObject* obj);
char * ASCIIHexDecode(char * stream, struct pdfObject * obj);
char * LZWDecode(char* stream, struct pdfObject * obj);
char * ASCII85Decode(char * stream, struct pdfObject * obj);
char * CCITTFaxDecode(char* stream, struct pdfObject * obj);



/***** pdf Structure analysis functions *****/
int documentStructureAnalysis(struct pdfDocument * pdf);
int checkXRef(struct pdfDocument * pdf);
int checkEmptyDocument(struct pdfDocument * pdf);
int checkTrailer(struct pdfDocument * pdf);


/***** pdf Objects analysis functions *****/
int getDangerousContent(struct pdfDocument* pdf);
int getJavaScript(struct pdfDocument * pdf, struct pdfObject* obj);
int getXFA(struct pdfDocument * pdf, struct pdfObject* obj);
int getEmbeddedFile(struct pdfDocument * pdf , struct pdfObject* obj);
int getInfoObject(struct pdfDocument * pdf);
int unknownPatternRepetition(char * stream, int size,struct pdfDocument * pdf, struct pdfObject * obj);
int findDangerousKeywords(char * stream , struct pdfDocument * pdf, struct pdfObject * obj);
int getURI(struct pdfDocument * pdf, struct pdfObject * obj);

#endif
