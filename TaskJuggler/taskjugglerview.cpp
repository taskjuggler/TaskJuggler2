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


#include "taskjugglerview.h"

#include <qpainter.h>
#include <qlayout.h>
#include <qlayout.h>
#include <qsplitter.h>
#include <qtabwidget.h>
#include <qtoolbox.h>
#include <qvaluelist.h>
#include <qstringlist.h>
#include <qregexp.h>
#include <qtooltip.h>
#include <qheader.h>
#include <qfont.h>
#include <qwidgetstack.h>
#include <qprogressbar.h>
#include <qtimer.h>

#include <kdebug.h>
#include <kmainwindow.h>
#include <kapp.h>
#include <kurl.h>
#include <ktrader.h>
#include <klibloader.h>
#include <kmessagebox.h>
#include <krun.h>
#include <klistview.h>
#include <klocale.h>
#include <kcursor.h>
#include <kstatusbar.h>

#include "TjMessageHandler.h"
#include "MainWidget.h"
#include "Project.h"
#include "ProjectFile.h"
#include "Task.h"
#include "Resource.h"
#include "Account.h"
#include "TableColumnInfo.h"
#include "Report.h"
#include "QtTaskReport.h"
#include "QtTaskReportElement.h"
#include "FileManager.h"
#include "ManagedFileInfo.h"
#include "ReportManager.h"
#include "ManagedReportInfo.h"

TaskJugglerView::TaskJugglerView(QWidget *parent)
    : DCOPObject("TaskJugglerIface"), QWidget(parent)
{
    TJMH.setConsoleMode(FALSE);

    // setup our layout manager to automatically add our widgets
    QHBoxLayout *top_layout = new QHBoxLayout(this);
    top_layout->setAutoAdd(true);
    mw = new MainWidget(this, "Main Widget");
    QValueList<int> vl;
    vl.append(180);
    vl.append(400);
    mw->mainSplitter->setSizes(vl);

    project = 0;
    loadingProject = FALSE;

    loadDelayTimer = new QTimer(this);

    QVBoxLayout* l = new QVBoxLayout(mw->bigTab->page(0), 0, 6);
    editorSplitter = new QSplitter(mw->bigTab->page(0));
    editorSplitter->setOrientation(Vertical);

    QWidgetStack* editorStack = new QWidgetStack(editorSplitter);

    messageListView = new KListView(editorSplitter);
    messageListView->setSizePolicy(QSizePolicy::Maximum,
                                   QSizePolicy::Preferred);
    messageListView->setMinimumSize(400, 100);
    messageListView->setAllColumnsShowFocus(TRUE);
    messageListView->addColumn(i18n("Error Message"));
    messageListView->addColumn(i18n("File"));
    messageListView->addColumn(i18n("Line"));

    vl.clear();
    vl.append(int(100));
    vl.append(int(0));
    editorSplitter->setSizes(vl);
    l->addWidget(editorSplitter);

    fileManager = new FileManager(editorStack, mw->fileListView);
    reportManager = new ReportManager(mw->reportStack, mw->reportListView);

    connect(&TJMH, SIGNAL(printWarning(const QString&, const QString&, int)),
            this, SLOT(addWarningMessage(const QString&, const QString&, int)));
    connect(&TJMH, SIGNAL(printError(const QString&, const QString&, int)),
            this, SLOT(addErrorMessage(const QString&, const QString&, int)));

    connect(mw->taskListView, SIGNAL(clicked(QListViewItem*)),
            this, SLOT(taskListClicked(QListViewItem*)));
    connect(mw->taskListView, SIGNAL(returnPressed(QListViewItem*)),
            this, SLOT(taskListClicked(QListViewItem*)));

    connect(mw->listViews, SIGNAL(currentChanged(int)),
            this, SLOT(focusListViews(int)));
    connect(mw->bigTab, SIGNAL(currentChanged(QWidget*)),
            this, SLOT(focusBigTab(QWidget*)));

    connect(mw->resourceListView, SIGNAL(clicked(QListViewItem*)),
            this, SLOT(resourceListClicked(QListViewItem*)));
    connect(mw->resourceListView, SIGNAL(returnPressed(QListViewItem*)),
            this, SLOT(resourceListClicked(QListViewItem*)));

    connect(mw->accountListView, SIGNAL(clicked(QListViewItem*)),
            this, SLOT(accountListClicked(QListViewItem*)));
    connect(mw->accountListView, SIGNAL(returnPressed(QListViewItem*)),
            this, SLOT(accountListClicked(QListViewItem*)));

    connect(mw->reportListView, SIGNAL(clicked(QListViewItem*)),
            this, SLOT(reportListClicked(QListViewItem*)));
    connect(mw->reportListView, SIGNAL(returnPressed(QListViewItem*)),
            this, SLOT(reportListClicked(QListViewItem*)));

    connect(mw->fileListView, SIGNAL(clicked(QListViewItem*)),
            this, SLOT(fileListClicked(QListViewItem*)));
    connect(mw->fileListView, SIGNAL(returnPressed(QListViewItem*)),
            this, SLOT(fileListClicked(QListViewItem*)));

    connect(messageListView, SIGNAL(clicked(QListViewItem*)),
            this, SLOT(messageListClicked(QListViewItem*)));
    connect(messageListView, SIGNAL(returnPressed(QListViewItem*)),
            this, SLOT(messageListClicked(QListViewItem*)));

    connect(loadDelayTimer, SIGNAL(timeout()),
            this, SLOT(loadAfterTimerTimeout()));

    KStatusBar* statusBar = (static_cast<KMainWindow*>(parent))->statusBar();
    progressBar = new QProgressBar(statusBar);
    progressBar->setMaximumSize(150, progressBar->maximumHeight());
    statusBar->addWidget(progressBar, 0, TRUE);
}

