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
#include <kdebug.h>
#include <klocale.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <qbrush.h>
#include <qpen.h>
#include <qrect.h>
#include <qpainter.h>
#include <qlabel.h>
#include <qpixmap.h>
#include <qheader.h>

#include "ktvtaskcanvas.h"
#include "ktvtasktable.h"
#include "ktvheader.h"

#include "Project.h"
#include "Task.h"
#include "Utility.h"

#include "ktvcanvasitem.h"
#include <qfontmetrics.h>

#define STD_DAYWIDTH 20


KTVTaskCanvas::KTVTaskCanvas( QWidget *parent, KTVTaskTable* tab, KTVHeader *h, const char *name )
   :QCanvas( parent, name )
{
   m_taskTable = tab;
   m_header = h;
   m_canvasMarker = new QCanvasRectangle( this  );
   m_canvasMarker->hide();
   m_canvasMarker->move(0,0);
   m_canvasMarker->setPen( NoPen );

   m_itemFont.setFamily( "Helvetica [Cronyx]" );
   m_itemFont.setPointSize(8);


   m_dbgMark = new QCanvasLine( this );
   m_dbgMark->setPoints( 1, 0, 15, 0 );

   QColor markerCol = m_taskTable->colorGroup().highlight();
   m_canvasMarker->setBrush( m_taskTable->colorGroup().brush( QColorGroup::Highlight ));

   m_canvasMarker->setZ(-0.1);
   setDoubleBuffering( true ); // false );
   // resize( 400, 300);

   m_tasks.setAutoDelete(true);

   /* TODO: make visible again and synchronise with table in the same way the
    * table synchronises with this one.
    */

}




/* sets a pointer to the connected table. This class is friend of
 *  the table. */
void KTVTaskCanvas::setTable( KTVTaskTable *tab )
{
   m_taskTable = tab;
}


void KTVTaskCanvas::resize( int w, int h )
{
    QCanvas::resize( w, h);
    m_canvasMarker->setSize( w, m_taskTable->itemHeight());
}


void KTVTaskCanvas::drawBackground( QPainter &painter, const QRect & clip )
{
   // qDebug( "Drawing background" );
    if( m_header->startTime() == 0 || m_header->endTime() == 0 ) return;
    
   QCanvas::drawBackground( painter, clip );

   QBrush origBrush = painter.brush();

   /* take the start day of the project, calculate midnight
    * and draw vertical lines.
    */
   QBrush weekEndBrush( QColor(246,242,222));
   QBrush weekDayBrush( gray );
   // qDebug( "Clip %d %d - %d x %d is %s",
   //	   clip.x(), clip.y(), clip.width(), clip.height(),
   // 	   painter.hasClipping()? "on": "off");

   int x = 0;
   int y = clip.top();
   int dayWidth = m_header->dayWidth();

   /* wind to start of the clipping area */
   while ( x < clip.left() )
      x += dayWidth;
   /* remove one width again to be sure starting outside the visible area */
   x -= dayWidth;
   if( x < 0 ) x = 0;
   
   /* a starttime variable */
   time_t runtime = m_header->timeFromX(x) + ONEDAY/2;
   /* add half a day to avoid round probs. timeFromX is in danger of round probs */

   /* draw the head */
   m_monthStartX = x;   /* used in the drawHead-functions */
   m_weekStartX  = x;

   const QPen p1 ( QColor(222,222,222), 0 );
   painter.setPen(p1);

   while ( x <= clip.right() )
   {
       if( isWeekend( runtime ) )
       {
           /* Colorize Weekend */
           painter.setPen( NoPen );
           painter.setBrush( weekEndBrush);
           painter.drawRect( x, y,
                             dayWidth,
                             clip.height()+2);
           painter.setPen( p1 );
       }
       else
       {
           painter.setBrush( weekDayBrush );
       }
       painter.drawLine( x, y , x, y+clip.height()+2);
       /* Do drawing */
       x += dayWidth;
       runtime += ONEDAY;
   }

   painter.setBrush( origBrush );
   setChanged( clip );
   painter.setClipping( false );
}


void KTVTaskCanvas::slMoveItems( int y, int dy )
{
   // Move all items residing on a higher y position than y by dy.
   qDebug( "Moving canvas items from %d by %d", y, dy );

   int startHere = y;

   const CanvasItemList ktvItems = getCanvasItemsList();
   CanvasItemListIterator it(ktvItems);

   for ( ; it.current(); ++it )
   {
      int y = (*it)->y();
      if( y >= startHere )
      {
	 (*it)->moveBy(0, dy);
      }
   }

   if( m_canvasMarker && m_canvasMarker->y() >= y )
       m_canvasMarker->moveBy( 0, dy );
}


KTVCanvasItemBase* KTVTaskCanvas::taskToCanvasItem( const Task *t ) const
{
   KTVCanvasItemBase *cItem = 0;
   if( t )
      cItem = static_cast<KTVCanvasItemBase *>( m_canvasItems[ (void*) t ]);
   return cItem;
}

KTVCanvasItemBase* KTVTaskCanvas::tableItemToCanvasItem( const KTVTaskTableItem* tabItem ) const
{
   Task *t = static_cast<Task*>(m_tasks[ (void*) tabItem ]);

   return( taskToCanvasItem( t ));
}

void KTVTaskCanvas::slNewTask( Task *t, KTVTaskTableItem *it )
{
    m_tasks.insert( it, t );
}


void KTVTaskCanvas::slHideTask( KTVTaskTableItem *tabItem )
{
   KTVCanvasItemBase *cItem = tableItemToCanvasItem( tabItem );
   if( cItem )
      cItem->hide();


}

