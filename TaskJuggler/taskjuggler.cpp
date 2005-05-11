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


#include "taskjuggler.h"
#include "pref.h"

#include <qdragobject.h>
#include <qpainter.h>
#include <qpaintdevicemetrics.h>
#include <qmessagebox.h>

#include <kglobal.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kdeversion.h>
#include <kmenubar.h>
#include <kstatusbar.h>
#include <kkeydialog.h>
#include <kfiledialog.h>
#include <kaccel.h>
#include <kio/netaccess.h>
#include <kconfig.h>
#include <kurl.h>
#include <kurldrag.h>
#include <kurlrequesterdlg.h>
#include <kedittoolbar.h>
#include <kstdaccel.h>
#include <kaction.h>
#include <kstdaction.h>
#include <kdebug.h>
#include <kprinter.h>

TaskJuggler::TaskJuggler()
    : KMainWindow( 0, "TaskJuggler" ),
      m_view(new TaskJugglerView(this)),
      m_printer(0)
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

    readProperties(kapp->config());
}

TaskJuggler::~TaskJuggler()
{
}

void TaskJuggler::load(const KURL& url)
{
    setCaption(url.prettyURL());
    m_view->openURL(url);
    m_recentAction->addURL(url);
}

void TaskJuggler::setupActions()
{
    // "File" menu
    KStdAction::openNew(this, SLOT(fileNew()), actionCollection());
    new KAction(i18n("New &Include File" ), "file_temporary", KShortcut(),
                this, SLOT(fileNewInclude()),
                actionCollection(), "new_include");
    KStdAction::open(this, SLOT(fileOpen()), actionCollection());
    KStdAction::save(this, SLOT(fileSave()), actionCollection());
    KStdAction::saveAs(this, SLOT(fileSaveAs()), actionCollection());
    KStdAction::close(this, SLOT(fileClose()), actionCollection());
    KStdAction::print(this, SLOT(filePrint()), actionCollection());
    KStdAction::quit(kapp, SLOT(closeAllWindows()), actionCollection());

    // Setup "Open Recent" menu and load old recent files.
    m_recentAction = KStdAction::openRecent(this, SLOT(load(const KURL&)),
                                            actionCollection());


    // "Edit" menu
    KStdAction::undo(m_view, SLOT(undo()), actionCollection());
    KStdAction::redo(m_view, SLOT(redo()), actionCollection());
    KStdAction::cut(m_view, SLOT(cut()), actionCollection());
    KStdAction::copy(m_view, SLOT(copy()), actionCollection());
    KStdAction::paste(m_view, SLOT(paste()), actionCollection());
    KStdAction::selectAll(m_view, SLOT(selectAll()), actionCollection());
    KStdAction::find(m_view, SLOT(find()), actionCollection());
    KStdAction::findNext(m_view, SLOT(findNext()), actionCollection());
    KStdAction::findPrev(m_view, SLOT(findPrevious()), actionCollection());

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
    new KAction(i18n("Re&ports"), "tj_reports", KShortcut(KKey("ALT+p")),
                m_view, SLOT(setFocusToReportList()),
                actionCollection(), "reports");
    new KAction(i18n("F&iles"), "tj_file_list", KShortcut(KKey("ALT+i")),
                m_view, SLOT(setFocusToFileList()),
                actionCollection(), "files");
    new KAction(i18n("&Editor"), "tj_editor", KShortcut(),
                m_view, SLOT(setFocusToEditor()),
                actionCollection(), "editor");
    new KAction(i18n("Rep&ort"), "tj_report", KShortcut(),
                m_view, SLOT(setFocusToReport()),
                actionCollection(), "report");

    // "Tools" menu
    new KAction(i18n("&Schedule"), "tj_schedule", KShortcut(KKey("F9")),
                m_view, SLOT(schedule()),
                actionCollection(), "schedule");
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

    // "Settings" menu
    /* KStdAction::preferences(this, SLOT(optionsPreferences()),
                            actionCollection()); */
    new KAction(i18n("Configure Editor" ), "", 0,
                m_view, SLOT(configureEditor()),
                actionCollection(), "configure_editor");

    // "Help" menu
    new KAction(i18n("Explain Keyword" ), "tj_keyword_help",
                KShortcut(KKey("F2")),
                m_view, SLOT(keywordHelp()),
                actionCollection(), "keyword_help");

    setupGUI( ToolBar | Keys | StatusBar | Save | Create );
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
    QString url = config->readPathEntry("lastURL");

    if (!url.isEmpty())
        m_view->openURL(KURL(url));

    m_recentAction->loadEntries(config);

    m_view->readProperties(config);
}

void TaskJuggler::dragEnterEvent(QDragEnterEvent *event)
{
    // accept uri drops only
    event->accept(KURLDrag::canDecode(event));
}

void TaskJuggler::dropEvent(QDropEvent *event)
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
    return TRUE;
}

bool
TaskJuggler::queryExit()
{
    saveProperties(kapp->config());
    return TRUE;
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
TaskJuggler::fileSave()
{
    m_view->save();
}

void TaskJuggler::fileSaveAs()
{
    // this slot is called whenever the File->Save As menu is selected,
    m_view->saveAs();
}

void
TaskJuggler::fileClose()
{
    m_view->close();
}

void TaskJuggler::filePrint()
{
    QMessageBox::information(this, "TaskJuggler",
                             i18n("Sorry, printing is not yet implemented!"),
                             QMessageBox::Ok | QMessageBox::Default,
                             QMessageBox::NoButton);
    return;
#if 0
    // this slot is called whenever the File->Print menu is selected,
    // the Print shortcut is pressed (usually CTRL+P) or the Print toolbar
    // button is clicked
    if (!m_printer) m_printer = new KPrinter;
    if (m_printer->setup(this))
    {
        // setup the printer.  with Qt, you always "print" to a
        // QPainter.. whether the output medium is a pixmap, a screen,
        // or paper
        QPainter p;
        p.begin(m_printer);

        // we let our view do the actual printing
        QPaintDeviceMetrics metrics(m_printer);
        m_view->print(&p, metrics.height(), metrics.width());

        // and send the result to the printer
        p.end();
    }
#endif
}

void TaskJuggler::optionsPreferences()
{
    // popup some sort of preference dialog, here
    TaskJugglerPreferences dlg;
    if (dlg.exec())
    {
        // redo your settings
    }
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
}

#include "taskjuggler.moc"
