/*
 * The TaskJuggler Project Management Software
 *
 * Copyright (c) 2001, 2002, 2003, 2004, 2005 by Chris Schlaeger <cs@kde.org>
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
#include <qdict.h>
#include <qpaintdevicemetrics.h>

#include <klistview.h>
#include <klocale.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kapp.h>
#include <kcursor.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <ktextbrowser.h>
#include <krun.h>
#include <kprinter.h>
#include <kmessagebox.h>

#include "Project.h"
#include "Task.h"
#include "Resource.h"
#include "Journal.h"
#include "Utility.h"
#include "ExpressionTree.h"
#include "Report.h"
#include "TableColumnFormat.h"
#include "TextAttribute.h"
#include "ReferenceAttribute.h"
#include "QtTaskReport.h"
#include "QtTaskReportElement.h"
#include "QtResourceReport.h"
#include "QtResourceReportElement.h"
#include "ReportLayers.h"
#include "RichTextDisplay.h"
#include "TjPrintReport.h"
#include "TjGanttChart.h"
#include "TjObjPosTable.h"
#include "KPrinterWrapper.h"
#include "UsageLimits.h"
#include "ReportManager.h"

TjReport::TjReport(QWidget* p, ReportManager* m, Report* const rDef,
                   const QString& n)
    : TjUIReportBase(p, m, rDef, n)
{
    loadingProject = FALSE;
    autoFit = true;

    QHBoxLayout* hl = new QHBoxLayout(this, 0, 0);
    splitter = new QSplitter(Horizontal, this);

    listView = new KListView(splitter);
    listView->setRootIsDecorated(TRUE);
    listView->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    listView->setAllColumnsShowFocus(TRUE);
    // The sorting does not work yet properly.
    listView->header()->setClickEnabled(FALSE);
    listView->setItemMargin(2);

    canvasFrame = new QWidget(splitter);
    QVBoxLayout* vl = new QVBoxLayout(canvasFrame, 0, 0);
    canvasFrame->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);

    ganttChart = new TjGanttChart(this);
    objPosTable = 0;

    ganttHeaderView = new QCanvasView(ganttChart->getHeaderCanvas(),
                                      canvasFrame);
    ganttHeaderView->setHScrollBarMode(QScrollView::AlwaysOff);
    ganttHeaderView->setVScrollBarMode(QScrollView::AlwaysOff);

    ganttChartView = new QCanvasView(ganttChart->getChartCanvas(),
                                     canvasFrame);
    ganttChartView->setVScrollBarMode(QScrollView::AlwaysOff);

    startTime = endTime = 0;

    vl->addWidget(ganttHeaderView);
    vl->addWidget(ganttChartView);
    hl->addWidget(splitter);

    statusBarUpdateTimer = delayTimer = 0;

    connect(listView, SIGNAL(expanded(QListViewItem*)),
            this, SLOT(expandReportItem(QListViewItem*)));
    connect(listView, SIGNAL(collapsed(QListViewItem*)),
            this, SLOT(collapsReportItem(QListViewItem*)));
    connect(listView, SIGNAL(clicked(QListViewItem*, const QPoint&, int)),
            this, SLOT(listClicked(QListViewItem*, const QPoint&, int)));
    connect(listView,
            SIGNAL(rightButtonPressed(QListViewItem*, const QPoint&, int)),
            this, SLOT(doPopupMenu(QListViewItem*, const QPoint&, int)));
    connect(listView->header(), SIGNAL(clicked(int)),
            this, SLOT(listHeaderClicked(int)));
    connect(ganttChartView, SIGNAL(contentsMoving(int, int)),
            this, SLOT(syncVSlidersGantt2List(int, int)));
    connect(listView, SIGNAL(contentsMoving(int, int)),
            this, SLOT(syncVSlidersList2Gantt(int, int)));

    indexColumns.insert("index");
    indexColumns.insert("hierarchindex");
    indexColumns.insert("hierarchno");
    indexColumns.insert("no");
    indexColumns.insert("seqno");
    indexColumns.insert("name");

    specialColumns.insert("daily");
    specialColumns.insert("weekly");
    specialColumns.insert("monthly");
    specialColumns.insert("quarterly");
    specialColumns.insert("yearly");
}

TjReport::~TjReport()
{
    delete ganttChart;
    delete objPosTable;
    delete statusBarUpdateTimer;
}

void
TjReport::print()
{
    KPrinter* printer = new KPrinter;
    TjPrintReport* tjpr = 0;

    printer->setFullPage(true);
    printer->setResolution(300);
    printer->setCreator(QString("TaskJuggler %1 - visit %2")
                        .arg(VERSION).arg(TJURL));
    if (!printer->setup(this, i18n("Print %1").arg(report->getFileName())))
        goto done;

    /* This is a hack to workaround the problem that the KPrinter settings
     * not transferred to the QPrinter object when not printing to a file. */
    ((KPrinterWrapper*) printer)->preparePrinting();

    if ((tjpr = this->newPrintReport(printer)) == 0)
        goto done;
    tjpr->initialize();
    tjpr->generate();

    int xPages, yPages;
    tjpr->getNumberOfPages(xPages, yPages);
    if (!tjpr->beginPrinting())
        goto done;

    // This block avoids a compile error due to gotos crossing 'first'.
    {
        bool first = TRUE;
        for (int y = 0; y < yPages; ++y)
            for (int x = 0; x < xPages; ++x)
            {
                if (first)
                    first = FALSE;
                else
                    printer->newPage();
                tjpr->printReportPage(x, y);
            }
        tjpr->endPrinting();
    }

