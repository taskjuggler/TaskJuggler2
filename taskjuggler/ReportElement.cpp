/*
 * ReportElement.cpp - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#include <config.h>
#include <stdio.h>

#include <qdict.h>

#include "ReportElement.h"
#include "Interval.h"
#include "TableColumnFormat.h"
#include "TableColumnInfo.h"
#include "TableCellInfo.h"
#include "TjMessageHandler.h"
#include "tjlib-internal.h"
#include "Project.h"
#include "Report.h"
#include "Task.h"
#include "Resource.h"
#include "Account.h"
#include "Utility.h"
#include "ExpressionTree.h"
#include "TaskTreeIterator.h"
#include "ResourceTreeIterator.h"
#include "AccountTreeIterator.h"

#define KW(a) a

ReportElement::ReportElement(Report* r, const QString& df, int dl) :
        report(r), defFileName(df), defFileLine(dl)
{
    columns.setAutoDelete(TRUE);
    columnFormat.setAutoDelete(TRUE);

    maxDepthTaskList = 1;
    maxDepthResourceList = 1;
    maxDepthAccountList = 1;

    start = r->getStart();
    end = r->getEnd();
    
    timeFormat = r->getTimeFormat();
    shortTimeFormat = r->getShortTimeFormat();
    numberFormat = r->getNumberFormat();
    currencyFormat = r->getCurrencyFormat();

    TableColumnFormat* tcf = new TableColumnFormat(this, i18n("Seq. No."));
    tcf->genTaskLine1 = &ReportElement::genCellSequenceNo;
    tcf->genResourceLine1 = &ReportElement::genCellSequenceNo;
    tcf->genAccountLine1 = &ReportElement::genCellSequenceNo;
    tcf->hAlign = "right";
    columnFormat.insert(KW("seqno"), tcf);

    tcf = new TableColumnFormat(this, i18n("No."));
    tcf->genTaskLine1 = &ReportElement::genCellNo;
    tcf->genResourceLine1 = &ReportElement::genCellNo;
    tcf->genAccountLine1 = &ReportElement::genCellNo;
    tcf->hAlign = "right";
    columnFormat.insert(KW("no"), tcf);
    
    tcf = new TableColumnFormat(this, i18n("No."));
    tcf->genTaskLine1 = &ReportElement::genCellHierarchNo;
    tcf->genResourceLine1 = &ReportElement::genCellHierarchNo;
    tcf->genAccountLine1 = &ReportElement::genCellHierarchNo;
    tcf->hAlign = "left";
    columnFormat.insert(KW("hierarchno"), tcf);
    
    tcf = new TableColumnFormat(this, i18n("Index"));
    tcf->genTaskLine1 = &ReportElement::genCellIndex;
    tcf->genResourceLine1 = &ReportElement::genCellIndex;
    tcf->genAccountLine1 = &ReportElement::genCellIndex;
    tcf->hAlign = "right";
    columnFormat.insert(KW("index"), tcf);
    
    tcf = new TableColumnFormat(this, i18n("No."));
    tcf->genTaskLine1 = &ReportElement::genCellHierarchIndex;
    tcf->genResourceLine1 = &ReportElement::genCellHierarchIndex;
    tcf->genAccountLine1 = &ReportElement::genCellHierarchIndex;
    tcf->hAlign = "left";
    columnFormat.insert(KW("hierarchindex"), tcf);
    
    tcf = new TableColumnFormat(this, i18n("Id"));
    tcf->genTaskLine1 = &ReportElement::genCellId;
    tcf->genResourceLine1 = &ReportElement::genCellId;
    tcf->genAccountLine1 = &ReportElement::genCellId;
    columnFormat.insert(KW("id"), tcf);
    
    tcf = new TableColumnFormat(this, i18n("Name"));
    tcf->genTaskLine1 = &ReportElement::genCellName;
    tcf->genResourceLine1 = &ReportElement::genCellName;
    tcf->genAccountLine1 = &ReportElement::genCellName;
    tcf->genSummaryLine1 = &ReportElement::genCellName;
    tcf->noWrap = TRUE;
    columnFormat.insert(KW("name"), tcf);
    
    tcf = new TableColumnFormat(this, i18n("Start"));
    tcf->genTaskLine1 = &ReportElement::genCellStart;
    tcf->genTaskLine2 = &ReportElement::genCellStart;
    columnFormat.insert(KW("start"), tcf);
    
    tcf = new TableColumnFormat(this, i18n("End"));
    tcf->genTaskLine1 = &ReportElement::genCellEnd;
    tcf->genTaskLine2 = &ReportElement::genCellEnd;
    columnFormat.insert(KW("end"), tcf);
    
    tcf = new TableColumnFormat(this, i18n("Min. Start"));
    tcf->genTaskLine1 = &ReportElement::genCellMinStart;
    columnFormat.insert(KW("minstart"), tcf);
    
    tcf = new TableColumnFormat(this, i18n("Max. Start"));
    tcf->genTaskLine1 = &ReportElement::genCellMaxStart;
    columnFormat.insert(KW("maxstart"), tcf);
    
    tcf = new TableColumnFormat(this, i18n("Min. End"));
    tcf->genTaskLine1 = &ReportElement::genCellMinEnd;
    columnFormat.insert(KW("minend"), tcf);
    
    tcf = new TableColumnFormat(this, i18n("Max. End"));
    tcf->genTaskLine1 = &ReportElement::genCellMaxEnd;
    columnFormat.insert(KW("maxend"), tcf);
    
    tcf = new TableColumnFormat(this, i18n("Start Buf. End"));
    tcf->genTaskLine1 = &ReportElement::genCellStartBufferEnd;
    tcf->genTaskLine2 = &ReportElement::genCellStartBufferEnd;
    columnFormat.insert(KW("startbufferend"), tcf);
    
    tcf = new TableColumnFormat(this, i18n("End Buf. Start"));
    tcf->genTaskLine1 = &ReportElement::genCellEndBufferStart;
    tcf->genTaskLine2 = &ReportElement::genCellEndBufferStart;
    columnFormat.insert(KW("endbufferstart"), tcf);
    
    tcf = new TableColumnFormat(this, i18n("Start Buf."));
    tcf->genTaskLine1 = &ReportElement::genCellStartBuffer;
    tcf->genTaskLine2 = &ReportElement::genCellStartBuffer;
    tcf->hAlign = "right";
    columnFormat.insert(KW("startbuffer"), tcf);
    
    tcf = new TableColumnFormat(this, i18n("End Buf."));
    tcf->genTaskLine1 = &ReportElement::genCellEndBuffer;
    tcf->genTaskLine2 = &ReportElement::genCellEndBuffer;
    tcf->hAlign = "right";
    columnFormat.insert(KW("endbuffer"), tcf);
    
    tcf = new TableColumnFormat(this, i18n("Duration"));
    tcf->genTaskLine1 = &ReportElement::genCellDuration;
    tcf->genTaskLine2 = &ReportElement::genCellDuration;
    tcf->hAlign = "right";
    tcf->realFormat = numberFormat;
    columnFormat.insert(KW("duration"), tcf);
    
    tcf = new TableColumnFormat(this, i18n("Effort"));
    tcf->genTaskLine1 = &ReportElement::genCellEffort;
    tcf->genTaskLine2 = &ReportElement::genCellEffort;
    tcf->genResourceLine1 = &ReportElement::genCellEffort;
    tcf->genResourceLine2 = &ReportElement::genCellEffort;
    tcf->hAlign = "right";
    tcf->realFormat = numberFormat;
    columnFormat.insert(KW("effort"), tcf);
    
    tcf = new TableColumnFormat(this, i18n("Project ID"));
    tcf->genTaskLine1 = &ReportElement::genCellProjectId;
    columnFormat.insert(KW("projectid"), tcf);
    
    tcf = new TableColumnFormat(this, i18n("Resources"));
    tcf->genTaskLine1 = &ReportElement::genCellResources;
    tcf->genTaskLine2 = &ReportElement::genCellResources;
    tcf->fontFactor = 80;
    columnFormat.insert(KW("resources"), tcf);
    
    tcf = new TableColumnFormat(this, i18n("Responsible"));
    tcf->genTaskLine1 = &ReportElement::genCellResponsible;
    columnFormat.insert(KW("responsible"), tcf);
    
    tcf = new TableColumnFormat(this, i18n("Responsibilities"));
    tcf->genResourceLine1 = &ReportElement::genCellResponsibilities;
    tcf->fontFactor = 80;
    columnFormat.insert(KW("responsibilities"), tcf);
    
    tcf = new TableColumnFormat(this, i18n("Dependencies"));
    tcf->genTaskLine1 = &ReportElement::genCellDepends;
    tcf->fontFactor = 80;
    columnFormat.insert(KW("depends"), tcf);
    
    tcf = new TableColumnFormat(this, i18n("Followers"));
    tcf->genTaskLine1 = &ReportElement::genCellFollows;
    tcf->fontFactor = 80;
    columnFormat.insert(KW("follows"), tcf);
    
    tcf = new TableColumnFormat(this, i18n("Schedule"));
    tcf->genResourceLine1 = &ReportElement::genCellSchedule;
    tcf->genResourceLine2 = &ReportElement::genCellSchedule;
    columnFormat.insert(KW("schedule"), tcf);
    
    tcf = new TableColumnFormat(this, i18n("Min. Effort"));
    tcf->genResourceLine1 = &ReportElement::genCellMinEffort;
    tcf->hAlign = "right";
    tcf->realFormat = numberFormat;
    columnFormat.insert(KW("mineffort"), tcf);
    
    tcf = new TableColumnFormat(this, i18n("Max. Effort"));
    tcf->genResourceLine1 = &ReportElement::genCellMaxEffort;
    tcf->hAlign = "right";
    tcf->realFormat = numberFormat;
    columnFormat.insert(KW("maxeffort"), tcf);
    
    tcf = new TableColumnFormat(this, i18n("Flags"));
    tcf->genTaskLine1 = &ReportElement::genCellFlags;
    tcf->genResourceLine1 = &ReportElement::genCellFlags;
    columnFormat.insert(KW("flags"), tcf);
    
    tcf = new TableColumnFormat(this, i18n("Completed"));
    tcf->genTaskLine1 = &ReportElement::genCellCompleted;
    tcf->genTaskLine2 = &ReportElement::genCellCompleted;
    tcf->hAlign = "right";
    columnFormat.insert(KW("completed"), tcf);
    
    tcf = new TableColumnFormat(this, i18n("Status"));
    tcf->genTaskLine1 = &ReportElement::genCellStatus;
    tcf->genTaskLine2 = &ReportElement::genCellStatus;
    tcf->hAlign = "center";
    columnFormat.insert(KW("status"), tcf);
    
    tcf = new TableColumnFormat(this, i18n("Kotrus ID"));
    tcf->genResourceLine1 = &ReportElement::genCellKotrusId;
    columnFormat.insert(KW("kotrusid"), tcf);
    
    tcf = new TableColumnFormat(this, i18n("Note"));
    tcf->genTaskLine1 = &ReportElement::genCellNote;
    tcf->fontFactor = 80;
    columnFormat.insert(KW("note"), tcf);
    
    tcf = new TableColumnFormat(this, i18n("Status Note"));
    tcf->genTaskLine1 = &ReportElement::genCellStatusNote;
    tcf->fontFactor = 80;
    columnFormat.insert(KW("statusnote"), tcf);
    
    tcf = new TableColumnFormat(this, i18n("Priority"));
    tcf->genTaskLine1 = &ReportElement::genCellPriority;
    tcf->hAlign = "right";
    columnFormat.insert(KW("priority"), tcf);
    
    tcf = new TableColumnFormat(this, i18n("Reference"));
    tcf->genTaskLine1 = &ReportElement::genCellReference;
    columnFormat.insert(KW("reference"), tcf);
    
    tcf = new TableColumnFormat(this, i18n("Scenario"));
    tcf->genTaskLine1 = &ReportElement::genCellScenario;
    tcf->genTaskLine2 = &ReportElement::genCellScenario;
    tcf->genResourceLine1 = &ReportElement::genCellScenario;
    tcf->genResourceLine2 = &ReportElement::genCellScenario;
    tcf->genAccountLine1 = &ReportElement::genCellScenario;
    tcf->genAccountLine2 = &ReportElement::genCellScenario;
    tcf->genSummaryLine1 = &ReportElement::genCellScenario;
    tcf->genSummaryLine2 = &ReportElement::genCellScenario;
    tcf->hAlign = "left";
    columnFormat.insert(KW("scenario"), tcf);
    
    tcf = new TableColumnFormat(this, i18n("Rate"));
    tcf->genHeadLine1 = &ReportElement::genHeadCurrency;
    tcf->genResourceLine1 = &ReportElement::genCellRate;
    tcf->hAlign = "right";
    tcf->realFormat = currencyFormat;
    columnFormat.insert(KW("rate"), tcf);
    
    tcf = new TableColumnFormat(this, i18n("Cost"));
    tcf->genHeadLine1 = &ReportElement::genHeadCurrency;
    tcf->genTaskLine1 = &ReportElement::genCellCost;
    tcf->genTaskLine2 = &ReportElement::genCellCost;
    tcf->genResourceLine1 = &ReportElement::genCellCost;
    tcf->genResourceLine2 = &ReportElement::genCellCost;
    tcf->hAlign = "right";
    tcf->realFormat = currencyFormat;
    columnFormat.insert(KW("cost"), tcf);
    
    tcf = new TableColumnFormat(this, i18n("Revenue"));
    tcf->genHeadLine1 = &ReportElement::genHeadCurrency;
    tcf->genTaskLine1 = &ReportElement::genCellRevenue;
    tcf->genTaskLine2 = &ReportElement::genCellRevenue;
    tcf->genResourceLine1 = &ReportElement::genCellRevenue;
    tcf->genResourceLine2 = &ReportElement::genCellRevenue;
    tcf->hAlign = "right";
    tcf->realFormat = currencyFormat;
    columnFormat.insert(KW("revenue"), tcf);
    
    tcf = new TableColumnFormat(this, i18n("Profit"));
    tcf->genHeadLine1 = &ReportElement::genHeadCurrency;
    tcf->genTaskLine1 = &ReportElement::genCellProfit;
    tcf->genTaskLine2 = &ReportElement::genCellProfit;
    tcf->genResourceLine1 = &ReportElement::genCellProfit;
    tcf->genResourceLine2 = &ReportElement::genCellProfit;
    tcf->hAlign = "right";
    tcf->realFormat = currencyFormat;
    columnFormat.insert(KW("profit"), tcf);
   
    tcf = new TableColumnFormat(this, i18n("Total"));
    tcf->genHeadLine1 = &ReportElement::genHeadCurrency;
    tcf->genAccountLine1 = &ReportElement::genCellTotal;
    tcf->genAccountLine2 = &ReportElement::genCellTotal;
    tcf->genSummaryLine1 = &ReportElement::genCellSummary;
    tcf->genSummaryLine2 = &ReportElement::genCellSummary;
    tcf->hAlign = "right";
    tcf->realFormat = currencyFormat;
    columnFormat.insert(KW("total"), tcf);

    tcf = new TableColumnFormat(this, "");
    tcf->genHeadLine1 = &ReportElement::genHeadDaily1;
    tcf->genHeadLine2 = &ReportElement::genHeadDaily2;
    tcf->genTaskLine1 = &ReportElement::genCellDailyTask;
    tcf->genTaskLine2 = &ReportElement::genCellDailyTask;
    tcf->genResourceLine1 = &ReportElement::genCellDailyResource;
    tcf->genResourceLine2 = &ReportElement::genCellDailyResource;
    tcf->genAccountLine1 = &ReportElement::genCellDailyAccount;
    tcf->genAccountLine2 = &ReportElement::genCellDailyAccount;
    tcf->genSummaryLine1 = &ReportElement::genCellSummary;
    tcf->genSummaryLine2 = &ReportElement::genCellSummary;
    tcf->hAlign = "right";
    tcf->realFormat = numberFormat;
    columnFormat.insert(KW("daily"), tcf);
    
    tcf = new TableColumnFormat(this, "");
    tcf->genHeadLine1 = &ReportElement::genHeadWeekly1;
    tcf->genHeadLine2 = &ReportElement::genHeadWeekly2;
    tcf->genTaskLine1 = &ReportElement::genCellWeeklyTask;
    tcf->genTaskLine2 = &ReportElement::genCellWeeklyTask;
    tcf->genResourceLine1 = &ReportElement::genCellWeeklyResource;
    tcf->genResourceLine2 = &ReportElement::genCellWeeklyResource;
    tcf->genAccountLine1 = &ReportElement::genCellWeeklyAccount;
    tcf->genAccountLine2 = &ReportElement::genCellWeeklyAccount;
    tcf->genSummaryLine1 = &ReportElement::genCellSummary;
    tcf->genSummaryLine2 = &ReportElement::genCellSummary;
    tcf->hAlign = "right";
    tcf->realFormat = numberFormat;
    columnFormat.insert(KW("weekly"), tcf);
    
    tcf = new TableColumnFormat(this, "");
    tcf->genHeadLine1 = &ReportElement::genHeadMonthly1;
    tcf->genHeadLine2 = &ReportElement::genHeadMonthly2;
    tcf->genTaskLine1 = &ReportElement::genCellMonthlyTask;
    tcf->genTaskLine2 = &ReportElement::genCellMonthlyTask;
    tcf->genResourceLine1 = &ReportElement::genCellMonthlyResource;
    tcf->genResourceLine2 = &ReportElement::genCellMonthlyResource;
    tcf->genAccountLine1 = &ReportElement::genCellMonthlyAccount;
    tcf->genAccountLine2 = &ReportElement::genCellMonthlyAccount;
    tcf->genSummaryLine1 = &ReportElement::genCellSummary;
    tcf->genSummaryLine2 = &ReportElement::genCellSummary;
    tcf->hAlign = "right";
    tcf->realFormat = numberFormat;
    columnFormat.insert(KW("monthly"), tcf);
   
    tcf = new TableColumnFormat(this, "");
    tcf->genHeadLine1 = &ReportElement::genHeadQuarterly1;
    tcf->genHeadLine2 = &ReportElement::genHeadQuarterly2;
    tcf->genTaskLine1 = &ReportElement::genCellQuarterlyTask;
    tcf->genTaskLine2 = &ReportElement::genCellQuarterlyTask;
    tcf->genResourceLine1 = &ReportElement::genCellQuarterlyResource;
    tcf->genResourceLine2 = &ReportElement::genCellQuarterlyResource;
    tcf->genAccountLine1 = &ReportElement::genCellQuarterlyAccount;
    tcf->genAccountLine2 = &ReportElement::genCellQuarterlyAccount;
    tcf->genSummaryLine1 = &ReportElement::genCellSummary;
    tcf->genSummaryLine2 = &ReportElement::genCellSummary;
    tcf->hAlign = "right";
    tcf->realFormat = numberFormat;
    columnFormat.insert(KW("quarterly"), tcf);
   
    tcf = new TableColumnFormat(this, "");
    tcf->genHeadLine1 = &ReportElement::genHeadYear;
    tcf->genTaskLine1 = &ReportElement::genCellYearlyTask;
    tcf->genTaskLine2 = &ReportElement::genCellYearlyTask;
    tcf->genResourceLine1 = &ReportElement::genCellYearlyResource;
    tcf->genResourceLine2 = &ReportElement::genCellYearlyResource;
    tcf->genAccountLine1 = &ReportElement::genCellYearlyAccount;
    tcf->genAccountLine2 = &ReportElement::genCellYearlyAccount;
    tcf->genSummaryLine1 = &ReportElement::genCellSummary;
    tcf->genSummaryLine2 = &ReportElement::genCellSummary;
    tcf->hAlign = "right";
    tcf->realFormat = numberFormat;
    columnFormat.insert(KW("yearly"), tcf);
   
    scenarios.append(0);

    barLabels = BLT_LOAD;

    registerUrl(KW("dayheader"));
    registerUrl(KW("monthheader"));
    registerUrl(KW("resourcename"));
    registerUrl(KW("taskname"));
    registerUrl(KW("weekheader"));
    registerUrl(KW("yearheader"));

    accumulate = FALSE;

    for (int i = 0; i < CoreAttributesList::maxSortingLevel; ++i)
    {
        taskSortCriteria[i] = r->getTaskSorting(i);
        resourceSortCriteria[i] = r->getResourceSorting(i);
        accountSortCriteria[i] = r->getAccountSorting(i);
    }

    hideTask = r->getHideTask() ? 
        new ExpressionTree(*r->getHideTask()) : 0;
    rollUpTask = r->getRollUpTask() ? 
        new ExpressionTree(*r->getRollUpTask()) : 0;
    hideResource = r->getHideResource() ?
        new ExpressionTree(*r->getHideResource()) : 0;
    rollUpResource = r->getRollUpResource() ?
        new ExpressionTree(*r->getRollUpResource()) : 0;
    hideAccount = r->getHideAccount() ?
        new ExpressionTree(*r->getHideAccount()) : 0;
    rollUpAccount = r->getRollUpAccount() ?
        new ExpressionTree(*r->getRollUpAccount()) : 0;

    taskRoot = r->getTaskRoot();
    loadUnit = r->getLoadUnit();
    showPIDs = r->getShowPIDs();
}

ReportElement::~ReportElement()
{
    delete hideTask;
    delete rollUpTask;
    delete hideResource;
    delete rollUpResource;
    delete hideAccount;
    delete rollUpAccount;
}
    
QTextStream& 
ReportElement::s() const
{
    return report->stream(); 
}

void
ReportElement::registerUrl(const QString& key)
{
    urls[key] = QString::null;
}

bool
ReportElement::setUrl(const QString& key, const QString& url)
{
    if (urls.find(key) == urls.end())
        return FALSE;

    urls[key] = url;
    return TRUE;
}

const QString*
ReportElement::getUrl(const QString& key) const
{
    if (urls.find(key) == urls.end() || urls[key] == "")
        return 0;
    return &urls[key];
}

bool
ReportElement::isSupportedColumn(const QString& id) const
{
    return columnFormat[id];
}

QStringList
ReportElement::getSupportedColumnList() const
{
    QStringList l;
    QDictIterator<TableColumnFormat> it(columnFormat);
    for ( ; it.current(); ++it)
        l.append(it.currentKey());

    l.sort();
    return l;
}

bool 
ReportElement::setTaskSorting(int sc, int level)
{
    if (level >= 0 && level < CoreAttributesList::maxSortingLevel)
    {
        if ((sc == CoreAttributesList::TreeMode && level > 0) ||
            !TaskList::isSupportedSortingCriteria(sc & 0xFFFF))
            return FALSE;
        taskSortCriteria[level] = sc;
    }
    else
        return FALSE;
    return TRUE;
}

bool 
ReportElement::setResourceSorting(int sc, int level)
{
    if (level >= 0 && level < CoreAttributesList::maxSortingLevel)
    {
        if ((sc == CoreAttributesList::TreeMode && level > 0) ||
            !ResourceList::isSupportedSortingCriteria(sc & 0xFFFF))
            return FALSE;
        resourceSortCriteria[level] = sc;
    }
    else
        return FALSE;
    return TRUE;
}

bool 
ReportElement::setAccountSorting(int sc, int level)
{
    if (level >= 0 && level < CoreAttributesList::maxSortingLevel)
    {
        if ((sc == CoreAttributesList::TreeMode && level > 0) ||
            !AccountList::isSupportedSortingCriteria(sc & 0xFFFF))
            return FALSE;
        accountSortCriteria[level] = sc;
    }
    else
        return FALSE;
    return TRUE;
}

bool
ReportElement::isHidden(const CoreAttributes* c, ExpressionTree* et) const
{
    /* Determine whether a CoreAttributes object should be hidden according to
     * the ExpressionTree et. */

    /* First check if the object is part of the sub-tree specified by
     * taskRoot. The check only applies to Task objects. */
    if (strcmp(c->getType(), "Task") == 0 &&
        !taskRoot.isEmpty() &&
        taskRoot != c->getId().left(taskRoot.length()))
    {
        return TRUE;
    }
    
    // If we don't have an ExpressionTree the object is always visible.
    if (!et)
        return FALSE;

    // Pump all flags into the symbol table.
    et->clearSymbolTable();
    QStringList flags = c->getFlagList();
    for (QStringList::Iterator it = flags.begin(); it != flags.end(); ++it)
        et->registerSymbol(*it, 1);

    return et->evalAsInt(c) != 0;
}

