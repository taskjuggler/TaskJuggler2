TASKJUGGLER=../../../taskjuggler/taskjuggler
TJX2GANTT=../../../tjx2gantt/tjx2gantt
TARGETNAME=Project1

EXTRA_DIST = *.html *.tjp *.tjsp
CLEANFILES = *.html *.tjsp

docprefix = $(prefix)/share/doc/packages/taskjuggler/Examples/BigProject/$(TARGETNAME)

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(docprefix)
	$(INSTALL_DATA) $(srcdir)/*.html $(DESTDIR)$(docprefix)
	$(INSTALL_DATA) $(srcdir)/*.tjp $(DESTDIR)$(docprefix)
	$(INSTALL_DATA) $(srcdir)/*.tjsp $(DESTDIR)$(docprefix)

uninstall-local:
	-rm -f $(DESTDIR)$(docprefix)

$(TARGETNAME).tjsp : $(TARGETNAME).tjp
	$(TASKJUGGLER) --debug 1 $(TARGETNAME).tjp

all: $(TARGETNAME).tjsp

