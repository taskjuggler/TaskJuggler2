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
#include <qlistview.h>
#include <qdatetime.h>
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
#include <kfilterdev.h>
#include <kprinter.h>

// local includes
#include "koKoolBar.h"
#include "kdgantt/KDGanttView.h"
#include "kdgantt/KDGanttViewEventItem.h"
#include "kdgantt/KDGanttViewSummaryItem.h"
#include "kdgantt/KDGanttViewTaskItem.h"
#include "kdgantt/KDGanttViewTaskLink.h"
#include "ktjview2view.h"
#include "timedialog.h"

// TJ includes
#include "XMLFile.h"
#include "ProjectFile.h"


ktjview2View::ktjview2View(QWidget *parent)
    : DCOPObject("ktjview2Iface"), QWidget(parent)
{
    // setup our layout manager to automatically add our widgets
    QHBoxLayout *top_layout = new QHBoxLayout(this);

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
    m_ganttView->setWeekendBackgroundColor( Qt::lightGray );
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
    m_resListView = new QListView( this, "resources_view" );
    m_resListView->addColumn( i18n( "ID" ) );
    m_resListView->addColumn( i18n( "Name" ) );
    m_resListView->addColumn( i18n( "Group" ) );
    m_resListView->addColumn( i18n( "Working hours" ) );
    m_resListView->setShowSortIndicator( true );
    m_resListView->setSortColumn( 1 ); // sort by name by default
    m_widgetStack->addWidget( m_resListView );

    // task table
    m_taskView = new QListView( this, "task_view" );
    m_taskView->addColumn( i18n( "Name" ) );
    m_taskView->addColumn( i18n( "Start" ) );
    m_taskView->addColumn( i18n( "End" ) );
    m_taskView->addColumn( i18n( "Duration" ) );
    m_taskView->addColumn( i18n( "Priority" ) );
    m_taskView->addColumn( i18n( "Completion" ) );
    m_taskView->addColumn( i18n( "Cost" ) );
    m_taskView->addColumn( i18n( "Status" ) );
    m_taskView->addColumn( i18n( "Responsible" ) );
    m_taskView->setShowSortIndicator( true );
    m_taskView->setSortColumn( 1 ); // sort by start date by default
    m_widgetStack->addWidget( m_taskView );

    m_textBrowser->setText( i18n( "<h1>No Project Loaded</h1><p>Choose 'File -> Open...' to select one.</p>" ) );
    m_widgetStack->raiseWidget( m_textBrowser );
}

ktjview2View::~ktjview2View()
{
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
        XMLFile* xf = new XMLFile( &m_project );
        if ( !xf->readDOM( tmpFile, QDir::currentDirPath(), "", true ) )
        {
            delete xf;
            return;
        }
        xf->parse();
        delete xf;
    }
    else if ( tmpFile.endsWith( ".tjp" ) ) // source file
    {
        ProjectFile* pf = new ProjectFile( &m_project );
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

    m_project.scheduleAllScenarios();
    m_project.generateReports();

    KIO::NetAccess::removeTempFile( tmpFile );

    m_ganttView->setUpdateEnabled( false );
    parseProjectInfo();
    //parseResources( m_dom.documentElement().namedItem( "resourceList" ), m_resListView );
    //parseTasks( m_dom.documentElement().namedItem( "taskList" ) );
    //parseLinks( m_dom.documentElement().namedItem( "taskList" ).toElement(), m_ganttView );
    m_ganttView->setUpdateEnabled( true );
    m_ganttView->setTimelineToStart();

    m_projectURL = url;
    signalChangeStatusbar( i18n( "Successfully loaded project %1" ).arg( m_projectURL.prettyURL() ) );
    signalChangeCaption( m_projectURL.fileName() );
}

void ktjview2View::slotKoolBar( int grp, int item )
{
    //kdDebug() << k_funcinfo << grp << " " << item << endl;
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
    text += QString( "<h1>%1 (%2)</h1>" ).arg( m_project.getName() ).arg( m_project.getId() );
    text += i18n( "Version: %1<br>" ).arg( m_project.getVersion() );
    text += i18n( "Currency: %1<br>" ).arg( m_project.getCurrency() );

    // project start
    m_ganttView->setHorizonStart( time_t2Q( m_project.getStart()) );
    text += i18n( "Project start: %1<br>" ).arg( time_t2QS( m_project.getEnd() ) );

    // end date
    m_ganttView->setHorizonEnd( time_t2Q( m_project.getEnd() ) );
    text += i18n( "Project end: %1<br>" ).arg( time_t2QS( m_project.getEnd() ) );

    // TJ current date
    text += i18n( "XML report generated: %1<br>" ).arg( time_t2QS( m_project.getNow() ) );

    m_textBrowser->setText( text );
}

