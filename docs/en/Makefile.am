EXTRA_DIST = taskjuggler/*.html

docprefix = $(DESTDIR)$(prefix)/share/doc/packages/taskjuggler/manual

SGMLSOURCES = taskjuggler.sgml changelog.sgml firstproject.sgml reference.sgml example.sgml
CLEANFILES = *.log *.aux *.dvi *.ps *.tex taskjuggler.sgml example.sgml

install-data-local:
	$(mkinstalldirs) $(docprefix)
	$(INSTALL_DATA) $(srcdir)/taskjuggler/*.html $(docprefix)
	$(INSTALL_DATA) $(srcdir)/taskjuggler.ps $(docprefix)

uninstall-local:
	-rm -f $(docprefix)

example.sgml : ../../Examples/FirstProject/AccountingSoftware.tjp
	rm -f example.sgml
	echo "<para><screen>" > example.sgml
	sed 's/&/\&amp;/g; s/</\&lt;/g; s/>/\&gt;/g' ../../Examples/FirstProject/AccountingSoftware.tjp | expand -t 2 >> example.sgml
	echo "</screen></para>" >> example.sgml

taskjuggler/book1.html: $(SGMLSOURCES)
	@if test "${HTMLMANUAL}" != "no" ; then \ 
	  db2html taskjuggler.sgml ;\
	  rm -rf taskjuggler.junk ;\
	fi

taskjuggler.ps: $(SGMLSOURCES)
	@if test ${PSMANUAL} != "no" ; then \
	  db2ps taskjuggler.sgml ;\
	fi

all: taskjuggler/book1.html taskjuggler.ps

check:
	xmllint ${SGMLSOURCES}

