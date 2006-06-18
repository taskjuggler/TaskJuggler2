# Makefile.am for TaskJuggler
# Chris Schlaeger <cs@suse.de>
# Klaas Freitag <freitag@suse.de>
# Stephan Kulow <coolo@kde.org>
# $Id$
#
# This Makefile is only needed when you use source directly out of the
# subversion repository. It's not needed for the SVN snapshots.

COMPILE_FIRST = taskjuggler

EXTRA_DIST = admin AUTHORS COPYING ChangeLog INSTALL README TODO taskjuggler.lsm 

AUTOMAKE_OPTIONS = foreign 

include admin/deps.am

MAINTAINERCLEANFILES = subdirs configure.in acinclude.m4 configure.files 

package-messages:
	$(MAKE) -f admin/Makefile.common package-messages
	$(MAKE) -C po merge

dist-hook:
	cd $(top_distdir) && perl admin/am_edit -padmin
	cd $(top_distdir) && $(MAKE) -f admin/Makefile.common subdirs

SUBDIRS=$(TOPSUBDIRS)

