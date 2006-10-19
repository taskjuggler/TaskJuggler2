/*
 * ProjectFile.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _ProjectFile_h_
#define _ProjectFile_h_

#include <time.h>

#include <qptrlist.h>

#include "taskjuggler.h"
#include "Token.h"
#include "MacroTable.h"
#include "FileInfo.h"

class QBitArray;
class Project;
class Scenario;
class CoreAttributes;
class Task;
class Resource;
class Account;
class Shift;
class Booking;
class Interval;
class Operation;
class Report;
class HTMLReport;
class ReportElement;
class RealFormat;
class TableColumnInfo;
class UsageLimits;
class TaskDependency;
class JournalEntry;

/**
 * @short File Parser for project files.
 * @author Chris Schlaeger <cs@kde.org>
 */
class ProjectFile
{
public:
    /**
     * A ProjectFile cannot exist without a project. So the constructor needs
     * to know what Project object to fill, when it parses the project files.
     */
    ProjectFile(Project* p);
    ~ProjectFile() { }

    /**
     * The top-level project files needs to be opened before the parser can be
     * started.
     * @param file The file name of the file to start with.
     * @param parentPath The path of the file that included this file. This
     * feature is for internal use only. It's not part of the public API.
     * @param taskPrefix The ID prefix of the parent task. This is needed when
     * the tasks of the project file should be read as a sub-task of an
     * already existing task.
     * @param This flag must be true if a top-level file should be processed.
     * This is a file, that is not included by any other file.
     */
    bool open(const QString& file, const QString& parentPath,
              const QString& taskPrefix, bool masterfile = FALSE);
    /**
     * Close the just read input file.
     */
    bool close();

    /**
     * Calling the parse function will start the processing of the opened
     * project file. It will automatically read all include files as well. The
     * collected data is stored into the Project object.
     */
    bool parse();

    /*
     * The rest of the public methods are for use by FileInfo and are not part
     * of the library public interface.
     */

    TokenType nextToken(QString& token);
    void returnToken(TokenType t, const QString& buf)
    {
        if (!openFiles.isEmpty())
            openFiles.last()->returnToken(t, buf);
    }
    const QString& getFile()
    {
        if (openFiles.isEmpty())
            return QString::null;
        return openFiles.last()->getFile();
    }
    int getLine()
    {
        if (openFiles.isEmpty())
            return -1;
        return openFiles.last()->getLine();
    }

    bool moreFiles() { return !openFiles.isEmpty(); }

    const QString& getTaskPrefix();

    bool generateMakeDepList(const QString& fileName, bool append) const;

    void errorMessage(const char* msg, ...);

    MacroTable& getMacros() { return macros; }

private:
    ProjectFile() {};   // don't use

    bool readProject();
    bool readExtend();
    bool readScenario(Scenario* parent);
    bool readProjection(Scenario* scenario);
    bool readInclude();
    bool readCustomAttribute(CoreAttributes* property, const QString& id,
                             CustomAttributeType type);
    bool readTask(Task* parent);
    bool readTaskSupplement(QString prefix);
    bool readTaskBody(Task* task);
    int readTaskScenarioAttribute(const QString attribute, Task* t, int sc,
                                  bool enforce);
    bool readResource(Resource* parent);
    bool readResourceSupplement();
    bool readResourceBody(Resource* r);

    JournalEntry* readJournalEntry();
    bool readVacation(time_t& from, time_t& to, bool readName = FALSE,
                      QString* = 0);
    bool readAccount(Account* parent);
    bool readShift(Shift* parent);
    Shift* readShiftSelection(Interval& iv);
    bool readBooking(int sc, Resource* resource);
    bool readCredit(Account* a);
    bool readAllocate(Task* t);
    UsageLimits* readLimits();
    bool readInterval(Interval& iv, bool check = true);
    bool readTimeFrame(double& d, bool workingDays, bool allowZero = false);
    bool readDate(time_t& val, time_t correction, bool checkPrjInterval = true);
    bool readRealFormat(RealFormat* format);
    bool readReference(QString& ref, QString& label);
    bool readTimeValue(ulong& value);
    bool readPercent(double& value);
    bool readWeekDay(int& dayOfWeek);
    bool readDaysToShow(QBitArray& days);
    bool readWorkingHours(int& dayOfWeek, QPtrList<Interval>* l);
    bool readPriority(int& priority);
    bool readReport(const QString& reportType);
    bool readHTMLReport(const QString& reportType);
    bool readHTMLStatusReport();
    bool readCSVReport(const QString& reportType);
    bool readExportReport();
    bool readXMLReport();
    bool readReportElement(ReportElement* el);
    bool checkReportInterval(ReportElement* report);
    bool checkReportInterval(HTMLReport* report);
    bool readICalTaskReport();

    Operation* readLogicalExpression(int precedence = 0);
    Operation* readFunctionCall(const QString& name);
    int checkScenarioSorting(const QString token);
    bool readSorting(Report* report, int which);
    bool readSorting(ReportElement* el, int which);
    bool readSortingMode(int& sorting);
    TableColumnInfo* readColumn(uint maxScenarios, ReportElement* tab);
    bool readTaskDepOptions(TaskDependency* td);
    bool date2time(const QString& date, time_t& val);
    int hhmm2time(const QString& hhmm);

    QString masterFile;
    Project* proj;
    QPtrList<FileInfo> openFiles;
    QStringList includedFiles;
    MacroTable macros;
};

#endif
