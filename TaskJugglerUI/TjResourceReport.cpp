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

#include "TjResourceReport.h"

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
#include "QtResourceReport.h"
#include "QtResourceReportElement.h"
#include "ReportLayers.h"
#include "TjPrintResourceReport.h"
#include "TjGanttChart.h"

TjResourceReport::TjResourceReport(QWidget* p, ReportManager* m,
                                   Report* const rDef,
                                   const QString& n) : TjReport(p, m, rDef, n)
{
}

TjResourceReport::~TjResourceReport()
{
}

TjPrintReport*
TjResourceReport::newPrintReport(KPrinter* pr)
{
    return new TjPrintResourceReport(report, pr);
}

const QtReportElement*
TjResourceReport::getReportElement() const
{
    return reportElement;
}

bool
TjResourceReport::generateList()
{
    // Remove all items and columns from list view.
    listView->clear();
    ca2lviDict.clear();
    lvi2caDict.clear();

    while (listView->columns())
        listView->removeColumn(0);
    maxDepth = 0;

    if (!report)
        return false;

    // We need those values frequently. So let's store them in a more
    // accessible place.
    reportElement = dynamic_cast<QtResourceReportElement*>(
        dynamic_cast<QtResourceReport*>(report)->getTable());
    scenario = reportElement->getScenario(0);
    resourceList = report->getProject()->getResourceList();
    taskList = report->getProject()->getTaskList();

    if (!reportElement->filterResourceList
        (resourceList, 0, reportElement->getHideResource(), 0))
        return false;

    if (resourceList.isEmpty())
        return true;

    generateListHeader(i18n("Resource"), reportElement);

    /* The resource list need to be generated in two phases. First we insert
     * all resources and the nested tasks, and then we fill the rest of all
     * the lines. For some columns we need to know the maximum tree depth, so
     * we have to fill the table first with all entries before we can fill
     * those columns. */
    int i = 0;
    bool treeMode = reportElement->getResourceSorting(0) ==
        CoreAttributesList::TreeMode;
    for (ResourceListIterator rli(resourceList); *rli; ++rli)
    {
        /* Create a new item. If it has a parent, we need to look up the
         * parent LVI, so it get's inserted as child of that parent. */
        KListViewItem* newLvi;
        if ((*rli)->getParent() && treeMode &&
            resourceList.findRef((*rli)->getParent()) >= 0 &&
            ca2lviDict[QString("r:") + (*rli)->getParent()->getId()])
            newLvi = new KListViewItem
                (ca2lviDict[QString("r:") + (*rli)->getParent()->getId()],
                 (*rli)->getName());
        else
            newLvi = new KListViewItem(listView, (*rli)->getName());

        newLvi->setText(1, QString().sprintf("%07d", i++));

        // Store the new LVI into the dictionary and check if we have reached
        // a new tree level maximum.
        ca2lviDict[QString("r:") + (*rli)->getId()] = newLvi;
        lvi2caDict[QString().sprintf("%p", newLvi)] = *rli;

        if (treeLevel(newLvi) > maxDepth)
            maxDepth = treeLevel(newLvi);

        if ((*rli)->hasSubs())
        {
            // We have just inserted a resource group. So we need to check
            // whether the resource group's children should be shown or not.
            // The user can specify this in the report declaration.
            newLvi->setPixmap(0, KGlobal::iconLoader()->
                              loadIcon("tj_resource_group", KIcon::Small));
            if (reportElement->getRollUpResource())
            {
                if (!report->isRolledUp(*rli,
                                           reportElement->getRollUpResource()))
                    newLvi->setOpen(true);
                // Check if during the evaluation of the expression an error
                // had occured.
                if (reportElement->getRollUpResource()->getErrorFlag())
                    return false;
            }
            else
                newLvi->setOpen(true);
        }
        else
        {
            // We have just inserted an idividual resource.
            newLvi->setPixmap(0, KGlobal::iconLoader()->loadIcon
                              ("tj_resource", KIcon::Small));

            newLvi->setOpen(false);
            // Now we add all tasks that this resource is working on, if the
            // user did not want to hide this task.
            TaskList filteredTaskList = taskList;
            if (!reportElement->filterTaskList
                (filteredTaskList, *rli, reportElement->getHideTask(), 0))
                return false;
            reportElement->sortTaskList(filteredTaskList);

            for (TaskListIterator tli(filteredTaskList); *tli; ++tli)
            {
                /* We iterate through the filtered task list and check whether
                 * we need to insert the tasks in a tree or not. */
                int parentIdx = 0;
                QListViewItem* lvi;
                if ((*tli)->getParent() &&
                    reportElement->showTaskTree() &&
                    (parentIdx =
                     filteredTaskList.findRef((*tli)->getParent())) >= 0)
                {
                    // Find the corresponding parent LVI and insert the task
                    // as a child of it.
                    CoreAttributes* parent = filteredTaskList.at(parentIdx);
                    QListViewItem* parentLvi =
                        ca2lviDict[QString("t:") + (*rli)->getId() +
                        ":" + parent->getId()];
                    lvi = new KListViewItem(parentLvi, (*tli)->getName());
                }
                else
                {
                    // Insert the task as child of the resource.
                    lvi = new KListViewItem(newLvi, (*tli)->getName());
                }
                lvi->setText(1, QString().sprintf("%07d", i++));

                // The the proper icon for the task.
                lvi->setPixmap(0, KGlobal::iconLoader()->
                               loadIcon(((*tli)->isContainer() ?
                                         "tj_task_group" : "tj_task"),
                                        KIcon::Small));
                // Insert the task in the LVI lookup dictionary.
                ca2lviDict[QString("t:") + (*rli)->getId() +
                    ":" + (*tli)->getId()] = lvi;
                lvi2caDict[QString().sprintf("%p", lvi)] = *tli;
                lvi2ParentCaDict[QString().sprintf("%p", lvi)] = *rli;

                // Adjust the maxDepth setting if new treelevel maximum has
                // been found.
                if (treeLevel(lvi) > maxDepth)
                    maxDepth = treeLevel(lvi);
            }
        }
    }

    // Now we know the maximum tree depth and can fill in the rest of the
    // columns.
    for (ResourceListIterator rli(resourceList); *rli; ++rli)
    {
        generateResourceListLine
            (reportElement,
             *rli, ca2lviDict[QString("r:") + (*rli)->getId()]);
        if (!(*rli)->hasSubs())
        {
            for (TaskListIterator tli((*rli)->getTaskListIterator(scenario));
                 *tli; ++tli)
            {
                QListViewItem* lvi = ca2lviDict[QString("t:") +
                    (*rli)->getId() + ":" + (*tli)->getId()];
                // Some tasks may be hidden, so we have to ignore those.
                if (lvi)
                    generateTaskListLine(reportElement, *tli, lvi, *rli);
            }
        }
    }
    return true;
}

