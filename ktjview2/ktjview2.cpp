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
    connect(m_view, SIGNAL(signalChangeStatusbar(const QString&)),
            this,   SLOT(changeStatusbar(const QString&)));
    connect(m_view, SIGNAL(signalChangeCaption(const QString&)),
            this,   SLOT(changeCaption(const QString&)));
}

ktjview2::~ktjview2()
{
}

void ktjview2::load( const KURL& url )
{
    m_view->openURL( url );
    m_recentAction->addURL( url );
}

void ktjview2::setupActions()
{
    KStdAction::openNew( this, SLOT(fileNew()), actionCollection() );
    KStdAction::open( this, SLOT(fileOpen()), actionCollection() );

    m_recentAction = KStdAction::openRecent( this, SLOT( load( const KURL& ) ), actionCollection() );
    m_recentAction->setMaxItems( 10 );
    m_recentAction->loadEntries( kapp->config() );

    KStdAction::print(this, SLOT(filePrint()), actionCollection());
    KStdAction::quit(kapp, SLOT(quit()), actionCollection());

    KStdAction::keyBindings( this, SLOT(optionsConfigureKeys()), actionCollection() );
    KStdAction::configureToolbars( this, SLOT(optionsConfigureToolbars()), actionCollection() );
    KStdAction::preferences( this, SLOT(optionsPreferences()), actionCollection() );

    new KAction( i18n( "Zoom In" ), "viewmag+", KStdAccel::shortcut( KStdAccel::ZoomIn ),
                 m_view, SLOT( zoomIn() ), actionCollection(), "zoom_in" );
    new KAction( i18n( "Zoom Out" ), "viewmag-", KStdAccel::shortcut( KStdAccel::ZoomOut ),
                 m_view, SLOT( zoomOut() ), actionCollection(), "zoom_out" );
    new KAction( i18n( "Fit To Page" ), "viewmagfit", KShortcut( "Ctrl+0" ),
                 m_view, SLOT( zoomFit() ), actionCollection(), "fit_to_page" );
    new KAction( i18n( "Set Timeframe" ), "timespan", KShortcut(),
                 m_view, SLOT( slotZoomTimeframe() ), actionCollection(), "timeframe" );

    m_scaleAction = new KSelectAction( i18n( "Scale" ), 0, actionCollection(), "scale" );
    QStringList items = QStringList();
    items << i18n( "Minute" ) << i18n( "Hour" ) << i18n( "Day" ) <<
        i18n( "Week" ) << i18n( "Month" ) << i18n( "Auto" );
    m_scaleAction->setItems( items );
    m_scaleAction->setCurrentItem( 5 ); // TODO make configurable
    connect( m_scaleAction, SIGNAL( activated( int ) ),
             m_view, SLOT( slotScale( int ) ) );

    setStandardToolBarMenuEnabled( true );
    createStandardStatusBarAction();

    createGUI();
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

    m_recentAction->loadEntries( config );
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
        // okay, we have a URI.. process it
        const KURL &url = urls.first();

        // load in the file
        load( url );
    }
}

void ktjview2::fileNew()
{
    // create a new window
    (new ktjview2)->show();
}

void ktjview2::fileOpen()
{
    KURL url = KFileDialog::getOpenURL( ":projects", "*.tjp *.tjx|" + i18n( "Taskjuggler files (*.tjp *.tjx)" ),
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
    //connect(dialog, SIGNAL(settingsChanged()), this, SLOT(loadSettings()));
    dialog->show();
}

void ktjview2::changeStatusbar( const QString& text )
{
    // display the text on the statusbar
    statusBar()->message( text );
}

void ktjview2::changeCaption( const QString& text )
{
    // display the text on the caption
    setCaption( text );
}

bool ktjview2::queryExit()
{
    m_recentAction->saveEntries( kapp->config() );
    return true;
}

#include "ktjview2.moc"
