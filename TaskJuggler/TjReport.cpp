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

#include <assert.h>

#include <qsplitter.h>
#include <qlayout.h>
#include <qfont.h>
#include <qheader.h>
#include <qcanvas.h>
#include <qdatetime.h>
#include <qtimer.h>
#include <qpopupmenu.h>

#include <klistview.h>
#include <klocale.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kapp.h>
#include <kcursor.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <ktextbrowser.h>

#include "Project.h"
#include "Task.h"
#include "Resource.h"
#include "Journal.h"
#include "Utility.h"
#include "ExpressionTree.h"
#include "Report.h"
#include "TableColumnFormat.h"
#include "QtTaskReport.h"
#include "QtTaskReportElement.h"
#include "QtResourceReport.h"
#include "QtResourceReportElement.h"
#include "ReportLayers.h"
#include "RichTextDisplay.h"

//                                           Boundary
const int TjReport::minStepHour = 20;    //   365 * 24 * 20 = 175200
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
    41946, // Hour
    176000
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
    listView->setAllColumnsShowFocus(TRUE);

    canvasFrame = new QWidget(splitter);
    QVBoxLayout* vl = new QVBoxLayout(canvasFrame, 0, 0);
    canvasFrame->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);

    ganttHeader = new QCanvas(this);
    ganttHeaderView = new QCanvasView(ganttHeader, canvasFrame);
    ganttHeaderView->setHScrollBarMode(QScrollView::AlwaysOff);
    ganttHeaderView->setVScrollBarMode(QScrollView::AlwaysOff);

    ganttChart = new QCanvas(this);
    ganttChart->setBackgroundColor(listView->colorGroup().base());
    ganttChartView = new QCanvasView(ganttChart, canvasFrame);

    currentZoomStep = 5;
    stepUnit = week;
    pixelPerYear = zoomSteps[currentZoomStep];
    startTime = endTime = 0;

    vl->addWidget(ganttHeaderView);
    vl->addWidget(ganttChartView);
    hl->addWidget(splitter);

    statusBarUpdateTimer = 0;

    connect(listView, SIGNAL(expanded(QListViewItem*)),
            this, SLOT(expandReportItem(QListViewItem*)));
    connect(listView, SIGNAL(collapsed(QListViewItem*)),
            this, SLOT(collapsReportItem(QListViewItem*)));
    connect(listView, SIGNAL(clicked(QListViewItem*, const QPoint&, int)),
            this, SLOT(listClicked(QListViewItem*, const QPoint&, int)));
    connect(listView,
            SIGNAL(rightButtonPressed(QListViewItem*, const QPoint&, int)),
            this, SLOT(doPopupMenu(QListViewItem*, const QPoint&, int)));
    connect(ganttChartView, SIGNAL(contentsMoving(int, int)),
            this, SLOT(syncVSlidersGantt2List(int, int)));
    connect(listView, SIGNAL(contentsMoving(int, int)),
            this, SLOT(syncVSlidersList2Gantt(int, int)));
}

TjReport::~TjReport()
{
    delete statusBarUpdateTimer;
}

bool
TjReport::event(QEvent* ev)
{
    // Regenerate the chart in case of a palette change.
    if (ev->type() == QEvent::ApplicationPaletteChange)
    {
        ganttChart->setBackgroundColor(listView->colorGroup().base());
        regenerateChart();
    }

    return QWidget::event(ev);
}

bool
TjReport::generateReport()
{
    setLoadingProject(TRUE);

    setCursor(KCursor::waitCursor());
    if (!this->generateList())
    {
        setLoadingProject(FALSE);
        setCursor(KCursor::arrowCursor());
        return FALSE;
    }
    setLoadingProject(FALSE);
    setCursor(KCursor::arrowCursor());

    if (!this->generateChart(TRUE))
        return FALSE;

    QTimer* delayTimer = new QTimer(this);
    connect(delayTimer, SIGNAL(timeout()),
            this, SLOT(regenerateChart()));
    delayTimer->start(200, TRUE);

    delete statusBarUpdateTimer;
    statusBarUpdateTimer = new QTimer(this);
    connect(statusBarUpdateTimer, SIGNAL(timeout()),
            this, SLOT(updateStatusBar()));
    statusBarUpdateTimer->start(500, FALSE);

    return TRUE;
}

