/*
 * TaskJuggler Viewer
 *
 * Copyright (c) 2001, 2002 by Klaas Freitag <freitag@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */
#include <qdatetime.h>
#include <qheader.h>
#include <qtimer.h>

#include <kdebug.h>
#include <klocale.h>
#include <kglobal.h>
#include <kiconloader.h>

#include "ktvtasktable.h"
#include "ktvtaskcanvasview.h"
#include "Project.h"
#include "Task.h"
#include "Utility.h"
#include "ktvreport.h"

#define PIX_MILESTONE "flag"
#define PIX_TASK      "package_settings"
#define PIX_CONTAINER "attach"
#define PIX_PROJECT   "finish"
#define START_ITEM_HEIGHT 24   // TODO !
#define INITIAL_COL_WIDTH 60

/* need to inherit Report here because the sort functions are protected */

KTVTaskTable::KTVTaskTable( QWidget *parent, const char *name )
   :KListView( parent, name ),
    m_itemHeight(0),
    m_root(0),
    m_canvasView(0)
{
   // addColumn( i18n( "No." ));
   addColumn( i18n("Task" ));
   setSorting( -1, false );

   connect( this, SIGNAL( expanded( QListViewItem* )),
	    this, SLOT( slExpanded( QListViewItem* )));

   connect( this, SIGNAL( collapsed( QListViewItem* )),
	    this, SLOT( slCollapsed( QListViewItem* )));

   connect( this, SIGNAL( selectionChanged(QListViewItem*)),
	    this, SLOT(slSelectionChanged(QListViewItem*)));


   setVScrollBarMode( QScrollView::AlwaysOff );

   /* load pixmaps */
   KIconLoader *loader = KGlobal::iconLoader();
   m_milestonePix = loader->loadIcon( PIX_MILESTONE, KIcon::Small );
   m_taskPix      = loader->loadIcon( PIX_TASK     , KIcon::Small );
   m_containerPix = loader->loadIcon( PIX_CONTAINER, KIcon::Small );
   
   setMargin(0);
   setLineWidth(0);
   setItemMargin(8);
   createRoot();

   /* Fill the list of available columns */
   columnList.setAutoDelete(true);

   columnList.append( new TaskColumnType( i18n(COMPLETE),
					  i18n("The percentage the task has been completed already. This is either the value specified by complete or a value computed according to the current date (or the date specified by now) and the length, duration or effort."),
					  Int ));
   
   columnList.append( new TaskColumnType( i18n(COST),
					  i18n("The accumulated costs of the task and its sub tasks"),
					  Money ));
   
   columnList.append( new TaskColumnType( i18n(DEPEND),
					  i18n("The task index of the tasks on which this task depends"),
					  Tasks ));

   columnList.append( new TaskColumnType( i18n(PRECEDES),
					  i18n("The task index of the tasks on which this task depends"),
					  Tasks ));

   columnList.append( new TaskColumnType( i18n(PREVIOUS),
					  i18n("The task index of the tasks on which this task depends"),
					  Tasks ));

   columnList.append( new TaskColumnType( i18n(FOLLOWERS),
					  i18n("The task index of the tasks on which this task depends"),
					  Tasks ));

   columnList.append( new TaskColumnType( i18n(DURATION),
					  i18n("The duration of the task"),
					  Timespan ));

   columnList.append( new TaskColumnType( i18n(EFFORT),
					  i18n("The effort put into the task"),
					  Time ));

   columnList.append( new TaskColumnType( i18n(END),
					  i18n("The enddate of the task"),
					  TimeStamp ));

   columnList.append( new TaskColumnType( i18n(START),
					  i18n("The startdate of the task"),
					  TimeStamp ));

   setupColumns();
}


void KTVTaskTable::createRoot()
{
    if( m_root ) return;
    m_root = new KTVTaskTableItem( this, START_ITEM_HEIGHT ); //
    m_root->setExpandable( true );

    setItemHeight( m_root->height()+itemMargin() );
}


void KTVTaskTable::setCanvasView( KTVTaskCanvasView *view )

{
   m_canvasView = view;
}



