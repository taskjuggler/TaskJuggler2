TASKJUGGLER=../../taskjuggler/taskjuggler
TJX2GANTTPL=../../tjx2gantt/tjx2gantt
TARGETNAME=AccountingSoftware

EXTRA_DIST = *.html *.tjp *.tjx *.eps
CLEANFILES = *.html *.tjx *.eps

docprefix = $(prefix)/share/doc/packages/taskjuggler/Examples/FirstProject

install-data-local:
	$(mkinstalldirs) -m 755 $(DESTDIR)$(docprefix) 
	$(INSTALL_DATA) $(srcdir)/*.html $(DESTDIR)$(docprefix)
	$(INSTALL_DATA) $(srcdir)/*.tjp $(DESTDIR)$(docprefix)
	$(INSTALL_DATA) $(srcdir)/*.tjx $(DESTDIR)$(docprefix)
	@if test -f acso.eps ; then \
	  $(INSTALL_DATA) $(srcdir)/*.eps $(DESTDIR)$(docprefix) ; \
	fi

uninstall-local:
	-rm -f $(DESTDIR)$(docprefix)

$(TARGETNAME).tjx : $(TARGETNAME).tjp $(TASKJUGGLER)
	$(TASKJUGGLER) --debug 1 $(TARGETNAME).tjp

acso.eps : $(TARGETNAME).tjx
	@if test "$(TJX2GANTT)" != "no" ; then \
	  $(TJX2GANTTPL) $(TARGETNAME).tjx ;\
	fi

all-local: $(TARGETNAME).tjx acso.eps

