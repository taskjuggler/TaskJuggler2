/*
 * The TaskJuggler Project Management Software
 *
 * Copyright (c) 2001, 2002, 2003, 2004, 2005, 2006
 * by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#include "taskjugglerview.h"

#include <map>

#include <qpainter.h>
#include <qlayout.h>
#include <qlayout.h>
#include <qsplitter.h>
#include <qtabwidget.h>
#include <qtoolbox.h>
#include <qstringlist.h>
#include <qregexp.h>
#include <qtooltip.h>
#include <qheader.h>
#include <qfont.h>
#include <qwidgetstack.h>
#include <qprogressbar.h>
#include <qtimer.h>
#include <qlistview.h>
#include <qpushbutton.h>

#include <kdebug.h>
#include <kapp.h>
#include <kaction.h>
#include <ktrader.h>
#include <klibloader.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <kfileitem.h>
#include <kstandarddirs.h>
#include <krun.h>
#include <klistview.h>
#include <klistviewsearchline.h>
#include <klocale.h>
#include <kcursor.h>
#include <kstatusbar.h>
#include <kconfig.h>
#include <kiconloader.h>
#include <kstdguiitem.h>
#include <klistviewsearchline.h>

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
#include "FileManager.h"
#include "ReportManager.h"
#include "TemplateSelector.h"
#include "taskjuggler.h"

TaskJugglerView::TaskJugglerView(QWidget *parent)
    : DCOPObject("TaskJugglerIface"), QWidget(parent)
{
    TJMH.setConsoleMode(false);

    // setup our layout manager to automatically add our widgets
    QHBoxLayout *top_layout = new QHBoxLayout(this);
    top_layout->setAutoAdd(true);
    mw = new MainWidget(this, "Main Widget");
    QValueList<int> vl;
    vl.append(180);
    vl.append(400);
    mw->mainSplitter->setSizes(vl);

    mw->taskListViewSearch->setListView(mw->taskListView);
    mw->resourceListViewSearch->setListView(mw->resourceListView);
    mw->accountListViewSearch->setListView(mw->accountListView);
    mw->reportListViewSearch->setListView(mw->reportListView);
    mw->fileListViewSearch->setListView(mw->fileListView);

    project = 0;
    loadingProject = false;

    loadDelayTimer = new QTimer(this);

    mw->messageListView->setColumnText(0, "");
    mw->messageListView->setSortOrder(Qt::Ascending);
    mw->messageListView->setSortColumn(4);
    mw->messageListView->setColumnWidthMode(4, QListView::Manual);
    mw->messageListView->hideColumn(4);

    // Make sure the error list is completely hidden.
    QValueList<int> sizes;
    sizes.append(100);
    sizes.append(0);
    mw->editorSplitter->setSizes(sizes);

    fileManager = new FileManager(dynamic_cast<KMainWindow*>(parent),
                                  mw->editorStack, mw->fileListView,
                                  mw->fileListViewSearch);
    reportManager = new ReportManager(dynamic_cast<KMainWindow*>(parent),
                                      mw->reportStack, mw->reportListView,
                                      mw->reportListViewSearch);
    lastBrowserUsedWithEditor = 0;

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

    connect(mw->reportListView, SIGNAL(returnPressed(QListViewItem*)),
            this, SLOT(reportListClicked(QListViewItem*)));
    connect(mw->reportListView,
            SIGNAL(mouseButtonClicked(int, QListViewItem*, const QPoint&, int)),
            this,
            SLOT(reportListClicked(int, QListViewItem*, const QPoint&, int)));

    connect(mw->fileListView, SIGNAL(clicked(QListViewItem*)),
            this, SLOT(fileListClicked(QListViewItem*)));
    connect(mw->fileListView, SIGNAL(returnPressed(QListViewItem*)),
            this, SLOT(fileListClicked(QListViewItem*)));

    connect(mw->messageListView, SIGNAL(clicked(QListViewItem*)),
            this, SLOT(messageListClicked(QListViewItem*)));
    connect(mw->messageListView, SIGNAL(returnPressed(QListViewItem*)),
            this, SLOT(messageListClicked(QListViewItem*)));

    connect(reportManager, SIGNAL(signalChangeStatusBar(const QString&)),
            this, SLOT(changeStatusBar(const QString&)));
    connect(reportManager, SIGNAL(signalEditCoreAttributes(CoreAttributes*)),
            this, SLOT(showInEditor(CoreAttributes*)));
    connect(reportManager, SIGNAL(signalEditReport(const Report*)),
            this, SLOT(showInEditor(const Report*)));
    connect(reportManager, SIGNAL(signalEditFile(const KURL&)),
            this, SLOT(showInEditor(const KURL&)));

    connect(loadDelayTimer, SIGNAL(timeout()),
            this, SLOT(loadAfterTimerTimeout()));

    KStatusBar* statusBar = (static_cast<KMainWindow*>(parent))->statusBar();
    progressBar = new QProgressBar(statusBar);
    progressBar->setMaximumSize(150, progressBar->maximumHeight());
    statusBar->addWidget(progressBar, 0, true);

    // Add icons to the toolbox tab headers
    mw->listViews->setItemIconSet(mw->listViews->indexOf(mw->tasksPage),
                                  QIconSet(KGlobal::iconLoader()->
                                           loadIcon("tj_task_group",
                                                    KIcon::Toolbar)));
    mw->listViews->setItemIconSet(mw->listViews->indexOf(mw->resourcesPage),
                                  QIconSet(KGlobal::iconLoader()->
                                           loadIcon("tj_resource_group",
                                                    KIcon::Toolbar)));
    mw->listViews->setItemIconSet(mw->listViews->indexOf(mw->accountsPage),
                                  QIconSet(KGlobal::iconLoader()->
                                           loadIcon("tj_account_group",
                                                    KIcon::Toolbar)));
    mw->listViews->setItemIconSet(mw->listViews->indexOf(mw->reportsPage),
                                  QIconSet(KGlobal::iconLoader()->
                                           loadIcon("tj_report_list",
                                                    KIcon::Toolbar)));
    mw->listViews->setItemIconSet(mw->listViews->indexOf(mw->filesPage),
                                  QIconSet(KGlobal::iconLoader()->
                                           loadIcon("tj_file_list",
                                                    KIcon::Toolbar)));
}

TaskJugglerView::~TaskJugglerView()
{
    delete project;
    delete fileManager;
    delete reportManager;
}

void
TaskJugglerView::print()
{
    switch (mw->bigTab->currentPageIndex())
    {
    case 0: // Text Editor
        fileManager->print();
    case 1: // Report
        reportManager->print();
    default: // A new widget?
        break;
    }
}

void
TaskJugglerView::readProperties(KConfig* config)
{
    QValueList<int> sizes = config->readIntListEntry("MainSplitter");
    mw->mainSplitter->setSizes(sizes);

    fileManager->readProperties(config);
}

void
TaskJugglerView::saveProperties(KConfig* config)
{
    fileManager->writeProperties(config);

    config->setGroup("Global Settings");
    /* Save the URL of the current project so we can restore it in case
     * TaskJuggler is restarted without a new URL specified. */
    if (fileManager->getMasterFile())
        config->writePathEntry("lastURL",
                               fileManager->getMasterFileURL().url());

    // Save the position of the main splitter.
    config->writeEntry("MainSplitter", mw->mainSplitter->sizes());
}

