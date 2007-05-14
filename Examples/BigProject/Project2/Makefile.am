TASKJUGGLER=../../../taskjuggler/taskjuggler
TJX2GANTT=../../../tjx2gantt/tjx2gantt
TARGETNAME=Project2

EXTRA_DIST = *.html *.tjp
CLEANFILES = *.html

docprefix = @PACKAGES_DIR@Examples/BigProject/$(TARGETNAME)

install-data-local:
	$(mkinstalldirs) -m 755 $(DESTDIR)$(docprefix)
	$(INSTALL_DATA) $(srcdir)/*.html $(DESTDIR)$(docprefix)
	$(INSTALL_DATA) $(srcdir)/*.tjp $(DESTDIR)$(docprefix)

uninstall-local:
	-rm -f $(DESTDIR)$(docprefix)

$(TARGETNAME).html : $(TARGETNAME).tjp
	$(TASKJUGGLER) --debug 1 $(TARGETNAME).tjp

all-local: $(TARGETNAME).html

