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

#define PROFILE               // profiling enabled if defined

// Qt includes
#include <qpainter.h>
#include <qlayout.h>
#include <qwidgetstack.h>
#include <qcolor.h>
#include <qtextbrowser.h>
#include <qpaintdevicemetrics.h>
#include <qdialog.h>
#include <qdir.h>
#include <qpopupmenu.h>
#include <qptrlist.h>

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
#include <kapplication.h>
#include <kmainwindow.h>
#include <kprogress.h>

#include <ktexteditor/clipboardinterface.h>
#include <ktexteditor/undointerface.h>
#include <ktexteditor/selectioninterface.h>
#include <ktexteditor/markinterface.h>

// local includes
#include "kdgantt/KDGanttViewEventItem.h"
#include "kdgantt/KDGanttViewSummaryItem.h"
#include "kdgantt/KDGanttViewTaskItem.h"
#include "kdgantt/KDGanttViewTaskLink.h"
#include "ktjview2view.h"
#include "ktjview2.h"
#include "timedialog.h"
#include "TaskItem.h"
#include "ResourceItem.h"
#include "settings.h"
#include "filterDialog.h"
#include "selectDialog.h"
#include "resUsageView.h"
#include "editorView.h"
#include "ktjUtils.h"
#include "reportView.h"
#include "ktjTaskReport.h"

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

#ifdef PROFILE
int pr_INFO = 0;
int pr_GANTT = 0;
int pr_GANTT_LINKS = 0;
int pr_RES = 0;
int pr_TASKS = 0;
int pr_RU = 0;
#include "kprofilemethod.h"
#endif

ktjview2View::ktjview2View( QWidget *parent )
    : DCOPObject( "ktjview2Iface" ), QWidget( parent ), m_project( 0 ), m_taskReport( 0 )
{
    // setup our layout manager to automatically add our widgets
    QHBoxLayout *top_layout = new QHBoxLayout( this );

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
    m_ganttView->setDisplaySubitemsAsGroup( false );
    bool use12Clock = KGlobal::locale()->use12Clock();
    if ( use12Clock )
        m_ganttView->setHourFormat( KDGanttView::Hour_12 );
    else
        m_ganttView->setHourFormat( KDGanttView::Hour_24 );
    m_ganttView->setShowTaskLinks( true );
    m_ganttView->setShowHeaderPopupMenu( true ); // ### TODO implement thru KActions
    m_ganttView->setShowLegendButton( true );
    //m_ganttView->setShowTimeTablePopupMenu( true );  // not useful, we can't add or manipulate items
    m_ganttView->setWeekendDays( 6, 7 );
    connect( m_ganttView, SIGNAL( lvItemDoubleClicked( KDGanttViewItem * ) ),
             this, SLOT( ensureItemVisible( KDGanttViewItem * ) ) );
    connect( m_ganttView, SIGNAL( lvContextMenuRequested ( KDGanttViewItem *, const QPoint &, int ) ),
             this, SLOT( popupGanttItemMenu( KDGanttViewItem *, const QPoint &, int ) ) );
    connect( m_ganttView, SIGNAL( rescaling( KDGanttView::Scale ) ),
             this, SLOT( slotScaleChanged( KDGanttView::Scale ) ) );
    m_widgetStack->addWidget( m_ganttView );
    m_ganttView->show();

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
    m_taskView->addColumn( i18n( "Allocations" ) );
    m_taskView->setShowSortIndicator( true );
    m_taskView->setSortColumn( 2 ); // sort by start date by default
    m_taskView->setAllColumnsShowFocus( true );
    m_widgetStack->addWidget( m_taskView );

    m_textBrowser->setText( i18n( "<h1>No Project Loaded</h1><p>Choose 'File -> Open...' to select one.</p>" ) );
    m_widgetStack->raiseWidget( m_textBrowser );

    // resource usage view
    m_resUsageView = new ResUsageView( this, "res_usage_view" );
    m_widgetStack->addWidget( m_resUsageView );

    // report view
    m_reportView = new ReportView( this, "report_view" );
    m_widgetStack->addWidget( m_reportView );

    // editor view
    m_editorView = new EditorView( this, "editor_view" );
    m_widgetStack->addWidget( m_editorView );
    connect( m_editorView->doc(), SIGNAL( setWindowCaption( const QString & ) ),
             this, SIGNAL( signalChangeCaption( const QString & ) ) );
    connect( m_editorView->doc(), SIGNAL( setStatusBarText( const QString & ) ),
             this, SIGNAL( signalChangeStatusbar( const QString & ) ) );

    // gantt popup menu
    m_ganttPopupMenu = new QPopupMenu( this, "gantt_popup" );
    m_ganttPopupMenu->insertItem( i18n( "Jump to task details" ), this, SLOT( slotJumpToTask() ) );

    loadSettings();             // load the config-dialog related settings
}