done:
    delete tjpr;
    delete printer;
}

bool
TjReport::event(QEvent* ev)
{
    // Regenerate the chart in case of a palette change.
    if (ev->type() == QEvent::ApplicationPaletteChange)
    {
        setGanttChartColors();
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

    /* The first time we generate the report, the window has not been fully
     * layouted yet. So we can't set the splitter to a good size and generate
     * the gantt report immediately. We use a 200ms timer to delay the
     * rendering. Hopefully by then the window has been layouted properly. */
    delayTimer = new QTimer(this);
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
    delete delayTimer;
    delayTimer = 0;

    setCursor(KCursor::waitCursor());

    prepareChart();

    // When we are here, we have rendered the widgets at least once. So we can
    // turn off autoFit mode.
    autoFit = false;

    ganttChart->getHeaderCanvas()->update();
    ganttChart->getChartCanvas()->update();

    setCursor(KCursor::arrowCursor());
}

void
TjReport::generateTaskListLine(const QtReportElement* reportElement,
                               const Task* t, QListViewItem* lvi,
                               const Resource* r)
{
    assert(reportElement != 0);
    assert(t != 0);
    assert(lvi != 0);

    // Skip the first two columns. They contain the hardwired task name and the
    // sort index column.
    int column = 2;
    for (QPtrListIterator<TableColumnInfo>
         ci = reportElement->getColumnsIterator(); *ci; ++ci, ++column)
    {
        /* The name and indices columns are automatically added as first
         * columns, so we will just ignore them if the user has requested them
         * as well. Calendar and chart columns get special treatment as well. */
        if (indexColumns.find((*ci)->getName()) != indexColumns.end() ||
            specialColumns.find((*ci)->getName()) != specialColumns.end() ||
            (*ci)->getName() == "chart")
        {
            column--;
            continue;
        }

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
        else if ((*ci)->getName() == "depends")
        {
            for (TaskListIterator it(t->getPreviousIterator()); *it != 0; ++it)
            {
                if (!cellText.isEmpty())
                    cellText += ", ";
                cellText += (*it)->getId();
            }
        }
        else if ((*ci)->getName() == "duration")
            cellText = reportElement->scaledDuration
                (t->getCalcDuration(scenario), tcf->realFormat);
        else if ((*ci)->getName() == "effort")
        {
            double val = 0.0;
            val = t->getLoad(scenario, Interval(t->getStart(scenario),
                                                t->getEnd(scenario)), r);
            cellText = indent
                (reportElement->scaledLoad(val, tcf->realFormat),
                 lvi, tcf->getHAlign() == TableColumnFormat::right);
        }
        else if ((*ci)->getName() == "end")
            cellText = time2user(t->getEnd(scenario) + 1,
                                 reportElement->getTimeFormat());
        else if ((*ci)->getName() == "endbuffer")
            cellText.sprintf("%3.0f", t->getEndBuffer(scenario));
        else if ((*ci)->getName() == "endbufferstart")
            cellText = time2user(t->getEndBufferStart(scenario),
                                 reportElement->getTimeFormat());
        else if ((*ci)->getName() == "follows")
        {
            for (TaskListIterator it(t->getFollowersIterator()); *it != 0; ++it)
            {
                if (!cellText.isEmpty())
                    cellText += ", ";
                cellText += (*it)->getId();
            }
        }
        else if ((*ci)->getName() == "id")
            cellText = t->getId();
        else if ((*ci)->getName() == "maxend")
            cellText = time2user(t->getMaxEnd(scenario),
                                 reportElement->getTimeFormat());
        else if ((*ci)->getName() == "maxstart")
            cellText = time2user(t->getMaxStart(scenario),
                                 reportElement->getTimeFormat());
        else if ((*ci)->getName() == "minend")
            cellText = time2user(t->getMinEnd(scenario),
                                 reportElement->getTimeFormat());
        else if ((*ci)->getName() == "minstart")
            cellText = time2user(t->getMinStart(scenario),
                                 reportElement->getTimeFormat());
        else if ((*ci)->getName() == "note" && !t->getNote().isEmpty())
        {
            if (t->getNote().length() > 25 || isRichText(t->getNote()))
                icon = KGlobal::iconLoader()->
                    loadIcon("document", KIcon::Small);
            else
                cellText = t->getNote();
        }
        else if ((*ci)->getName() == "pathcriticalness")
            cellText = indent(QString().sprintf
                              ("%f", t->getPathCriticalness(scenario)),
                              lvi, tcf->getHAlign() ==
                              TableColumnFormat::right);
        else if ((*ci)->getName() == "priority")
            cellText = indent(QString().sprintf("%d", t->getPriority()),
                              lvi, tcf->getHAlign() ==
                              TableColumnFormat::right);
        else if ((*ci)->getName() == "projectid")
            cellText = t->getProjectId() + " (" +
                reportElement->getReport()->getProject()->getIdIndex
                (t->getProjectId()) + ")";
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
        else if ((*ci)->getName() == "resources")
        {
            for (ResourceListIterator rli
                 (t->getBookedResourcesIterator(scenario)); *rli != 0; ++rli)
            {
                if (!cellText.isEmpty())
                    cellText += ", ";

                cellText += (*rli)->getName();
            }
        }
        else if ((*ci)->getName() == "responsible")
        {
            if (t->getResponsible())
                cellText = t->getResponsible()->getName();
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
                                 reportElement->getTimeFormat());
        else if ((*ci)->getName() == "startbuffer")
            cellText.sprintf("%3.0f", t->getStartBuffer(scenario));
        else if ((*ci)->getName() == "startbufferend")
            cellText = time2user(t->getStartBufferEnd(scenario),
                                 reportElement->getTimeFormat());
        else if ((*ci)->getName() == "status")
        {
            cellText = t->getStatusText(scenario);
        }
        else if ((*ci)->getName() == "statusnote")
        {
            if (t->getStatusNote(scenario).length() > 25 ||
                isRichText(t->getStatusNote(scenario)))
                icon = KGlobal::iconLoader()->
                    loadIcon("document", KIcon::Small);
            else
                cellText = t->getStatusNote(scenario);
        }
        else
            generateCustomAttribute(t, (*ci)->getName(), cellText, icon);

        lvi->setText(column, cellText);
        if (!icon.isNull())
            lvi->setPixmap(column, icon);
    }
}

void
TjReport::generateResourceListLine(const QtReportElement* reportElement,
                                   Resource* r, QListViewItem* lvi,
                                   const Task* t)
{
    assert(reportElement != 0);
    assert(r != 0);
    assert(lvi != 0);

    // Skip the first colum. It contains the hardwired resource name.
    int column = 2;
    for (QPtrListIterator<TableColumnInfo>
         ci = reportElement->getColumnsIterator(); *ci; ++ci, ++column)
    {
        /* The name and indices columns are automatically added as first
         * columns, so we will just ignore them if the user has requested them
         * as well. Calendar and chart columns get special treatment as well. */
        if (indexColumns.find((*ci)->getName()) != indexColumns.end() ||
            specialColumns.find((*ci)->getName()) != specialColumns.end() ||
            (*ci)->getName() == "chart")
        {
            column--;
            continue;
        }

        QString cellText;
        QPixmap icon;
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
        else if ((*ci)->getName() == "efficiency")
        {
            cellText = QString().sprintf("%.1lf", r->getEfficiency());
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
        else if ((*ci)->getName() == "id")
        {
            cellText = r->getFullId();
        }
        else if ((*ci)->getName() == "maxeffort")
        {
            const UsageLimits* limits = r->getLimits();
            if (limits == 0)
                cellText = i18n("no Limits");
            else
            {
                int sg = report->getProject()->getScheduleGranularity();
                if (limits->getDailyMax() > 0)
                    cellText = i18n("D: %1h").arg(limits->getDailyMax() *
                                               sg / (60 * 60));
                if (limits->getWeeklyMax() > 0)
                {
                    if (!cellText.isEmpty())
                        cellText += ", ";
                    cellText += i18n("W: %1h").arg(limits->getWeeklyMax() *
                                                sg / (60 * 60));
                }
                if (limits->getMonthlyMax() > 0)
                {
                    if (!cellText.isEmpty())
                        cellText += ", ";
                    cellText += i18n("M: %1d").arg(limits->getMonthlyMax() *
                                                      sg / (60 * 60 * 24));
                }
            }
        }
        else if ((*ci)->getName() == "projectids")
            cellText = r->getProjectIDs
                (scenario, Interval(reportElement->getStart(),
                                    reportElement->getEnd()));
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
        else if ((*ci)->getName() == "utilization")
        {
            if (!t)
            {
                double load = r->getLoad
                    (scenario, Interval(reportElement->getStart(),
                                        reportElement->getEnd()));
                double val;
                if (load <= 0.0)
                    val = 0.0;
                else
                {
                    double freeLoad = r->getAvailableWorkLoad
                        (scenario, Interval(reportElement->getStart(),
                                            reportElement->getEnd()));
                    val = 100.0 / (1.0 + (freeLoad / load));
                }
                cellText = indent(QString().sprintf("%.1f%%", val), lvi,
                                  tcf->getHAlign() == TableColumnFormat::right);
            }
        }
        else
            generateCustomAttribute(r, (*ci)->getName(), cellText, icon);

        lvi->setText(column, cellText);
        if (!icon.isNull())
            lvi->setPixmap(column, icon);
    }
}

void
TjReport::generateCustomAttribute(const CoreAttributes* ca, const QString name,
                                  QString& cellText, QPixmap& icon) const
{
    // Handle custom attributes
    const CustomAttribute* custAttr =
        ca->getCustomAttribute(name);
    if (custAttr)
    {
        switch (custAttr->getType())
        {
            case CAT_Undefined:
                break;
            case CAT_Text:
            {
                QString text =
                    dynamic_cast<const TextAttribute*>(custAttr)->
                    getText();
                if (text.length() > 25 || isRichText(text))
                    icon = KGlobal::iconLoader()->
                        loadIcon("document", KIcon::Small);
                else
                    cellText = text;
                break;
            }
            case CAT_Reference:
                cellText =
                    dynamic_cast<const
                    ReferenceAttribute*>(custAttr)->getLabel();
                icon = KGlobal::iconLoader()->
                    loadIcon("html", KIcon::Small);
                break;
        }
    }
}

void
TjReport::prepareChart()
{
    /* The object position mapping table changes most likely with every
     * re-generation. So we delete it and create a new one. */
    delete objPosTable;
    objPosTable = new TjObjPosTable;
    for (std::map<const QString, QListViewItem*, ltQString>::iterator
         lvit = ca2lviDict.begin(); lvit != ca2lviDict.end(); ++lvit)
    {
        const QListViewItem* lvi = (*lvit).second;
        if (!lvi)
            continue;

        // Find out if the list entry is visible at all.
        const QListViewItem* p;
        bool isVisible = true;
        for (p = lvi->parent(); p; p = p->parent())
            if (!p->isOpen())
            {
                isVisible = false;
                break;
            }
        // If no, we ignore it.
        if (!isVisible)
            continue;

        // Reconstruct the CoreAttributes pointers.
        QStringList tokens = QStringList::split(":", (*lvit).first);
        CoreAttributes* ca1 = 0;
        CoreAttributes* ca2 = 0;
        const Project* project = report->getProject();
        if (tokens[0] == "t")
        {
            if (tokens[2].isEmpty())
                ca1 = project->getTask(tokens[1]);
            else
            {
                ca1 = project->getResource(tokens[1]);
                ca2 = project->getTask(tokens[2]);
                assert(ca2 != 0);
            }
        }
        else
        {
            if (tokens[2].isEmpty())
                ca1 = project->getResource(tokens[1]);
            else
            {
                ca1 = project->getTask(tokens[1]);
                ca2 = project->getResource(tokens[2]);
                assert(ca2 != 0);
            }
        }
        assert(ca1 != 0);
        objPosTable->addEntry(ca1, ca2, lvi->itemPos(), lvi->height());
    }

    // Make sure that we only prepare the chart if the listView isn't empty.

    if (!listView->firstChild())
    {
        QValueList<int> sizes;
        sizes.append(width());
        sizes.append(0);
        splitter->setSizes(sizes);
        KMessageBox::information
            (this, i18n("The report does not contain any data. Either there "
                        "were no properties defined for this report or the "
                        "filter expressions have eliminated all entries."),
             QString::null, "EmptyReportInfo");
        return;
    }

    // Calculate some commenly used values;
    headerHeight = listView->header()->height();
    itemHeight = listView->firstChild()->height();
    QListViewItem* lvi;
    for (lvi = listView->firstChild(); lvi && lvi->itemBelow();
         lvi = lvi->itemBelow())
        ;
    listHeight = lvi->itemPos() + itemHeight;

    // Resize header canvas to new size.
    ganttHeaderView->setFixedHeight(headerHeight);

    ganttChart->setProjectAndReportData(getReportElement());
    QValueList<int> sizes = splitter->sizes();
    if (autoFit)
    {
        /* In autoFit mode we show 1/3 table and 2/3 gantt chart. Otherwise we
         * just keep the current size of the splitter. */
        if (showGantt)
        {
            sizes[0] = static_cast<int>(width() / 3.0);
            sizes[1] = static_cast<int>(width() * 2.0/3.0);
        }
        else
        {
            sizes[0] = width();
            sizes[1] = 0;
        }
        splitter->setSizes(sizes);
    }

    ganttChart->setSizes(objPosTable, headerHeight, listHeight,
                         sizes[1] == 0 ? static_cast<int>(width() * 2.0/3.0) :
                         sizes[1],
                         itemHeight);
    QPaintDeviceMetrics metrics(ganttChartView);
    ganttChart->setDPI(metrics.logicalDpiX(), metrics.logicalDpiY());
    setGanttChartColors();
    ganttChart->setHeaderHeight(headerHeight);
    ganttChart->generate(autoFit ? TjGanttChart::fitSize:
                         TjGanttChart::manual);
    updateZoomSelector();

    canvasFrame->setMaximumWidth(ganttChart->getWidth());
}

void
TjReport::generateListHeader(const QString& firstHeader, QtReportElement* tab)
{
    // The first column is always the Task/Resource column
    listView->addColumn(firstHeader + "\n");
    // The second column is the sort index. It is always hidden.
    listView->addColumn("sortIndex");
    listView->setColumnWidthMode(1, QListView::Manual);
    listView->hideColumn(1);
    listView->setSortOrder(Qt::Ascending);
    listView->setSortColumn(1);

    showGantt = false;
    int col = 2;
    for (QPtrListIterator<TableColumnInfo>
         ci = tab->getColumnsIterator(); *ci; ++ci, ++col)
    {
        /* The name and indices columns are automatically added as first
         * columns, so we will just ignore them if the user has requested them
         * as well. Calendar columns get special treatment as well. */
        if ((*ci)->getName() == "chart")
        {
            showGantt = true;
            col--;
            continue;
        }

        if (indexColumns.find((*ci)->getName()) != indexColumns.end() ||
            specialColumns.find((*ci)->getName()) != specialColumns.end())
        {
            col--;
            continue;
        }

        const TableColumnFormat* tcf =
            tab->getColumnFormat((*ci)->getName());
        QString title = tcf->getTitle();
        if (!(*ci)->getTitle().isEmpty())
            title = (*ci)->getTitle();
        listView->addColumn(title + "\n");
        listView->setColumnAlignment(col, tcf->getHAlign());
    }
}

void
TjReport::collapsReportItem(QListViewItem*)
{
    if (loadingProject)
        return;

    regenerateChart();

    syncVSlidersGantt2List(ganttChartView->contentsX(), listView->contentsY());
}

void
TjReport::expandReportItem(QListViewItem*)
{
    if (loadingProject)
        return;

    regenerateChart();
    syncVSlidersGantt2List(ganttChartView->contentsX(), listView->contentsY());
}

void
TjReport::listClicked(QListViewItem* lvi, const QPoint&, int column)
{
    // The first column is always the name and the second column is the hidden
    // sort index. Both are not in the TCI table. All clickable columns have
    // an icon.
    if (!lvi || column <= 1 || !lvi->pixmap(column))
        return;

    CoreAttributes* ca = lvi2caDict[QString().sprintf("%p", lvi)];
    const TableColumnInfo* tci =
        this->getReportElement()->columnsAt(column - 2);

    if (ca->getType() == CA_Task &&
        tci->getName() == "note" &&
        !(dynamic_cast<Task*>(ca))->getNote().isEmpty())
    {
        Task* t = dynamic_cast<Task*>(ca);
        // Open a new window that displays the note attached to the task.
        RichTextDisplay* richTextDisplay =
            new RichTextDisplay(topLevelWidget());
        richTextDisplay->setCaption(i18n("Note for Task %1 (%2) - TaskJuggler")
                                    .arg(t->getName()).arg(t->getId()));
        richTextDisplay->textDisplay->setTextFormat(Qt::RichText);

        richTextDisplay->textDisplay->setText(t->getNote());
        richTextDisplay->show();
    }
    else if (ca->getType() == CA_Task &&
             tci->getName() == "statusnote" &&
             !(dynamic_cast<Task*>(ca))->getStatusNote(scenario).isEmpty())
    {
        Task* t = dynamic_cast<Task*>(ca);
        // Open a new window that displays the note attached to the task.
        RichTextDisplay* richTextDisplay =
            new RichTextDisplay(topLevelWidget());
        richTextDisplay->setCaption
            (i18n("Status Note for Task %1 (%2) - TaskJuggler")
             .arg(t->getName()).arg(t->getId()));
        richTextDisplay->textDisplay->setTextFormat(Qt::RichText);

        richTextDisplay->textDisplay->setText(t->getStatusNote(scenario));
        richTextDisplay->show();
    }
    else if (ca->getCustomAttribute(tci->getName()))
    {
        switch (ca->getCustomAttribute(tci->getName())->getType())
        {
            case CAT_Undefined:
                break;
            case CAT_Text:
            {
                const TextAttribute* textAttr =
                    dynamic_cast<const TextAttribute*>
                    (ca->getCustomAttribute(tci->getName()));
                RichTextDisplay* richTextDisplay =
                    new RichTextDisplay(topLevelWidget());
                richTextDisplay->setCaption
                    (i18n("%1 for %2 %3 (%4) - TaskJuggler")
                     .arg(tci->getName())
                     .arg(ca->getType() == CA_Task ? i18n("Task") :
                          i18n("Resource"))
                     .arg(ca->getName())
                     .arg(ca->getId()));
                richTextDisplay->textDisplay->setTextFormat(Qt::RichText);

                richTextDisplay->textDisplay->setText(textAttr->getText());
                richTextDisplay->show();
                break;
            }
            case CAT_Reference:
            {
                const ReferenceAttribute* refAttr =
                    dynamic_cast<const ReferenceAttribute*>
                    (ca->getCustomAttribute(tci->getName()));
                KRun::runURL(KURL(refAttr->getURL()), "text/html");
                break;
            }
        }
    }
}

void
TjReport::listHeaderClicked(int)
{
    regenerateChart();
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
    if (!task->getNote().isEmpty())
    {
        if (!text.isEmpty())
            text += "<hr/>";
        text += i18n("<b>Note:</b> %1<br/>").arg(task->getNote());
    }

    if (task->isMilestone())
    {
        if (!text.isEmpty())
            text += "<hr/>";
        text += i18n("<b>Date:</b> %1<br/>")
            .arg(time2tjp(task->getStart(scenario)));
    }
    else
    {
        if (!text.isEmpty())
            text += "<hr/>";
        text += i18n("<b>Start:</b> %1<br/>"
                     "<b>End:</b> %2<br/>"
                     "<b>Status:</b> %3<br/>")
            .arg(time2tjp(task->getStart(scenario)))
            .arg(time2tjp(task->getEnd(scenario) + 1))
            .arg(task->getStatusText(scenario));

        if (task->getEffort(scenario) > 0.0)
        {
            const ReportElement* reportElement = getReportElement();
            double completion = task->getCompletionDegree(scenario) / 100.0;
            text += i18n("<hr/><b>Effort:</b> %1<br/>"
                         "<b>Completion degree:</b> %2%<br/>"
                         "<b>Done effort:</b> %3<br/>"
                         "<b>Remaining effort:</b> %4<br/>")
                .arg(reportElement->scaledLoad
                 (task->getEffort(scenario), report->getNumberFormat(),
                  true, false))
                .arg(task->getCompletionDegree(scenario))
                .arg(reportElement->scaledLoad
                     (task->getEffort(scenario) * completion,
                      report->getNumberFormat(), true, false))
                .arg(reportElement->scaledLoad
                     (task->getEffort(scenario) * (1.0 - completion),
                      report->getNumberFormat(), true, false));
        }

        if (!task->getStatusNote(scenario).isEmpty())
            text += i18n("<b>Note:</b> %1<br/>")
                .arg(task->getStatusNote(scenario));
    }

    QString predecessors;
    for (TaskListIterator tli(task->getPreviousIterator()); *tli; ++tli)
        predecessors += "<li>" + (*tli)->getName() + " (" + (*tli)->getId() +
            ")</li>";

    QString successors;
    for (TaskListIterator tli(task->getFollowersIterator()); *tli; ++tli)
        successors += "<li>" + (*tli)->getName() + " (" + (*tli)->getId() +
            ")<li/>";

    if (!predecessors.isEmpty() || !successors.isEmpty())
    {
        text += "<hr/>";
        if (!predecessors.isEmpty())
            text += i18n("<b>Predecessors:</b><ul>%1</ul><br/>")
                .arg(predecessors);
        if (!successors.isEmpty())
            text += i18n("<b>Successors:</b><ul>%1</ul><br/>").arg(successors);
    }

    ResourceListIterator rli = task->getBookedResourcesIterator(scenario);
    if (*rli)
    {
        text += "<hr/><b>Allocated resources:</b><ul>";
        for (; *rli; ++rli)
            text += "<li>" + (*rli)->getName() + " (" + (*rli)->getId() +
                ")</li>";
        text += "</ul>";
    }

    text += generateRTCustomAttributes(task);

    if (task->hasJournal())
    {
        text += "<hr/>";
        text += generateJournal(task->getJournalIterator());
    }

    richTextDisplay->textDisplay->setText(text);
    richTextDisplay->show();
}

void
TjReport::showResourceDetails(Resource* resource)
{
    RichTextDisplay* richTextDisplay = new RichTextDisplay(topLevelWidget());
    richTextDisplay->setCaption
        (i18n("Details of Resource %1 (%2) - TaskJuggler")
         .arg(resource->getName()).arg(resource->getFullId()));
    richTextDisplay->textDisplay->setTextFormat(Qt::RichText);

    QString text;
    Interval iv = Interval(report->getStart(),
                           report->getEnd());
    double load = resource->getLoad(scenario, iv);
    double freeLoad = resource->getAvailableWorkLoad(scenario, iv);

    text = i18n("<b>Effort:</b> %1 <b>Free Load:</b> %2 "
                "<b>Utilization:</b> %3%")
        .arg(scaledLoad(load, numberFormat, true))
        .arg(scaledLoad(freeLoad, numberFormat, true))
        .arg((int) (load / (load + freeLoad) * 100.0));

    if (resource->hasJournal())
    {
        if (!text.isEmpty())
            text += "<hr/>";
        text += generateJournal(resource->getJournalIterator());
    }

    text += generateRTCustomAttributes(resource);

    if (resource->hasJournal())
    {
        text += "<hr/>";
        text += generateJournal(resource->getJournalIterator());
    }

    richTextDisplay->textDisplay->setText(text);
    richTextDisplay->show();
}

QString
TjReport::generateRTCustomAttributes(const CoreAttributes* ca) const
{
    QDict<CustomAttribute> caDict = ca->getCustomAttributeDict();

    QString text = "<hr/>";
    if (caDict.isEmpty())
        return text;

    for (QDictIterator<CustomAttribute> cadi(caDict); cadi.current(); ++cadi)
    {
        text += "<b>" + cadi.currentKey() + ":</b> ";
        CustomAttribute* custAttr = cadi.current();
        switch (cadi.current()->getType())
        {
            case CAT_Reference:
            {
                QString label = dynamic_cast<const
                    ReferenceAttribute*>(custAttr)->getLabel();
                QString url = dynamic_cast<const
                    ReferenceAttribute*>(custAttr)->getURL();
                text += "<a href=\"" + url + "\">" +
                    (label.isEmpty() ? url : label) + "</a>";
                break;
            }
            case CAT_Text:
                text += dynamic_cast<const TextAttribute*>(custAttr)->
                    getText();
                break;
            case CAT_Undefined:
                break;
        }
        text += "<br/>";
    }

    return text;
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
    if (loadingProject || !isVisible() || !ganttChartView->isVisible())
        return;

    QPoint pos = ganttChartView->mapFromGlobal(QCursor::pos());
    if (pos.x() < 0 || pos.y() < 0 ||
        pos.x() > ganttChartView->width() ||
        pos.y() > ganttChartView->height())
    {
        emit signalChangeStatusBar("");
        return;
    }

    QListViewItem* lvi = listView->itemAt(QPoint(50, pos.y()));
    if (!lvi)
    {
        emit signalChangeStatusBar("");
        return;
    }

    CoreAttributes* ca = lvi2caDict[QString().sprintf("%p", lvi)];
    CoreAttributes* parent = lvi2ParentCaDict[QString().sprintf("%p", lvi)];

    emit signalChangeStatusBar(this->generateStatusBarText(pos, ca, parent));
}

void
TjReport::zoomTo(const QString& label)
{
    if (!isVisible())
        return;

    time_t x = ganttChart->x2time(ganttChartView->contentsX());
    int y = ganttChartView->contentsY();

    if (!ganttChart->zoomTo(label))
        return;

    canvasFrame->setMaximumWidth(ganttChart->getWidth());

    ganttHeaderView->repaint();
    ganttChartView->repaint();
    update();

    ganttHeaderView->setContentsPos(ganttChart->time2x(x), 0);
    ganttChartView->setContentsPos(ganttChart->time2x(x), y);
}

void
TjReport::zoomIn()
{
    if (!isVisible())
        return;

    time_t x = ganttChart->x2time(ganttChartView->contentsX());
    int y = ganttChartView->contentsY();

    if (!ganttChart->zoomIn())
        return;
    canvasFrame->setMaximumWidth(ganttChart->getWidth());

    ganttHeaderView->repaint();
    ganttChartView->repaint();
    update();

    ganttHeaderView->setContentsPos(ganttChart->time2x(x), 0);
    ganttChartView->setContentsPos(ganttChart->time2x(x), y);

    updateZoomSelector();
}

void
TjReport::zoomOut()
{
    if (!isVisible())
        return;

    time_t x = ganttChart->x2time(ganttChartView->contentsX());
    int y = ganttChartView->contentsY();

    if (!ganttChart->zoomOut())
        return;
    canvasFrame->setMaximumWidth(ganttChart->getWidth());

    ganttHeaderView->repaint();
    ganttChartView->repaint();
    update();

    ganttHeaderView->setContentsPos(ganttChart->time2x(x), 0);
    ganttChartView->setContentsPos(ganttChart->time2x(x), y);

    updateZoomSelector();
}

void
TjReport::show()
{
    QWidget::show();

    if (statusBarUpdateTimer)
        statusBarUpdateTimer->start(500, FALSE);

    updateZoomSelector();
}

void
TjReport::hide()
{
    if (statusBarUpdateTimer)
        statusBarUpdateTimer->stop();

    QWidget::hide();
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
    assert(lvi != 0);

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

QString
TjReport::generateJournal(JournalIterator jit) const
{
    QString text;

    for ( ; *jit; ++jit)
        text += "<b><i>" + time2user((*jit)->getDate(),
                                     report->getTimeFormat()) +
            "</i></b><br/>" + (*jit)->getText() + "<br/>";

    return text;
}

void
TjReport::setGanttChartColors()
{
    ganttChart->setColor("headerBackgroundCol", colorGroup().background());
    ganttChart->setColor("headerLineCol", Qt::black);
    ganttChart->setColor("headerShadowCol", colorGroup().mid());
    ganttChart->setColor("chartBackgroundCol", listView->colorGroup().base());
    ganttChart->setColor("chartAltBackgroundCol",
                         KGlobalSettings::calculateAlternateBackgroundColor
                         (listView->colorGroup().base()));
    ganttChart->setColor("chartTimeOffCol",
                         KGlobalSettings::calculateAlternateBackgroundColor
                         (listView->colorGroup().base()).dark(110));
    ganttChart->setColor("chartLineCol",
                         KGlobalSettings::calculateAlternateBackgroundColor
                         (listView->colorGroup().base()).dark(130));
}

void
TjReport::updateZoomSelector()
{
    manager->updateZoomSelector(ganttChart->getZoomStepLabels(),
                                ganttChart->getCurrentZoomStep());
}

#include "TjReport.moc"
