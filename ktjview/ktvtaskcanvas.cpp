#include <qdatetime.h>
#include <kdebug.h>
#include <klocale.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <qbrush.h>
#include <qrect.h>
#include <qpainter.h>
#include <qlabel.h>
#include <qpixmap.h>
#include <qheader.h>

#include "ktvtaskcanvas.h"
#include "ktvtaskcanvasitem.h"
#include "ktvtasktable.h"
#include "Project.h"
#include "Task.h"
#include "Utility.h"
#include "ktvcanvasitem.h"

KTVTaskCanvas::KTVTaskCanvas( QWidget *parent, KTVTaskTable* tab, const char *name )
   :QCanvas( parent, name ),
    m_dayHeight( 15 ),
    m_topOffset( 25 ),
    m_markerHeight( 0 ), /* shitty if not set */ 
    m_markerOffset(0)
{
   m_taskTable = tab;
   m_days = 0;
   m_dayWidth = 20;
   if( m_taskTable )
      m_dayHeight = m_taskTable->header()->height()-1;
      
   m_start = 0;
   m_end = 0;
   setDoubleBuffering( false );
   resize( 100, 100);
}


/* sets a pointer to the connected table. This class is friend of
 *  the table. */
void KTVTaskCanvas::setTable( KTVTaskTable *tab )
{
   m_taskTable = tab;
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

void KTVTaskCanvas::drawDayHead( int x, QPainter &p )
{
   QLabel l(0);
   l.setText( QString::number(dayOfMonth(timeFromX( x ))) );
   l.resize( m_dayWidth, topOffset());
   l.setAlignment( Qt::AlignCenter );
   l.setFrameStyle( QFrame::Panel | QFrame::Sunken );

   QPixmap pix( m_dayWidth, topOffset() );
   p.drawPixmap( x, 0, pix.grabWidget( &l ));
   
   QPen p1 ( black, 0 );
   p.setPen( p1 );
   p.drawLine( x, topOffset()-1, width(), topOffset()-1);
}


void KTVTaskCanvas::drawBackground( QPainter &painter, const QRect & clip )
{
   // qDebug( "Drawing background" ); 
   QCanvas::drawBackground( painter, clip );
   
   QBrush origBrush = painter.brush();

   // if( m_start == m_end ) return;
   
   /* take the start day of the project, calculate midnight
    * and draw vertical lines. 
    */
   QBrush weekEndBrush( QColor(255,255,170));
   QBrush weekDayBrush( gray );

   // qDebug( "Clip %d %d - %d x %d is %s",
   //	   clip.x(), clip.y(), clip.width(), clip.height(),
   // 	   painter.hasClipping()? "on": "off");

   int x = 0;
   int y = clip.top() > topOffset() ? clip.top()-1: topOffset();

   /* wind to start of the clipping area */
   while ( x < clip.left() ) 
      x += m_dayWidth;
   x -= m_dayWidth;
   while ( x <= clip.right() )
   {
      drawDayHead( x, painter );
      if( isWeekend( timeFromX(x) ) )
      {
	 /* Colorize Weekend */
	 painter.setBrush( weekEndBrush);
	 painter.drawRect( x, y,
			   m_dayWidth, clip.height()+2);

      }
      else
      {
	 painter.setBrush( weekDayBrush );
	 // painter.drawRect(r);
      
	 QPen p1 ( black, 0 );
	 painter.setPen( p1 );
	 painter.drawLine( x, y , x, y+clip.height()+2);
      }
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
   m_markerHeight = (m_rowHeight*4) / 5;
   m_markerOffset = topOffset() + (m_rowHeight-m_markerHeight)/2;
   qDebug( "Setting row metrix: %d, %d and offset %d",
	   m_rowHeight, m_markerHeight, m_markerOffset );
   // TODO: redraw();
}


void KTVTaskCanvas::slMoveItems( int y, int dy )
{
   // Move all items residing on a higher y position than y by dy.
   qDebug( "Moving canvas items from %d by %d", y, dy );
   QCanvasItemList items = allItems();
   QCanvasItemList::iterator it;
   int yoff = topOffset();
   
   for ( it = items.begin(); it != items.end(); ++it )
      if( (*it)->y() > yoff+y )
      {
	 qDebug( "moving!" );
	 (*it)->moveBy( 0, double(dy) );
      }
}


void KTVTaskCanvas::slHideTask( KTVTaskTableItem *tabItem )
{
   Task *t = static_cast<Task*>(m_tasks[ (void*) tabItem ]);

   if( t )
   {
      KTVCanvasItemBase *cItem = static_cast<KTVCanvasItemBase *>( m_canvasItems[ (void*) t ]);
      if( cItem )
	 cItem->hide();
   }
   
}

void KTVTaskCanvas::slShowTask( KTVTaskTableItem *tabItem )
{
   int yPos = tabItem->itemPos();

   Task *t = static_cast<Task*>(m_tasks[ (void*) tabItem ]);

   if( t )
   {
      /* paint */
      KTVCanvasItemBase *cItem = static_cast<KTVCanvasItemBase *>( m_canvasItems[ (void*) t ]);
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
	 cItem->setTask(t);
	 Q_ASSERT(cItem );
	 m_canvasItems.insert( t, cItem );
	 int w = timeToX( t->getPlanEnd() )-x;
	 cItem->setSize( w, m_markerHeight );
      }
      qDebug("---###  showing task at %d, %d!", x, yPos);
      
      cItem->move( x, yPos + m_markerOffset );
      cItem->show();
   }
}
