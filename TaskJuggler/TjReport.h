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
#include "ResourceList.h"

class QSplitter;
class KListView;
class Report;
class QCanvas;
class QCanvasView;
class Task;
class Resource;
class QtReportElement;

class TjReport : public QWidget
{
    Q_OBJECT
public:
    TjReport(QWidget* p, Report* const rDef, const QString& n = QString::null);
    virtual ~TjReport() { }

    virtual bool generateReport();

    void setLoadingProject(bool lp) { loadingProject = lp; }

    void generateTaskListLine(const QtReportElement* reportElement,
                              const Task* t, QListViewItem* lvi,
                              const Resource* r = 0);
    void generateResourceListLine(const QtReportElement* reportElement,
                                  const Resource* r, QListViewItem* lvi,
                                  const Task* t = 0);
public slots:
    void zoomIn();
    void zoomOut();

private slots:
    void regenerateChart();
    void collapsReportItem(QListViewItem* lvi);
    void expandReportItem(QListViewItem* lvi);
    void syncVSliders(int, int);

protected:
    enum StepUnits { day = 0, week, month, quarter, year };
    TjReport() : reportDef(0) { }

    int time2x(time_t t);
    time_t x2time(int x);

    virtual bool generateList() = 0;
    virtual bool generateChart(bool autoFit) = 0;

    void prepareChart(bool autoFit, QtReportElement* repElement);
    void generateHeaderAndGrid();
    void generateDayHeader(int y);
    void generateWeekHeader(int y);
    void generateMonthHeader(int y, bool withYear);
    void generateQuarterHeader(int y);
    void generateYearHeader(int y);
    void generateGanttBackground();
    void markNonWorkingHoursOnBackground();
    void markNonWorkingDaysOnBackground();
    void markBoundary(int x, const QColor& col, int layer = 2);
    void markHourBoundaries();
    void markDayBoundaries();
    void markWeekBoundaries();
    void markMonthsBoundaries();
    void markQuarterBoundaries();
    void generateGanttTasks();
    void generateLoadBars();
    void generateTask(Task* const t, int y);
    void generateResource(Resource* const r, int y);
    void generateDependencies(Task* const t, QListViewItem* lvi);
    void generateListHeader(const QString& firstHeader, QtReportElement* tab);
    void generateTaskResources(Task* const t);
    QString indent(const QString& input, const QListViewItem* lvi,
                   bool right);
    int treeLevel(const QListViewItem* lvi) const;

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
    static const int minStepHour;
    static const int minStepDay;
    static const int minStepWeek;
    static const int minStepMonth;
    static const int minStepQuarter;
    static const int minStepYear;
    static const int zoomSteps[];
    uint currentZoomStep;
    /**
     * We often need to find out if a CoreAttribute is in the ListView and
     * find the appropriate list item. So we keep a dictionary that maps the
     * CoreAttribute ID to the QListViewItem*. As the namespaces of the
     * different CoreAttributes may contain duplicates we use a single
     * character plus colon prefix to create a unified namespace. So
     * t:mytask.subtask is a task and r:team.nick is a resource.
     */
    QDict<QListViewItem> ca2lviDict;
    /**
     * This is the maximum indentation of the list view. It only takes visible
     * items into account. Visible means not hidden by closed parents.
     */
    int maxDepth;

    int scenario;
    int headerHeight;
    int listHeight;
    int itemHeight;
    int canvasWidth;
    time_t startTime;
    time_t endTime;

    bool loadingProject;

    TaskList taskList;
    ResourceList resourceList;
} ;

#endif

