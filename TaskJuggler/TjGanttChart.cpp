/*
 * The TaskJuggler Project Management Software
 *
 * Copyright (c) 2001, 2002, 2003, 2004, 2005 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id: taskjuggler.cpp 1085 2005-06-23 20:34:54Z cs $
 */

#include "TjGanttChart.h"

#include <assert.h>

#include <qnamespace.h>
#include <qcanvas.h>
#include <qpen.h>
#include <qbrush.h>
#include <qdatetime.h>
#include <klocale.h>
#include <qpainter.h>

#include "Project.h"
#include "Task.h"
#include "Resource.h"
#include "TaskList.h"
#include "ResourceList.h"
#include "QtReport.h"
#include "QtReportElement.h"

//                                           Boundary
const int TjGanttChart::minStepHour = 20;    //   365 * 24 * 20 = 175200
const int TjGanttChart::minStepDay = 20;     //   365 * 20 = 7300
const int TjGanttChart::minStepWeek = 35;    //   52 * 35 = 1820
const int TjGanttChart::minStepMonth = 35;   //   12 * 35 = 420
const int TjGanttChart::minStepQuarter = 25; //   4 * 25 = 100
const int TjGanttChart::minStepYear = 80;    //   1 * 80 = 80

const int TjGanttChart::zoomSteps[] =
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
    41946, // Hour
    176000
} ;

TjGanttChart::TjGanttChart(QObject* obj)
{
    header = new QCanvas(obj);
    chart = new QCanvas(obj);

    headerHeight = 0;
    chartHeight = 0;
    width = 0;

    scaleMode = fitSize;
}

TjGanttChart::~TjGanttChart()
{
    delete header;
    delete chart;
}

void
TjGanttChart::setProjectAndReportData(const QtReportElement* r,
                                      const TaskList* tl,
                                      const ResourceList* rl)
{
    reportElement = r;
    reportDef = static_cast<const QtReport*>(reportElement->getReport());
    project = reportDef->getProject();
    taskList = tl;
    resourceList = rl;
    scenario = reportElement->getScenario(0);
}

void
TjGanttChart::setSizes(int hh, int ch, int w)
{
    headerHeight = hh;
    chartHeight = ch;
    width = w;

    assert(headerHeight > 0);
    assert(headerHeight < 32767);   // QCanvas limit
    assert(chartHeight > 0);
    assert(chartHeight < 32767);    // QCanvas limit
    assert(width > 0);
    assert(width < 32767);          // QCanvas limit

    header->resize(width, headerHeight);
    chart->resize(width, chartHeight);
}

void
TjGanttChart::setColors(const QColor& hBg, const QColor& cBg,
                        const QColor& aBg, const QColor& bs,
                        const QColor& bs2, const QColor& md)
{
    headerBackgroundCol = hBg;
    chartBackgroundCol = cBg;
    altBackgroundCol = aBg;
    baseCol = bs;
    base2Col = bs2;
    midCol = md;
}

void
TjGanttChart::generate()
{
    // Clear ganttHeader canvas.
    QCanvasItemList cis = header->allItems();
    for (QCanvasItemList::Iterator it = cis.begin(); it != cis.end(); ++it)
        delete *it;

    // Clear ganttChart canvas.
    cis = chart->allItems();
    for (QCanvasItemList::Iterator it = cis.begin(); it != cis.end(); ++it)
        delete *it;

    int overallDuration;
    switch (scaleMode)
    {
        case fitSize:
        {
            startTime = reportElement->getStart();
            endTime = reportElement->getEnd();
            overallDuration = endTime - startTime;
            pixelPerYear = (int) ((double) width /
                                  (overallDuration / (60.0 * 60 * 24 * 365)));
            if (pixelPerYear / (4 * minStepHour) > 365 * 24)
            {
                stepUnit = hour;
                endTime += 60 * 60;
            }
            else if (pixelPerYear / (4 * minStepDay) >= 365)
            {
                stepUnit = day;
                endTime += 60 * 60 * 24;
            }
            else if (pixelPerYear / (4 * minStepWeek) >= 52)
            {
                stepUnit = week;
                endTime += 60 * 60 * 24 * 7;
            }
            else if (pixelPerYear / (4 * minStepMonth) >= 12)
            {
                stepUnit = month;
                endTime += 60 * 60 * 24 * 31;
            }
            else if (pixelPerYear / (4 * minStepQuarter) >= 4)
            {
                stepUnit = quarter;
                endTime += 60 * 60 * 24 * 92;
            }
            else
            {
                stepUnit = year;
                endTime += 60 * 60 * 24 * 366;
            }
            break;
        }
        case fitInterval:
        {
            /* In autoFit mode we try to fit the full timespan of the project
             * into the view. We ignore the project time frame specified by
             * the user and use the start time of the earliest task and the
             * end time of the last task instead. For a better look we add 5%
             * more at both sides. */
            startTime = reportElement->getEnd();
            endTime = reportElement->getStart();
            for (TaskListIterator tli(*taskList); *tli; ++tli)
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
            for (currentZoomStep = 0; currentZoomStep < sizeof(zoomSteps) /
                 sizeof(int) && (((float) (endTime - startTime) /
                                  (60 * 60 * 24 * 365)) *
                                 zoomSteps[currentZoomStep]) < 800;
                 ++currentZoomStep)
                ;
            if (currentZoomStep > 0)
                --currentZoomStep;
            pixelPerYear = zoomSteps[currentZoomStep];
            setBestStepUnit();
            break;
        }
        case manual:
        {
            if (startTime == 0)
                startTime = reportElement->getStart();
            if (endTime == 0)
                endTime = reportElement->getEnd();
            overallDuration = endTime - startTime;
        }
    }

    /* Some of the algorithems here require a mininum project duration of 1 day
     * to work properly. */
    if (endTime - startTime < 60 * 60 * 24)
        endTime = startTime + 60 * 60 * 24 + 1;

    /* QCanvas can only handle 32767 pixel width. So we have to shorten the
     * report period if the chart exceeds this size. */
    if (time2x(endTime) > 32767)
        endTime = x2time(32767 - 2);

    width = time2x(endTime);

    // Resize header canvas to new size.
    header->resize(width, headerHeight);

    // Resize chart canvas to new size.
    chart->resize(width, chartHeight);

    generateHeaderAndGrid();
}