void
TjReport::regenerateChart()
{
    this->generateChart(FALSE);
}

void
TjReport::generateTaskListLine(const QtReportElement* reportElement,
                               const Task* t, QListViewItem* lvi,
                               const Resource* r)
{
    // Skip the first colum. It contains the hardwired task name.
    int column = 1;
    for (QPtrListIterator<TableColumnInfo>
         ci = reportElement->getColumnsIterator(); *ci; ++ci, ++column)
    {
        QString cellText;
        QPixmap icon;

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
            double val = t->getCredits
                (scenario, Interval(reportElement->getStart(),
                                    reportElement->getEnd()), Cost, r);
            cellText = indent(tcf->realFormat.format(val, FALSE),
                              lvi, tcf->getHAlign() ==
                              TableColumnFormat::right);
        }
        else if ((*ci)->getName() == "criticalness")
        {
            cellText = indent(QString().sprintf("%f",
                                                t->getCriticalness(scenario)),
                              lvi, tcf->getHAlign() ==
                              TableColumnFormat::right);
        }
        else if ((*ci)->getName() == "duration")
            cellText = reportElement->scaledLoad
                (t->getCalcDuration(scenario), tcf->realFormat);
        else if ((*ci)->getName() == "effort")
        {
            double val = 0.0;
            val = t->getLoad(scenario, Interval(t->getStart(scenario),
                                                t->getEnd(scenario)), r);
            cellText = indent
                (reportElement->scaledLoad (val, tcf->realFormat),
                 lvi, tcf->getHAlign() == TableColumnFormat::right);
        }
        else if ((*ci)->getName() == "end")
            cellText = time2user((t->isMilestone() ? 1 : 0) +
                                 t->getEnd(scenario),
                                 reportDef->getTimeFormat());
        else if ((*ci)->getName() == "endbuffer")
        {
        }
        else if ((*ci)->getName() == "endbufferstart")
        {
        }
        else if ((*ci)->getName() == "id")
            cellText = t->getId();
        else if ((*ci)->getName() == "maxend")
            cellText = time2user(t->getMaxEnd(scenario),
                                 reportDef->getTimeFormat());
        else if ((*ci)->getName() == "maxstart")
            cellText = time2user(t->getMaxStart(scenario),
                                 reportDef->getTimeFormat());
        else if ((*ci)->getName() == "minend")
            cellText = time2user(t->getMinEnd(scenario),
                                 reportDef->getTimeFormat());
        else if ((*ci)->getName() == "minstart")
            cellText = time2user(t->getMinStart(scenario),
                                 reportDef->getTimeFormat());
        else if ((*ci)->getName() == "note" && !t->getNote().isEmpty())
            icon = KGlobal::iconLoader()->
                loadIcon("tj_note", KIcon::Small);
        else if ((*ci)->getName() == "pathcriticalness")
        {
        }
        else if ((*ci)->getName() == "priority")
            cellText = indent(QString().sprintf("%d", t->getPriority()),
                              lvi, tcf->getHAlign() ==
                              TableColumnFormat::right);
        else if ((*ci)->getName() == "projectid")
        {
        }
        else if ((*ci)->getName() == "projectids")
        {
        }
        else if ((*ci)->getName() == "profit")
        {
            double val = t->getCredits
                (scenario, Interval(reportElement->getStart(),
                                    reportElement->getEnd()), Revenue, r) -
                t->getCredits
                (scenario, Interval(reportElement->getStart(),
                                    reportElement->getEnd()), Cost, r);
            cellText = indent(tcf->realFormat.format(val, FALSE),
                              lvi, tcf->getHAlign() ==
                              TableColumnFormat::right);
        }
        else if ((*ci)->getName() == "responsible")
        {
        }
        else if ((*ci)->getName() == "revenue")
        {
            double val = t->getCredits
                (scenario, Interval(reportElement->getStart(),
                                    reportElement->getEnd()), Revenue, r);
            cellText = indent(tcf->realFormat.format(val, FALSE),
                              lvi, tcf->getHAlign() ==
                              TableColumnFormat::right);
        }
        else if ((*ci)->getName() == "start")
            cellText = time2user(t->getStart(scenario),
                                 reportDef->getTimeFormat());
        else if ((*ci)->getName() == "startbuffer")
        {
        }
        else if ((*ci)->getName() == "startbufferend")
        {
        }
        else if ((*ci)->getName() == "status")
        {
        }
        else if ((*ci)->getName() == "statusnote")
        {
        }

        lvi->setText(column, cellText);
        if (!icon.isNull())
            lvi->setPixmap(column, icon);
    }
}