void KTVTaskTable::showProject( Project *p )
{
   QString pName = p->getName();

   createRoot();

   KIconLoader *loader = KGlobal::iconLoader();
   m_root->setPixmap( COL_NAME, loader->loadIcon( PIX_MILESTONE, KIcon::Small ));

   qDebug( " ++++++ Starting to show +++++++, itemHeight: %d", m_itemHeight);
   m_root->setText( COL_NAME, i18n("Project: %1").arg(pName) );

   qDebug( "Y-Pos of root: %d, height is %d", m_root->itemPos(), m_root->height() );

   KTVReport rep( p, QString("dummy"), p->getStart(), p->getEnd() );

   /* Tasklist from Project and Sorting */
   TaskList filteredList = p->getTaskList();

   rep.sortTaskList(filteredList);

   for (TaskListIterator tli(filteredList); *tli != 0; ++tli)
   {
      if( ((*tli)->getParent() == 0) && (*tli)->isContainer() )
      {
	 addTask( static_cast<KTVTaskTableItem*>(m_root), *tli );
      }
   }

   /* open the first child of the project */
   m_root->setOpen( true );
   QListViewItem *fi = m_root->firstChild();
   if( fi )
      fi->setOpen(true);
}

void KTVTaskTable::setupColumns()
{
    // remove all columns
    for( int i = 1; i < columns(); i++ )
    {
	removeColumn(i);
    }

    // add columns in new sequence
    QPtrListIterator<TaskColumnType> it(columnList);

    TaskColumnType *colType = 0;
    int column = 1;

    /* go through the list of possible columns. The columns that are
     * visible get a new column index
     */
    while( (colType = it.current()) != 0 )
    {
	++it;
	if( colType->isVisible() )
	{
	    QString name = colType->getName();
	    kdDebug() << "Setting Name " << name << " to col " << column << endl;
	    addColumn( colType->getName(), column );
	    setColumnWidth( column, INITIAL_COL_WIDTH );
	    colType->setColumn( column );
	    column++;
	}
	else
	{
	    colType->setColumn( -1 );
	}
    }
}


void KTVTaskTable::showTaskInTable( KTVTaskTableItem *ktv, Task *t, int scene)
{
    if( ! ktv) return;
    
    ktv->setText( COL_NAME, t->getName() );
    if( t->isContainer() )
	ktv->setPixmap( COL_NAME, m_containerPix );
    else if( t->isMilestone() )
	ktv->setPixmap( COL_NAME, m_milestonePix );
    else
	ktv->setPixmap( COL_NAME, m_taskPix );
    
    QPtrListIterator<TaskColumnType> it(columnList);
    TaskColumnType *colType = 0;
    /* go through the list of possible columns. The columns that are
     * visible get a new column index
     */
    while( (colType = it.current()) != 0 )
    {
	++it;
	if( colType->isVisible() )
	{
	    ktv->setText( colType->getColumn(),
			  colType->toString( t, scene ) );
	}
    }

#if 0
    ktv->setText( COL_ID, t->getId() );
    ktv->setText( COL_PLAN_LEN, beautyTimeSpan( t->getEnd(0),
						t->getStart(0) ));
    ktv->setText( COL_PRIORITY, QString::number( t->getPriority() ));

    double cmplt = t->getComplete(0);
    ktv->setText( COL_COMPLETE, cmplt == -1 ? i18n("iP"):  i18n("%1%").arg(cmplt));

    dt.setTime_t( t->getStart(0) );
    ktv->setText( COL_PLAN_START_DATE, KGlobal::locale()->formatDate( dt.date(), true ));
    ktv->setText( COL_PLAN_START_TIME, KGlobal::locale()->formatTime( dt.time(), false ));
   
    dt.setTime_t( t->getEnd(0) );
    ktv->setText( COL_PLAN_END_DATE, KGlobal::locale()->formatDate( dt.date(), true ));
    ktv->setText( COL_PLAN_END_TIME, KGlobal::locale()->formatTime( dt.time(), false ));
#endif
    
}


void KTVTaskTable::addTask( KTVTaskTableItem *parent, Task *t )
{
   if( ! t ) return;

   QString idx = QString::number(t->getIndex());
   QString name = t->getName();
   // qDebug( "Adding task: " + t->getId());
   QDateTime dt;

   KTVTaskTableItem *ktv =  new KTVTaskTableItem( parent, t, m_itemHeight );
   showTaskInTable(ktv, t, 0 );  // TODO Scenario !!!
   
   /* signal to create a new task in the canvas */
   emit( newTaskAdded( t, ktv));

   // int cnt = subTasks.count();
   // qDebug( "Amount of subtasks: " + QString::number(cnt) );

   // qDebug( "START: Subpackages for "+ t->getId());
   TaskListIterator tli(t->getSubListIterator());
   for (tli.toLast(); *tli != 0; --tli)
   {
      // qDebug( "Calling subtask " + st->getId() + " from " + t->getId() );
      if( (*tli)->getParent() == t )
	 addTask( ktv, *tli );
      // qDebug( "Calling subtask " + st->getId() + " from " + t->getId() + " <FIN>" );
   }
   // qDebug( "END: Subpackages for "+ t->getId());
   // qDebug( "Adding task: " + t->getId() + "<FIN>");

}


