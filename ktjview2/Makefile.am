## Makefile.am for ktjview2

# this is the program that gets installed.  it's name is used for all
# of the other Makefile.am variables
SUBDIRS = kdgantt pixmaps

bin_PROGRAMS = ktjview2

# set the include path for X, qt and KDE
INCLUDES = -I$(top_srcdir)/ktjview2/kdgantt -I$(top_srcdir)/taskjuggler $(all_includes)

# the library search path. 
ktjview2_LDFLAGS = $(KDE_RPATH) $(all_libraries)

# the libraries to link against.
ktjview2_LDADD = $(top_builddir)/ktjview2/kdgantt/libkdgantt.la $(top_builddir)/taskjuggler/libtaskjuggler.la $(LIB_KIO) $(LIB_KDEPRINT)

# which sources should be compiled for ktjview2
ktjview2_SOURCES = main.cpp ktjview2.cpp ktjview2view.cpp TaskItem.cpp ResourceItem.cpp \
	ktjview2iface.skel timedialog.cpp settings.kcfgc gantt.ui \
	filterWidget.ui filterDialog.cpp filter.cpp klistviewsearchline.cpp selectDialog.cpp quickSearchWidget.cpp \
	resUsageView.cpp

kde_kcfg_DATA = ktjview2.kcfg

# let automoc handle all of the meta source files (moc)
METASOURCES = AUTO

messages:
	extractrc *.ui *.kcfg *.rc > rc.cpp
	xgettext -ki18n *.cpp kdgantt/*.cpp -o $(top_srcdir)/ktjview2/po/ktjview2.pot
	-rm -f rc.cpp

KDE_ICON = ktjview2

xdg_apps_DATA = ktjview2.desktop

# this is where the XML-GUI resource file goes
rcdir = $(kde_datadir)/ktjview2
rc_DATA = ktjview2ui.rc
