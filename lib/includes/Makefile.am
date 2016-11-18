AUTOMAKE_OPTIONS=subdir-objects no-dependencies

noinst_HEADERS = \
$(top_srcdir)/lib/includes/armaditopdf.h \
$(top_srcdir)/lib/includes/filters.h \
$(top_srcdir)/lib/includes/log.h \
$(top_srcdir)/lib/includes/miniz.c \
$(top_srcdir)/lib/includes/osdeps.h \
$(top_srcdir)/lib/includes/pdfAnalysis.h \
$(top_srcdir)/lib/includes/pdfParsing.h \
$(top_srcdir)/lib/includes/pdfStructs.h \
$(top_srcdir)/lib/includes/utils.h


noinst_LIBRARIES = libarmadito-pdf.a

libarmadito_pdf_a_SOURCES = \
$(top_srcdir)/lib/src/armaditopdf.c \
$(top_srcdir)/lib/src/filters.c \
$(top_srcdir)/lib/src/log.c \
$(top_srcdir)/lib/src/osdeps.c \
$(top_srcdir)/lib/src/pdfObjectsAnalysis.c \
$(top_srcdir)/lib/src/pdfParsing.c \
$(top_srcdir)/lib/src/pdfStructAnalysis.c \
$(top_srcdir)/lib/src/pdfStructs.c \
$(top_srcdir)/lib/src/utils.c