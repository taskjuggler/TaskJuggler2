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
#include <kprinter.h>
#include <qpainter.h>
#include <qpaintdevicemetrics.h>

#include <kglobal.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kdeversion.h>
#include <kmenubar.h>
#include <kstatusbar.h>
#include <kkeydialog.h>
#include <kaccel.h>
#include <kio/netaccess.h>
#include <kfiledialog.h>
#include <kconfig.h>
#include <kurl.h>
#include <kurldrag.h>
#include <kurlrequesterdlg.h>
#include <kedittoolbar.h>
#include <kstdaccel.h>
#include <kaction.h>
#include <kstdaction.h>
#include <kdebug.h>

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

    // apply the saved mainwindow settings, if any, and ask the mainwindow
    // to automatically save settings if changed: window size, toolbar
    // position, icon size, etc.
    setAutoSaveSettings();

    // allow the view to change the statusbar and caption
    connect(m_view, SIGNAL(signalChangeStatusbar(const QString&)),
            this,   SLOT(changeStatusbar(const QString&)));
    connect(m_view, SIGNAL(signalChangeCaption(const QString&)),
            this,   SLOT(changeCaption(const QString&)));
    connect(m_view, SIGNAL(announceRecentURL(const KURL&)),
            this, SLOT(addRecentURL(const KURL&)));

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
    KStdAction::openNew(this, SLOT(fileNew()), actionCollection());
    new KAction(i18n("New &Include File" ), "file_temporary", KShortcut(),
                this, SLOT(fileNewInclude()),
                actionCollection(), "new_include");
    KStdAction::open(this, SLOT(fileOpen()), actionCollection());
    KStdAction::save(this, SLOT(fileSave()), actionCollection());
    KStdAction::saveAs(this, SLOT(fileSaveAs()), actionCollection());
    KStdAction::close(this, SLOT(fileClose()), actionCollection());
    KStdAction::print(this, SLOT(filePrint()), actionCollection());
    KStdAction::quit(kapp, SLOT(quit()), actionCollection());

    // Setup "Open Recent" menu and load old recent files.
    m_recentAction = KStdAction::openRecent(this, SLOT( load(const KURL&)),
                                            actionCollection());
    m_recentAction->loadEntries(kapp->config());

    m_toolbarAction = KStdAction::showToolbar(this, SLOT(optionsShowToolbar()),
                                              actionCollection());
    m_statusbarAction = KStdAction::showStatusbar
        (this, SLOT(optionsShowStatusbar()), actionCollection());

    KStdAction::keyBindings(this, SLOT(optionsConfigureKeys()),
                            actionCollection());
    KStdAction::configureToolbars(this, SLOT(optionsConfigureToolbars()),
                                  actionCollection());
    KStdAction::preferences(this, SLOT(optionsPreferences()),
                            actionCollection());

    new KAction(i18n("Tas&ks" ), "kontact_todo", KShortcut(KKey("ALT+k")),
                m_view, SLOT(setFocusToTaskList()),
                actionCollection(), "tasks");
    new KAction(i18n("&Resources" ), "kontact_contacts",
                KShortcut(KKey("ALT+r")),
                m_view, SLOT(setFocusToResourceList()),
                actionCollection(), "resources");
    new KAction(i18n("&Accounts" ), "kontact_summary", KShortcut(KKey("ALT+a")),
                m_view, SLOT(setFocusToAccountList()),
                actionCollection(), "accounts");
    new KAction(i18n("Re&ports" ), "identity", KShortcut(KKey("ALT+p")),
                m_view, SLOT(setFocusToReportList()),
                actionCollection(), "reports");
    new KAction(i18n("F&iles" ), "editcopy", KShortcut(KKey("ALT+i")),
                m_view, SLOT(setFocusToFileList()),
                actionCollection(), "files");
    new KAction(i18n("&Editor" ), "edit", KShortcut(),
                m_view, SLOT(setFocusToEditor()),
                actionCollection(), "editor");
    new KAction(i18n("Rep&ort" ), "contents", KShortcut(),
                m_view, SLOT(setFocusToReport()),
                actionCollection(), "report");

    new KAction(i18n("&Schedule" ), "history", KShortcut(KKey("F9")),
                m_view, SLOT(schedule()),
                actionCollection(), "schedule");
    new KAction(i18n("Goto &previous Problem"), "previous",
                KShortcut(KKey("F10")),
                m_view, SLOT(previousProblem()),
                actionCollection(), "previous_problem");
    new KAction(i18n("Goto &next Problem"), "next", KShortcut(KKey("F11")),
                m_view, SLOT(nextProblem()),
                actionCollection(), "next_problem");

    new KAction(i18n("Explain Keyword" ), "kghostview", KShortcut(KKey("F2")),
                m_view, SLOT(keywordHelp()),
                actionCollection(), "keyword_help");

    createGUI();
}

