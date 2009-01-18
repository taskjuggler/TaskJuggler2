/*
 * Report.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _Report_h_
#define _Report_h_

#include <qvaluelist.h>
#include <qfile.h>
#include <qtextstream.h>

#include "CoreAttributesList.h"
#include "RealFormat.h"
#include "Interval.h"

class Project;
class CoreAttributes;
class Task;
class Resource;
class TaskList;
class ResourceList;
class AccountList;
class ExpressionTree;
class ReportElement;

/**
 * @short The base class for all report generating classes.
 * @author Chris Schlaeger <cs@kde.org>
 */
class Report
{
public:
    Report(Project* p, const QString& f, const QString& df, int dl);
    virtual ~Report();

    virtual const char* getType() const { return "Report"; }

    bool open();
    bool close();

    QTextStream& stream() { return s; }

    void puts(const QString& str)
    {
        s.writeRawBytes(str.data(), str.length());
    }
    const Project* getProject() const { return project; }
    const QString& getFileName() const { return fileName; }
    QString getFullFileName() const;

    const QString& getDefinitionFile() const { return defFileName; }
    uint getDefinitionLine() const { return defFileLine; }

    void addScenario(int sc) { scenarios.append(sc); }
    void clearScenarios() { scenarios.clear(); }
    size_t getScenarioCount() const { return scenarios.count(); }
    int getScenario(int sc) const { return scenarios[sc]; }

    void setWeekStartsMonday(bool wsm) { weekStartsMonday = wsm; }
    bool getWeekStartsMonday() const { return weekStartsMonday; }

    void setShowPIDs(bool sp) { showPIDs = sp; }
    bool getShowPIDs() const { return showPIDs; }

    void addReportColumn(const QString& c) { columns.append(c); }
    const QString& columnsAt(uint idx) { return columns[idx]; }
    void clearColumns() { columns.clear(); }

    void setStart(time_t st) { start = st; }
    time_t getStart() const { return start; }

    void setEnd(time_t e) { end = e; }
    time_t getEnd() const { return end; }

    void setPeriod(const Interval& iv)
    {
        start = iv.getStart();
        end = iv.getEnd();
    }

    void setHeadline(const QString& hl) { headline = hl; }
    const QString& getHeadline() const { return headline; }

    void setCaption(const QString& c) { caption = c; }
    const QString& getCaption() const { return caption; }

    bool isHidden(const CoreAttributes* c, ExpressionTree* et) const;
    bool isRolledUp(const CoreAttributes* c, ExpressionTree* et) const;

    void setHideTask(ExpressionTree* et);
    ExpressionTree* getHideTask() const { return hideTask; }

    void setHideResource(ExpressionTree* et);
    ExpressionTree* getHideResource() const { return hideResource; }

    void setHideAccount(ExpressionTree* et);
    ExpressionTree* getHideAccount() const { return hideAccount; }

    void setRollUpTask(ExpressionTree* et);
    ExpressionTree* getRollUpTask() const { return rollUpTask; }

    void setRollUpResource(ExpressionTree* et);
    ExpressionTree* getRollUpResource() const { return rollUpResource; }

    void setRollUpAccount(ExpressionTree* et);
    ExpressionTree* getRollUpAccount() const { return rollUpAccount; }

    bool setTaskSorting(int sc, int level);
    int getTaskSorting(int level) const { return taskSortCriteria[level]; }

    bool setResourceSorting(int sc, int level);
    int getResourceSorting(int level) const
    {
        return resourceSortCriteria[level];
    }

    bool setAccountSorting(int sc, int level);
    int getAccountSorting(int level) const
    {
        return accountSortCriteria[level];
    }

    void setTaskRoot(const QString& root) { taskRoot = root; }
    const QString& getTaskRoot() const { return taskRoot; }

    bool setLoadUnit(const QString& u);
    LoadUnit getLoadUnit() const { return loadUnit; }

    void setTimeFormat(const QString& tf) { timeFormat = tf; }
    const QString& getTimeFormat() const { return timeFormat; }

    void setShortTimeFormat(const QString& tf) { shortTimeFormat = tf; }
    const QString& getShortTimeFormat() const { return shortTimeFormat; }

    void setNumberFormat(const RealFormat& rf) { numberFormat = rf; }
    const RealFormat& getNumberFormat() const { return numberFormat; }

    void setCurrencyFormat(const RealFormat& rf) { currencyFormat = rf; }
    const RealFormat& getCurrencyFormat() const { return currencyFormat; }

    bool filterTaskList(TaskList& filteredList, const Resource* r,
                        ExpressionTree* hideExp, ExpressionTree* rollUpExp)
        const;
    void sortTaskList(TaskList& filteredList);

    bool filterResourceList(ResourceList& filteredList, const Task* t,
                            ExpressionTree* hideExp, ExpressionTree* rollUpExp)
        const;
    void sortResourceList(ResourceList& filteredList);

    bool filterAccountList(AccountList& filteredList, AccountType at,
                           ExpressionTree* hideExp, ExpressionTree*
                           rollUpExp) const;
    void sortAccountList(AccountList& filteredList);

    void setTimeStamp(bool t)
    {
        timeStamp = t;
    }
    bool getTimeStamp() const
    {
        return timeStamp;
    }

    virtual bool generate() = 0;

protected:
    void errorMessage(const QString& msg);

    /**
     * This utility function removes the path that matches the taskRoot
     * variable from the passed taskId.
     */
    QString stripTaskRoot(QString taskId) const;

    Project* project;
    QString fileName;

    /* We store the location of the report definition in case we need it
     * for error reporting. */
    QString defFileName;
    int defFileLine;
    QFile f;
    QTextStream s;

    QValueList<int> scenarios;

    bool weekStartsMonday;

    QString headline;
    QString caption;

    /* The maximum depth of the tree that we have to report in tree-sorting
     * mode. */
    uint maxDepthTaskList;
    uint maxDepthResourceList;
    uint maxDepthAccountList;

    /* The following variables store values that are not used by the Report
     * class or its derived classes directly if the class contains
     * ReportElements. They only contain the default values for the
     * ReportElements. */
    QStringList columns;
    time_t start;
    time_t end;
    QString timeFormat;
    QString shortTimeFormat;
    RealFormat numberFormat;
    RealFormat currencyFormat;

    int taskSortCriteria[CoreAttributesList::maxSortingLevel];
    int resourceSortCriteria[CoreAttributesList::maxSortingLevel];
    int accountSortCriteria[CoreAttributesList::maxSortingLevel];

    ExpressionTree* hideTask;
    ExpressionTree* hideResource;
    ExpressionTree* hideAccount;
    ExpressionTree* rollUpTask;
    ExpressionTree* rollUpResource;
    ExpressionTree* rollUpAccount;

    /* A report can be limited to the sub-tasks of a certain task. The
     * taskRoot specifies this task. If set it always ends with a '.' if it's
     * not empty. */
    QString taskRoot;

    LoadUnit loadUnit;

    bool showPIDs;

    bool timeStamp;
} ;

#endif