ktjview2View::~ktjview2View()
{
    delete m_project;
    delete m_ganttPopupMenu;
    delete m_taskReport;
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
    QString date = i18n( "Printing Time: %1" ).arg( KGlobal::locale()->formatDateTime( QDateTime::currentDateTime() ) );
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
    p.scale( scale, scale );    // TODO make it possible to disable the scaling, cut the printout into pieces instead
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

QString ktjview2View::projectURL() const
{
    return m_projectURL.url();
}

KURL ktjview2View::currentURL() const
{
    return m_projectURL;
}

void ktjview2View::openURL( QString url )
{
    openURL( KURL::fromPathOrURL( url ) );
}

bool ktjview2View::openURL( const KURL& url )
{
    //kdDebug() << "Loading project from URL: " << url << endl;

    // prompt the user to save the doc first, if modified
    if ( !m_editorView->doc()->closeURL() )
        return false;

    KProgressDialog progressDlg( this, "progress_dialog", i18n( "Loading project" ), QString::null, true );
    progressDlg.setMinimumDuration( 0 ); // no delay
    progressDlg.progressBar()->setTotalSteps( 6 );
    progressDlg.setAutoReset( true );
    progressDlg.setAllowCancel( false );

    QString tmpFile;
    if ( !KIO::NetAccess::download( url, tmpFile, this ) )
        return false;

    //kdDebug() << "Project is in temp file: " << tmpFile << endl;

    recreateProject();

    bool syntaxError = false;
    bool accessError = false;

    QString errorFile;
    int errorLine = 0;

    if ( tmpFile.endsWith( ".tjx" ) ) // XML file
    {
        XMLFile* xf = new XMLFile( m_project );
        progressDlg.progressBar()->setProgress( 1 );
        progressDlg.setLabel( i18n( "Opening XML project" ) );
        if ( !xf->readDOM( tmpFile, QDir::currentDirPath(), "", true ) )
        {
            accessError = true;
            progressDlg.cancel();
            KMessageBox::error( this, i18n( "Taskjuggler failed to open the project file. Please check your permissions." ) );
        }

        if ( !accessError )
        {
            progressDlg.progressBar()->setProgress( 2 );
            progressDlg.setLabel( i18n( "Parsing XML project" ) );
            //kdDebug() << "Parsing XML project" << endl;
            if ( !xf->parse() )
            {
                syntaxError = true;
                errorFile = tmpFile;
                progressDlg.cancel();
                KMessageBox::error( this, i18n( "Taskjuggler failed to parse the project file. "
                                                "Most probably there is a syntax error.\n"
                                                "You should edit the project and try to fix it.") );
            }
        }
        delete xf;
    }
    else if ( tmpFile.endsWith( ".tjp" ) || tmpFile.endsWith( ".tji" ) ) // source file
    {
        ProjectFile* pf = new ProjectFile( m_project );
        progressDlg.progressBar()->setProgress( 1 );
        progressDlg.setLabel( i18n( "Opening TJP project" ) );
        if ( !pf->open( tmpFile, QDir::currentDirPath(), "", true ) )
        {
            accessError = true;
            progressDlg.cancel();
            KMessageBox::error( this, i18n( "Taskjuggler failed to open the project file. Please check your permissions." ) );
        }
        //kdDebug() << "Parsing TJP project" << endl;
        if ( !accessError )
        {
            progressDlg.progressBar()->setProgress( 2 );
            progressDlg.setLabel( i18n( "Parsing TJP project" ) );

            if ( !pf->parse() )
            {
                syntaxError = true;
                errorFile = pf->getFile();
                errorLine = pf->getLine();
                progressDlg.cancel();
                KMessageBox::error( this, i18n( "Taskjuggler failed to parse the project file. Most probably there is a syntax error.\n"
                                                "You should edit the project and try to fix it.") );
            }
        }
        delete pf;
    }
    else
    {
        accessError = true;
        progressDlg.cancel();
        KMessageBox::sorry( this, i18n( "This filetype is not supported." ) );
    }

    KIO::NetAccess::removeTempFile( tmpFile );

    if ( accessError )
        return false;

    //kdDebug() << "Generating cross references (pass2)" << endl;
    progressDlg.progressBar()->setProgress( 3 );
    progressDlg.setLabel( i18n( "Generating cross references" ) );
    if ( !m_project->pass2( false ) )
    {
        syntaxError = true;
        progressDlg.cancel();
        //KMessageBox::error( this, i18n( "Taskjugggler failed to generate cross references on data structures." ) );
        //return false;
    }

    //kdDebug() << "Scheduling all scenarios " << endl;
    progressDlg.progressBar()->setProgress( 4 );
    progressDlg.setLabel( i18n( "Scheduling all scenarios" ) );
    if ( !m_project->scheduleAllScenarios() )
    {
        syntaxError = true;
        progressDlg.cancel();
        //KMessageBox::error( this, i18n( "Taskjugggler failed to schedule the scenarios." ) );
        //return false;
    }
    //m_project->generateReports(); // FIXME do we need that?

    clearAllViews();

    if ( !syntaxError )
    {
        progressDlg.progressBar()->setProgress( 5 );
        progressDlg.setLabel( i18n( "Building the views" ) );

        progressDlg.setLabel( i18n( "Parsing project info" ) );
        parseProjectInfo();

        progressDlg.setLabel( i18n( "Parsing resources" ) );
        parseResources( m_project->getResourceListIterator() );

        progressDlg.setLabel( i18n( "Parsing tasks" ) );
        parseTasks( m_project->getTaskListIterator() );

        progressDlg.setLabel( i18n( "Building the Gantt chart" ) );
        parseGantt( m_project->getTaskListIterator() );
        m_ganttView->sort();

        progressDlg.setLabel( i18n( "Building the Gantt chart links" ) );
        parseLinks( m_project->getTaskListIterator() );

        progressDlg.setLabel( i18n( "Building the Resource usage view" ) );
        parseResUsage();

#ifdef PROFILE
        PROFILE_METHOD_PRINT( pr_INFO, "Parsing info" );
        PROFILE_METHOD_PRINT( pr_RES, "Parsing resources" );
        PROFILE_METHOD_PRINT( pr_TASKS, "Parsing tasks" );
        PROFILE_METHOD_PRINT( pr_GANTT, "Parsing gantt" );
        PROFILE_METHOD_PRINT( pr_GANTT_LINKS, "Parsing gantt links" );
        PROFILE_METHOD_PRINT( pr_RU, "Parsing RU" );
#endif

        m_ganttView->setTimelineToStart();

        progressDlg.setLabel( i18n( "Loading the project source" ) );
        m_editorView->loadDocument( url );

        emit signalChangeStatusbar( i18n( "Successfully loaded project %1" ).arg( m_projectURL.prettyURL() ) );
    }
    else                        // errors to correct
    {
        m_editorView->loadDocument( errorFile );
        m_editorView->gotoLine( errorLine - 1 );
        KTextEditor::markInterface( m_editorView->doc() )->setMark( errorLine - 1, KTextEditor::MarkInterface::markType07 );
        emit signalSwitchView( ID_VIEW_EDITOR );

        emit signalChangeStatusbar( i18n( "Project %1 loaded with errors" ).arg( m_projectURL.prettyURL() ) );
    }

    m_projectURL = url;

    progressDlg.progressBar()->setProgress( 6 );

    return true;
}

void ktjview2View::parseProjectInfo()
{
#ifdef PROFILE
    PROFILE_METHOD_BEGIN( pr_INFO );
#endif

    QString text;

    // general info
    text += QString( "<h1>%1 (%2)</h1>" ).arg( m_project->getName() ).arg( m_project->getId() );
    text += i18n( "Version: %1<br>" ).arg( m_project->getVersion() );
    text += i18n( "Currency: %1<br>" ).arg( m_project->getCurrency() );

    // project start
    m_ganttView->setHorizonStart( KtjUtils::time_t2Q( m_project->getStart() ) );
    text += i18n( "Project start: %1<br>" ).arg( KtjUtils::time_t2QS( m_project->getStart() ) );

    // end date
    m_ganttView->setHorizonEnd( KtjUtils::time_t2Q( m_project->getEnd() ) );
    text += i18n( "Project end: %1<br>" ).arg( KtjUtils::time_t2QS( m_project->getEnd() ) );

    // TJ current date
    text += i18n( "Report date: %1<br>" ).arg( KtjUtils::time_t2QS( m_project->getNow() ) );

    text += "<hr>";

    text += m_project->getCopyright();

    m_textBrowser->setText( text );

#ifdef PROFILE
    PROFILE_METHOD_END( pr_INFO );
#endif
}

void ktjview2View::parseTasks( TaskListIterator it, int sc )
{
#ifdef PROFILE
    PROFILE_METHOD_BEGIN( pr_TASKS );
#endif

    Task * task;

    while ( ( task = static_cast<Task *>( it.current() ) ) != 0 )
    {
        ++it;

        TaskItem * item = new TaskItem( m_taskView, task->getId(), KtjUtils::time_t2Q( task->getStart( sc ) ),
                                        KtjUtils::time_t2Q( task->getEnd( sc ) ) );
        item->setText( 1, task->getName() );
        item->setText( 2, KGlobal::locale()->formatDateTime( item->startDate() ) );
        item->setText( 3, KGlobal::locale()->formatDateTime( item->endDate() ) );
        item->setText( 4, KGlobal::locale()->formatNumber( task->getCalcDuration( sc ), 0 ) );
        item->setText( 5, QString::number( task->getPriority() ) );
        item->setText( 6, i18n( "%1%" ).arg( KGlobal::locale()->formatNumber( task->getCompletionDegree( sc ), 2 ) ) );
        item->setText( 7, status2Str( task->getStatus( sc ) ) );

        Resource * resp = task->getResponsible();
        if ( resp )
            item->setText( 8, resp->getName() );

        item->setText( 9, KGlobal::locale()->formatNumber( task->getEffort( sc ) ) );
        item->setText( 10, formatAllocations( task ) );
    }

#ifdef PROFILE
    PROFILE_METHOD_END( pr_TASKS );
#endif
}

void ktjview2View::parseGantt( TaskListIterator it, int sc )
{
#ifdef PROFILE
    PROFILE_METHOD_BEGIN( pr_GANTT );
#endif

    Task * task;

    while ( ( task = static_cast<Task *>( it.current() ) ) != 0 )
    {
        ++it;

        const QString id = task->getId();

        //kdDebug() << "Parsing gantt item: " << id << endl;

        const QString taskName = task->getName();
        QDateTime start = KtjUtils::time_t2Q( task->getStart( sc ) );
        QDateTime end = KtjUtils::time_t2Q( task->getEnd( sc ) );
        const QString duration = KGlobal::locale()->formatNumber( task->getCalcDuration( sc ), 1 );
        //int prio = task->getPriority();

        QString toolTip;

        const bool isRoot = task->isRoot();
        const bool isContainer = task->isContainer();
        const bool isLeaf = task->isLeaf();
        const bool hasPrev = task->hasPrevious();

        KDGanttViewItem * prevItem = 0;
        if ( hasPrev )
            prevItem = KDGanttViewItem::find( static_cast<Task *>( task->getPreviousIterator().toLast() )->getId() );

        if ( isRoot ) // toplevel container task
        {
            KDGanttViewSummaryItem * item = new KDGanttViewSummaryItem( m_ganttView, taskName, id );
            item->setStartTime( start );
            item->setEndTime( end );
            toolTip = i18n( "<em>Task group: %1</em><br>Start: %2<br>End: %3" )
                      .arg( taskName )
                      .arg( KGlobal::locale()->formatDateTime( start ) )
                      .arg( KGlobal::locale()->formatDateTime( end ) ) ;
            item->setTooltipText( toolTip );
            item->setText( taskName + " " +i18n( "(%1 d)" ).arg( duration ) ) ;
        }
        else if ( isContainer ) // non-toplevel container task
        {
            KDGanttViewItem * parentItem = KDGanttViewItem::find( task->getParent()->getId() );

            KDGanttViewSummaryItem * item;
            if ( prevItem )
                item = new KDGanttViewSummaryItem( parentItem, prevItem, taskName, id );
            else
                item = new KDGanttViewSummaryItem( parentItem, taskName, id );

            item->setStartTime( start );
            item->setEndTime( end );
            toolTip = i18n( "<em>Task group: %1</em><br>Start: %2<br>End: %3" )
                      .arg( taskName )
                      .arg( KGlobal::locale()->formatDateTime( start ) )
                      .arg( KGlobal::locale()->formatDateTime( end ) ) ;
            item->setTooltipText( toolTip );
            item->setText( taskName + " " +i18n( "(%1 d)" ).arg( duration ) );
        }
        else if ( isLeaf )   // terminal (leaf) task
        {
            //kdDebug() << "Parent ID: " << task->getParent()->getId() << endl;

            KDGanttViewItem * parentItem = KDGanttViewItem::find( task->getParent()->getId() );

            if ( task->isMilestone() )  // milestone
            {
                KDGanttViewEventItem * item;
                if ( prevItem )
                    item = new KDGanttViewEventItem( parentItem, prevItem, taskName, id );
                else
                    item = new KDGanttViewEventItem( parentItem, taskName, id );

                item->setStartTime( start );

                toolTip = i18n( "<em>Milestone: %1</em><br>Date: %2" )
                          .arg( taskName )
                          .arg( KGlobal::locale()->formatDateTime( start ) );
                item->setTooltipText( toolTip );
                item->setText( taskName );
            }
            else                // task
            {
                KDGanttViewTaskItem * item;
                if ( prevItem )
                    item = new KDGanttViewTaskItem( parentItem, prevItem, taskName, id );
                else
                    item = new KDGanttViewTaskItem( parentItem, taskName, id );

                item->setStartTime( start );
                item->setEndTime( end );

                toolTip = i18n( "<em>Task: %1</em><br>Start: %2<br>End: %3<br>Completion: %4%<br>Allocations: %5" )
                          .arg( taskName )
                          .arg( KGlobal::locale()->formatDateTime( start ) )
                          .arg( KGlobal::locale()->formatDateTime( end ) )
                          .arg( KGlobal::locale()->formatNumber( task->getCompletionDegree( sc ), 2 ) )
                          .arg( formatAllocations( task ) );
                item->setTooltipText( toolTip );
                item->setText( taskName  + " " +i18n( "(%1 d)" ).arg( duration ) );
            }
        }
        else
        {
            kdWarning() << "Unsupported task type with ID: " << id << endl;
            continue;             // uhoh, something bad happened
        }

        //kdDebug() << "Done parsing gantt item: " << id << endl;
    }

    KDGanttViewItem * root = m_ganttView->firstChild(); // expand the root item
    if ( root )
        root->setOpen( true );

#ifdef PROFILE
    PROFILE_METHOD_END( pr_GANTT );
#endif
}

void ktjview2View::parseResources( ResourceListIterator it )
{
#ifdef PROFILE
    PROFILE_METHOD_BEGIN( pr_RES );
#endif

    Resource * res;

    while ( ( res = static_cast<Resource *>( it.current() ) ) != 0 )
    {
        ++it;

        const QString id = res->getId();

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

        const bool isRoot = res->isRoot();
        const bool isGroup = res->isGroup();
        const bool isLeaf = res->isLeaf();

        ResourceItem * item;
        if ( isRoot )
            item = new ResourceItem( m_resListView, id );
        else if ( isGroup || isLeaf )
        {
            KListViewItem * parentItem = static_cast<KListViewItem *>( m_resListView->findItem( res->getParent()->getId(), 0 ) );
            item = new ResourceItem( parentItem, id );
        }
        else
        {
            kdWarning() << "Unexpected resource type with ID: " << id << endl;
            continue;
        }
        item->setText( 1, name );
        item->setText( 2, rate );
        item->setText( 3, eff );
        item->setText( 4, minEffort );
        item->setText( 5, dailyMax );
        item->setText( 6, weeklyMax );
        item->setText( 7, monthlyMax );
        item->setOpen( true );
    }

#ifdef PROFILE
    PROFILE_METHOD_END( pr_RES );
#endif
}

void ktjview2View::parseLinks( TaskListIterator it )
{
#ifdef PROFILE
    PROFILE_METHOD_BEGIN( pr_GANTT_LINKS );
#endif

    Task * task;

    while ( ( task = static_cast<Task *>( it.current() ) ) != 0 )
    {
        ++it;

        TaskListIterator depIt = task->getDependsIterator();

        if ( depIt.isEmpty() )
            continue;

        Task * depTask;
        QPtrList<KDGanttViewItem> fromList;
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
        QPtrList<KDGanttViewItem> toList;
        //kdDebug() << "Got toId: " << toId << endl;
        toList.append( KDGanttViewItem::find( toId ) );

        KDGanttViewTaskLink * taskLink = new KDGanttViewTaskLink( fromList, toList );
        taskLink->setTooltipText( formatLinks( fromList, toList ) );
    }

#ifdef PROFILE
    PROFILE_METHOD_END( pr_GANTT_LINKS );
#endif
}

void ktjview2View::ensureItemVisible( KDGanttViewItem * item )
{
    if ( item )
    {
        //kdDebug() << "Centering on item: " << item->name() << endl;
        m_ganttView->ensureVisible( item );
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
    TimeDialog dlg( this, m_ganttView->horizonStart(), m_ganttView->horizonEnd() );
    if ( dlg.exec() == QDialog::Accepted )
    {
        m_ganttView->zoomToSelection( dlg.getStartDate(), dlg.getEndDate() );
    }
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
    // setup bg colors
    for ( int i = 0; i < 7; i++ )
    {
        m_ganttView->setWeekdayBackgroundColor( Settings::weekdayColor(), i );
    }

    m_ganttView->setWeekendBackgroundColor( Settings::weekendColor() );

    // setup bg hatching
    if ( Settings::bgLines() )
        m_ganttView->setHorBackgroundLines();
    else
        m_ganttView->setHorBackgroundLines( 0 );

    // setup item colors
    m_ganttView->setDefaultColor( KDGanttViewItem::Task, Settings::taskColor() );
    m_ganttView->setDefaultColor( KDGanttViewItem::Summary, Settings::groupColor() );
    m_ganttView->setDefaultColor( KDGanttViewItem::Event, Settings::milestoneColor() );

    m_ganttView->setColors( KDGanttViewItem::Task, Settings::taskColor(), Settings::taskColor(), Settings::taskColor() );
    m_ganttView->setColors( KDGanttViewItem::Summary, Settings::groupColor(), Settings::groupColor(), Settings::groupColor() );
    m_ganttView->setColors( KDGanttViewItem::Event, Settings::milestoneColor(), Settings::milestoneColor(), Settings::milestoneColor() );

    // setup legend
    KDGanttViewItem::Shape taskShape;
    m_ganttView->shapes( KDGanttViewItem::Task, taskShape, taskShape, taskShape );
    KDGanttViewItem::Shape summaryShape;
    m_ganttView->shapes( KDGanttViewItem::Summary, summaryShape, summaryShape, summaryShape );
    KDGanttViewItem::Shape eventShape;
    m_ganttView->shapes( KDGanttViewItem::Event, eventShape, eventShape, eventShape );

    //m_ganttView->clearLegend();  //FIXME doesn't work :(

    m_ganttView->addLegendItem( taskShape, Settings::taskColor(), i18n( "Task" ) );
    m_ganttView->addLegendItem( summaryShape, Settings::groupColor(), i18n( "Task Group" ) );
    m_ganttView->addLegendItem( eventShape, Settings::milestoneColor(), i18n( "Milestone" ) );

    m_ganttView->update();      // repaint the widget (necessary)
}

void ktjview2View::loadSettings()
{
    setupGantt();
    m_resUsageView->updateContents();
    // TODO setup other (future) config options
}

void ktjview2View::filter()
{
    FilterDialog * dlg = new FilterDialog( FT_TASK, this, "filter_dlg" ); // FIXME
    dlg->exec();
    delete dlg;
    dlg = 0;
}

void ktjview2View::clearAllViews()
{
#if 0                           // FIXME: crashes when loading a second project
    m_ganttView->setUpdateEnabled( false );
    m_ganttView->taskLinks().clear();
    m_ganttView->clear();
    m_ganttView->setUpdateEnabled( true );
#endif
    m_resListView->clear();

    m_taskView->clear();

    m_resUsageView->clear();

    delete m_taskReport;
    m_taskReport = 0;
}

QString ktjview2View::formatAllocations( Task* task ) const
{
    QStringList result;

    for ( ResourceListIterator tli( task->getBookedResourcesIterator( 0 ) ); *tli != 0; ++tli )
    {
        Resource* res = ( *tli );
        result.append( res->getName() );
    }

    return result.join( ", " );
}

QString ktjview2View::formatLinks( const QPtrList<KDGanttViewItem> & from, const QPtrList<KDGanttViewItem> & to ) const
{
    QString result = i18n( "<em>Task Link</em>" ) + "<br>" + i18n( "From: " );

    QPtrListIterator<KDGanttViewItem> fromIt( from );
    KDGanttViewItem * item;
    QStringList fromLinks;
    while ( ( item = fromIt.current() ) != 0 )
    {
        ++fromIt;
        fromLinks.append( item->text() );
    }
    result += fromLinks.join( ", " );

    result += "<br>" + i18n( "To: " );

    QPtrListIterator<KDGanttViewItem> toIt( to );
    item = 0;
    QStringList toLinks;
    while ( ( item = toIt.current() ) != 0 )
    {
        ++toIt;
        toLinks.append( item->text() );
    }
    result += toLinks.join( ", " );

    return result;
}

void ktjview2View::setCalendarMode( bool flag )
{
    m_ganttView->setCalendarMode( flag );
    m_ganttView->setDisplaySubitemsAsGroup( flag );
    m_ganttView->setShowTaskLinks( !flag );
}

void ktjview2View::popupGanttItemMenu( KDGanttViewItem * item, const QPoint & pos, int /*col*/ )
{
    if ( item )
        m_ganttPopupMenu->popup( pos );
}

void ktjview2View::slotJumpToTask()
{
    KDGanttViewItem * sel = m_ganttView->selectedItem();
    if ( sel )
    {
        QListViewItem * item = m_taskView->findItem( sel->name(), 0 );
        if ( item )
        {
            emit signalSwitchView( ID_VIEW_TASKS );
            item->setVisible( true ); // might be hidden thru a filter
            m_taskView->setSelected( item, true );
            m_taskView->ensureItemVisible( item );
        }
    }
}

bool ktjview2View::filterForTasks( int id )
{
    QDateTime start, end;
    QStringList resultList;

    if ( id == 2 )              // date range dialog
    {
        TimeDialog dlg( this, KtjUtils::time_t2Q( m_project->getStart() ), KtjUtils::time_t2Q( m_project->getEnd() ) );
        dlg.setCaption( i18n( "Date Range" ) );
        if ( dlg.exec() != QDialog::Accepted )
            return false;
        start = dlg.getStartDate();
        end = dlg.getEndDate();
    }
    else if ( id == 6 )         // resource selection dialog
    {
        SelectDialog dlg( m_project->getResourceListIterator(), true, this );
        if ( dlg.exec() != QDialog::Accepted )
            return false;
        resultList = dlg.resultList();
    }
    else if ( id == 7 )         // flag selection dialog
    {
        SelectDialog dlg( m_project->getAllowedFlags().begin(), true, this );
        if ( dlg.exec() != QDialog::Accepted )
            return false;
        resultList = dlg.resultList();
    }

    //kdDebug() << "result list: " << resultList << endl;

    QListViewItemIterator it( m_taskView );

    bool showIt;

    while ( it.current() )
    {
        showIt = false;

        Task * task = m_project->getTask( static_cast<TaskItem *>( *it )->id() );

        if ( !task )
            return false;       // better bail out

        if ( id == 0 )          // All tasks
        {
            showIt = true;
        }
        if ( id == 1 )     // Completed tasks
        {
            showIt = ( task->getStatus(0) == Finished );
        }
        else if ( id == 2 )     // Date range
        {
            showIt = ( static_cast<TaskItem *>( *it )->startDate() >= start
                       && static_cast<TaskItem *>( *it )->endDate() <= end );
        }
        else if ( id == 3 )     // Incomplete tasks
        {
            showIt = ( task->getStatus( 0 ) != Finished );
        }
        else if ( id == 4 )     // Milestones
        {
            showIt = ( task->isMilestone() );
        }
        else if ( id == 5 )     // Summary (group) tasks
        {
            showIt = ( task->isContainer() );
        }
        else if ( id == 6 )     // Using resource
        {
            for ( QStringList::ConstIterator it = resultList.begin(); it != resultList.end(); ++it )
            {
                showIt = showIt || task->isDutyOf( 0, m_project->getResource( (*it) ) );
            }
        }
        else if ( id == 7 )     // Having Flag
        {
            for ( QStringList::ConstIterator it = resultList.begin(); it != resultList.end(); ++it )
            {
                showIt = showIt || task->hasFlag( (*it) );
            }
        }

        ( *it )->setVisible( showIt );
        ++it;
    }

    return true;
}

bool ktjview2View::filterForResources( int id )
{
    time_t start, end;
    time_t vacation;
    QStringList resultList;

    if ( id == 1 )              // on vacation
    {
        DateDialog dlg( this, QDate::currentDate() );
        if ( dlg.exec() != QDialog::Accepted )
            return false;
        vacation = QDateTime( dlg.getDate() ).toTime_t();
    }
    else if ( id == 2 || id == 3 || id == 4 )              // date range dialog
    {
        TimeDialog dlg( this, KtjUtils::time_t2Q( m_project->getStart() ), KtjUtils::time_t2Q( m_project->getEnd() ) );
        dlg.setCaption( i18n( "Date Range" ) );
        if ( dlg.exec() != QDialog::Accepted )
            return false;
        start = dlg.getStartDate().toTime_t();
        end = dlg.getEndDate().toTime_t();
    }
    else if ( id == 5 )         // flag selection dialog
    {
        SelectDialog dlg( m_project->getAllowedFlags().begin(), true, this );
        if ( dlg.exec() != QDialog::Accepted )
            return false;
        resultList = dlg.resultList();
    }

    bool showIt;

    QListViewItemIterator it( m_resListView );

    while ( it.current() )
    {
        showIt = false;

        Resource * res = m_project->getResource( static_cast<ResourceItem *>( *it )->id() );

        if ( !res )
            return false;

        if ( id == 0 )          // all
        {
            showIt = true;
        }
        else if ( id == 1 )     // on vacation
        {
            showIt = ( res->hasVacationDay( vacation ) );
        }
        else if ( id == 2 )     // on shift
        {
            showIt = ( res->isOnShift( Interval( start, end ) ) );
        }
        else if ( id == 3 )     // allocated
        {
            showIt = ( res->isAllocated( 0, Interval( start, end ) ) ); // FIXME scenario
        }
        else if ( id == 4 )     // unallocated
        {
            showIt = !( res->isAllocated( 0, Interval( start, end ) ) ); // FIXME scenario
        }
        else if ( id == 5 )     // having flag
        {
            for ( QStringList::ConstIterator it = resultList.begin(); it != resultList.end(); ++it )
            {
                showIt = showIt || res->hasFlag( ( *it ) );
            }
        }

        ( *it )->setVisible( showIt );
        ++it;
    }

    return true;
}

void ktjview2View::activateView( int id )
{
    if ( id == ID_VIEW_INFO )
        m_widgetStack->raiseWidget( m_textBrowser );
    else if ( id == ID_VIEW_GANTT )
        m_widgetStack->raiseWidget( m_ganttView );
    else if ( id == ID_VIEW_RESOURCES )
        m_widgetStack->raiseWidget( m_resListView );
    else if ( id == ID_VIEW_TASKS )
        m_widgetStack->raiseWidget( m_taskView );
    else if ( id == ID_VIEW_RES_USAGE )
        m_widgetStack->raiseWidget( m_resUsageView );
    else if ( id == ID_VIEW_REPORTS )
        m_widgetStack->raiseWidget( m_reportView );
    else if ( id == ID_VIEW_EDITOR )
        m_widgetStack->raiseWidget( m_editorView );
}

void ktjview2View::recreateProject()
{
    delete m_project;
    m_project = 0;
    m_project = new Project();
}

void ktjview2View::slotScaleChanged( KDGanttView::Scale scale )
{
    emit signalScaleChanged( static_cast<int>( scale ) );
}

void ktjview2View::expandToLevel( KListView * view, int level )
{
    QListViewItemIterator it( view, QListViewItemIterator::Visible );

    while ( it.current() )
    {
        if ( (*it)->depth() <= level )
            (*it)->setOpen( true );
        else
            (*it)->setOpen( false );

        ++it;
    }
}

void ktjview2View::collapseAll( KListView * view )
{
    QListViewItemIterator it( view, QListViewItemIterator::Visible );

    while ( it.current() )
    {
        (*it)->setOpen( false );

        ++it;
    }
}

void ktjview2View::parseResUsage()
{
#ifdef PROFILE
    PROFILE_METHOD_BEGIN( pr_RU );
#endif

    m_resUsageView->setProject( m_project );
    m_resUsageView->setStartDate( KtjUtils::time_t2Q( m_project->getStart() ) );
    m_resUsageView->setEndDate( KtjUtils::time_t2Q( m_project->getEnd() ) );
    m_resUsageView->assignResources( m_project->getResourceList() );

#ifdef PROFILE
    PROFILE_METHOD_END( pr_RU );
#endif
}

void ktjview2View::slotResScale( int scale )
{
    m_resUsageView->setScale( scale );
}

void ktjview2View::slotRUFind()
{
    m_resUsageView->find();
}

void ktjview2View::slotResDisplay( int display )
{
    m_resUsageView->setDisplayData( display );
}

void ktjview2View::slotCut()
{
    KTextEditor::clipboardInterface( m_editorView->view() )->cut();
}

void ktjview2View::slotCopy()
{
    KTextEditor::clipboardInterface( m_editorView->view() )->copy();
}

void ktjview2View::slotPaste()
{
    KTextEditor::clipboardInterface( m_editorView->view() )->paste();
}

void ktjview2View::slotUndo()
{
    KTextEditor::undoInterface( m_editorView->doc() )->undo();
}

void ktjview2View::slotRedo()
{
    KTextEditor::undoInterface( m_editorView->doc() )->redo();
}

void ktjview2View::slotBuild()
{
    openURL( m_projectURL );
}

void ktjview2View::slotSelectAll()
{
    KTextEditor::selectionInterface( m_editorView->doc() )->selectAll();
}

QString ktjview2View::projectName() const
{
    if ( m_project )
        return m_project->getName();
    else
        return QString::null;
}

void ktjview2View::slotTaskCoverage()
{
    if ( !m_project )
        return;

    if ( !m_taskReport )
        m_taskReport = new KTJTaskReport( m_project );
    kdDebug() << k_funcinfo << "Clearing the report" << endl;
    m_reportView->clear();
    kdDebug() << k_funcinfo << "Generating data model" << endl;
    QicsDataModelDefault * model = m_taskReport->generate();
    kdDebug() << k_funcinfo << QString( "Model is %1x%2" )
        .arg( model->numRows() ).arg( model->numColumns() ) << endl;
    m_reportView->setDataModel( model ); // FIXME freezes on third(!) invokation
    kdDebug() << k_funcinfo << "Setting row header" << endl;
    QicsDataModelColumn rowHeader = m_taskReport->rowHeader();
    kdDebug() << k_funcinfo << QString( "Row header has %1 rows" ).arg( rowHeader.count() )
              << endl;
    m_reportView->setRowHeader( rowHeader );
    kdDebug() << k_funcinfo << "Setting column header" << endl;
    QicsDataModelRow colHeader = m_taskReport->columnHeader();
    kdDebug() << k_funcinfo << QString( "Col header has %1 columns" ).arg( colHeader.count() )
              << endl;
    m_reportView->setColumnHeader( colHeader );
    //m_reportView->setTopTitleWidget( m_taskReport->description() ); // TODO
}

#include "ktjview2view.moc"