void
TjReport::generateResourceListLine(const QtReportElement* reportElement,
                                   const Resource* r, QListViewItem* lvi,
                                   const Task* t)
{
    // Skip the first colum. It contains the hardwired resource name.
    int column = 1;
    for (QPtrListIterator<TableColumnInfo>
         ci = reportElement->getColumnsIterator(); *ci; ++ci, ++column)
    {
        QString cellText;
        const TableColumnFormat* tcf =
            reportElement->getColumnFormat((*ci)->getName());

        if ((*ci)->getName() == "cost")
        {
            double val = r->getCredits
                (scenario, Interval(reportElement->getStart(),
                                    reportElement->getEnd()), Cost, t);
            cellText = indent(tcf->realFormat.format(val, FALSE),
                              lvi, tcf->getHAlign() ==
                              TableColumnFormat::right);
        }
        else if ((*ci)->getName() == "id")
        {
            cellText = r->getFullId();
        }
        else if ((*ci)->getName() == "freeload")
        {
            if (!t)
            {
                double val = 0.0;
                val = r->getAvailableWorkLoad
                    (scenario, Interval(reportElement->getStart(),
                                        reportElement->getEnd()));
                cellText = indent
                    (reportElement->scaledLoad(val, tcf->realFormat), lvi,
                     tcf->getHAlign() == TableColumnFormat::right);
            }
        }
        else if ((*ci)->getName() == "effort")
        {
            double val = 0.0;
            if (t)
                val = r->getLoad(scenario, Interval(t->getStart(scenario),
                                                    t->getEnd(scenario)),
                                 AllAccounts, t);
            else
                val = r->getLoad(scenario, Interval(reportElement->getStart(),
                                                    reportElement->getEnd()));
            cellText = indent
                (reportElement->scaledLoad(val, tcf->realFormat), lvi,
                 tcf->getHAlign() == TableColumnFormat::right);
        }
        else if ((*ci)->getName() == "utilization")
        {
            if (!t)
            {
                double load = r->getLoad
                    (scenario, Interval(reportElement->getStart(),
                                        reportElement->getEnd()));
                double freeLoad = r->getAvailableWorkLoad
                    (scenario, Interval(reportElement->getStart(),
                                        reportElement->getEnd()));
                double val = 100.0 / (1.0 + (freeLoad / load));
                cellText = indent(QString().sprintf("%.1f%%", val), lvi,
                                  tcf->getHAlign() == TableColumnFormat::right);
            }
        }
        else if ((*ci)->getName() == "rate")
        {
            cellText = indent(tcf->realFormat.format(r->getRate(), FALSE),
                              lvi, tcf->getHAlign() ==
                              TableColumnFormat::right);
        }
        else if ((*ci)->getName() == "revenue")
        {
            double val = r->getCredits
                (scenario, Interval(reportElement->getStart(),
                                    reportElement->getEnd()), Revenue, t);
            cellText = indent(tcf->realFormat.format(val, FALSE),
                              lvi, tcf->getHAlign() ==
                              TableColumnFormat::right);
        }

        lvi->setText(column, cellText);
    }
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

    /* QCanvas can only handle 32767 pixel width. So we have to shorten the
     * report period if the chart exceeds this size. */
    if (time2x(endTime) > 32767)
        endTime = x2time(32767 - 2);

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
TjReport::generateHourHeader(int y)
{
    for (time_t hour = midnight(startTime); hour < endTime;
         hour = hoursLater(1, hour))
    {
        int x = time2x(hour);
        if (x < 0)
            continue;
        QCanvasLine* line = new QCanvasLine(ganttHeader);
        line->setPoints(x, y, x, y + headerHeight / 2);
        QPen pen = line->pen();
        pen.setColor(colorGroup().mid());
        line->setPen(pen);
        line->setZ(TJRL_GRIDLINES);
        line->show();

        // Write hour of day.
        QString label;
        label = QString("%1").arg(hourOfDay(hour));
        QCanvasText* text = new QCanvasText(label, ganttHeader);
        text->setX(x + 2);
        text->setY(y);
        text->setZ(TJRL_GRIDLABLES);
        text->show();
    }
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
        QCanvasText* text = new QCanvasText(label, ganttHeader);
        text->setX(x + 2);
        text->setY(y);
        text->setZ(TJRL_GRIDLABLES);
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
        line->setZ(TJRL_GRIDLINES);
        line->show();

        // Write week number.
        QCanvasText* text = new QCanvasText
            (i18n("short for week of year", "W%1")
             .arg(weekOfYear(week, weekStartsMonday)),
             ganttHeader);
        text->move(x + 2, y);
        text->setZ(TJRL_GRIDLABLES);
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
        line->setZ(TJRL_GRIDLINES);
        line->show();

        // Write month name (and year).
        QString s = withYear ?
            QString("%1 %2").arg(QDate::shortMonthName(monthOfYear(month)))
            .arg(::year(month)) :
            QString("%1").arg(QDate::shortMonthName(monthOfYear(month)));
        QCanvasText* text = new QCanvasText(s, ganttHeader);
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
            QCanvasText* text = new QCanvasText(s, ganttHeader);
            text->move(x + 2, y);
            text->setZ(TJRL_GRIDLABLES);
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
        line->setZ(TJRL_GRIDLINES);
        line->show();

        // Write quarter number.
        QCanvasText* text =
            new QCanvasText(i18n("short for quater of year", "Q%1")
                            .arg(quarterOfYear(quarter)),
                            ganttHeader);
        text->move(x + 2, y);
        text->setZ(TJRL_GRIDLABLES);
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
        line->setZ(TJRL_GRIDLINES);
        line->show();

        // Write year.
        QCanvasText* text =
            new QCanvasText(QString("%1").arg(::year(year)), ganttHeader);
        text->move(x + 2, y);
        text->setZ(TJRL_GRIDLABLES);
        text->show();
    }
}