bool
ReportElement::isRolledUp(const CoreAttributes* c, ExpressionTree* et) const
{
    /* Determine wheter a CoreAttributes object should hide all it's 
     * descendants. */

    /* If we don't have an ExpressionTree the object's descendants are always
     * visible. */
    if (!et)
        return FALSE;

    // Pump all flags into the symbol table.
    et->clearSymbolTable();
    QStringList flags = c->getFlagList();
    for (QStringList::Iterator it = flags.begin(); it != flags.end(); ++it)
        et->registerSymbol(*it, 1);
    return et->evalAsInt(c) != 0;
}

void
ReportElement::setHideTask(ExpressionTree* et)
{
    delete hideTask;
    hideTask = et;
}

void
ReportElement::setRollUpTask(ExpressionTree* et)
{
    delete rollUpTask;
    rollUpTask = et; 
}

void
ReportElement::setHideResource(ExpressionTree* et)
{
    delete hideResource;
    hideResource = et; 
}

void
ReportElement::setRollUpResource(ExpressionTree* et)
{
    delete rollUpResource;
    rollUpResource = et;
}

void
ReportElement::setHideAccount(ExpressionTree* et)
{
    delete hideAccount;
    hideAccount = et;
}

void
ReportElement::setRollUpAccount(ExpressionTree* et)
{
    delete rollUpAccount;
    rollUpAccount = et;
}

