 // -*- Mode: C++; c-basic-offset: 4; indent-tabs-mode: nil; tab-width: 4; -*-
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

// Qt includes
#include <qpainter.h>
#include <qlayout.h>
#include <qwidgetstack.h>
#include <qcolor.h>
#include <qtextbrowser.h>
#include <qpaintdevicemetrics.h>
#include <qdialog.h>
#include <qdir.h>

// KDE includes
#include <kurl.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kglobal.h>
#include <kicontheme.h>
#include <kiconloader.h>
#include <kdebug.h>
#include <kio/netaccess.h>
#include <kprinter.h>
#include <klistview.h>

// local includes
#include "koKoolBar.h"
#include "kdgantt/KDGanttView.h"
#include "kdgantt/KDGanttViewEventItem.h"
#include "kdgantt/KDGanttViewSummaryItem.h"
#include "kdgantt/KDGanttViewTaskItem.h"
#include "kdgantt/KDGanttViewTaskLink.h"
#include "ktjview2view.h"
#include "timedialog.h"
#include "TaskItem.h"
#include "settings.h"
#include "filterDialog.h"

// TJ includes
#include "XMLFile.h"
#include "ProjectFile.h"
#include "TaskList.h"
#include "CoreAttributesList.h"
#include "Task.h"
#include "Resource.h"
#include "taskjuggler.h"
#include "CoreAttributes.h"
#include "UsageLimits.h"
#include "Allocation.h"

ktjview2View::ktjview2View( QWidget *parent )
    : DCOPObject( "ktjview2Iface" ), QWidget( parent ), m_project( 0 )
{
    m_project = new Project();

    // setup our layout manager to automatically add our widgets
    QHBoxLayout *top_layout = new QHBoxLayout( this );

    // --- side bar
    m_koolBar = new KoKoolBar( this, "kool_bar" );
    m_koolBar->setFixedWidth( 80 );
    mainGroup = m_koolBar->insertGroup( i18n( "Project" ) );
    infoPage = m_koolBar->insertItem( mainGroup, KGlobal::iconLoader()->loadIcon( "gohome", KIcon::Desktop ),
                                      i18n( "Info" ), this, SLOT( slotKoolBar( int, int ) ) );
    ganttPage = m_koolBar->insertItem( mainGroup, KGlobal::iconLoader()->loadIcon( "gantt", KIcon::Desktop ),
                                       i18n( "Gantt" ), this, SLOT( slotKoolBar( int, int ) ) );
    resPage = m_koolBar->insertItem( mainGroup, KGlobal::iconLoader()->loadIcon( "resources", KIcon::Desktop ),
                                     i18n( "Resources" ), this, SLOT( slotKoolBar( int, int ) ) );
    tasksPage = m_koolBar->insertItem( mainGroup, KGlobal::iconLoader()->loadIcon( "tasks", KIcon::Desktop ),
                                       i18n( "Tasks" ), this, SLOT( slotKoolBar( int, int ) ) );
    top_layout->addWidget( m_koolBar );

    // --- right view (stacked)
    m_widgetStack = new QWidgetStack( this, "widget_stack" );
    top_layout->addWidget( m_widgetStack, 1 );

    // info page
    m_textBrowser = new QTextBrowser( this, "text_browser" );
    m_widgetStack->addWidget( m_textBrowser );

    // gantt chart
    m_ganttView = new KDGanttView( this, "gantt_view" );
    m_ganttView->setEditorEnabled( false );
    m_ganttView->setEditable( false );
    m_ganttView->setScale( KDGanttView::Auto );
    //m_ganttView->setCalendarMode( true );
    m_ganttView->setDisplaySubitemsAsGroup( false );
    bool use12Clock = KGlobal::locale()->use12Clock();
    if ( use12Clock )
        m_ganttView->setHourFormat( KDGanttView::Hour_12 );
    else
        m_ganttView->setHourFormat( KDGanttView::Hour_24 );
    m_ganttView->setShowTaskLinks( true );
    m_ganttView->setShowHeaderPopupMenu( true ); //### TODO implement thru KActions
    m_ganttView->setShowLegendButton( false ); // ### TODO legend?
    //m_ganttView->setShowTimeTablePopupMenu( true );
    m_ganttView->setWeekendDays( 6, 7 );
    connect( m_ganttView, SIGNAL( itemDoubleClicked( KDGanttViewItem * ) ),
             this, SLOT( ensureItemVisible( KDGanttViewItem * ) ) );
    m_widgetStack->addWidget( m_ganttView );

    // resources list view
    m_resListView = new KListView( this, "resources_view" );
    m_resListView->addColumn( i18n( "ID" ) );
    m_resListView->addColumn( i18n( "Name" ) );
    m_resListView->addColumn( i18n( "Rate" ) );
    m_resListView->addColumn( i18n( "Efficiency" ) );
    m_resListView->addColumn( i18n( "Min. Effort" ) );
    m_resListView->addColumn( i18n( "Daily Max" ) );
    m_resListView->addColumn( i18n( "Weekly Max" ) );
    m_resListView->addColumn( i18n( "Monthly Max" ) );
    m_resListView->setShowSortIndicator( true );
    m_resListView->setSortColumn( 1 ); // sort by name by default
    m_resListView->setAllColumnsShowFocus( true );
    m_resListView->setRootIsDecorated( true );
    m_widgetStack->addWidget( m_resListView );

    // task list view
    m_taskView = new KListView( this, "task_view" );
    m_taskView->addColumn( i18n( "ID" ) );
    m_taskView->addColumn( i18n( "Name" ) );
    m_taskView->addColumn( i18n( "Start" ) );
    m_taskView->addColumn( i18n( "End" ) );
    m_taskView->addColumn( i18n( "Duration" ) );
    m_taskView->addColumn( i18n( "Priority" ) );
    m_taskView->addColumn( i18n( "Completion" ) );
    m_taskView->addColumn( i18n( "Status" ) );
    m_taskView->addColumn( i18n( "Responsible" ) );
    m_taskView->addColumn( i18n( "Effort" ) );
    m_taskView->setShowSortIndicator( true );
    m_taskView->hideColumn( 0 ); // hide the ID column
    m_taskView->setSortColumn( 2 ); // sort by start date by default
    m_taskView->setAllColumnsShowFocus( true );
    m_widgetStack->addWidget( m_taskView );

    m_textBrowser->setText( i18n( "<h1>No Project Loaded</h1><p>Choose 'File -> Open...' to select one.</p>" ) );
    m_widgetStack->raiseWidget( m_textBrowser );

    loadSettings();             // load the config-dialog related settings

    // ### TODO setup popup menus
}