void
TjReport::markNonWorkingHoursOnBackground()
{
    QColor col = KGlobalSettings::calculateAlternateBackgroundColor
        (listView->colorGroup().base()).dark(110);
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
            new QCanvasRectangle(x, 0, w, listHeight, ganttChart);
        rect->setPen(QPen(col));
        rect->setBrush(QBrush(col));
        rect->setZ(TJRL_OFFTIME);
        rect->show();
    }
}

void
TjReport::markNonWorkingDaysOnBackground()
{
    QColor col = KGlobalSettings::calculateAlternateBackgroundColor
        (listView->colorGroup().base()).dark(110);
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
                new QCanvasRectangle(x, 0, w, listHeight, ganttChart);
            rect->setPen(QPen(col));
            rect->setBrush(QBrush(col));
            rect->setZ(TJRL_OFFTIME);
            rect->show();
        }
    }
}

void
TjReport::markBoundary(int x, bool now, int layer)
{
    QColor col;
    if (now)
        col = QColor(Qt::red);
    else
        col = KGlobalSettings::calculateAlternateBackgroundColor
            (listView->colorGroup().base()).dark(130);

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
TjReport::markHourBoundaries(int distance)
{
    for (time_t hour = midnight(startTime);
         hour < endTime; hour = hoursLater(distance, hour))
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
    QCanvasRectangle* rect =
        new QCanvasRectangle(0, 0, ganttChart->width(), ganttChart->height(),
                             ganttChart);
    rect->setPen(QPen(NoPen));
    QColor bgColor = listView->colorGroup().base();
    QColor altBgColor = KGlobalSettings::calculateAlternateBackgroundColor
        (bgColor);
    rect->setBrush(QBrush(bgColor));
    rect->setZ(TJRL_BACKGROUND);
    rect->show();

    bool toggle = FALSE;
    for (int y = 0; y < listHeight; y += itemHeight)
    {
        toggle = !toggle;
        if (toggle)
            continue;

        rect = new QCanvasRectangle(0, y, canvasWidth, itemHeight, ganttChart);
        rect->setPen(QPen(NoPen));
        rect->setBrush(QBrush(altBgColor));
        rect->setZ(TJRL_BACKLINES);
        rect->show();
    }
}

int
TjReport::time2x(time_t t) const
{
    return (int) ((t - startTime) *
                  (((float) pixelPerYear) / (60 * 60 * 24 * 365)));
}

time_t
TjReport::x2time(int x) const
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

    syncVSlidersGantt2List(ganttChartView->contentsX(), listView->contentsY());
}

