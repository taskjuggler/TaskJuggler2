EXTRA_DIST = taskjuggler/*.html

docprefix = $(DESTDIR)$(prefix)/share/doc/packages

install-data-local:
	$(mkinstalldirs) $(docprefix)/taskjuggler/
	$(INSTALL_DATA) $(srcdir)/taskjuggler/*.html $(docprefix)/taskjuggler/
	$(INSTALL_DATA) $(srcdir)/taskjuggler.ps $(docprefix)/taskjuggler

uninstall-local:
	-rm -f $(docprefix)/taskjuggler/*.html
	-rm -f $(docprefix)/taskjuggler/taskjuggler.ps

taskjuggler/book1.html: taskjuggler.sgml
	db2html taskjuggler.sgml

taskjuggler.ps: taskjuggler.sgml
	db2ps taskjuggler.sgml

all: taskjuggler/book1.html taskjuggler.ps

