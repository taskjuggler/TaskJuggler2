/*
 * ReportElement.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */
#ifndef _ReportElement_h_
#define _ReportElement_h_

#include <time.h>

#include <qvaluelist.h>

#include "Project.h"

class Report;
class Scenario;

class ReportElement
{
public:
    ReportElement(const Project* p, Report* r, const QString& df, int dl);
    virtual ~ReportElement();

    void addScenario(int sc) { scenarios.append(sc); }
    void clearScenarios() { scenarios.clear(); }

    const Project* getProject() const { return project; }

    void addReportColumn(const QString& c) { columns.append(c); }
    const QString& columnsAt(uint idx) { return columns[idx]; }
    void clearColumns() { columns.clear(); }

    void setStart(time_t s) { start = s; }
    time_t getStart() const { return start; }
    
    void setEnd(time_t e) { end = e; }
    time_t getEnd() const { return end; }

    bool isHidden(const CoreAttributes* c, ExpressionTree* et) const;
    bool isRolledUp(const CoreAttributes* c, ExpressionTree* et) const;

    void setHideTask(ExpressionTree* et);
    void setHideResource(ExpressionTree* et);
    void setHideAccount(ExpressionTree* et);
    void setRollUpTask(ExpressionTree* et);
    void setRollUpResource(ExpressionTree* et);
    void setRollUpAccount(ExpressionTree* et);

    bool setTaskSorting(int sc, int level);
    bool setResourceSorting(int sc, int level);
    bool setAccountSorting(int sc, int level);

    void setTaskRoot(const QString& root) { taskRoot = root; }
    const QString& getTaskRoot() const { return taskRoot; }

    enum LoadUnit { minutes, hours, days, weeks, months, years, shortAuto,
        longAuto };

    bool setLoadUnit(const QString& u);

    void setTimeFormat(const QString& tf) { timeFormat = tf; }
    void setShortTimeFormat(const QString& tf) { shortTimeFormat = tf; }

    ReportElement() { }

    void filterTaskList(TaskList& filteredList, const Resource* r,
                        ExpressionTree* hideExp, ExpressionTree* rollUpExp)
        const;
    void sortTaskList(TaskList& filteredList);

    void filterResourceList(ResourceList& filteredList, const Task* t,
                            ExpressionTree* hideExp, ExpressionTree* rollUpExp)
        const;
    void sortResourceList(ResourceList& filteredList);

    void filterAccountList(AccountList& filteredList, AccountType at,
                           ExpressionTree* hideExp, ExpressionTree*
                           rollUpExp) const;
    void sortAccountList(AccountList& filteredList);

    QString scaledLoad(double t) const;
protected:
    QTextStream& s() { return report->stream(); }
    void errorMessage(const char* msg, ... );

    /**
     * This utility function removes the path that matches the taskRoot
     * variable from the passed taskId.
     */
    QString stripTaskRoot(QString taskId) const;
    
    const Project* project;
    Report* report;
    QValueList<int> scenarios;
    QStringList columns;
    time_t start;
    time_t end;

    QString timeFormat;
    QString shortTimeFormat;
    
    /* We store the location of the report definition in case we need it
     * for error reporting. */
    QString defFileName;
    int defFileLine;
    
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
     * taskRoot specifies this task. If set it always ends with a '.'. */
    QString taskRoot;
    
    LoadUnit loadUnit;

    /* The maximum depth of the tree that we have to report in tree-sorting
     * mode. */
    uint maxDepthTaskList;
    uint maxDepthResourceList;
    uint maxDepthAccountList;
    
    bool showPIDs;
} ;

#endif