void
TaskJugglerView::newProject(KURL fileURL)
{
    if (fileManager->getMasterFile())
    {
        if (KMessageBox::warningContinueCancel
            (this, i18n("You must close the current project before you can\n"
                        "create a new project. Do you really want to do this?"),
             QString::null, KStdGuiItem::close()) != KMessageBox::Continue)
            return;
    }

    for ( ; ; )
    {
        if (fileURL.isEmpty())
            fileURL = KFileDialog::getSaveURL
                (":project", "*.tjp", this,
                 i18n("Pick a name for the new project file"));

        // Check if user cancelled the message box
        if (fileURL.isEmpty())
            return;

        if (!fileURL.isValid())
        {
            KMessageBox::sorry
                (this, i18n("The specified project name is not valid."),
                 i18n("Please enter a valid project file name."));
        }
        else if (fileURL.url().right(4) != ".tjp")
        {
            KMessageBox::sorry
                (this, i18n("Project file names must have a .tjp extension."),
                 i18n("Please enter a valid project file name."));
        }
        else if (KFileItem(fileURL, "application/x-tjp",
                           KFileItem::Unknown).size() != 0)
        {
            if (KMessageBox::warningContinueCancel
                (this, i18n("The file %1 already exists.\n"
                            "Do you really want replace the content?")
                 .arg(fileURL.url()),
                 QString::null, KStdGuiItem::ok()) == KMessageBox::Continue)
                break;
        }
        else
            break;

        fileURL = "";
    }

    QString templateFile;
    if ((templateFile = pickTemplateFile("tjp")).isEmpty())
        return;

    /* When we switch to a new project, we clear all lists and reports.
     * Loading and processing may take some time, so it looks odd to still see
     * all the old data after the load had been initiated.  So cleanup all
     * project specific data structures. */
    closeProject();

    fileManager->addFile(KURL(templateFile), fileURL);
    fileManager->setMasterFile(fileURL);
    fileManager->expandMacros();
    fileManager->saveCurrentFile(false);
    showEditor();
}

