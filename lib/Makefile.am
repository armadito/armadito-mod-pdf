AUTOMAKE_OPTIONS=subdir-objects no-dependencies

lib_LTLIBRARIES = libarmadito-pdf.la
libarmadito_pdf_la_LDFLAGS = -version-number 0:12:6

AM_CPPFLAGS= -I$(top_srcdir)/lib/includes

libarmadito_pdf_la_SOURCES = \
$(top_srcdir)/lib/src/armaditopdf.c \
$(top_srcdir)/lib/src/filters.c \
$(top_srcdir)/lib/src/log.c \
$(top_srcdir)/lib/src/osdeps.c \
$(top_srcdir)/lib/src/pdfObjectsAnalysis.c \
$(top_srcdir)/lib/src/pdfParsing.c \
$(top_srcdir)/lib/src/pdfStructAnalysis.c \
$(top_srcdir)/lib/src/pdfStructs.c \
$(top_srcdir)/lib/src/utils.c

armadito_pdfincludedir=$(includedir)/libarmadito-pdf

armadito_pdfinclude_HEADERS =\
$(top_srcdir)/lib/includes/armaditopdf.h \
$(top_srcdir)/lib/includes/filters.h \
$(top_srcdir)/lib/includes/log.h \
$(top_srcdir)/lib/includes/miniz.c \
$(top_srcdir)/lib/includes/osdeps.h \
$(top_srcdir)/lib/includes/pdfAnalysis.h \
$(top_srcdir)/lib/includes/pdfParsing.h \
$(top_srcdir)/lib/includes/pdfStructs.h \
$(top_srcdir)/lib/includes/utils.h


pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libarmadito-pdf.pc

libarmadito-pdf.pc: libarmadito-pdf.pc.in
		sed -e 's![@]prefix[@]!$(prefix)!g' \
		    -e 's![@]exec_prefix[@]!$(exec_prefix)!g' \
		    -e 's![@]includedir[@]!$(includedir)!g' \
		    -e 's![@]libdir[@]!$(libdir)!g' \
		    -e 's![@]PACKAGE_VERSION[@]!$(PACKAGE_VERSION)!g' \
$(top_srcdir)/lib/libarmadito-pdf.pc.in > $@