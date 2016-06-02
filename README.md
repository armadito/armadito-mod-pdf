------------------------
-- ARMADITO PDF ANALYZER --
------------------------
=====

Copyright (C) Teclib', 2015, 2016

Project home : http://www.teclib-edition.com/teclib-products/armadito-antivirus

See Online documentation at : http://armadito-av.readthedocs.io/en/latest/

What is it?
-----------

Armadito PDF analyzert is a module for PDF documents scanning that includes:

	* a PDF parser

	* an heuristic analyzer that computes the document confidence level

Licensing
---------

Armadito PDF analyzer is licensed under the GPLv3 https://www.gnu.org/licenses/license-list.html#GNUGPLv3	

Dependencies
------------

> miniz.c

FEATURES
---------------------

==> Parsing		<==

* Suppression des commentaires PostScript présents dans le contenu du document.
* Récupération du header du document (Ex: %PDF-1.7).
* Récupération des trailers.
* Récupération des objets (reference, dictionaire, type, stream, filtres, etc).
* Extraction des objets presents dans des "Object Stream".
* Decodage des streams via les filtres : FlateDecode, ASCIIHexDecode, ASCII85Decode, LZWDecode, CCITTFaxDecode.

==> Analyse anti-malware <==

* Tests portant sur la structure du document
	- Vérifier de la conformité du header et de la version PDF;
	- Vérifier que le document ne soit pas chiffré.
	- Vérifier que toutes les pages du document ne soient pas vides.
	- Vérifier la collision de déclaration des objets (objet définis plusieurs fois dans le document).
	- Vérifier de la présence et conformité des trailers.
	- Vérifier de la conformité de la table de reference des objets "XRef table" (offset + entrées).
	- Vérifier la présence de commentaires potentiellement malicieux (dont le but est de tromper les parser).

* Tests portant sur le contenu des objets
	- Présence de contenu potentiellement dangereux (JavaScript, Embedded Files, Formulaires, URI, etc.).
	- Analyse du contenu JavaScript, Embedded File, Formulaire.
	- Analyse du contenu de l'objet "Info";
	- Recherche de mots clés potentiellement dangereux.
	- Recherche de répétition de pattern.
	- Présence d'obfuscation (Hexa) des dictionaires des objets.

==> Evaluation 	<==

* Chaque test décris ci-dessus à un coefficient (parametrable).
* Calcul arithmetique du coefficient de supicious (En fonction des résultats des tests).


LIMITATIONS
---------------------

- PDF version supportées : %PDF-1.1 à %PDF-1.7
- Ne traite pas les documents dont le contenu est chiffrés.
- L'opération consistant à retirer les commentaires postScript est très couteuse en temps pour les fichiers dont la taille est supérieure à 2M.