void
TaskJugglerView::newInclude()
{
    // Make sure that we have already a project we can add the file to.
    if (!fileManager->getMasterFile())
    {
        KMessageBox::sorry
            (this, i18n("You need to load or create a project before\n"
                        "you can create a new include file."),
             i18n("New Include File"));
        return;
    }

    KURL fileURL;
    for ( ; ; )
    {
        // Ask user for the name of the new file.
        fileURL = KFileDialog::getSaveURL
            (":include", "*.tji",
             this, i18n("Pick a name for the new include file"));

        // Check if user cancelled the message box
        if (fileURL.isEmpty())
            return;

        if (!fileURL.isValid())
        {
            KMessageBox::sorry
                (this, i18n("The specified file name is not valid."),
                 i18n("Please enter a valid include file name."));
        }
        else if (fileURL.url().right(4) != ".tji")
        {
            KMessageBox::sorry
                (this, i18n("Include files must have a .tji extension."),
                 i18n("Please enter a valid include file name."));
        }
        else if (KFileItem(fileURL, "application/x-tji",
                           KFileItem::Unknown).size() != 0)
        {
            if (KMessageBox::warningContinueCancel
                (this, i18n("The file %1 already exists.\n"
                            "Do you really want replace the content?")
                 .arg(fileURL.url()),
                 QString::null, KStdGuiItem::ok()) == KMessageBox::Continue)
                break;
        }
        else
            break;  // We have a valid name.
    }

    QString templateFile;
    if ((templateFile = pickTemplateFile("tji")).isEmpty())
        return;

    // Add template to list of files.
    fileManager->addFile(KURL(templateFile), fileURL);
    // Open the file list.
    mw->listViews->setCurrentItem(mw->filesPage);
    // Show the new file in the editor.
    showEditor();
    // Save the file so that it physically exists.
    fileManager->saveCurrentFile();
}

QString
TaskJugglerView::pickTemplateFile(const QString& extension)
{
    // Generate list with all template file names.
    KStandardDirs stdDirs;
    QStringList templates = KStandardDirs().findAllResources
        ("data", QString("taskjuggler/templates/") +
         KGlobal().locale()->language() + "/*." + extension);
    /* If no templates for the current language were found, try the default
     * language. */
    if (templates.count() == 0)
        templates = KStandardDirs().findAllResources
            ("data", QString("taskjuggler/templates/") +
             KGlobal().locale()->defaultLanguage() + "/*." + extension);

    if (templates.count() == 0)
    {
        KMessageBox::sorry(this, i18n("Could not find any project templates."),
                           i18n("Please check your TaskJuggler installation."));
        return QString::null;
    }

    TemplateSelector* selector = new TemplateSelector(topLevelWidget());
    for (QStringList::Iterator it = templates.begin(); it != templates.end();
         ++it)
    {
        /* We show the template name (file name without path and extension)
         * and the full file name. */
        int nameStart = (*it).findRev('/') + 1;
        QString name = (*it).mid
            (nameStart, (*it).length() - nameStart -
             QString("." + extension).length());
        // Convert all underscores to spaces.
        name.replace('_', ' ');
        KListViewItem* lvi = new KListViewItem(selector->templateList, name);
        lvi->setText(1, *it);
    }

    // Select the first entry in the list and enable the OK button.
    if (selector->templateList->firstChild())
    {
        selector->templateList->setSelected
            (selector->templateList->firstChild(), true);
        selector->buttonOk->setEnabled(true);
    }

    if (selector->exec() == QDialog::Rejected)
        return QString::null;

    return selector->templateList->selectedItem()->text(1);
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
    if (fileManager->getMasterFile() && fileManager->getMasterFileURL() == url)
        return;

    if (fileManager->getMasterFile())
    {
        // Make sure the user really wants to load a new project.
        int but = KMessageBox::warningContinueCancel
            (this, i18n("You must close the current project before you can "
                        "load a new project.\n"
                        "Do you really want to do this?"),
             i18n("Load a new Project"),
             KStdGuiItem::close());

        if (but != KMessageBox::Continue)
            return;
    }

    do
    {
        // If the URL hasn't been specified yet, ask the user for it.
        if (url.isEmpty())
        {
            url = KFileDialog::getOpenURL(":project", "*.tjp", this,
                                          i18n("Open a new Project"));
            if (url.isEmpty())
                return;
        }
        if (url.path().right(4) != ".tjp")
        {
            KMessageBox::error
                (this, i18n("The project file name must have a '.tjp' "
                            "extension."), i18n("Bad file name"));
            url = "";
        }
    }
    while (url.isEmpty());

    /* When we switch to a new project, we clear all lists and reports.
     * Loading and processing may take some time, so it looks odd to still see
     * all the old data after the load had been initiated.  So cleanup all
     * project specific data structures. */
    closeProject();

    /* This function can be triggered before the app->exec() has been called.
     * So the GUI won't show up before all files are loaded and scheduled.
     * With a timer we delay the load so we are sure app->exec has been
     * called. */
    loadDelayTimer->start(200, true);
    urlToLoad = url;
    showReportAfterLoad = true;
}

