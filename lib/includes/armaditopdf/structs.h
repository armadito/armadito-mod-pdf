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



#ifndef _PDF_STRUCTS_H_
#define _PDF_STRUCTS_H_


#include <stdio.h>
#include <stdlib.h>
#include <string.h>


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
	
	struct pdfObject* next;	// next object in the list.

	
};


// PDF Trailer structure
struct pdfTrailer{

	int offset;	// offset in the document
	char * content; // content of the trailer
	int size;
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


// Suit of tests for PDF objects content
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


// PDF Document structure
struct pdfDocument{
	
	FILE * fh;	// File handle of the document
	int fd;
	char * fname;
	char * content;
	struct pdfObject * objects;	// List of objects
	int coef;	// Suspicious coefficient
	int size;	// size in bytes of the PDF
	char * version;	// PDF specification version
	struct pdfTrailer* trailers;
	struct pdfXRef* xref;
	struct testsPDFStruct * testStruct;
	struct testsPDFObjAnalysis * testObjAnalysis;
	double scan_time; // time elapsed in second for parse or scan.
	int errors; // treatment errors

};



/* pdf structures functions prototypes */

struct pdfDocument* initPDFDocument();
struct pdfDocument * init_pdf_document(int fd, FILE * fh, char * filename, char * version);
struct pdfObject* initPDFObject();
struct pdfTrailer* initPDFTrailer();
struct pdfTrailer * init_pdf_trailer(int content, unsigned int size);

struct testsPDFStruct * initTestsPDFStruct();
struct testsPDFObjAnalysis * initTestsPDFObjAnalysisStruct();

void freePDFDocumentStruct(struct pdfDocument * pdf);
void freePDFObjectStruct(struct pdfObject * obj);
void freePDFTrailerStruct(struct pdfTrailer * trailer);
void free_pdf_trailer(struct pdfTrailer * trailer);
void free_pdf_document(struct pdfDocument * pdf);

int addObjectInList(struct pdfObject* obj, struct pdfDocument* pdf);
int add_pdf_trailer(struct pdfDocument * pdf, struct pdfTrailer * trailer);


#endif
