/*
 * The TaskJuggler Project Management Software
 *
 * Copyright (c) 2001, 2002, 2003, 2004, 2005 by Chris Schlaeger <cs@suse.de>
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

#include "Project.h"
#include "Task.h"
#include "Resource.h"
#include "ExpressionTree.h"
#include "QtResourceReport.h"
#include "QtResourceReportElement.h"
#include "ReportLayers.h"

TjResourceReport::TjResourceReport(QWidget* p, Report* const rDef,
                                   const QString& n) : TjReport(p, rDef, n)
{
}

TjResourceReport::~TjResourceReport()
{
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

    if (!reportDef)
        return FALSE;

    // We need those values frequently. So let's store them in a more
    // accessible place.
    reportElement =
        (dynamic_cast<QtResourceReport*>(reportDef))->getTable();
    scenario = reportElement->getScenario(0);
    resourceList = reportDef->getProject()->getResourceList();
    taskList = reportDef->getProject()->getTaskList();

    if (!reportElement->filterResourceList
        (resourceList, 0, reportElement->getHideResource(), 0))
        return FALSE;

    if (resourceList.isEmpty())
        return TRUE;

    generateListHeader(i18n("Resource"), reportElement);

    /* The resource list need to be generated in two phases. First we insert
     * all resources and the nested tasks, and then we fill the rest of all
     * the lines. For some columns we need to know the maximum tree depth, so
     * we have to fill the table first with all entries before we can fill
     * those columns. */
    int i = 0;
    for (ResourceListIterator rli(reportDef->getProject()->
                                  getResourceListIterator()); *rli; ++rli)
    {
        /* Create a new item. If it has a parent, we need to look up the
         * parent LVI, so it get's inserted as child of that parent. */
        KListViewItem* newLvi;
        if ((*rli)->getParent())
            newLvi = new KListViewItem
                (ca2lviDict[QString("r:") + (*rli)->getParent()->getFullId()],
                 (*rli)->getName());
        else
            newLvi = new KListViewItem(listView, (*rli)->getName());

        // Store the new LVI into the dictionary and check if we have reached
        // a new tree level maximum.
        ca2lviDict.insert(QString("r:") + (*rli)->getFullId(), newLvi);
        lvi2caDict.insert(QString().sprintf("%p", newLvi), *rli);

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
                if (!reportDef->isRolledUp(*rli,
                                           reportElement->getRollUpResource()))
                    newLvi->setOpen(TRUE);
                // Check if during the evaluation of the expression an error
                // had occured.
                if (reportElement->getRollUpResource()->getErrorFlag())
                    return FALSE;
            }
            else
                newLvi->setOpen(TRUE);
        }
        else
        {
            // We have just inserted an idividual resource.
            newLvi->setPixmap(0, KGlobal::iconLoader()->loadIcon
                              ("tj_resource", KIcon::Small));

            newLvi->setOpen(FALSE);
            // Now we add all tasks that this resource is working on, if the
            // user did not want to hide this task.
            TaskList filteredTaskList = taskList;
            if (!reportElement->filterTaskList
                (filteredTaskList, *rli, reportElement->getHideTask(), 0))
                return FALSE;
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
                        ca2lviDict[QString("t:") + (*rli)->getFullId() +
                        ":" + parent->getId()];
                    lvi = new KListViewItem(parentLvi, (*tli)->getName());
                }
                else
                {
                    // Insert the task as child of the resource.
                    lvi = new KListViewItem(newLvi, (*tli)->getName());
                }
                // The the proper icon for the task.
                lvi->setPixmap(0, KGlobal::iconLoader()->
                               loadIcon(((*tli)->isContainer() ?
                                         "tj_task_group" : "tj_task"),
                                        KIcon::Small));
                // Insert the task in the LVI lookup dictionary.
                ca2lviDict.insert(QString("t:") + (*rli)->getFullId() +
                                  ":" + (*tli)->getId(), lvi);
                lvi2caDict.insert(QString().sprintf("%p", lvi), *tli);
                lvi2ParentCaDict.insert(QString().sprintf("%p", lvi), *rli);

                // Adjust the maxDepth setting if new treelevel maximum has
                // been found.
                if (treeLevel(lvi) > maxDepth)
                    maxDepth = treeLevel(lvi);
            }
        }
        newLvi->setText(1, QString().sprintf("%05d", i++));
    }

    // Now we know the maximum tree depth and can fill in the rest of the
    // columns.
    for (ResourceListIterator rli(reportDef->getProject()->
                                  getResourceListIterator()); *rli; ++rli)
    {
        generateResourceListLine
            (reportElement,
             *rli, ca2lviDict[QString("r:") + (*rli)->getFullId()]);
        if (!(*rli)->hasSubs())
        {
            for (TaskListIterator tli((*rli)->getTaskListIterator(scenario));
                 *tli; ++tli)
            {
                generateTaskListLine(reportElement,
                                     *tli, ca2lviDict[QString("t:") +
                                     (*rli)->getFullId() + ":" +
                                     (*tli)->getId()], *rli);
            }
        }
    }
    return TRUE;
}

