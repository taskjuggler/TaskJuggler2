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

#include "TjTaskReport.h"

#include <qdict.h>
#include <qcanvas.h>

#include <klistview.h>
#include <klocale.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kdebug.h>

#include "Project.h"
#include "Task.h"
#include "Resource.h"
#include "ExpressionTree.h"
#include "QtTaskReport.h"
#include "QtTaskReportElement.h"

TjTaskReport::TjTaskReport(QWidget* p, Report* const rDef,
                           const QString& n) : TjReport(p, rDef, n)
{
}

TjTaskReport::~TjTaskReport()
{
}

bool
TjTaskReport::generateList()
{
    // Remove all items and columns from list view.
    listView->clear();
    ca2lviDict.clear();
    while (listView->columns())
        listView->removeColumn(0);

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
        return FALSE;

    if (taskList.isEmpty())
        return TRUE;

    generateListHeader(i18n("Task"), reportElement);

    /* The task list need to be generated in two phases. First we insert all
     * tasks and the nested resources, and then we fill the rest of all the
     * lines. For some columns we need to know the maximum tree depth, so we
     * have to fill the table first with all entries before we can fill those
     * columns. */
    for (TaskListIterator tli(reportDef->getProject()->
                              getTaskListIterator()); *tli; ++tli)
    {
        KListViewItem* newLvi;
        if ((*tli)->getParent())
            newLvi = new KListViewItem
                (ca2lviDict[QString("t:") + (*tli)->getParent()->getId()],
                 (*tli)->getName());
        else
            newLvi = new KListViewItem(listView, (*tli)->getName());

        ca2lviDict.insert(QString("t:") + (*tli)->getId(), newLvi);
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
                ca2lviDict.insert(QString("r:") + (*tli)->getId() +
                                  ":" + (*rli)->getFullId(), lvi);
                if (treeLevel(lvi) > maxDepth)
                    maxDepth = treeLevel(lvi);
            }
        }
    }

    // Now we know the maximum tree depth and can fill in the rest of the
    // columns.
    for (TaskListIterator tli(reportDef->getProject()->
                              getTaskListIterator()); *tli; ++tli)
    {
        generateListLine(*tli, ca2lviDict[QString("t:") + (*tli)->getId()]);
        if (!(*tli)->isContainer() && !(*tli)->isMilestone())
        {
            for (ResourceListIterator rli((*tli)->
                                          getBookedResourcesIterator(scenario));
                 *rli; ++rli)
            {
                generateResourceListLine(*rli, ca2lviDict[QString("r:") +
                                         (*tli)->getId() + ":" +
                                         (*rli)->getFullId()], *tli);
            }
        }
    }

    return TRUE;
}

bool
TjTaskReport::generateChart(bool autoFit)
{
    prepareChart(autoFit, reportElement);

    generateGanttTasks();

    ganttHeader->update();
    ganttChart->update();

    return TRUE;
}

void
TjTaskReport::generateGanttTasks()
{
    for (TaskListIterator tli(taskList); *tli; ++tli)
    {
        QListViewItem* lvi = getTaskListEntry(*tli);
        if (lvi)
        {
            drawTask(*tli, lvi->itemPos());
            drawDependencies(*tli, lvi);
            if (lvi->isOpen())
                drawTaskResources(*tli);
        }
    }
}

void
TjTaskReport::generateListLine(Task* t, QListViewItem* lvi)
{
    // Skip the first colum. It contains the hardwired task name.
    int column = 1;
    for (QPtrListIterator<TableColumnInfo>
         ci = reportElement->getColumnsIterator(); *ci; ++ci, ++column)
    {
        QString cellText;
        const TableColumnFormat* tcf =
            reportElement->getColumnFormat((*ci)->getName());

        if ((*ci)->getName() == "completed")
        {
            if (t->getCompletionDegree(scenario) ==
                t->getCalcedCompletionDegree(scenario))
            {
                cellText = QString("%1%")
                    .arg((int) t->getCompletionDegree(scenario));
            }
            else
            {
                cellText = QString("%1% (%2%)")
                    .arg((int) t->getCompletionDegree(scenario))
                    .arg((int) t->getCalcedCompletionDegree(scenario));
            }
        }
        else if ((*ci)->getName() == "cost")
        {
        }
        else if ((*ci)->getName() == "criticalness")
        {
        }
        else if ((*ci)->getName() == "duration")
            cellText = reportElement->scaledLoad
                (t->getCalcDuration(scenario), tcf);
        else if ((*ci)->getName() == "effort")
        {
            double val = 0.0;
            val = t->getLoad(scenario, Interval(t->getStart(scenario),
                                                t->getEnd(scenario)), 0);
            cellText = indent(reportElement->scaledLoad(val, tcf), lvi,
                              tcf->getHAlign() == TableColumnFormat::right);
        }
        else if ((*ci)->getName() == "end")
            cellText = time2user((t->isMilestone() ? 1 : 0) +
                                 t->getEnd(scenario),
                                 reportDef->getTimeFormat());
        else if ((*ci)->getName() == "note")
        {
        }
        else if ((*ci)->getName() == "pathcriticalness")
        {
        }
        else if ((*ci)->getName() == "priority")
        {
        }
        else if ((*ci)->getName() == "profit")
        {
        }
        else if ((*ci)->getName() == "responsible")
        {
        }
        else if ((*ci)->getName() == "revenue")
        {
        }
        else if ((*ci)->getName() == "start")
            cellText = time2user(t->getStart(scenario),
                                 reportDef->getTimeFormat());
        else if ((*ci)->getName() == "status")
        {
        }

        lvi->setText(column, cellText);
    }
}

