####### kdevelop will overwrite this part!!! (begin)##########


EXTRA_DIST = index.html index-1.html index-2.html index-3.html index-4.html index-5.html index-6.html index-7.html index-8.html index-9.html index-10.html index-11.html

docprefix = $(prefix)/share/doc/packages

install-data-local:
	$(mkinstalldirs) $(docprefix)/taskjuggler/
	$(INSTALL_DATA) $(srcdir)/index.sgml $(docprefix)/taskjuggler/index.sgml
	$(INSTALL_DATA) $(srcdir)/index.html $(docprefix)/taskjuggler/index.html
	$(INSTALL_DATA) $(srcdir)/index-*.html $(docprefix)/taskjuggler/

uninstall-local:
	-rm -f $(docprefix)/taskjuggler/index*

index.sgml: ../../../config.h index.sgml.in
	sed 's,TJVER,@VERSION@,' index.sgml.in > index.sgml
	sgml2html index.sgml

all: index.sgml

