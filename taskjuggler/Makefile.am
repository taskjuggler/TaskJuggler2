lib_LTLIBRARIES = libtaskjuggler.la

libtaskjuggler_la_SOURCES = Utility.cpp RealFormat.cpp \
Tokenizer.cpp ExpressionFunctionTable.cpp ExpressionParser.cpp \
ParserTreeContext.cpp ParserNode.cpp ParserElement.cpp \
TjMessageHandler.cpp \
DecisionNode.cpp OptimizerRun.cpp Optimizer.cpp \
Operation.cpp ExpressionTree.cpp ExpressionTreeFunction.cpp \
ShiftSelectionList.cpp ShiftSelection.cpp \
BookingList.cpp TransactionList.cpp \
TaskScenario.cpp VacationList.cpp \
CoreAttributes.cpp CoreAttributesList.cpp \
CoreAttributesTreeIterator.cpp \
ScenarioList.cpp Scenario.cpp \
TaskList.cpp ResourceList.cpp AccountList.cpp ShiftList.cpp \
Task.cpp Resource.cpp Account.cpp Shift.cpp \
Allocation.cpp \
MacroTable.cpp FileInfo.cpp ProjectFile.cpp \
XMLFile.cpp \
ReportElement.cpp Report.cpp ReportXML.cpp \
QtReportElement.cpp QtReport.cpp QtTaskReport.cpp \
QtTaskReportElement.cpp \
HTMLPrimitives.cpp TableColumnFormat.cpp TableColumnInfo.cpp \
HTMLReport.cpp HTMLReportElement.cpp \
HTMLTaskReport.cpp HTMLTaskReportElement.cpp \
HTMLResourceReport.cpp HTMLResourceReportElement.cpp \
HTMLWeeklyCalendar.cpp HTMLWeeklyCalendarElement.cpp \
HTMLAccountReport.cpp HTMLAccountReportElement.cpp \
HTMLStatusReport.cpp \
CSVPrimitives.cpp CSVReport.cpp CSVReportElement.cpp \
CSVTaskReport.cpp CSVTaskReportElement.cpp \
CSVResourceReport.cpp CSVResourceReportElement.cpp \
CSVAccountReport.cpp CSVAccountReportElement.cpp \
ExportReport.cpp \
XMLReport.cpp \
Project.cpp ReportICal.cpp kotrus.cpp

libtaskjuggler_la_LDFLAGS = $(all_libraries) $(QT_LDFLAGS) -no-undefined
libtaskjuggler_la_LIBADD = $(LIB_QT) $(KDE_LIBS)
libtaskjuggler_la_METASOURCES = AUTO

bin_PROGRAMS = taskjuggler
taskjuggler_SOURCES = main.cpp 

taskjuggler_LDADD = libtaskjuggler.la

# set the include path for X, qt and KDE
INCLUDES = $(ICAL_INCLUDES) $(all_includes)

KDE_OPTIONS = qtonly

messages: 
	$(XGETTEXT) -ki18n -kI18N_NOOP -d taskjuggler `find . -name "*.cpp"`

#include ../admin/Doxyfile.am

