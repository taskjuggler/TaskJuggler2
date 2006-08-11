/*
 * The TaskJuggler Project Management Software
 *
 * Copyright (c) 2001, 2002, 2003, 2004, 2005 by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#include "TjTaskReport.h"

#include <qdict.h>
#include <qcanvas.h>

#include <klistview.h>
#include <klocale.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kdebug.h>
#include <kcursor.h>
#include <kmessagebox.h>

#include "Project.h"
#include "Task.h"
#include "Resource.h"
#include "ExpressionTree.h"
#include "QtTaskReport.h"
#include "QtTaskReportElement.h"
#include "ReportLayers.h"
#include "TjPrintTaskReport.h"
#include "TjGanttChart.h"

TjTaskReport::TjTaskReport(QWidget* p, ReportManager* m, Report* const rDef,
                           const QString& n) : TjReport(p, m, rDef, n)
{
}

TjTaskReport::~TjTaskReport()
{
}

TjPrintReport*
TjTaskReport::newPrintReport(KPrinter* pr)
{
    return new TjPrintTaskReport(reportDef, pr);
}

const QtReportElement*
TjTaskReport::getReportElement() const
{
    return reportElement;
}

bool
TjTaskReport::generateList()
{
    // Remove all items and columns from list view.
    listView->clear();
    ca2lviDict.clear();
    lvi2caDict.clear();
    lvi2ParentCaDict.clear();

    while (listView->columns())
        listView->removeColumn(0);
    maxDepth = 0;

    if (!reportDef)
        return FALSE;

    // We need those values frequently. So let's store them in a more
    // accessible place.
    reportElement =
        (dynamic_cast<QtTaskReport*>(reportDef))->getTable();
    scenario = reportElement->getScenario(0);
    taskList = reportDef->getProject()->getTaskList();

    // QListView can hide subtasks. So we feed the list with all tasks first
    // and then later close those items that we want to roll up. This
    // expression means "roll-up none".
    ExpressionTree* et = new ExpressionTree;
    et->setTree("0", reportDef->getProject());

    if (!reportElement->filterTaskList(taskList, 0,
                                       reportElement->getHideTask(), et))
    {
        delete et;
        return FALSE;
    }
    delete et;

    reportElement->sortTaskList(taskList);

    if (taskList.isEmpty())
        return TRUE;

    generateListHeader(i18n("Task"), reportElement);

    /* The task list need to be generated in two phases. First we insert all
     * tasks and the nested resources, and then we fill the rest of all the
     * lines. For some columns we need to know the maximum tree depth, so we
     * have to fill the table first with all entries before we can fill those
     * columns. */
    int i = 0;
    bool treeMode = reportElement->getTaskSorting(0) ==
        CoreAttributesList::TreeMode;
    for (TaskListIterator tli(taskList); *tli; ++tli)
    {
        KListViewItem* newLvi;
        if ((*tli)->getParent() && treeMode &&
            taskList.findRef((*tli)->getParent()) >= 0 &&
            ca2lviDict[QString("t:") + (*tli)->getParent()->getId()] &&
            (*tli)->getParent()->getId().length() >
            reportElement->getTaskRoot().length())
        {
            newLvi = new KListViewItem
                (ca2lviDict[QString("t:") + (*tli)->getParent()->getId()],
                 (*tli)->getName());
        }
        else
            newLvi = new KListViewItem(listView, (*tli)->getName());

        ca2lviDict[QString("t:") + (*tli)->getId()] = newLvi;
        lvi2caDict[QString().sprintf("%p", newLvi)] = *tli;

        if (treeLevel(newLvi) > maxDepth)
            maxDepth = treeLevel(newLvi);

        if ((*tli)->isContainer())
        {
            newLvi->setPixmap(0, KGlobal::iconLoader()->
                              loadIcon("tj_task_group", KIcon::Small));
            if (reportElement->getRollUpTask())
            {
                if (!reportDef->isRolledUp(*tli,
                                           reportElement->getRollUpTask()))
                    newLvi->setOpen(TRUE);
                if (reportElement->getRollUpTask()->getErrorFlag())
                    return FALSE;
            }
            else
                newLvi->setOpen(TRUE);
        }
        else if ((*tli)->isMilestone())
        {
            newLvi->setPixmap(0, KGlobal::iconLoader()->loadIcon("tj_milestone",
                                                                 KIcon::Small));
        }
        else
        {
            newLvi->setPixmap(0, KGlobal::iconLoader()->loadIcon("tj_task",
                                                                 KIcon::Small));
            for (ResourceListIterator rli((*tli)->
                                          getBookedResourcesIterator(scenario));
                 *rli; ++rli)
            {
                QListViewItem* lvi =
                    new KListViewItem(newLvi, (*rli)->getName());
                lvi->setPixmap(0, KGlobal::iconLoader()->
                               loadIcon("tj_resource", KIcon::Small));
                ca2lviDict[QString("r:") + (*tli)->getId() +
                    ":" + (*rli)->getId()] = lvi;
                lvi2caDict[QString().sprintf("%p", lvi)] = *rli;
                lvi2ParentCaDict[QString().sprintf("%p", lvi)] = *tli;
                if (treeLevel(lvi) > maxDepth)
                    maxDepth = treeLevel(lvi);
            }
        }
        newLvi->setText(1, QString().sprintf("%05d", i++));
    }

    // Now we know the maximum tree depth and can fill in the rest of the
    // columns.
    for (TaskListIterator tli(taskList); *tli; ++tli)
    {
        QListViewItem* lvi = ca2lviDict[QString("t:") + (*tli)->getId()];
        generateTaskListLine(reportElement, *tli, lvi);
        if (!(*tli)->isContainer() && !(*tli)->isMilestone())
        {
            for (ResourceListIterator rli((*tli)->
                                          getBookedResourcesIterator(scenario));
                 *rli; ++rli)
            {
                QListViewItem* lvi = ca2lviDict[QString("r:") +
                    (*tli)->getId() + ":" + (*rli)->getId()];
                // Make sure that the resource is not hidden.
                if (lvi)
                    generateResourceListLine(reportElement, *rli, lvi, *tli);
            }
        }
    }

    return TRUE;
}

