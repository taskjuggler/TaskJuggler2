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
#include <qmessagebox.h>

#include <kdebug.h>
#include <kmainwindow.h>
#include <kapp.h>
#include <kurl.h>
#include <ktrader.h>
#include <klibloader.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <kstandarddirs.h>
#include <krun.h>
#include <klistview.h>
#include <klocale.h>
#include <kcursor.h>
#include <kstatusbar.h>
#include <kconfig.h>
#include <kiconloader.h>

#include "TjMessageHandler.h"
#include "taskjuggler.h"
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
    // Should be high enough for at least 4 lines.
    messageListView->setMinimumSize(400, 120);
    messageListView->setRootIsDecorated(TRUE);
    messageListView->setAllColumnsShowFocus(TRUE);
    messageListView->setSortOrder(Qt::Ascending);
    messageListView->setSortColumn(4);
    messageListView->addColumn("");
    messageListView->addColumn(i18n("Error Message"));
    messageListView->addColumn(i18n("File"));
    messageListView->addColumn(i18n("Line"));
    messageListView->addColumn(i18n("Counter"));
    messageListView->setColumnWidthMode(4, QListView::Manual);
    messageListView->hideColumn(4);

    vl.clear();
    vl.append(int(100));
    vl.append(int(0));
    editorSplitter->setSizes(vl);
    l->addWidget(editorSplitter);

    fileManager = new FileManager(dynamic_cast<KMainWindow*>(parent),
                                  editorStack, mw->fileListView);
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

    connect(reportManager, SIGNAL(signalChangeStatusBar(const QString&)),
            this, SLOT(changeStatusBar(const QString&)));
    connect(reportManager, SIGNAL(signalEditCoreAttributes(CoreAttributes*)),
            this, SLOT(showInEditor(CoreAttributes*)));

    connect(loadDelayTimer, SIGNAL(timeout()),
            this, SLOT(loadAfterTimerTimeout()));

    KStatusBar* statusBar = (static_cast<KMainWindow*>(parent))->statusBar();
    progressBar = new QProgressBar(statusBar);
    progressBar->setMaximumSize(150, progressBar->maximumHeight());
    statusBar->addWidget(progressBar, 0, TRUE);
}

TaskJugglerView::~TaskJugglerView()
{
    delete fileManager;
    delete reportManager;
}

void
TaskJugglerView::print(QPainter*, int, int)
{
}

void
TaskJugglerView::readProperties(KConfig* config)
{
    QValueList<int> sizes = config->readIntListEntry("MainSplitter");
    mw->mainSplitter->setSizes(sizes);
    editorSplitterSizes = config->readIntListEntry("EditorSplitter");

    fileManager->readProperties(config);
}

void
TaskJugglerView::saveProperties(KConfig* config)
{
    fileManager->writeProperties(config);

    config->setGroup("Global Settings");
    /* Save the URL of the current project so we can restore it in case
     * TaskJuggler is restarted without a new URL specified. */
    if (!fileManager->getMasterFile().url().isEmpty())
        config->writePathEntry("lastURL", fileManager->getMasterFile().url());

    // Save the position of the main splitter.
    config->writeEntry("MainSplitter", mw->mainSplitter->sizes());

    // Save the position of the editor splitter.
    config->writeEntry("EditorSplitter", editorSplitterSizes);
}