bool
ReportElement::setLoadUnit(const QString& u)
{
    if (u == KW("minutes"))
        loadUnit = minutes;
    else if (u == KW("hours"))
        loadUnit = hours;
    else if (u == KW("days"))
        loadUnit = days;
    else if (u == KW("weeks"))
        loadUnit = weeks;
    else if (u == KW("months"))
        loadUnit = months;
    else if (u == KW("years"))
        loadUnit = years;
    else if (u == KW("shortauto"))
        loadUnit = shortAuto;
    else if (u == KW("longauto"))
        loadUnit = longAuto;
    else
        return FALSE;

    return TRUE;
}

void
ReportElement::filterTaskList(TaskList& filteredList, const Resource* r,
                              ExpressionTree* hideExp, 
                              ExpressionTree* rollUpExp)
const
{
    /* Create a new list that contains only those tasks that were not
     * hidden. */
    filteredList.clear();
    for (TaskListIterator tli(report->getProject()->getTaskListIterator()); 
         *tli != 0; ++tli)
    {
        bool resourceLoadedInAnyScenario = FALSE;
        if (r != 0)
        {
            QValueList<int>::const_iterator it;
            for (it = scenarios.begin(); it != scenarios.end(); ++it)
                if (r->getLoad(*it, Interval(start, end), 
                               AllAccounts, *tli) > 0.0)
                {
                    resourceLoadedInAnyScenario = TRUE;
                    break;
                }
        } 
        Interval iv(start, end);
        if (!isHidden(*tli, hideExp) &&
            iv.overlaps(Interval((*tli)->getStart(Task::Plan),
                                 (*tli)->isMilestone() ? 
                                 (*tli)->getStart(Task::Plan) :
                                 (*tli)->getEnd(Task::Plan))) &&
            (r == 0 || resourceLoadedInAnyScenario)) 
        {
            filteredList.append(tli);
        }
    }

    /* Now we have to remove all sub tasks of rolled-up tasks
     * from the filtered list */
    for (TaskListIterator tli(report->getProject()->getTaskListIterator());
         *tli != 0; ++tli)
        if (isRolledUp(*tli, rollUpExp))
            for (TaskTreeIterator tti(*tli,
                                      TaskTreeIterator::parentAfterLeaves);
                 *tti != 0; ++tti)
                if (*tti != *tli)
                    filteredList.removeRef(*tti);
}

