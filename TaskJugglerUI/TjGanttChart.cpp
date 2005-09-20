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
#include <qpainter.h>

#include <klocale.h>
#include <kglobal.h>

#include "Project.h"
#include "Task.h"
#include "Resource.h"
#include "TaskList.h"
#include "ResourceList.h"
#include "QtReport.h"
#include "QtReportElement.h"
#include "TjObjPosTable.h"
#include "TjGanttZoomStep.h"

TjGanttChart::TjGanttChart(QObject* obj)
{
    header = new QCanvas(obj);
    chart = new QCanvas(obj);
    legend = new QCanvas(obj);

    headerHeight = 0;
    chartHeight = 0;
    width = 0;
    minRowHeight = 0;

    project = 0;
    objPosTable = 0;

    dpiX = dpiY = 0;
    headerMargin = 0;
    startTime = endTime = 0;

    colors["headerBackgroundCol"] = Qt::white;
    colors["headerLineCol"] = Qt::black;
    colors["headerShadowCol"] = Qt::white;
    colors["chartBackgroundCol"] = Qt::white;
    colors["chartAltBackgroundCol"] = Qt::white;
    colors["chartTimeOffCol"] = QColor("#CECECE");
    colors["chartLineCol"] = QColor("#4F4F4F");
    colors["todayLineCol"] = Qt::red;
    colors["taskCol"] = QColor("#4C5EFF");
    colors["milestoneCol"] = Qt::black;
    colors["containerCol"] = Qt::black;
    colors["completionCol"] = Qt::black;
    colors["depLineCol"] = QColor("#4C5EFF");
    colors["taskLoadCol"] = QColor("#FD13C6");
    colors["otherLoadCol"] = QColor("#FF8D13");
    colors["freeLoadCol"] = QColor("#00AC00");

    currentZoomStep = 0;
    clipped = false;
}

TjGanttChart::~TjGanttChart()
{
    delete header;
    delete chart;
    delete legend;
}

void
TjGanttChart::setProjectAndReportData(const QtReportElement* r)
{
    reportElement = r;
    reportDef = static_cast<const QtReport*>(reportElement->getReport());
    project = reportDef->getProject();
    scenario = reportElement->getScenario(0);
}