QString KTVTaskTable::beautyTimeSpan( time_t tStart, time_t tEnd ) const
{
   if( tStart == tEnd ) return( "-" );
   int daysBet = daysBetween( tStart>tEnd?tEnd:tStart, tStart<tEnd?tEnd:tStart );

   if( daysBet != 0 )
      return( i18n( "%1 days").arg( daysBet ));
   else
   {
      int hours = (tEnd-tStart) / 3600;
      return ( i18n( "%1 hours").arg( hours ) );
   }
}

void KTVTaskTable::setItemHeight( int h )
{
   m_itemHeight = h;
}


void KTVTaskTable::slCollapsed( QListViewItem *it )
{
   if( ! it ) return;

   int y = it->itemPos();
   int childCnt = it->childCount();

   if( childCnt > 0 )
   {
       qDebug("---- ### collapsed, childs: %d !", childCnt );
       QListViewItem* child = it->firstChild();

       while( child )
       {
           // if( child->childCount() > 0 && isOpen(child) )
           // {
           //    setOpen( child, false );
           // }

	   // Call recursivly to close all children
	   if( child->childCount() )
	       slCollapsed( child );
	   
	   // Note: The next signal is not connected internally.
           emit hideTaskByItem( static_cast<KTVTaskTableItem*>(child) );
           m_canvasView->getCanvas()->slHideTask(static_cast<KTVTaskTableItem*>(child));
           child = child->nextSibling();
       }

       m_canvasView->getCanvas()->slMoveItems(y + m_itemHeight, -1 * childCnt * m_itemHeight );
       emit moveItems( y + m_itemHeight, -1 * childCnt * m_itemHeight );
       slUpdateCanvas();
   }
}

void KTVTaskTable::slExpanded( QListViewItem* it)
{
    int y = it->itemPos();
    int childCnt = it->childCount();

    if( childCnt > 0 )
    {
        qDebug("---- ### expanded, childs: %d !", childCnt );

        // emit moveItems( y, childCnt * m_itemHeight );
        /* move items of the whole canvas to gain space for the new subproject */
        m_canvasView->getCanvas()->slMoveItems( y, childCnt*m_itemHeight );

        QListViewItem* child = it->firstChild();
        while( child )
        {
            m_canvasView->getCanvas()->slShowTask(static_cast<KTVTaskTableItem*>(child));

	    if( child->childCount() && child->isOpen() )
		slExpanded( child );
	    
            /* No longer connected to internal reasons, but maybe extern */
            emit showTaskByItem( static_cast<KTVTaskTableItem*>(child) );

            child = child->nextSibling();
        }
        slUpdateCanvas();
    }
}


void KTVTaskTable::slSelectionChanged( QListViewItem *it )
{
   if( ! it ) return;

   int y = it->itemPos();

   emit moveMarker( y- m_root->height() );

}


void KTVTaskTable::slUpdateCanvas()
{
   if( m_canvasView )
      m_canvasView->canvas()->update();
}

/*
 * reimplementation of resizeContents which additionally informs the
 * canvas of the new height of both scrollview contents. Both of the
 * contents need to be synced in height in order to provide one correct
 * scrollbar for both scrollviews.
 *
 */
void KTVTaskTable::resizeContents( int w, int h )
{
   QScrollView::resizeContents(w, h);
   if( m_canvasView && m_root )
   {
      int width = m_canvasView->contentsWidth();
      if( m_canvasView->contentsHeight() != h )
      {
	  // m_canvasView->resizeContents(width, h - m_itemHeight - itemMargin());
	 m_canvasView->resizeContents(width, h - m_root->height() );
      }
   }

}

int KTVTaskTable::headerHeight()
{
    int m = header()->height() + m_root->height(); // +2*itemMargin()-1;
    return( m );
}


int KTVTaskTable::rootItemHeight()
{
    if( m_root )
	return m_root->height();

    return 0;
}

void KTVTaskTable::clear()
{
    m_root = 0;
    QListView::clear();
}
