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
#include "TjObjPosTable.h"

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
    legend = new QCanvas(obj);

    headerHeight = 0;
    chartHeight = 0;
    width = 0;
    minRowHeight = 0;

    scaleMode = fitSize;
}

TjGanttChart::~TjGanttChart()
{
    delete header;
    delete chart;
    delete legend;
}

void
TjGanttChart::setDPI(int dx, int dy)
{
    dpiX = dx;
    dpiY = dy;

    // The line width should be roughly 1 pixel per 100 DPI.
    lineWidth = ((dpiX + dpiY) / 2) / 100;
    // Lines with even width look odd. So make sure we always have an odd line
    // width.
    if (lineWidth % 2 == 0)
        lineWidth++;
}

void
TjGanttChart::setProjectAndReportData(const QtReportElement* r,
                                      TaskList* tl, ResourceList* rl)
{
    reportElement = r;
    reportDef = static_cast<const QtReport*>(reportElement->getReport());
    project = reportDef->getProject();
    taskList = tl;
    resourceList = rl;
    scenario = reportElement->getScenario(0);
}

void
TjGanttChart::setSizes(const TjObjPosTable* opt, int hh, int ch, int w,
                       int mrh)
{
    objPosTable = opt;

    headerHeight = hh;
    chartHeight = ch;
    width = w;
    minRowHeight = mrh;

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

int
TjGanttChart::calcHeaderHeight()
{
    headerFont.setPixelSize(pointsToYPixels(7));
    QFontMetrics fm(headerFont);

    headerMargin = (int) (fm.height() * 0.15);
    // The header consists of 2 lines seperated by a line.
    return (2 * (fm.height() + 2 * headerMargin)) + lineWidth;
}

int
TjGanttChart::calcLegendHeight(int width)
{
    if (!legendLabels.isEmpty())
        return legendLabelRows * legendLabelHeight +
            (int) (legendLabelHeight * 0.3);

    legendLabels.append(i18n("Container Task"));
    legendLabels.append(i18n("Milestone"));
    legendLabels.append(i18n("Planned Task"));
    legendLabels.append(i18n("In-progress Task"));
    legendLabels.append(i18n("Completed Task"));

    maxLegendLabelWidth = 0;
    legendFont.setPixelSize(pointsToYPixels(7));
    QFontMetrics fm(legendFont);
    legendLabelHeight = (int) (fm.height() * 1.2);
    for (QStringList::Iterator it = legendLabels.begin();
         it != legendLabels.end(); ++it)
    {
        QRect br = fm.boundingRect(*it);
        br.setWidth((int) (br.width() + (int) legendLabelHeight * 0.2));
        if (maxLegendLabelWidth < br.width())
            maxLegendLabelWidth = br.width();
    }

    /* The symbols' width will be 4 * the height. The margin is half the label
     * height. We have a margin before the symbol, between the symbol and the
     * text and 2 times after the text. That's 2 times the label height as
     * margins. */
    int columns = width / (maxLegendLabelWidth + 6 * legendLabelHeight);

    legendLabelRows = legendLabels.count() / columns +
        (legendLabels.count() % columns != 0 ? 1 : 0);

    return (1 + (int) (1.5 * legendLabelRows)) * legendLabelHeight;
}

void
TjGanttChart::generate()
{
    // Clear ganttHeader canvas.
    QCanvasItemList cis = header->allItems();
    for (QCanvasItemList::Iterator it = cis.begin(); it != cis.end(); ++it)
        delete *it;

    // Clear chart canvas.
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
            }
            else if (pixelPerYear / (4 * minStepDay) >= 365)
            {
                stepUnit = day;
            }
            else if (pixelPerYear / (4 * minStepWeek) >= 52)
            {
                stepUnit = week;
            }
            else if (pixelPerYear / (4 * minStepMonth) >= 12)
            {
                stepUnit = month;
            }
            else if (pixelPerYear / (4 * minStepQuarter) >= 4)
            {
                stepUnit = quarter;
            }
            else
            {
                stepUnit = year;
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

    generateGanttTasks();
}

void
TjGanttChart::generateLegend(int width, int height)
{
    legend->resize(width, height);

    int col = 0;
    int row = 0;
    // The margin around or between all elements in pixels
    int margin = (int) (0.5 * legendLabelHeight);
    // The horizontal size of the graphical elements in pixels
    int symbolWidth = (int) (4.0 * legendLabelHeight);
    for (QStringList::Iterator it = legendLabels.begin();
         it != legendLabels.end(); ++it)
    {
        /* The elements of the legends are drawn in columns from left to
         * right. */
        int x = margin +
            col * (maxLegendLabelWidth + (int) (5.5 * legendLabelHeight));
        int yCenter = (int) (legendLabelHeight * (1.5 * row + 1.0));

        // Draw graphical element
        switch (col * legendLabelRows + row)
        {
            case 0:     // Container task
                drawContainterShape(x + 2 * margin,
                                    x + symbolWidth, yCenter,
                                    legendLabelHeight, false, legend);
                break;
            case 1:     // Milestone
                drawMilestoneShape(x + symbolWidth, yCenter,
                                   legendLabelHeight, false, legend);
                break;
            case 2:     // Planned task
                drawTaskShape(x + margin,
                              x + margin + symbolWidth, yCenter,
                              legendLabelHeight, 0, false, legend);
                break;
            case 3:     // In-progress task
                drawTaskShape(x + margin,
                              x + margin + symbolWidth, yCenter,
                              legendLabelHeight, symbolWidth / 2, false,
                              legend);
                break;
            case 4:     // Completed task
                drawTaskShape(x + margin,
                              x + margin + symbolWidth, yCenter,
                              legendLabelHeight, symbolWidth, false, legend);
                break;
        }

        // Draw description of graphical element
        QCanvasText* text = new QCanvasText(*it, legend);
        text->setColor(Qt::black);
        text->setFont(legendFont);
        text->setX(x + 2 * margin + symbolWidth);
        text->setY((int) (legendLabelHeight * (0.5 + 1.5 * row)));
        text->setZ(0);
        text->show();

        /* If we have finished a column advance to the top of the next right
         * column. */
        if (++row >= legendLabelRows)
        {
            row = 0;
            col++;
        }
    }
}

void
TjGanttChart::paintHeader(const QRect& clip, QPainter* p, bool dbuf)
{
    QRect vpSave = p->viewport();
    QRect vp;
    vp.setX(clip.x());
    vp.setY(clip.y());
    vp.setWidth(vpSave.width());
    vp.setHeight(vpSave.height());
    p->setViewport(vp);

    p->setClipping(true);
    p->setClipRect(clip);

    header->drawArea(QRect(0, 0, header->width(), header->height()), p, dbuf);

    p->setClipping(false);

    p->setViewport(vpSave);
}

void
TjGanttChart::paintChart(int x, int y, const QRect& clip, QPainter* p,
                         bool dbuf)
{
    QRect vpSave = p->viewport();
    QRect vp;
    vp.setX(clip.x() - x);
    vp.setY(clip.y() - y);
    vp.setWidth(vpSave.width());
    vp.setHeight(vpSave.height());
    p->setViewport(vp);

    p->setClipping(true);
    p->setClipRect(clip);

    chart->drawArea(QRect(0, 0, chart->width(), chart->height()), p, dbuf);

    p->setClipping(false);

    p->setViewport(vpSave);
}

void
TjGanttChart::paintLegend(const QRect& clip, QPainter* p, bool dbuf)
{
    QRect vpSave = p->viewport();
    QRect vp;
    vp.setX(clip.x());
    vp.setY(clip.y());
    vp.setWidth(vpSave.width());
    vp.setHeight(vpSave.height());
    p->setViewport(vp);

    p->setClipping(true);
    p->setClipRect(clip);

    legend->drawArea(QRect(0, 0, legend->width(), legend->height()), p, dbuf);

    p->setClipping(false);

    p->setViewport(vpSave);
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
    //generateGanttBackground();

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
        QString label;
        label = QString("%1").arg(hourOfDay(hour));
        drawHeaderCell(hour, hoursLater(1, hour), y, label);
    }
}

