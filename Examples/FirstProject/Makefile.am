TASKJUGGLER=../../taskjuggler/taskjuggler
TJX2GANTTPL=../../tjx2gantt/tjx2gantt
TARGETNAME=AccountingSoftware

EXTRA_DIST = *.html *.tjp *.tjx *.eps
CLEANFILES = *.html *.tjx *.eps

docprefix = $(DESTDIR)$(prefix)/share/doc/packages/taskjuggler/Examples/FirstProject

install-data-local:
	$(mkinstalldirs) $(docprefix) 
	$(INSTALL_DATA) $(srcdir)/*.html $(docprefix)
	$(INSTALL_DATA) $(srcdir)/*.tjp $(docprefix)
	$(INSTALL_DATA) $(srcdir)/*.tjx $(docprefix)
	$(INSTALL_DATA) $(srcdir)/*.eps $(docprefix)

uninstall-local:
	-rm -f $(docprefix)

$(TARGETNAME).tjx : $(TARGETNAME).tjp $(TASKJUGGLER)
	$(TASKJUGGLER) --debug 1 $(TARGETNAME).tjp

acso.eps : $(TARGETNAME).tjx
	@if test "$(TJX2GANTT)" != "no" ; then \
	  $(TJX2GANTTPL) $(TARGETNAME).tjx ;\
	fi

all: $(TARGETNAME).tjx acso.eps