void
TaskJugglerView::loadAfterTimerTimeout()
{
    // This function catches the signal triggered by the timer set in openURL.
    loadProject(urlToLoad);
}

void
TaskJugglerView::close()
{
    fileManager->saveCurrentFile(true);
    fileManager->closeCurrentFile();
    // If we have closed the last file, we need to hide the Editor as well.
    if (!fileManager->getCurrentFile())
        hideEditor();

    // In case we closed the master file, we will close the whole project.
    if (!fileManager->getMasterFile())
        closeProject();
}

void
TaskJugglerView::closeProject()
{
    // Remove the editor component
    hideEditor();

    // Give the user the option to save all modified files.
    fileManager->saveAllFiles(true);

    // Clear all project specific data structures.
    fileManager->clear();

    reportManager->clear();
    mw->taskListView->clear();
    mw->resourceListView->clear();
    mw->accountListView->clear();
    mw->reportListView->clear();
    mw->fileListView->clear();
    mw->messageListView->clear();
    messageCounter = 0;
    slotSetTitle(i18n("No Project"));
    changeStatusBar(QString::null); // clear the status bar

    delete project;
    project = 0;
}

bool
TaskJugglerView::quit(bool force)
{
    fileManager->saveAllFiles(!force);
    fileManager->hideEditor();

    return true;
}

void
TaskJugglerView::schedule()
{
    if (!fileManager->getMasterFile())
    {
        kdDebug() << "TaskJugglerView::schedule(): No master file set" << endl;
        return;
    }

    fileManager->saveAllFiles();

    /* If the message list is visible we store the settings of the editor
     * splitter into a value list. This is reused for later errors and also
     * stored as property in the config file. */
    QValueList<int> vl = mw->editorSplitter->sizes();
    if (vl[1] > 0)
        editorSplitterSizes = vl;

    showReportAfterLoad = true;
    loadProject(fileManager->getMasterFileURL());
}

void
TaskJugglerView::stop()
{
    if (project)
        project->breakScheduling();
}

void
TaskJugglerView::generate()
{
    if (!fileManager->getMasterFile())
    {
        kdDebug() << "TaskJugglerView::schedule(): No master file set" << endl;
        return;
    }

    fileManager->saveAllFiles();

    /* If the message list is visible we store the settings of the editor
     * splitter into a value list. This is reused for later errors and also
     * stored as property in the config file. */
    QValueList<int> vl = mw->editorSplitter->sizes();
    if (vl[1] > 0)
        editorSplitterSizes = vl;

    showReportAfterLoad = false;
    loadProject(fileManager->getMasterFileURL());
    changeStatusBar(i18n("Generating Reports..."));
    if (!project->generateReports())
        showErrorMessages();
    changeStatusBar(i18n("All reports generated."));
}

void
TaskJugglerView::nextProblem()
{
    QListViewItem* lvi = mw->messageListView->currentItem();
    if (!lvi)
        return;
    // Not all items have a file name, skip those.
    do
    {
        lvi = lvi->itemBelow();
    } while (lvi && lvi->text(2).isEmpty());
    if (!lvi)
        return;
    mw->messageListView->clearSelection();
    mw->messageListView->setCurrentItem(lvi);
    lvi->setSelected(true);
    messageListClicked(lvi);

    // Messages can consist of multiple lines, so we try to make sure that at
    // least the next 2 lines are visible as well.
    mw->messageListView->ensureItemVisible(lvi);
    for (int i = 0; i < 3; ++i)
        if ((lvi = lvi->itemBelow()) != 0)
            mw->messageListView->ensureItemVisible(lvi);
        else
            break;
}

