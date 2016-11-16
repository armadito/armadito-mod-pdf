ARMADITO PDF ANALYZER
=====================
[![Build Status](https://travis-ci.org/armadito/armadito-mod-pdf.svg?branch=DEV)](https://travis-ci.org/armadito/armadito-mod-pdf)
<a href="https://scan.coverity.com/projects/armadito-armadito-mod-pdf">
  <img alt="Coverity Scan Build Status"
       src="https://scan.coverity.com/projects/10496/badge.svg"/>
</a>

Armadito module PDF is an heuristic module for PDF documents analysis.

Copyright (C) Teclib', 2015, 2016

See Online documentation at : http://armadito-av.readthedocs.io/en/latest/

License : GPLv3 <https://www.gnu.org/licenses/license-list.html#GNUGPLv3>

What is it?
-----------

Armadito PDF analyzer is a module for PDF documents scanning that includes:

* a PDF parser

* an heuristic analyzer that computes the document confidence level

Licensing
---------

Armadito PDF analyzer is licensed under the GPLv3 https://www.gnu.org/licenses/license-list.html#GNUGPLv3	

Dependencies
------------

> miniz.c

FEATURES
--------

==> Parsing	<==

* Remove PostScript comments in the content of the document.
* Get PDF version in header (Ex: %PDF-1.7).
* Get trailers and xref table or xref objects.
* Get objects informations described in the document (reference, dictionary, type, stream, filters, etc).
* Extract objects embedded in stream objects.
* Decode object streams encoded with filters : FlateDecode, ASCIIHexDecode, ASCII85Decode, LZWDecode, CCITTFaxDecode

==> Analysis <==

* Tests based on PDF document structure (accodring to PDF specifications):
	- Check the PDF header version (from version 1.1 to 1.7).
	- Check if the content of the document is encrypted.
	- Check that the document contains non-empty pages.
	- Check object collision in object declaration.
	- Check trailers format.
	- Check xref table and xref object.
	- Check the presence of malicious Postscript comments (which could cause parsing errors).


* Tests based on PDF objects content:
	- Get potentially malicious active contents (JavaScripts, Embedded files, Forms, URI, etc.)
	- JavaScript content analysis (malicious keywords, pattern repetition, unicode strings, etc).
	- Info object content analysis (search potentially malicious strings).
	- Check if object dictionary is hexa obfuscated.


==>	Notation <==

* A suspicious coefficient is attributed to each test.
* Calc the suspicious coefficient of the pdf document.


LIMITATIONS
-----------

- Supported PDF versions are: %PDF-1.1 to %PDF-1.7.
- PDF documents with encrypted content are not supported.
- Removing comments is skipped for document > 2MB
