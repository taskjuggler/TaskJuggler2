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

#include <kdebug.h>
#include <kapp.h>
#include <kurl.h>
#include <ktrader.h>
#include <klibloader.h>
#include <kmessagebox.h>
#include <krun.h>
#include <klistview.h>
#include <klocale.h>
#include <kcursor.h>

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

    connect(mw->leftReport, SIGNAL(expanded(QListViewItem*)),
            this, SLOT(expandReportItem(QListViewItem*)));
    connect(mw->leftReport, SIGNAL(collapsed(QListViewItem*)),
            this, SLOT(collapsReportItem(QListViewItem*)));
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
    return 0;
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
    fileManager->clear();
    if (loadProject(url))
        emit announceRecentURL(url);
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
    project = new Project();
    connect(project, SIGNAL(updateProgressInfo(const QString&)),
            this, SLOT(showProgressInfo(const QString&)));

    ProjectFile* pf = new ProjectFile(project);
    setCursor(KCursor::workingCursor());
    if (!pf->open(fileName, "", "", TRUE))
        return FALSE;

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

    // Show message list when errors have occured
    QValueList<int> vl;
    int h = editorSplitter->height();
    if (errors)
    {
        vl.append(int(h * 0.85));
        vl.append(int(h * 0.15));
        emit signalChangeStatusbar(i18n("The project contains problems!"));
        messageListClicked(messageListView->firstChild());
    }
    else
    {
        vl.append(int(h));
        vl.append(int(0));
        emit signalChangeStatusbar(i18n("The project has been scheduled "
                                        "without problems!"));
    }
    editorSplitter->setSizes(vl);

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
    KListView* plv = mw->reportListView;
    plv->clear();
    plv->setColumnWidthMode(1, QListView::Manual);
    plv->hideColumn(1);
    KListViewItem* qtReports =
        new KListViewItem(plv, i18n("Interactive Reports"));
    KListViewItem* htmlReports = new KListViewItem(plv, i18n("HTML Reports"));
    KListViewItem* csvReports = new KListViewItem(plv, i18n("CSV Reports"));
    KListViewItem* xmlReports = new KListViewItem(plv, i18n("XML Reports"));

    int i = 0;
    for (QPtrListIterator<Report> pli(project->getReportListIterator()); *pli;
         ++pli, ++i)
        if (strncmp((*pli)->getType(), "Qt", 2) == 0)
            new KListViewItem(qtReports, (*pli)->getFileName(),
                              QString().sprintf("%d", i),
                              (*pli)->getDefinitionFile(),
                              QString().sprintf("%d",
                                            (*pli)->getDefinitionLine()));
        else if (strncmp((*pli)->getType(), "HTML", 4) == 0)
            new KListViewItem(htmlReports, (*pli)->getFileName(),
                              QString().sprintf("%d", i),
                              (*pli)->getDefinitionFile(),
                              QString().sprintf("%d",
                                                (*pli)->getDefinitionLine()));
        else if (strncmp((*pli)->getType(), "CSV", 3) == 0)
            new KListViewItem(csvReports, (*pli)->getFileName(),
                              QString().sprintf("%d", i),
                              (*pli)->getDefinitionFile(),
                              QString().sprintf("%d",
                                                (*pli)->getDefinitionLine()));
        else if (strncmp((*pli)->getType(), "XML", 3) == 0)
            new KListViewItem(xmlReports, (*pli)->getFileName(),
                              QString().sprintf("%d", i),
                              (*pli)->getDefinitionFile(),
                              QString().sprintf("%d",
                                                (*pli)->getDefinitionLine()));
        else
            new KListViewItem(plv, (*pli)->getFileName(),
                              QString().sprintf("%d", i),
                              (*pli)->getDefinitionFile(),
                              QString().sprintf("%d",
                                                (*pli)->getDefinitionLine()));
    qtReports->setOpen(TRUE);

    // Load file list into File List View
    fileManager->updateFileList(project->getSourceFiles(), url);
    fileManager->showInEditor(fileName);

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
    mw->bigTab->showPage(mw->tab);
    fileManager->setFocusToEditor();
}

void
TaskJugglerView::setFocusToReport()
{
    mw->bigTab->showPage(mw->tab_2);
    mw->leftReport->setFocus();
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
            fileManager->setFocusToEditor();
            break;
        case 1:
            mw->leftReport->setFocus();
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
        mw->bigTab->showPage(mw->tab);
    }
}

