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
#include <qdict.h>
#include <qlistview.h>

#include "TaskList.h"

class QSplitter;
class KListView;
class Report;
class QCanvas;
class QCanvasView;
class Task;
class QtTaskReportElement;

class TjReport : public QWidget
{
    Q_OBJECT
public:
    TjReport(QWidget* p, Report* const rDef, const QString& n = QString::null);
    virtual ~TjReport() { }

    bool generateTaskReport();

    void setLoadingProject(bool lp) { loadingProject = lp; }

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
    time_t x2time(int x);

    void generateGanttChart(bool autoFit);
    void generateHeaderAndGrid();
    void generateDayHeader(int y);
    void generateWeekHeader(int y);
    void generateMonthHeader(int y, bool withYear);
    void generateQuarterHeader(int y);
    void generateYearHeader(int y);
    void generateGanttBackground();
    void markNonWorkingHoursOnBackground();
    void markNonWorkingDaysOnBackground();
    void markBoundary(int x);
    void markHourBoundaries();
    void markDayBoundaries();
    void markWeekBoundaries();
    void markMonthsBoundaries();
    void markQuarterBoundaries();
    void generateGanttTasks();
    void generateTask(Task* const t, int y);
    void generateDependencies(Task* const t, QListViewItem* lvi);
    void generateLeftHeader();
    void generateRightHeader();

    QListViewItem* getTaskListEntry(Task* const t);
    void setBestStepUnit();

    int lvi2yPos(QListViewItem* lvi) const;

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
    static const int minStepHour;
    static const int minStepDay;
    static const int minStepWeek;
    static const int minStepMonth;
    static const int minStepQuarter;
    static const int minStepYear;
    static const int zoomSteps[];
    uint currentZoomStep;
    /**
     * This hash table is used to speed up calculation of the Y coordinate of
     * an item in the list view. We use the Address of the LVI converted to a
     * QString as the lookup key. */
    QDict<int> lvi2yPosDict;
    // And the same for the CoreAttribute pointer.
    QDict<QListViewItem> ca2lviDict;

    QtTaskReportElement* reportElement;
    int scenario;
    int headerHeight;
    int listHeight;
    int itemHeight;
    int canvasWidth;
    time_t startTime;
    time_t endTime;

    bool loadingProject;

    TaskList taskList;
} ;

#endif

