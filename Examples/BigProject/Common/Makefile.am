docprefix = $(DESTDIR)$(prefix)/share/doc/packages/taskjuggler/Examples/BigProject/Common

install-data-local:
	$(mkinstalldirs) $(docprefix) 
	$(INSTALL_DATA) $(srcdir)/Resources.tjsp $(docprefix)

uninstall-local:
	-rm -f $(docprefix)
 
