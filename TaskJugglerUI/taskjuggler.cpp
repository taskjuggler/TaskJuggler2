/*
 * The TaskJuggler Project Management Software
 *
 * Copyright (c) 2001, 2002, 2003, 2004, 2005, 2006, 2007
 *               by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */


#include "taskjuggler.h"
#include "pref.h"

#include <qdragobject.h>
#include <qpainter.h>
#include <qpaintdevicemetrics.h>
#include <qtimer.h>
#include <qstringlist.h>

#include <kglobal.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kdeversion.h>
#include <kmenubar.h>
#include <kstatusbar.h>
#include <kkeydialog.h>
#include <kfiledialog.h>
#include <kfileitem.h>
#include <kaccel.h>
#include <kio/netaccess.h>
#include <kconfig.h>
#include <kstandarddirs.h>
#include <kurl.h>
#include <kurldrag.h>
#include <kurlrequesterdlg.h>
#include <kedittoolbar.h>
#include <kstdaccel.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <kstdaction.h>
#include <kdebug.h>
#include <kprinter.h>
#include <kmessagebox.h>
#include <ktip.h>

TaskJuggler::TaskJuggler()
    : KMainWindow( 0, "TaskJuggler" ),
      m_view(new TaskJugglerView(this))
{
    // accept dnd
    setAcceptDrops(true);

    // tell the KMainWindow that this is indeed the main widget
    setCentralWidget(m_view);

    // then, setup our actions
    setupActions();

    // and a status bar
    statusBar()->show();

    // allow the view to change the statusbar and caption
    connect(m_view, SIGNAL(signalChangeStatusbar(const QString&)),
            this,   SLOT(changeStatusbar(const QString&)));
    connect(m_view, SIGNAL(signalChangeCaption(const QString&)),
            this,   SLOT(changeCaption(const QString&)));
    connect(m_view, SIGNAL(announceRecentURL(const KURL&)),
            this, SLOT(addRecentURL(const KURL&)));

    /* Add the taskjuggler apps dir as additional resource so the katepart
     * will find the katefiletyperc file in there. */
    KGlobal().dirs()->addResourceDir
        ("config", KGlobal().dirs()->findResourceDir
         ("data", "taskjuggler/katefiletyperc") + "/taskjuggler/");

    readProperties(kapp->config());

    delayTimer = new QTimer(this);
    connect(delayTimer, SIGNAL(timeout()),
            this, SLOT(showTipOnStart()));
    delayTimer->start(200, true);
}

TaskJuggler::~TaskJuggler()
{
}

void
TaskJuggler::load(const KURL& fileURL)
{
    if (KFileItem(fileURL, "application/x-tjp",
                           KFileItem::Unknown).size() == 0)
    {
        if (KMessageBox::warningContinueCancel
            (this, i18n("The project file '%1' cannot be opened. If you "
                        "continue, a new file with this name will be "
                        "created.").arg(fileURL.url()), i18n("File not found"),
             KStdGuiItem::cont(), "WarnOnNewFileCreation") ==
            KMessageBox::Cancel)
            return;

        // Create a new project with the specified name.`
        m_view->newProject(fileURL);
    }
    else
        m_view->openURL(fileURL);
}

void TaskJuggler::setupActions()
{
    // "File" menu
    KStdAction::openNew(this, SLOT(fileNew()), actionCollection());
    new KAction(i18n("New &Include File" ), "file_temporary", KShortcut(),
                this, SLOT(fileNewInclude()),
                actionCollection(), "new_include");
    KStdAction::open(this, SLOT(fileOpen()), actionCollection());
    KStdAction::close(this, SLOT(fileClose()), actionCollection());
    KStdAction::print(this, SLOT(filePrint()), actionCollection());
    KStdAction::quit(kapp, SLOT(closeAllWindows()), actionCollection());

    // Setup "Open Recent" menu and load old recent files.
    m_recentAction = KStdAction::openRecent(this, SLOT(load(const KURL&)),
                                            actionCollection());


    // "Goto" menu
    new KAction(i18n("Tas&ks"), "tj_task_group", KShortcut(KKey("ALT+k")),
                m_view, SLOT(setFocusToTaskList()),
                actionCollection(), "tasks");
    new KAction(i18n("&Resources"), "tj_resource_group",
                KShortcut(KKey("ALT+r")),
                m_view, SLOT(setFocusToResourceList()),
                actionCollection(), "resources");
    new KAction(i18n("&Accounts"), "tj_account_group",
                KShortcut(KKey("ALT+a")),
                m_view, SLOT(setFocusToAccountList()),
                actionCollection(), "accounts");
    new KAction(i18n("Re&ports"), "tj_report_list", KShortcut(KKey("ALT+p")),
                m_view, SLOT(setFocusToReportList()),
                actionCollection(), "reports");
    new KAction(i18n("F&iles"), "tj_file_list", KShortcut(KKey("ALT+i")),
                m_view, SLOT(setFocusToFileList()),
                actionCollection(), "files");
    new KAction(i18n("E&ditor"), "tj_editor", KShortcut("ALT+d"),
                m_view, SLOT(setFocusToEditor()),
                actionCollection(), "editor");
    new KAction(i18n("Rep&ort"), "tj_report", KShortcut("ALT+o"),
                m_view, SLOT(setFocusToReport()),
                actionCollection(), "report");

    // "Tools" menu
    new KAction(i18n("&Schedule"), "tj_schedule", KShortcut(KKey("F9")),
                m_view, SLOT(schedule()),
                actionCollection(), "schedule");
    new KAction(i18n("Stop scheduling"), "stop", 0,
                m_view, SLOT(stop()),
                actionCollection(), "stop");
    new KAction(i18n("Goto &previous Problem"), "tj_previous_problem",
                KShortcut(KKey("F10")),
                m_view, SLOT(previousProblem()),
                actionCollection(), "previous_problem");
    new KAction(i18n("Goto &next Problem"), "tj_next_problem",
                KShortcut(KKey("F11")),
                m_view, SLOT(nextProblem()),
                actionCollection(), "next_problem");

    new KAction(i18n("Zoom &In"), "viewmag+", KShortcut(KKey("F7")),
                m_view, SLOT(zoomIn()),
                actionCollection(), "zoom_in");
    new KAction(i18n("Zoom &Out"), "viewmag-", KShortcut(KKey("F8")),
                m_view, SLOT(zoomOut()),
                actionCollection(), "zoom_out");

    // "Help" menu
    new KAction(i18n("Tip of the day"), "idea", 0, this,
                SLOT(showTip()), actionCollection(), "tip");
    new KAction(i18n("Explain Keyword"), "tj_keyword_help",
                KShortcut(KKey("F2")),
                m_view, SLOT(keywordHelp()),
                actionCollection(), "keyword_help");
    new KAction(i18n("Tutorial"), "tj_tutorial", 0,
                m_view, SLOT(tutorial()),
                actionCollection(), "tutorial");

    setupGUI(ToolBar | Keys | StatusBar | Save | Create);
}

