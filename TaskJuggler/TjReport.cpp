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

#include "TjReport.h"

#include <qsplitter.h>
#include <qlayout.h>
#include <qfont.h>
#include <qheader.h>
#include <qcanvas.h>
#include <qdatetime.h>

#include <klistview.h>
#include <klocale.h>
#include <kdebug.h>

#include "Project.h"
#include "Task.h"
#include "Resource.h"
#include "Utility.h"
#include "ExpressionTree.h"
#include "Report.h"
#include "TableColumnFormat.h"
#include "QtTaskReport.h"
#include "QtTaskReportElement.h"

//                                           Boundary
const int TjReport::minStepHour = 4;    //   365 * 24 * 4 = 35040
const int TjReport::minStepDay = 20;     //   365 * 20 = 7300
const int TjReport::minStepWeek = 35;    //   52 * 35 = 1820
const int TjReport::minStepMonth = 35;   //   12 * 35 = 420
const int TjReport::minStepQuarter = 25; //   4 * 25 = 100
const int TjReport::minStepYear = 80;    //   1 * 80 = 80

const int TjReport::zoomSteps[] =
{
           // Mode       Sublines
    80,    // Year       Quarter
    141,   // Quarter
    250,   // Quarter    Month
    441,   // Month
    780,   // Month      Week
    1378,  // Month      Week
    2435,  // Week
    4302,  // Week       Day
    7602,  // Day
    13434, // Day
    23738, // Day        Hour
    41946, // Hour
    74118  // Hour
} ;

TjReport::TjReport(QWidget* p, Report* const rDef, const QString& n)
    : QWidget(p, n), reportDef(rDef)
{
    loadingProject = FALSE;

    QHBoxLayout* hl = new QHBoxLayout(this, 0, 0);
    splitter = new QSplitter(Horizontal, this);

    listView = new KListView(splitter);
    listView->setRootIsDecorated(TRUE);
    listView->setVScrollBarMode(QScrollView::AlwaysOff);
    listView->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);

    canvasFrame = new QWidget(splitter);
    QVBoxLayout* vl = new QVBoxLayout(canvasFrame, 0, 0);
    canvasFrame->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);

    ganttHeader = new QCanvas(this);
    ganttHeaderView = new QCanvasView(ganttHeader, canvasFrame);
    ganttHeaderView->setHScrollBarMode(QScrollView::AlwaysOff);
    ganttHeaderView->setVScrollBarMode(QScrollView::AlwaysOff);

    ganttChart = new QCanvas(this);
    ganttChartView = new QCanvasView(ganttChart, canvasFrame);

    currentZoomStep = 5;
    stepUnit = week;
    pixelPerYear = zoomSteps[currentZoomStep];
    startTime = endTime = 0;

    vl->addWidget(ganttHeaderView);
    vl->addWidget(ganttChartView);
    hl->addWidget(splitter);

    connect(listView, SIGNAL(expanded(QListViewItem*)),
            this, SLOT(expandReportItem(QListViewItem*)));
    connect(listView, SIGNAL(collapsed(QListViewItem*)),
            this, SLOT(collapsReportItem(QListViewItem*)));
    connect(ganttChartView, SIGNAL(contentsMoving(int, int)),
            this, SLOT(syncVSliders(int, int)));
}

bool
TjReport::generateTaskReport()
{
    setLoadingProject(TRUE);

    if (!generateTaskList())
    {
        setLoadingProject(FALSE);
        return FALSE;
    }
    generateGanttChart(TRUE);

    setLoadingProject(FALSE);

    return TRUE;
}

