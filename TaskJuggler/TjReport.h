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

#ifndef _TjReport_h_
#define _TjReport_h_

#include <time.h>

#include <qwidget.h>
#include <qstring.h>

#include "TaskList.h"

class QListViewItem;
class QSplitter;
class KListView;
class Report;
class QCanvas;
class QCanvasView;
class Task;

class TjReport : public QWidget
{
    Q_OBJECT
public:
    TjReport(QWidget* p, Report* const rDef, const QString& n = QString::null);
    virtual ~TjReport() { }

    void generateTaskReport();

public slots:
    void zoomIn();
    void zoomOut();

private slots:
    void collapsReportItem(QListViewItem* lvi);
    void expandReportItem(QListViewItem* lvi);
    void syncVSliders(int, int);

private:
    enum StepUnits { day = 0, week, month, quarter, year };
    TjReport() : reportDef(0) { }

    int time2x(time_t t);

    void generateGanttChart(bool autoFit);
    void generateHeaderAndGrid();
    void generateDayHeader(int y);
    void generateWeekHeader(int y);
    void generateMonthHeader(int y, bool withYear);
    void generateQuarterHeader(int y);
    void generateYearHeader(int y);
    void generateGanttBackground();
    void markNonWorkingDaysOnBackground();
    void markMonthsBoundaries();
    void generateGanttTasks();
    void generateTask(Task* t, int y);
    void generateLeftHeader();
    void generateRightHeader();

    void setBestStepUnit();

    Report* const reportDef;
    QSplitter* splitter;
    KListView* listView;
    QWidget* canvasFrame;
    QCanvas* ganttHeader;
    QCanvasView* ganttHeaderView;
    QCanvas* ganttChart;
    QCanvasView* ganttChartView;

    int pixelPerYear;
    StepUnits stepUnit;
    static const int minStepDay = 20;
    static const int minStepWeek = 35;
    static const int minStepMonth = 35;
    static const int minStepQuarter = 50;
    static const int minStepYear = 80;
    static const int zoomSteps[];
    uint currentZoomStep;

    int scenario;
    int headerHeight;
    int listHeight;
    int itemHeight;
    int canvasWidth;
    time_t startTime;
    time_t endTime;

    TaskList taskList;
} ;

#endif

