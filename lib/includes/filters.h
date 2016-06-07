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
unsigned short readData(char ** data, int * partial_code, int * partial_bits, int code_len);
void printDico(struct LZWdico * dico);

// CCITTFaxDecode functions.
int getRunLengthCodeInTable(char ** table, char * bits, int table_size);
int getMakeUpCodeInTable(char ** table, char *bits, int table_size);


// ASCII85Decode functions
char * getTuple(char * data, int len);


// CCITTFaxDecode

static char * WHITE_RUN_LENGTH_TERMINATING_CODES[] = {
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

static char * BLACK_RUN_LENGTH_TERMINATING_CODES[] = {
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

static char * WHITE_MAKE_UP_CODES[] = {
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

static char * BLACK_MAKE_UP_CODES[] = {
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


static int WHITE_BLACK_MAKE_UP_CODES_VALUES[] = {
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







#endif