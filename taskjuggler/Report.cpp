/*
 * Report.cpp - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#include "Report.h"

#include "TjMessageHandler.h"
#include "tjlib-internal.h"
#include "Project.h"
#include "Task.h"
#include "Resource.h"
#include "Account.h"
#include "ExpressionTree.h"
#include "TaskTreeIterator.h"
#include "ResourceTreeIterator.h"
#include "AccountTreeIterator.h"


Report::Report(Project* p, const QString& f, const QString& df, int dl) :
    project(p),
    fileName(f),
    defFileName(df),
    defFileLine(dl),
    f(),
    s(),
    scenarios(),
    weekStartsMonday(p->getWeekStartsMonday()),
    headline(),
    caption(),
    maxDepthTaskList(1),
    maxDepthResourceList(1),
    maxDepthAccountList(1),
    columns(),
    start(p->getStart()),
    end(p->getEnd()),
    timeFormat(p->getTimeFormat()),
    shortTimeFormat(p->getShortTimeFormat()),
    numberFormat(p->getNumberFormat()),
    currencyFormat(p->getCurrencyFormat()),
    taskSortCriteria(),
    resourceSortCriteria(),
    accountSortCriteria(),
    hideTask(0),
    hideResource(0),
    hideAccount(0),
    rollUpTask(0),
    rollUpResource(0),
    rollUpAccount(0),
    taskRoot()
{
    for (int i = 0; i < CoreAttributesList::maxSortingLevel; ++i)
    {
        taskSortCriteria[i] = CoreAttributesList::SequenceUp;
        resourceSortCriteria[i] = CoreAttributesList::SequenceUp;
        accountSortCriteria[i] = CoreAttributesList::SequenceUp;
    }

    showPIDs = false;
    loadUnit = days;
    timeStamp = true;
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
            return false;
        }
    }
    else
    {
        QString fullFileName = getFullFileName();
        f.setName(fullFileName);

        if (!f.open(IO_WriteOnly))
        {
            TJMH.errorMessage
                (QString(i18n("Cannot open report file %1!\n"))
                 .arg(fullFileName.latin1()));
            return false;
        }
    }
    s.setDevice(&f);
    return true;
}

QString
Report::getFullFileName() const
{
    QString fullFileName = fileName;
    // Check if the fileName is not an absolute file name.
    if (fileName[0] != '/')
    {
        // Prepend the path of the file where the report was defined, so
        // relative report file names are always interpreted relative to
        // their definition.
        QString path;
        if (defFileName[0] == '/')
            path = defFileName.left(defFileName.findRev('/', -1) + 1);
        fullFileName = path + fileName;
    }

    return fullFileName;
}

bool
Report::setTaskSorting(int sc, int level)
{
    if (level >= 0 && level < CoreAttributesList::maxSortingLevel)
    {
        if ((sc == CoreAttributesList::TreeMode && level > 0) ||
            !TaskList::isSupportedSortingCriteria(sc & 0xFFFF))
            return false;
        taskSortCriteria[level] = sc;
    }
    else
        return false;
    return true;
}

bool
Report::setResourceSorting(int sc, int level)
{
    if (level >= 0 && level < CoreAttributesList::maxSortingLevel)
    {
        if ((sc == CoreAttributesList::TreeMode && level > 0) ||
            !ResourceList::isSupportedSortingCriteria(sc & 0xFFFF))
            return false;
        resourceSortCriteria[level] = sc;
    }
    else
        return false;
    return true;
}

bool
Report::setAccountSorting(int sc, int level)
{
    if (level >= 0 && level < CoreAttributesList::maxSortingLevel)
    {
        if ((sc == CoreAttributesList::TreeMode && level > 0) ||
            !AccountList::isSupportedSortingCriteria(sc & 0xFFFF))
            return false;
        accountSortCriteria[level] = sc;
    }
    else
        return false;
    return true;
}

bool
Report::isHidden(const CoreAttributes* c, ExpressionTree* et) const
{
    if (!taskRoot.isEmpty() && c->getType() == CA_Task &&
        taskRoot != c->getId().left(taskRoot.length()))
    {
        return true;
    }

    if (!et)
        return false;

    et->clearSymbolTable();
    QStringList allFlags = project->getAllowedFlags();
    for (QStringList::Iterator ait = allFlags.begin(); ait != allFlags.end();
         ++ait)
    {
        bool found = false;
        QStringList flags = c->getFlagList();
        for (QStringList::Iterator it = flags.begin(); it != flags.end(); ++it)
            if (*it == *ait)
            {
                et->registerSymbol(*it, 1);
                found = true;
                break;
            }
        if (!found)
            et->registerSymbol(*ait, 0);
    }
    return et->evalAsInt(c) != 0;
}

bool
Report::isRolledUp(const CoreAttributes* c, ExpressionTree* et) const
{
    if (!et)
        return false;

    et->clearSymbolTable();
    QStringList allFlags = project->getAllowedFlags();
    for (QStringList::Iterator ait = allFlags.begin(); ait != allFlags.end();
         ++ait)
    {
        bool found = false;
        QStringList flags = c->getFlagList();
        for (QStringList::Iterator it = flags.begin(); it != flags.end(); ++it)
            if (*it == *ait)
            {
                et->registerSymbol(*it, 1);
                found = true;
                break;
            }
        if (!found)
            et->registerSymbol(*ait, 0);
    }
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

bool
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
        bool resourceLoadedInAnyScenario = false;
        if (r != 0)
        {
            QValueList<int>::const_iterator it;
            for (it = scenarios.begin(); it != scenarios.end(); ++it)
                if ((*tli)->isBookedResource(*it, r))
                {
                    resourceLoadedInAnyScenario = true;
                    break;
                }
        }
        bool taskOverlapsInAnyScenario = false;
        Interval iv(start, end);
        QValueList<int>::const_iterator it;
        for (it = scenarios.begin(); it != scenarios.end(); ++it)
            if (iv.overlaps(Interval((*tli)->getStart(*it),
                                 (*tli)->getEnd(*it) ==
                                 (*tli)->getStart(*it) - 1 ?
                                 (*tli)->getStart(*it) :
                                 (*tli)->getEnd(*it))))
            {
                taskOverlapsInAnyScenario = true;
                break;
            }
        if (taskOverlapsInAnyScenario &&
            (r == 0 || resourceLoadedInAnyScenario) &&
            !isHidden(*tli, hideExp))
        {
            filteredList.append(tli);
        }
        if (hideExp && hideExp->getErrorFlag())
            return false;
    }

    /* In tasktree sorting mode we need to make sure that we don't hide
     * parents of shown tasks. */
    TaskList list = filteredList;
    if (taskSortCriteria[0] == CoreAttributesList::TreeMode)
    {
        for (TaskListIterator tli(filteredList); *tli != 0; ++tli)
        {
            // Do not add the taskRoot task or any of it's parents.
            for (Task* p = (*tli)->getParent();
                 p != 0 && (p->getId() + "." != taskRoot);
                 p = p->getParent())
                if (list.containsRef(p) == 0)
                    list.append(p);
        }
    }
    filteredList = list;

    if (!rollUpExp)
        return true;

    /* Now we have to remove all sub tasks of rolled-up tasks
     * from the filtered list */
    for (TaskListIterator tli(project->getTaskListIterator());
         *tli != 0; ++tli)
    {
        if (isRolledUp(*tli, rollUpExp))
            for (TaskTreeIterator tti(*tli, parentAfterLeaves);
                 *tti != 0; ++tti)
                if (*tti != *tli)
                    filteredList.removeRef(*tti);
        if (rollUpExp && rollUpExp->getErrorFlag())
            return false;
    }

    return true;
}

