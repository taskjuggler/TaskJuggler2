docprefix = $(prefix)/share/doc/packages/taskjuggler/Examples/BigProject/Common

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(docprefix) 
	$(INSTALL_DATA) $(srcdir)/Resources.tjsp $(DESTDIR)$(docprefix)

uninstall-local:
	-rm -f $(docprefix)
 