TaskJugglerView::~TaskJugglerView()
{
}

void
TaskJugglerView::print(QPainter*, int, int)
{
    // do the actual printing, here
    // p->drawText(etc..)
}

QString
TaskJugglerView::currentURL()
{
    return fileManager->getMasterFile().url();
}

void
TaskJugglerView::newProject(const KURL& url)
{
    kdDebug() << "New Project: " << url << endl;
}

void
TaskJugglerView::newInclude(const KURL& url)
{
    kdDebug() << "New include file: " << url << endl;
}

void
TaskJugglerView::openURL(QString url)
{
    openURL(KURL(url));
}

void
TaskJugglerView::openURL(const KURL& url)
{
    /* When we switch to a new project, we clear all lists and reports.
     * Loading and processing may take some time, so it looks odd to still see
     * all the old data after the load had been initiated. */
    if (fileManager->getMasterFile() != url)
    {
        fileManager->clear();
        reportManager->clear();
    }

    /* This function can be triggered before the app->exec() has been called.
     * So the GUI won't show up before all files are loaded and scheduled.
     * With a timer we delay the load so we are sure app->exec has been
     * called. */
    loadDelayTimer->start(200, TRUE);
    urlToLoad = url;
}

void
TaskJugglerView::loadAfterTimerTimeout()
{
    // This function catches the signal triggered by the timer set in openURL.
    loadProject(urlToLoad);
}

void
TaskJugglerView::save()
{
    fileManager->saveCurrentFile();
}

void
TaskJugglerView::saveAs(const KURL& url)
{
    fileManager->saveCurrentFileAs(url);
}

void
TaskJugglerView::close()
{
    fileManager->closeCurrentFile();
    if (fileManager->getMasterFile().path().isEmpty())
    {
        mw->taskListView->clear();
        mw->resourceListView->clear();
        mw->accountListView->clear();
        mw->reportListView->clear();
        messageListView->clear();
    }
}