void
TjTaskReport::generateResourceListLine(Resource* r, QListViewItem* lvi,
                                           Task* t)
{
    // Skip the first colum. It contains the hardwired resource name.
    int column = 1;
    for (QPtrListIterator<TableColumnInfo>
         ci = reportElement->getColumnsIterator(); *ci; ++ci, ++column)
    {
        QString cellText;
        const TableColumnFormat* tcf =
            reportElement->getColumnFormat((*ci)->getName());

        if ((*ci)->getName() == "effort")
        {
            double val = 0.0;
            val = r->getLoad(scenario, Interval(t->getStart(scenario),
                                                t->getEnd(scenario)),
                             AllAccounts, t);
            cellText = indent(reportElement->scaledLoad(val, tcf), lvi,
                              tcf->getHAlign() == TableColumnFormat::right);
        }

        lvi->setText(column, cellText);
    }
}

void
TjTaskReport::drawTask(Task* const t, int y)
{
    if (t->isMilestone())
    {
        // A black diamond.
        QPointArray a(5);
        int centerX = time2x(t->getStart(scenario));
        int centerY = y + itemHeight / 2;
        int radius = (itemHeight - 8) / 2;
        a.setPoint(0, centerX, centerY - radius);
        a.setPoint(1, centerX + radius, centerY);
        a.setPoint(2, centerX, centerY + radius);
        a.setPoint(3, centerX - radius, centerY);
        a.setPoint(4, centerX, centerY - radius);

        QCanvasPolygon* polygon = new QCanvasPolygon(ganttChart);
        polygon->setPoints(a);
        polygon->setPen(QColor("#000000"));
        polygon->setBrush(QColor("#000000"));
        polygon->setZ(10);
        polygon->show();
    }
    else if (t->isContainer())
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

        QCanvasPolygon* polygon = new QCanvasPolygon(ganttChart);
        polygon->setPoints(a);
        polygon->setPen(QColor("#000000"));
        polygon->setBrush(QColor("#000000"));
        polygon->setZ(10);
        polygon->show();
    }
    else
    {
        // A blue box with some fancy interior.
        QCanvasRectangle* rect =
            new QCanvasRectangle(time2x(t->getStart(scenario)), y + 4,
                                 time2x(t->getEnd(scenario)) -
                                 time2x(t->getStart(scenario)),
                                 itemHeight - 8, ganttChart);

        rect->setPen(QPen(QColor("#4C5EFF")));
        rect->setBrush(QBrush(QColor("#4C5EFF"), Dense4Pattern));
        rect->setZ(10);
        rect->show();

        // The black progress bar.
        if (t->getCompletionDegree(scenario) > 0.0)
        {
            /* TODO: This does not work 100% correct for effort or length
             * based tasks. It's only correct for duration tasks. */
            int barWidth;
            if (t->getCompletionDegree(scenario) ==
                t->getCalcedCompletionDegree(scenario) &&
                reportDef->getProject()->getNow() < t->getEnd(scenario))
            {
                barWidth = time2x(reportDef->getProject()->getNow()) -
                    time2x(t->getStart(scenario));
            }
            else
                barWidth = (int) ((time2x(t->getEnd(scenario)) -
                                   time2x(t->getStart(scenario))) *
                                  (t->getCompletionDegree(scenario) / 100.0));

            rect = new QCanvasRectangle
                (time2x(t->getStart(scenario)), y + 8, barWidth,
                 itemHeight - 16, ganttChart);

            rect->setPen(QPen(QColor("black")));
            rect->setBrush(QBrush(QColor("black")));
            rect->setZ(11);
            rect->show();
        }
    }
}