void
Report::sortTaskList(TaskList& filteredList)
{
    /* The sorting can only honor the first scenario. Other scenarios are
     * ignort for the sorting. */
    filteredList.setSortScenario(scenarios[0]);

    for (int i = 0; i < CoreAttributesList::maxSortingLevel; i++)
        filteredList.setSorting(taskSortCriteria[i], i);
    filteredList.sort();
}

bool
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
        bool taskLoadedInAnyScenario = false;
        if (t != 0)
        {
            QValueList<int>::const_iterator it;
            for (it = scenarios.begin(); it != scenarios.end();
                 ++it)
                if ((*rli)->getLoad(*it, Interval(start, end),
                                    AllAccounts, t) > 0.0)
                {
                    taskLoadedInAnyScenario = true;
                    break;
                }
        }
        if (!isHidden(*rli, hideExp) &&
            (t == 0 || taskLoadedInAnyScenario))
        {
            filteredList.append(*rli);
        }
        if (hideExp && hideExp->getErrorFlag())
            return false;
    }

    /* In resourcetree sorting mode we need to make sure that we don't
     * hide parents of shown resources. */
    ResourceList list = filteredList;
    if (resourceSortCriteria[0] == CoreAttributesList::TreeMode)
    {
        for (ResourceListIterator rli(filteredList); *rli != 0; ++rli)
        {
            for (Resource* p = (*rli)->getParent(); p != 0; p = p->getParent())
                if (list.containsRef(p) == 0)
                    list.append(p);
        }
    }
    filteredList = list;

    if (!rollUpExp)
        return true;

    /* Now we have to remove all sub resources of resource in the
     * roll-up list from the filtered list */
    for (ResourceListIterator rli(project->getResourceListIterator());
         *rli != 0; ++rli)
    {
        if (isRolledUp(*rli, rollUpExp))
            for (ResourceTreeIterator rti(*rli, parentAfterLeaves);
                 *rti != 0; ++rti)
                if (*rti != *rli)
                    filteredList.removeRef(*rti);
        if (rollUpExp && rollUpExp->getErrorFlag())
            return false;
    }

    return true;
}

void
Report::sortResourceList(ResourceList& filteredList)
{
    for (int i = 0; i < CoreAttributesList::maxSortingLevel; i++)
        filteredList.setSorting(resourceSortCriteria[i], i);
    filteredList.sort();
}

bool
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
        if (hideExp && hideExp->getErrorFlag())
            return false;
    }

    /* In accounttree sorting mode we need to make sure that we don't hide
     * parents of shown accounts. */
    AccountList list = filteredList;
    if (accountSortCriteria[0] == CoreAttributesList::TreeMode)
    {
        for (AccountListIterator ali(filteredList); *ali != 0; ++ali)
        {
            for (Account* p = (*ali)->getParent(); p != 0; p = p->getParent())
                if (list.containsRef(p) == 0)
                    list.append(p);
        }
    }
    filteredList = list;

    if (!rollUpExp)
        return true;

    /* Now we have to remove all sub accounts of account in the roll-up list
     * from the filtered list */
    for (AccountListIterator ali(project->getAccountListIterator());
         *ali != 0; ++ali)
    {
        if (isRolledUp(*ali, rollUpExp))
            for (AccountTreeIterator ati(*ali, parentAfterLeaves);
                 *ati != 0; ++ati)
                if (*ati != *ali)
                    filteredList.removeRef(*ati);
        if (rollUpExp && rollUpExp->getErrorFlag())
            return false;
    }

    return true;
}

void
Report::sortAccountList(AccountList& filteredList)
{
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
        return false;

    return true;
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