bool
TjReport::generateTaskList()
{
    // Remove all items and columns from list view.
    listView->clear();
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

    generateLeftHeader();

    for (TaskListIterator tli(reportDef->getProject()->
                              getTaskListIterator()); *tli; ++tli)
    {
        KListViewItem* newLvi;
        if ((*tli)->getParent())
        {
            newLvi = new KListViewItem
                (listView->findItem((*tli)->getParent()->getId(),
                                    1), (*tli)->getName(),
                 (*tli)->getId());
        }
        else
        {
            newLvi = new KListViewItem(listView, (*tli)->getName(),
                                       (*tli)->getId());
        }

        if ((*tli)->isContainer())
        {
            if (reportDef->getRollUpTask())
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
        else
        {
            for (ResourceListIterator rli((*tli)->
                                          getBookedResourcesIterator(scenario));
                 *rli; ++rli)
            {
                new KListViewItem(newLvi, (*rli)->getName(),
                                  (*rli)->getFullId());
            }
        }

        int column = 2;
        for (QPtrListIterator<TableColumnInfo>
             ci = reportElement->getColumnsIterator(); *ci; ++ci, ++column)
        {

            if ((*ci)->getName() == "start")
                newLvi->setText(column,
                                time2user((*tli)->getStart(scenario),
                                          reportDef->getTimeFormat()));
            else if ((*ci)->getName() == "end")
                newLvi->setText(column,
                                time2user(((*tli)->isMilestone() ? 1 :
                                           0) + (*tli)->getEnd(scenario),
                                          reportDef->getTimeFormat()));
        }
    }

    return TRUE;
}

void
TjReport::generateGanttChart(bool autoFit)
{
    // Clear ganttHeader canvas.
    QCanvasItemList cis = ganttHeader->allItems();
    for (QCanvasItemList::Iterator it = cis.begin(); it != cis.end(); ++it)
        delete *it;

    // Clear ganttChart canvas.
    cis = ganttChart->allItems();
    for (QCanvasItemList::Iterator it = cis.begin(); it != cis.end(); ++it)
        delete *it;

    // Calculate some commenly used values;
    headerHeight = listView->header()->height();
    itemHeight = listView->lastItem()->height();
    listHeight = listView->lastItem()->itemPos();

    int overallDuration;
    if (autoFit)
    {
        startTime = reportElement->getEnd();
        endTime = reportElement->getStart();
        for (TaskListIterator tli(taskList); *tli; ++tli)
        {
            if ((*tli)->getStart(scenario) < startTime)
                startTime = (*tli)->getStart(scenario);
            if ((*tli)->getEnd(scenario) > endTime)
                endTime = (*tli)->getEnd(scenario);
        }
        overallDuration = endTime - startTime;
        startTime -= (time_t) (0.1 * overallDuration);
        endTime += (time_t) (0.1 * overallDuration);
        // We try to use a scaling so that the initial Gantt chart is about
        // 800 pixels width.
        for (currentZoomStep = 0;
             currentZoomStep < sizeof(zoomSteps) / sizeof(int) &&
             (((float) (endTime - startTime) / (60 * 60 * 24 * 365)) *
             zoomSteps[currentZoomStep]) < 800;
             ++currentZoomStep)
            ;
        if (currentZoomStep > 0)
            --currentZoomStep;
        pixelPerYear = zoomSteps[currentZoomStep];
        setBestStepUnit();
    }
    else
    {
        if (startTime == 0)
            startTime = reportElement->getStart();
        if (endTime == 0)
            endTime = reportElement->getEnd();
        overallDuration = endTime - startTime;
    }

    /* Some of the algorithms here require a mininum project duration of 1 day
     * to work properly. */
    if (endTime - startTime < 60 * 60 * 24)
        endTime = startTime + 60 * 60 * 24 + 1;

    canvasWidth = time2x(endTime);
    canvasFrame->setMaximumWidth(canvasWidth + 2);

    // Resize header canvas to new size.
    ganttHeader->resize(canvasWidth, headerHeight);
    ganttHeaderView->setFixedHeight(headerHeight);

    // Resize chart canvas to new size.
    ganttChart->resize(canvasWidth, listHeight);

    generateHeaderAndGrid();

    generateGanttTasks();

    ganttHeader->update();
    ganttChart->update();
}

void
TjReport::generateHeaderAndGrid()
{
    QCanvasLine* line = new QCanvasLine(ganttHeader);
    line->setPoints(0, headerHeight - 1, canvasWidth, headerHeight - 1);
    QPen pen = line->pen();
    pen.setColor(colorGroup().background());
    line->setPen(pen);
    line->show();

    QCanvasRectangle* rect =
        new QCanvasRectangle(0, 0, canvasWidth, headerHeight - 1, ganttHeader);
    pen = rect->pen();
    pen.setColor(colorGroup().mid());
    rect->setPen(pen);
    QBrush brush = rect->brush();
    brush.setStyle(QBrush::SolidPattern);
    brush.setColor(colorGroup().background());
    rect->setBrush(brush);
    rect->setZ(0);
    rect->show();

    switch (stepUnit)
    {
        case day:
            generateMonthHeader(0, TRUE);
            generateDayHeader(headerHeight / 2);
            markDayBoundaries();
            if (pixelPerYear > 365 * 24 * 2)
                markNonWorkingHoursOnBackground();
            else
                markNonWorkingDaysOnBackground();
            if (pixelPerYear > 365 * 8 * 8)
                markHourBoundaries();
            break;
        case week:
            generateMonthHeader(0, TRUE);
            generateWeekHeader(headerHeight / 2);
            markNonWorkingDaysOnBackground();
            markWeekBoundaries();
            if (pixelPerYear > 365 * 10)
                markDayBoundaries();
            break;
        case month:
            generateYearHeader(0);
            generateMonthHeader(headerHeight / 2, FALSE);
            markMonthsBoundaries();
            // Ensure that we have at least 2 pixels per day.
            if (pixelPerYear > 52 * 14)
                markNonWorkingDaysOnBackground();
            break;
        case quarter:
            generateYearHeader(0);
            generateQuarterHeader(headerHeight / 2);
            markQuarterBoundaries();
            if (pixelPerYear > 12 * 20)
                markMonthsBoundaries();
            break;
        case year:
            generateYearHeader(headerHeight / 2);
            // Ensure that we have at least 20 pixels per month or per
            // quarter.
            if (pixelPerYear > 20 * 12)
                markMonthsBoundaries();
            else if (pixelPerYear >= 20 * 4)
                markQuarterBoundaries();
            break;
    }
    generateGanttBackground();
}

void
TjReport::generateDayHeader(int y)
{
    for (time_t day = midnight(startTime);
         day < endTime; day = sameTimeNextDay(day))
    {
        int x = time2x(day);
        if (x < 0)
            continue;
        QCanvasLine* line = new QCanvasLine(ganttHeader);
        line->setPoints(x, y, x, y + headerHeight / 2);
        QPen pen = line->pen();
        pen.setColor(colorGroup().mid());
        line->setPen(pen);
        line->setZ(1);
        line->show();

        // Draw day of month.
        QString label;
        int stepSize = pixelPerYear / 365;
        if (stepSize > 70)
            label = QString("%1, %2 %3")
                .arg(QDate::shortDayName(dayOfWeek(day, TRUE) + 1))
                .arg(QDate::shortMonthName(monthOfYear(day)))
                .arg(dayOfMonth(day));
        else if (stepSize > 45)
            label = QString("%1 %2")
                .arg(QDate::shortDayName(dayOfWeek(day, TRUE) + 1))
                .arg(dayOfMonth(day));
        else
            label = QString("%1").arg(dayOfMonth(day));
        QCanvasText* text = new QCanvasText(label, ganttHeader);
        text->setX(x + 2);
        text->setY(y);
        text->setZ(1);
        text->show();
    }
}

void
TjReport::generateWeekHeader(int y)
{
    bool weekStartsMonday = reportDef->getWeekStartsMonday();

    for (time_t week = beginOfWeek(startTime, weekStartsMonday);
         week < endTime;
         week = sameTimeNextWeek(week))
    {
        // Draw vertical line at beginning of week.
        int x = time2x(week);
        if (x < 0)
            continue;
        QCanvasLine* line = new QCanvasLine(ganttHeader);
        line->setPoints(x, y, x, y + headerHeight / 2);
        QPen pen = line->pen();
        pen.setColor(colorGroup().mid());
        line->setPen(pen);
        line->setZ(1);
        line->show();

        // Draw week number.
        QCanvasText* text = new QCanvasText
            (i18n("short for week of year", "W%1")
             .arg(weekOfYear(week, weekStartsMonday)),
             ganttHeader);
        text->move(x + 2, y);
        text->setZ(1);
        text->show();
    }
}

void
TjReport::generateMonthHeader(int y, bool withYear)
{
    for (time_t month = beginOfMonth(startTime);
         month < endTime; month =
         sameTimeNextMonth(month))
    {
        int x = time2x(month);
        if (x < 0)
            continue;
        QCanvasLine* line = new QCanvasLine(ganttHeader);
        line->setPoints(x, y, x, y + headerHeight / 2);
        QPen pen = line->pen();
        pen.setColor(colorGroup().mid());
        line->setPen(pen);
        line->setZ(1);
        line->show();

        // Draw month name (and year).
        QString s = withYear ?
            QString("%1 %2").arg(QDate::shortMonthName(monthOfYear(month)))
            .arg(::year(month)) :
            QString("%1").arg(QDate::shortMonthName(monthOfYear(month)));
        QCanvasText* text = new QCanvasText(s, ganttHeader);
        text->move(x + 2, y);
        text->setZ(1);
        text->show();

        if (pixelPerYear / 12 > 600)
        {
            x += pixelPerYear / (2 * 12);
            // Draw month name (and year).
            QString s = withYear ?
                QString("%1 %2").arg(QDate::shortMonthName(monthOfYear(month)))
                .arg(::year(month)) :
                QString("%1").arg(QDate::shortMonthName(monthOfYear(month)));
            QCanvasText* text = new QCanvasText(s, ganttHeader);
            text->move(x + 2, y);
            text->setZ(1);
            text->show();
        }
    }
}

void
TjReport::generateQuarterHeader(int y)
{
    for (time_t quarter = beginOfQuarter(startTime);
         quarter < endTime; quarter =
         sameTimeNextQuarter(quarter))
    {
        int x = time2x(quarter);
        if (x < 0)
            continue;
        QCanvasLine* line = new QCanvasLine(ganttHeader);
        line->setPoints(x, y, x, y + headerHeight / 2);
        QPen pen = line->pen();
        pen.setColor(colorGroup().mid());
        line->setPen(pen);
        line->setZ(1);
        line->show();

        // Draw quarter number.
        QCanvasText* text =
            new QCanvasText(i18n("short for quater of year", "Q%1")
                            .arg(quarterOfYear(quarter)),
                            ganttHeader);
        text->move(x + 2, y);
        text->setZ(1);
        text->show();
    }
}

void
TjReport::generateYearHeader(int y)
{
    for (time_t year = beginOfYear(startTime);
         year < endTime; year = sameTimeNextYear(year))
    {
        int x = time2x(year);
        if (x < 0)
            continue;
        QCanvasLine* line = new QCanvasLine(ganttHeader);
        line->setPoints(time2x(year), y, time2x(year), y + headerHeight / 2);
        QPen pen = line->pen();
        pen.setColor(colorGroup().mid());
        line->setPen(pen);
        line->setZ(1);
        line->show();

        // Draw year.
        QCanvasText* text =
            new QCanvasText(QString("%1").arg(::year(year)), ganttHeader);
        text->move(x + 2, y);
        text->setZ(1);
        text->show();
    }
}

void
TjReport::markNonWorkingHoursOnBackground()
{
    // Mark non-working days with a light grey background.
    for (time_t hour = midnight(startTime);
         hour < endTime; hour = hoursLater(1, hour))
    {
        int x = time2x(hour);
        if (x < 0)
            continue;

        if (!reportDef->getProject()->
            isWorkingTime(Interval(hour, hoursLater(1, hour) - 1)))
        {
            QCanvasRectangle* rect =
                new QCanvasRectangle(x, 0, pixelPerYear / (365 * 24),
                                     listHeight, ganttChart);
            rect->setPen(QPen(NoPen));
            rect->setBrush(QBrush(QColor("#F0F0F0")));
            rect->setZ(1);
            rect->show();
        }
    }
}

void
TjReport::markNonWorkingDaysOnBackground()
{
    // Mark non-working days with a light grey background.
    for (time_t day = midnight(startTime);
         day < endTime; day = sameTimeNextDay(day))
    {
        int x = time2x(day);
        if (x < 0)
            continue;

        if (isWeekend(day) ||
            reportDef->getProject()->isVacation(day))
        {
            QCanvasRectangle* rect =
                new QCanvasRectangle(x, 0, pixelPerYear / 365 + 1,
                                     listHeight, ganttChart);
            rect->setPen(QPen(NoPen));
            rect->setBrush(QBrush(QColor("#F0F0F0")));
            rect->setZ(1);
            rect->show();
        }
    }
}

void
TjReport::markBoundary(int x)
{
    // Draws a vertical line on the ganttChart to higlight a time period
    // boundary.
    if (x < 0)
        return;
    QCanvasLine* line = new QCanvasLine(ganttChart);
    line->setPoints(x, 0, x, listHeight);
    QPen pen = line->pen();
    pen.setColor(colorGroup().mid());
    line->setPen(pen);
    line->setZ(2);
    line->show();
}

void
TjReport::markHourBoundaries()
{
    for (time_t hour = midnight(startTime);
         hour < endTime; hour = hoursLater(3, hour))
        markBoundary(time2x(hour));
}

void
TjReport::markDayBoundaries()
{
    for (time_t day = midnight(startTime);
         day < endTime; day = sameTimeNextDay(day))
        markBoundary(time2x(day));
}

void
TjReport::markWeekBoundaries()
{
    bool weekStartsMonday = reportDef->getWeekStartsMonday();

    for (time_t week = beginOfWeek(startTime, weekStartsMonday);
         week < endTime;
         week = sameTimeNextWeek(week))
        markBoundary(time2x(week));
}

void
TjReport::markMonthsBoundaries()
{
    for (time_t month = beginOfMonth(startTime);
         month < endTime; month =
         sameTimeNextMonth(month))
        markBoundary(time2x(month));
}

void
TjReport::markQuarterBoundaries()
{
    for (time_t quarter = beginOfQuarter(startTime);
         quarter < endTime; quarter =
         sameTimeNextQuarter(quarter))
        markBoundary(time2x(quarter));
}

void
TjReport::generateGanttBackground()
{
    bool toggle = FALSE;
    for (int y = 0; y < listHeight; y += itemHeight)
    {
        toggle = !toggle;
        if (toggle)
            continue;

        QCanvasRectangle* rect =
            new QCanvasRectangle(0, y, canvasWidth, itemHeight, ganttChart);
        rect->setPen(QPen(NoPen));
        rect->setBrush(QBrush(colorGroup().highlight().light(197)));
        rect->setZ(0);
        rect->show();
    }
}

void
TjReport::generateGanttTasks()
{
    for (TaskListIterator tli(reportDef->getProject()->
                              getTaskListIterator()); *tli; ++tli)
    {
        QListViewItem* lvi = getTaskListEntry(*tli);
        if (lvi)
        {
            generateTask(*tli, lvi->itemPos());
            generateDependencies(*tli, lvi);
            if (lvi->isOpen())
                generateTaskResources(*tli, lvi->itemPos());
        }
    }
}

void
TjReport::generateTask(Task* const t, int y)
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

        rect->setPen(QPen(QColor("#000090")));
        rect->setBrush(QBrush(QColor("#000090"), Dense4Pattern));
        rect->setZ(10);
        rect->show();
    }
}

