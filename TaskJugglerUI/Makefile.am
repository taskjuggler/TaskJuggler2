## Makefile.am for taskjuggler

SUBDIRS=pics Templates

# this is the program that gets installed.  it's name is used for all
# of the other Makefile.am variables
bin_PROGRAMS = TaskJugglerUI

# set the include path for X, qt and KDE
INCLUDES = -I$(top_srcdir)/taskjuggler $(all_includes)

# the library search path. 
LIBADD=-L../taskjuggler/libtaskjuggler.la
TaskJugglerUI_LDFLAGS = $(KDE_RPATH) $(all_libraries)

# the libraries to link against.
TaskJugglerUI_LDADD = -lktexteditor $(top_builddir)/taskjuggler/libtaskjuggler.la $(LIB_KIO) $(LIB_KDEPRINT) $(LIB_KHTML)

# which sources should be compiled for taskjuggler
TaskJugglerUI_SOURCES = main.cpp taskjuggler.cpp taskjugglerview.cpp pref.cpp \
	TjObjPosTable.cpp TjGanttZoomStep.cpp TjGanttChart.cpp \
	TjLineAccounter.cpp \
	TjPrintReport.cpp TjPrintTaskReport.cpp TjPrintResourceReport.cpp \
	TjReportRow.cpp TjReportCell.cpp \
	FileManager.cpp ManagedFileInfo.cpp \
	ReportManager.cpp ManagedReportInfo.cpp \
	TjReportBase.cpp TjSummaryReport.cpp \
	TjHTMLReport.cpp \
	TjReport.cpp TjTaskReport.cpp TjResourceReport.cpp \
	taskjuggleriface.skel \
	MainWidget.ui RichTextDisplay.ui \
	TemplateSelector.ui TjDatePicker.ui

# these are the headers for your project
noinst_HEADERS = taskjuggler.h taskjugglerview.h pref.h

# let automoc handle all of the meta source files (moc)
METASOURCES = AUTO

messages: main.cpp
	extractrc `find . -name "*.ui"` > rc.cpp
	$(XGETTEXT) -ki18n -kI18N_NOOP --output=../i18n/TaskJugglerUI/TaskJugglerUI.pot *.cpp

KDE_ICON = taskjuggler taskjuggler_tjp taskjuggler_tji taskjuggler_tjx

# register the application mimetype
xdg_apps_DATA = taskjuggler.desktop

# register the mimetype
tjmimedir = $(kde_mimedir)/application
tjmime_DATA = x-tjp.desktop x-tji.desktop x-tjx.desktop

# this is where the XML-GUI resource file goes
rcdir = $(kde_datadir)/taskjuggler
rc_DATA = taskjugglerui.rc

miscdir = $(kde_datadir)/taskjuggler
misc_DATA = tips katefiletyperc

kde_conf_DATA = taskjugglerrc