void ktjview2View::parseTasks( QDomNode node, KDGanttViewItem * parent )
{
    for ( QDomNode task = node.firstChild(); !task.isNull(); task = task.nextSibling() )
    {
        QDomElement elem = task.toElement();

        if ( !task.namedItem( "task" ).isNull() ) // it's a "summary" task, recurse
        {
            // add a container task
            KDGanttViewSummaryItem * summary = new KDGanttViewSummaryItem( m_ganttView, elem.attribute( "name" ),
                                                                           elem.attribute( "id" ) );
            summary->setPriority( elem.attribute( "priority" ).toInt() );

            QDomElement scenario = elem.namedItem( "taskScenario" ).toElement();

            QDateTime start = time_t2Q( scenario.namedItem( "start" ).toElement().text().toUInt() );
            QDateTime end = time_t2Q( scenario.namedItem( "end" ).toElement().text().toUInt() );

            summary->setStartTime( start );
            summary->setEndTime( end );
            //summary->setActualEndTime( end );

            parseTasks( task, summary );
            continue;
        }

        if ( elem.tagName() == "task" )
        {
            parseTask( elem, parent );  // add a normal task
        }
    }
}

void ktjview2View::parseTask( const QDomElement & taskElem, KDGanttViewItem * parent )
{
    kdDebug() << "Parsing task: " << taskElem.attribute( "id" ) << endl;

    bool isMilestone = ( taskElem.attribute( "milestone", "0" ).toInt() == 1 );

    QDomElement scenario = taskElem.namedItem( "taskScenario" ).toElement();

    QString taskName = taskElem.attribute( "name" );
    QDateTime start = time_t2Q( scenario.namedItem( "start" ).toElement().text().toUInt() );
    QDateTime end = time_t2Q( scenario.namedItem( "end" ).toElement().text().toUInt() );

    QString toolTip;

    if ( isMilestone )
    {
        KDGanttViewEventItem * event = new KDGanttViewEventItem( parent, taskName,
                                                                 taskElem.attribute( "id" ) );
        event->setPriority( taskElem.attribute( "priority" ).toInt() );
        event->setStartTime( start );
        event->setText( taskName );
        toolTip = i18n( "Milestone: %1\nDate: %2" )
                  .arg( taskName )
                  .arg( KGlobal::locale()->formatDateTime( start ) );
        event->setTooltipText( toolTip );
    }
    else
    {
        KDGanttViewTaskItem * task = new KDGanttViewTaskItem( parent, taskName,
                                                              taskElem.attribute( "id" ) );
        task->setPriority( taskElem.attribute( "priority" ).toInt() );
        task->setStartTime( start );
        task->setEndTime( end );
        task->setText( taskName );
        toolTip = i18n( "Task: %1\nStart: %2\nEnd: %3" )
                  .arg( taskName )
                  .arg( KGlobal::locale()->formatDateTime( start ) )
                  .arg( KGlobal::locale()->formatDateTime( end ) ) ;
        task->setTooltipText( toolTip );
    }
}

void ktjview2View::parseResources( QDomNode node, QListView * view, const QString & group )
{
    for ( QDomNode res = node.firstChild(); !res.isNull(); res = res.nextSibling() )
    {
        QDomElement elem = res.toElement();

        if ( !res.namedItem( "resource" ).isNull() ) // it's a "container" resource, recurse
        {
            // add a group resource
            QString aGroup = elem.attribute( "name" );
            parseResources( res, view, aGroup );
            continue;
        }

        if ( elem.tagName() == "resource" )
        {
            parseResource( elem, view, group );  // adds a normal resource
        }
    }
}

void ktjview2View::parseResource( const QDomElement & resElem, QListView * view, const QString & group )
{
    kdDebug() << "Parsing resource: " << resElem.attribute( "id" ) << endl;

    // TODO parse <workingHours>
    // TODO parse <vacationList>

    new QListViewItem( view, resElem.attribute( "id" ), resElem.attribute( "name" ), group );
}

void ktjview2View::parseLinks( const QDomElement & taskList, KDGanttView * view )
{
    // ### perhaps get all <task> elements separately and parse <depends> individually
    // to be able to insert links into groups
    QDomNodeList dependencies = taskList.elementsByTagName( "depends" );

    for ( uint i = 0; i < dependencies.count(); i++ )
    {
        QDomElement depElem = dependencies.item( i ).toElement();

        QString fromName = depElem.text();
        KDGanttViewItem * fromItem = view->getItemByName( fromName );

        QString toName = depElem.parentNode().toElement().attribute( "id" );
        KDGanttViewItem * toItem = view->getItemByName( toName );

        kdDebug() << "Got dependency from: " << fromName << " to: " << toName << endl;

        KDGanttViewTaskLink * taskLink = new KDGanttViewTaskLink( fromItem, toItem );
        taskLink->setVisible( true );
        taskLink->setTooltipText( fromName  + " -> " + toName );
    }
}

QDateTime ktjview2View::time_t2Q( time_t secs )
{
    QDateTime result;
    result.setTime_t( secs );
    return result;
}

QString ktjview2View::time_t2QS( time_t secs )
{
    return KGlobal::locale()->formatDateTime( time_t2Q( secs ) );
}

void ktjview2View::ensureItemVisible( KDGanttViewItem * item )
{
    //kdDebug() << "Centering on item: " << item->name() << endl;
    m_ganttView->ensureVisible( item );
    m_ganttView->center( item );
    m_ganttView->centerTimeline( item->startTime() );
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

#include "ktjview2view.moc"
