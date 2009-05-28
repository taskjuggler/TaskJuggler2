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

#include "TjReport.h"

#include <assert.h>

#include <qsplitter.h>
#include <qlayout.h>
#include <qheader.h>
#include <qcanvas.h>
#include <qdatetime.h>
#include <qtimer.h>
#include <qpopupmenu.h>
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
#include <kmessagebox.h>
#include <klistviewsearchline.h>

#include "Project.h"
#include "Task.h"
#include "Resource.h"
#include "Account.h"
#include "Utility.h"
#include "ExpressionTree.h"
#include "Report.h"
#include "TableColumnFormat.h"
#include "TextAttribute.h"
#include "ReferenceAttribute.h"
#include "QtTaskReport.h"
#include "QtResourceReport.h"
#include "RichTextDisplay.h"
#include "TjPrintReport.h"
#include "TjGanttChart.h"
#include "TjObjPosTable.h"
#include "KPrinterWrapper.h"
#include "UsageLimits.h"
#include "ReportManager.h"
#include "ReportController.h"
#include "kdateedit.h"

TjReport::TjReport(QWidget* p, ReportManager* m, Report* rDef,
                   const QString& n)
    : TjUIReportBase(p, m, rDef, n)
{
    loadingProject = false;
    scaleMode = TjGanttChart::fitSize;

    QVBoxLayout* vl = new QVBoxLayout(this, 0, 0);
    reportFrame = new QWidget(this);
    reportFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QHBoxLayout* hl = new QHBoxLayout(reportFrame, 0, 0);
    splitter = new QSplitter(Horizontal, reportFrame);

    listView = new KListView(splitter);
    listView->setRootIsDecorated(true);
    listView->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    listView->setAllColumnsShowFocus(true);
    // The sorting does not work yet properly.
    listView->header()->setClickEnabled(false);
    listView->setItemMargin(2);

    canvasFrame = new QWidget(splitter);
    QVBoxLayout* vlChart = new QVBoxLayout(canvasFrame, 0, 0);
    canvasFrame->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);

    ganttChart = new TjGanttChart(reportFrame);
    objPosTable = 0;

    ganttHeaderView = new QCanvasView(ganttChart->getHeaderCanvas(),
                                      canvasFrame);
    ganttHeaderView->setHScrollBarMode(QScrollView::AlwaysOff);
    ganttHeaderView->setVScrollBarMode(QScrollView::AlwaysOff);

    ganttChartView = new QCanvasView(ganttChart->getChartCanvas(),
                                     canvasFrame);
    ganttChartView->setVScrollBarMode(QScrollView::AlwaysOff);

    reportController = new ReportController(this);
    reportController->reportSearch->setListView(listView);
    reportController->reportStart->setDate(time2qdate(rDef->getStart()));
    reportController->reportEnd->setDate(time2qdate(rDef->getEnd()));

    vl->addWidget(reportFrame);
    vl->addWidget(reportController);
    vlChart->addWidget(ganttHeaderView);
    vlChart->addWidget(ganttChartView);
    hl->addWidget(splitter);

    statusBarUpdateTimer = delayTimer = 0;

    connect(listView, SIGNAL(selectionChanged()),
            this, SLOT(regenerateChart()));
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
    connect(reportController->reportSearch, SIGNAL(textChanged(const QString&)),
            this, SLOT(reportSearchTriggered(const QString&)));
    connect(reportController->reportStart, SIGNAL(dateChanged(const QDate&)),
            this, SLOT(setReportStart(const QDate&)));
    connect(reportController->reportEnd, SIGNAL(dateChanged(const QDate&)),
            this, SLOT(setReportEnd(const QDate&)));

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
        bool first = true;
        for (int y = 0; y < yPages; ++y)
            for (int x = 0; x < xPages; ++x)
            {
                if (first)
                    first = false;
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

void
TjReport::setFocus()
{
    listView->setFocus();
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
    // Double click facility
    else if (ev->type() == QEvent::MouseButtonPress || ev->type() == QEvent::MouseButtonDblClick)
        handleMouseEvent(static_cast<QMouseEvent*>(ev));

    return QWidget::event(ev);
}

bool
TjReport::generateReport()
{
    setLoadingProject(true);

    setCursor(KCursor::waitCursor());
    if (!this->generateList())
    {
        setLoadingProject(false);
        setCursor(KCursor::arrowCursor());
        return false;
    }
    setLoadingProject(false);
    setCursor(KCursor::arrowCursor());

    /* The first time we generate the report, the window has not been fully
     * layouted yet. So we can't set the splitter to a good size and generate
     * the gantt report immediately. We use a 200ms timer to delay the
     * rendering. Hopefully by then the window has been layouted properly. */
    triggerChartRegeneration(200);

    delete statusBarUpdateTimer;
    statusBarUpdateTimer = new QTimer(this);
    connect(statusBarUpdateTimer, SIGNAL(timeout()),
            this, SLOT(updateStatusBar()));
    statusBarUpdateTimer->start(500, false);

    return true;
}

void
TjReport::triggerChartRegeneration(int msDelay)
{
    if (delayTimer == 0)
    {
        delayTimer = new QTimer(this);
        connect(delayTimer, SIGNAL(timeout()),
                this, SLOT(regenerateChart()));
    }
    delayTimer->start(msDelay, true);
}

void
TjReport::regenerateChart()
{
    delete delayTimer;
    delayTimer = 0;

    if (loadingProject) return;

    setCursor(KCursor::waitCursor());
    setLoadingProject(true);

    prepareChart();

    // When we are here, we have rendered the widgets at least once. So we can
    // turn off manual mode.
    scaleMode = TjGanttChart::manual;

    ganttChart->getHeaderCanvas()->update();
    ganttChart->getChartCanvas()->update();

    setLoadingProject(false);
    setCursor(KCursor::arrowCursor());
}

void
TjReport::generateTaskListLine(const QtReportElement* reportElement,
                               const Task* t, KListViewItem* lvi,
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
        const QString& name((*ci)->getName());

        /* The name and indices columns are automatically added as first
         * columns, so we will just ignore them if the user has requested them
         * as well. Calendar and chart columns get special treatment as well. */
        if (indexColumns.find(name) != indexColumns.end() ||
            specialColumns.find(name) != specialColumns.end() ||
            name == "chart")
        {
            column--;
            continue;
        }

        QString cellText;
        QPixmap icon;

        const TableColumnFormat* tcf =
            reportElement->getColumnFormat(name);

        if (name == "accounts")
        {
            if (t->getAccount())
                cellText = t->getAccount()->getId();
        }
        else if (name == "completed")
        {
            double calcedCompletionDegree =
                t->getCalcedCompletionDegree(scenario);
            double providedCompletionDegree =
                t->getCompletionDegree(scenario);

            if (calcedCompletionDegree < 0)
            {
                if (calcedCompletionDegree == providedCompletionDegree)
                {
                    cellText = i18n("in progress");
                }
                else
                {
                    cellText = QString(i18n("%1% (in progress)"))
                        .arg((int) providedCompletionDegree);
                }
            }
            else
            {
                if (calcedCompletionDegree == providedCompletionDegree)
                {
                    cellText = QString("%1%")
                        .arg((int) providedCompletionDegree);
                }
                else
                {
                    cellText = QString("%1% (%2%)")
                            .arg((int) providedCompletionDegree)
                            .arg((int) calcedCompletionDegree);
                }
            }
        }
        else if (name == "completedeffort")
        {
            double val = 0.0;
            if (!r && t->isLeaf())
            {
                // Task line, no resource.
                val = t->getCompletedLoad(scenario);
            }
            else if (t && t->isLeaf())
            {
                // Task line, nested into a resource
                const Project* project = report->getProject();
                time_t now = project->getNow();
                if (now < project->getStart())
                    now = project->getStart();
                if (now > project->getEnd())
                    now = project->getEnd();
                Interval iv = Interval(project->getStart(), now);
                val = t->getLoad(scenario, iv, r);
            }
            cellText = indent
                (reportElement->scaledLoad(val, tcf->realFormat),
                 lvi, tcf->getHAlign() == TableColumnFormat::right);
        }
        else if (name == "cost")
        {
            double val = t->getCredits
                (scenario, Interval(reportElement->getStart(),
                                    reportElement->getEnd()), Cost, r);
            cellText = indent(tcf->realFormat.format(val, false),
                              lvi, tcf->getHAlign() ==
                              TableColumnFormat::right);
        }
        else if (name == "criticalness")
        {
            cellText = indent(QString().sprintf("%f",
                                                t->getCriticalness(scenario)),
                              lvi, tcf->getHAlign() ==
                              TableColumnFormat::right);
        }
        else if (name == "depends")
        {
            for (TaskListIterator it(t->getPreviousIterator()); *it != 0; ++it)
            {
                if (!cellText.isEmpty())
                    cellText += ", ";
                cellText += (*it)->getId();
            }
        }
        else if (name == "duration")
            cellText = reportElement->scaledDuration
                (t->getCalcDuration(scenario), tcf->realFormat);
        else if (name == "effort")
        {
            double val = 0.0;
            val = t->getLoad(scenario, Interval(t->getStart(scenario),
                                                t->getEnd(scenario)), r);
            cellText = indent
                (reportElement->scaledLoad(val, tcf->realFormat),
                 lvi, tcf->getHAlign() == TableColumnFormat::right);
        }
        else if (name == "end")
            cellText = time2user(t->getEnd(scenario) + 1,
                                 reportElement->getTimeFormat());
        else if (name == "endbuffer")
            cellText.sprintf("%3.0f", t->getEndBuffer(scenario));
        else if (name == "endbufferstart")
            cellText = time2user(t->getEndBufferStart(scenario),
                                 reportElement->getTimeFormat());
        else if (name == "follows")
        {
            for (TaskListIterator it(t->getFollowersIterator()); *it != 0; ++it)
            {
                if (!cellText.isEmpty())
                    cellText += ", ";
                cellText += (*it)->getId();
            }
        }
        else if (name == "id")
            cellText = t->getId();
        else if (name == "maxend")
            cellText = time2user(t->getMaxEnd(scenario),
                                 reportElement->getTimeFormat());
        else if (name == "maxstart")
            cellText = time2user(t->getMaxStart(scenario),
                                 reportElement->getTimeFormat());
        else if (name == "minend")
            cellText = time2user(t->getMinEnd(scenario),
                                 reportElement->getTimeFormat());
        else if (name == "minstart")
            cellText = time2user(t->getMinStart(scenario),
                                 reportElement->getTimeFormat());
        else if (name == "note" && !t->getNote().isEmpty())
        {
            if (t->getNote().length() > 25 || isRichText(t->getNote()))
                icon = KGlobal::iconLoader()->
                    loadIcon("document", KIcon::Small);
            else
                cellText = t->getNote();
        }
        else if (name == "pathcriticalness")
            cellText = indent(QString().sprintf
                              ("%f", t->getPathCriticalness(scenario)),
                              lvi, tcf->getHAlign() ==
                              TableColumnFormat::right);
        else if (name == "priority")
            cellText = indent(QString().sprintf("%d", t->getPriority()),
                              lvi, tcf->getHAlign() ==
                              TableColumnFormat::right);
        else if (name == "projectid")
            cellText = t->getProjectId() + " (" +
                reportElement->getReport()->getProject()->getIdIndex
                (t->getProjectId()) + ")";
        else if (name == "profit")
        {
            double val = t->getCredits
                (scenario, Interval(reportElement->getStart(),
                                    reportElement->getEnd()), Revenue, r) -
                t->getCredits
                (scenario, Interval(reportElement->getStart(),
                                    reportElement->getEnd()), Cost, r);
            cellText = indent(tcf->realFormat.format(val, false),
                              lvi, tcf->getHAlign() ==
                              TableColumnFormat::right);
        }
        else if (name == "remainingeffort")
        {
            double val = 0.0;
            if (!r && t->isLeaf())
            {
                // Task line, no resource.
                val = t->getRemainingLoad(scenario);
            }
            else if (t && t->isLeaf())
            {
                // Task line, nested into a resource
                const Project* project = report->getProject();
                time_t now = project->getNow();
                if (now < project->getStart())
                    now = project->getStart();
                if (now > project->getEnd())
                    now = project->getEnd();
                Interval iv = Interval(now, project->getEnd());
                val = t->getLoad(scenario, iv, r);
            }
            cellText = indent
                (reportElement->scaledLoad(val, tcf->realFormat),
                 lvi, tcf->getHAlign() == TableColumnFormat::right);
        }
        else if (name == "resources")
        {
            for (ResourceListIterator rli
                 (t->getBookedResourcesIterator(scenario)); *rli != 0; ++rli)
            {
                if (!cellText.isEmpty())
                    cellText += ", ";

                cellText += (*rli)->getName();
            }
        }
        else if (name == "responsible")
        {
            if (t->getResponsible())
                cellText = t->getResponsible()->getName();
        }
        else if (name == "revenue")
        {
            double val = t->getCredits
                (scenario, Interval(reportElement->getStart(),
                                    reportElement->getEnd()), Revenue, r);
            cellText = indent(tcf->realFormat.format(val, false),
                              lvi, tcf->getHAlign() ==
                              TableColumnFormat::right);
        }
        else if (name == "scheduling")
            cellText = t->getSchedulingText();
        else if (name == "start")
            cellText = time2user(t->getStart(scenario),
                                 reportElement->getTimeFormat());
        else if (name == "startbuffer")
            cellText.sprintf("%3.0f", t->getStartBuffer(scenario));
        else if (name == "startbufferend")
            cellText = time2user(t->getStartBufferEnd(scenario),
                                 reportElement->getTimeFormat());
        else if (name == "status")
        {
            cellText = t->getStatusText(scenario);
        }
        else if (name == "statusnote")
        {
            if (t->getStatusNote(scenario).length() > 25 ||
                isRichText(t->getStatusNote(scenario)))
                icon = KGlobal::iconLoader()->
                    loadIcon("document", KIcon::Small);
            else
                cellText = t->getStatusNote(scenario);
        }
        else
            generateCustomAttribute(t, name, cellText, icon);

        lvi->setText(column, cellText);
        if (!icon.isNull())
            lvi->setPixmap(column, icon);
    }
}