bool
TjResourceReport::generateChart(bool autoFit)
{
    if (!listView->firstChild())
        return TRUE;

    setCursor(KCursor::waitCursor());
    prepareChart(autoFit, reportElement);

    if (!generateChartLoadBars())
    {
        setCursor(KCursor::arrowCursor());
        return FALSE;
    }

    ganttHeader->update();
    ganttChart->update();

    setCursor(KCursor::arrowCursor());
    return TRUE;
}

QString
TjResourceReport::generateStatusBarText(const QPoint& pos,
                                        const CoreAttributes* ca,
                                        const CoreAttributes* parent) const
{
    QPoint chartPos = ganttChartView->viewportToContents(pos);
    time_t refTime = x2time(chartPos.x());
    Interval iv = stepInterval(refTime);
    QString ivName = stepIntervalName(refTime);

    QString text;
    if (ca->getType() == CA_Task)
    {
        const Task* t = dynamic_cast<const Task*>(ca);
        const Resource* r = dynamic_cast<const Resource*>(parent);
        double load = t->getLoad(scenario, iv, r);
        text = i18n("%1(%2) - %3:  Load=%4  Task %5(%6)")
            .arg(r->getName())
            .arg(r->getFullId())
            .arg(ivName)
            .arg(reportElement->scaledLoad
                 (load, reportDef->getNumberFormat()))
            .arg(t->getName())
            .arg(t->getId());
    }
    else
    {
        const Resource* r = dynamic_cast<const Resource*>(ca);
        double load = r->getLoad(scenario, iv, AllAccounts);
        double freeLoad = r->getAvailableWorkLoad(scenario, iv);
        text = i18n("%1(%2) - %3:  Load=%4  Free=%5")
            .arg(r->getName())
            .arg(r->getFullId())
            .arg(ivName)
            .arg(reportElement->scaledLoad
                 (load, reportDef->getNumberFormat()))
            .arg(reportElement->scaledLoad
                 (freeLoad, reportDef->getNumberFormat()));
    }

    return text;
}

bool
TjResourceReport::generateChartLoadBars()
{
    for (ResourceListIterator rli(resourceList); *rli; ++rli)
    {
        QListViewItem* lvi = getResourceListEntry(*rli);
        if (lvi)
        {
            drawResource(*rli, lvi->itemPos());
            if (lvi->isOpen())
                if (!drawResourceTasks(*rli))
                    return FALSE;
        }
    }

    return TRUE;
}

void
TjResourceReport::drawResource(const Resource* r, int y)
{
    time_t start = reportElement->getStart();
    time_t end = reportElement->getEnd();

    switch (stepUnit)
    {
        case hour:
            for (time_t i = beginOfHour(start); i <= end;
                 i = hoursLater(1, i))
                drawResourceLoadColumn(r, i, hoursLater(1, i), y);
            break;
        case day:
            for (time_t i = midnight(start); i <= end;
                 i = sameTimeNextDay(i))
                drawResourceLoadColumn(r, i, sameTimeNextDay(i), y);
            break;
        case week:
            for (time_t i = beginOfWeek(start,
                                        r->getProject()->getWeekStartsMonday());
                 i <= (end); i = sameTimeNextWeek(i))
                drawResourceLoadColumn(r, i, sameTimeNextWeek(i), y);
            break;
        case month:
            for (time_t i = beginOfMonth(start); i <= end;
                 i = sameTimeNextMonth(i))
                drawResourceLoadColumn(r, i, sameTimeNextMonth(i), y);
            break;
        case quarter:
            for (time_t i = beginOfQuarter(start); i <= end;
                 i = sameTimeNextQuarter(i))
                drawResourceLoadColumn(r, i, sameTimeNextQuarter(i), y);
            break;
        case year:
            for (time_t i = beginOfYear(start); i <= end;
                 i = sameTimeNextYear(i))
                drawResourceLoadColumn(r, i, sameTimeNextYear(i), y);
            break;
        default:
            kdError() << "Unknown stepUnit";
            break;
    }
}

