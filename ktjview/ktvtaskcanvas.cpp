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
#include "Project.h"
#include "Task.h"
#include "Utility.h"
#include "ktvcanvasitem.h"
#include <qfontmetrics.h>

#define STD_DAYWIDTH 20


KTVTaskCanvas::KTVTaskCanvas( QWidget *parent, KTVTaskTable* tab, const char *name )
   :QCanvas( parent, name ),
    m_dayHeight( 15 ),
    m_topOffset( 25 )
{
   m_taskTable = tab;
   m_days = 0;
   m_dayWidth = STD_DAYWIDTH;
   if( m_taskTable )
      m_dayHeight = m_taskTable->header()->height()-1;


   m_canvasMarker = new QCanvasRectangle( this  );
   m_canvasMarker->hide();
   m_canvasMarker->move(0,0);
   m_canvasMarker->setPen( NoPen );

   m_dbgMark = new QCanvasLine( this );
   m_dbgMark->setPoints( 1, 0, 15, 0 );

   QBrush wBrush( QColor(222,222,222), Dense4Pattern);
   m_canvasMarker->setBrush( wBrush ); // QBrush(gray, Dense4Pattern ));

   m_canvasMarker->setZ(-0.1);
   m_start = 0;
   m_end = 0;
   setDoubleBuffering( true ); // false );
   resize( 400, 300);
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

void KTVTaskCanvas::slSetTopOffset( int o )
{
   m_topOffset = o;
}

void KTVTaskCanvas::slSetDayWidthStandard( )
{
   slSetDayWidth( STD_DAYWIDTH );
}

void KTVTaskCanvas::slSetDayWidth( int _w )
{
   m_dayWidth = _w;

   QSize s = size();
   resize( m_dayWidth * m_days, s.height() );

   /* recalc width an x-positions of all items*/
   const CanvasItemList ktvItems = getCanvasItemsList();
   CanvasItemListIterator it(ktvItems);

   for ( ; it.current(); ++it )
   {
      Task *t = (*it)->getTask();
      int old_x = (*it)->x();
      int new_x = timeToX( t->getPlanStart() );
      int w = timeToX( t->getPlanEnd() )-new_x;

      (*it)->setSize(w, (*it)->height() );

      (*it)->moveBy(new_x - old_x, 0);
   }
   setAllChanged();
   update();
}

void KTVTaskCanvas::setInterval( time_t start, time_t end )
{
   m_start = midnight( start-ONEDAY );
   m_end = midnight( end );
   m_days = 1 + daysBetween( m_start, m_end );
   qDebug( "Setting interval from %ld -> %ld", m_start, m_end );

   int w = m_days*m_dayWidth;
   int h = 100*m_dayHeight; // TODO 100*

   qDebug("Resizing to %dx%d =%d days", w, h, m_days );
   resize( w, h ); // TODO: amount of tasks
   m_canvasMarker->setSize( w, m_canvasMarker->height() );
}

time_t KTVTaskCanvas::timeFromX( int x )
{
   return m_start + time_t(double(ONEDAY) * double(x)/double(m_dayWidth));
}

int KTVTaskCanvas::midnightToX( time_t t )
{
   if( t < m_start ) return 0;
   if( t > m_end ) return width();

   return m_dayWidth * daysBetween( m_start, midnight(t) );
}

int KTVTaskCanvas::timeToX( time_t t )
{
   if( t < m_start ) return 0;
   if( t > m_end ) return width();

   double p = double(m_dayWidth)/double(ONEDAY);

   return int( p* double(t-m_start));
}

int  KTVTaskCanvas::topOffset( topOffsetPart part )
{

    int upper = m_topOffset - m_rowHeight;
    int lower = m_rowHeight;

    switch( part )
    {
    case Upper:
        return upper;
        break;
    case Lower:
        return lower;
        break;
    default:
        return m_topOffset;
    }
}



void KTVTaskCanvas::drawMonthHead( const QFont &f, time_t time, int x, QPainter &p )
{
    if( dayOfMonth( time ) == 1 )
    {
        int third = topOffset()/3;

        QLabel l(0);
        l.setFont(f);
        l.setFrameStyle( QFrame::Panel | QFrame::Sunken );

        /* draw from 0 to x the fragment of the month before this coming month */
        time_t t1 = beginOfMonth( time );
        l.setText( QString( monthAndYear( t1-100 )));

        int width = x - m_monthStartX;

        l.resize( width, third );
        QPixmap pix( width, third );
        p.drawPixmap( m_monthStartX, 0, pix.grabWidget( &l ));
        m_monthStartX = timeToX( t1 );
    }
}

void KTVTaskCanvas::drawWeekHead( const QFont &f, time_t time, int x , QPainter &p )
{
    if( dayOfWeek( time ) != 0 ) return;
    int third = topOffset() / 3;

    QLabel l(0);
    l.setFont(f);
    l.setText( QString::number( weekOfYear( time )));
    l.resize( 7*m_dayWidth, third );
    l.setFrameStyle( QFrame::Panel | QFrame::Sunken );

    QPixmap pix( 7*m_dayWidth, third );
    p.drawPixmap( x, third, pix.grabWidget( &l ));
}

void KTVTaskCanvas::drawDayHead( QFont &f, time_t time, int x, QPainter &p )
{
   QLabel l(0);
   int third = topOffset() / 3;

   l.setFont(f);
   l.setText( QString::number( dayOfMonth(time) ) );
   l.resize( m_dayWidth, third );
   l.setAlignment( Qt::AlignCenter );
   l.setFrameStyle( QFrame::Panel | QFrame::Sunken );

   QPixmap pix( m_dayWidth, topOffset() );
   p.drawPixmap( x, 2*third, pix.grabWidget( &l ));

   QPen p1 ( black, 0 );
   p.setPen( p1 );
   p.drawLine( x, topOffset()-1, width(), topOffset()-1);
}


void KTVTaskCanvas::drawBackground( QPainter &painter, const QRect & clip )
{
   // qDebug( "Drawing background" );
   QCanvas::drawBackground( painter, clip );

   QBrush origBrush = painter.brush();

   /* take the start day of the project, calculate midnight
    * and draw vertical lines.
    */
   QBrush weekEndBrush( QColor(246,242,222));
   QBrush weekDayBrush( gray );
   QFont headerFont( "Helvetica [Cronyx]", 8 );
   // qDebug( "Clip %d %d - %d x %d is %s",
   //	   clip.x(), clip.y(), clip.width(), clip.height(),
   // 	   painter.hasClipping()? "on": "off");

   int x = 0;
   int y = clip.top() > topOffset() ? clip.top(): topOffset();

   /* wind to start of the clipping area */
   while ( x < clip.left() )
      x += m_dayWidth;
   /* remove one width again to be sure starting outside the visible area */
   x -= m_dayWidth;

   /* draw the head */
   headerReso reso = Day;
   if( reso == Day )
   {
       m_monthStartX = x;   /* used in the drawHead-functions */
       m_weekStartX  = x;

       while ( x <= clip.right() )
       {
           time_t time = timeFromX(x);
           drawDayHead  ( headerFont, time, x, painter );
           drawWeekHead ( headerFont, time, x, painter );
           drawMonthHead( headerFont, time, x, painter );

           if( isWeekend( time ) )
           {
               /* Colorize Weekend */
               painter.setPen( NoPen );
               painter.setBrush( weekEndBrush);
               painter.drawRect( x, y,
                                 m_dayWidth, clip.height()+2);
           }
           else
           {
               painter.setBrush( weekDayBrush );
           }
           QPen p1 ( QColor(222,222,222), 0 );
           painter.setPen( p1 );
           painter.drawLine( x, y , x, y+clip.height()+2);
           /* Do drawing */
           x += m_dayWidth;
       }
   }
   painter.setBrush( origBrush );
   setChanged( clip );
   painter.setClipping( false );
}


void KTVTaskCanvas::slSetRowHeight(int h )
{
   m_rowHeight = h;

   qDebug( "Setting row height: %d", m_rowHeight );

   int w = m_canvasMarker->width();
   m_canvasMarker->setSize( w, m_rowHeight );

   // TODO: redraw();
}


void KTVTaskCanvas::slMoveItems( int y, int dy )
{
   // Move all items residing on a higher y position than y by dy.
   qDebug( "Moving canvas items from %d by %d", y, dy );

   int startHere = y+topOffset()-m_rowHeight;
   slShowDebugMarker( startHere );

   const CanvasItemList ktvItems = getCanvasItemsList();
   CanvasItemListIterator it(ktvItems);

   for ( ; it.current(); ++it )
   {
      int y = (*it)->y();
      if( y > startHere )
      {
	 (*it)->moveBy(0, dy);
      }
   }
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


void KTVTaskCanvas::slHideTask( KTVTaskTableItem *tabItem )
{
   KTVCanvasItemBase *cItem = tableItemToCanvasItem( tabItem );
   if( cItem )
      cItem->hide();


}

void KTVTaskCanvas::slShowTask( KTVTaskTableItem *tabItem )
{
   int yPos = tabItem->itemPos();

   Task *t = static_cast<Task*>(m_tasks[ (void*) tabItem ]);

   if( t )
   {
      /* paint */
      KTVCanvasItemBase *cItem = taskToCanvasItem( t );
      int x = timeToX( t->getPlanStart() );
      if( ! cItem )
      {
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
	 cItem->setFont( QFont( "Helvetica", 8 ));
	 cItem->setTask(t);
	 // cItem->setZ(1.0);
	 Q_ASSERT(cItem );
	 /* a dict which makes the items easily accessible by knowing the task
	  * it is needed to find out if a canvas item already exits. */
	 m_canvasItems.insert( t, cItem );

	 /* A list of all items */
	 m_canvasItemList.append( cItem );
      }

      int w = timeToX( t->getPlanEnd() )-x;
      cItem->setSize( w, cItem->height() );

      bool itemConnects = false; /* local flag if item connects to others with a line */
      itemConnects = !( t->isContainer() );

      // qDebug("---###  showing task at %d, %d!", x, yPos);

      /* yPos contains the bottom-position, from that we subtract the height
       * of the row and add the offset.
       */

      /* This moves the item to the position where it should be */
      cItem->move( x, yPos + topOffset() - m_rowHeight + (m_rowHeight-cItem->height())/2  );
      cItem->show();

      /* check for connections  */
      for( Task* tp = t->firstPrevious(); itemConnects && tp != 0; tp = t->nextPrevious())
      {
	 /* tp is a previous task. Connection starts at tps endpoint and goes to
	  * this (ts) start point */
	 qDebug( "handling previous!" );
	 KTVCanvasItemBase *tpItem = taskToCanvasItem( tp );
	 if( tpItem )
	 {
	    connectTasks( tp, t, tpItem, cItem );
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
      qDebug("Connector exists!" );
      newCon->setConnectPoints( from, to );
   }
   if( fromItem->isVisible() && actItem->isVisible() )
      newCon->show();

}



/* called on selectionChanged of the listview */

void KTVTaskCanvas::slShowMarker( int y )
{
   qDebug( "Showing marker at %d",y );
   if( y > 0 && m_canvasMarker->y() != y )
   {
      qDebug( "Marker size is %dx%d", m_canvasMarker->width(), m_canvasMarker->height());
      /* move and show the thing */
      m_canvasMarker->move(1, y + m_topOffset);
      m_canvasMarker->show();
   }
   else if( y == 0 )
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