void
TjReport::generateDependencies(Task* const t1, QListViewItem* t1lvi)
{
#define abs(a) ((a) < 0 ? (-(a)) : (a))

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
                a.setPoint(3, t2x - minGap, yCenter);
                a.setPoint(4, t2x - minGap, t2y);
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
TjReport::generateTaskResources(Task* const t, int taskY)
{
    Interval iv;
    int rY = taskY + itemHeight;
    for (ResourceListIterator rli(t->getBookedResourcesIterator(scenario));
         *rli; ++rli)
    {
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
        rY += itemHeight;
    }
}

void
TjReport::drawResourceLoadColum(Task* const t, Resource* const r,
                                time_t start, time_t end, int rY)
{
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
        cx = time2x(t->getStart(scenario));
        cw -= time2x(end) - 1 - cx;
    }
    if (end > t->getEnd(scenario))
    {
        end = t->getEnd(scenario);
        cw = time2x(t->getEnd(scenario)) - cx;
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
    QColor otherTaskCol = QColor("#1AE85B");
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
        rect->setBrush(QBrush(otherTaskCol, Dense4Pattern));
        rect->setPen(otherTaskCol);
        rect->setZ(10);
        rect->show();
    }

    if (freeLoad > 0.0)
    {
        QCanvasRectangle* rect = new QCanvasRectangle
            (cx, colTop, cw, colOtherLoadTop - colTop,
             ganttChart);
        rect->setBrush(QBrush(freeLoadCol, Dense4Pattern));
        rect->setPen(freeLoadCol);
        rect->setZ(10);
        rect->show();
    }
}

