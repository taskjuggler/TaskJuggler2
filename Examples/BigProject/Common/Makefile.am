docprefix = $(prefix)/share/doc/packages/taskjuggler/Examples/BigProject/Common

install-data-local:
	$(mkinstalldirs) -m 755 $(DESTDIR)$(docprefix) 
	$(INSTALL_DATA) $(srcdir)/Resources.tjsp $(DESTDIR)$(docprefix)

uninstall-local:
	-rm -f $(DESTDIR)$(docprefix)
 
