TASKJUGGLER=../../taskjuggler/taskjuggler
TJX2GANTT=../../tjx2gantt/tjx2gantt
TARGETNAME=ShiftSchedule

EXTRA_DIST = *.html *.tjp *.tjx *.eps
CLEANFILES = *.html *.tjx *.eps

docprefix = $(prefix)/share/doc/packages/taskjuggler/Examples/$(TARGETNAME)

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(docprefix)
	$(INSTALL_DATA) $(srcdir)/*.html $(DESTDIR)$(docprefix)
	$(INSTALL_DATA) $(srcdir)/*.tjp $(DESTDIR)$(docprefix)

uninstall-local:
	-rm -f $(docprefix)

$(TARGETNAME).tjx : $(TARGETNAME).tjp $(TASKJUGGLER)
	$(TASKJUGGLER) --debug 1 $(TARGETNAME).tjp

all: $(TARGETNAME).tjx

