/***************************************************************************
 *   Copyright (C) 2004 by Lukas Tinkl                                     *
 *   lukas.tinkl@suse.cz                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "ktjview2.h"
#include "settings.h"
#include "gantt.h"
#include "resUsageCfg.h"

#include <qdragobject.h>
#include <qpainter.h>
#include <qpaintdevicemetrics.h>

#include <kprinter.h>
#include <kglobal.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kdeversion.h>
#include <kmenubar.h>
#include <ktoolbar.h>
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
#include <kconfigdialog.h>


ktjview2::ktjview2()
    : KMainWindow( 0, "ktjview2" ),
      m_view( new ktjview2View(this) )
{
    // accept dnd
    setAcceptDrops( true );

    // tell the KMainWindow that this is indeed the main widget
    setCentralWidget( m_view );

    // then, setup our actions
    setupActions();

    // and a status bar
    statusBar()->show();

    // apply the saved mainwindow settings, if any, and ask the mainwindow
    // to automatically save settings if changed: window size, toolbar
    // position, icon size, etc.
    setAutoSaveSettings();

    // allow the view to change the statusbar and caption
    connect( m_view, SIGNAL(signalChangeStatusbar(const QString&)),
             this,   SLOT(changeStatusbar(const QString&)) );
    connect( m_view, SIGNAL(signalChangeCaption(const QString&)),
             this,   SLOT(setCaption(const QString&)) );

    m_activeTaskFilter = 0;
    m_activeResourceFilter = 0;

    slotSwitchView( ID_VIEW_INFO );
}

ktjview2::~ktjview2()
{
}

void ktjview2::load( const KURL& url )
{
    if ( m_view->openURL( url ) )
        m_recentAction->addURL( url );
}

void ktjview2::setupActions()
{
#if 0
    KStdAction::openNew( this, SLOT(fileNew()), actionCollection() ); // FIXME
#endif

    KStdAction::open( this, SLOT(fileOpen()), actionCollection() );

    m_recentAction = KStdAction::openRecent( this, SLOT( load( const KURL& ) ), actionCollection() );
    m_recentAction->setMaxItems( 10 );
    m_recentAction->loadEntries( kapp->config() );

    KStdAction::print(this, SLOT(filePrint()), actionCollection());
    KStdAction::quit(kapp, SLOT(quit()), actionCollection());

    KStdAction::keyBindings( this, SLOT(optionsConfigureKeys()), actionCollection() );
    KStdAction::configureToolbars( this, SLOT(optionsConfigureToolbars()), actionCollection() );
    KStdAction::preferences( this, SLOT(optionsPreferences()), actionCollection() );

    // View menu
#if 0
    KStdAction::find( m_view, SLOT( find() ), actionCollection()); // TODO
    new KAction( i18n( "&Filter..." ), "filter", KShortcut(),
                 m_view, SLOT( filter() ), actionCollection(), "filter" );
#endif

    // Tasks menu
    m_filterForTasksAction = new KSelectAction( i18n( "&Filter for: All Tasks" ), "filter", KShortcut(),
                                                this, SLOT( slotFilterForTasks() ), actionCollection(), "filter_for_tasks" );
    m_taskFilterItems << i18n( "All Tasks" ) << i18n( "Completed Tasks" ) << i18n( "Date Range..." )
                      << i18n( "Incomplete Tasks" ) << i18n( "Milestones" ) << i18n( "Summary Tasks" )
                      << i18n( "Using Resource..." ) << i18n( "Having Flag..." );

    m_filterForTasksAction->setItems( m_taskFilterItems );
    m_filterForTasksAction->setCurrentItem( 0 ); // All Tasks by default

    // Gantt menu
    m_calendarAction = new KToggleAction( i18n( "&Calendar mode" ), "today", KShortcut(),
                                          this, SLOT( setCalendarMode() ),
                                          actionCollection(), "calendar_mode" );
    new KAction( i18n( "Zoom &in" ), "viewmag+", KStdAccel::shortcut( KStdAccel::ZoomIn ),
                 m_view, SLOT( zoomIn() ), actionCollection(), "zoom_in" );
    new KAction( i18n( "Zoom &out" ), "viewmag-", KStdAccel::shortcut( KStdAccel::ZoomOut ),
                 m_view, SLOT( zoomOut() ), actionCollection(), "zoom_out" );
    new KAction( i18n( "&Fit to page" ), "viewmagfit", KShortcut( "Ctrl+0" ),
                 m_view, SLOT( zoomFit() ), actionCollection(), "fit_to_page" );
    new KAction( i18n( "Set &timeframe" ), "clock", KShortcut(),
                 m_view, SLOT( slotZoomTimeframe() ), actionCollection(), "timeframe" );

    m_scaleAction = new KSelectAction( i18n( "&Scale" ), 0, actionCollection(), "scale" );
    QStringList items = QStringList();
    items << i18n( "Minute" ) << i18n( "Hour" ) << i18n( "Day" ) <<
        i18n( "Week" ) << i18n( "Month" ) << i18n( "Auto" );
    m_scaleAction->setItems( items );
    m_scaleAction->setCurrentItem( 5 ); // TODO make configurable
    connect( m_scaleAction, SIGNAL( activated( int ) ),
             m_view, SLOT( slotScale( int ) ) );
    connect( m_view, SIGNAL( signalScaleChanged( int ) ),
             m_scaleAction, SLOT( setCurrentItem( int ) ) );

    // Resources menu
    new KAction( i18n( "&Collapse All" ), 0, KShortcut(), this, SLOT( collapseAll() ), actionCollection(), "collapse_all" );
    new KAction( i18n( "Show Level &1" ), 0, KShortcut(), this, SLOT( outline1() ), actionCollection(), "outline_1" );
    new KAction( i18n( "Show Level &2" ), 0, KShortcut(), this, SLOT( outline2() ), actionCollection(), "outline_2" );
    new KAction( i18n( "Show Level &3" ), 0, KShortcut(), this, SLOT( outline3() ), actionCollection(), "outline_3" );
    new KAction( i18n( "Show Level &4" ), 0, KShortcut(), this, SLOT( outline4() ), actionCollection(), "outline_4" );
    new KAction( i18n( "Show Level &5" ), 0, KShortcut(), this, SLOT( outline5() ), actionCollection(), "outline_5" );
    new KAction( i18n( "Show Level &6" ), 0, KShortcut(), this, SLOT( outline6() ), actionCollection(), "outline_6" );
    new KAction( i18n( "Show Level &7" ), 0, KShortcut(), this, SLOT( outline7() ), actionCollection(), "outline_7" );
    new KAction( i18n( "Show Level &8" ), 0, KShortcut(), this, SLOT( outline8() ), actionCollection(), "outline_8" );
    new KAction( i18n( "Show Level &9" ), 0, KShortcut(), this, SLOT( outline9() ), actionCollection(), "outline_9" );
    new KAction( i18n( "&Expand All" ), 0, KShortcut(), this, SLOT( expandAll() ), actionCollection(), "expand_all" );


    m_filterForResourcesAction = new KSelectAction( i18n( "&Filter for: All Resources" ), "filter", KShortcut(),
                                                    this, SLOT( slotFilterForResources() ), actionCollection(), "filter_for_resources" );
    m_resourceFilterItems << i18n( "All Resources" ) << i18n( "Resources On Vacation..." ) << i18n( "Resources On Shift..." )
                          << i18n( "Allocated Resources..." ) << i18n( "Having Flag..." );

    m_filterForResourcesAction->setItems( m_resourceFilterItems );
    m_filterForResourcesAction->setCurrentItem( 0 ); // All Resources by default


    // Filter toolbar
    m_quickSearch = new QuickSearchWidget( this );
    ( void ) new KWidgetAction( m_quickSearch, i18n( "Quick search" ), KShortcut(), 0, 0,
                                actionCollection(), "quick_search" );

#if 0
    // Resource menu
    new KAction( i18n( "Query resource..." ), 0, KShortcut(),
                 m_view, SLOT( queryResource() ), actionCollection(), "query_resource" );
#endif

    // Resource usage menu
    m_resScaleAction = new KSelectAction( i18n( "&Scale" ), 0, actionCollection(), "ru_scale" );
    items.clear();
    items << i18n( "&Days" ) << i18n( "&Weeks" ) << i18n( "&Months" ) << i18n( "&Quarters" );
    m_resScaleAction->setItems( items );
    m_resScaleAction->setCurrentItem( 0 ); // TODO make configurable
    connect( m_resScaleAction, SIGNAL( activated( int ) ),
             m_view, SLOT( slotResScale( int ) ) );

    m_ruDisplayAction = new KSelectAction( i18n( "&Display" ), 0, actionCollection(), "ru_display" );
    items.clear();
    items << i18n( "Load and &total load" ) << i18n( "&Load" ) << i18n( "&Available load" );
    m_ruDisplayAction->setItems( items );
    m_ruDisplayAction->setCurrentItem( 0 );
    connect( m_ruDisplayAction, SIGNAL( activated( int ) ),
             m_view, SLOT( slotResDisplay( int ) ) );

    m_ruFindAction = new KAction( i18n( "&Find..." ), "find", KStdAccel::find(),
                                  m_view, SLOT( slotRUFind() ), actionCollection(), "ru_find" );

    // Sidebar
    m_sidebarInfo = new KRadioAction( i18n( "Info" ), "projectinfo", KShortcut(), actionCollection(), "sidebar_info" );
    m_sidebarInfo->setExclusiveGroup( "sidebar" );
    connect( m_sidebarInfo, SIGNAL( activated() ), this, SLOT( slotSidebarInfo() ) );

    m_sidebarGantt = new KRadioAction( i18n( "Gantt" ), "gantt", KShortcut(), actionCollection(), "sidebar_gantt" );
    m_sidebarGantt->setExclusiveGroup( "sidebar" );
    connect( m_sidebarGantt, SIGNAL( activated() ), this, SLOT( slotSidebarGantt() ) );

    m_sidebarResources = new KRadioAction( i18n( "Resources" ), "resources", KShortcut(), actionCollection(), "sidebar_resources" );
    m_sidebarResources->setExclusiveGroup( "sidebar" );
    connect( m_sidebarResources, SIGNAL( activated() ), this, SLOT( slotSidebarResources() ) );

    m_sidebarTasks = new KRadioAction( i18n( "Tasks" ), "tasks", KShortcut(), actionCollection(), "sidebar_tasks" );
    m_sidebarTasks->setExclusiveGroup( "sidebar" );
    connect( m_sidebarTasks, SIGNAL( activated() ), this, SLOT( slotSidebarTasks() ) );

    m_sidebarResUsage = new KRadioAction( i18n( "Usage" ), "resourceusage",
                                          KShortcut(), actionCollection(), "sidebar_res_usage" );
    m_sidebarResUsage->setExclusiveGroup( "sidebar" );
    connect( m_sidebarResUsage, SIGNAL( activated() ), this, SLOT( slotSidebarResUsage() ) );

    connect( m_view, SIGNAL( signalSwitchView( int ) ), this, SLOT( slotSwitchView( int ) ) );

    setStandardToolBarMenuEnabled( true );
    createStandardStatusBarAction();

    createGUI();

    setupSidebar();             // must be after createGUI()
}

void ktjview2::setupSidebar()
{
    KToolBar * sidebar = toolBar( "sidebar" );

    if ( sidebar )
    {
        sidebar->setMovingEnabled( false );
        sidebar->setEnableContextMenu( false );
    }
}

void ktjview2::saveProperties( KConfig *config )
{
    // the 'config' object points to the session managed
    // config file.  anything you write here will be available
    // later when this app is restored

    if ( !m_view->currentURL().isEmpty() )
    {
#if KDE_IS_VERSION(3,1,3)
        config->writePathEntry( "lastURL", m_view->currentURL().url() );
#else
        config->writeEntry( "lastURL", m_view->currentURL().url() );
#endif
    }
}

void ktjview2::readProperties( KConfig *config )
{
    // the 'config' object points to the session managed
    // config file.  this function is automatically called whenever
    // the app is being restored.  read in here whatever you wrote
    // in 'saveProperties'

    QString url = config->readPathEntry( "lastURL" );
    if ( !url.isEmpty() )
        m_view->openURL( KURL::fromPathOrURL( url ) );
}

void ktjview2::dragEnterEvent( QDragEnterEvent *event )
{
    // accept uri drops only
    event->accept( KURLDrag::canDecode( event ) );
}

void ktjview2::dropEvent( QDropEvent *event )
{
    // this is a very simplistic implementation of a drop event.  we
    // will only accept a dropped URL.  the Qt dnd code can do *much*
    // much more, so please read the docs there
    KURL::List urls;

    // see if we can decode a URI.. if not, just ignore it
    if ( KURLDrag::decode( event, urls ) && !urls.isEmpty() )
    {
        load( urls.first() );
    }
}

void ktjview2::fileNew()
{
    // create a new window
    (new ktjview2)->show();
}

void ktjview2::fileOpen()
{
    KURL url = KFileDialog::getOpenURL( ":projects", "*.tjp *.tjx *.tji|" + i18n( "Taskjuggler files (*.tjp *.tjx *.tji)" ),
                                        this, i18n( "to open ...", "Open Project" ) );
    if ( !url.isEmpty() )
        load( url );
}

void ktjview2::filePrint()
{
    KPrinter * printer = new KPrinter();

    // do some printer initialization
    printer->setFullPage( true );

    // initialize the printer using the print dialog
    if ( printer->setup( this ) )
    {
        m_view->print( printer );
    }

    delete printer;
}

void ktjview2::optionsConfigureKeys()
{
    KKeyDialog::configure( actionCollection() );
}

void ktjview2::optionsConfigureToolbars()
{
    saveMainWindowSettings( KGlobal::config(), autoSaveGroup() );
    KEditToolbar dlg( actionCollection() );
    connect( &dlg, SIGNAL( newToolbarConfig() ), this, SLOT( newToolbarConfig() ) );
    dlg.exec();
}

void ktjview2::newToolbarConfig()
{
    createGUI();
    applyMainWindowSettings( KGlobal::config(), autoSaveGroup() );
}

void ktjview2::optionsPreferences()
{
    if ( KConfigDialog::showDialog( "settings" ) )
        return;

    KConfigDialog *dialog = new KConfigDialog( this, "settings", Settings::self() );
    //dialog->addPage(new General(0, "General"), i18n("General"), "package_settings");
    dialog->addPage( new Gantt(0, "Gantt"), i18n("Gantt"), "gantt" );
    dialog->addPage( new ResUsageCfg(0, "ResUsageCfg"), i18n("Resource Usage"), "resourceusage" );
    connect(dialog, SIGNAL(settingsChanged()), m_view, SLOT(loadSettings()));
    dialog->show();
}

void ktjview2::changeStatusbar( const QString& text )
{
    // display the text on the statusbar
    statusBar()->message( text );
}

bool ktjview2::queryExit()
{
    m_recentAction->saveEntries( kapp->config() );
    return true;
}

void ktjview2::slotSidebarInfo()
{
    toolBar( "filterToolBar" )->hide();
    toolBar( "ganttToolBar" )->hide();
    enableGanttActions( false );
    enableTasksActions( false );
    enableResourceActions( false );
    enableResUsageActions( false );
    m_view->activateView( ID_VIEW_INFO );
}

void ktjview2::slotSidebarGantt()
{
    toolBar( "filterToolBar" )->hide();
    toolBar( "ganttToolBar" )->show();
    enableGanttActions( true );
    enableTasksActions( false );
    enableResourceActions( false );
    enableResUsageActions( false );
    m_view->activateView( ID_VIEW_GANTT );
}

void ktjview2::slotSidebarResources()
{
    m_quickSearch->reset();
    m_quickSearch->setListView( m_view->resListView() );
    toolBar( "filterToolBar" )->show();
    toolBar( "ganttToolBar" )->hide();
    enableGanttActions( false );
    enableTasksActions( false );
    enableResourceActions( true );
    enableResUsageActions( false );
    m_view->activateView( ID_VIEW_RESOURCES );
}

void ktjview2::slotSidebarTasks()
{
    m_quickSearch->reset();
    m_quickSearch->setListView( m_view->taskListView() );
    toolBar( "filterToolBar" )->show();
    toolBar( "ganttToolBar" )->hide();
    enableGanttActions( false );
    enableTasksActions( true );
    enableResourceActions( false );
    enableResUsageActions( false );
    m_view->activateView( ID_VIEW_TASKS );
}

void ktjview2::slotSidebarResUsage()
{
    toolBar( "filterToolBar" )->hide();
    toolBar( "ganttToolBar" )->hide();
    enableGanttActions( false );
    enableTasksActions( false );
    enableResourceActions( false );
    enableResUsageActions( true );
    m_view->activateView( ID_VIEW_RES_USAGE );
}

void ktjview2::slotSwitchView( int type )
{
    if ( type == ID_VIEW_INFO )
        m_sidebarInfo->activate();
    else if ( type == ID_VIEW_GANTT )
        m_sidebarGantt->activate();
    else if ( type == ID_VIEW_RESOURCES )
        m_sidebarResources->activate();
    else if ( type == ID_VIEW_TASKS )
        m_sidebarTasks->activate();
    else if ( type == ID_VIEW_RES_USAGE )
        m_sidebarResUsage->activate();
}

void ktjview2::setCalendarMode()
{
    m_view->setCalendarMode( m_calendarAction->isChecked() );
}

void ktjview2::slotFilterForTasks()
{
    int id = m_filterForTasksAction->currentItem();

    if ( m_view->filterForTasks( id ) )
    {
        m_filterForTasksAction->setText( i18n( "&Filter for: %1" ).arg( m_taskFilterItems[id] ) );
        m_activeTaskFilter = id;
        statusBar()->message( i18n( "Task list filtered for '%1'" ).arg( m_taskFilterItems[id] ), 2000 );
    }
    else
        m_filterForTasksAction->setCurrentItem( m_activeTaskFilter );
}

void ktjview2::slotFilterForResources()
{
    int id = m_filterForResourcesAction->currentItem();

    if ( m_view->filterForResources( id ) )
    {
        m_filterForResourcesAction->setText( i18n( "&Filter for: %1" ).arg( m_resourceFilterItems[id] ) );
        m_activeResourceFilter = id;
        statusBar()->message( i18n( "Resource list filtered for '%1'" ).arg( m_resourceFilterItems[id] ), 2000 );
    }
    else
        m_filterForResourcesAction->setCurrentItem( m_activeResourceFilter );
}

void ktjview2::enableGanttActions( bool enable )
{
    m_scaleAction->setEnabled( enable );
    m_calendarAction->setEnabled( enable );
    action( "zoom_in" )->setEnabled( enable );
    action( "zoom_out" )->setEnabled( enable );
    action( "fit_to_page" )->setEnabled( enable );
    action( "timeframe" )->setEnabled( enable );
}

void ktjview2::enableTasksActions( bool enable )
{
    m_filterForTasksAction->setEnabled( enable );
}

void ktjview2::enableResourceActions( bool enable )
{
    m_filterForResourcesAction->setEnabled( enable );
}

void ktjview2::enableResUsageActions( bool enable )
{
    m_resScaleAction->setEnabled( enable );
    m_ruFindAction->setEnabled( enable );
    m_ruDisplayAction->setEnabled( enable );
}

void ktjview2::expandAll()
{
    m_view->expandToLevel( m_view->resListView() );
}

void ktjview2::outline1()
{
    m_view->expandToLevel( m_view->resListView(), 0 );
}

void ktjview2::outline2()
{
    m_view->expandToLevel( m_view->resListView(), 1 );
}

void ktjview2::outline3()
{
    m_view->expandToLevel( m_view->resListView(), 2 );
}

void ktjview2::outline4()
{
    m_view->expandToLevel( m_view->resListView(), 3 );
}

void ktjview2::outline5()
{
    m_view->expandToLevel( m_view->resListView(), 4 );
}

void ktjview2::outline6()
{
    m_view->expandToLevel( m_view->resListView(), 5 );
}

void ktjview2::outline7()
{
    m_view->expandToLevel( m_view->resListView(), 6 );
}

void ktjview2::outline8()
{
    m_view->expandToLevel( m_view->resListView(), 7 );
}

void ktjview2::outline9()
{
    m_view->expandToLevel( m_view->resListView(), 8 );
}

void ktjview2::collapseAll()
{
    m_view->collapseAll( m_view->resListView() );
}

#include "ktjview2.moc"