void
TjGanttChart::generateDayHeader(int y)
{
    for (time_t day = midnight(startTime);
         day < endTime; day = sameTimeNextDay(day))
    {
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
        drawHeaderCell(day, sameTimeNextDay(day), y, label);
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
        drawHeaderCell(week, sameTimeNextWeek(week), y,
                       (i18n("short for week of year", "W%1")
                        .arg(weekOfYear(week, weekStartsMonday))));
    }
}

void
TjGanttChart::generateMonthHeader(int y, bool withYear)
{
    for (time_t month = beginOfMonth(startTime);
         month < endTime; month = sameTimeNextMonth(month))
    {
         QString label = withYear ?
            QString("%1 %2").arg(QDate::shortMonthName(monthOfYear(month)))
            .arg(::year(month)) :
            QString("%1").arg(QDate::shortMonthName(monthOfYear(month)));
        drawHeaderCell(month, sameTimeNextMonth(month), y, label);
    }
}

void
TjGanttChart::generateQuarterHeader(int y)
{
    for (time_t quarter = beginOfQuarter(startTime);
         quarter < endTime; quarter =
         sameTimeNextQuarter(quarter))
    {
        drawHeaderCell(quarter, sameTimeNextQuarter(quarter), y,
                       i18n("short for quater of year", "Q%1")
                       .arg(quarterOfYear(quarter)));
    }
}

