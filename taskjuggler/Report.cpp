/*
 * Report.cpp - TaskJuggler
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

#include "Report.h"
#include "Interval.h"
#include "TjMessageHandler.h"
#include "tjlib-internal.h"
#include "Project.h"
#include "Task.h"
#include "Resource.h"
#include "Account.h"
#include "Utility.h"
#include "ExpressionTree.h"
#include "TaskTreeIterator.h"
#include "ResourceTreeIterator.h"
#include "AccountTreeIterator.h"

#define KW(a) a

Report::Report(const Project* p, const QString& f, const QString& df, int dl) :
        project(p), fileName(f), defFileName(df), defFileLine(dl)
{
    for (int i = 0; i < CoreAttributesList::maxSortingLevel; ++i)
    {
        taskSortCriteria[i] = CoreAttributesList::SequenceUp;
        resourceSortCriteria[i] = CoreAttributesList::SequenceUp;
        accountSortCriteria[i] = CoreAttributesList::SequenceUp;
    }

    start = p->getStart();
    end = p->getEnd();

    weekStartsMonday = p->getWeekStartsMonday();
    timeFormat = p->getTimeFormat();
    shortTimeFormat = p->getShortTimeFormat();
    numberFormat = p->getNumberFormat();
    currencyFormat = p->getCurrencyFormat();

    hideTask = 0;
    rollUpTask = 0;
    hideResource = 0;
    rollUpResource = 0;
    hideAccount = 0;
    rollUpAccount = 0;

    showPIDs = FALSE;

    loadUnit = days;

    timeStamp = TRUE;

    maxDepthTaskList = 1;
    maxDepthResourceList = 1;
    maxDepthAccountList = 1;
}

Report::~Report()
{
    delete hideTask;
    delete rollUpTask;
    delete hideResource;
    delete rollUpResource;
    delete hideAccount;
    delete rollUpAccount;
}

bool
Report::open()
{
    if (fileName == "--" || fileName == ".")
    {
        if (!f.open(IO_WriteOnly, stdout))
        {
            TJMH.errorMessage(i18n("Cannout open stdout"));
            return FALSE;
        }
    }
    else
    {
        f.setName(fileName);
        if (!f.open(IO_WriteOnly))
        {
            TJMH.errorMessage
                (QString(i18n("Cannot open report file %1!\n"))
                 .arg(fileName.latin1()));
            return FALSE;
        }
    }
    s.setDevice(&f);
    return TRUE;
}

bool 
Report::setTaskSorting(int sc, int level)
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
Report::setResourceSorting(int sc, int level)
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
Report::setAccountSorting(int sc, int level)
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
Report::isHidden(const CoreAttributes* c, ExpressionTree* et) const
{
    if (!taskRoot.isEmpty() && c->getType() == CA_Task &&
        taskRoot != c->getId().left(taskRoot.length()))
    {
        return TRUE;
    }
    
    if (!et)
        return FALSE;

    et->clearSymbolTable();
    QStringList flags = c->getFlagList();
    for (QStringList::Iterator it = flags.begin(); it != flags.end(); ++it)
        et->registerSymbol(*it, 1);
    return et->evalAsInt(c) != 0;
}

bool
Report::isRolledUp(const CoreAttributes* c, ExpressionTree* et) const
{
    if (!et)
        return FALSE;

    et->clearSymbolTable();
    QStringList flags = c->getFlagList();
    for (QStringList::Iterator it = flags.begin(); it != flags.end(); ++it)
        et->registerSymbol(*it, 1);
    return et->evalAsInt(c) != 0;
}

void
Report::setHideTask(ExpressionTree* et)
{
    delete hideTask;
    hideTask = et;
}

void
Report::setRollUpTask(ExpressionTree* et)
{
    delete rollUpTask;
    rollUpTask = et; 
}

void
Report::setHideResource(ExpressionTree* et)
{
    delete hideResource;
    hideResource = et; 
}

void
Report::setRollUpResource(ExpressionTree* et)
{
    delete rollUpResource;
    rollUpResource = et;
}

void
Report::setHideAccount(ExpressionTree* et)
{
    delete hideAccount;
    hideAccount = et;
}

void
Report::setRollUpAccount(ExpressionTree* et)
{
    delete rollUpAccount;
    rollUpAccount = et;
}

void
Report::filterTaskList(TaskList& filteredList, const Resource* r,
                       ExpressionTree* hideExp, ExpressionTree* rollUpExp)
const
{
    /* Create a new list that contains only those tasks that were not
     * hidden. */
    filteredList.clear();
    for (TaskListIterator tli(project->getTaskListIterator());
         *tli != 0; ++tli)
    {
        bool resourceLoadedInAnyScenario = FALSE;
        if (r != 0)
        {
            QValueList<int>::const_iterator it;
            for (it = scenarios.begin(); it != scenarios.end(); ++it)
                if (r->getLoad(*it, Interval(start, end), AllAccounts, *tli) 
                    > 0.0)
                {
                    resourceLoadedInAnyScenario = TRUE;
                    break;
                }
        }
        bool taskOverlapsInAnyScenario = FALSE;
        Interval iv(start, end);
        QValueList<int>::const_iterator it;
        for (it = scenarios.begin(); it != scenarios.end(); ++it)
            if (iv.overlaps(Interval((*tli)->getStart(*it),
                                 (*tli)->isMilestone() ?
                                 (*tli)->getStart(*it) :
                                 (*tli)->getEnd(*it))))
            {
                taskOverlapsInAnyScenario = TRUE;
                break;
            }
        if (!isHidden(*tli, hideExp) && taskOverlapsInAnyScenario &&
            (r == 0 || resourceLoadedInAnyScenario))
        {
            filteredList.append(tli);
        }
    }

    /* Now we have to remove all sub tasks of rolled-up tasks
     * from the filtered list */
    for (TaskListIterator tli(project->getTaskListIterator());
         *tli != 0; ++tli)
        if (isRolledUp(*tli, rollUpExp))
            for (TaskTreeIterator tti(*tli,
                                      TaskTreeIterator::parentAfterLeaves);
                 *tti != 0; ++tti)
                if (*tti != *tli)
                    filteredList.removeRef(*tti);
}

