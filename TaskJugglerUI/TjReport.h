/*
 * The TaskJuggler Project Management Software
 *
 * Copyright (c) 2001, 2002, 2003, 2004, 2005, 2006, 2007
 * by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _TjReport_h_
#define _TjReport_h_

#include <set>
#include <map>
#include <vector>

#include <qlistview.h>

#include "TjUIReportBase.h"
#include "TaskList.h"
#include "ResourceList.h"
#include "Journal.h"
#include "ReportLayers.h"
#include "TjGanttChart.h"
#include "ltQString.h"
#include "ltstr.h"

class QSplitter;
class QCanvas;
class QCanvasView;
class QTimer;
class KListViewItem;
class KListView;
class KPrinter;
class Report;
class Task;
class Resource;
class QtReportElement;
class Interval;
class JournalIterator;
class TjPrintReport;
class TjObjPosTable;
class TableColumnInfo;
class ReportController;

class TjReport : public TjUIReportBase
{
    Q_OBJECT
public:
    TjReport(QWidget* p, ReportManager* m, Report* rDef,
             const QString& n = QString::null);
    virtual ~TjReport();

    virtual bool generateReport();

    virtual QtReportElement* getReportElement() const = 0;

    virtual void setFocus();

    void generateTaskListLine(const QtReportElement* reportElement,
                              const Task* t, KListViewItem* lvi,
                              const Resource* r = 0);
    void generateResourceListLine(const QtReportElement* reportElement,
                                  Resource* r, KListViewItem* lvi,
                                  const Task* t = 0);
    void print();

    virtual TjPrintReport* newPrintReport(KPrinter* pr) = 0;

public slots:
    void zoomIn();
    void zoomOut();
    void zoomTo(const QString& lable);

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
    void reportSearchTriggered(const QString&);
    void setReportStart(const QDate& d);
    void setReportEnd(const QDate& d);

protected:
    virtual bool event(QEvent* ev);
    virtual bool generateList() = 0;

    void prepareChart();

    void triggerChartRegeneration(int msDelay);

    void generateListHeader(const QString& firstHeader, QtReportElement* tab);

    void showTaskDetails(const Task* task);
    void showResourceDetails(Resource* resource);

    void generateCustomAttribute(const CoreAttributes* ca, const QString name,
                                 QString& cellText, QPixmap& icon) const;
    void handleMouseEvent(const QMouseEvent* ev);

    QListViewItem* getChartItemBelowCursor(QPoint& pos);

    virtual QString generateStatusBarText(const QPoint& pos,
                                          CoreAttributes* ca,
                                          CoreAttributes* parent) = 0;

    QString indent(const QString& input, const QListViewItem* lvi,
                   bool right);
    int treeLevel(const QListViewItem* lvi) const;

    QString generateJournal(Journal::Iterator jit) const;

    QString generateRTCustomAttributes(const CoreAttributes* ca) const;

    void setGanttChartColors();

    time_t stepLength() const;
    void setBestStepUnit();

    void updateZoomSelector();

    QSplitter* splitter;
    KListView* listView;
    QWidget* canvasFrame;
    QWidget* reportFrame;
    ReportController* reportController;

    TjObjPosTable* objPosTable;
    TjGanttChart* ganttChart;

    QCanvasView* ganttHeaderView;
    QCanvasView* ganttChartView;

    /**
     * We some widgets that need to be fit into the window the first time the
     * widget is rendered on the screen. We use the following variable to keep
     * track of this.
     */
    TjGanttChart::ScaleMode scaleMode;

    /**
     * We often need to find out if a CoreAttribute is in the ListView and
     * find the appropriate list item. So we keep a dictionary that maps the
     * CoreAttribute ID to the QListViewItem*. As the namespaces of the
     * different CoreAttributes may contain duplicates we use a single
     * character plus colon prefix to create a unified namespace. So
     * t:mytask.subtask is a task and r:team.nick is a resource.
     */
    std::map<const QString, KListViewItem*, ltQString> ca2lviDict;

    /* And the same in the other direction. We use the hex-ed address of the
     * LVI as key. */
    std::map<const QString, CoreAttributes*, ltQString> lvi2caDict;

    /* For nested lists we need to be able to map the lvi to the parent
     * CoreAttributes. */
    std::map<const QString, CoreAttributes*, ltQString> lvi2ParentCaDict;

    /* The interactive reports don't have a 1:1 mapping between the defintion
     * and the column in the widget. Some special columns are not included in
     * the widget. To link a widget column back to the TCI we need this list.
     */
    std::vector<TableColumnInfo*> lvCol2tci;

    /**
     * This is the maximum indentation of the list view. It only takes visible
     * items into account. Visible means not hidden by closed parents.
     */
    int maxDepth;

    int scenario;
    bool showGantt;
    int headerHeight;
    int listHeight;
    int itemHeight;
    int canvasWidth;

    QTimer* statusBarUpdateTimer;
    QTimer* delayTimer;

    TaskList taskList;
    ResourceList resourceList;

    /* The interactive reports treat the indexes, name and gantt columns
     * differently than most other reports. They provide special rendering for
     * them and need to be ignored during generic column rendering. */
    std::set<const char*, ltstr> indexColumns;
    std::set<const char*, ltstr> specialColumns;
} ;

#endif