void
TjResourceReport::drawResourceLoadColumn(const Resource* r, time_t start,
                                         time_t end, int rY)
{
    // Determin the width of the cell that we draw the column in.
    int cellStart = time2x(start);
    int cellEnd = time2x(end);

    // We will draw the load column into the cell with a margin of 1 pixel
    // plus the cell seperation line.
    // Let's try a first shot for column start and width.
    int cx = cellStart + 2;
    int cw = cellEnd - cellStart - 3;

    // Now we are calculation the load of the resource with respect to this
    // task, to all tasks, and we calculate the not yet allocated load.
    Interval period(start, end);
    double freeLoad = r->getAvailableWorkLoad(scenario, period);
    double load = r->getLoad(scenario, period, AllAccounts);
    double maxLoad = load + freeLoad;
    if (maxLoad <= 0.0)
        return;

    // Transform the load values into colum Y coordinates.
    int colBottom = rY + itemHeight - 1;
    int colTop = rY + 1;
    int colLoadTop = colBottom - (int) ((colBottom - colTop) *
                                        (load / maxLoad));

    // Just some interim variables so we can change the color with only a
    // single change.
    QColor loadCol = QColor("#FD13C6");
    QColor freeLoadCol = QColor("#C4E00E");
    QBrush loadBrush;
    QBrush freeLoadBrush;
    if (r->hasSubs())
    {
        loadBrush = QBrush(loadCol, Dense6Pattern);
        freeLoadBrush = QBrush(freeLoadCol, Dense6Pattern);
    }
    else
    {
        loadBrush = QBrush(loadCol, Dense4Pattern);
        freeLoadBrush = QBrush(freeLoadCol, Dense4Pattern);
    }

    // Now we draw the columns. But only if the load is larger than 0.
    if (load > 0.0)
    {
        // Load for this task.
        QCanvasRectangle* rect = new QCanvasRectangle
            (cx, colLoadTop, cw, colBottom - colLoadTop, ganttChart);
        rect->setBrush(loadBrush);
        rect->setPen(loadCol);
        rect->setZ(TJRL_LOADBARS);
        rect->show();
    }

    if (freeLoad > 0.0)
    {
        QCanvasRectangle* rect = new QCanvasRectangle
            (cx, colTop, cw, colLoadTop - colTop,
             ganttChart);
        rect->setBrush(freeLoadBrush);
        rect->setPen(freeLoadCol);
        rect->setZ(TJRL_LOADBARS);
        rect->show();
    }
}

bool
TjResourceReport::drawResourceTasks(const Resource* r)
{
    // Now we draw all tasks that this resource is working on, if the
    // user did not want to hide the task.
    TaskList filteredTaskList = taskList;
    if (!reportElement->filterTaskList
        (filteredTaskList, r, reportElement->getHideTask(), 0))
        return FALSE;
    reportElement->sortTaskList(filteredTaskList);
    for (TaskListIterator tli(filteredTaskList); *tli; ++tli)
    {
        /* We iterate through the filtered task list and check whether
         * we need to insert the tasks in a tree or not. */
        QListViewItem* lvi = getTaskListEntry(*tli, r);
        if (!lvi)
            continue;

        int y = lvi->itemPos();
        drawTaskOutline((*tli), y);

        if ((*tli)->hasSubs() && lvi->isOpen())
            continue;

        time_t start = reportElement->getStart();
        time_t end = reportElement->getEnd();

        switch (stepUnit)
        {
            case hour:
                for (time_t i = beginOfHour(start); i <= end;
                     i = hoursLater(1, i))
                    drawTaskLoadColumn(*tli, r, i, hoursLater(1, i), y);
                break;
            case day:
                for (time_t i = midnight(start); i <= end;
                     i = sameTimeNextDay(i))
                    drawTaskLoadColumn(*tli, r, i, sameTimeNextDay(i), y);
                break;
            case week:
                for (time_t i = beginOfWeek
                     (start, r->getProject()->getWeekStartsMonday());
                     i <= (end); i = sameTimeNextWeek(i))
                    drawTaskLoadColumn(*tli, r, i, sameTimeNextWeek(i), y);
                break;
            case month:
                for (time_t i = beginOfMonth(start); i <= end;
                     i = sameTimeNextMonth(i))
                    drawTaskLoadColumn(*tli, r, i, sameTimeNextMonth(i), y);
                break;
            case quarter:
                for (time_t i = beginOfQuarter(start); i <= end;
                     i = sameTimeNextQuarter(i))
                    drawTaskLoadColumn(*tli, r, i, sameTimeNextQuarter(i), y);
                break;
            case year:
                for (time_t i = beginOfYear(start); i <= end;
                     i = sameTimeNextYear(i))
                    drawTaskLoadColumn(*tli, r, i, sameTimeNextYear(i), y);
                break;
            default:
                kdError() << "Unknown stepUnit";
                break;
        }
    }

    return TRUE;
}

