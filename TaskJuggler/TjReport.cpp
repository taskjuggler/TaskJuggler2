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
#include <kiconloader.h>

#include "Project.h"
#include "Task.h"
#include "Resource.h"
#include "Utility.h"
#include "ExpressionTree.h"
#include "Report.h"
#include "TableColumnFormat.h"
#include "QtTaskReport.h"
#include "QtTaskReportElement.h"
#include "QtResourceReport.h"
#include "QtResourceReportElement.h"

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
TjReport::generateReport()
{
    setLoadingProject(TRUE);

    if (!this->generateList())
    {
        setLoadingProject(FALSE);
        return FALSE;
    }
    if (!this->generateChart(TRUE))
    {
        setLoadingProject(FALSE);
        return FALSE;
    }

    setLoadingProject(FALSE);

    return TRUE;
}

void
TjReport::prepareChart(bool autoFit, QtReportElement* repElement)
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
    itemHeight = listView->firstChild()->height();
    QListViewItem* lvi;
    for (lvi = listView->firstChild(); lvi && lvi->itemBelow();
         lvi = lvi->itemBelow())
        ;
    listHeight = lvi->itemPos() + itemHeight;

    int overallDuration;
    if (autoFit)
    {
        /* In autoFit mode we try to fit the full timespan of the project into
         * the view. We ignore the project time frame specified by the user
         * and use the start time of the earliest task and the end time of the
         * last task instead. For a better look we add 5% more at both sides.
         */
        startTime = repElement->getEnd();
        endTime = repElement->getStart();
        for (TaskListIterator tli(taskList); *tli; ++tli)
        {
            if ((*tli)->getStart(scenario) < startTime)
                startTime = (*tli)->getStart(scenario);
            if ((*tli)->getEnd(scenario) > endTime)
                endTime = (*tli)->getEnd(scenario);
        }
        overallDuration = endTime - startTime;
        startTime -= (time_t) (0.05 * overallDuration);
        endTime += (time_t) (0.05 * overallDuration);
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
            startTime = repElement->getStart();
        if (endTime == 0)
            endTime = repElement->getEnd();
        overallDuration = endTime - startTime;
    }

    /* Some of the algorithems here require a mininum project duration of 1 day
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

    // Draw a red line to mark the current time.
    if (reportDef->getProject()->getNow() >=
        reportDef->getProject()->getStart() &&
        reportDef->getProject()->getNow() < reportDef->getProject()->getEnd())
        markBoundary(time2x(reportDef->getProject()->getNow()),
                            Qt::red, 3);
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
TjReport::markBoundary(int x, const QColor& col, int layer)
{
    // Draws a vertical line on the ganttChart to higlight a time period
    // boundary.
    if (x < 0)
        return;
    QCanvasLine* line = new QCanvasLine(ganttChart);
    line->setPoints(x, 0, x, listHeight);
    QPen pen(col);
    line->setPen(pen);
    line->setZ(layer);
    line->show();
}

void
TjReport::markHourBoundaries()
{
    for (time_t hour = midnight(startTime);
         hour < endTime; hour = hoursLater(3, hour))
        markBoundary(time2x(hour), colorGroup().mid());
}

void
TjReport::markDayBoundaries()
{
    for (time_t day = midnight(startTime);
         day < endTime; day = sameTimeNextDay(day))
        markBoundary(time2x(day), colorGroup().mid());
}

void
TjReport::markWeekBoundaries()
{
    bool weekStartsMonday = reportDef->getWeekStartsMonday();

    for (time_t week = beginOfWeek(startTime, weekStartsMonday);
         week < endTime;
         week = sameTimeNextWeek(week))
        markBoundary(time2x(week), colorGroup().mid());
}

void
TjReport::markMonthsBoundaries()
{
    for (time_t month = beginOfMonth(startTime);
         month < endTime; month =
         sameTimeNextMonth(month))
        markBoundary(time2x(month), colorGroup().mid());
}

void
TjReport::markQuarterBoundaries()
{
    for (time_t quarter = beginOfQuarter(startTime);
         quarter < endTime; quarter =
         sameTimeNextQuarter(quarter))
        markBoundary(time2x(quarter), colorGroup().mid());
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
TjReport::generateListHeader(const QString& firstHeader, QtReportElement* tab)
{
    // The first column is always the Task/Resource column
    listView->addColumn(firstHeader);

    int col = 1;
    for (QPtrListIterator<TableColumnInfo>
         ci = tab->getColumnsIterator(); *ci; ++ci, ++col)
    {
        const TableColumnFormat* tcf =
            tab->getColumnFormat((*ci)->getName());
        listView->addColumn(tcf->getTitle() + "\n");
        listView->setColumnAlignment(col, tcf->getHAlign());
    }
}

void
TjReport::collapsReportItem(QListViewItem*)
{
    if (loadingProject)
        return;

    this->generateChart(FALSE);

    syncVSliders(ganttChartView->contentsX(), listView->contentsY());
}

void
TjReport::expandReportItem(QListViewItem*)
{
    if (loadingProject)
        return;

    this->generateChart(FALSE);
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
    this->generateChart(FALSE);

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
    this->generateChart(FALSE);

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

QString
TjReport::indent(const QString& input, const QListViewItem* lvi, bool right)
{
    // First let's find out how deep we are down the tree;
    int level = treeLevel(lvi);

    if (right)
    {
        QString spaces = QString().fill(' ', 2 * (maxDepth - level));
        return input + spaces;
    }
    else
    {
        QString spaces = QString().fill(' ', 2 * level);
        return spaces + input;
    }
}

int
TjReport::treeLevel(const QListViewItem* lvi) const
{
    int level = 0;
    while (lvi->parent())
    {
        level++;
        lvi = lvi->parent();
    }
    return level;
}