QString
TjTaskReport::generateStatusBarText(const QPoint& pos,
                                    CoreAttributes* ca,
                                    CoreAttributes* parent)
{
    QPoint chartPos = ganttChartView->viewportToContents(pos);
    time_t refTime = ganttChart->x2time(chartPos.x());
    Interval iv = ganttChart->stepInterval(refTime);
    QString ivName = ganttChart->stepIntervalName(refTime);

    QString text;
    if (ca->getType() == CA_Task)
    {
        Task* t = dynamic_cast<Task*>(ca);
        double load = t->getLoad(scenario, iv);
        double allocatedTimeLoad = t->getAllocatedTimeLoad(scenario, iv);
        text = i18n("%1(%2) - %3:  Effort=%4  Allocated Time=%5")
            .arg(t->getName())
            .arg(t->getId())
            .arg(ivName)
            .arg(reportElement->scaledLoad
                 (load, reportDef->getNumberFormat(), true, false))
            .arg(reportElement->scaledLoad
                 (allocatedTimeLoad, reportDef->getNumberFormat(), true,
                  false));
    }
    else
    {
        Resource* r = dynamic_cast<Resource*>(ca);
        Task* t = dynamic_cast<Task*>(parent);
        double load = r->getLoad(scenario, iv, AllAccounts, t);
        double allocatedTimeLoad = r->getAllocatedTimeLoad
            (scenario, iv, AllAccounts, t);
        text = i18n("%1(%2) - %3:  Effort=%4  Allocated Time=%5  %6(%7)")
            .arg(r->getName())
            .arg(r->getId())
            .arg(ivName)
            .arg(reportElement->scaledLoad
                 (load, reportDef->getNumberFormat(), true, false))
            .arg(reportElement->scaledLoad
                 (allocatedTimeLoad, reportDef->getNumberFormat(), true, false))
            .arg(t->getName())
            .arg(t->getId());
    }

    return text;
}

QListViewItem*
TjTaskReport::getTaskListEntry(const Task* t)
{
    /* Returns the QListViewItem pointer for the task if the task is shown in
     * the list view. Tasks that have closed parents are not considered to be
     * visible even though they are part of the list view. Offscreen tasks
     * are considered visible if they meet the above condition. */

    // Check that the task is in the list. Colum 1 contains the task ID.
    QListViewItem* lvi = ca2lviDict[QString("t:") + t->getId()];
    if (!lvi)
        return 0;

    // Now make sure that all parents are open.
    for (QListViewItem* i = lvi; i; i = i->parent())
        if (i->parent() && !i->parent()->isOpen())
            return 0;

    return lvi;
}

