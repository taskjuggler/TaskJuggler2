####### kdevelop will overwrite this part!!! (begin)##########


EXTRA_DIST = index.html index-1.html index-2.html index-3.html index-4.html index-5.html index-6.html index-7.html index-8.html index-9.html index-10.html index-11.html

docprefix = $(DESTDIR)$(prefix)/share/doc/packages

install-data-local:
	$(mkinstalldirs) $(docprefix)/taskjuggler/
	$(INSTALL_DATA) $(srcdir)/index.shtml $(docprefix)/taskjuggler/index.shtml
	$(INSTALL_DATA) $(srcdir)/index.html $(docprefix)/taskjuggler/index.html
	$(INSTALL_DATA) $(srcdir)/index-1.html $(docprefix)/taskjuggler/index-1.html
	$(INSTALL_DATA) $(srcdir)/index-2.html $(docprefix)/taskjuggler/index-2.html
	$(INSTALL_DATA) $(srcdir)/index-3.html $(docprefix)/taskjuggler/index-3.html
	$(INSTALL_DATA) $(srcdir)/index-4.html $(docprefix)/taskjuggler/index-4.html
	$(INSTALL_DATA) $(srcdir)/index-5.html $(docprefix)/taskjuggler/index-5.html
	$(INSTALL_DATA) $(srcdir)/index-6.html $(docprefix)/taskjuggler/index-6.html
	$(INSTALL_DATA) $(srcdir)/index-7.html $(docprefix)/taskjuggler/index-7.html
	$(INSTALL_DATA) $(srcdir)/index-8.html $(docprefix)/taskjuggler/index-8.html
	$(INSTALL_DATA) $(srcdir)/index-9.html $(docprefix)/taskjuggler/index-9.html
	$(INSTALL_DATA) $(srcdir)/index-10.html $(docprefix)/taskjuggler/index-10.html
	$(INSTALL_DATA) $(srcdir)/index-11.html $(docprefix)/taskjuggler/index-11.html

uninstall-local:
	-rm -f $(docprefix)/taskjuggler/index.html
	-rm -f $(docprefix)/taskjuggler/index-1.html
	-rm -f $(docprefix)/taskjuggler/index-2.html
	-rm -f $(docprefix)/taskjuggler/index-3.html
	-rm -f $(docprefix)/taskjuggler/index-4.html
	-rm -f $(docprefix)/taskjuggler/index-5.html
	-rm -f $(docprefix)/taskjuggler/index-6.html
	-rm -f $(docprefix)/taskjuggler/index-7.html
	-rm -f $(docprefix)/taskjuggler/index-8.html
	-rm -f $(docprefix)/taskjuggler/index-9.html
	-rm -f $(docprefix)/taskjuggler/index-10.html
	-rm -f $(docprefix)/taskjuggler/index-11.html

index.sgml: ../../../config.h index.sgml.in
	sed 's,TJVER,@VERSION@,' index.sgml.in > index.sgml
	sgml2html index.sgml

all: index.sgml