QString
TjResourceReport::generateStatusBarText(const QPoint& pos,
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
        Resource* r = dynamic_cast<Resource*>(parent);
        double load = t->getLoad(scenario, iv, r);
        double allocatedTimeLoad = t->getAllocatedTimeLoad(scenario, iv, r);
        text = i18n("%1(%2) - %3 -  Effort: %4  Allocated Time: %5  Task %6(%7)")
            .arg(r->getName())
            .arg(r->getId())
            .arg(ivName)
            .arg(reportElement->scaledLoad
                 (load, report->getNumberFormat(), true, false))
            .arg(reportElement->scaledLoad
                 (allocatedTimeLoad, report->getNumberFormat(), true, false))
            .arg(t->getName())
            .arg(t->getId());
    }
    else
    {
        Resource* r = dynamic_cast<Resource*>(ca);
        double load = r->getEffectiveLoad(scenario, iv, AllAccounts);
        double allocatedTimeLoad = r->getAllocatedTimeLoad
            (scenario, iv, AllAccounts);
        double freeLoad = r->getEffectiveFreeLoad(scenario, iv);
        double freeTimeLoad = r->getAvailableTimeLoad (scenario, iv);
        double totalLoad = load + freeLoad;
        double totalTimeLoad = allocatedTimeLoad + freeTimeLoad;
        text = i18n("%1(%2) - %3 -  Effort: %4 (%5%)  "
                    "Allocated Time: %6 (%7%)  Free: %8")
            .arg(r->getName())
            .arg(r->getId())
            .arg(ivName)
            .arg(reportElement->scaledLoad
                 (load, report->getNumberFormat(), true, false))
            .arg(totalLoad > 0.0 ? (int) ((load / totalLoad) * 100.0): 100)
            .arg(reportElement->scaledLoad
                 (allocatedTimeLoad, report->getNumberFormat(), true, false))
            .arg(totalTimeLoad > 0.0 ?
                 (int) ((allocatedTimeLoad / totalTimeLoad) * 100.0) : 100)
            .arg(reportElement->scaledLoad
                 (freeLoad, report->getNumberFormat(), true, false));
    }

    return text;
}