void
TaskJuggler::enableActions(bool enable)
{
    if (!enable)
        enabledActionsBuf.clear();

    KActionPtrList actionList = actionCollection()->actions();
    for (KActionPtrList::iterator it = actionList.begin();
         it != actionList.end(); ++it)
    {
        /* The "stop" action will be handled opposite to all other actions. */
        if (strcmp((*it)->name(), "stop") == 0)
        {
            (*it)->setEnabled(!enable);
            continue;
        }

        if (enable)
        {
            if (enabledActionsBuf.find((*it)->name()) !=
                enabledActionsBuf.end())
                (*it)->setEnabled(true);
        }
        else
        {
            if ((*it)->isEnabled())
            {
                enabledActionsBuf.insert((*it)->name());
                (*it)->setEnabled(false);
            }
        }
    }
}

void
TaskJuggler::saveProperties(KConfig* config)
{
    config->setGroup("Global Settings");

    m_view->saveProperties(config);

    m_recentAction->saveEntries(config);
}

void
TaskJuggler::readProperties(KConfig* config)
{
    config->setGroup("Global Settings");
    lastURL = config->readPathEntry("lastURL");

    m_recentAction->loadEntries(config);

    m_view->readProperties(config);
}

void
TaskJuggler::loadLastProject()
{
    if (!lastURL.isEmpty())
        m_view->openURL(KURL(lastURL));
}

void
TaskJuggler::dragEnterEvent(QDragEnterEvent *event)
{
    // accept uri drops only
    event->accept(KURLDrag::canDecode(event));
}

void
TaskJuggler::dropEvent(QDropEvent *event)
{
    // this is a very simplistic implementation of a drop event.  we
    // will only accept a dropped URL.  the Qt dnd code can do *much*
    // much more, so please read the docs there
    KURL::List urls;

    // see if we can decode a URI.. if not, just ignore it
    if (KURLDrag::decode(event, urls) && !urls.isEmpty())
    {
        // okay, we have a URI.. process it
        const KURL &url = urls.first();

        // load in the file
        load(url);
    }
}

bool
TaskJuggler::queryClose()
{
    return true;
}

bool
TaskJuggler::queryExit()
{
    m_view->quit(false);
    saveProperties(kapp->config());
    return true;
}

void
TaskJuggler::fileNew()
{
    m_view->newProject();
}

void
TaskJuggler::fileNewInclude()
{
    m_view->newInclude();
}

void
TaskJuggler::fileOpen()
{
    m_view->openURL(KURL());
}

void
TaskJuggler::fileClose()
{
    m_view->close();
}

void
TaskJuggler::filePrint()
{
    m_view->print();
}

void
TaskJuggler::optionsPreferences()
{
    // popup some sort of preference dialog, here
    TaskJugglerPreferences dlg;
    if (dlg.exec())
    {
        // redo your settings
    }
}

void
TaskJuggler::showTip()
{
    KTipDialog::showTip(this, QString::null, true);
}

void
TaskJuggler::showTipOnStart()
{
    delete delayTimer;
    delayTimer = 0;

    KTipDialog::showTip(this);
}

void TaskJuggler::changeStatusbar(const QString& text)
{
    // display the text on the statusbar
    statusBar()->message(text);
}

void TaskJuggler::changeCaption(const QString& text)
{
    // display the text on the caption
    setCaption(text);
}

void
TaskJuggler::addRecentURL(const KURL& url)
{
    m_recentAction->addURL(url);
    setCaption(url.prettyURL());
}

#include "taskjuggler.moc"