ktjview2View::~ktjview2View()
{
    delete m_project;
    m_project = 0;
}

void ktjview2View::print( KPrinter * printer )
{
    // print the Gantt chart
    // set orientation to landscape
    printer->setOrientation( KPrinter::Landscape );

    // now we have a printer to print on
    QPainter p( printer );
    // get the paper metrics
    QPaintDeviceMetrics m = QPaintDeviceMetrics( printer );
    float dx, dy;
    // get the size of the desired output for scaling.
    // here we want to print all: ListView, TimeLine, and Legend
    // for this purpose, we call drawContents() with a 0 pointer as painter
    QSize size = m_ganttView->drawContents( 0 ); // TODO customize parts

    // at the top, we want to print current time/date
    QString date = i18n( "Printing Time: %1" ).arg( QDateTime::currentDateTime().toString() );
    int hei = p.boundingRect(0,0, 5, 5, Qt::AlignLeft, date ).height();
    p.drawText( 0, 0, date );

    // compute the scale
    dx = (float) m.width()  / (float)size.width();
    dy  = (float)(m.height() - ( 2 * hei )) / (float)size.height();
    float scale;
    // scale to fit the width or height of the paper
    if ( dx < dy )
        scale = dx;
    else
        scale = dy;
    // set the scale
    p.scale( scale, scale );
    // now printing with y offset:  2 hei
    p.translate( 0, 2*hei );
    m_ganttView->drawContents( &p ); // TODO customize parts
    // the drawContents() has the side effect, that the painter translation is
    // after drawContents() set to the bottom of the painted stuff
    // for instance a
    // p.drawText(0, 0, "printend");
    // would be painted directly below the paintout of drawContents()
    p.end();
}

