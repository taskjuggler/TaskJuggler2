/*
 * ReportElement.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@suse.de>
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
#include "TableColorSet.h"
#include "MacroTable.h"
#include "RealFormat.h"
#include "TableColumnFormat.h"
#include "Report.h"

class QStringList;

class Project;
class CoreAttributes;
class ExpressionTree;
class Scenario;
class Task;
class TaskList;
class Resource;
class ResourceList;
class Account;
class AccountList;
class TableColumnInfo;
class TableCellInfo;
class TableLineInfo;
class CustomAttributeDefinition;

/**
 * @short A class that forms the basic element of a report. 
 * @author Chris Schlaeger <cs@suse.de>
 */
class ReportElement
{
public:
    enum BarLabelText { BLT_EMPTY = 0, BLT_LOAD };

    ReportElement(Report* r, const QString& df, int dl);
    virtual ~ReportElement();

    Report* getReport() const { return report; }

    void addScenario(int sc) { scenarios.append(sc); }
    void clearScenarios() { scenarios.clear(); }
    uint getScenarioCount() const { return scenarios.count(); }
    int getScenario(int sc) const { return scenarios[sc]; }

    void setHeadline(const QString& hl) { headline = hl; }
    void setCaption(const QString& c) { caption = c; }

    void setRawHead(const QString& head)
    {
        rawHead = head;
    }

    void setRawTail(const QString& tail)
    {
        rawTail = tail;
    }

    bool isSupportedColumn(const QString& id) const;
    QStringList getSupportedColumnList() const;
    void addColumn(const TableColumnInfo* c) { columns.append(c); }
    const TableColumnInfo* columnsAt(uint idx) { return columns.at(idx); }
    void clearColumns() { columns.clear(); }
    void setBarLabels(BarLabelText blt) { barLabels = blt; }

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

    void addColumnFormat(const QString& id, TableColumnFormat* tcf)
    {
        columnFormat.insert(id, tcf);
    }

    void setMacros(TableLineInfo* tli);

    virtual void genHeadDefault(TableCellInfo*) = 0;
    virtual void genHeadCurrency(TableCellInfo*) = 0;
    virtual void genHeadDaily1(TableCellInfo*) = 0;
    virtual void genHeadDaily2(TableCellInfo*) = 0;
    virtual void genHeadWeekly1(TableCellInfo*) = 0;
    virtual void genHeadWeekly2(TableCellInfo*) = 0;
    virtual void genHeadMonthly1(TableCellInfo*) = 0;
    virtual void genHeadMonthly2(TableCellInfo*) = 0;
    virtual void genHeadQuarterly1(TableCellInfo*) = 0;
    virtual void genHeadQuarterly2(TableCellInfo*) = 0;
    virtual void genHeadYear(TableCellInfo*) = 0;