void
TaskJugglerView::newProject()
{
    if (!fileManager->getMasterFile().isEmpty())
    {
        if (QMessageBox::question
            (this, "TaskJuggler",
             i18n("You must close the current project before you can \n"
                  "create a new project. Do you really want to do this?"),
             QMessageBox::Yes | QMessageBox::Escape,
             QMessageBox::No | QMessageBox::Default,
             QMessageBox::NoButton) == QMessageBox::No)
            return;
    }
    KURL fileURL = KFileDialog::getSaveURL
        (i18n("TaskJuggler - Pick a name for the new project file"), "*.tjp",
         this, "New Project File");
    if (fileURL.isEmpty() || !fileURL.isValid())
    {
        QMessageBox::critical(this, i18n("Error while creating new Project"),
                              i18n("The specified file URL is not valid!"),
                              QMessageBox::Ok | QMessageBox::Default,
                              QMessageBox::NoButton);
        return;
    }

    QString templateProject = KGlobal::dirs()->findResource
        ("data", "taskjuggler/ProjectTemplate.tjp");
    if (templateProject.isEmpty())
    {
        QMessageBox::critical
            (this, i18n("TaskJuggler - Error while creating new Project"),
             i18n("Could not find ProjectTemplate.tjp!"),
             QMessageBox::Ok | QMessageBox::Default,
             QMessageBox::NoButton);
        return;
    }

    // Make sure we don't lose any changes.
    fileManager->saveAllFiles();

    /* When we switch to a new project, we clear all lists and reports.
     * Loading and processing may take some time, so it looks odd to still see
     * all the old data after the load had been initiated.  So cleanup all
     * project specific data structures. */
    closeProject();

    showReportAfterLoad = FALSE;
    loadProject(KURL(templateProject));
    saveAs(fileURL);
}

void
TaskJugglerView::newInclude()
{
    // Make sure that we have already a project we can add the file to.
    if (fileManager->getMasterFile().isEmpty())
    {
        QMessageBox::warning
            (this, i18n("TaskJuggler - New Include File"),
             i18n("You need to load or create a project before \n"
                  "you can create a new include file."),
             QMessageBox::Yes | QMessageBox::Default, QMessageBox::NoButton);
        return;
    }

    // Ask user for the name of the new file.
    KURL fileURL = KFileDialog::getSaveURL
        (i18n("Pick a name for the new include file"), "*.tji",
         this, "New Include File");

    // Find out the name of the include file template.
    QString templateInclude = KGlobal::dirs()->findResource
        ("data", "taskjuggler/IncludeTemplate.tji");
    if (templateInclude.isEmpty())
    {
        QMessageBox::critical
            (this, i18n("TaskJuggler - Error while creating include File"),
             i18n("Could not find IncludeTemplate.tji!"),
             QMessageBox::Ok | QMessageBox::Default,
             QMessageBox::NoButton);
        return;
    }

    if (!fileURL.isEmpty() && fileURL.isValid())
    {
        // Add template to list of files.
        fileManager->addFile(KURL(templateInclude), fileURL);
        // Open the file list.
        mw->listViews->setCurrentItem(mw->filesPage);
        // Show the new file in the editor.
        showEditor();
    }
}

void
TaskJugglerView::openURL(QString url)
{
    openURL(KURL(url));
}