KURL ktjview2View::currentURL() const
{
    return m_projectURL;
}

void ktjview2View::openURL( QString url )
{
    openURL( KURL::fromPathOrURL( url ) );
}

void ktjview2View::openURL( const KURL& url )
{
    kdDebug() << "Loading project from URL: " << url << endl;

    QString tmpFile;
    if ( !KIO::NetAccess::download( url, tmpFile, this ) )
        return;

    kdDebug() << "Project is in temp file: " << tmpFile << endl;

    if ( tmpFile.endsWith( ".tjx" ) ) // XML file
    {
        XMLFile* xf = new XMLFile( m_project );
        if ( !xf->readDOM( tmpFile, QDir::currentDirPath(), "", true ) )
        {
            delete xf;
            return;
        }
        xf->parse();
        delete xf;
    }
    else if ( tmpFile.endsWith( ".tjp" ) || tmpFile.endsWith( ".tji" ) ) // source file
    {
        ProjectFile* pf = new ProjectFile( m_project );
        if ( !pf->open( tmpFile, QDir::currentDirPath(), "", true ) )
        {
            delete pf;
            return;
        }
        pf->parse();
        delete pf;
    }
    else
    {
        KMessageBox::sorry( this, i18n( "This filetype is not supported." ) );
        return;
    }

    m_project->pass2( false );
    m_project->scheduleAllScenarios();
    //m_project->generateReports(); // FIXME do we need that?

    clearAllViews();

    KIO::NetAccess::removeTempFile( tmpFile );

    m_ganttView->setUpdateEnabled( false );
    parseProjectInfo();
    parseResources( m_project->getResourceListIterator() );
    parseTasks();
    parseGantt( m_project->getTaskListIterator() );
    parseLinks( m_project->getTaskListIterator() );

    m_ganttView->setUpdateEnabled( true );
    m_ganttView->setTimelineToStart();

    m_projectURL = url;
    signalChangeStatusbar( i18n( "Successfully loaded project %1" ).arg( m_projectURL.prettyURL() ) );
    signalChangeCaption( m_projectURL.fileName() );
}

void ktjview2View::slotKoolBar( int /*grp*/, int item )
{
    //kdDebug() << k_funcinfo << grp << " " << item << endl;

    emit signalUpdateToolbars( item );     // notify the view change

    if ( item == infoPage )
        m_widgetStack->raiseWidget( m_textBrowser );
    else if ( item == ganttPage )
        m_widgetStack->raiseWidget( m_ganttView );
    else if ( item == resPage )
        m_widgetStack->raiseWidget( m_resListView );
    else if ( item == tasksPage )
        m_widgetStack->raiseWidget( m_taskView );
}

void ktjview2View::parseProjectInfo()
{
    QString text;

    // general info
    text += QString( "<h1>%1 (%2)</h1>" ).arg( m_project->getName() ).arg( m_project->getId() );
    text += i18n( "Version: %1<br>" ).arg( m_project->getVersion() );
    text += i18n( "Currency: %1<br>" ).arg( m_project->getCurrency() );

    // project start
    m_ganttView->setHorizonStart( time_t2Q( m_project->getStart()) );
    text += i18n( "Project start: %1<br>" ).arg( time_t2QS( m_project->getStart() ) );

    // end date
    m_ganttView->setHorizonEnd( time_t2Q( m_project->getEnd() ) );
    text += i18n( "Project end: %1<br>" ).arg( time_t2QS( m_project->getEnd() ) );

    // TJ current date
    text += i18n( "XML report generated: %1<br>" ).arg( time_t2QS( m_project->getNow() ) );

    text += "<hr>";

    text += m_project->getCopyright();

    m_textBrowser->setText( text );
}

