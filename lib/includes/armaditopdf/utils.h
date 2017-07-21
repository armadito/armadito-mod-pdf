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



#ifndef _UTILS_H_
#define _UTILS_H_


#include <armaditopdf/structs.h>

/* Utils functions prototypes */

void * searchPattern(char* src, char* pat , int pat_size ,  int size);
struct pdfObject * getPDFObjectByRef(struct pdfDocument * pdf, char * ref);
struct pdfObject * getPDFNextObjectByRef(struct pdfDocument * pdf, struct pdfObject * obj, char * ref);
void printObject(struct pdfObject * obj);
void printObjectByRef(struct pdfDocument * pdf, char * ref);
void printObjectInFile(struct pdfObject * obj);
void printPDFObjects(struct pdfDocument * pdf);
int getNumber(char* ptr, int size);
char* getNumber_s(char* ptr, int size);
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
void debugPrint(char * stream, int len); // print in a debug file
void print_actives_contents(struct pdfDocument * pdf);




#endif
