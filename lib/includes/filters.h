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


    
#ifndef _filters_h_
#define _filters_h_


#include "pdfStructs.h"


// LZWDecode
#define FIRST_CODE 258
#define EOD_MARKER 257
#define CLEAR_TABLE 256

#define MAX_CODES 512

struct LZWdico{

	unsigned short code;
	char * entry;
	int entry_len;

	struct LZWdico * next;
};

/* Functions prototypes */

char * FlateDecode(char * stream, struct pdfObject* obj);
char * ASCIIHexDecode(char * stream, struct pdfObject * obj);
char * LZWDecode(char* stream, struct pdfObject * obj);
char * ASCII85Decode(char * stream, struct pdfObject * obj);
char * CCITTFaxDecode(char* stream, struct pdfObject * obj);

// LZWDdecode functions.
struct LZWdico * initDico(int code, char * entry);
struct LZWdico * initDico_(int code, char * entry, int len);
int addInDico(struct LZWdico * dico, int code, char * entry);
void freeDico(struct LZWdico * dico);
char * getEntryInDico(struct LZWdico * dico, int code);
unsigned short readData(char ** data, unsigned int * partial_code, unsigned int * partial_bits, unsigned int code_len);
void printDico(struct LZWdico * dico);

// CCITTFaxDecode functions.
int getRunLengthCodeInTable(char ** table, char * bits, int table_size);
int getMakeUpCodeInTable(char ** table, char *bits, int table_size);


// ASCII85Decode functions
char * getTuple(char * data, int len);


// CCITTFaxDecode

extern char * WHITE_RUN_LENGTH_TERMINATING_CODES[];
extern char * BLACK_RUN_LENGTH_TERMINATING_CODES[];
extern char * WHITE_MAKE_UP_CODES[];
extern char * BLACK_MAKE_UP_CODES[];
extern int WHITE_BLACK_MAKE_UP_CODES_VALUES[];


#endif