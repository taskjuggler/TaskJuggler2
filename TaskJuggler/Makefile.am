## Makefile.am for taskjuggler

# this is the program that gets installed.  it's name is used for all
# of the other Makefile.am variables
bin_PROGRAMS = TaskJuggler TaskJuggler_client

# set the include path for X, qt and KDE
INCLUDES = -I ../taskjuggler $(all_includes)

# the library search path. 
TaskJuggler_LDFLAGS = $(KDE_RPATH) $(all_libraries)

# the libraries to link against.
TaskJuggler_LDADD = -lktexteditor -ltaskjuggler $(LIB_KFILE) $(LIB_KDEPRINT)

# which sources should be compiled for taskjuggler
TaskJuggler_SOURCES = main.cpp taskjuggler.cpp taskjugglerview.cpp pref.cpp \
	FileManager.cpp ManagedFileInfo.cpp \
	ReportManager.cpp ManagedReportInfo.cpp \
	TjReport.cpp \
	taskjuggleriface.skel MainWidget.ui

# these are the headers for your project
noinst_HEADERS = taskjuggler.h taskjugglerview.h pref.h

# client stuff
TaskJuggler_client_LDFLAGS = $(KDE_RPATH) $(all_libraries)
TaskJuggler_client_LDADD   = $(LIB_KDECORE)
TaskJuggler_client_SOURCES = TaskJuggler_client.cpp

# let automoc handle all of the meta source files (moc)
METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/taskjuggler.pot

KDE_ICON = taskjuggler

# this is where the kdelnk file will go 
kdelnkdir = $(kde_appsdir)/Utilities
kdelnk_DATA = taskjuggler.desktop

# this is where the XML-GUI resource file goes
rcdir = $(kde_datadir)/taskjuggler
rc_DATA = taskjugglerui.rc