void
TjGanttChart::generateYearHeader(int y)
{
    for (time_t year = beginOfYear(startTime);
         year < endTime; year = sameTimeNextYear(year))
    {
        drawHeaderCell(year, sameTimeNextYear(year), y,
                       QString("%1").arg(::year(year)));
    }
}

void
TjGanttChart::drawHeaderCell(int start, int end, int y, const QString label)
{
    int xs = time2x(start);
    int xe = time2x(end);
    // Draw vertical line at beginning of week.
    QCanvasLine* line = new QCanvasLine(header);
    line->setPoints(xs, y, xs, y + headerHeight / 2);
    QPen pen = line->pen();
    pen.setColor(midCol);
    line->setPen(pen);
    line->setZ(TJRL_GRIDLINES);
    line->show();

    // Write week number.
    QCanvasText* text = new QCanvasText(label, header);
    text->setFont(headerFont);
    // Center the label horizontally in the cell.
    QFontMetrics fm(headerFont);
    QRect br = fm.boundingRect(label);
    // Make sure that at least some parts of the label are visible.
    if (xe > header->width())
        xe = header->width();
    if (xs < 0)
        xs = 0;
    if (xs + br.width() > xe)
        xs = xe - br.width();
    xs += ((xe - xs) - br.width()) / 2;
    text->move(xs + lineWidth / 2 + 1 + headerMargin, y + headerMargin);
    text->setZ(TJRL_GRIDLABLES);
    text->show();
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
        rect->setPen(QPen(col, lineWidth));
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
            rect->setPen(QPen(col, lineWidth));
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

void
TjGanttChart::generateGanttTasks()
{
    for (TjObjPosTableIterator it(*objPosTable); it.current(); ++it)
    {
        if (it.current()->getCoreAttributes()->getType() == CA_Task &&
            it.current()->getSubCoreAttributes() == 0)
        {
            const Task* t = static_cast<const Task*>
                (it.current()->getCoreAttributes());
            drawTask(t, false);
            drawDependencies(t);
        }
        else if (it.current()->getCoreAttributes()->getType() == CA_Resource
                 && it.current()->getSubCoreAttributes() == 0)
        {
            const Resource* r = static_cast<const Resource*>
                (it.current()->getCoreAttributes());
            drawResource(r);
        }
        else if (it.current()->getCoreAttributes()->getType() == CA_Task &&
                 it.current()->getSubCoreAttributes()->getType() == CA_Resource)
        {
            const Task* t = static_cast<const Task*>
                (it.current()->getCoreAttributes());
            const Resource* r = static_cast<const Resource*>
                (it.current()->getSubCoreAttributes());
            drawTaskResource(r, t);
        }
        else if (it.current()->getCoreAttributes()->getType() ==
                 CA_Resource &&
                 it.current()->getSubCoreAttributes()->getType() == CA_Task)
        {
            const Resource* r = static_cast<const Resource*>
                (it.current()->getCoreAttributes());
            const Task* t = static_cast<const Task*>
                (it.current()->getSubCoreAttributes());
            drawTask(t, r);
        }
        else
            qFatal("TjGanttChart::generateGanttTasks(): Unknown CA");
    }
}

void
TjGanttChart::drawTask(const Task* t, const Resource* r)
{
    int y;
    if (r)
        y = objPosTable->caToPos(r, t);
    else
        y = objPosTable->caToPos(t);

    int centerY = y + minRowHeight / 2;

    if (t->isMilestone())
    {
        int centerX = time2x(t->getStart(scenario));

        drawMilestoneShape(centerX, centerY, minRowHeight, r != 0, chart);
    }
    else if (t->isContainer())
    {
        int start = time2x(t->getStart(scenario));
        int end = time2x(t->getEnd(scenario));

        drawContainterShape(start, end, centerY, minRowHeight, r != 0, chart);
    }
    else
    {
        int start = time2x(t->getStart(scenario));
        int end = time2x(t->getEnd(scenario));

        /* TODO: This does not work 100% correct for effort or length
         * based tasks. It's only correct for duration tasks. */
        int barWidth = 0;
        if (t->getCompletionDegree(scenario) ==
            t->getCalcedCompletionDegree(scenario) &&
            reportDef->getProject()->getNow() < t->getEnd(scenario))
        {
            barWidth = time2x(reportDef->getProject()->getNow()) -
                start;
        }
        else
            barWidth = (int) ((end - start) *
                              (t->getCompletionDegree(scenario) / 100.0));

        drawTaskShape(start, end, centerY, minRowHeight, barWidth, r != 0,
                      chart);
    }
}

void
TjGanttChart::drawTaskShape(int start, int end, int centerY, int height,
                            int barWidth, bool outlineOnly, QCanvas* canvas)
{
    // A blue box with some fancy interior.
    QCanvasRectangle* rect =
        new QCanvasRectangle(start, centerY - (int) (height * 0.4),
                             end - start, (int) (height * 0.8), canvas);

    rect->setPen(QPen(QColor("#4C5EFF"), lineWidth));
    rect->setBrush(outlineOnly ? Qt::NoBrush :
                   QBrush(QColor("#4C5EFF"), Qt::Dense4Pattern));
    rect->setZ(TJRL_TASKS);
    rect->show();

    // The black progress bar.
    if (barWidth > 0)
    {
        rect = new QCanvasRectangle(start, centerY - (int) (height * 0.2),
                                    barWidth, (int) (height * 0.4), canvas);

        rect->setPen(QPen(Qt::black, lineWidth));
        rect->setBrush(QBrush(Qt::black,
                              outlineOnly ? Qt::NoBrush : Qt::SolidPattern));
        rect->setZ(TJRL_TASKCOMP);
        rect->show();
    }
}

void
TjGanttChart::drawMilestoneShape(int centerX, int centerY, int height,
                                 bool outlineOnly, QCanvas* canvas)
{
    int radius = (int) (height * 0.4);

    // A black diamond.
    QPointArray a(5);
    a.setPoint(0, centerX, centerY - radius);
    a.setPoint(1, centerX + radius, centerY);
    a.setPoint(2, centerX, centerY + radius);
    a.setPoint(3, centerX - radius, centerY);
    a.setPoint(4, centerX, centerY - radius);

    QCanvasPolygon* polygon = new QCanvasPolygon(canvas);
    polygon->setPoints(a);
    polygon->setBrush(QBrush(Qt::black,
                             outlineOnly ? Qt::NoBrush : Qt::SolidPattern));
    polygon->setZ(TJRL_TASKS);
    polygon->show();
}

void
TjGanttChart::drawContainterShape(int start, int end, int centerY, int height,
                                  bool outlineOnly, QCanvas* canvas)
{
    // A bar with jag at both ends.
    int jagWidth = (int) (height * 0.25);
    int top = centerY - (int) (height * 0.2);
    int halfbottom = centerY + (int) (height * 0.2);
    int bottom = halfbottom + jagWidth + (int) (height * 0.1);

    QPointArray a(9);
    a.setPoint(0, start - jagWidth, top);
    a.setPoint(1, start - jagWidth, halfbottom);
    a.setPoint(2, start, bottom);
    a.setPoint(3, start + jagWidth, halfbottom);
    a.setPoint(4, end - jagWidth, halfbottom);
    a.setPoint(5, end, bottom);
    a.setPoint(6, end + jagWidth, halfbottom);
    a.setPoint(7, end + jagWidth, top);
    a.setPoint(8, start - jagWidth, top);

    if (outlineOnly)
    {
        for (uint i = 0; i < a.count() - 1; ++i)
        {
            QCanvasLine* line = new QCanvasLine(canvas);
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
        QCanvasPolygon* polygon = new QCanvasPolygon(canvas);
        polygon->setPoints(a);
        polygon->setBrush(Qt::black);
        polygon->setZ(TJRL_TASKS);
        polygon->show();
    }
}

void
TjGanttChart::drawDependencies(const Task* t1)
{
#define abs(a) ((a) < 0 ? (-(a)) : (a))
#define min(a, b) ((a) < (b) ? (a) : (b))

    int arrowCounter = 0;
    TaskList sortedFollowers;

    /* To avoid unnecessary crossing of dependency arrows, we sort the
     * followers of the current task according to their absolute distance to
     * the Y position of this task in the list view. */
    int yPos = objPosTable->caToPos(t1) + minRowHeight / 2;
    for (TaskListIterator tli(t1->getFollowersIterator()); *tli; ++tli)
    {
        int t2y = objPosTable->caToPos(*tli);
        int i = 0;
        for (TaskListIterator stli(sortedFollowers); *stli; ++stli, ++i)
        {
            int t3y = objPosTable->caToPos(*stli);
            if (t3y < 0)
                continue;
            if (abs(yPos - t2y) > abs(yPos - t3y))
                break;
        }
        sortedFollowers.insert(i, *tli);
    }

    for (TaskListIterator tli(sortedFollowers); *tli; ++tli)
    {
        Task* t2 = *tli;
        int t2Top = objPosTable->caToPos(*tli);
        if (t2Top >= 0)
        {
            int t1x = time2x(t1->getEnd(scenario));
            int t2x = time2x(t2->getStart(scenario));
            if (t2->isMilestone())
                t2x -= (int) (minRowHeight * 0.4);
            else if (t2->isContainer())
                t2x -= 3;

            int t1y = objPosTable->caToPos(t1) + minRowHeight / 2;
            int t2y = objPosTable->caToPos(t2) + minRowHeight / 2;
            int yCenter = t1y < t2y ? t1y + (t2y - t1y) / 2 :
                t2y + (t1y - t2y) / 2;
            // Ensure that yCenter is between the task lines.
            //yCenter = (yCenter / objPosTable->caToPos(t2)) * itemHeight;

            // Draw connection line.
            // Distance between task end and the first break of the arrow.
            const int minGap = (int) (minRowHeight * 0.5);
            // Min distance between parallel arrors.
            const int arrowGap = (int) (minRowHeight * 0.2);
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
                QCanvasLine* line = new QCanvasLine(chart);
                line->setPen(QPen(Qt::black, lineWidth));
                int x1, y1, x2, y2;
                a.point(i, &x1, &y1);
                a.point(i + 1, &x2, &y2);
                line->setPoints(x1, y1, x2, y2);
                line->setZ(TJRL_DEPARROWS);
                line->show();
            }

            // Draw arrow head.
            const int arrowSize = (int) (minRowHeight * 0.3);
            a.resize(4);
            a.setPoint(0, t2x, t2y);
            a.setPoint(1, t2x - arrowSize, t2y - arrowSize);
            a.setPoint(2, t2x - arrowSize, t2y + arrowSize);
            a.setPoint(3, t2x, t2y);

            QCanvasPolygon* polygon = new QCanvasPolygon(chart);
            polygon->setPoints(a);
            polygon->setBrush(Qt::black);
            polygon->setZ(TJRL_DEPARROWS);
            polygon->show();
        }
    }
}

void
TjGanttChart::drawTaskResource(const Resource* r, const Task* t)
{
    Interval iv;
    int rY = objPosTable->caToPos(t, r);
    switch (stepUnit)
    {
        case hour:
            for (time_t i = beginOfHour(t->getStart(scenario));
                 i <= (t->getEnd(scenario)); i = hoursLater(1, i))
                drawResourceLoadColum(r, t, i, hoursLater(1, i) - 1, rY);
            break;
        case day:
            for (time_t i = midnight(t->getStart(scenario));
                 i <= (t->getEnd(scenario)); i = sameTimeNextDay(i))
                drawResourceLoadColum(r, t, i, sameTimeNextDay(i) - 1, rY);
            break;
        case week:
            for (time_t i = beginOfWeek(t->getStart(scenario),
                                        t->getProject()->
                                        getWeekStartsMonday());
                 i <= (t->getEnd(scenario)); i = sameTimeNextWeek(i))
                drawResourceLoadColum(r, t, i, sameTimeNextWeek(i) - 1, rY);
            break;
        case month:
            for (time_t i = beginOfMonth(t->getStart(scenario));
                 i <= (t->getEnd(scenario)); i = sameTimeNextMonth(i))
                drawResourceLoadColum(r, t, i, sameTimeNextMonth(i) - 1, rY);
            break;
        case quarter:
            for (time_t i = beginOfQuarter(t->getStart(scenario));
                 i <= (t->getEnd(scenario)); i = sameTimeNextQuarter(i))
                drawResourceLoadColum(r, t, i, sameTimeNextQuarter(i) - 1, rY);
            break;
        case year:
            for (time_t i = beginOfYear(t->getStart(scenario));
                 i <= (t->getEnd(scenario)); i = sameTimeNextYear(i))
                drawResourceLoadColum(r, t, i, sameTimeNextYear(i) - 1, rY);
            break;
        default:
            qFatal("TjGanttChart::drawTaskResource(): Unknown stepUnit");
            break;
    }
}

void
TjGanttChart::drawResource(const Resource* r)
{
    Interval iv;
    int rY = objPosTable->caToPos(r, 0);
    switch (stepUnit)
    {
        case hour:
            for (time_t i = beginOfHour(reportElement->getStart());
                 i <= reportElement->getEnd(); i = hoursLater(1, i))
                drawResourceLoadColum(r, 0, i, hoursLater(1, i) - 1, rY);
            break;
        case day:
            for (time_t i = midnight(reportElement->getStart());
                 i <= reportElement->getEnd(); i = sameTimeNextDay(i))
                drawResourceLoadColum(r, 0, i, sameTimeNextDay(i) - 1, rY);
            break;
        case week:
            for (time_t i = beginOfWeek(reportElement->getStart(),
                                        reportDef->getProject()->
                                        getWeekStartsMonday());
                 i <= (reportElement->getEnd()); i = sameTimeNextWeek(i))
                drawResourceLoadColum(r, 0, i, sameTimeNextWeek(i) - 1, rY);
            break;
        case month:
            for (time_t i = beginOfMonth(reportElement->getStart());
                 i <= (reportElement->getEnd()); i = sameTimeNextMonth(i))
                drawResourceLoadColum(r, 0, i, sameTimeNextMonth(i) - 1, rY);
            break;
        case quarter:
            for (time_t i = beginOfQuarter(reportElement->getStart());
                 i <= (reportElement->getEnd()); i = sameTimeNextQuarter(i))
                drawResourceLoadColum(r, 0, i, sameTimeNextQuarter(i) - 1, rY);
            break;
        case year:
            for (time_t i = beginOfYear(reportElement->getStart());
                 i <= (reportElement->getEnd()); i = sameTimeNextYear(i))
                drawResourceLoadColum(r, 0, i, sameTimeNextYear(i) - 1, rY);
            break;
        default:
            qFatal("TjGanttChart::drawTaskResource(): Unknown stepUnit");
            break;
    }
}

void
TjGanttChart::drawResourceLoadColum(const Resource* r, const Task* t,
                                    time_t start, time_t end, int rY)
{
    // Determin the width of the cell that we draw the column in.
    int cellStart = time2x(start);
    int cellEnd = time2x(end);

    // We will draw the load column into the cell with a margin of 1 pixel
    // plus the cell seperation line.
    // Let's try a first shot for column start and width.
    int cx = cellStart + mmToXPixels(0.5);
    int cw = cellEnd - cellStart - mmToXPixels(0.5) - 1;

    if (t)
    {
        /* Now we trim it so it does not extend over the ends of the task bar.
         * We also trim the interval so the load is only calculated for
         * intervals within the task period. */
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
    }

    // Since the above calculation might have destroyed our 0.5mm margin, we
    // check it again.
    if (cx < cellStart + mmToXPixels(0.5))
        cx = cellStart + mmToXPixels(0.5);
    if (cx + cw > cellEnd - mmToXPixels(0.5) - 1)
        cw = cellEnd - mmToXPixels(0.5) - 1 - cx;

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
    int colBottom = rY + minRowHeight - 1;
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
             chart);
        rect->setBrush(QBrush(thisTaskCol, Qt::Dense4Pattern));
        rect->setPen(QPen(thisTaskCol, lineWidth));
        rect->setZ(TJRL_LOADBARS);
        rect->show();
    }

    if (otherLoad > 0.0)
    {
        QCanvasRectangle* rect = new QCanvasRectangle
            (cx, colOtherLoadTop, cw,
             colTaskLoadTop - colOtherLoadTop, chart);
        rect->setBrush(QBrush(otherTaskCol, Qt::Dense6Pattern));
        rect->setPen(QPen(otherTaskCol, lineWidth));
        rect->setZ(TJRL_LOADBARS);
        rect->show();
    }

    if (freeLoad > 0.0)
    {
        QCanvasRectangle* rect = new QCanvasRectangle
            (cx, colTop, cw, colOtherLoadTop - colTop,
             chart);
        rect->setBrush(QBrush(freeLoadCol, Qt::Dense6Pattern));
        rect->setPen(QPen(freeLoadCol, lineWidth));
        rect->setZ(TJRL_LOADBARS);
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

int
TjGanttChart::mmToXPixels(double mm)
{
    return (int) ((mm / 25.4) * dpiX);
}

int
TjGanttChart::mmToYPixels(double mm)
{
    return (int) ((mm / 25.4) * dpiY);
}

int
TjGanttChart::pointsToYPixels(double pts)
{
    return (int) ((pts * (0.376 / 25.4)) * dpiY);
}