void
TjGanttChart::paintHeader(const QRect& clip, QPainter* p, bool dbuf)
{
    header->drawArea(clip, p, dbuf);
}

void
TjGanttChart::paintChart(const QRect& clip, QPainter* p, bool dbuf)
{
    p->setClipping(true);
    p->setClipRect(clip);
    chart->drawArea(clip, p, dbuf);
    p->setClipping(false);
}

void
TjGanttChart::generateHeaderAndGrid()
{
    QCanvasLine* line = new QCanvasLine(header);
    line->setPoints(0, headerHeight / 2, width, headerHeight / 2);
    QPen pen = line->pen();
    pen.setColor(midCol);
    line->setPen(pen);
    line->setZ(TJRL_GRIDLINES);
    line->show();

    line = new QCanvasLine(header);
    line->setPoints(0, headerHeight - 1, width, headerHeight - 1);
    pen = line->pen();
    pen.setColor(headerBackgroundCol);
    line->setPen(pen);
    line->setZ(TJRL_BACKGROUND);
    line->show();

    QCanvasRectangle* rect =
        new QCanvasRectangle(0, 0, width, headerHeight - 1, header);
    pen = rect->pen();
    pen.setColor(midCol);
    rect->setPen(pen);
    QBrush brush = rect->brush();
    brush.setStyle(QBrush::SolidPattern);
    brush.setColor(headerBackgroundCol);
    rect->setBrush(brush);
    rect->setZ(TJRL_BACKGROUND);
    rect->show();

    switch (stepUnit)
    {
        case hour:
            generateDayHeader(0);
            generateHourHeader(headerHeight / 2);
            markHourBoundaries(1);
            markNonWorkingHoursOnBackground();
            break;
        case day:
            generateMonthHeader(0, TRUE);
            generateDayHeader(headerHeight / 2);
            markDayBoundaries();
            if (pixelPerYear > 365 * 24 * 2)
                markNonWorkingHoursOnBackground();
            else
                markNonWorkingDaysOnBackground();
            if (pixelPerYear > 365 * 8 * 8)
                markHourBoundaries(3);
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
                            TRUE, TJRL_NOWLINE);
}

void
TjGanttChart::generateHourHeader(int y)
{
    for (time_t hour = midnight(startTime); hour < endTime;
         hour = hoursLater(1, hour))
    {
        int x = time2x(hour);
        if (x < 0)
            continue;
        QCanvasLine* line = new QCanvasLine(header);
        line->setPoints(x, y, x, y + headerHeight / 2);
        QPen pen = line->pen();
        pen.setColor(midCol);
        line->setPen(pen);
        line->setZ(TJRL_GRIDLINES);
        line->show();

        // Write hour of day.
        QString label;
        label = QString("%1").arg(hourOfDay(hour));
        QCanvasText* text = new QCanvasText(label, header);
        text->setX(x + 2);
        text->setY(y);
        text->setZ(TJRL_GRIDLABLES);
        text->show();
    }
}