QListViewItem*
TjReport::getTaskListEntry(Task* const t)
{
    /* Returns the QListViewItem pointer for the task if the task is shown in
     * the list view. Tasks that have closed parents are not considered to be
     * visible even though they are part of the list view. Offscreen tasks
     * are considered visible if they meet the above condition. */

    // Check that the task is in the list. Colum 1 contains the task ID.
    QListViewItem* lvi = listView->findItem(t->getId(), 1);
    if (!lvi)
        return 0;

    // Now make sure that all parents are open.
    for (QListViewItem* i = lvi; i; i = i->parent())
        if (i->parent() && !i->parent()->isOpen())
            return 0;

    return lvi;
}

int
TjReport::time2x(time_t t)
{
    return (int) ((t - startTime) *
                  (((float) pixelPerYear) / (60 * 60 * 24 * 365)));
}

time_t
TjReport::x2time(int x)
{
    return (time_t) (startTime + ((float) x * 60 * 60 * 24 * 365) /
                     pixelPerYear);
}

void
TjReport::generateLeftHeader()
{
    // The first column is always the Task/Resource column
    listView->addColumn(i18n("Task"));

    // The 2nd column contains the ID and is always hidden.
    listView->addColumn(i18n("Id"));
    listView->setColumnWidthMode(1, QListView::Manual);
    listView->hideColumn(1);

    const QtTaskReportElement* tab =
        (dynamic_cast<QtTaskReport*>(reportDef))->getTable();
    for (QPtrListIterator<TableColumnInfo>
         ci = tab->getColumnsIterator(); *ci; ++ci)
    {
        const TableColumnFormat* tcf =
            tab->getColumnFormat((*ci)->getName());
        listView->addColumn(tcf->getTitle() + "\n");
    }
}

