## Makefile.am for ktjview2

SUBDIRS = kdgantt pixmaps pics po

# this is the program that gets installed. its name is used for all
# of the other Makefile.am variables
bin_PROGRAMS = ktjview2

# set the include path for X, qt and KDE
INCLUDES = -I$(top_srcdir)/ktjview2/kdgantt -I$(top_srcdir)/taskjuggler $(all_includes)

# the library search path. 
ktjview2_LDFLAGS = $(KDE_RPATH) $(all_libraries)

# the libraries to link against.
ktjview2_LDADD = $(top_builddir)/ktjview2/kdgantt/libkdgantt.la $(top_builddir)/taskjuggler/libtaskjuggler.la \
	$(LIB_KIO) $(LIB_KDEPRINT) -lktexteditor

# which sources should be compiled for ktjview2
ktjview2_SOURCES = main.cpp ktjview2.cpp ktjview2view.cpp TaskItem.cpp ResourceItem.cpp \
	ktjview2iface.skel timedialog.cpp settings.kcfgc gantt.ui \
	filterWidget.ui filterDialog.cpp filter.cpp klistviewsearchline.cpp selectDialog.cpp quickSearchWidget.cpp \
	resUsageView.cpp resUsageCfg.ui ruFindDialog.cpp editorView.cpp ktjUtils.cpp \
	ktjReportView.cpp ktjTaskReport.cpp ktjReport.cpp reportSelectionWidget.ui ktjReportDialog.cpp ktjResourceReport.cpp

kde_kcfg_DATA = ktjview2.kcfg

# let moc handle all of the meta source files
METASOURCES = AUTO

messages:
	extractrc *.ui *.kcfg *.rc > rc.cpp
	xgettext -ki18n *.cpp kdgantt/*.cpp -o $(top_srcdir)/ktjview2/po/ktjview2.pot
	-rm -f rc.cpp

KDE_ICON = ktjview2

xdg_apps_DATA = ktjview2.desktop

# register the mimetype
tjmimedir = $(kde_mimedir)/application
tjmime_DATA = x-tjxml.desktop x-tjp.desktop

# Kate HL file
syntaxkatedir = $(kde_datadir)/katepart/syntax
syntaxkate_DATA = taskjuggler.xml

# this is where the XML-GUI resource file goes
rcdir = $(kde_datadir)/ktjview2
rc_DATA = ktjview2ui.rc
