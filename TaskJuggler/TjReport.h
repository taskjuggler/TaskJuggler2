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
#include "ReportLayers.h"

class QSplitter;
class QCanvas;
class QCanvasView;
class QTimer;
class KListView;
class Report;
class Task;
class Resource;
class QtReportElement;
class Interval;
class JournalIterator;

class TjReport : public QWidget
{
    Q_OBJECT
public:
    TjReport(QWidget* p, Report* const rDef, const QString& n = QString::null);
    virtual ~TjReport();

    virtual bool generateReport();

    virtual const QtReportElement* getReportElement() const = 0;

    void setLoadingProject(bool lp) { loadingProject = lp; }

    void generateTaskListLine(const QtReportElement* reportElement,
                              const Task* t, QListViewItem* lvi,
                              const Resource* r = 0);
    void generateResourceListLine(const QtReportElement* reportElement,
                                  const Resource* r, QListViewItem* lvi,
                                  const Task* t = 0);

signals:
    void signalChangeStatusBar(const QString& text);
    void signalEditCoreAttributes(CoreAttributes*);

public slots:
    void zoomIn();
    void zoomOut();
    virtual void show();
    virtual void hide();

private slots:
    void regenerateChart();
    void collapsReportItem(QListViewItem* lvi);
    void expandReportItem(QListViewItem* lvi);
    void listClicked(QListViewItem* lvi, const QPoint&, int column);
    void listHeaderClicked(int colmun);
    void doPopupMenu(QListViewItem* lvi, const QPoint& pos, int);
    void syncVSlidersGantt2List(int, int);
    void syncVSlidersList2Gantt(int, int);
    void updateStatusBar();

protected:
    enum StepUnits { hour = 0, day, week, month, quarter, year };
    TjReport() : reportDef(0) { }

    int time2x(time_t t) const;
    time_t x2time(int x) const;

    virtual bool event(QEvent* ev);
    virtual bool generateList() = 0;
    virtual bool generateChart(bool autoFit) = 0;

    void prepareChart(bool autoFit, QtReportElement* repElement);
    void generateHeaderAndGrid();
    void generateHourHeader(int y);
    void generateDayHeader(int y);
    void generateWeekHeader(int y);
    void generateMonthHeader(int y, bool withYear);
    void generateQuarterHeader(int y);
    void generateYearHeader(int y);
    void generateGanttBackground();
    void markNonWorkingHoursOnBackground();
    void markNonWorkingDaysOnBackground();
    void markBoundary(int x, bool now = FALSE, int layer = TJRL_GRIDLINES);
    void markHourBoundaries(int distance);
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

    void showTaskDetails(const Task* task);
    void showResourceDetails(const Resource* resource);

    virtual QString generateStatusBarText(const QPoint& pos,
                                          const CoreAttributes* ca,
                                          const CoreAttributes* parent)
        const = 0;

    QString indent(const QString& input, const QListViewItem* lvi,
                   bool right);
    int treeLevel(const QListViewItem* lvi) const;

    Interval stepInterval(time_t ref) const;
    QString stepIntervalName(time_t ref) const;

    QString generateJournal(JournalIterator jit) const;

    time_t stepLength() const;
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

    /* And the same in the other direction. We use the hex-ed address of the
     * LVI as key. */
    QDict<CoreAttributes> lvi2caDict;

    /* For nested lists we need to be able to map the lvi to the parent
     * CoreAttributes. */
    QDict<CoreAttributes> lvi2ParentCaDict;

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

    QTimer* statusBarUpdateTimer;

    TaskList taskList;
    ResourceList resourceList;
} ;

#endif

