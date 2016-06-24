AUTOMAKE_OPTIONS=subdir-objects

modulesdir=$(libdir)/armadito/modules
modules_LTLIBRARIES=modulePDF.la

modulePDF_la_SOURCES= \
modulePDF.c \
modulePDF.h \
../lib/includes/utils.h \
../lib/includes/armaditopdf.h \
../lib/includes/filters.h \
../lib/includes/log.h \
../lib/includes/osdeps.h \
../lib/includes/pdfAnalysis.h \
../lib/includes/pdfparsing.h \
../lib/includes/pdfStructs.h \
../lib/src/armaditopdf.c \
../lib/src/filters.c \
../lib/src/log.c \
../lib/src/osdeps.c \
../lib/src/pdfObjectsAnalysis.c \
../lib/src/pdfParsing.c \
../lib/src/pdfStructAnalysis.c \
../lib/src/pdfStructs.c \
../lib/src/utils.c

AM_CPPFLAGS= 
modulePDF_la_LDFLAGS= -module -avoid-version -export-symbols "$(srcdir)/modulePDF.sym"
modulePDF_la_CFLAGS= -I$(top_srcdir)/lib/includes
modulePDF_la_LIBADD=
modulePDF_la_CFLAGS+= @LIBARMADITO_CFLAGS@ 
modulePDF_la_LIBADD+= @LIBARMADITO_LIBS@


install-exec-hook:
	-rm -f "$(DESTDIR)$(modulesdir)"/modulePDF.la "$(DESTDIR)$(modulesdir)"/modulePDF.a

install-data-hook:
	-rm -f "$(DESTDIR)$(modulesdir)"/modulePDF.la "$(DESTDIR)$(modulesdir)"/modulePDF.a