void ktjview2View::parseTasks( int sc )
{
    TaskListIterator it = m_project->getTaskListIterator();
    Task * task;

    while ( ( task = dynamic_cast<Task *>( it.current() ) ) != 0 )
    {
        ++it;

        TaskItem * item = new TaskItem( m_taskView, task->getId(), time_t2Q( task->getStart( sc ) ), time_t2Q( task->getEnd( sc ) ) );
        item->setText( 1, task->getName() );
        item->setText( 2, KGlobal::locale()->formatDateTime( item->startDate() ) );
        item->setText( 3, KGlobal::locale()->formatDateTime( item->endDate() ) );
        item->setText( 4, KGlobal::locale()->formatNumber( task->getCalcDuration( sc ), 0 ) );
        item->setText( 5, QString::number( task->getPriority() ) );
        item->setText( 6, QString( "%1%" ).arg( KGlobal::locale()->formatNumber( task->getCompletionDegree( sc ), 2 ) ) );
        item->setText( 7, status2Str( task->getStatus( sc ) ) );

        Resource * resp = task->getResponsible();
        if ( resp )
            item->setText( 8, resp->getName() );

        item->setText( 9, KGlobal::locale()->formatNumber( task->getEffort( sc ) ) );
        // TODO add allocation info
    }
}

void ktjview2View::parseGantt( TaskListIterator it, int sc )
{
    Task * task;

    while ( ( task = static_cast<Task *>( it.current() ) ) != 0 )
    {
        ++it;

        const QString id = task->getId();

        if ( KDGanttViewItem::find( id ) ) // seen this task, go on
            continue;

        kdDebug() << "Parsing gantt item: " << id << endl;

        const QString taskName = task->getName();
        QDateTime start = time_t2Q( task->getStart( sc ) );
        QDateTime end = time_t2Q( task->getEnd( sc ) );
        QString duration = KGlobal::locale()->formatNumber( task->getCalcDuration( sc ), 1 );
        //int prio = task->getPriority();

        QString toolTip;

        bool isRoot = task->isRoot();
        bool isContainer = task->isContainer();
        bool isLeaf = task->isLeaf();

        if ( isRoot ) // toplevel container task
        {
            KDGanttViewSummaryItem * item = new KDGanttViewSummaryItem( m_ganttView, taskName, id );
            item->setStartTime( start );
            item->setEndTime( end );
            toolTip = i18n( "Task group: %1\nStart: %2\nEnd: %3" )
                      .arg( taskName )
                      .arg( KGlobal::locale()->formatDateTime( start ) )
                      .arg( KGlobal::locale()->formatDateTime( end ) ) ;
            item->setTooltipText( toolTip );
            item->setText( taskName + " " +i18n( "(%1 d)" ).arg( duration ) ) ;

            parseGantt( task->getSubListIterator() ); // recurse
        }
        else if ( isContainer ) // non-toplevel container task
        {
            KDGanttViewItem * parentItem = KDGanttViewItem::find( task->getParent()->getId() );

            KDGanttViewSummaryItem * item = new KDGanttViewSummaryItem( parentItem, taskName, id );
            item->setStartTime( start );
            item->setEndTime( end );
            toolTip = i18n( "Task group: %1\nStart: %2\nEnd: %3" )
                      .arg( taskName )
                      .arg( KGlobal::locale()->formatDateTime( start ) )
                      .arg( KGlobal::locale()->formatDateTime( end ) ) ;
            item->setTooltipText( toolTip );
            item->setText( taskName + " " +i18n( "(%1 d)" ).arg( duration ) );

            parseGantt( task->getSubListIterator() ); // recurse
        }
//#if 0
        else if ( isLeaf )   // terminal (leaf) task
        {
            kdDebug() << "Parent ID: " << task->getParent()->getId() << endl;

            KDGanttViewItem * parentItem = KDGanttViewItem::find( task->getParent()->getId() );

            if ( task->isMilestone() )  // milestone
            {
                KDGanttViewEventItem * item = new KDGanttViewEventItem( parentItem, taskName, id );
                item->setStartTime( start );

                toolTip = i18n( "Milestone: %1\nDate: %2" )
                          .arg( taskName )
                          .arg( KGlobal::locale()->formatDateTime( start ) );
                item->setTooltipText( toolTip );
                item->setText( taskName );
            }
            else                // task
            {
                KDGanttViewTaskItem * item = new KDGanttViewTaskItem( parentItem, taskName, id );
                item->setStartTime( start );
                item->setEndTime( end );

                toolTip = i18n( "Task: %1\nStart: %2\nEnd: %3\nAllocations: %4" )
                          .arg( taskName )
                          .arg( KGlobal::locale()->formatDateTime( start ) )
                          .arg( KGlobal::locale()->formatDateTime( end ) )
                          .arg( formatAllocations( task ) );
                item->setTooltipText( toolTip );
                item->setText( taskName  + " " +i18n( "(%1 d)" ).arg( duration ) );
            }
        }
//#endif
        else
        {
            kdWarning() << "Unsupported task type with ID: " << id << endl;
            continue;             // uhoh, something bad happened
        }

        kdDebug() << "Done parsing gantt item: " << id << endl;
    }

    KDGanttViewItem * root = m_ganttView->firstChild(); // expand the root item
    if ( root )
        root->setOpen( true );
}