void
TaskJugglerView::resourceListClicked(QListViewItem* lvi)
{
    if (lvi)
    {
        fileManager->showInEditor(KURL(lvi->text(2)),
                                  lvi->text(3).toUInt() - 1, 0);
        mw->bigTab->showPage(mw->tab);
    }
}

void
TaskJugglerView::accountListClicked(QListViewItem* lvi)
{
    if (lvi)
    {
        fileManager->showInEditor(KURL(lvi->text(2)),
                                  lvi->text(3).toUInt() - 1, 0);
        mw->bigTab->showPage(mw->tab);
    }
}

void
TaskJugglerView::reportListClicked(QListViewItem* lvi)
{
    if (!lvi)
        return;

    mw->bigTab->showPage(mw->tab_2);
    mw->leftReport->setFocus();

    mw->leftReport->clear();
    while (mw->leftReport->columns())
        mw->leftReport->removeColumn(0);
    mw->rightReport->clear();
    while (mw->rightReport->columns())
        mw->rightReport->removeColumn(0);

    Report* report = project->getReport(lvi->text(1).toUInt());
    if (!report)
        return;

    if (strcmp(report->getType(), "QtTaskReport") == 0)
    {
        QFont f = mw->rightReport->header()->font();
        f.setPixelSize(10);
        mw->rightReport->header()->setFont(f);
        mw->leftReport->addColumn(i18n("Task"));
        mw->leftReport->addColumn(i18n("Id"));
        mw->leftReport->setColumnWidthMode(1, QListView::Manual);
        mw->rightReport->addColumn(i18n("Task"));
        mw->rightReport->setColumnWidthMode(0, QListView::Manual);
        mw->rightReport->addColumn(i18n("Id"));
        mw->rightReport->addColumn("Line1\nLine2");
        mw->rightReport->setColumnWidthMode(1, QListView::Manual);
        mw->leftReport->hideColumn(1);
        mw->rightReport->hideColumn(0);
        mw->rightReport->hideColumn(1);
        mw->leftReport->header()->setFixedHeight
            (mw->rightReport->header()->height());
        mw->leftReport->move(mw->rightReport->pos());

        QtTaskReportElement* tab =
            (dynamic_cast<QtTaskReport*>(report))->getTable();
        for (QPtrListIterator<TableColumnInfo>
             ci = tab->getColumnsIterator(); *ci; ++ci)
        {
            mw->leftReport->addColumn((*ci)->getName());
        }
        for (TaskListIterator tli(project->getTaskListIterator()); *tli;
             ++tli)
        {
            KListViewItem* lLine;
            KListViewItem* rLine;
            if ((*tli)->getParent())
            {
                lLine = new KListViewItem
                    (mw->leftReport->findItem((*tli)->getParent()->getId(),
                                              1), (*tli)->getName(),
                     (*tli)->getId());
                rLine = new KListViewItem
                    (mw->rightReport->findItem((*tli)->getParent()->getId(),
                                               1), (*tli)->getName(),
                     (*tli)->getId());
            }
            else
            {
                lLine = new KListViewItem(mw->leftReport, (*tli)->getName(),
                                          (*tli)->getId());
                rLine = new KListViewItem(mw->rightReport,
                                          (*tli)->getName(),
                                          (*tli)->getId());
            }

            int column = 2;
            for (QPtrListIterator<TableColumnInfo>
                 ci = tab->getColumnsIterator(); *ci; ++ci, ++column)
            {
                if ((*ci)->getName() == "start")
                    lLine->setText(column,
                                   time2user((*tli)->getStart(0),
                                             report->getTimeFormat()));
                else if ((*ci)->getName() == "end")
                    lLine->setText(column,
                                   time2user(((*tli)->isMilestone() ? 1 :
                                              0) + (*tli)->getEnd(0),
                                             report->getTimeFormat()));
            }
        }
    }
/*    QValueList<int> vl;
    vl.append(int(mw->leftReport->columnWidth(0)));
    vl.append(int(mw->reportSplitter->width() - vl[0]));
    mw->reportSplitter->setSizes(vl);*/
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
TaskJugglerView::collapsReportItem(QListViewItem* lvi)
{
    mw->rightReport->setOpen(mw->rightReport->findItem(lvi->text(1), 1), FALSE);
}

void
TaskJugglerView::expandReportItem(QListViewItem* lvi)
{
    mw->rightReport->setOpen(mw->rightReport->findItem(lvi->text(1), 1), TRUE);
}

#include "taskjugglerview.moc"
