SUBDIRS = Common Project1 Project2 

docprefix = $(prefix)/share/doc/packages/taskjuggler/Examples/BigProject

install-data-local:
	$(mkinstalldirs) -m 755 $(DESTDIR)$(docprefix) 
	$(INSTALL_DATA) $(srcdir)/README $(DESTDIR)$(docprefix)

uninstall-local:
	-rm -f $(DESTDIR)$(docprefix)
 
