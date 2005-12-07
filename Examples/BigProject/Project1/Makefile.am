TASKJUGGLER=../../../taskjuggler/taskjuggler
TJX2GANTT=../../../tjx2gantt/tjx2gantt
TARGETNAME=Project1

EXTRA_DIST = *.html *.tjp *.tji
CLEANFILES = *.html *.tji

docprefix = @PACKAGES_DIR@Examples/BigProject/$(TARGETNAME)

install-data-local:
	$(mkinstalldirs) -m 755 $(DESTDIR)$(docprefix)
	$(INSTALL_DATA) $(srcdir)/*.html $(DESTDIR)$(docprefix)
	$(INSTALL_DATA) $(srcdir)/*.tjp $(DESTDIR)$(docprefix)
	$(INSTALL_DATA) $(srcdir)/*.tji $(DESTDIR)$(docprefix)

uninstall-local:
	-rm -f $(DESTDIR)$(docprefix)

$(TARGETNAME).tji : $(TARGETNAME).tjp
	$(TASKJUGGLER) --debug 1 $(TARGETNAME).tjp

all-local: $(TARGETNAME).tji

