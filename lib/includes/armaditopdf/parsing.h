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



#ifndef _PDF_PARSING_H_
#define _PDF_PARSING_H_


#include <armaditopdf/structs.h>

#define LARGE_FILE_SIZE 1500000

/***** PDF Parsing functions prototypes *****/

int pdf_parse(struct pdfDocument * pdf);
char * pdf_get_version_from_data(char * data, unsigned int data_size);
char * pdf_get_version_from_fd(int fd, int * retcode);
int pdf_get_content(struct pdfDocument * pdf);
int pdf_get_trailers(struct pdfDocument * pdf);
int pdf_parse_obj_content(struct pdfDocument * pdf, struct pdfObject * obj);
int pdf_parse_objects(struct pdfDocument * pdf);
char * get_dico_from_data(char *data, unsigned int data_size);
int pdf_get_javascript(struct pdfDocument * pdf, struct pdfObject* obj);
int pdf_get_xfa(struct pdfDocument * pdf, struct pdfObject* obj);
int pdf_get_embedded_file(struct pdfDocument * pdf, struct pdfObject* obj);
int pdf_get_uri(struct pdfDocument * pdf, struct pdfObject * obj);
int pdf_get_actions(struct pdfDocument * pdf, struct pdfObject * obj);
int pdf_get_active_contents(struct pdfDocument * pdf);

// TO RE-IMPLEMENT
int removeComments(struct pdfDocument * pdf);
char *removeCommentLine(char * src, int size, int * ret_len);

#endif