    virtual void genCellEmpty(TableCellInfo*) = 0;
    virtual void genCellSequenceNo(TableCellInfo*) = 0;
    virtual void genCellNo(TableCellInfo*) = 0;
    virtual void genCellHierarchNo(TableCellInfo*) = 0;
    virtual void genCellIndex(TableCellInfo*) = 0;
    virtual void genCellHierarchIndex(TableCellInfo*) = 0;
    virtual void genCellId(TableCellInfo*) = 0;
    virtual void genCellName(TableCellInfo*) = 0;
    virtual void genCellStart(TableCellInfo*) = 0;
    virtual void genCellEnd(TableCellInfo*) = 0;
    virtual void genCellMinStart(TableCellInfo*) = 0;
    virtual void genCellMaxStart(TableCellInfo*) = 0;
    virtual void genCellMinEnd(TableCellInfo*) = 0;
    virtual void genCellMaxEnd(TableCellInfo*) = 0;
    virtual void genCellStartBuffer(TableCellInfo*) = 0;
    virtual void genCellEndBuffer(TableCellInfo*) = 0;
    virtual void genCellStartBufferEnd(TableCellInfo*) = 0;
    virtual void genCellEndBufferStart(TableCellInfo*) = 0;
    virtual void genCellDuration(TableCellInfo*) = 0;
    virtual void genCellEffort(TableCellInfo*) = 0;
    virtual void genCellProjectId(TableCellInfo*) = 0;
    virtual void genCellProjectIDs(TableCellInfo*) = 0;
    virtual void genCellResources(TableCellInfo*) = 0;
    virtual void genCellResponsible(TableCellInfo*) = 0;
    virtual void genCellText(TableCellInfo*) = 0;
    virtual void genCellStatusNote(TableCellInfo*) = 0;
    virtual void genCellCost(TableCellInfo*) = 0;
    virtual void genCellRevenue(TableCellInfo*) = 0;
    virtual void genCellProfit(TableCellInfo*) = 0;
    virtual void genCellPriority(TableCellInfo*) = 0;
    virtual void genCellFlags(TableCellInfo*) = 0;
    virtual void genCellCompleted(TableCellInfo*) = 0;
    virtual void genCellStatus(TableCellInfo*) = 0;
    virtual void genCellReference(TableCellInfo*) = 0;
    virtual void genCellScenario(TableCellInfo*) = 0;
    virtual void genCellDepends(TableCellInfo*) = 0;
    virtual void genCellFollows(TableCellInfo*) = 0;
    virtual void genCellDailyTask(TableCellInfo*) = 0;
    virtual void genCellDailyResource(TableCellInfo*) = 0;
    virtual void genCellDailyAccount(TableCellInfo*) = 0;
    virtual void genCellWeeklyTask(TableCellInfo*) = 0;
    virtual void genCellWeeklyResource(TableCellInfo*) = 0;
    virtual void genCellWeeklyAccount(TableCellInfo*) = 0;
    virtual void genCellMonthlyTask(TableCellInfo*) = 0;
    virtual void genCellMonthlyResource(TableCellInfo*) = 0;
    virtual void genCellMonthlyAccount(TableCellInfo*) = 0;
    virtual void genCellQuarterlyTask(TableCellInfo*) = 0;
    virtual void genCellQuarterlyResource(TableCellInfo*) = 0;
    virtual void genCellQuarterlyAccount(TableCellInfo*) = 0;
    virtual void genCellYearlyTask(TableCellInfo*) = 0;
    virtual void genCellYearlyResource(TableCellInfo*) = 0;
    virtual void genCellYearlyAccount(TableCellInfo*) = 0;
    virtual void genCellResponsibilities(TableCellInfo*) = 0;
    virtual void genCellSchedule(TableCellInfo*) = 0;
    virtual void genCellMinEffort(TableCellInfo*) = 0;
    virtual void genCellMaxEffort(TableCellInfo*) = 0;
    virtual void genCellRate(TableCellInfo*) = 0;
    virtual void genCellKotrusId(TableCellInfo*) = 0;
    virtual void genCellTotal(TableCellInfo*) = 0;
    virtual void genCellSummary(TableCellInfo*) = 0;
    
protected:
    ReportElement() { }

    void addCustomAttributeColumns
        (const QDict<const CustomAttributeDefinition>& cad);

    QTextStream& s() const;
    void puts(const QString& str) { report->puts(str); }

    void errorMessage(const char* msg, ... );

    /**
     * This utility function removes the path that matches the taskRoot
     * variable from the passed taskId.
     */
    QString stripTaskRoot(QString taskId) const;
    
    QString scaledLoad(double t, TableCellInfo* tci) const;
    void reportValue(double value, const QString& bgcol, bool bold);

    Report* report;
    QValueList<int> scenarios;
    QPtrList<TableColumnInfo> columns;
    QDict<TableColumnFormat> columnFormat;
    time_t start;
    time_t end;

    BarLabelText barLabels;

    QString rawHead;
    QString rawTail;

    QString timeFormat;
    QString shortTimeFormat;
    RealFormat numberFormat;
    RealFormat currencyFormat;
    
    /* We store the location of the report definition in case we need it
     * for error reporting. */
    QString defFileName;
    int defFileLine;
    
    TableColorSet colors;
    
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
    
    /* The maximum depth of the tree that we have to report in tree-sorting
     * mode. */
    uint maxDepthTaskList;
    uint maxDepthResourceList;
    uint maxDepthAccountList;

    MacroTable mt;
} ;

#endif