void
TaskJugglerView::previousProblem()
{
    QListViewItem* lvi = mw->messageListView->currentItem();
    if (!lvi)
        return;
    // Not all items have a file name, skip those.
    do
    {
        lvi = lvi->itemAbove();
    } while (lvi && lvi->text(2).isEmpty());
    if (!lvi)
        return;
    mw->messageListView->clearSelection();
    mw->messageListView->setCurrentItem(lvi);
    lvi->setSelected(true);
    messageListClicked(lvi);

    // Messages can consist of multiple lines, so we try to make sure that at
    // least the next 2 lines are visible as well.
    mw->messageListView->ensureItemVisible(lvi);
    for (int i = 0; i < 3; ++i)
        if ((lvi = lvi->itemBelow()) != 0)
            mw->messageListView->ensureItemVisible(lvi);
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
    {
        kdDebug() << "TaskJugglerView::loadProject(): Project loading flag is "
                                                      "already set" << endl;
        return true;
    }

    QString fileName = url.path();

    delete project;
    project = 0;
    setLoadingProject(true);
    project = new Project();
    connect(project, SIGNAL(updateProgressInfo(const QString&)),
            this, SLOT(showProgressInfo(const QString&)));
    connect(project, SIGNAL(updateProgressBar(int, int)),
            progressBar, SLOT(setProgress(int, int)));

    ProjectFile* pf = new ProjectFile(project);
    (dynamic_cast<TaskJuggler*>(parent()))->enableActions(false);
    setCursor(KCursor::waitCursor());
    if (!pf->open(fileName, "", "", true))
    {
        KMessageBox::error(this, i18n("Cannot open file %1.")
                           .arg(url.prettyURL()),
                           i18n("Error loading Project") );
        setCursor(KCursor::arrowCursor());
        (dynamic_cast<TaskJuggler*>(parent()))->enableActions(true);
        setLoadingProject(false);
        delete pf;
        return false;
    }

    emit announceRecentURL(url);

    mw->messageListView->clear();
    messageCounter = 0;
    // Reset error and warning counters
    TJMH.resetCounters();

    pf->parse();
    delete pf;
    changeStatusBar(i18n("Checking project..."));
    if (TJMH.getErrors() == 0)
        project->pass2(false);

    changeStatusBar(i18n("Scheduling..."));
    if (TJMH.getErrors() == 0)
        project->scheduleAllScenarios();

    // Load tasks into Task List View
    updateTaskList();

    // Load resources into Resource List View
    updateResourceList();

    // Load accounts into Account List View
    updateAccountList();

    // Load reports into Report List View
    reportManager->updateReportList(project);

    // Load file list into File List View
    fileManager->updateFileList(project->getSourceFiles(), url);

    setCursor(KCursor::arrowCursor());
    (dynamic_cast<TaskJuggler*>(parent()))->enableActions(true);
    setLoadingProject(false);

    // Show message list when errors have occured
    if (TJMH.getErrors() > 0)
        showErrorMessages();
    else
    {
        QValueList<int> vl;
        int h = mw->editorSplitter->height();

        vl.append(int(h));
        vl.append(int(0));
        mw->editorSplitter->setSizes(vl);
        if (TJMH.getWarnings() == 0)
            changeStatusBar(i18n("The project has been scheduled "
                             "without problems."));

        KMainWindow* mainWindow = dynamic_cast<KMainWindow*>(parent());
        mainWindow->action("next_problem")->setEnabled(false);
        mainWindow->action("previous_problem")->setEnabled(false);

        if (showReportAfterLoad)
        {
            // Open the report list.
            showReport();
            mw->listViews->setCurrentItem(mw->reportsPage);
            bool dummy;
            if (!reportManager->showReport(0, dummy))
                showErrorMessages();
        }
        else
            showEditor();

        if (TJMH.getWarnings() > 0)
            showErrorMessages();
    }


    return true;
}

void
TaskJugglerView::addWarningMessage(const QString& msg, const QString& file,
                                   int line)
{
    addMessage(msg, file, line, false);
}

void
TaskJugglerView::addErrorMessage(const QString& msg, const QString& file,
                                 int line)
{
    addMessage(msg, file, line, true);
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
                    (mw->messageListView, "", textLine, file,
                     QString::number(line),
                     QString().sprintf("%03d", ++messageCounter));
            else
                parent = new KListViewItem
                    (mw->messageListView, "", textLine, "", "",
                     QString().sprintf("%03d", ++messageCounter));

            parent->setOpen(true);
            parent->setPixmap(0, KGlobal::iconLoader()->
                              loadIcon(error ? "tj_error" : "tj_warning",
                                       KIcon::Small));
        }
        else
            new KListViewItem(parent, "", textLine, "", "",
                              QString().sprintf("%03d", ++messageCounter));
    } while (!text.isEmpty());
}

void
TaskJugglerView::setFocusToTaskList()
{
    mw->listViews->setCurrentItem(mw->tasksPage);
    focusListViews(mw->listViews->indexOf(mw->tasksPage));
}

void
TaskJugglerView::setFocusToResourceList()
{
    mw->listViews->setCurrentItem(mw->resourcesPage);
    focusListViews(mw->listViews->indexOf(mw->resourcesPage));
}

void
TaskJugglerView::setFocusToAccountList()
{
    mw->listViews->setCurrentItem(mw->accountsPage);
    focusListViews(mw->listViews->indexOf(mw->accountsPage));
}

void
TaskJugglerView::setFocusToReportList()
{
    mw->listViews->setCurrentItem(mw->reportsPage);
    focusListViews(mw->listViews->indexOf(mw->reportsPage));
}