void
TjGanttChart::generateDayHeader(int y)
{
    for (time_t day = midnight(startTime);
         day < endTime; day = sameTimeNextDay(day))
    {
        int x = time2x(day);
        if (x < 0)
            continue;
        QCanvasLine* line = new QCanvasLine(header);
        line->setPoints(x, y, x, y + headerHeight / 2);
        QPen pen = line->pen();
        pen.setColor(midCol);
        line->setPen(pen);
        line->setZ(TJRL_GRIDLINES);
        line->show();

        // Write day of month.
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
        QCanvasText* text = new QCanvasText(label, header);
        text->setX(x + 2);
        text->setY(y);
        text->setZ(TJRL_GRIDLABLES);
        text->show();
    }
}

void
TjGanttChart::generateWeekHeader(int y)
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
        QCanvasLine* line = new QCanvasLine(header);
        line->setPoints(x, y, x, y + headerHeight / 2);
        QPen pen = line->pen();
        pen.setColor(midCol);
        line->setPen(pen);
        line->setZ(TJRL_GRIDLINES);
        line->show();

        // Write week number.
        QCanvasText* text = new QCanvasText
            (i18n("short for week of year", "W%1")
             .arg(weekOfYear(week, weekStartsMonday)),
             header);
        text->move(x + 2, y);
        text->setZ(TJRL_GRIDLABLES);
        text->show();
    }
}

void
TjGanttChart::generateMonthHeader(int y, bool withYear)
{
    for (time_t month = beginOfMonth(startTime);
         month < endTime; month = sameTimeNextMonth(month))
    {
        int x = time2x(month);
        if (x < 0)
            continue;
        QCanvasLine* line = new QCanvasLine(header);
        line->setPoints(x, y, x, y + headerHeight / 2);
        QPen pen = line->pen();
        pen.setColor(midCol);
        line->setPen(pen);
        line->setZ(TJRL_GRIDLINES);
        line->show();

        // Write month name (and year).
        QString s = withYear ?
            QString("%1 %2").arg(QDate::shortMonthName(monthOfYear(month)))
            .arg(::year(month)) :
            QString("%1").arg(QDate::shortMonthName(monthOfYear(month)));
        QCanvasText* text = new QCanvasText(s, header);
        text->move(x + 2, y);
        text->setZ(TJRL_GRIDLABLES);
        text->show();

        if (pixelPerYear / 12 > 600)
        {
            x += pixelPerYear / (2 * 12);
            // Draw month name (and year).
            QString s = withYear ?
                QString("%1 %2").arg(QDate::shortMonthName(monthOfYear(month)))
                .arg(::year(month)) :
                QString("%1").arg(QDate::shortMonthName(monthOfYear(month)));
            QCanvasText* text = new QCanvasText(s, header);
            text->move(x + 2, y);
            text->setZ(TJRL_GRIDLABLES);
            text->show();
        }
    }
}

void
TjGanttChart::generateQuarterHeader(int y)
{
    for (time_t quarter = beginOfQuarter(startTime);
         quarter < endTime; quarter =
         sameTimeNextQuarter(quarter))
    {
        int x = time2x(quarter);
        if (x < 0)
            continue;
        QCanvasLine* line = new QCanvasLine(header);
        line->setPoints(x, y, x, y + headerHeight / 2);
        QPen pen = line->pen();
        pen.setColor(midCol);
        line->setPen(pen);
        line->setZ(TJRL_GRIDLINES);
        line->show();

        // Write quarter number.
        QCanvasText* text =
            new QCanvasText(i18n("short for quater of year", "Q%1")
                            .arg(quarterOfYear(quarter)),
                            header);
        text->move(x + 2, y);
        text->setZ(TJRL_GRIDLABLES);
        text->show();
    }
}

void
TjGanttChart::generateYearHeader(int y)
{
    for (time_t year = beginOfYear(startTime);
         year < endTime; year = sameTimeNextYear(year))
    {
        int x = time2x(year);
        if (x < 0)
            continue;
        QCanvasLine* line = new QCanvasLine(header);
        line->setPoints(time2x(year), y, time2x(year), y + headerHeight / 2);
        QPen pen = line->pen();
        pen.setColor(midCol);
        line->setPen(pen);
        line->setZ(TJRL_GRIDLINES);
        line->show();

        // Write year.
        QCanvasText* text =
            new QCanvasText(QString("%1").arg(::year(year)), header);
        text->move(x + 2, y);
        text->setZ(TJRL_GRIDLABLES);
        text->show();
    }
}