void
TjGanttChart::setSizes(const TjObjPosTable* opt, int hh, int ch, int w,
                       int mrh)
{
    // Make sure that setProjectAndReportData() has been called first.
    assert(project != 0);

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
TjGanttChart::setDPI(int dx, int dy)
{
    // Make sure that setProjectAndReportData() has been called first.
    assert(project != 0);

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
TjGanttChart::setColor(const char* name, QColor col)
{
    assert(colors.find(name) != colors.end());

    colors[name] = col;
}

int
TjGanttChart::calcHeaderHeight()
{
    // Make sure setDPI() has been called first.
    assert (dpiX > 0 && dpiY > 0);

    headerFont.setPixelSize(pointsToYPixels(7));
    QFontMetrics fm(headerFont);

    calcStepSizes();

    headerMargin = (int) (fm.height() * 0.15);
    // The header consists of 2 lines seperated by a line.
    return (2 * (fm.height() + 2 * headerMargin)) + lineWidth;
}

void
TjGanttChart::setHeaderHeight(int hh)
{
    // Make sure setDPI() has been called first.
    assert (dpiX > 0 && dpiY > 0);

    // Same as in calcHeaderHeight(). textHeight is fm.height()
    int textHeight = (int) ((hh - lineWidth) / (2 * (1 + 2 * 0.15)));
    int fontSize = textHeight;
    int fontHeight;
    do
    {
        headerFont.setPixelSize(--fontSize);
        QFontMetrics fm(headerFont);
        fontHeight = fm.height();
    } while (fontHeight > textHeight);

    calcStepSizes();

    headerMargin = (int) (fontHeight * 0.15);
}

void
TjGanttChart::calcStepSizes()
{
    // Remove and delete all entries from the zoomStep list.
    for (std::vector<TjGanttZoomStep*>::iterator it = zoomSteps.begin();
         it != zoomSteps.end(); ++it)
        delete *it;
    zoomSteps.clear();

    zoomSteps.push_back(new TjGanttZoomStep
                        (TjGanttZoomStep::day,
                         "WWW WWW 00, 0000", "%a %b %d, %Y",
                         TjGanttZoomStep::hour,
                         "00", "%H", 24, reportDef->getWeekStartsMonday(),
                         headerFont));
    zoomSteps.push_back(new TjGanttZoomStep
                        (TjGanttZoomStep::month,
                         "WWW 0000", "%b %Y",
                         TjGanttZoomStep::day,
                         "00", "%d", 31, reportDef->getWeekStartsMonday(),
                         headerFont));
    zoomSteps.push_back(new TjGanttZoomStep
                        (TjGanttZoomStep::month,
                         "WWW 0000", "%b %Y",
                         TjGanttZoomStep::day,
                         "00", "%d", 31, reportDef->getWeekStartsMonday(),
                         headerFont));
    zoomSteps.push_back(new TjGanttZoomStep
                        (TjGanttZoomStep::month,
                         "WWW 0000", "%b %Y",
                         TjGanttZoomStep::day,
                         "00", "%d", 31, reportDef->getWeekStartsMonday(),
                         headerFont));
    zoomSteps.push_back(new TjGanttZoomStep
                        (TjGanttZoomStep::month,
                         "WWW 0000", "%b %Y",
                         TjGanttZoomStep::day,
                         "W", "#w", 31, reportDef->getWeekStartsMonday(),
                         headerFont));
    zoomSteps.push_back(new TjGanttZoomStep
                        (TjGanttZoomStep::month,
                         "0000", "%b %Y",
                         TjGanttZoomStep::week,
                         "W00", i18n("W#W"), 5,
                         reportDef->getWeekStartsMonday(),
                         headerFont));
    zoomSteps.push_back(new TjGanttZoomStep
                        (TjGanttZoomStep::month,
                         "0000", "%b %Y",
                         TjGanttZoomStep::week,
                         "W00", i18n("W#W"), 5,
                         reportDef->getWeekStartsMonday(),
                         headerFont));
    zoomSteps.push_back(new TjGanttZoomStep
                        (TjGanttZoomStep::year,
                         "0000", "%Y",
                         TjGanttZoomStep::month,
                         "WWW", "%b", 12, reportDef->getWeekStartsMonday(),
                         headerFont));
    zoomSteps.push_back(new TjGanttZoomStep
                        (TjGanttZoomStep::year,
                         "0000", "%Y",
                         TjGanttZoomStep::month,
                         "00", "%m", 12, reportDef->getWeekStartsMonday(),
                         headerFont));
    zoomSteps.push_back(new TjGanttZoomStep
                        (TjGanttZoomStep::year,
                         "0000", "%Y",
                         TjGanttZoomStep::quarter,
                         "W0", i18n("Q#Q"), 4,
                         reportDef->getWeekStartsMonday(),
                         headerFont));
    zoomSteps.push_back(new TjGanttZoomStep
                        (TjGanttZoomStep::year, "", "",
                         TjGanttZoomStep::year, "0000", "%Y", 1,
                         reportDef->getWeekStartsMonday(),
                         headerFont));
    zoomSteps.push_back(new TjGanttZoomStep
                        (TjGanttZoomStep::year, "", "",
                         TjGanttZoomStep::year, "0000", "%Y", 1,
                         reportDef->getWeekStartsMonday(),
                         headerFont));

    int ppyHint = 24 * 365;
    for (std::vector<TjGanttZoomStep*>::iterator it = zoomSteps.begin();
         it != zoomSteps.end(); ++it)
    {
        ppyHint = (int) ((*it)->calcStepSize(ppyHint) / 2.2);
    }
}

int
TjGanttChart::calcLegendHeight(int width)
{
    // Make sure setDPI() has been called first.
    assert (dpiX > 0 && dpiY > 0);

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
TjGanttChart::generate(ScaleMode scaleMode)
{
    // Make sure setSizes() has been called first();
    assert(objPosTable != 0);
    /* Make sure that setHeaderHeight() or calcHeaderHeight() have been called
     * first. */
    assert(headerMargin > 0);

    // Clear ganttHeader canvas.
    QCanvasItemList cis = header->allItems();
    for (QCanvasItemList::Iterator it = cis.begin(); it != cis.end(); ++it)
        delete *it;

    // Clear chart canvas.
    cis = chart->allItems();
    for (QCanvasItemList::Iterator it = cis.begin(); it != cis.end(); ++it)
        delete *it;

    switch (scaleMode)
    {
        case autoZoom:
        {
            startTime = reportElement->getStart();
            endTime = reportElement->getEnd();
            /* In case the user has not choosen a report interval that differs
             * from the project interval, we use the best fit interval. */
            if (startTime == reportDef->getProject()->getStart() &&
                endTime == reportDef->getProject()->getEnd())
                allTasksInterval();

            zoomToFitWindow(width, endTime - startTime);

            endTime = x2time(width);
            break;
        }
        case fitSize:
        {
            allTasksInterval();
            zoomToFitWindow(header->width(), endTime - startTime);
            break;
        }
        case manual:
        {
            if (startTime == 0)
                startTime = reportElement->getStart();
            if (endTime == 0)
                endTime = reportElement->getEnd();
            if (clipped)
            {
                endTime = unclippedEndTime;
                clipped = false;
            }

            break;
        }
        default:
            assert(0);
    }

    /* Some of the algorithems here require a mininum project duration of 1 day
     * to work properly. */
    if (endTime - startTime < 60 * 60 * 24)
        endTime = startTime + 60 * 60 * 24 + 1;

    /* QCanvasView can only handle 32767 pixel width. So we have to shorten
     * the report period if the chart exceeds this size. */
    if (time2x(endTime) > 32767)
    {
        unclippedEndTime = endTime;
        clipped = true;
        endTime = x2time(32767 - 2);
    }

    width = time2x(endTime);

    // Resize header canvas to new size.
    header->resize(width, headerHeight);

    // Resize chart canvas to new size.
    chart->resize(width, chartHeight);

    generateHeaderAndGrid();

    generateGanttElements();

    header->update();
    chart->update();
}

void
TjGanttChart::generateLegend(int width, int height)
{
    // Make sure setDPI() has been called first.
    assert (dpiX > 0 && dpiY > 0);

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
    // Make sure generate() has been called first.
    assert(startTime > 0 && endTime > 0);

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
    // Make sure generate() has been called first.
    assert(startTime > 0 && endTime > 0);

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

Interval
TjGanttChart::stepInterval(time_t ref) const
{
    assert(zoomSteps.size() > 0);

    Interval iv;
    switch (zoomSteps[currentZoomStep]->getStepUnit(false))
    {
        case TjGanttZoomStep::hour:
            iv.setStart(beginOfHour(ref));
            iv.setEnd(hoursLater(1, iv.getStart()) - 1);
            break;
        case TjGanttZoomStep::day:
            iv.setStart(midnight(ref));
            iv.setEnd(sameTimeNextDay(iv.getStart()) - 1);
            break;
        case TjGanttZoomStep::week:
            iv.setStart(beginOfWeek
                        (ref, reportDef->getProject()->getWeekStartsMonday()));
            iv.setEnd(sameTimeNextWeek(iv.getStart()) - 1);
            break;
        case TjGanttZoomStep::month:
            iv.setStart(beginOfMonth(ref));
            iv.setEnd(sameTimeNextMonth(iv.getStart()) - 1);
            break;
        case TjGanttZoomStep::quarter:
            iv.setStart(beginOfQuarter(ref));
            iv.setEnd(sameTimeNextQuarter(iv.getStart()) - 1);
            break;
        case TjGanttZoomStep::year:
            iv.setStart(beginOfYear(ref));
            iv.setEnd(sameTimeNextYear(iv.getStart()) - 1);
            break;
        default:
            assert(0);
    }
    return iv;
}

QString
TjGanttChart::stepIntervalName(time_t ref) const
{
    assert(zoomSteps.size() > 0);

    QString name;
    switch (zoomSteps[currentZoomStep]->getStepUnit(false))
    {
        case TjGanttZoomStep::hour:
            name = time2user(beginOfHour(ref), "%k:%M " +
                             KGlobal::locale()->dateFormat());
            break;
        case TjGanttZoomStep::day:
            name = time2user(midnight(ref), KGlobal::locale()->dateFormat());
            break;
        case TjGanttZoomStep::week:
        {
            bool wsm = reportDef->getProject()->getWeekStartsMonday();
            name = i18n("Week %1, %2").arg(weekOfYear(ref, wsm))
                .arg(::year(ref));
            break;
        }
        case TjGanttZoomStep::month:
            name = QString("%1 %2").arg(QDate::shortMonthName(monthOfYear(ref)))
                .arg(::year(ref));
            break;
        case TjGanttZoomStep::quarter:
            name = QString("Q%1 %2").arg(quarterOfYear(ref)).arg(::year(ref));
            break;
        case TjGanttZoomStep::year:
            name = QString().sprintf("%d", ::year(ref));
            break;
        default:
            assert(0);
    }
    return name;
}

bool
TjGanttChart::zoomIn()
{
    assert(zoomSteps.size() > 0);

    if (currentZoomStep == 0)
        return false;

    currentZoomStep--;
    generate(manual);

    return true;
}

bool
TjGanttChart::zoomOut()
{
    assert(zoomSteps.size() > 0);

    if (currentZoomStep >= zoomSteps.size() - 1)
        return false;

    currentZoomStep++;
    generate(manual);

    return true;
}

void
TjGanttChart::generateHeaderAndGrid()
{
    // Draw divider line between the two header lines
    QCanvasLine* line = new QCanvasLine(header);
    line->setPoints(0, headerHeight / 2, width, headerHeight / 2);
    QPen pen = line->pen();
    pen.setColor(colors["headerLineCol"]);
    line->setPen(pen);
    line->setZ(TJRL_GRIDLINES);
    line->show();
/*
    line = new QCanvasLine(header);
    line->setPoints(0, headerHeight - 1, width, headerHeight - 1);
    pen = line->pen();
    pen.setColor(colors["headerBackgroundCol"]);
    line->setPen(pen);
    line->setZ(TJRL_BACKGROUND);
    line->show();
*/
    // Fill header background
    QCanvasRectangle* rect =
        new QCanvasRectangle(0, 0, width, headerHeight - 1, header);
    pen = rect->pen();
    pen.setColor(colors["headerBackgroundCol"]);
    rect->setPen(pen);
    QBrush brush = rect->brush();
    brush.setStyle(QBrush::SolidPattern);
    brush.setColor(colors["headerBackgroundCol"]);
    rect->setBrush(brush);
    rect->setZ(TJRL_BACKGROUND);
    rect->show();

    generateHeaderLine(0);
    generateHeaderLine(headerHeight / 2);
    TjGanttZoomStep* czs = zoomSteps[currentZoomStep];
    switch (czs->getStepUnit(false))
    {
        case TjGanttZoomStep::hour:
            markHourBoundaries(1);
            markNonWorkingHoursOnBackground();
            break;
        case TjGanttZoomStep::day:
            markDayBoundaries();
            if (czs->getPixelsPerYear() > 365 * 24 * 2)
                markNonWorkingHoursOnBackground();
            else
                markNonWorkingDaysOnBackground();
            if (czs->getPixelsPerYear() > 365 * 8 * 8)
                markHourBoundaries(3);
            break;
        case TjGanttZoomStep::week:
            markNonWorkingDaysOnBackground();
            markWeekBoundaries();
            if (czs->getPixelsPerYear() > 365 * 10)
                markDayBoundaries();
            break;
        case TjGanttZoomStep::month:
            markMonthsBoundaries();
            // Ensure that we have at least 2 pixels per day.
            if (czs->getPixelsPerYear() > 52 * 14)
                markNonWorkingDaysOnBackground();
            break;
        case TjGanttZoomStep::quarter:
            markQuarterBoundaries();
            if (czs->getPixelsPerYear() > 12 * 20)
                markMonthsBoundaries();
            break;
        case TjGanttZoomStep::year:
            // Ensure that we have at least 20 pixels per month or per
            // quarter.
            if (czs->getPixelsPerYear() > 20 * 12)
                markMonthsBoundaries();
            else if (czs->getPixelsPerYear() >= 20 * 4)
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
TjGanttChart::generateHeaderLine(int y)
{
    assert(zoomSteps.size() > 0);

    TjGanttZoomStep* czs = zoomSteps[currentZoomStep];
    bool first = true;

    for (time_t t = czs->intervalStart(y == 0, startTime); t < endTime;
         t = czs->nextIntervalStart(y == 0, t))
    {
        QString format = czs->getFormat(y == 0);

        int pos;
        if ((pos = format.find("#w")) >= 0)
            format.replace
                (pos, 2, QString("%1").
                 arg(time2user(t, "%a")[0]));
        else if ((pos = format.find("#W")) >= 0)
            format.replace
                (pos, 2, QString("%1").
                 arg(weekOfYear(t, czs->getWeekStartsMonday())));
        else if ((pos = format.find("#Q")) >= 0)
            format.replace(pos, 2, QString("%1").arg(quarterOfYear(t)));

        drawHeaderCell(t, czs->nextIntervalStart(y == 0, t), y,
                       time2user(t, format),
                       first);
        first = false;
    }
}

void
TjGanttChart::drawHeaderCell(int start, int end, int y, const QString label,
                             bool first)
{
    int xs = time2x(start);
    int xe = time2x(end);
    if (!first)
    {
        // Draw vertical line at beginning of cell.
        QCanvasLine* line = new QCanvasLine(header);
        line->setPoints(xs, y, xs, y + headerHeight / 2);
        QPen pen = line->pen();
        pen.setColor(colors["headerLineCol"]);
        line->setPen(pen);
        line->setZ(TJRL_GRIDLINES);
        line->show();
    }

    // Write week number.
    QCanvasText* text = new QCanvasText(label, header);
    text->setFont(headerFont);
    // Center the label horizontally in the cell.
    QFontMetrics fm(headerFont);
    QRect br = fm.boundingRect(label);
    br.setWidth(br.width() + 2 * headerMargin);
    // Make sure that at least some parts of the label are visible.
    if (xe > header->width())
        xe = header->width();
    if (xs < 0)
        xs = 0;
    // Make sure the label does not overwrite the 2nd cell
    if (xs == 0 && xs + br.width() > xe)
        xs = xe - br.width();
    // Make sure that the label starts at the left cell end in case the last
    // cell does not fit in the header anymore.
    if (xe == header->width() && xs + br.width() > xe)
        xe = xs + br.width();
    xs += ((xe - xs) - br.width()) / 2;
    text->move(xs, y + headerMargin);
    text->setZ(TJRL_GRIDLABLES);
    text->show();
}

void
TjGanttChart::markNonWorkingHoursOnBackground()
{
    QColor col = colors["chartTimeOffCol"];
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
    QColor col = colors["chartTimeOffCol"];
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
        col = colors["todayLineCol"];
    else
        col = colors["chartLineCol"];

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
    QColor bgColor = colors["chartBackgroundCol"];
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
        rect->setBrush(QBrush(colors["chartAltBackgroundCol"]));
        rect->setZ(TJRL_BACKLINES);
        rect->show();
    }
}

void
TjGanttChart::generateGanttElements()
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
            assert(0);
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
    /* Workaround for a QCanvasView problem. In Qt3.x it can only handle 32767
     * pixels per dimension. */
    if (start < 0)
        start = 0;
    else if (start > 32767)
        start = 32767;
    if (end < 0)
        end = 0;
    else if (end > 32767)
        end = 32767;

    // A blue box with some fancy interior.
    QCanvasRectangle* rect =
        new QCanvasRectangle(start, centerY - (int) (height * 0.35),
                             end - start, (int) (height * 0.7) + 1, canvas);

    rect->setPen(QPen(colors["taskCol"], lineWidth));
    rect->setBrush(QBrush(colors["taskCol"], outlineOnly ?
                          Qt::Dense6Pattern : Qt::Dense4Pattern));
    rect->setZ(TJRL_TASKS);
    rect->show();

    // The black progress bar.
    if (barWidth > 0)
    {
        rect = new QCanvasRectangle(start, centerY - (int) (height * 0.15),
                                    barWidth, (int) (height * 0.3) + 1, canvas);

        rect->setPen(QPen(colors["completionCol"], lineWidth));
        rect->setBrush(QBrush(colors["completionCol"], outlineOnly ?
                              Qt::Dense4Pattern : Qt::SolidPattern));
        rect->setZ(TJRL_TASKCOMP);
        rect->show();
    }
}

void
TjGanttChart::drawMilestoneShape(int centerX, int centerY, int height,
                                 bool outlineOnly, QCanvas* canvas)
{
    int radius = (int) (height * 0.375);

    // A black diamond.
    QPointArray a(5);
    a.setPoint(0, centerX, centerY - radius);
    a.setPoint(1, centerX + radius, centerY);
    a.setPoint(2, centerX, centerY + radius);
    a.setPoint(3, centerX - radius, centerY);
    a.setPoint(4, centerX, centerY - radius);

    QCanvasPolygon* polygon = new QCanvasPolygon(canvas);
    polygon->setPoints(a);
    polygon->setBrush(QBrush(colors["milestoneCol"], outlineOnly ?
                             Qt::Dense4Pattern : Qt::SolidPattern));
    polygon->setZ(TJRL_TASKS);
    polygon->show();
}

void
TjGanttChart::drawContainterShape(int start, int end, int centerY, int height,
                                  bool outlineOnly, QCanvas* canvas)
{
    // A bar with jag at both ends.
    int jagWidth = (int) (height * 0.25);
    int top = centerY - (int) (height * 0.15);
    int halfbottom = centerY + (int) (height * 0.15);
    int bottom = halfbottom + jagWidth;

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

    /* Workaround for a QCanvasView problem. In Qt3.x it can only handle 32767
     * pixels per dimension. */
    for (uint i = 0; i < a.count(); ++i)
    {
        QPoint p = a[i];
        if (p.x() < 0)
            p.setX(0);
        if (p.x() > 32767)
            p.setX(32767);
        a[i] = p;
    }

    /* QCanvasPolygon does not draw a solid perimeter. So we have to do this
     * on our own. */
    if (outlineOnly)
    {
        for (uint i = 0; i < a.count() - 1; ++i)
        {
            QCanvasLine* line = new QCanvasLine(canvas);
            QPen pen(colors["containerCol"]);
            line->setPen(pen);
            int x1, y1, x2, y2;
            a.point(i, &x1, &y1);
            a.point(i + 1, &x2, &y2);
            line->setPoints(x1, y1, x2, y2);
            line->setZ(TJRL_TASKS);
            line->show();
        }
    }

    QCanvasPolygon* polygon = new QCanvasPolygon(canvas);
    polygon->setPoints(a);
    polygon->setBrush(QBrush(colors["containerCol"], outlineOnly ?
                             Qt::Dense4Pattern : Qt::SolidPattern));
    polygon->setZ(TJRL_TASKS);
    polygon->show();
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
            if (t1->isMilestone())
                t1x += (int) (minRowHeight * 0.4);
            else if (t1->isContainer())
                t1x += (int) (minRowHeight * 0.25);

            int t2x = time2x(t2->getStart(scenario));
            if (t2->isMilestone())
                t2x -= (int) (minRowHeight * 0.4);
            else if (t2->isContainer())
                t2x -= (int) (minRowHeight * 0.25);

            int t1y = objPosTable->caToPos(t1) + minRowHeight / 2;
            int t2y = objPosTable->caToPos(t2) + minRowHeight / 2;
            int yCenter = t1y < t2y ? t1y + (t2y - t1y) / 2 :
                t2y + (t1y - t2y) / 2;

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

            /* Workaround for a QCanvasView problem. In Qt3.x it can only
             * handle 32767 pixels per dimension. */
            for (uint i = 0; i < a.count(); ++i)
            {
                QPoint p = a[i];
                if (p.x() < 0)
                    p.setX(0);
                if (p.x() > 32767)
                    p.setX(32767);
                a[i] = p;
            }

            for (uint i = 0; i < a.count() - 1; ++i)
            {
                QCanvasLine* line = new QCanvasLine(chart);
                line->setPen(QPen(colors["depLineCol"], lineWidth));
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
            polygon->setBrush(colors["depLineCol"]);
            polygon->setZ(TJRL_DEPARROWS);
            polygon->show();
        }
    }
}

void
TjGanttChart::drawTaskResource(const Resource* r, const Task* t)
{
    assert(zoomSteps.size() > 0);

    /* For each time interval we draw a column that represents the load of the
     * resource allocated to the task. The end columns are trimmed to be in
     * the same horizontal interval as the task bar. */
    Interval iv;
    TjGanttZoomStep* czs = zoomSteps[currentZoomStep];
    int rY = objPosTable->caToPos(t, r);
    for (time_t i = czs->intervalStart(false, t->getStart(scenario));
         i <= (t->getEnd(scenario)); i = czs->nextIntervalStart(false, i))
        drawResourceLoadColum(r, t, i, czs->nextIntervalStart(false, i) - 1,
                              rY);
}

void
TjGanttChart::drawResource(const Resource* r)
{
    assert(zoomSteps.size() > 0);

    /* For each horizontal interval of the Gantt chart we draw a column that
     * represents the load of the resource for that interval. */
    Interval iv;
    TjGanttZoomStep* czs = zoomSteps[currentZoomStep];
    int rY = objPosTable->caToPos(r, 0);
    for (time_t i = czs->intervalStart(false, reportElement->getStart());
         i <= reportElement->getEnd(); i = czs->nextIntervalStart(false, i))
        drawResourceLoadColum(r, 0, i, czs->nextIntervalStart(false, i) - 1, rY);
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

    // Now we draw the columns. But only if the load is larger than 0.
    if (taskLoad > 0.0)
    {
        // Load for this task.
        QCanvasRectangle* rect = new QCanvasRectangle
            (cx, colTaskLoadTop, cw, colBottom - colTaskLoadTop,
             chart);
        rect->setBrush(QBrush(colors["taskLoadCol"], Qt::Dense4Pattern));
        rect->setPen(QPen(colors["taskLoadCol"], lineWidth));
        rect->setZ(TJRL_LOADBARS);
        rect->show();
    }

    if (otherLoad > 0.0)
    {
        // Load for other tasks.
        QCanvasRectangle* rect = new QCanvasRectangle
            (cx, colOtherLoadTop, cw,
             colTaskLoadTop - colOtherLoadTop, chart);
        rect->setBrush(QBrush(colors["otherLoadCol"], Qt::Dense4Pattern));
        rect->setPen(QPen(colors["otherLoadCol"], lineWidth));
        rect->setZ(TJRL_LOADBARS);
        rect->show();
    }

    if (freeLoad > 0.0)
    {
        // Unallocated load.
        QCanvasRectangle* rect = new QCanvasRectangle
            (cx, colTop, cw, colOtherLoadTop - colTop,
             chart);
        rect->setBrush(QBrush(colors["freeLoadCol"], Qt::Dense4Pattern));
        rect->setPen(QPen(colors["freeLoadCol"], lineWidth));
        rect->setZ(TJRL_LOADBARS);
        rect->show();
    }
}

int
TjGanttChart::time2x(time_t t) const
{
    assert(zoomSteps.size() > 0);

    return (int) ((t - startTime) *
                  (((float) zoomSteps[currentZoomStep]->getPixelsPerYear())
                   / (60 * 60 * 24 * 365)));
}

time_t
TjGanttChart::x2time(int x) const
{
    assert(zoomSteps.size() > 0);

    return (time_t) (startTime + ((float) x * 60 * 60 * 24 * 365) /
                     zoomSteps[currentZoomStep]->getPixelsPerYear());
}

void
TjGanttChart::zoomToFitWindow(int width, time_t duration)
{
    double years = static_cast<double>(duration) / (60 * 60 * 24 * 365);
    currentZoomStep = 0;
    for (std::vector<TjGanttZoomStep*>::iterator it = zoomSteps.begin();
         it != zoomSteps.end(); ++it)
    {
        if ((*it)->getPixelsPerYear() * years < width)
            return;
        currentZoomStep++;
    }
}

void
TjGanttChart::allTasksInterval()
{
    /* Find the ealiest start of any task and the latest end of any
     * task. */
    startTime = reportElement->getEnd();
    endTime = reportElement->getStart();
    for (TjObjPosTableIterator it(*objPosTable); it.current(); ++it)
    {
        const Task* t;
        if (it.current()->getCoreAttributes()->getType() == CA_Task)
            t = static_cast<const Task*>
                (it.current()->getCoreAttributes());
        else if (it.current()->getSubCoreAttributes() &&
                 it.current()->getSubCoreAttributes()->getType() ==
                 CA_Task)
            t = static_cast<const Task*>
                (it.current()->getSubCoreAttributes());
        else
            continue;
        if (t->getStart(scenario) < startTime)
            startTime = t->getStart(scenario);
        if (t->getEnd(scenario) > endTime)
            endTime = t->getEnd(scenario);
    }
    if (startTime <= endTime)
    {
        /* Add 5% of the interval duration to both ends, so the tasks are not
         * cut off right at the end. */
        int duration = endTime - startTime;
        startTime -= (time_t) (0.05 * duration);
        endTime += (time_t) (0.05 * duration);
    }
    else
    {
        /* The report does not contain any tasks. So we simply use the
         * requested interval. */
        startTime = reportElement->getStart();
        endTime = reportElement->getEnd();
    }

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


