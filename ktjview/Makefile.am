# this has all of the subdirectories that make will recurse into.  if
# there are none, comment this out
#SUBDIRS = .

ICAL_INCLUDES = -I/space/kde3/kdepim

# set the include path for X, qt and KDE
INCLUDES = $(all_includes) $(ICAL_INCLUDES) -I../taskjuggler

# these are the headers for your project
noinst_HEADERS = ktjview.h ktjview_part.h ktvtasktable.h ktvtasktableitem.h

# let automoc handle all of the meta source files (moc)
METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/ktjview.pot

KDE_ICON = ktjview

# this Makefile creates both a KPart application and a KPart
#########################################################################
# APPLICATION SECTION
#########################################################################
# this is the program that gets installed.  it's name is used for all
# of the other Makefile.am variables
bin_PROGRAMS = ktjview

# the application source, library search path, and link libraries
ktjview_SOURCES = main.cpp ktjview.cpp
ktjview_LDFLAGS = $(KDE_RPATH) $(all_libraries) 
ktjview_LDADD   = $(LIB_KPARTS)

# this is where the desktop file will go 
# shelldesktopdir   = $(kde_appsdir)/Utilities
# shelldesktop_DATA = ktjview.desktop

# this is where the shell's XML-GUI resource file goes
shellrcdir   = $(kde_datadir)/ktjview
shellrc_DATA = ktjview_shell.rc

#########################################################################
# KPART SECTION
#########################################################################
kde_module_LTLIBRARIES = libktjviewpart.la

# the Part's source, library search path, and link libraries
libktjviewpart_la_SOURCES = ktjview_part.cpp ktvtasktable.cpp ktvtasktableitem.cpp 
libktjviewpart_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries) 
libktjviewpart_la_LIBADD  = $(LIB_KPARTS) $(LIB_KFILE)  ../taskjuggler/libtaskjuggler.la

# this is where the desktop file will go 
partdesktopdir   = $(kde_servicesdir)
partdesktop_DATA = ktjview.desktop

# this is where the part's XML-GUI resource file goes
partrcdir   = $(kde_datadir)/ktjviewpart
partrc_DATA = ktjview_part.rc