void
TaskJugglerView::schedule()
{
    if (fileManager->getMasterFile().path().isEmpty())
        return;

    fileManager->saveAllFiles();
    loadProject(fileManager->getMasterFile());
}

void
TaskJugglerView::nextProblem()
{
    QListViewItem* oldLvi = messageListView->currentItem();
    if (!oldLvi)
        return;
    QListViewItem* lvi = oldLvi->itemBelow();
    if (!lvi)
        return;
    messageListView->setSelected(lvi, TRUE);
    messageListClicked(lvi);
}

void
TaskJugglerView::previousProblem()
{
    QListViewItem* oldLvi = messageListView->currentItem();
    if (!oldLvi)
        return;
    QListViewItem* lvi = oldLvi->itemAbove();
    if (!lvi)
        return;
    messageListView->setSelected(lvi, TRUE);
    messageListClicked(lvi);
}

void
TaskJugglerView::slotOnURL(const QString& url)
{
    emit signalChangeStatusbar(url);
}

void
TaskJugglerView::slotSetTitle(const QString& title)
{
    emit signalChangeCaption(title);
}

bool
TaskJugglerView::loadProject(const KURL& url)
{
    QString fileName = url.path();

    delete project;
    setLoadingProject(TRUE);
    project = new Project();
    connect(project, SIGNAL(updateProgressInfo(const QString&)),
            this, SLOT(showProgressInfo(const QString&)));
    connect(project, SIGNAL(updateProgressBar(int, int)),
            progressBar, SLOT(setProgress(int, int)));

    ProjectFile* pf = new ProjectFile(project);
    setCursor(KCursor::workingCursor());
    if (!pf->open(fileName, "", "", TRUE))
    {
        setLoadingProject(FALSE);
        return FALSE;
    }

    emit announceRecentURL(url);

    bool errors = FALSE;
    messageListView->clear();
    if (!pf->parse())
        errors = TRUE;
    emit signalChangeStatusbar(i18n("Checking project..."));
    if (!errors && !project->pass2(FALSE))
        errors = TRUE;
    emit signalChangeStatusbar(i18n("Scheduling..."));
    if (!errors && !project->scheduleAllScenarios())
        errors = TRUE;

    setCursor(KCursor::arrowCursor());

    // Load tasks into Task List View
    KListView* tlv = mw->taskListView;
    tlv->clear();
    tlv->setColumnWidthMode(1, QListView::Manual);
    tlv->hideColumn(1);
    for (TaskListIterator tli(project->getTaskListIterator()); *tli; ++tli)
        if ((*tli)->getParent())
            new KListViewItem(tlv->findItem((*tli)->getParent()->getId(), 1),
                (*tli)->getName(),
                (*tli)->getId(),
                (*tli)->getDefinitionFile(),
                QString().sprintf("%d", (*tli)->getDefinitionLine()));
        else
            new KListViewItem(tlv, (*tli)->getName(),
             (*tli)->getId(),
             (*tli)->getDefinitionFile(),
             QString().sprintf("%d", (*tli)->getDefinitionLine()));

    // Load resources into Resource List View
    KListView* rlv = mw->resourceListView;
    rlv->clear();
    rlv->setColumnWidthMode(1, QListView::Manual);
    //rlv->hideColumn(1);
    for (ResourceListIterator rli(project->getResourceListIterator()); *rli;
         ++rli)
        if ((*rli)->getParent())
            new KListViewItem(rlv->findItem((*rli)->getParent()->getFullId(),
                                            1),
                              (*rli)->getName(),
                              (*rli)->getFullId(),
                              (*rli)->getDefinitionFile(),
                              QString().sprintf("%d",
                                                (*rli)->getDefinitionLine()));
        else
            new KListViewItem(rlv, (*rli)->getName(),
                              (*rli)->getFullId(),
                              (*rli)->getDefinitionFile(),
                              QString().sprintf("%d",
                                                (*rli)->getDefinitionLine()));

    // Load accounts into Account List View
    KListView* alv = mw->accountListView;
    alv->clear();
    alv->setColumnWidthMode(1, QListView::Manual);
    //alv->hideColumn(1);
    for (AccountListIterator ali(project->getAccountListIterator()); *ali;
         ++ali)
        if ((*ali)->getParent())
            new KListViewItem(alv->findItem((*ali)->getParent()->getFullId(),
                                            1),
                              (*ali)->getName(),
                              (*ali)->getFullId(),
                              (*ali)->getDefinitionFile(),
                              QString().sprintf("%d",
                                                (*ali)->getDefinitionLine()));
        else
            new KListViewItem(alv, (*ali)->getName(),
                              (*ali)->getFullId(),
                              (*ali)->getDefinitionFile(),
                              QString().sprintf("%d",
                                                (*ali)->getDefinitionLine()));

    // Load reports into Report List View
    reportManager->updateReportList(project->getReportListIterator());

    // Load file list into File List View
    fileManager->updateFileList(project->getSourceFiles(), url);

    setLoadingProject(FALSE);

    // We handle warnings like errors, so in case there any messages, we open
    // the message window.
    if (messageListView->lastItem())
        errors = TRUE;

    // Show message list when errors have occured
    QValueList<int> vl;
    int h = editorSplitter->height();
    if (errors)
    {
        vl.append(int(h * 0.85));
        vl.append(int(h * 0.15));
        editorSplitter->setSizes(vl);
        emit signalChangeStatusbar(i18n("The project contains problems!"));
        messageListClicked(messageListView->firstChild());
    }
    else
    {
        vl.append(int(h));
        vl.append(int(0));
        editorSplitter->setSizes(vl);
        emit signalChangeStatusbar(i18n("The project has been scheduled "
                                        "without problems!"));
        // Load the main file into the editor.
        fileManager->showInEditor(fileName);
        QListViewItem* firstReport =
            reportManager->getFirstInteractiveReportItem();
        if (firstReport)
        {
            // Open the report list.
            mw->listViews->setCurrentItem(mw->reportsPage);
            // Simulate a click on the first interactive report to open the
            // report view and show the first report.
            mw->reportListView->setSelected(firstReport, TRUE);
            reportListClicked(firstReport);
        }
    }

    return TRUE;
}