void KTVTaskCanvas::slShowTask( KTVTaskTableItem *tabItem )
{
    if( !tabItem ) return;
    int yPos = tabItem->itemPos()-
	m_taskTable->rootItemHeight();  /* returns y position in table */


    Task *t = static_cast<Task*>(m_tasks[ (void*) tabItem ]);
    slShowTask( t, yPos );
}

/*
 * Shows the task at position ypos, which must be in canvas coordinates.
 */
void KTVTaskCanvas::slShowTask( Task *t, int ypos )
{
   if( t )
   {
       slShowDebugMarker( ypos );


      /* paint */
      KTVCanvasItemBase *cItem = taskToCanvasItem( t );
      int x = m_header->timeToX( t->getStart(0)); // Task::Plan) );

      if( ! cItem )
      {
          // qDebug(" ***** creating new !" );
	 if( t->isMilestone() )
	 {
	    cItem = new KTVCanvasItemMilestone( this );
	 }
	 else if( t->isContainer() )
	 {
	    cItem = new KTVCanvasItemContainer( this );
	 }
	 else
	 {
	    cItem = new KTVCanvasItemTask( this );
	 }
	 Q_ASSERT(cItem );
	 cItem->setFont( m_itemFont );
	 cItem->setTask(t);
	 /* a dict which makes the items easily accessible by knowing the task
	  * it is needed to find out if a canvas item already exits. */
	 m_canvasItems.insert( t, cItem );

	 /* A list of all items */
	 m_canvasItemList.append( cItem );
      }

      /* set the items with */
      int w = m_header->timeToX( t->getEnd(0 /* Task::Plan */ ) )-x;
      cItem->setSize( w, cItem->height() );

      int itHeight = m_taskTable->itemHeight();
      ypos += (itHeight-cItem->height())/2;  /* center item */

      bool itemConnects = false; /* local flag if item connects to others with a line */
      itemConnects = !( t->isContainer() );
      itemConnects = true;
      // qDebug("---###  showing task at %d, %d!", x, yPos);

      /* yPos contains the bottom-position, from that we subtract the height
       * of the row and add the offset.
       */

      /* This moves the item to the position where it should be */
      cItem->move( x, ypos );
      cItem->show();

      /* check for connections to other tasks, showing dependencies. */
	  for (TaskListIterator pi(t->getPreviousIterator()); *pi != 0; ++pi)
      {
	 /* tp is a previous task. Connection starts at tps endpoint and goes to
	  * this (ts) start point */
	 // qDebug( "handling previous!" );
	 KTVCanvasItemBase *tpItem = taskToCanvasItem( *pi );
	 if( tpItem )
	 {
	    connectTasks( *pi, t, tpItem, cItem );
	 }
      }
   }
}

void KTVTaskCanvas::connectTasks( Task *fromTask, Task* actTask,
				  KTVCanvasItemBase *fromItem,
				  KTVCanvasItemBase *actItem )
{
   /* need a list here that knows both of the connected-to and -from
      tasks. In case a task should be hided, all connections dealing with that
      task need to be hidden, regardless if connected to or from..
   */
   if( !( fromItem && actItem ) ) return;

   if( !fromTask )
   {
      fromTask = fromItem->getTask();
   }

   if( ! actTask )
   {
      actTask = actItem->getTask();
   }

   QPoint from = fromItem->getConnectorOut();
   QPoint to   =  actItem->getConnectorIn();

   KTVConnector *newCon = 0;

   newCon = fromItem->connectorOut( actTask );
   if( ! newCon )
   {
      qDebug("Making a new connector!" );
      newCon = new KTVConnector(this, from, to );

      fromItem->addConnectOut( newCon, actTask );
       actItem->addConnectIn( newCon, fromTask );
   }
   else
   {
       // qDebug("Connector exists!" );
      newCon->setConnectPoints( from, to );
   }
   if( fromItem->isVisible() && actItem->isVisible() )
      newCon->show();

   /* No update here - slows down everything */
}



/* called on selectionChanged of the listview */

void KTVTaskCanvas::slShowMarker( int y )
{
   qDebug( "Showing marker at %d",y );
   if( y >= 0 && m_canvasMarker->y() != y )
   {
      qDebug( "Marker size is %dx%d", m_canvasMarker->width(), m_canvasMarker->height());
      /* move and show the thing */
      m_canvasMarker->move(1, y );
      m_canvasMarker->show();
   }
   else if( y < 0 )
   {
      /* hide it */
      m_canvasMarker->hide();
   }
   update();
}


void KTVTaskCanvas::slShowDebugMarker( int y )
{
   m_dbgMark->move( 0, y );
   if( !m_dbgMark->visible() ) m_dbgMark->show();
   update();
   qDebug( "Moving debug-mark to %d", y );
}

void KTVTaskCanvas::slHeightChanged( int newHeight )
{
   qDebug(" Setting new height %d", newHeight );

   int w = width();

   resize( w, newHeight );
}


KTVCanvasItemBase* KTVTaskCanvas::qCanvasItemToItemBase( QCanvasItem* qItem )
{
   const CanvasItemList ktvItems = getCanvasItemsList();

   CanvasItemListIterator it(ktvItems);

   for ( ; it.current(); ++it )
   {
      if( (*it)->contains(qItem ) )
      {
	 return (*it);
      }
   }
   return 0L;
}


void KTVTaskCanvas::clear()
{
    qDebug("Clearing all!");
    // QCanvas::clear();
    
    m_canvasItems.clear();  // removes the memory of the canvas items.
    m_canvasItemList.clear(); // removes the pointer to the items
    m_tasks.clear();
    resize(0,0);
}
