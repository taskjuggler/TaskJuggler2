## Makefile.am for ktjview2

# this is the program that gets installed.  it's name is used for all
# of the other Makefile.am variables
SUBDIRS = kdgantt pixmaps

bin_PROGRAMS = ktjview2 ktjview2_client

# set the include path for X, qt and KDE
INCLUDES = -I$(top_srcdir)/ktjview2/kdgantt -I$(top_srcdir)/taskjuggler $(all_includes)

# the library search path. 
ktjview2_LDFLAGS = $(KDE_RPATH) $(all_libraries)

# the libraries to link against.
ktjview2_LDADD = $(top_builddir)/ktjview2/kdgantt/libkdgantt.la $(top_builddir)/taskjuggler/libtaskjuggler.la $(LIB_KIO) $(LIB_KDEPRINT)

# which sources should be compiled for ktjview2
ktjview2_SOURCES = main.cpp ktjview2.cpp ktjview2view.cpp \
	ktjview2iface.skel koKoolBar.cc timedialog.cpp \
	settings.kcfgc gantt.ui

kde_kcfg_DATA = ktjview2.kcfg

# these are the headers for your project
noinst_HEADERS = ktjview2.h ktjview2view.h koKoolBar.h timedialog.h

# client stuff
ktjview2_client_LDFLAGS = $(KDE_RPATH) $(all_libraries)
ktjview2_client_LDADD   = $(LIB_KDECORE)
ktjview2_client_SOURCES = ktjview2_client.cpp

# let automoc handle all of the meta source files (moc)
METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp *.cc kdgantt/*.cpp -o $(podir)/ktjview2.pot

KDE_ICON = ktjview2

xdg_apps_DATA = ktjview2.desktop

# this is where the XML-GUI resource file goes
rcdir = $(kde_datadir)/ktjview2
rc_DATA = ktjview2ui.rc