void
TjTaskReport::drawDependencies(Task* const t1, QListViewItem* t1lvi)
{
#define abs(a) ((a) < 0 ? (-(a)) : (a))
#define min(a, b) ((a) < (b) ? (a) : (b))

    int arrowCounter = 0;
    TaskList sortedFollowers;

    /* To avoid unnecessary crossing of dependency arrows, we sort the
     * followers of the current task according to their absolute distance to
     * the Y position of this task in the list view. */
    int yPos = t1lvi->itemPos() + itemHeight / 2;
    for (TaskListIterator tli(t1->getFollowersIterator()); *tli; ++tli)
    {
        QListViewItem* lvi2 = getTaskListEntry(*tli);
        if (!lvi2)
            continue;
        int i = 0;
        for (TaskListIterator stli(sortedFollowers); *stli; ++stli, ++i)
        {
            QListViewItem* lvi3 = getTaskListEntry(*stli);
            if (abs(yPos - lvi2->itemPos()) >
                abs(yPos - lvi3->itemPos()))
                break;
        }
        sortedFollowers.insert(i, *tli);
    }

    for (TaskListIterator tli(sortedFollowers); *tli; ++tli)
    {
        Task* t2 = *tli;
        QListViewItem* t2lvi= getTaskListEntry(*tli);
        if (t2lvi)
        {
            int t1x = time2x(t1->getEnd(scenario));
            int t2x = time2x(t2->getStart(scenario));
            if (t2->isMilestone())
                t2x -= (itemHeight - 8) / 2;
            else if (t2->isContainer())
                t2x -= 3;

            int t1y = t1lvi->itemPos() + itemHeight / 2;
            int t2y = t2lvi->itemPos() + itemHeight / 2;
            int yCenter = t1y < t2y ? t1y + (t2y - t1y) / 2 :
                t2y + (t1y - t2y) / 2;
            // Ensure that yCenter is between the task lines.
            yCenter = (yCenter / itemHeight) * itemHeight;

            // Draw connection line.
            // Distance between task end and the first break of the arrow.
            const int minGap = 8;
            // Min distance between parallel arrors.
            const int arrowGap = 3;
            QPointArray a;
            if (t2x - t1x < 2 * minGap + arrowGap * arrowCounter)
            {
                a.resize(6);
                a.setPoint(0, t1x, t1y);
                int cx = t1x + minGap + arrowGap * arrowCounter;
                a.setPoint(1, cx, t1y);
                a.setPoint(2, cx, yCenter);
                a.setPoint(3, min(t2x, cx) - minGap, yCenter);
                a.setPoint(4, min(t2x, cx) - minGap, t2y);
                a.setPoint(5, t2x, t2y);
            }
            else
            {
                a.resize(4);
                a.setPoint(0, t1x, t1y);
                int cx = t1x + minGap + arrowGap * arrowCounter;
                a.setPoint(1, cx, t1y);
                a.setPoint(2, cx, t2y);
                a.setPoint(3, t2x, t2y);
            }
            arrowCounter++;

            for (uint i = 0; i < a.count() - 1; ++i)
            {
                QCanvasLine* line = new QCanvasLine(ganttChart);
                QPen pen(QColor("black"));
                line->setPen(pen);
                int x1, y1, x2, y2;
                a.point(i, &x1, &y1);
                a.point(i + 1, &x2, &y2);
                line->setPoints(x1, y1, x2, y2);
                line->setZ(20);
                line->show();
            }

            // Draw arrow head.
            const int arrowSize = 4;
            a.resize(4);
            a.setPoint(0, t2x, t2y);
            a.setPoint(1, t2x - arrowSize, t2y - arrowSize);
            a.setPoint(2, t2x - arrowSize, t2y + arrowSize);
            a.setPoint(3, t2x, t2y);

            QCanvasPolygon* polygon = new QCanvasPolygon(ganttChart);
            polygon->setPoints(a);
            polygon->setPen(QColor("black"));
            polygon->setBrush(QColor("black"));
            polygon->setZ(20);
            polygon->show();
        }
    }
}