void
ReportElement::sortTaskList(TaskList& filteredList)
{
    /* In tasktree sorting mode we need to make sure that we don't hide
     * parents of shown tasks. */
    TaskList list = filteredList;
    if (taskSortCriteria[0] == CoreAttributesList::TreeMode)
    {
        // Set sorting criteria so sequence no since list.contains() needs it.
        filteredList.setSorting(CoreAttributesList::SequenceUp, 0);
        for (TaskListIterator tli(filteredList); *tli != 0; ++tli)
        {
            // Do not add the taskRoot task or any of it's parents.
            for (Task* p = (*tli)->getParent();
                 p != 0 && (p->getId() + "." != taskRoot);
                 p = p->getParent())
                if (list.contains(p) == 0)
                    list.append(p);
        }
    }
    filteredList = list;

    for (int i = 0; i < CoreAttributesList::maxSortingLevel; i++)
        filteredList.setSorting(taskSortCriteria[i], i);
    filteredList.sort();
}

void
ReportElement::filterResourceList(ResourceList& filteredList, const Task* t,
                           ExpressionTree* hideExp, ExpressionTree* rollUpExp)
const
{
    /* Create a new list that contains only those resources that were
     * not hidden. */
    filteredList.clear();
    for (ResourceListIterator rli(report->getProject()->
                                  getResourceListIterator());
         *rli != 0; ++rli)
    {
        bool taskLoadedInAnyScenario = FALSE;
        if (t != 0)
        {
            QValueList<int>::const_iterator it;
            for (it = scenarios.begin(); it != scenarios.end(); ++it)
                if ((*rli)->getLoad(*it, Interval(start, end), 
                                    AllAccounts, t) > 0.0)
                {
                    taskLoadedInAnyScenario = TRUE;
                    break;
                }
        } 
        if (!isHidden(*rli, hideExp) &&
            (t == 0 || taskLoadedInAnyScenario)) 
        {
            filteredList.append(*rli);
        }
    }

    /* Now we have to remove all sub resources of resource in the
     * roll-up list from the filtered list */
    for (ResourceListIterator rli(report->getProject()->
                                  getResourceListIterator());
         *rli != 0; ++rli)
        if (isRolledUp(*rli, rollUpExp))
            for (ResourceTreeIterator rti(*rli,
                                          ResourceTreeIterator::
                                          parentAfterLeaves);
                 *rti != 0; ++rti)
                if (*rti != *rli)
                    filteredList.removeRef(*rti);
}

