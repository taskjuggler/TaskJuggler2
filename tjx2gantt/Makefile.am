docprefix = $(prefix)/share/doc/packages/taskjuggler/tjx2gantt

install-data-local:
	$(mkinstalldirs) -m 755 $(docprefix)
	$(INSTALL_DATA) $(srcdir)/README $(srcdir)/BUGS $(srcdir)/TODO \
		$(srcdir)/testProject.tjp $(docprefix)/

bin_SCRIPTS = tjx2gantt