void
TjResourceReport::drawTaskLoadColumn(const Task* t, const Resource* r,
                                     time_t start, time_t end, int rY)
{
    // Determin the width of the cell that we draw the column in.
    int cellStart = time2x(start);
    int cellEnd = time2x(end);

    // We will draw the load column into the cell with a margin of 1 pixel
    // plus the cell seperation line.
    // Let's try a first shot for column start and width.
    int cx = cellStart + 2;
    int cw = cellEnd - cellStart - 3;

    // Now we are calculation the load of the resource with respect to this
    // task, to all tasks, and we calculate the not yet allocated load.
    Interval period(start, end);
    double resourceLoad = t->getLoad(scenario, period, r);
    double maxLoad = r->getLoad(scenario, period) +
        r->getAvailableWorkLoad(scenario, period);
    if (maxLoad <= 0.0)
        return;

    // Transform the load values into colum Y coordinates.
    int colBottom = rY + itemHeight - 1;
    int colTop = rY + 1;
    int colResourceLoadTop = colBottom - (int) ((colBottom - colTop) *
                                                (resourceLoad / maxLoad));

    // Just some interim variables so we can change the color with only a
    // single change.
    QColor resourceLoadCol = QColor("#4C5EFF");
    QBrush resourceLoadBrush = QBrush(resourceLoadCol, Dense4Pattern);

    // Now we draw the columns. But only if the load is larger than 0.
    if (resourceLoad > 0.0)
    {
        // Load for this task.
        QCanvasRectangle* rect = new QCanvasRectangle
            (cx, colResourceLoadTop, cw, colBottom - colResourceLoadTop,
             ganttChart);
        rect->setBrush(resourceLoadBrush);
        rect->setPen(resourceLoadCol);
        rect->setZ(TJRL_LOADBARS);
        rect->show();
    }
}

void
TjResourceReport::drawTaskOutline(const Task* t, int y)
{
    if (t->isContainer())
    {
        // A black bar with jag at both ends.
        QPointArray a(9);
        int start = time2x(t->getStart(scenario));
        int end = time2x(t->getEnd(scenario));
        int top = y + 4;
        int bottom = y + itemHeight - 7;
        int halfbottom = y + itemHeight / 2 - 1;
        int jagWidth = 4;
        a.setPoint(0, start - jagWidth, top);
        a.setPoint(1, start - jagWidth, halfbottom);
        a.setPoint(2, start, bottom);
        a.setPoint(3, start + jagWidth, halfbottom);
        a.setPoint(4, end - jagWidth, halfbottom);
        a.setPoint(5, end, bottom);
        a.setPoint(6, end + jagWidth, halfbottom);
        a.setPoint(7, end + jagWidth, top);
        a.setPoint(8, start - jagWidth, top);

        for (uint i = 0; i < a.count() - 1; ++i)
        {
            QCanvasLine* line = new QCanvasLine(ganttChart);
            QPen pen(Qt::black);
            line->setPen(pen);
            int x1, y1, x2, y2;
            a.point(i, &x1, &y1);
            a.point(i + 1, &x2, &y2);
            line->setPoints(x1, y1, x2, y2);
            line->setZ(TJRL_TASKOUTLINE);
            line->show();
        }
    }
    else
    {
        // A black box outline with a progress bar outline.
        QCanvasRectangle* rect =
            new QCanvasRectangle(time2x(t->getStart(scenario)), y + 4,
                                 time2x(t->getEnd(scenario)) -
                                 time2x(t->getStart(scenario)),
                                 itemHeight - 8, ganttChart);

        rect->setPen(Qt::black);
        rect->setBrush(QBrush(NoBrush));
        rect->setZ(TJRL_TASKOUTLINE);
        rect->show();
    }
}

QListViewItem*
TjResourceReport::getResourceListEntry(const Resource* r)
{
    /* Returns the QListViewItem pointer for the task if the task is shown in
     * the list view. Tasks that have closed parents are not considered to be
     * visible even though they are part of the list view. Offscreen tasks
     * are considered visible if they meet the above condition. */

    // Check that the task is in the list. Colum 1 contains the task ID.
    QListViewItem* lvi = ca2lviDict[QString("r:") + r->getFullId()];
    if (!lvi)
        return 0;

    // Now make sure that all parents are open.
    for (QListViewItem* i = lvi; i; i = i->parent())
        if (i->parent() && !i->parent()->isOpen())
            return 0;

    return lvi;
}

QListViewItem*
TjResourceReport::getTaskListEntry(const Task* t, const Resource* r)
{
    /* Returns the QListViewItem pointer for the task if the task is shown in
     * the list view. Tasks that have closed parents are not considered to be
     * visible even though they are part of the list view. Offscreen tasks
     * are considered visible if they meet the above condition. */

    // Check that the task is in the list. Colum 1 contains the task ID.
    QListViewItem* lvi = ca2lviDict[QString("t:") + r->getFullId() + ":" +
        t->getId()];
    if (!lvi)
        return 0;

    // Now make sure that all parents are open.
    for (QListViewItem* i = lvi; i; i = i->parent())
        if (i->parent() && !i->parent()->isOpen())
            return 0;

    return lvi;
}