void
TaskJugglerView::openURL(KURL url)
{
    /* If the URL matches the currently loaded project do nothing. */
    if (!fileManager->getMasterFile().isEmpty() &&
        fileManager->getMasterFile() == url)
        return;

    if (!fileManager->getMasterFile().isEmpty())
    {
        // Make sure the user really wants to load a new project.
        int but = QMessageBox::question
            (this, i18n("Load a new Project"),
             i18n("You must close the current project before you can load\n"
                  "a new project. All files of the current project will \n"
                  "be saved automatically. Do you really want to do this?"),
             QMessageBox::Yes, QMessageBox::No | QMessageBox::Default);

        if (but == QMessageBox::No)
            return;
    }

    // If the URL hasn't been specified yet, ask the user for it.
    if (url.isEmpty())
    {
        url = KFileDialog::getOpenURL(QString::null, QString::null, this,
                                      i18n("Open a new Project"));
        if (url.isEmpty())
            return;
    }

    // Make sure we don't lose any changes.
    fileManager->saveAllFiles();

    /* When we switch to a new project, we clear all lists and reports.
     * Loading and processing may take some time, so it looks odd to still see
     * all the old data after the load had been initiated.  So cleanup all
     * project specific data structures. */
    closeProject();

    /* This function can be triggered before the app->exec() has been called.
     * So the GUI won't show up before all files are loaded and scheduled.
     * With a timer we delay the load so we are sure app->exec has been
     * called. */
    loadDelayTimer->start(200, TRUE);
    urlToLoad = url;
    showReportAfterLoad = TRUE;
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
TaskJugglerView::saveAs()
{
    KURL url = KFileDialog::getSaveURL
        (i18n("Pick an alternative name for the current file"),
         "*.tjp, *.tji", this, i18n("Save file as"));

    if (!url.isEmpty() && url.isValid())
        saveAs(url);
}

void
TaskJugglerView::saveAs(const KURL& url)
{
    fileManager->saveCurrentFileAs(url);
    // Update window caption.
    showEditor();
}

void
TaskJugglerView::close()
{
    fileManager->closeCurrentFile();
    if (fileManager->getMasterFile().path().isEmpty())
        closeProject();
}

void
TaskJugglerView::closeProject()
{
    // Clear all project specific data structures.
    fileManager->clear();
    reportManager->clear();
    mw->taskListView->clear();
    mw->resourceListView->clear();
    mw->accountListView->clear();
    mw->reportListView->clear();
    mw->fileListView->clear();
    messageListView->clear();
    messageCounter = 0;
    slotSetTitle( i18n( "No Project" ) );
    changeStatusBar( QString::null ); // clear the status bar
}

void
TaskJugglerView::undo()
{
    fileManager->undo();
}

void
TaskJugglerView::redo()
{
    fileManager->redo();
}

void
TaskJugglerView::cut()
{
    fileManager->cut();
}

void
TaskJugglerView::copy()
{
    fileManager->copy();
}

void
TaskJugglerView::paste()
{
    fileManager->paste();
}

void
TaskJugglerView::selectAll()
{
    fileManager->selectAll();
}

void
TaskJugglerView::find()
{
    fileManager->find();
}

void
TaskJugglerView::findNext()
{
    fileManager->findNext();
}

void
TaskJugglerView::findPrevious()
{
    fileManager->findPrevious();
}

void
TaskJugglerView::schedule()
{
    if (fileManager->getMasterFile().path().isEmpty())
        return;

    fileManager->saveAllFiles();

    /* If the message list is visible we store the settings of the editor
     * splitter into a value list. This is reused for later errors and also
     * stored as property in the config file. */
    QValueList<int> vl = editorSplitter->sizes();
    if (vl[1] > 0)
        editorSplitterSizes = vl;

    showReportAfterLoad = TRUE;
    loadProject(fileManager->getMasterFile());
}

void
TaskJugglerView::nextProblem()
{
    QListViewItem* lvi = messageListView->currentItem();
    if (!lvi)
        return;
    // Not all items have a file name, skip those.
    do
    {
        lvi = lvi->itemBelow();
    } while (lvi && lvi->text(2).isEmpty());
    if (!lvi)
        return;
    messageListView->setSelected(lvi, TRUE);
    messageListClicked(lvi);

    // Messages can consist of multiple lines, so we try to make sure that at
    // least the next 2 lines are visible as well.
    messageListView->ensureItemVisible(lvi);
    for (int i = 0; i < 3; ++i)
        if ((lvi = lvi->itemBelow()) != 0)
            messageListView->ensureItemVisible(lvi);
        else
            break;
}

void
TaskJugglerView::previousProblem()
{
    QListViewItem* lvi = messageListView->currentItem();
    if (!lvi)
        return;
    // Not all items have a file name, skip those.
    do
    {
        lvi = lvi->itemAbove();
    } while (lvi && lvi->text(2).isEmpty());
    if (!lvi)
        return;
    messageListView->setSelected(lvi, TRUE);
    messageListClicked(lvi);

    // Messages can consist of multiple lines, so we try to make sure that at
    // least the next 2 lines are visible as well.
    messageListView->ensureItemVisible(lvi);
    for (int i = 0; i < 3; ++i)
        if ((lvi = lvi->itemBelow()) != 0)
            messageListView->ensureItemVisible(lvi);
        else
            break;
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
    if (loadingProject)
        return TRUE;

    QString fileName = url.path();

    delete project;
    setLoadingProject(TRUE);
    project = new Project();
    connect(project, SIGNAL(updateProgressInfo(const QString&)),
            this, SLOT(showProgressInfo(const QString&)));
    connect(project, SIGNAL(updateProgressBar(int, int)),
            progressBar, SLOT(setProgress(int, int)));

    ProjectFile* pf = new ProjectFile(project);
    setCursor(KCursor::waitCursor());
    if (!pf->open(fileName, "", "", TRUE))
    {
        QMessageBox::critical(this, i18n("Error loading Project"),
                              i18n("Cannot open file %1!")
                              .arg(url.prettyURL()),
                              QMessageBox::Ok | QMessageBox::Default,
                              QMessageBox::NoButton);
        setCursor(KCursor::arrowCursor());
        setLoadingProject(FALSE);
        return FALSE;
    }

    emit announceRecentURL(url);

    bool errors = FALSE;
    messageListView->clear();
    messageCounter = 0;
    if (!pf->parse())
        errors = TRUE;
    changeStatusBar(i18n("Checking project..."));
    if (!errors && !project->pass2(FALSE))
        errors = TRUE;
    changeStatusBar(i18n("Scheduling..."));
    if (!errors && !project->scheduleAllScenarios())
        errors = TRUE;

    // Load tasks into Task List View
    updateTaskList();

    // Load resources into Resource List View
    updateResourceList();

    // Load accounts into Account List View
    updateAccountList();

    // Load reports into Report List View
    reportManager->updateReportList(project->getReportListIterator());

    // Load file list into File List View
    fileManager->updateFileList(project->getSourceFiles(), url);

    setCursor(KCursor::arrowCursor());
    setLoadingProject(FALSE);

    // We handle warnings like errors, so in case there any messages, we open
    // the message window.
    if (messageCounter)
        errors = TRUE;

    // Show message list when errors have occured
    QValueList<int> vl;
    int h = editorSplitter->height();
    if (errors)
    {
        // The messages should be visible, so we check whether we already have
        // a setting for the splitter that is large enough. Otherwise we make
        // the message list 15% of the splitter size.
        if (editorSplitterSizes.isEmpty() || editorSplitterSizes[1] < 120)
        {
            vl.append(int(h * 0.85));
            vl.append(int(h * 0.15));
        }
        else
            vl = editorSplitterSizes;
        editorSplitter->setSizes(vl);
        changeStatusBar(i18n("The project contains problems!"));
        showEditor();
        messageListView->setSelected(messageListView->firstChild(), TRUE);
        messageListClicked(messageListView->firstChild());
    }
    else
    {
        vl.append(int(h));
        vl.append(int(0));
        editorSplitter->setSizes(vl);
        changeStatusBar(i18n("The project has been scheduled "
                             "without problems!"));
        // Load the main file into the editor.
        fileManager->showInEditor(fileName);
        if (showReportAfterLoad)
        {
            QListViewItem* firstReport =
                reportManager->getFirstInteractiveReportItem();
            if (firstReport)
            {
                // Open the report list.
                mw->listViews->setCurrentItem(mw->reportsPage);
                // Simulate a click on the first interactive report to open the
                // report view and show the first report.
                mw->reportListView->setSelected(firstReport, TRUE);
                showReport();
                reportListClicked(firstReport);
            }
            else
                showEditor();
        }
        else
            showEditor();
    }

    return TRUE;
}

void
TaskJugglerView::addWarningMessage(const QString& msg, const QString& file,
                                   int line)
{
    addMessage(msg, file, line, FALSE);
}

void
TaskJugglerView::addErrorMessage(const QString& msg, const QString& file,
                                 int line)
{
    addMessage(msg, file, line, TRUE);
}

void
TaskJugglerView::addMessage(const QString& msg, const QString& file,
                            int line, bool error)
{
    /* The message list view has 5 columns:
     * Column 0: Decoration
     * Column 1: Message
     * Column 2: File name (empty for additional lines)
     * Column 3: Line number (emty for additional lines)
     * Column 4: Counter (Hidden)
     */
    ++messageCounter;
    QString text = msg;
    QListViewItem* parent = 0;
    do
    {
        QString textLine;
        if (text.find("\n") >= 0)
        {
            textLine = text.left(text.find("\n"));
            text = text.right(text.length() - text.find("\n") - 1);
        }
        else
        {
            textLine = text;
            text = "";
        }

        if (!parent)
        {
            if (!file.isEmpty() && line > 0)
                parent= new KListViewItem
                    (messageListView, "", textLine, file,
                     QString().sprintf("%d", line),
                     QString().sprintf("%03d", messageCounter));
            else
                parent = new KListViewItem
                    (messageListView, "", textLine, "", "",
                     QString().sprintf("%03d", messageCounter));

            parent->setOpen(TRUE);
            parent->setPixmap(0, KGlobal::iconLoader()->
                              loadIcon(error ? "tj_error" : "tj_warning",
                                       KIcon::Small));
        }
        else
            new KListViewItem(parent, "", textLine, "", "",
                              QString().sprintf("%03d", messageCounter));
    } while (!text.isEmpty());
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
TaskJugglerView::changeStatusBar(const QString& text)
{
    emit signalChangeStatusbar(text);
}

void
TaskJugglerView::configureEditor()
{
    fileManager->configureEditor();
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
    QString windowCaption;
    if (project)
        windowCaption = project->getName() + " - ";

    switch (mw->bigTab->currentPageIndex())
    {
        case 0:
            // The editor has become visible
            if (fileManager->getCurrentFile())
                windowCaption +=
                    fileManager->getCurrentFile()->getUniqueName();
            fileManager->enableEditorActions(TRUE);
            fileManager->setFocusToEditor();

            break;

        case 1:
            // The report page has become visible
            if (reportManager->getCurrentReport())
                windowCaption +=
                    reportManager->getCurrentReport()->getName();

            // Open report list in toolbox and select current report.
            mw->listViews->setCurrentIndex(3);
            reportManager->setFocusToReport();

            // Disable all editor actions.
            fileManager->enableEditorActions(FALSE);

            break;
    }
    (dynamic_cast<TaskJuggler*>(parent()))->changeCaption(windowCaption);
}

void
TaskJugglerView::showEditor()
{
    QString windowCaption;
    if (project)
        windowCaption = project->getName() + " - ";

    if (fileManager && fileManager->getCurrentFile())
        windowCaption += fileManager->getCurrentFile()->getUniqueName();

    (dynamic_cast<TaskJuggler*>(parent()))->changeCaption(windowCaption);

    mw->bigTab->showPage(mw->editorTab);
}

void
TaskJugglerView::showReport()
{
    // Set the window caption to "<project name> - <report name>"
    QString windowCaption;
    if (project)
        windowCaption = project->getName() + " - ";

    if (reportManager && reportManager->getCurrentReport())
        windowCaption += reportManager->getCurrentReport()->getName();

    (dynamic_cast<TaskJuggler*>(parent()))->changeCaption(windowCaption);

    // Bring report page ontop.
    mw->bigTab->showPage(mw->reportTab);
}

void
TaskJugglerView::showInEditor(CoreAttributes* ca)
{
    fileManager->showInEditor(ca);
    showEditor();
}

void
TaskJugglerView::taskListClicked(QListViewItem* lvi)
{
    if (lvi)
    {
        fileManager->showInEditor(KURL(lvi->text(2)),
                                  lvi->text(3).toUInt() - 1, 0);
        showEditor();
    }
}

void
TaskJugglerView::resourceListClicked(QListViewItem* lvi)
{
    if (lvi)
    {
        fileManager->showInEditor(KURL(lvi->text(2)),
                                  lvi->text(3).toUInt() - 1, 0);
        showEditor();
    }
}

void
TaskJugglerView::accountListClicked(QListViewItem* lvi)
{
    if (lvi)
    {
        fileManager->showInEditor(KURL(lvi->text(2)),
                                  lvi->text(3).toUInt() - 1, 0);
        showEditor();
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

        changeStatusBar(i18n("The project contains problems!"));
        messageListClicked(messageListView->firstChild());
    }
    else
        showReport();
}

void
TaskJugglerView::fileListClicked(QListViewItem* lvi)
{
    if (!lvi)
        return;

    fileManager->showInEditor(fileManager->getCurrentFileURL());
    showEditor();
}

void
TaskJugglerView::messageListClicked(QListViewItem* lvi)
{
    if (lvi && !lvi->text(2).isEmpty() && !lvi->text(3).isEmpty())
        fileManager->showInEditor(KURL(lvi->text(2)),
                                  lvi->text(3).toUInt() - 1, 0);
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

        // In case there is no word under the cursor, show the help for the
        // global scope. This is a good entry point to find other keywords.
        if (keyword.isEmpty())
            keyword = "global_scope";

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

void
TaskJugglerView::updateTaskList()
{
    // Load tasks into Task List View
    KListView* tlv = mw->taskListView;
    tlv->clear();
    tlv->setColumnWidthMode(1, QListView::Manual);
    tlv->hideColumn(1);
    for (TaskListIterator tli(project->getTaskListIterator()); *tli; ++tli)
    {
        QListViewItem* lvi;
        if ((*tli)->getParent())
            lvi = new
                KListViewItem(tlv->findItem((*tli)->getParent()->getId(), 1),
                              (*tli)->getName(),
                              (*tli)->getId(),
                              (*tli)->getDefinitionFile(),
                              QString().sprintf
                              ("%d", (*tli)->getDefinitionLine()));
        else
            lvi = new KListViewItem(tlv, (*tli)->getName(),
                                    (*tli)->getId(),
                                    (*tli)->getDefinitionFile(),
                                    QString().sprintf
                                    ("%d", (*tli)->getDefinitionLine()));
        if ((*tli)->isContainer())
        {
            lvi->setPixmap(0, KGlobal::iconLoader()->
                           loadIcon("tj_task_group", KIcon::Small));
            lvi->setOpen((*tli)->treeLevel() < 2);
        }
        else if ((*tli)->isMilestone())
            lvi->setPixmap(0, KGlobal::iconLoader()->
                           loadIcon("tj_milestone", KIcon::Small));
        else
            lvi->setPixmap(0, KGlobal::iconLoader()->
                           loadIcon("tj_task", KIcon::Small));
    }

}

void
TaskJugglerView::updateResourceList()
{
    // Load resources into Resource List View
    KListView* rlv = mw->resourceListView;
    rlv->clear();
    rlv->setColumnWidthMode(1, QListView::Manual);
    //rlv->hideColumn(1);
    for (ResourceListIterator rli(project->getResourceListIterator()); *rli;
         ++rli)
    {
        QListViewItem* lvi;
        if ((*rli)->getParent())
            lvi = new KListViewItem
                (rlv->findItem((*rli)->getParent()->getFullId(), 1),
                 (*rli)->getName(), (*rli)->getFullId(),
                 (*rli)->getDefinitionFile(),
                 QString().sprintf("%d", (*rli)->getDefinitionLine()));
        else
            lvi = new KListViewItem(rlv, (*rli)->getName(),
                                    (*rli)->getFullId(),
                                    (*rli)->getDefinitionFile(),
                                    QString().sprintf
                                    ("%d", (*rli)->getDefinitionLine()));
        if ((*rli)->hasSubs())
        {
            lvi->setPixmap(0, KGlobal::iconLoader()->
                           loadIcon("tj_resource_group", KIcon::Small));
            lvi->setOpen((*rli)->treeLevel() < 2);
        }
        else
            lvi->setPixmap(0, KGlobal::iconLoader()->
                           loadIcon("tj_resource", KIcon::Small));
    }

}

void
TaskJugglerView::updateAccountList()
{
    // Load accounts into Account List View
    KListView* alv = mw->accountListView;
    alv->clear();
    alv->setColumnWidthMode(1, QListView::Manual);
    //alv->hideColumn(1);
    for (AccountListIterator ali(project->getAccountListIterator()); *ali;
         ++ali)
    {
        QListViewItem* lvi;
        if ((*ali)->getParent())
            lvi = new KListViewItem
                (alv->findItem((*ali)->getParent()->getFullId(), 1),
                 (*ali)->getName(), (*ali)->getFullId(),
                 (*ali)->getDefinitionFile(),
                 QString().sprintf("%d", (*ali)->getDefinitionLine()));
        else
            lvi = new KListViewItem
                (alv, (*ali)->getName(), (*ali)->getFullId(),
                 (*ali)->getDefinitionFile(),
                 QString().sprintf("%d", (*ali)->getDefinitionLine()));
        if ((*ali)->hasSubs())
        {
            lvi->setPixmap(0, KGlobal::iconLoader()->
                           loadIcon("tj_account_group", KIcon::Small));
            lvi->setOpen((*ali)->treeLevel() < 2);
        }
        else
            lvi->setPixmap(0, KGlobal::iconLoader()->
                           loadIcon("tj_account", KIcon::Small));
    }

}

#include "taskjugglerview.moc"