void ktjview2View::parseResources( ResourceListIterator it, KListViewItem * parentItem )
{
    Resource * res;

    while ( ( res = static_cast<Resource *>( it.current() ) ) != 0 )
    {
        ++it;

        const QString id = res->getId();

        if ( m_resListView->findItem( id, 0 ) ) // been there, seen that, go on :)
            continue;           // FIXME speed this up

        const QString rate = KGlobal::locale()->formatMoney( res->getRate(), m_project->getCurrency() );
        const QString name = res->getName();
        const QString eff = KGlobal::locale()->formatNumber( res->getEfficiency() );
        const QString minEffort = KGlobal::locale()->formatNumber( res->getMinEffort() );
        const UsageLimits * limits = res->getLimits();
        QString dailyMax = "0";
        QString weeklyMax = "0";
        QString monthlyMax = "0";

        if ( limits )
        {
            dailyMax = QString::number( limits->getDailyMax() );
            weeklyMax = QString::number( limits->getWeeklyMax() );
            monthlyMax = QString::number( limits->getMonthlyMax() );
        }

        if ( res->isGroup() && ( res->getParent() == 0 ) ) // toplevel group item
        {
            //kdDebug() << "Case1: " << id << endl;
            KListViewItem * item = new KListViewItem( m_resListView, id, name, rate, eff, minEffort,
                                                      dailyMax, weeklyMax, monthlyMax );
            item->setOpen( true );
            parseResources( res->getSubListIterator(), item );
        }
        else if ( res->isGroup() && ( res->getParent() != 0 ) && parentItem ) // group item, non-toplevel
        {
            //kdDebug() << "Case2: " << id << endl;
            KListViewItem * item = new KListViewItem( parentItem, id, name, rate, eff, minEffort,
                                                      dailyMax, weeklyMax, monthlyMax );
            item->setOpen( true );
            parseResources( res->getSubListIterator(), item );
        }
        else if ( parentItem )                   // leaf item
        {
            //kdDebug() << "Case3: " << id << endl;
            ( void ) new KListViewItem( parentItem, id, name, rate, eff, minEffort,
                                        dailyMax, weeklyMax, monthlyMax );
        }
        else if ( ( res->getParent() == 0 ) || ( parentItem == 0 ) ) // standalone item
        {
            //kdDebug() << "Case4: " << id << endl;
            ( void ) new KListViewItem( m_resListView, id, name, rate, eff, minEffort,
                                        dailyMax, weeklyMax, monthlyMax );
        }
        else
        {
            kdWarning() << "Unsupported resource type with ID: " << id << endl;
            continue;             // uhoh, something bad happened
        }
    }
}

void ktjview2View::parseLinks( TaskListIterator it )
{
    Task * task;

    while ( ( task = static_cast<Task *>( it.current() ) ) != 0 )
    {
        ++it;

        TaskListIterator depIt = task->getDependsIterator();
        Task * depTask;
        QPtrList<KDGanttViewItem> fromList;   // ### auto delete?
        //fromList.setAutoDelete( true );
        while ( ( depTask = static_cast<Task *>( depIt.current() ) ) != 0 )
        {
            ++depIt;

            QString depId = depTask->getId();
            //kdDebug() << "Got depId: " << depId << endl;
            fromList.append( KDGanttViewItem::find( depId ) );
        }

        if ( fromList.isEmpty() )
            continue;

        QString toId = task->getId();
        QPtrList<KDGanttViewItem> toList; // ### auto delete?
        //kdDebug() << "Got toId: " << toId << endl;
        //toList.setAutoDelete( true );
        toList.append( KDGanttViewItem::find( toId ) );

        KDGanttViewTaskLink * taskLink = new KDGanttViewTaskLink( fromList, toList );
        //taskLink->setTooltipText( fromName  + " -> " + toName );
    }
}