void
TaskJugglerView::setFocusToFileList()
{
    mw->listViews->setCurrentItem(mw->filesPage);
    focusListViews(mw->listViews->indexOf(mw->filesPage));
}

void
TaskJugglerView::setFocusToEditor()
{
    showEditor();
}

void
TaskJugglerView::setFocusToReport()
{
    showReport();
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
            mw->fileListView->setFocus();
            break;
        case 1:
            mw->taskListView->setFocus();
            break;
        case 2:
            mw->resourceListView->setFocus();
            break;
        case 3:
            mw->accountListView->setFocus();
            break;
        case 4:
            mw->reportListView->setFocus();
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
            fileManager->enableEditorActions(true);
            mw->listViews->setCurrentIndex(lastBrowserUsedWithEditor);
            fileManager->showEditor();
            reportManager->enableReportActions(false);

            break;

        case 1:
            if (mw->listViews->currentItem() != mw->reportsPage)
                lastBrowserUsedWithEditor = mw->listViews->currentIndex();

            // The report page has become visible
            if (reportManager->getCurrentReport())
                windowCaption +=
                    reportManager->getCurrentReport()->getName();

            // Open report list in toolbox and select current report.
            mw->listViews->setCurrentIndex(4);
            reportManager->setFocusToReport();
            // Enable report menu and toolbar actions.
            reportManager->enableReportActions(true);

            hideEditor();

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
    fileManager->showEditor();
}

void
TaskJugglerView::hideEditor()
{
    fileManager->hideEditor();
}

void
TaskJugglerView::showReport()
{
    hideEditor();

    if (mw->listViews->currentItem() != mw->reportsPage)
        lastBrowserUsedWithEditor = mw->listViews->currentIndex();

    // Set the window caption to "<project name> - <report name>"
    QString windowCaption;
    if (project)
        windowCaption = project->getName() + " - ";

    if (reportManager && reportManager->getCurrentReport())
        windowCaption += reportManager->getCurrentReport()->getName();

    (dynamic_cast<TaskJuggler*>(parent()))->changeCaption(windowCaption);

    // Bring report page ontop.
    mw->bigTab->showPage(mw->reportTab);
    reportManager->setFocusToReport();
}

void
TaskJugglerView::showInEditor(CoreAttributes* ca)
{
    fileManager->showInEditor(ca);
    showEditor();
}

void
TaskJugglerView::showInEditor(const Report* report)
{
    fileManager->showInEditor(report);
    showEditor();
}

void
TaskJugglerView::showInEditor(const KURL& url)
{
    fileManager->addFile(url, url);
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
    reportListClicked(Qt::LeftButton, lvi, QPoint(), 0);
}