void
TjReport::expandReportItem(QListViewItem*)
{
    if (loadingProject)
        return;

    this->generateChart(FALSE);
    syncVSlidersGantt2List(ganttChartView->contentsX(), listView->contentsY());
}

void
TjReport::listClicked(QListViewItem* lvi, const QPoint&, int column)
{
    // The first column is always the name and it's not in the TCI table.
    if (!lvi || column == 0)
        return;

    CoreAttributes* ca = lvi2caDict[QString().sprintf("%p", lvi)];
    if (ca->getType() == CA_Task)
    {
        Task* t = dynamic_cast<Task*>(ca);
        const TableColumnInfo* tci =
            this->getReportElement()->columnsAt(column - 1);
        if (tci->getName() == "note" && !t->getNote().isEmpty())
        {
            // Open a new window that displays the note attached to the task.
            RichTextDisplay* richTextDisplay =
                new RichTextDisplay(topLevelWidget());
            richTextDisplay->setCaption
                (QString("Note for Task %1 (%2) - TaskJuggler")
                 .arg(t->getName()).arg(t->getId()));
            richTextDisplay->textDisplay->setTextFormat(Qt::RichText);

            richTextDisplay->textDisplay->setText(t->getNote());
            richTextDisplay->show();
        }
    }
}

void
TjReport::doPopupMenu(QListViewItem* lvi, const QPoint& pos, int)
{
    if (!lvi)
        return;

    CoreAttributes* ca = lvi2caDict[QString().sprintf("%p", lvi)];
    QPopupMenu menu;
    if (ca->getType() == CA_Task)
    {
        Task* t = dynamic_cast<Task*>(ca);

        menu.insertItem(i18n("&Edit Task"), 1);
        menu.insertItem(i18n("Show Task &Details"), 2);
        //menu.insertItem(i18n("&Zoom to fit Task"), 3);
        switch (menu.exec(pos))
        {
            case 1:
                emit signalEditCoreAttributes(ca);
                break;
            case 2:
                showTaskDetails(t);
                break;
            case 3:
                break;
            default:
                break;
        }
    }
    else
    {
        Resource* r = dynamic_cast<Resource*>(ca);

        menu.insertItem(i18n("&Edit Resource"), 1);
        menu.insertItem(i18n("Show Resource &Details"), 2);
        switch (menu.exec(pos))
        {
            case 1:
                emit signalEditCoreAttributes(ca);
                break;
            case 2:
                showResourceDetails(r);
                break;
            default:
                break;
        }
    }
}

