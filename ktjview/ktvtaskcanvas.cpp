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
    m_topOffset( 25 ),
    m_weekStartMon( true )
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

   m_headerFont.setFamily( "Helvetica [Cronyx]" );
   m_headerFont.setPointSize(8);


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
   // setAllChanged();

   /* recalc width an x-positions of all items*/
   const CanvasItemList ktvItems = getCanvasItemsList();
   CanvasItemListIterator it(ktvItems);

   for ( ; it.current(); ++it )
   {
       slShowTask( (*it)->getTask(), (*it)->y() );
   }
   setAllChanged();
   update();

}



void KTVTaskCanvas::setInterval( time_t start, time_t end )
{
   m_start = midnight( start );
   m_end = midnight( end+ONEDAY );
   m_days = 1 + daysBetween( m_start, m_end );
   qDebug( "Setting interval from %ld -> %ld", m_start, m_end );

   int w = m_days*m_dayWidth;
   int h = 100*m_dayHeight; // TODO 100*

   qDebug("Resizing to %dx%d =%d days", w, h, m_days );
   resize( w, h ); // TODO: amount of tasks
   m_canvasMarker->setSize( w, m_canvasMarker->height() );
}

void KTVTaskCanvas::drawCalendar()
{
   /* Draw month, week and day-header */
   time_t trun = m_start;
   QBrush weekEndBrush( QColor(246,242,222), Dense4Pattern);
   QPen p1 ( QColor(222,222,222), 0 );

   int dayH = 1+getHeaderLineHeight(Day);
   int yDays = topOffset(Day);
   qDebug( "Y-Value for Days: %d", yDays );
   for( int i = 0; i < m_days; i++ )
   {
      int dayX= timeToX(trun);
      int day = dayOfMonth(trun);

      QCanvasRectangle *re = new QCanvasRectangle( this );
      re->setPen( p1 );
      re->setSize( 1+m_dayWidth, dayH );
      re->setBrush( weekEndBrush );
      re->move( dayX, yDays );
      re->setZ(1.0);
      re->show();

      QCanvasText *text = new QCanvasText( QString::number(day), this );
      text->setTextFlags( Qt::AlignCenter );
      text->move( dayX + m_dayWidth/2, yDays + getHeaderLineHeight(Day)/2 );
      text->setZ( 2.0 );
      text->show();

      trun = sameTimeNextDay(trun);
   }

   /* Draw weeks */
   int weeks = weeksBetween( m_start, m_end );
   dayH = 1+getHeaderLineHeight( Week );
   yDays = topOffset(Week);
   qDebug( "Y-Value for Weeks: %d", yDays );

   trun = m_start;
   for( int w = 0; w < weeks; w++ )
   {
      int weekX = timeToX( beginOfWeek(trun, m_weekStartMon));

      QCanvasRectangle *re = new QCanvasRectangle( this );
      re->setPen( p1 );
      re->setSize( 1+7*m_dayWidth, dayH );
      re->setBrush( weekEndBrush );
      re->move( weekX, yDays );
      re->setZ(1.0);
      re->show();

      QCanvasText *text = new QCanvasText( i18n("Week %1").arg( weekOfYear(trun, m_weekStartMon)), this );
      text->setTextFlags( Qt::AlignCenter );
      text->move( weekX + (7*m_dayWidth)/2, yDays + getHeaderLineHeight(Week)/2 );
      text->setZ( 2.0 );
      text->show();

      trun = sameTimeNextWeek(trun);

   }

   /* Draw months */
   int months = monthsBetween( m_start, m_end );
   dayH = 1+getHeaderLineHeight( Month );
   yDays = topOffset(Month);
   qDebug( "Y-Value for Months: %d", yDays );

   trun = m_start;
   for( int w = 0; w < months; w++ )
   {
      time_t mBegin = beginOfMonth( trun );
      int monthX = timeToX( mBegin );
      int cntDays = daysLeftInMonth( mBegin );
      // qDebug("Showing up month %d, %d days long", w, cntDays );

      QCanvasRectangle *re = new QCanvasRectangle( this );
      re->setPen( p1 );
      qDebug( "showing month at %d, %d", 1+cntDays * m_dayWidth, dayH );
      re->setSize( 1+cntDays * m_dayWidth, dayH );
      re->setBrush( weekEndBrush );
      re->move( monthX, yDays );
      re->setZ(1.0);
      re->show();

      QCanvasText *text = new QCanvasText( QString(monthAndYear(trun)), this );
      text->setTextFlags( Qt::AlignCenter );
      text->move( monthX + (cntDays*m_dayWidth)/2, yDays + getHeaderLineHeight(Month)/2 );
      text->setZ( 2.0 );
      text->show();

      trun = sameTimeNextMonth(trun);

   }

   setAllChanged();
   update();
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


int KTVTaskCanvas::getHeaderLineHeight( topOffsetPart p )
{
   QFontMetrics fm( m_headerFont );
   int lineHeight = fm.height()+2;
   int re=0;
   // qDebug( "TopOffset: %d and lineHeight: %d", m_topOffset, lineHeight );
   switch( p )
   {
      case Month:
	 re = m_topOffset-(2*lineHeight);
	 break;
      case Week:
      case Day:
	 re = lineHeight;
	 break;
      case All:
	 re = m_topOffset;
	 break;
      default: /* is Day */
	 re = lineHeight;
   }
   // qDebug( "returning value %d for p %d", re, int(p) );

   return re;
}

int  KTVTaskCanvas::topOffset( topOffsetPart part )
{
   int re = 0;

   switch( part )
   {
      case Month:
	 re = 0;
	 break;
      case All:
	 re = m_topOffset;
	 break;
      case Week:
	 // re = 1+getHeaderLineHeight( Month );
	 re = getHeaderLineHeight(All)- getHeaderLineHeight( Day )
	    - getHeaderLineHeight( Week );
	 break;
      case Day:
	 re = getHeaderLineHeight(All)- getHeaderLineHeight( Day );
	 break;
   }
   return re;
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
   // qDebug( "Clip %d %d - %d x %d is %s",
   //	   clip.x(), clip.y(), clip.width(), clip.height(),
   // 	   painter.hasClipping()? "on": "off");

   int x = 0;
   int y = clip.top() > getHeaderLineHeight(All) ? clip.top(): getHeaderLineHeight(All);

   /* wind to start of the clipping area */
   while ( x < clip.left() )
      x += m_dayWidth;
   /* remove one width again to be sure starting outside the visible area */
   x -= m_dayWidth;

   /* draw the head */
   m_monthStartX = x;   /* used in the drawHead-functions */
   m_weekStartX  = x;

   while ( x <= clip.right() )
   {

      time_t time = timeFromX(x);
      // drawDayHead  ( time, x, painter );
      // drawWeekHead ( time, x, painter );
      // drawMonthHead( time, x, painter );

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

   int startHere = y+getHeaderLineHeight(All);
   slShowDebugMarker( startHere+dy );

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
    if( !tabItem ) return;
    int yPos = tabItem->itemPos()  /* returns y position in table */
               + getHeaderLineHeight(All) /* Top offset for calendar */
               - m_rowHeight              /* one line back */
               + (m_rowHeight-tabItem->height())/2;  /* center item */

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
      /* paint */
      KTVCanvasItemBase *cItem = taskToCanvasItem( t );
      int x = timeToX( t->getPlanStart() );
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
	 cItem->setFont( m_headerFont );
	 cItem->setTask(t);
	 // cItem->setZ(1.0);
	 Q_ASSERT(cItem );
	 /* a dict which makes the items easily accessible by knowing the task
	  * it is needed to find out if a canvas item already exits. */
	 m_canvasItems.insert( t, cItem );

	 /* A list of all items */
	 m_canvasItemList.append( cItem );
      }

      /* set the items with */
      int w = timeToX( t->getPlanEnd() )-x;
      cItem->setSize( w, cItem->height() );

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
      for( Task* tp = t->firstPrevious(); itemConnects && tp != 0; tp = t->nextPrevious())
      {
	 /* tp is a previous task. Connection starts at tps endpoint and goes to
	  * this (ts) start point */
	 // qDebug( "handling previous!" );
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


