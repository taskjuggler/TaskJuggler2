## Makefile.am for taskjuggler

SUBDIRS=pics

# this is the program that gets installed.  it's name is used for all
# of the other Makefile.am variables
bin_PROGRAMS = TaskJuggler TaskJuggler_client

# set the include path for X, qt and KDE
INCLUDES = -I$(top_srcdir)/taskjuggler $(all_includes)

# the library search path. 
LIBADD=-L../taskjuggler/libtaskjuggler.la
TaskJuggler_LDFLAGS = $(KDE_RPATH) $(all_libraries)

# the libraries to link against.
TaskJuggler_LDADD = -lktexteditor $(top_builddir)/taskjuggler/libtaskjuggler.la $(LIB_KIO) $(LIB_KDEPRINT)

# which sources should be compiled for taskjuggler
TaskJuggler_SOURCES = main.cpp taskjuggler.cpp taskjugglerview.cpp pref.cpp \
	FileManager.cpp ManagedFileInfo.cpp \
	ReportManager.cpp ManagedReportInfo.cpp \
	TjReport.cpp TjTaskReport.cpp TjResourceReport.cpp \
	taskjuggleriface.skel \
	MainWidget.ui RichTextDisplay.ui FindDialog.ui

# these are the headers for your project
noinst_HEADERS = taskjuggler.h taskjugglerview.h pref.h

# client stuff
TaskJuggler_client_LDFLAGS = $(KDE_RPATH) $(all_libraries)
TaskJuggler_client_LDADD   = $(LIB_KDECORE)
TaskJuggler_client_SOURCES = TaskJuggler_client.cpp

# let automoc handle all of the meta source files (moc)
METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/TaskJuggler.pot

KDE_ICON = taskjuggler taskjuggler_tjp taskjuggler_tji taskjuggler_tjx

# register the application mimetype
xdg_apps_DATA = taskjuggler.desktop

# register the mimetype
tjmimedir = $(kde_mimedir)/application
tjmime_DATA = x-tjp.desktop x-tji.desktop x-tjx.desktop

# this is where the XML-GUI resource file goes
rcdir = $(kde_datadir)/taskjuggler
rc_DATA = taskjugglerui.rc ProjectTemplate.tjp IncludeTemplate.tji