void
TjGanttChart::markNonWorkingHoursOnBackground()
{
    QColor col = base2Col.dark(110);
    // Mark non-working days with a light grey background.
    for (time_t hour = midnight(startTime);
         hour < endTime; )
    {
        int x = time2x(hour);
        // The hour intervals are adjecent. To avoid breaks due to rounding
        // effects, we make them 1 pixel larger than usual.

        while (!reportDef->getProject()->
            isWorkingTime(Interval(hour, hoursLater(1, hour) - 1)))
        {
            hour = hoursLater(1, hour);
        }
        int w = time2x(hour) - x + 1;
        hour = hoursLater(1, hour);
        if (w == 1 || x + w - 1 < 0)
            continue;

        QCanvasRectangle* rect =
            new QCanvasRectangle(x, 0, w, chartHeight, chart);
        rect->setPen(QPen(col));
        rect->setBrush(QBrush(col));
        rect->setZ(TJRL_OFFTIME);
        rect->show();
    }
}

void
TjGanttChart::markNonWorkingDaysOnBackground()
{
    QColor col = base2Col.dark(110);
    // Mark non-working days with a light grey background.
    for (time_t day = midnight(startTime);
         day < endTime; day = sameTimeNextDay(day))
    {
        int x = time2x(day);
        int w = time2x(sameTimeNextDay(day)) - x + 1;
        if (x < 0)
            continue;

        if (isWeekend(day) ||
            reportDef->getProject()->isVacation(day))
        {
            QCanvasRectangle* rect =
                new QCanvasRectangle(x, 0, w, chartHeight, chart);
            rect->setPen(QPen(col));
            rect->setBrush(QBrush(col));
            rect->setZ(TJRL_OFFTIME);
            rect->show();
        }
    }
}

void
TjGanttChart::markBoundary(int x, bool now, int layer)
{
    QColor col;
    if (now)
        col = QColor(Qt::red);
    else
        col = altBackgroundCol;

    // Draws a vertical line on the chart to higlight a time period
    // boundary.
    if (x < 0)
        return;
    QCanvasLine* line = new QCanvasLine(chart);
    line->setPoints(x, 0, x, chartHeight);
    QPen pen(col);
    line->setPen(pen);
    line->setZ(layer);
    line->show();
}

void
TjGanttChart::markHourBoundaries(int distance)
{
    for (time_t hour = midnight(startTime);
         hour < endTime; hour = hoursLater(distance, hour))
        markBoundary(time2x(hour));
}

void
TjGanttChart::markDayBoundaries()
{
    for (time_t day = midnight(startTime);
         day < endTime; day = sameTimeNextDay(day))
        markBoundary(time2x(day));
}

void
TjGanttChart::markWeekBoundaries()
{
    bool weekStartsMonday = reportDef->getWeekStartsMonday();

    for (time_t week = beginOfWeek(startTime, weekStartsMonday);
         week < endTime;
         week = sameTimeNextWeek(week))
        markBoundary(time2x(week));
}

void
TjGanttChart::markMonthsBoundaries()
{
    for (time_t month = beginOfMonth(startTime);
         month < endTime; month =
         sameTimeNextMonth(month))
        markBoundary(time2x(month));
}

void
TjGanttChart::markQuarterBoundaries()
{
    for (time_t quarter = beginOfQuarter(startTime);
         quarter < endTime; quarter =
         sameTimeNextQuarter(quarter))
        markBoundary(time2x(quarter));
}

void
TjGanttChart::generateGanttBackground()
{
    QCanvasRectangle* rect =
        new QCanvasRectangle(0, 0, chart->width(), chart->height(),
                             chart);
    rect->setPen(QPen(Qt::NoPen));
    QColor bgColor = chartBackgroundCol;
    rect->setBrush(QBrush(bgColor));
    rect->setZ(TJRL_BACKGROUND);
    rect->show();

    bool toggle = FALSE;
    for (int y = 0; y < chartHeight; y += 25) // Just temporary
    {
        toggle = !toggle;
        if (toggle)
            continue;

        rect = new QCanvasRectangle(0, y, width, 25, chart);
        rect->setPen(QPen(Qt::NoPen));
        rect->setBrush(QBrush(altBackgroundCol));
        rect->setZ(TJRL_BACKLINES);
        rect->show();
    }
}

int
TjGanttChart::time2x(time_t t) const
{
    return (int) ((t - startTime) *
                  (((float) pixelPerYear) / (60 * 60 * 24 * 365)));
}

time_t
TjGanttChart::x2time(int x) const
{
    return (time_t) (startTime + ((float) x * 60 * 60 * 24 * 365) /
                     pixelPerYear);
}

void
TjGanttChart::setBestStepUnit()
{
    if (pixelPerYear / minStepHour > 365 * 24)
        stepUnit = hour;
    else if (pixelPerYear / minStepDay >= 365)
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