void
TjReport::collapsReportItem(QListViewItem*)
{
    if (loadingProject)
        return;

    generateGanttChart(FALSE);

    syncVSliders(ganttChartView->contentsX(), listView->contentsY());
}

void
TjReport::expandReportItem(QListViewItem*)
{
    if (loadingProject)
        return;

    generateGanttChart(FALSE);
    syncVSliders(ganttChartView->contentsX(), listView->contentsY());
}

void
TjReport::syncVSliders(int x, int y)
{
    ganttHeaderView->setContentsPos(x, ganttHeaderView->contentsY());
    listView->setContentsPos(listView->contentsX(), y);
}

void
TjReport::zoomIn()
{
    if (currentZoomStep >= sizeof(zoomSteps) / sizeof(int) - 1)
        return;

    time_t x = x2time(ganttChartView->contentsX());
    int y = ganttChartView->contentsY();

    pixelPerYear = zoomSteps[++currentZoomStep];
    setBestStepUnit();
    generateGanttChart(FALSE);

    ganttHeaderView->setContentsPos(time2x(x), 0);
    ganttChartView->setContentsPos(time2x(x), y);
}

void
TjReport::zoomOut()
{
    if (currentZoomStep == 0 ||
        ((zoomSteps[currentZoomStep - 1] * (float) (endTime - startTime))
         / (60 * 60 * 24 * 365)) < canvasFrame->minimumWidth())
        return;

    time_t x = x2time(ganttChartView->contentsX());
    int y = ganttChartView->contentsY();

    pixelPerYear = zoomSteps[--currentZoomStep];
    setBestStepUnit();
    generateGanttChart(FALSE);

    ganttHeaderView->setContentsPos(time2x(x), 0);
    ganttChartView->setContentsPos(time2x(x), y);
}

void
TjReport::setBestStepUnit()
{
    if (pixelPerYear / minStepDay >= 365)
        stepUnit = day;
    else if (pixelPerYear / minStepWeek >= 52)
        stepUnit = week;
    else if (pixelPerYear / minStepMonth >= 12)
        stepUnit = month;
    else if (pixelPerYear / minStepQuarter >= 4)
        stepUnit = quarter;
    else
        stepUnit = year;
}

