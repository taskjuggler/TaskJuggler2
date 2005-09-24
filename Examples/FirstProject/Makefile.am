TASKJUGGLER=../../taskjuggler/taskjuggler
TARGETNAME=AccountingSoftware

EXTRA_DIST = *.html *.tjp *.tjx
CLEANFILES = *.html *.tjx

docprefix = $(prefix)/share/doc/packages/taskjuggler/Examples/FirstProject

install-data-local:
	$(mkinstalldirs) -m 755 $(DESTDIR)$(docprefix) 
	$(INSTALL_DATA) $(srcdir)/*.html $(DESTDIR)$(docprefix)
	$(INSTALL_DATA) $(srcdir)/*.tjp $(DESTDIR)$(docprefix)
	$(INSTALL_DATA) $(srcdir)/*.tjx $(DESTDIR)$(docprefix)

uninstall-local:
	-rm -f $(DESTDIR)$(docprefix)

$(TARGETNAME).tjx : $(TARGETNAME).tjp $(TASKJUGGLER)
	$(TASKJUGGLER) --debug 1 $(TARGETNAME).tjp

all-local: $(TARGETNAME).tjx

