bin_PROGRAMS = taskjuggler
taskjuggler_SOURCES = ExpressionTree.cpp \
Task.cpp ResourceList.cpp VacationList.cpp \
Account.cpp Utility.cpp MacroTable.cpp ProjectFile.cpp \
Report.cpp HTMLTaskReport.cpp HTMLResourceReport.cpp \
Project.cpp main.cpp kotrus.cpp ReportHtml.cpp ReportXML.cpp

SOURCES = main.cpp 
INTERFACES = 
TRANSLATIONS = 
KDE_OPTIONS = qtonly
taskjuggler_LDADD   =  $(LIB_QT) $(LIBSOCKET)
CXXFLAGS = -Wall

SUBDIRS = docs 

EXTRA_DIST = main.cpp ProjectFile.cpp



# set the include path for X, qt and KDE
INCLUDES= $(all_includes)

taskjuggler_METASOURCES = AUTO

# the library search path. 
taskjuggler_LDFLAGS = $(all_libraries) 

# them while "make clean", use CLEANFILES
DISTCLEANFILES = $(taskjuggler_METASOURCES)

# this is to enable Qt behavoir of am_edit
KDE_OPTIONS = qtonly