void ktjview2View::ensureItemVisible( KDGanttViewItem * item )
{
    if ( item )
    {
        //kdDebug() << "Centering on item: " << item->name() << endl;
        m_ganttView->ensureVisible( item );
        m_ganttView->center( item );
        m_ganttView->centerTimeline( item->startTime() );
    }
}

void ktjview2View::zoomIn()
{
    m_ganttView->setZoomFactor( 2.0, false );
}

void ktjview2View::zoomOut()
{
    m_ganttView->setZoomFactor( 0.5, false );
}

void ktjview2View::zoomFit()
{
    m_ganttView->zoomToFit();
}

void ktjview2View::slotScale( int scale )
{
    m_ganttView->setScale( ( KDGanttView::Scale ) scale );
}

void ktjview2View::slotZoomTimeframe()
{
    TimeDialog * dlg = new TimeDialog( this, m_ganttView->horizonStart(), m_ganttView->horizonEnd() );
    if ( dlg->exec() == QDialog::Accepted )
    {
        m_ganttView->zoomToSelection( dlg->getStartDate(), dlg->getEndDate() );
    }
    delete dlg;
}

QString ktjview2View::status2Str( int ts ) const
{
    switch ( ( TaskStatus ) ts )
    {
    case NotStarted:
        return i18n( "Not started" );
        break;
    case InProgressLate:
        return i18n( "In progress (late)" );
        break;
    case InProgress:
        return i18n( "In progress" );
        break;
    case OnTime:
        return i18n( "On time" );
        break;
    case InProgressEarly:
        return i18n( "In progress (early)" );
        break;
    case Finished:
        return i18n( "Finished" );
        break;
    case Undefined:
    default:
        return i18n( "Undefined" );
        break;
    }
}

#if 0
void ktjview2View::queryResource()
{
    //TODO res->isAvailable, isAllocated, getCredits, getLoad/getAvailableWorkload/getCurrentLoad, hasVacationDay

    QListViewItem * curResItem = m_resListView->currentItem();

    Resource * res;

    if ( curResItem )
        res = m_project->getResource( curResItem->text( 0 ) );
    else
        res = 0;

    ResourceQueryDialog * dlg = new ResourceQueryDialog( m_project, res, this );
    dlg->exec();
    delete dlg;
    dlg = 0;
}
#endif

void ktjview2View::setupGantt()
{
    for ( int i = 0; i < 7; i++ )
    {
        m_ganttView->setWeekdayBackgroundColor( Settings::weekdayColor(), i );
    }
    m_ganttView->setWeekendBackgroundColor( Settings::weekendColor() );
    if ( Settings::bgLines() )
        m_ganttView->setHorBackgroundLines();
    else
        m_ganttView->setHorBackgroundLines( 0 );
    m_ganttView->update();      // repaint the widget (necessary)
}

void ktjview2View::loadSettings()
{
    setupGantt();
    // setup other (future) config options
}

QDateTime ktjview2View::time_t2Q( time_t secs ) const
{
    QDateTime result;
    result.setTime_t( secs );
    return result;
}

QString ktjview2View::time_t2QS( time_t secs ) const
{
    return KGlobal::locale()->formatDateTime( time_t2Q( secs ) );
}


void ktjview2View::filter()
{
    FilterDialog * dlg = new FilterDialog( this, "filter_dlg" );
    dlg->exec();
    delete dlg;
    dlg = 0;
}

void ktjview2View::clearAllViews()
{
    m_ganttView->clear();
    m_resListView->clear();
    m_taskView->clear();
}

QString ktjview2View::formatAllocations( Task* task )
{
    QStringList result;

    for ( ResourceListIterator tli(task->getBookedResourcesIterator(0)); *tli != 0; ++tli )
    {
        Resource* res = (*tli);
        result.append( res->getName() );
    }

    return result.join( ", " );
}

#include "ktjview2view.moc"