void
ReportElement::sortResourceList(ResourceList& filteredList)
{
    /* In resourcetree sorting mode we need to make sure that we don't
     * hide parents of shown resources. */
    ResourceList list = filteredList;
    if (resourceSortCriteria[0] == CoreAttributesList::TreeMode)
    {
        // Set sorting criteria to sequence no since list.contains() needs it.
        filteredList.setSorting(CoreAttributesList::SequenceUp, 0);
        for (ResourceListIterator rli(filteredList); *rli != 0; ++rli)
        {
            for (Resource* p = (*rli)->getParent(); p != 0; p = p->getParent())
                if (list.contains(p) == 0)
                    list.append(p);
        }
    }
    filteredList = list;

    for (int i = 0; i < CoreAttributesList::maxSortingLevel; i++)
        filteredList.setSorting(resourceSortCriteria[i], i);
    filteredList.sort();
}

void
ReportElement::filterAccountList(AccountList& filteredList, AccountType at,
                          ExpressionTree* hideExp, ExpressionTree* rollUpExp)
const
{
    /* Create a new list that contains only those accounts that were not
     * hidden. */
    filteredList.clear();
    for (AccountListIterator ali(report->getProject()->
                                 getAccountListIterator()); 
         *ali != 0; ++ali)
    {
        if (!isHidden(*ali, hideExp) && (*ali)->getAcctType() == at)
            filteredList.append(*ali);
    }

    /* Now we have to remove all sub accounts of account in the roll-up list
     * from the filtered list */
    for (AccountListIterator ali(report->getProject()->
                                 getAccountListIterator()); 
         *ali != 0; ++ali)
        if (isRolledUp(*ali, rollUpExp))
            for (AccountTreeIterator ati(*ali,
                                         AccountTreeIterator::
                                         parentAfterLeaves);
                 *ati != 0; ++ati)
                if (*ati != *ali)
                    filteredList.removeRef(*ati);
}

