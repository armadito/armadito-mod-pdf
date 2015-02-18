#ifndef pdfAnalyzer
#define pdfAnalyzer

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <regex.h>

// FILTERS
/*#define FlateDecode 10
#define AsciiHexDecode 11
#define Ascii86Decode 12
#define
*/


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
	int decoded_stream_size;	// Size in byte of the object's decoded stream
	int content_size;	// size in byte of the object's content
	//enum efilter * filter;
	//filter* stream_filter[];
	
	struct pdfObject* next;	// next object in the list.
	
};

// PDF Trailer structure
struct pdfTrailer{

	int offset;	// offset in the document
	char * content; // content of the trailer
	struct pdfTrailer* next;	// next trailer in the document
	
};


// PDF Cross-reference table structure
struct pdfXRef{

	int offset;	// offset in the document
	char * content; // content of the XRef
	struct pdfXRef* next;	// next trailer in the document
	
};


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

};


/* Functions */



/***** Utils functions *****/
void Helper();
void freePDFDocumentStruct(struct pdfDocument * pdf);
void * searchPattern(char* src, char* pat , int pat_size ,  int size);
int addObjectInList(struct pdfObject* obj, struct pdfDocument* pdf);
struct pdfObject * getPDFObjectByRef(struct pdfDocument * pdf, char * ref);
struct pdfDocument* initPDFDocument();
struct pdfObject* initPDFObject();
struct pdfTrailer* initPDFTrailer();
int analyze(char * filename);
void printObject(struct pdfObject * obj);
int getNumber(char* ptr, int size);
char* getNumber_a(char* ptr, int size);


/***** pdf Parsing functions *****/
int checkMagicNumber(struct pdfDocument * pdf);
int getPDFContent(struct pdfDocument * pdf);
char * getObjectDictionary(struct pdfObject * obj);
char * getObjectType(struct pdfObject * obj);
char * getObjectStream(struct pdfObject * obj);
char * getStreamFilters(struct pdfObject * obj);
int extractObjectFromObjStream(struct pdfDocument * pdf, struct pdfObject *obj);
int getObjectInfos(struct pdfObject * obj);
int getPDFObjects(struct pdfDocument * pdf);
int getPDFTrailers_1(struct pdfDocument * pdf);
int getPDFTrailers_2(struct pdfDocument * pdf);
int decodeObjectStream(struct pdfObject * obj);


/***** filters functions *****/
char * FlateDecode(char * stream, struct pdfObject* obj);
char * ASCIIHexDecode(char * stream, struct pdfObject * obj);


/***** pdf Structure analysis functions *****/


/***** pdf Objects analysis functions *****/

#endif
