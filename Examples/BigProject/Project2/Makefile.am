TASKJUGGLER=../../../taskjuggler/taskjuggler
TJX2GANTT=../../../tjx2gantt/tjx2gantt
TARGETNAME=Project2

EXTRA_DIST = *.html *.tjp
CLEANFILES = *.html

docprefix = $(DESTDIR)$(prefix)/share/doc/packages/taskjuggler/Examples/BigProject/$(TARGETNAME)

install-data-local:
	$(mkinstalldirs) $(docprefix)
	$(INSTALL_DATA) $(srcdir)/*.html $(docprefix)
	$(INSTALL_DATA) $(srcdir)/*.tjp $(docprefix)

uninstall-local:
	-rm -f $(docprefix)

$(TARGETNAME).tjsp : $(TARGETNAME).tjp
	$(TASKJUGGLER) --debug 1 $(TARGETNAME).tjp

all: $(TARGETNAME).tjsp

