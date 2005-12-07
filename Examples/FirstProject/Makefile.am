TASKJUGGLER=../../taskjuggler/taskjuggler
TARGETNAME=AccountingSoftware

EXTRA_DIST = *.html *.tjp
CLEANFILES = *.html

docprefix = @PACKAGES_DIR@Examples/FirstProject

install-data-local:
	$(mkinstalldirs) -m 755 $(DESTDIR)$(docprefix) 
	$(INSTALL_DATA) $(srcdir)/*.html $(DESTDIR)$(docprefix)
	$(INSTALL_DATA) $(srcdir)/*.tjp $(DESTDIR)$(docprefix)

uninstall-local:
	-rm -f $(DESTDIR)$(docprefix)

Calendar.html : $(TARGETNAME).tjp $(TASKJUGGLER)
	$(TASKJUGGLER) --debug 1 $(TARGETNAME).tjp

all-local: Calendar.html

