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
#include <qptrlist.h>
#include <qtextstream.h>
#include <qdict.h>
#include <qmap.h>

#include "taskjuggler.h"
#include "CoreAttributesList.h"

class Project;
class CoreAttributes;
class ExpressionTree;
class Report;
class Scenario;
class TableColumn;
class Task;
class TaskList;
class Resource;
class ResourceList;
class Account;
class AccountList;
class TableColumnFormat;
class TableLineInfo;

/**
 * @short A class that forms the basic element of a report. 
 * @author Chris Schlaeger <cs@suse.de>
 */
class ReportElement
{
public:
    ReportElement(Report* r, const QString& df, int dl);
    virtual ~ReportElement();

    Report* getReport() const { return report; }

    void addScenario(int sc) { scenarios.append(sc); }
    void clearScenarios() { scenarios.clear(); }
    uint getScenarioCount() { return scenarios.count(); }

    void setHeadline(const QString& hl) { headline = hl; }
    void setCaption(const QString& c) { caption = c; }

    void addColumn(const TableColumn* c) { columns.append(c); }
    const TableColumn* columnsAt(uint idx) { return columns.at(idx); }
    void clearColumns() { columns.clear(); }

    void setStart(time_t s) { start = s; }
    time_t getStart() const { return start; }
    
    void setEnd(time_t e) { end = e; }
    time_t getEnd() const { return end; }

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
    bool setResourceSorting(int sc, int level);
    bool setAccountSorting(int sc, int level);

    void setTaskRoot(const QString& root) { taskRoot = root; }
    const QString& getTaskRoot() const { return taskRoot; }

    bool setLoadUnit(const QString& u);

    void setTimeFormat(const QString& tf) { timeFormat = tf; }
    void setShortTimeFormat(const QString& tf) { shortTimeFormat = tf; }

    void setShowPIDs(bool s) { showPIDs = s; }
    bool getShowPIDs() const { return showPIDs; }

    bool setUrl(const QString& key, const QString& url);
    const QString* getUrl(const QString& key) const;

    void setAccumulate(bool s) { accumulate = s; }

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

    virtual void genHeadDefault(TableColumnFormat*) = 0;
    virtual void genHeadCurrency(TableColumnFormat*) = 0;
    virtual void genHeadDaily1(TableColumnFormat*) = 0;
    virtual void genHeadDaily2(TableColumnFormat*) = 0;
    virtual void genHeadWeekly1(TableColumnFormat*) = 0;
    virtual void genHeadWeekly2(TableColumnFormat*) = 0;
    virtual void genHeadMonthly1(TableColumnFormat*) = 0;
    virtual void genHeadMonthly2(TableColumnFormat*) = 0;
    virtual void genHeadQuarterly1(TableColumnFormat*) = 0;
    virtual void genHeadQuarterly2(TableColumnFormat*) = 0;
    virtual void genHeadYear(TableColumnFormat*) = 0;

