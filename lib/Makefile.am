AUTOMAKE_OPTIONS=subdir-objects no-dependencies

lib_LTLIBRARIES = libarmadito-pdf.la
libarmadito_pdf_la_LDFLAGS = -version-number 0:12:6

AM_CPPFLAGS= -I$(top_srcdir)/lib/includes

libarmadito_pdf_la_SOURCES = \
$(top_srcdir)/lib/src/armaditopdf.c \
$(top_srcdir)/lib/src/filters.c \
$(top_srcdir)/lib/src/log.c \
$(top_srcdir)/lib/src/objectsAnalysis.c \
$(top_srcdir)/lib/src/osdeps.c \
$(top_srcdir)/lib/src/parsing.c \
$(top_srcdir)/lib/src/structAnalysis.c \
$(top_srcdir)/lib/src/structs.c \
$(top_srcdir)/lib/src/utils.c

libarmadito_pdfincludedir=$(includedir)/libarmadito-pdf
libarmadito_pdfinclude_HEADERS = $(top_srcdir)/lib/includes/armaditopdf.h

armadito_pdfincludedir=$(includedir)/libarmadito-pdf/armaditopdf

armadito_pdfinclude_HEADERS =\
$(top_srcdir)/lib/includes/armaditopdf/analysis.h \
$(top_srcdir)/lib/includes/armaditopdf/errors.h \
$(top_srcdir)/lib/includes/armaditopdf/filters.h \
$(top_srcdir)/lib/includes/armaditopdf/log.h \
$(top_srcdir)/lib/includes/armaditopdf/miniz.c \
$(top_srcdir)/lib/includes/armaditopdf/osdeps.h \
$(top_srcdir)/lib/includes/armaditopdf/parsing.h \
$(top_srcdir)/lib/includes/armaditopdf/structs.h \
$(top_srcdir)/lib/includes/armaditopdf/utils.h


pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libarmadito-pdf.pc

libarmadito-pdf.pc: libarmadito-pdf.pc.in
		sed -e 's![@]prefix[@]!$(prefix)!g' \
		    -e 's![@]exec_prefix[@]!$(exec_prefix)!g' \
		    -e 's![@]includedir[@]!$(includedir)!g' \
		    -e 's![@]libdir[@]!$(libdir)!g' \
		    -e 's![@]PACKAGE_VERSION[@]!$(PACKAGE_VERSION)!g' \
$(top_srcdir)/lib/libarmadito-pdf.pc.in > $@