EXTRA_DIST = taskjuggler/*.html

docprefix = $(prefix)/share/doc/packages/taskjuggler/manual

SGMLSOURCES = taskjuggler.sgml changelog.sgml firstproject.sgml reference.sgml example.sgml
CLEANFILES = *.log *.aux *.dvi *.ps *.tex taskjuggler.sgml example.sgml

install-data-local:
	$(mkinstalldirs) -m 755 $(DESTDIR)$(docprefix)
	@if test "${HTMLMANUAL}" != "no" ; then \
	  $(INSTALL_DATA) $(srcdir)/taskjuggler/*.html $(DESTDIR)$(docprefix);\
	fi
	@if test ${PSMANUAL} != "no" ; then \
	  $(INSTALL_DATA) $(srcdir)/taskjuggler.ps $(DESTDIR)$(docprefix);\
	fi

uninstall-local:
	-rm -f $(DESTDIR)$(docprefix)

example.sgml : ../../Examples/FirstProject/AccountingSoftware.tjp
	rm -f example.sgml
	echo "<para><screen>" > example.sgml
	sed 's/&/\&amp;/g; s/</\&lt;/g; s/>/\&gt;/g' ../../Examples/FirstProject/AccountingSoftware.tjp | expand -t 2 >> example.sgml
	echo "</screen></para>" >> example.sgml

taskjuggler/book1.html: $(SGMLSOURCES)
	@if test "${HTMLMANUAL}" != "no" ; then \
	  db2html taskjuggler.sgml ;\
	  cp index.html taskjuggler/index.html ; \
	  rm -rf taskjuggler.junk ;\
	fi

taskjuggler.ps: $(SGMLSOURCES)
	@if test ${PSMANUAL} != "no" ; then \
	  db2ps taskjuggler.sgml ;\
	  rm -rf db2html.* taskjuggler.aux taskjuggler.log \
	    CATALOG.local *.dsl ; \
	fi

tarball: 
	@if test -d taskjuggler ; then \
	  rm -f $$doctarball ; \
	  doctarball="tjmanual-$(VERSION).tar.gz" ; \
	  cp -a taskjuggler manual ; \
	  tar czf $$doctarball manual ; \
	  rm -r manual ; \
	  echo $$doctarball " created" ; \
	fi

all: taskjuggler/book1.html taskjuggler.ps

check:
	xmllint ${SGMLSOURCES}

