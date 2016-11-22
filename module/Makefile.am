AUTOMAKE_OPTIONS=subdir-objects

modulesdir=$(libdir)/armadito/modules
modules_LTLIBRARIES=modulePDF.la


modulePDF_la_SOURCES= \
modulePDF.c \
modulePDF.h


AM_CPPFLAGS= 
modulePDF_la_LDFLAGS= -module -avoid-version -export-symbols "$(srcdir)/modulePDF.sym"
modulePDF_la_CFLAGS= -I$(top_srcdir)/lib/includes
modulePDF_la_LIBADD=../lib/libarmadito-pdf.la
modulePDF_la_CFLAGS+= @LIBARMADITO_CFLAGS@ 
modulePDF_la_LIBADD+= @LIBARMADITO_LIBS@


install-exec-hook:
	-rm -f "$(DESTDIR)$(modulesdir)"/modulePDF.la "$(DESTDIR)$(modulesdir)"/modulePDF.a

install-data-hook:
	-rm -f "$(DESTDIR)$(modulesdir)"/modulePDF.la "$(DESTDIR)$(modulesdir)"/modulePDF.a