void
TjTaskReport::drawTaskResources(Task* const t)
{
    Interval iv;
    for (ResourceListIterator rli(t->getBookedResourcesIterator(scenario));
         *rli; ++rli)
    {
        int rY = ca2lviDict[QString("r:") + t->getId() + ":" +
            (*rli)->getFullId()]->itemPos();
        switch (stepUnit)
        {
            case day:
                for (time_t i = midnight(t->getStart(scenario));
                     i <= (t->getEnd(scenario)); i = sameTimeNextDay(i))
                    drawResourceLoadColum(t, *rli, i, sameTimeNextDay(i), rY);
                break;
            case week:
                for (time_t i = beginOfWeek(t->getStart(scenario),
                                            t->getProject()->
                                            getWeekStartsMonday());
                     i <= (t->getEnd(scenario)); i = sameTimeNextWeek(i))
                    drawResourceLoadColum(t, *rli, i, sameTimeNextWeek(i), rY);
                break;
            case month:
                for (time_t i = beginOfMonth(t->getStart(scenario));
                     i <= (t->getEnd(scenario)); i = sameTimeNextMonth(i))
                    drawResourceLoadColum(t, *rli, i, sameTimeNextMonth(i), rY);
                break;
            case quarter:
                for (time_t i = beginOfQuarter(t->getStart(scenario));
                     i <= (t->getEnd(scenario)); i = sameTimeNextQuarter(i))
                    drawResourceLoadColum(t, *rli, i, sameTimeNextQuarter(i),
                                          rY);
                break;
            case year:
                for (time_t i = beginOfYear(t->getStart(scenario));
                     i <= (t->getEnd(scenario)); i = sameTimeNextYear(i))
                    drawResourceLoadColum(t, *rli, i, sameTimeNextYear(i), rY);
                break;
            default:
                kdError() << "Unknown stepUnit";
                break;
        }
    }
}

void
TjTaskReport::drawResourceLoadColum(Task* const t, Resource* const r,
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
    // Now we trim it so it does not extend over the ends of the task bar. We
    // also trim the interval so the load is only calculated for intervals
    // within the task period.
    if (start < t->getStart(scenario))
    {
        start = t->getStart(scenario);
        cx = time2x(start);
        cw = time2x(end) - cx + 1;
    }
    if (end > t->getEnd(scenario))
    {
        end = t->getEnd(scenario);
        cw = time2x(end) - cx + 1;
    }
    // Since the above calculation might have destroyed our 1 pixel margin, we
    // check it again.
    if (cx < cellStart + 2)
        cx = cellStart + 2;
    if (cx + cw > cellEnd - 2)
        cw = cellEnd - 2 - cx;

    // Now we are calculation the load of the resource with respect to this
    // task, to all tasks, and we calculate the not yet allocated load.
    Interval period(start, end);
    double freeLoad = r->getAvailableWorkLoad(scenario, period);
    double taskLoad = r->getLoad(scenario, period, AllAccounts, t);
    double load = r->getLoad(scenario, period, AllAccounts);
    double otherLoad = load - taskLoad;
    double maxLoad = load + freeLoad;
    if (maxLoad <= 0.0)
        return;

    // Transform the load values into colum Y coordinates.
    int colBottom = rY + itemHeight - 1;
    int colTop = rY + 1;
    int colTaskLoadTop = colBottom - (int) ((colBottom - colTop) *
                                            (taskLoad / maxLoad));
    int colOtherLoadTop = colBottom - (int) ((colBottom - colTop) *
                                             (load / maxLoad));

    // Just some interim variables so we can change the color with only a
    // single change.
    QColor thisTaskCol = QColor("#FD13C6");
    QColor otherTaskCol = QColor("#AB7979");
    QColor freeLoadCol = QColor("#C4E00E");

    // Now we draw the columns. But only if the load is larger than 0.
    if (taskLoad > 0.0)
    {
        // Load for this task.
        QCanvasRectangle* rect = new QCanvasRectangle
            (cx, colTaskLoadTop, cw, colBottom - colTaskLoadTop,
             ganttChart);
        rect->setBrush(QBrush(thisTaskCol, Dense4Pattern));
        rect->setPen(thisTaskCol);
        rect->setZ(10);
        rect->show();
    }

    if (otherLoad > 0.0)
    {
        QCanvasRectangle* rect = new QCanvasRectangle
            (cx, colOtherLoadTop, cw,
             colTaskLoadTop - colOtherLoadTop, ganttChart);
        rect->setBrush(QBrush(otherTaskCol, Dense6Pattern));
        rect->setPen(otherTaskCol);
        rect->setZ(10);
        rect->show();
    }

    if (freeLoad > 0.0)
    {
        QCanvasRectangle* rect = new QCanvasRectangle
            (cx, colTop, cw, colOtherLoadTop - colTop,
             ganttChart);
        rect->setBrush(QBrush(freeLoadCol, Dense6Pattern));
        rect->setPen(freeLoadCol);
        rect->setZ(10);
        rect->show();
    }
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

