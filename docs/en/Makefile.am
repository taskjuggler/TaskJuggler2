EXTRA_DIST = taskjuggler/*.html 
docprefix = /usr/share/doc/packages/taskjuggler/
kdeprefix = $(prefix)/share/doc/HTML/en/taskjuggler/

SGMLSOURCES = taskjuggler.xml ChangeLog.xml FirstProject.xml Example.xml SyntaxReference.xml GenPropertyReference.xml index.docbook
CLEANFILES = *.log *.aux *.dvi *.ps *.tex taskjuggler.xml GenPropertyReference.xml Example.xml

install-data-local:
	$(mkinstalldirs) -m 755 $(DESTDIR)$(kdeprefix)
	$(INSTALL_DATA) $(srcdir)/index.docbook $(DESTDIR)$(kdeprefix)
	$(INSTALL_DATA) $(srcdir)/index.cache.bz2 $(DESTDIR)$(kdeprefix)
	$(INSTALL_DATA) $(srcdir)/*.xml $(DESTDIR)$(kdeprefix)
	$(mkinstalldirs) -m 755 $(DESTDIR)$(docprefix)
	@if test "${HTMLMANUAL}" != "no" ; then \
	  $(INSTALL_DATA) $(srcdir)/taskjuggler/*.html $(DESTDIR)$(docprefix);\
	fi
	@if test "${PSMANUAL}" != "no" ; then \
	  $(INSTALL_DATA) $(srcdir)/taskjuggler.ps $(DESTDIR)$(docprefix);\
	fi

uninstall-local:
	-rm -f $(DESTDIR)$(kdeprefix)
	-rm -f $(DESTDIR)$(docprefix)

Example.xml : ../../Examples/FirstProject/AccountingSoftware.tjp
	rm -f Example.xml
	echo "<para><screen>" > Example.xml
	sed 's/&/\&amp;/g; s/</\&lt;/g; s/>/\&gt;/g' ../../Examples/FirstProject/AccountingSoftware.tjp | expand -t 2 >> Example.xml
	echo "</screen></para>" >> Example.xml

GenPropertyReference.xml: PropertyReference.xml PropertyReference.xsl
	xsltproc PropertyReference.xml PropertyReference.xsl -o \
	  GenPropertyReference.pre.xml
	sed "s/<?xml version=\"1.0\"?>//" GenPropertyReference.pre.xml > \
	  GenPropertyReference.xml
	rm GenPropertyReference.pre.xml  

index.docbook : taskjuggler.xml
	cp taskjuggler.xml index.docbook
	patch index.docbook < kde-doc.patch

index.cache.bz2 : index.docbook GenPropertyReference.xml
	meinproc --check --cache index.cache.bz2 ./index.docbook

taskjuggler/book1.html: $(SGMLSOURCES)
	@if test "${HTMLMANUAL}" != "no" ; then \
	  mkdir -p taskjuggler ; \
	  db2html --xencoding utf-8 taskjuggler.xml ;\
	  cp index.html taskjuggler/index.html ; \
	  rm -rf taskjuggler.junk ;\
	fi

taskjuggler.ps: $(SGMLSOURCES)
	@if test "${PSMANUAL}" != "no" ; then \
	  db2ps --xencoding utf-8 taskjuggler.xml ;\
	  rm -rf db2html.* taskjuggler.aux taskjuggler.log \
	    CATALOG.local *.dsl ; \
	fi

taskjuggler.pdf: $(SGMLSOURCES)
	@if test "${PSMANUAL}" != "no" ; then \
	  db2ps --pdf --xencoding utf-8 taskjuggler.xml ;\
	  rm -rf db2html.* taskjuggler.aux taskjuggler.log \
	    CATALOG.local *.dsl ; \
	fi

tarball: 
	@if test -d taskjuggler ; then \
	  rm -f $$doctarball ; \
	  doctarball="tjmanual-$(VERSION).tar.gz" ; \
	  cp -r taskjuggler manual ; \
	  chmod 755 manual ; \
	  tar czf $$doctarball manual ; \
	  rm -r manual ; \
	  echo $$doctarball " created" ; \
	fi

all-local: index.cache.bz2 taskjuggler/book1.html taskjuggler.ps taskjuggler.pdf

check-local:
	xmllint ${SGMLSOURCES}

clean-local:
	rm -rf taskjuggler index.cache.bz2 index.docbook \
	taskjuggler.out taskjuggler.pdf