void
TjReport::showTaskDetails(const Task* task)
{
    RichTextDisplay* richTextDisplay = new RichTextDisplay(topLevelWidget());
    richTextDisplay->setCaption
        (i18n("Details of Task %1 (%2) - TaskJuggler")
         .arg(task->getName()).arg(task->getId()));
    richTextDisplay->textDisplay->setTextFormat(Qt::RichText);

    QString text;
    if (task->isMilestone())
    {
        text += i18n("<b>Date:</b> %1<br/>")
            .arg(time2tjp(task->getStart(scenario)));
    }
    else
        text += i18n("<b>Start:</b> %1<br/>"
                     "<b>End:</b> %2<br/>")
            .arg(time2tjp(task->getStart(scenario)))
            .arg(time2tjp(task->getEnd(scenario) + 1));

    if (!task->getNote().isEmpty())
    {
        if (!text.isEmpty())
            text += "<hr/>";
        text += i18n("<b>Note:</b> %1<br/>").arg(task->getNote());
    }

    QString predecessors;
    for (TaskListIterator tli(task->getPreviousIterator()); *tli; ++tli)
    {
        if (!predecessors.isEmpty())
            predecessors += ", ";
        predecessors += (*tli)->getName() + "(" + (*tli)->getId() + ")";
    }

    QString successors;
    for (TaskListIterator tli(task->getFollowersIterator()); *tli; ++tli)
    {
        if (!successors.isEmpty())
            successors += ", ";
        successors += (*tli)->getName() + " (" + (*tli)->getId() + ")";
    }

    if (!predecessors.isEmpty() || !successors.isEmpty())
    {
        if (!text.isEmpty())
            text += "<hr/>";
        if (!predecessors.isEmpty())
            text += i18n("<b>Predecessors:</b> %1<br/>").arg(predecessors);
        if (!successors.isEmpty())
            text += i18n("<b>Successors:</b> %1<br/>").arg(successors);
    }

    if (task->hasJournal())
    {
        if (!text.isEmpty())
            text += "<hr/>";
        text += generateJournal(task->getJournalIterator());
    }

    richTextDisplay->textDisplay->setText(text);
    richTextDisplay->show();
}

void
TjReport::showResourceDetails(const Resource* resource)
{
    RichTextDisplay* richTextDisplay = new RichTextDisplay(topLevelWidget());
    richTextDisplay->setCaption
        (QString("Details of Resource %1 (%2) - TaskJuggler")
         .arg(resource->getName()).arg(resource->getFullId()));
    richTextDisplay->textDisplay->setTextFormat(Qt::RichText);

    QString text;

    if (resource->hasJournal())
    {
        if (!text.isEmpty())
            text += "<hr/>";
        text += generateJournal(resource->getJournalIterator());
    }

    richTextDisplay->textDisplay->setText(text);
    richTextDisplay->show();
}

void
TjReport::syncVSlidersGantt2List(int x, int y)
{
    ganttHeaderView->setContentsPos(x, ganttHeaderView->contentsY());
    if (y != listView->contentsY())
    {
        // To prevent endless loops we need to disconnect the contentsMoving
        // signal temoraryly.
        disconnect(listView, SIGNAL(contentsMoving(int, int)),
                   this, SLOT(syncVSlidersList2Gantt(int, int)));
        listView->setContentsPos(listView->contentsX(), y);
        connect(listView, SIGNAL(contentsMoving(int, int)),
                this, SLOT(syncVSlidersList2Gantt(int, int)));
    }
}

void
TjReport::syncVSlidersList2Gantt(int, int y)
{
    if (y != ganttChartView->contentsY())
    {
        // To prevent endless loops we need to disconnect the contentsMoving
        // signal temoraryly.
        disconnect(ganttChartView, SIGNAL(contentsMoving(int, int)),
                   this, SLOT(syncVSlidersGantt2List(int, int)));
        ganttChartView->setContentsPos(ganttChartView->contentsX(), y);
        connect(ganttChartView, SIGNAL(contentsMoving(int, int)),
                this, SLOT(syncVSlidersGantt2List(int, int)));
    }
}