void
TaskJuggler::saveProperties(KConfig* config)
{
    if (!m_view->currentURL().isEmpty()) {
#if KDE_IS_VERSION(3,1,3)
        config->writePathEntry("lastURL", m_view->currentURL());
#else
        config->writeEntry("lastURL", m_view->currentURL());
#endif
    }
    m_recentAction->saveEntries(config);
}

void
TaskJuggler::readProperties(KConfig* config)
{
    QString url = config->readPathEntry("lastURL");

    if (!url.isEmpty())
        m_view->openURL(KURL(url));
    m_recentAction->loadEntries(config);
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
TaskJuggler::queryExit()
{
    saveProperties(kapp->config());
    return TRUE;
}

void
TaskJuggler::fileNew()
{
    KURL file_url = KFileDialog::getSaveURL();
    if (!file_url.isEmpty() && file_url.isValid())
    {
        m_view->newProject(file_url);
    }
}

void
TaskJuggler::fileNewInclude()
{
    KURL file_url = KFileDialog::getSaveURL();
    if (!file_url.isEmpty() && file_url.isValid())
    {
        m_view->newInclude(file_url);
    }
}

void
TaskJuggler::fileOpen()
{
/*
    // this brings up the generic open dialog
    KURL url = KURLRequesterDlg::getURL(QString::null, this, i18n("Open Location") );
*/
    // standard filedialog
    KURL url = KFileDialog::getOpenURL(QString::null, QString::null, this, i18n("Open Location"));
    if (!url.isEmpty())
        m_view->openURL(url);
}

void
TaskJuggler::fileSave()
{
    m_view->save();
}

void TaskJuggler::fileSaveAs()
{
    // this slot is called whenever the File->Save As menu is selected,
    KURL file_url = KFileDialog::getSaveURL();
    if (!file_url.isEmpty() && file_url.isValid())
    {
        m_view->saveAs(file_url);
    }
}

void
TaskJuggler::fileClose()
{
    m_view->close();
}

void TaskJuggler::filePrint()
{
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
}

void TaskJuggler::optionsShowToolbar()
{
    // this is all very cut and paste code for showing/hiding the
    // toolbar
    if (m_toolbarAction->isChecked())
        toolBar()->show();
    else
        toolBar()->hide();
}

void TaskJuggler::optionsShowStatusbar()
{
    // this is all very cut and paste code for showing/hiding the
    // statusbar
    if (m_statusbarAction->isChecked())
        statusBar()->show();
    else
        statusBar()->hide();
}

void TaskJuggler::optionsConfigureKeys()
{
    KKeyDialog::configure(actionCollection());
}

void TaskJuggler::optionsConfigureToolbars()
{
    // use the standard toolbar editor
#if defined(KDE_MAKE_VERSION)
# if KDE_VERSION >= KDE_MAKE_VERSION(3,1,0)
    saveMainWindowSettings(KGlobal::config(), autoSaveGroup());
# else
    saveMainWindowSettings(KGlobal::config());
# endif
#else
    saveMainWindowSettings(KGlobal::config());
#endif
}

void TaskJuggler::newToolbarConfig()
{
    // this slot is called when user clicks "Ok" or "Apply" in the toolbar editor.
    // recreate our GUI, and re-apply the settings (e.g. "text under icons", etc.)
    createGUI();

#if defined(KDE_MAKE_VERSION)
# if KDE_VERSION >= KDE_MAKE_VERSION(3,1,0)
    applyMainWindowSettings(KGlobal::config(), autoSaveGroup());
# else
    applyMainWindowSettings(KGlobal::config());
# endif
#else
    applyMainWindowSettings(KGlobal::config());
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