void
TaskJugglerView::addWarningMessage(const QString& msg, const QString& file,
                                   int line)
{
    if (!file.isEmpty() && line > 0)
        new KListViewItem(messageListView, msg, file,
                          QString().sprintf("%d", line));
    else
        new KListViewItem(messageListView, msg);
}

void
TaskJugglerView::addErrorMessage(const QString& msg, const QString& file,
                                 int line)
{
    QString shortMsg;
    if (msg.find("\n") >= 0)
        shortMsg = msg.left(msg.find("\n"));
    else
        shortMsg = msg;

    KListViewItem* lvi;
    if (!file.isEmpty() && line > 0)
        lvi = new KListViewItem(messageListView, shortMsg, file,
                                QString().sprintf("%d", line));
    else
        lvi = new KListViewItem(messageListView, shortMsg);

}

void
TaskJugglerView::setFocusToTaskList()
{
    mw->listViews->setCurrentIndex(0);
    focusListViews(0);
}

void
TaskJugglerView::setFocusToResourceList()
{
    mw->listViews->setCurrentIndex(1);
    focusListViews(1);
}

void
TaskJugglerView::setFocusToAccountList()
{
    mw->listViews->setCurrentIndex(2);
    focusListViews(2);
}

void
TaskJugglerView::setFocusToReportList()
{
    mw->listViews->setCurrentIndex(3);
    focusListViews(3);
}

void
TaskJugglerView::setFocusToFileList()
{
    mw->listViews->setCurrentIndex(4);
    focusListViews(4);
}

