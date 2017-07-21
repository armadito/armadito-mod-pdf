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



#ifndef _PDF_ANALYSIS_H_
#define _PDF_ANALYSIS_H_


#include <armaditopdf/structs.h>


/***** pdf Structure analysis functions prototypes *****/
int documentStructureAnalysis(struct pdfDocument * pdf);
int checkXRef(struct pdfDocument * pdf);
int checkEmptyDocument(struct pdfDocument * pdf);
int checkTrailer(struct pdfDocument * pdf);
int pdf_check_valid_trailer(struct pdfDocument * pdf);


/***** pdf Objects analysis functions prototypes *****/
int pdf_obj_analysis(struct pdfDocument * pdf);
int getDangerousContent(struct pdfDocument* pdf);
int pdf_get_javascript(struct pdfDocument * pdf, struct pdfObject* obj);
int getXFA(struct pdfDocument * pdf, struct pdfObject* obj);
int getEmbeddedFile(struct pdfDocument * pdf, struct pdfObject* obj);
int getInfoObject(struct pdfDocument * pdf);
int unknownPatternRepetition(char * stream, int size, struct pdfDocument * pdf, struct pdfObject * obj);
int findDangerousKeywords(char * stream, struct pdfDocument * pdf, struct pdfObject * obj);
int getURI(struct pdfDocument * pdf, struct pdfObject * obj);
int getJSContentInXFA(char * stream, int size, struct pdfObject * obj, struct pdfDocument * pdf);



#endif