void
TaskJugglerView::reportListClicked(int button, QListViewItem* lvi,
                                   const QPoint& p, int col)
{
    if (loadingProject || !lvi)
        return;

    bool errors = false;
    bool showReportTab = true;
    switch (button)
    {
        case Qt::LeftButton:
            errors = !reportManager->showReport(lvi, showReportTab);
            break;
        case Qt::RightButton:
            reportManager->showRMBMenu(lvi, p, col, errors, showReportTab);
            break;
    }

    if (errors)
    {
        // The report generation can also produce errors.
        showEditor();
        int h = mw->editorSplitter->height();
        QValueList<int> vl;
        vl.append(int(h * 0.85));
        vl.append(int(h * 0.15));
        mw->editorSplitter->setSizes(vl);

        changeStatusBar(i18n("The project contains problems!"));
        messageListClicked(mw->messageListView->firstChild());
    }
    else if (showReportTab)
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
TaskJugglerView::tutorial()
{
    kapp->invokeHelp("tutorial");
}

void
TaskJugglerView::showErrorMessages()
{
    // The messages should be visible, so we check whether we already have
    // a setting for the splitter that is large enough. Otherwise we make
    // the message list 15% of the splitter size.
    QValueList<int> vl;
    int h = mw->editorSplitter->height();
    if (editorSplitterSizes.isEmpty() || editorSplitterSizes[1] < 120)
    {
        vl.append(int(h * 0.85));
        vl.append(int(h * 0.15));
    }
    else
        vl = editorSplitterSizes;
    mw->editorSplitter->setSizes(vl);
    changeStatusBar(i18n("The project contains problems!!"));
    showEditor();
    mw->messageListView->clearSelection();
    QListViewItem* lvi = mw->messageListView->firstChild();
    if (lvi)
    {
        /* Prevent GUI crash in case the back-end reports an error but
         * does not provide an error message. */
        mw->messageListView->setCurrentItem(lvi);
        lvi->setSelected(true);
        messageListClicked(lvi);

        KMainWindow* mainWindow = dynamic_cast<KMainWindow*>(parent());
        mainWindow->action("next_problem")->setEnabled(true);
        mainWindow->action("previous_problem")->setEnabled(true);
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

    /* We'd like to preserve as much state as possible during scheduling runs.
     * So we store the current item and all open items, so that we can restore
     * them in the new list after scheduling. This of course only works if
     * those tasks still exist. */
    QStringList openTasks, closedTasks;
    for (QListViewItemIterator lvi(tlv); *lvi; ++lvi)
        if ((*lvi)->firstChild())
            if ((*lvi)->isOpen())
                openTasks.append((*lvi)->text(1));
            else
                closedTasks.append((*lvi)->text(1));
    QString currentTask;
    if (tlv->currentItem())
        currentTask = tlv->currentItem()->text(1);

    tlv->clear();
    QListViewItem* newCurrentTask = 0;
    std::map<const char*, QListViewItem*, ltstr> tlvHash;
    for (TaskListIterator tli(project->getTaskListIterator()); *tli; ++tli)
    {
        QListViewItem* lvi;
        /* The list view has 4 columns: The task name, the task ID, the name
         * of the file where the tasks has been defined, and the line where
         * the definition started. */
        if ((*tli)->getParent() &&
            tlvHash.find((*tli)->getParent()->getId().latin1()) !=
            tlvHash.end())
            lvi = new
                KListViewItem(tlvHash[(*tli)->getParent()->getId().latin1()],
                              (*tli)->getName(),
                              (*tli)->getId(),
                              (*tli)->getDefinitionFile(),
                              QString::number((*tli)->getDefinitionLine()));
        else
            lvi = new KListViewItem
                (tlv, (*tli)->getName(), (*tli)->getId(),
                 (*tli)->getDefinitionFile(),
                 QString::number((*tli)->getDefinitionLine()));

        // Add new LVI to hash table
        char* key = new char[strlen((*tli)->getId().latin1()) + 1];
        strcpy(key, (*tli)->getId().latin1());
        tlvHash[key] = lvi;

        if ((*tli)->getId() == currentTask)
            newCurrentTask = lvi;

        if ((*tli)->isContainer())
        {
            lvi->setPixmap(0, KGlobal::iconLoader()->
                           loadIcon("tj_task_group", KIcon::Small));

            /* Tasks that have been in the list before will get their old
             * open/closed state back. New tasks will be open if they are on
             * the first 2 levels. */
            if (openTasks.find((*tli)->getId()) != openTasks.end())
                lvi->setOpen(true);
            else if (closedTasks.find((*tli)->getId()) != closedTasks.end())
                lvi->setOpen(false);
            else
                lvi->setOpen((*tli)->treeLevel() < 2);
        }
        else if ((*tli)->isMilestone())
            lvi->setPixmap(0, KGlobal::iconLoader()->
                           loadIcon("tj_milestone", KIcon::Small));
        else
            lvi->setPixmap(0, KGlobal::iconLoader()->
                           loadIcon("tj_task", KIcon::Small));
    }

    for (std::map<const char*, QListViewItem*, ltstr>::iterator
         lvit = tlvHash.begin(); lvit != tlvHash.end(); ++lvit)
        delete [] (*lvit).first;

    // Restore selected task if it's still in the list.
    if (newCurrentTask)
        tlv->setCurrentItem(newCurrentTask);

    mw->taskListViewSearch->updateSearch();
}

void
TaskJugglerView::updateResourceList()
{
    // Load resources into Resource List View
    KListView* rlv = mw->resourceListView;

    /* We'd like to preserve as much state as possible during scheduling runs.
     * So we store the current item and all open items, so that we can restore
     * them in the new list after scheduling. This of course only works if
     * those tasks still exist. */
    QStringList openResources, closedResources;
    for (QListViewItemIterator lvi(rlv); *lvi; ++lvi)
        if ((*lvi)->firstChild())
            if ((*lvi)->isOpen())
                openResources.append((*lvi)->text(1));
            else
                closedResources.append((*lvi)->text(1));
    QString currentResource;
    if (rlv->currentItem())
        currentResource = rlv->currentItem()->text(1);

    rlv->clear();
    QListViewItem* newCurrentResource = 0;
    std::map<const char*, QListViewItem*, ltstr> rlvHash;
    for (ResourceListIterator rli(project->getResourceListIterator()); *rli;
         ++rli)
    {
        QListViewItem* lvi;
        /* The list view has 4 columns: The resource name, the resource ID,
         * the name of the file where the resources has been defined, and the
         * line where the definition started. */
        if ((*rli)->getParent() &&
            rlvHash.find((*rli)->getParent()->getFullId().latin1()) !=
            rlvHash.end())
            lvi = new KListViewItem
                (rlvHash[(*rli)->getParent()->getFullId().latin1()],
                 (*rli)->getName(), (*rli)->getFullId(),
                 (*rli)->getDefinitionFile(),
                 QString::number((*rli)->getDefinitionLine()));
        else
            lvi = new KListViewItem
                (rlv, (*rli)->getName(), (*rli)->getFullId(),
                 (*rli)->getDefinitionFile(),
                 QString::number((*rli)->getDefinitionLine()));

        // Add new LVI to hash table
        char* key = new char[strlen((*rli)->getFullId().latin1()) + 1];
        strcpy(key, (*rli)->getFullId().latin1());
        rlvHash[key] = lvi;

        if ((*rli)->getFullId() == currentResource)
            newCurrentResource = lvi;

        if ((*rli)->hasSubs())
        {
            lvi->setPixmap(0, KGlobal::iconLoader()->
                           loadIcon("tj_resource_group", KIcon::Small));
            if (openResources.find((*rli)->getId()) != openResources.end())
                lvi->setOpen(true);
            else if (closedResources.find((*rli)->getId()) !=
                     closedResources.end())
                lvi->setOpen(false);
            else
                lvi->setOpen((*rli)->treeLevel() < 2);
        }
        else
            lvi->setPixmap(0, KGlobal::iconLoader()->
                           loadIcon("tj_resource", KIcon::Small));
    }

    for (std::map<const char*, QListViewItem*, ltstr>::iterator
         lvit = rlvHash.begin(); lvit != rlvHash.end(); ++lvit)
        delete [] (*lvit).first;

    // Restore selected resource if it's still in the list.
    if (newCurrentResource)
        rlv->setCurrentItem(newCurrentResource);

    mw->resourceListViewSearch->updateSearch();
}

void
TaskJugglerView::updateAccountList()
{
    // Load accounts into Account List View
    KListView* alv = mw->accountListView;

    /* We'd like to preserve as much state as possible during scheduling runs.
     * So we store the current item and all open items, so that we can restore
     * them in the new list after scheduling. This of course only works if
     * those tasks still exist. */
    QStringList openAccounts, closedAccounts;
    for (QListViewItemIterator lvi(alv); *lvi; ++lvi)
        if ((*lvi)->firstChild())
            if ((*lvi)->isOpen())
                openAccounts.append((*lvi)->text(1));
            else
                closedAccounts.append((*lvi)->text(1));
    QString currentAccount;
    if (alv->currentItem())
        currentAccount = alv->currentItem()->text(1);

    alv->clear();
    QListViewItem* newCurrentAccount= 0;
    std::map<const char*, QListViewItem*, ltstr> alvHash;
    for (AccountListIterator ali(project->getAccountListIterator()); *ali;
         ++ali)
    {
        QListViewItem* lvi;
        if ((*ali)->getParent() &&
            alvHash.find((*ali)->getParent()->getFullId().latin1()) !=
            alvHash.end())
            lvi = new KListViewItem
                (alvHash[(*ali)->getParent()->getFullId().latin1()],
                 (*ali)->getName(), (*ali)->getFullId(),
                 (*ali)->getDefinitionFile(),
                 QString::number((*ali)->getDefinitionLine()));
        else
            lvi = new KListViewItem
                (alv, (*ali)->getName(), (*ali)->getFullId(),
                 (*ali)->getDefinitionFile(),
                 QString::number((*ali)->getDefinitionLine()));

        // Add new LVI to hash table
        char* key = new char[strlen((*ali)->getFullId().latin1()) + 1];
        strcpy(key, (*ali)->getFullId().latin1());
        alvHash[key] = lvi;

        if ((*ali)->getFullId() == currentAccount)
            newCurrentAccount = lvi;

        if ((*ali)->hasSubs())
        {
            lvi->setPixmap(0, KGlobal::iconLoader()->
                           loadIcon("tj_account_group", KIcon::Small));
            if (openAccounts.find((*ali)->getId()) != openAccounts.end())
                lvi->setOpen(true);
            else if (closedAccounts.find((*ali)->getId()) !=
                     closedAccounts.end())
                lvi->setOpen(false);
            else
                lvi->setOpen((*ali)->treeLevel() < 2);
        }
        else
            lvi->setPixmap(0, KGlobal::iconLoader()->
                           loadIcon("tj_account", KIcon::Small));
    }

    for (std::map<const char*, QListViewItem*, ltstr>::iterator
         lvit = alvHash.begin(); lvit != alvHash.end(); ++lvit)
        delete [] (*lvit).first;

    // Restore selected resource if it's still in the list.
    if (newCurrentAccount)
        alv->setCurrentItem(newCurrentAccount);

    mw->accountListViewSearch->updateSearch();
}

#include "taskjugglerview.moc"