void
TjReport::generateResourceListLine(const QtReportElement* reportElement,
                                   Resource* r, KListViewItem* lvi,
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
        const QString& name((*ci)->getName());

        /* The name and indices columns are automatically added as first
         * columns, so we will just ignore them if the user has requested them
         * as well. Calendar and chart columns get special treatment as well. */
        if (indexColumns.find(name) != indexColumns.end() ||
            specialColumns.find(name) != specialColumns.end() ||
            name == "chart")
        {
            column--;
            continue;
        }

        QString cellText;
        QPixmap icon;
        const TableColumnFormat* tcf =
            reportElement->getColumnFormat(name);

        if (name == "cost")
        {
            double val = r->getCredits
                (scenario, Interval(reportElement->getStart(),
                                    reportElement->getEnd()), Cost, t);
            cellText = indent(tcf->realFormat.format(val, false),
                              lvi, tcf->getHAlign() ==
                              TableColumnFormat::right);
        }
        else if (name == "efficiency")
        {
            cellText = QString().sprintf("%.1lf", r->getEfficiency());
        }
        else if (name == "effort")
        {
            double val = 0.0;
            if (t)
                val = r->getEffectiveLoad
                    (scenario, Interval(t->getStart(scenario),
                                        t->getEnd(scenario)),
                     AllAccounts, t);
            else
                val = r->getEffectiveLoad
                    (scenario, Interval(reportElement->getStart(),
                                        reportElement->getEnd()));

            cellText = indent
                (reportElement->scaledLoad(val, tcf->realFormat), lvi,
                 tcf->getHAlign() == TableColumnFormat::right);
        }
        else if (name == "freeload")
        {
            if (!t)
            {
                double val = 0.0;
                val = r->getEffectiveFreeLoad
                    (scenario, Interval(reportElement->getStart(),
                                        reportElement->getEnd()));
                cellText = indent
                    (reportElement->scaledLoad(val, tcf->realFormat), lvi,
                     tcf->getHAlign() == TableColumnFormat::right);
            }
        }
        else if (name == "id")
        {
            cellText = r->getFullId();
        }
        else if (name == "maxeffort")
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
        else if (name == "projectids")
            cellText = r->getProjectIDs
                (scenario, Interval(reportElement->getStart(),
                                    reportElement->getEnd()));
        else if (name == "rate")
        {
            cellText = indent(tcf->realFormat.format(r->getRate(), false),
                              lvi, tcf->getHAlign() ==
                              TableColumnFormat::right);
        }
        else if (name == "revenue")
        {
            double val = r->getCredits
                (scenario, Interval(reportElement->getStart(),
                                    reportElement->getEnd()), Revenue, t);
            cellText = indent(tcf->realFormat.format(val, false),
                              lvi, tcf->getHAlign() ==
                              TableColumnFormat::right);
        }
        else if (name == "utilization")
        {
            if (!t)
            {
                double load = r->getEffectiveLoad
                    (scenario, Interval(reportElement->getStart(),
                                        reportElement->getEnd()));
                double val;
                if (load <= 0.0)
                    val = 0.0;
                else
                {
                    double freeLoad = r->getEffectiveFreeLoad
                        (scenario, Interval(reportElement->getStart(),
                                            reportElement->getEnd()));
                    val = 100.0 / (1.0 + (freeLoad / load));
                }
                cellText = indent(QString().sprintf("%.1f%%", val), lvi,
                                  tcf->getHAlign() == TableColumnFormat::right);
            }
        }
        else
            generateCustomAttribute(r, name, cellText, icon);

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
    TjObjPosTableEntry* selectedObject = 0;

    for (std::map<const QString, KListViewItem*, ltQString>::iterator
         lvit = ca2lviDict.begin(); lvit != ca2lviDict.end(); ++lvit)
    {
        KListViewItem* lvi = (*lvit).second;
        if (!lvi || !lvi->isVisible())
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
        TjObjPosTableEntry* tableEntry =
            objPosTable->addEntry(ca1, ca2, lvi->itemPos(), lvi->height(),
                                  lvi->isAlternate());

        if (lvi == listView->selectedItem())
            selectedObject = tableEntry;
    }

    // Calculate some commenly used values;
    headerHeight = listView->header()->height();
    QListViewItem* lvi;
    itemHeight = 0;
    listHeight = 0;
    for (lvi = listView->firstChild(); lvi; lvi = lvi->itemBelow())
        if (lvi->isVisible())
        {
            if (lvi->height() > itemHeight)
                itemHeight = lvi->height();
            listHeight = lvi->itemPos() + itemHeight - 1;
        }

    // Resize header canvas to new size.
    ganttHeaderView->setFixedHeight(headerHeight);

    ganttChart->setProjectAndReportData(getReportElement());
    QValueList<int> sizes = splitter->sizes();
    if (scaleMode == TjGanttChart::fitSize)
    {
        /* In fitSize mode we show 1/3 table and 2/3 gantt chart. Otherwise we
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
    ganttChart->setSelection(selectedObject);
    QPaintDeviceMetrics metrics(ganttChartView);
    ganttChart->setDPI(metrics.logicalDpiX(), metrics.logicalDpiY());
    setGanttChartColors();
    ganttChart->setHeaderHeight(headerHeight);
    ganttChart->generate(scaleMode);
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

    lvCol2tci.clear();

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

        /* Store a reference to the column info in the lvCol2tci map. */
        lvCol2tci.insert(lvCol2tci.end(), ci);

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
    if (loadingProject || !lvi || column <= 1 || !lvi->pixmap(column))
        return;

    CoreAttributes* ca = lvi2caDict[QString().sprintf("%p", lvi)];
    const TableColumnInfo* tci = lvCol2tci[column - 2];

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
    if (loadingProject || !lvi)
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
    text = i18n("<b>Task:</b> %1 (%2)<br/>")
        .arg(task->getName())
        .arg(task->getId());

    if (!task->getNote().isEmpty())
    {
        if (!text.isEmpty())
            text += "<hr/>";
        text += i18n("<b>Note:</b> %1<br/>").arg(task->getNote());
    }

    // Display status note if not empty.
    if (!task->getStatusNote(scenario).isEmpty())
    {
        if (!text.isEmpty())
            text += "<hr/>";
        text += i18n("<b>Status note:</b> %1<br/>")
            .arg(task->getStatusNote(scenario));
    }

    if (task->isMilestone())
    {
        if (!text.isEmpty())
            text += "<hr/>";
        text += i18n("<b>Date:</b> %1<br/>")
            .arg(time2user(task->getStart(scenario), task->getProject()->getTimeFormat()));

        // Display min and max start and end dates.
        if (task->getMinStart(scenario)!=0)
        {
            text += i18n("<b>Min Start:</b> %1<br/>")
                .arg(time2user(task->getMinStart(scenario), task->getProject()->getTimeFormat()));
        }
        if (task->getMinEnd(scenario)!=0)
        {
            text += i18n("<b>Min End:</b> %1<br/>")
                .arg(time2user(task->getMinEnd(scenario), task->getProject()->getTimeFormat()));
        }
        if (task->getMaxStart(scenario)!=0)
        {
            text += i18n("<b>Max Start:</b> %1<br/>")
                .arg(time2user(task->getMaxStart(scenario), task->getProject()->getTimeFormat()));
        }
        if (task->getMaxEnd(scenario)!=0)
        {
            text += i18n("<b>Max End :</b> %1<br/>")
                .arg(time2user(task->getMaxEnd(scenario), task->getProject()->getTimeFormat()));
        }
    }
    else
    {
        if (!text.isEmpty())
            text += "<hr/>";
        text += i18n("<b>Start:</b> %1<br/>"
                     "<b>End:</b> %2<br/>"
                     "<b>Status:</b> %3<br/>")
            .arg(time2user(task->getStart(scenario), task->getProject()->getTimeFormat()))
            .arg(time2user(task->getEnd(scenario) + 1, task->getProject()->getTimeFormat()))
            .arg(task->getStatusText(scenario));

        if (task->getEffort(scenario) > 0.0)
        {
            const ReportElement* reportElement = getReportElement();
            double completion = task->getCompletionDegree(scenario) / 100.0;
            text += i18n("<hr/><b>Effort:</b> %1<br/>")
                    .arg(reportElement->scaledLoad
                         (task->getEffort(scenario), report->getNumberFormat(),
                          true, false));
            if (completion < 0.0)
            {
                text += i18n("<b>Completion degree:</b> in progress<br/>");
            }
            else if (completion == 0.0)
            {
                text += i18n("<b>Completion degree:</b> 0%<br/>");
            }
            else if (completion >= 1.0)
            {
                text += i18n("<b>Completion degree:</b> 100%<br/>");
            }
            else
            {
                text += i18n("<b>Completion degree:</b> %1%<br/>"
                             "<b>Done effort:</b> %2<br/>"
                             "<b>Remaining effort:</b> %3<br/>")
                    .arg(task->getCompletionDegree(scenario))
                    .arg(reportElement->scaledLoad
                         (task->getEffort(scenario) * completion,
                          report->getNumberFormat(), true, false))
                    .arg(reportElement->scaledLoad
                         (task->getEffort(scenario) * (1.0 - completion),
                          report->getNumberFormat(), true, false));
            }
        }
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
    const ReportElement* reportElement = this->getReportElement();
    Interval iv = Interval(reportElement->getStart(),
                           reportElement->getEnd());
    double load = resource->getEffectiveLoad(scenario, iv);
    double freeLoad = resource->getEffectiveFreeLoad(scenario, iv);

    text = i18n("<b>Resource:</b> %1 (%2)<br/>")
        .arg(resource->getName())
        .arg(resource->getId());

    text += i18n("<hr><b>Effort:</b> %1 <b>Free Load:</b> %2 "
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

    // Display of vacations
    text += i18n("<hr/><b>Vacations:</b>");
    {
        QPtrListIterator<Interval> vli(resource->getVacationListIterator());

        if (*vli != 0)
        {
            text += i18n("<p/><p>Personnal vacations:</p>");
            for (QPtrListIterator<Interval> vli(resource->getVacationListIterator()); *vli != 0; ++vli)
            {
                if (sameTimeNextDay((*vli)->getStart()) == 1 + (*vli)->getEnd()
                    && secondsOfDay((*vli)->getStart()) == 0 )
                    text += i18n("<li>%1</li>")
                        .arg(time2user((*vli)->getStart(), resource->getProject()->getTimeFormat()));
                else
                    text += i18n("<li>%1 - %2</li>")
                        .arg(time2user((*vli)->getStart(), resource->getProject()->getTimeFormat()))
                        .arg(time2user((*vli)->getEnd(), resource->getProject()->getTimeFormat()));
            }
        }
        else
        {
            text += i18n("<p>No personnal vacations.</p>");
        }
    }

    {
        VacationList::Iterator vli(resource->getProject()->getVacationListIterator());
        if (*vli != 0)
        {
            text += i18n("<p>Global vacations:</p>");
            // Because the global vacation list is in reverse order and we want to display in original order, we use a temp string.
            QString tmpText;
            for ( ; *vli != 0; ++vli)
            {
                // Only display one day when vacation duration is one day, starting at midnight.
                if (sameTimeNextDay((*vli)->getStart()) == 1 + (*vli)->getEnd()
                    && secondsOfDay((*vli)->getStart()) == 0 )
                {
                    tmpText = i18n("<li>%1 : %2</li>")
                        .arg(time2user((*vli)->getStart(), resource->getProject()->getTimeFormat()))
                        .arg((*vli)->getName()) + tmpText;
                }
                // Otherwise, display the two vacation dates in user datetime format.
                else
                {
                    tmpText = i18n("<li>%1 - %2 : %3</li>")
                        .arg(time2user((*vli)->getStart(), resource->getProject()->getTimeFormat()))
                        .arg(time2user((*vli)->getEnd(), resource->getProject()->getTimeFormat()))
                        .arg((*vli)->getName()) + tmpText;
                }
            }
            text += tmpText;
        }
        else
        {
            text += i18n("<p>No global vacations:.</p>");
        }
    }
    text += "<hr/>";

    richTextDisplay->textDisplay->setText(text);
    richTextDisplay->show();
}

QString
TjReport::generateRTCustomAttributes(const CoreAttributes* ca) const
{
    QDict<CustomAttribute> caDict = ca->getCustomAttributeDict();

    QString text = "";
    if (caDict.isEmpty())
        return text;

    text += "<hr/>";

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
TjReport::handleMouseEvent(const QMouseEvent* ev)
{
    QPoint pos;
    QListViewItem* lvi = getChartItemBelowCursor(pos);
    if (loadingProject || !lvi)
        return;

    CoreAttributes* ca = lvi2caDict[QString().sprintf("%p", lvi)];

    // Middle button facility
    if (ev->button() == Qt::MidButton)
    {
        emit signalEditCoreAttributes(ca);
    }
    // Double click
    else if (ev->type() == QEvent::MouseButtonDblClick)
    {
        if (ca->getType() == CA_Task)
        {
            Task* t = dynamic_cast<Task*>(ca);
            if (t) showTaskDetails(t);
        }
        else
        {
            Resource* r = dynamic_cast<Resource*>(ca);
            if (r) showResourceDetails(r);
        }
    }
    else if (ev->button() == Qt::LeftButton)
        listView->setSelected(lvi, true);
    else if (ev->button() == Qt::RightButton)
        doPopupMenu(lvi, QCursor::pos(), 0);
}

void
TjReport::updateStatusBar()
{
    QPoint pos;
    QListViewItem* lvi = getChartItemBelowCursor(pos);
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
TjReport::reportSearchTriggered(const QString&)
{
    triggerChartRegeneration(200);
}

void
TjReport::setReportStart(const QDate& d)
{
    ReportElement* reportElement = this->getReportElement();
    time_t start = qdate2time(d);

    bool clipped = false;
    if (start < date2time("2000-01-01"))
    {
        start = date2time("2000-01-01");
        clipped = true;
    }
    if (start > date2time("2030-01-01"))
    {
        start = date2time("2030-01-01");
        clipped = true;
    }
    if (start + (24 * 60 * 60) > reportElement->getEnd())
    {
        start = reportElement->getEnd() - (24 * 60 * 60);
        clipped = true;
    }
    if (clipped)
        reportController->reportStart->setDate(time2qdate(start));

    reportElement->setStart(start);
    scaleMode = TjGanttChart::autoZoom;
    generateReport();
}

void
TjReport::setReportEnd(const QDate& d)
{
    ReportElement* reportElement = this->getReportElement();
    time_t end = qdate2time(d);

    bool clipped = false;
    if (end < date2time("2000-01-01"))
    {
        end = date2time("2000-01-01");
        clipped = true;
    }
    if (end > date2time("2030-01-01"))
    {
        end = date2time("2030-01-01");
        clipped = true;
    }
    if (end - (24 * 60 * 60) < reportElement->getStart())
    {
        end = reportElement->getStart() + (24 * 60 * 60);
        clipped = true;
    }
    if (clipped)
        reportController->reportEnd->setDate(time2qdate(end));

    reportElement->setEnd(end);
    scaleMode = TjGanttChart::autoZoom;
    generateReport();
}

QListViewItem*
TjReport::getChartItemBelowCursor(QPoint& pos)
{
    if (loadingProject || !isVisible() || !ganttChartView->isVisible())
        return 0;

    /* Since it is easier to map the global cursor position to the
     * ganttChartView coordinates than using the event position we'll got for
     * the easy way. */
    pos = ganttChartView->mapFromGlobal(QCursor::pos());
    // Make sure the cursor is really above the ganttChartView.
    if (pos.x() < 0 || pos.y() < 0 ||
        pos.x() > ganttChartView->width() ||
        pos.y() > ganttChartView->height())
        return 0;

    return listView->itemAt(QPoint(50, pos.y()));
}

void
TjReport::zoomTo(const QString& label)
{
    if (loadingProject || !isVisible())
        return;

    time_t x = ganttChart->x2time(ganttChartView->contentsX());
    int y = ganttChartView->contentsY();
    if (x <= 0 && y <= 0)
        return;

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
    if (loadingProject || !isVisible())
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
    if (loadingProject || !isVisible())
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
        statusBarUpdateTimer->start(500, false);

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
TjReport::generateJournal(Journal::Iterator jit) const
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
                         listView->alternateBackground());
    ganttChart->setColor("chartTimeOffCol",
                         listView->alternateBackground().dark(110));
    ganttChart->setColor("chartLineCol",
                         listView->alternateBackground().dark(130));
    ganttChart->setColor("hightlightCol", colorGroup().highlight());
}

void
TjReport::updateZoomSelector()
{
    manager->updateZoomSelector(ganttChart->getZoomStepLabels(),
                                ganttChart->getCurrentZoomStep());
}

#include "TjReport.moc"
