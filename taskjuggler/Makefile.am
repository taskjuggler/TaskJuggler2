lib_LTLIBRARIES = libtaskjuggler.la

libtaskjuggler_la_SOURCES = Utility.cpp RealFormat.cpp \
Tokenizer.cpp ExpressionFunctionTable.cpp ExpressionParser.cpp \
ParserTreeContext.cpp ParserNode.cpp ParserElement.cpp \
TjMessageHandler.cpp \
Operation.cpp ExpressionTree.cpp ExpressionTreeFunction.cpp \
ShiftSelectionList.cpp ShiftSelection.cpp \
BookingList.cpp TransactionList.cpp \
TaskDependency.cpp TaskScenario.cpp VacationList.cpp \
CoreAttributes.cpp CoreAttributesList.cpp \
ScenarioList.cpp Scenario.cpp \
TaskList.cpp ResourceList.cpp AccountList.cpp ShiftList.cpp \
Task.cpp Resource.cpp Account.cpp Shift.cpp \
Allocation.cpp \
Journal.cpp \
MacroTable.cpp FileInfo.cpp FileToken.cpp ProjectFile.cpp \
XMLFile.cpp \
ReportElementBase.cpp ReportElement.cpp Report.cpp ReportXML.cpp \
QtReportElement.cpp \
QtTaskReportElement.cpp \
QtResourceReport.cpp QtResourceReportElement.cpp \
HTMLPrimitives.cpp TableColumnFormat.cpp TableColumnInfo.cpp \
HTMLReport.cpp HTMLReportElement.cpp \
HTMLTaskReportElement.cpp \
HTMLResourceReportElement.cpp \
HTMLWeeklyCalendarElement.cpp \
HTMLMonthlyCalendar.cpp HTMLMonthlyCalendarElement.cpp \
HTMLAccountReportElement.cpp \
HTMLStatusReport.cpp \
CSVPrimitives.cpp CSVReportElement.cpp \
CSVTaskReport.cpp CSVTaskReportElement.cpp \
CSVResourceReport.cpp CSVResourceReportElement.cpp \
CSVAccountReportElement.cpp \
ExportReport.cpp \
XMLReport.cpp \
SVGReport.cpp SVGTimeTimeReport.cpp SVGGanttTaskReport.cpp \
Project.cpp ICalReport.cpp

libtaskjuggler_la_LDFLAGS = $(all_libraries) $(QT_LDFLAGS) -version-info 3:3:0 -no-undefined
libtaskjuggler_la_LIBADD = $(LIB_QT) $(KDE_LIBS) $(LIBKCAL)
libtaskjuggler_la_METASOURCES = AUTO

bin_PROGRAMS = taskjuggler
taskjuggler_SOURCES = main.cpp

taskjuggler_LDADD = libtaskjuggler.la

# set the include path for X, qt and KDE
INCLUDES = $(ICAL_INCLUDES) $(all_includes)

KDE_OPTIONS = qtonly

messages:
	$(XGETTEXT) -ki18n -kI18N_NOOP --output=../i18n/taskjuggler/taskjuggler.pot *.cpp

#include ../admin/Doxyfile.am