    virtual void genCellEmpty(TableLineInfo*) = 0;
    virtual void genCellSequenceNo(TableLineInfo*) = 0;
    virtual void genCellNo(TableLineInfo*) = 0;
    virtual void genCellIndex(TableLineInfo*) = 0;
    virtual void genCellId(TableLineInfo*) = 0;
    virtual void genCellName(TableLineInfo*) = 0;
    virtual void genCellStart(TableLineInfo*) = 0;
    virtual void genCellEnd(TableLineInfo*) = 0;
    virtual void genCellMinStart(TableLineInfo*) = 0;
    virtual void genCellMaxStart(TableLineInfo*) = 0;
    virtual void genCellMinEnd(TableLineInfo*) = 0;
    virtual void genCellMaxEnd(TableLineInfo*) = 0;
    virtual void genCellStartBuffer(TableLineInfo*) = 0;
    virtual void genCellEndBuffer(TableLineInfo*) = 0;
    virtual void genCellStartBufferEnd(TableLineInfo*) = 0;
    virtual void genCellEndBufferStart(TableLineInfo*) = 0;
    virtual void genCellDuration(TableLineInfo*) = 0;
    virtual void genCellEffort(TableLineInfo*) = 0;
    virtual void genCellProjectId(TableLineInfo*) = 0;
    virtual void genCellResources(TableLineInfo*) = 0;
    virtual void genCellResponsible(TableLineInfo*) = 0;
    virtual void genCellNote(TableLineInfo*) = 0;
    virtual void genCellStatusNote(TableLineInfo*) = 0;
    virtual void genCellCost(TableLineInfo*) = 0;
    virtual void genCellRevenue(TableLineInfo*) = 0;
    virtual void genCellProfit(TableLineInfo*) = 0;
    virtual void genCellPriority(TableLineInfo*) = 0;
    virtual void genCellFlags(TableLineInfo*) = 0;
    virtual void genCellCompleted(TableLineInfo*) = 0;
    virtual void genCellStatus(TableLineInfo*) = 0;
    virtual void genCellReference(TableLineInfo*) = 0;
    virtual void genCellScenario(TableLineInfo*) = 0;
    virtual void genCellDepends(TableLineInfo*) = 0;
    virtual void genCellFollows(TableLineInfo*) = 0;
    virtual void genCellDailyTask(TableLineInfo*) = 0;
    virtual void genCellDailyResource(TableLineInfo*) = 0;
    virtual void genCellDailyAccount(TableLineInfo*) = 0;
    virtual void genCellWeeklyTask(TableLineInfo*) = 0;
    virtual void genCellWeeklyResource(TableLineInfo*) = 0;
    virtual void genCellWeeklyAccount(TableLineInfo*) = 0;
    virtual void genCellMonthlyTask(TableLineInfo*) = 0;
    virtual void genCellMonthlyResource(TableLineInfo*) = 0;
    virtual void genCellMonthlyAccount(TableLineInfo*) = 0;
    virtual void genCellQuarterlyTask(TableLineInfo*) = 0;
    virtual void genCellQuarterlyResource(TableLineInfo*) = 0;
    virtual void genCellQuarterlyAccount(TableLineInfo*) = 0;
    virtual void genCellYearlyTask(TableLineInfo*) = 0;
    virtual void genCellYearlyResource(TableLineInfo*) = 0;
    virtual void genCellYearlyAccount(TableLineInfo*) = 0;
    virtual void genCellResponsibilities(TableLineInfo*) = 0;
    virtual void genCellSchedule(TableLineInfo*) = 0;
    virtual void genCellMinEffort(TableLineInfo*) = 0;
    virtual void genCellMaxEffort(TableLineInfo*) = 0;
    virtual void genCellRate(TableLineInfo*) = 0;
    virtual void genCellKotrusId(TableLineInfo*) = 0;
    virtual void genCellTotal(TableLineInfo*) = 0;
    virtual void genCellSummary(TableLineInfo*) = 0;
    
protected:
    ReportElement() { }

    QTextStream& s() const;
    void errorMessage(const char* msg, ... );

    /**
     * This utility function removes the path that matches the taskRoot
     * variable from the passed taskId.
     */
    QString stripTaskRoot(QString taskId) const;
    
    QString scaledLoad(double t) const;
    void reportValue(double value, const QString& bgcol, bool bold);

    Report* report;
    QValueList<int> scenarios;
    QPtrList<TableColumn> columns;
    QDict<TableColumnFormat> columnFormat;
    time_t start;
    time_t end;

    QString timeFormat;
    QString shortTimeFormat;
    
    /* We store the location of the report definition in case we need it
     * for error reporting. */
    QString defFileName;
    int defFileLine;
    
    QString headline;
    QString caption;

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

    bool showPIDs;

    bool accumulate;
    
    QMap<QString, QString> urls;
    
    /* The maximum depth of the tree that we have to report in tree-sorting
     * mode. */
    uint maxDepthTaskList;
    uint maxDepthResourceList;
    uint maxDepthAccountList;
    
    QMap<QString, double>* columnTotals;
    QMap<QString, double>* columnTotalsCosts;
    QMap<QString, double>* columnTotalsRevenue;
} ;

#endif