void
TaskJugglerView::setFocusToEditor()
{
    mw->bigTab->showPage(mw->editorTab);
    fileManager->setFocusToEditor();
}

void
TaskJugglerView::setFocusToReport()
{
    mw->bigTab->showPage(mw->reportTab);
    reportManager->setFocusToReport();
}

void
TaskJugglerView::zoomIn()
{
    if (loadingProject)
        return;

    reportManager->zoomIn();
}

void
TaskJugglerView::zoomOut()
{
    if (loadingProject)
        return;

    reportManager->zoomOut();
}

void
TaskJugglerView::focusListViews(int idx)
{
    switch (idx)
    {
        case 0:
            mw->taskListView->setFocus();
            break;
        case 1:
            mw->resourceListView->setFocus();
            break;
        case 2:
            mw->accountListView->setFocus();
            break;
        case 3:
            mw->reportListView->setFocus();
            break;
        case 4:
            mw->fileListView->setFocus();
            break;
    }
}

void
TaskJugglerView::focusBigTab(QWidget*)
{
    switch (mw->bigTab->currentPageIndex())
    {
        case 0:
            if (fileManager)
                fileManager->setFocusToEditor();
            break;
        case 1:
            if (reportManager)
                reportManager->setFocusToReport();
            break;
    }
}

void
TaskJugglerView::taskListClicked(QListViewItem* lvi)
{
    if (lvi)
    {
        fileManager->showInEditor(KURL(lvi->text(2)),
                                  lvi->text(3).toUInt() - 1, 0);
        mw->bigTab->showPage(mw->editorTab);
    }
}

void
TaskJugglerView::resourceListClicked(QListViewItem* lvi)
{
    if (lvi)
    {
        fileManager->showInEditor(KURL(lvi->text(2)),
                                  lvi->text(3).toUInt() - 1, 0);
        mw->bigTab->showPage(mw->editorTab);
    }
}

void
TaskJugglerView::accountListClicked(QListViewItem* lvi)
{
    if (lvi)
    {
        fileManager->showInEditor(KURL(lvi->text(2)),
                                  lvi->text(3).toUInt() - 1, 0);
        mw->bigTab->showPage(mw->editorTab);
    }
}

void
TaskJugglerView::reportListClicked(QListViewItem* lvi)
{
    if (!lvi)
        return;

    if (!reportManager->showReport(lvi))
    {
        // The report generation can also produce errors.
        mw->bigTab->showPage(mw->editorTab);
        int h = editorSplitter->height();
        QValueList<int> vl;
        vl.append(int(h * 0.85));
        vl.append(int(h * 0.15));
        editorSplitter->setSizes(vl);

        emit signalChangeStatusbar
            (i18n("The project contains problems!"));
        messageListClicked(messageListView->firstChild());
    }
    else
        mw->bigTab->showPage(mw->reportTab);
}

void
TaskJugglerView::fileListClicked(QListViewItem*)
{
    fileManager->showInEditor(fileManager->getCurrentFileURL());
}

void
TaskJugglerView::messageListClicked(QListViewItem* lvi)
{
    if (lvi && !lvi->text(1).isEmpty() && !lvi->text(2).isEmpty())
        fileManager->showInEditor(KURL(lvi->text(1)),
                                  lvi->text(2).toUInt() - 1, 0);
}

void
TaskJugglerView::showProgressInfo(const QString& i)
{
    emit signalChangeStatusbar(i);
    kapp->processEvents();
}

void
TaskJugglerView::keywordHelp()
{
    if (mw->bigTab->currentPageIndex() == 0 && fileManager->isProjectLoaded())
    {
        QString keyword = fileManager->getWordUnderCursor();
        kapp->invokeHelp(QString("PROPERTY_") + keyword);
    }
}

void
TaskJugglerView::setLoadingProject(bool lp)
{
    loadingProject = lp;
    if (reportManager)
        reportManager->setLoadingProject(lp);
}

#include "taskjugglerview.moc"