void
Report::sortTaskList(TaskList& filteredList)
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
Report::filterResourceList(ResourceList& filteredList, const Task* t,
                           ExpressionTree* hideExp, ExpressionTree* rollUpExp)
const
{
    /* Create a new list that contains only those resources that were
     * not hidden. */
    filteredList.clear();
    for (ResourceListIterator rli(project->getResourceListIterator());
         *rli != 0; ++rli)
    {
        bool taskLoadedInAnyScenario = FALSE;
        if (t != 0)
        {
            QValueList<int>::const_iterator it;
            for (it = scenarios.begin(); it != scenarios.end();
                 ++it)
                if ((*rli)->getLoad(*it, Interval(start, end),
                                    AllAccounts, t) > 0.0)
                {
                    taskLoadedInAnyScenario =
                        TRUE;
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
    for (ResourceListIterator rli(project->getResourceListIterator());
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
Report::sortResourceList(ResourceList& filteredList)
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
Report::filterAccountList(AccountList& filteredList, AccountType at,
                          ExpressionTree* hideExp, ExpressionTree* rollUpExp)
const
{
    /* Create a new list that contains only those accounts that were not
     * hidden. */
    filteredList.clear();
    for (AccountListIterator ali(project->getAccountListIterator()); 
         *ali != 0; ++ali)
    {
        if (!isHidden(*ali, hideExp) && (*ali)->getAcctType() == at)
            filteredList.append(*ali);
    }

    /* Now we have to remove all sub accounts of account in the roll-up list
     * from the filtered list */
    for (AccountListIterator ali(project->getAccountListIterator()); 
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
Report::sortAccountList(AccountList& filteredList)
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

bool
Report::setLoadUnit(const QString& u)
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
Report::errorMessage(const char* msg, ... )
{
    va_list ap;
    va_start(ap, msg);
    char buf[1024];
    vsnprintf(buf, 1024, msg, ap);
    va_end(ap);
        
    TJMH.errorMessage(buf, defFileName, defFileLine);
}

QString
Report::stripTaskRoot(QString taskId) const
{
    if (taskRoot == taskId.left(taskRoot.length()))
        return taskId.right(taskId.length() - taskRoot.length());
    else
        return taskId;
}