void
ReportElement::sortAccountList(AccountList& filteredList)
{
    /* In accounttree sorting mode we need to make sure that we don't hide
     * parents of shown accounts. */
    AccountList list = filteredList;
    if (accountSortCriteria[0] == CoreAttributesList::TreeMode)
    {
        // Set sorting criteria so sequence no since list.contains() needs it.
        list.setSorting(CoreAttributesList::SequenceUp, 0);
        for (AccountListIterator ali(filteredList); *ali != 0; ++ali)
        {
            for (Account* p = (*ali)->getParent(); p != 0; p = p->getParent())
                if (list.contains(p) == 0)
                    list.append(p);
        }
    }
    filteredList = list;

    for (int i = 0; i < CoreAttributesList::maxSortingLevel; i++)
        filteredList.setSorting(accountSortCriteria[i], i);
    filteredList.sort();
    
    maxDepthAccountList = filteredList.maxDepth();
}

QString
ReportElement::scaledLoad(double t, TableCellInfo* tci) const
{
    QStringList variations;
    QValueList<double> factors;
    const char* shortUnit[] = { "d", "min", "h", "w", "m", "y" };
    const char* unit[] = { "day", "minute", "hour", "week", "month", "year" };
    const char* units[] = { "days", "minutes", "hours", "weeks", "months", 
        "years"};
    double max[] = { 0, 60, 48, 8, 24, 0 };

    factors.append(1);
    factors.append(report->getProject()->getDailyWorkingHours() * 60);
    factors.append(report->getProject()->getDailyWorkingHours());
    factors.append(1.0 / report->getProject()->getWeeklyWorkingDays());
    factors.append(1.0 / report->getProject()->getMonthlyWorkingDays());
    factors.append(1.0 / report->getProject()->getYearlyWorkingDays());

    QString str;
    
    if (loadUnit == shortAuto || loadUnit == longAuto)
    {
        for (QValueList<double>::Iterator it = factors.begin();
             it != factors.end(); ++it)
        {
            str = tci->tcf->realFormat.format(t * *it, FALSE);
            int idx = factors.findIndex(*it);
            if ((*it != 1.0 && str == "0") ||
                (max[idx] != 0 && max[idx] < (t * *it)))
                variations.append("");
            else
                variations.append(str);
        }

        uint shortest = 0;
        for (QStringList::Iterator it = variations.begin();
             it != variations.end();
             ++it)
        {
            if ((*it).length() > 0 &&
                (*it).length() < variations[shortest].length())
            {
                shortest = variations.findIndex(*it);
            }
        }
        str = variations[shortest];
        if (loadUnit == longAuto)
        {
            if (variations[shortest] == "1")
                str += QString(" ") + unit[shortest];
            else
                str += QString(" ") + units[shortest];
        }
        else
            str += shortUnit[shortest];
    }
    else
    {
        switch (loadUnit)
        {
            case days:
                str = tci->tcf->realFormat.format(t * factors[0], FALSE);
                break;
            case minutes:
                str = tci->tcf->realFormat.format(t * factors[1], FALSE);
                break;
            case hours:
                str = tci->tcf->realFormat.format(t * factors[2], FALSE);
                break;
            case weeks:
                str = tci->tcf->realFormat.format(t * factors[3], FALSE);
                break;
            case months:
                str = tci->tcf->realFormat.format(t * factors[4], FALSE);
                break;
            case years:
                str = tci->tcf->realFormat.format(t * factors[5], FALSE);
                break;
            case shortAuto:
            case longAuto:
                break;  // handled above switch statement already
        }
    }
    return str;
}

void
ReportElement::errorMessage(const char* msg, ... )
{
    va_list ap;
    va_start(ap, msg);
    char buf[1024];
    vsnprintf(buf, 1024, msg, ap);
    va_end(ap);
        
    TJMH.errorMessage(buf, defFileName, defFileLine);
}

QString
ReportElement::stripTaskRoot(QString taskId) const
{
    if (taskRoot == taskId.left(taskRoot.length()))
        return taskId.right(taskId.length() - taskRoot.length());
    else
        return taskId;
}