void
TjReport::updateStatusBar()
{
    QString text;
    QPoint pos = ganttChartView->mapFromGlobal(QCursor::pos());
    if (pos.x() < 0 || pos.y() < 0 ||
        pos.x() > ganttChartView->width() ||
        pos.y() > ganttChartView->height())
        return;

    QListViewItem* lvi = listView->itemAt(QPoint(50, pos.y()));
    if (!lvi)
        return;

    CoreAttributes* ca = lvi2caDict[QString().sprintf("%p", lvi)];
    CoreAttributes* parent = lvi2ParentCaDict[QString().sprintf("%p", lvi)];

    emit signalChangeStatusBar(this->generateStatusBarText(pos, ca, parent));
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
TjReport::show()
{
    QWidget::show();

    if (statusBarUpdateTimer)
        statusBarUpdateTimer->start(500, FALSE);
}

void
TjReport::hide()
{
    if (statusBarUpdateTimer)
        statusBarUpdateTimer->stop();

    QWidget::hide();
}

void
TjReport::setBestStepUnit()
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
        if (level > 30)
            kdFatal() << "Tree level explosion";
    }
    return level;
}

Interval
TjReport::stepInterval(time_t ref) const
{
    Interval iv;
    switch (stepUnit)
    {
        case hour:
            iv.setStart(beginOfHour(ref));
            iv.setEnd(hoursLater(1, iv.getStart()) - 1);
            break;
        case day:
            iv.setStart(midnight(ref));
            iv.setEnd(sameTimeNextDay(iv.getStart()) - 1);
            break;
        case week:
            iv.setStart(beginOfWeek
                        (ref, reportDef->getProject()->getWeekStartsMonday()));
            iv.setEnd(sameTimeNextWeek(iv.getStart()) - 1);
            break;
        case month:
            iv.setStart(beginOfMonth(ref));
            iv.setEnd(sameTimeNextMonth(iv.getStart()) - 1);
            break;
        case quarter:
            iv.setStart(beginOfQuarter(ref));
            iv.setEnd(sameTimeNextQuarter(iv.getStart()) - 1);
            break;
        case year:
            iv.setStart(beginOfYear(ref));
            iv.setEnd(sameTimeNextYear(iv.getStart()) - 1);
            break;
        default:
            kdFatal() << "Unknown stepUnit";
    }
    return iv;
}

QString
TjReport::stepIntervalName(time_t ref) const
{
    QString name;
    switch (stepUnit)
    {
        case hour:
            name = time2user(beginOfHour(ref), "%k:%M " +
                             KGlobal::locale()->dateFormat());
            break;
        case day:
            name = time2user(midnight(ref), KGlobal::locale()->dateFormat());
            break;
        case week:
        {
            bool wsm = reportDef->getProject()->getWeekStartsMonday();
            name = i18n("Week %1, %2").arg(weekOfYear(ref, wsm))
                .arg(::year(ref));
            break;
        }
        case month:
            name = QString("%1 %2").arg(QDate::shortMonthName(monthOfYear(ref)))
                .arg(::year(ref));
            break;
        case quarter:
            name = QString("Q%1 %2").arg(quarterOfYear(ref)).arg(::year(ref));
            break;
        case year:
            name = QString().sprintf("%d", ::year(ref));
            break;
        default:
            kdFatal() << "Unknown stepUnit";
    }
    return name;
}

QString
TjReport::generateJournal(JournalIterator jit) const
{
    QString text;

    for ( ; *jit; ++jit)
        text += "<b><i>" + time2user((*jit)->getDate(),
                                  reportDef->getTimeFormat()) +
            "</i></b><br/>" + (*jit)->getText() + "<br/>";

    return text;
}

#include "TjReport.moc"
