EXTRA_DIST = taskjuggler/*.html

docprefix = $(DESTDIR)$(prefix)/share/doc/packages/taskjuggler/manual

SGMLSOURCES = taskjuggler.sgml reference.sgml
CLEANFILES = *.log *.aux *.tex taskjuggler.sgml *.ps *.dvi

install-data-local:
	$(mkinstalldirs) $(docprefix)
	$(INSTALL_DATA) $(srcdir)/taskjuggler/*.html $(docprefix)
	$(INSTALL_DATA) $(srcdir)/taskjuggler.ps $(docprefix)

uninstall-local:
	-rm -f $(docprefix)

taskjuggler/book1.html: $(SGMLSOURCES)
	db2html taskjuggler.sgml

taskjuggler.ps: $(SGMLSOURCES)
	db2ps taskjuggler.sgml
	-rm -rf taskjuggler.junk

all: taskjuggler/book1.html taskjuggler.ps

