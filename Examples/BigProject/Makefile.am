SUBDIRS = Common Project1 Project2 

docprefix = $(DESTDIR)$(prefix)/share/doc/packages/taskjuggler/Examples/BigProject

install-data-local:
	$(mkinstalldirs) $(docprefix) 
	$(INSTALL_DATA) $(srcdir)/README $(docprefix)

uninstall-local:
	-rm -f $(docprefix)
 
