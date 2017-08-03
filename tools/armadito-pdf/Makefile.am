AUTOMAKE_OPTIONS=subdir-objects no-dependencies

AM_CPPFLAGS=
armadito_pdf_LDADD= @LIBARMADITO_PDF_LIBS@
armadito_pdf_CFLAGS= @LIBARMADITO_PDF_CFLAGS@
armadito_pdf_CFLAGS= -I$(top_srcdir)/lib/includes -I$(includedir)/libarmadito-pdf

bin_PROGRAMS = armadito-pdf
armadito_pdf_SOURCES= main.c