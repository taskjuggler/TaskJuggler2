#include <qdatetime.h>
#include <klocale.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <qbrush.h>
#include <qrect.h>
#include <qpainter.h>

#include <qtooltip.h>
#include <qwmatrix.h>

#include "ktvtasktable.h"
#include "ktvtaskcanvas.h"
#include "ktvtaskcanvasview.h"
#include "ktvcanvasitem.h"
#include "Project.h"
#include "Task.h"
#include "Utility.h"
#include "tasktip.h"
#include <qevent.h>


KTVTaskCanvasView::KTVTaskCanvasView( QWidget *parent, KTVTaskTable* tab, const char *name )
   :QCanvasView( parent, name ),
    m_canvas(0),
    m_scaleFactor(1.0)
{
    m_canvas = new KTVTaskCanvas( parent, tab, name );
   setCanvas( m_canvas );
   (void) new TaskTip( this );

   connect( this, SIGNAL( contentsMoving( int, int )),
	    this, SLOT( slScrollTo( int, int )));

   setResizePolicy( QScrollView::Default);
   setVScrollBarMode( QScrollView::AlwaysOn );

   // m_canvas->resize( 400, 300);
}


void KTVTaskCanvasView::showProject( Project *p )
{
   qDebug( " ++++++ Starting to show +++++++" );
   m_pro = p;

   QString pName = p->getName();

   /* resize the canvas */
   m_canvas->slSetDayWidthStandard();
   m_canvas->setInterval( p->getStart(), p->getEnd() );

   m_canvas->slSetWeekStartsMonday( p->getWeekStartsMonday() );
}

void KTVTaskCanvasView::finalise( Project* )
{
   m_canvas->drawCalendar();
}


void KTVTaskCanvasView::addTask(Task *t )
{
   if( ! t ) return;

   QString idx = QString::number(t->getIndex());
   QString name = t->getName();
   qDebug( "Adding task: " + t->getId());
   QDateTime dt;

   if( t->isContainer() )
   {

   }
   else if( t->isMilestone() )
   {

   }
   else

   dt.setTime_t( t->getStart(Task::Plan) );

   dt.setTime_t( t->getEnd(Task::Plan) );

   TaskList subTasks;
   subTasks.clear();

   qDebug( "START: Subpackages for "+ t->getId());
   for (TaskListIterator tli(t->getSubListIterator()); *tli != 0; ++tli)
   {
      // qDebug( "Calling subtask " + st->getId() + " from " + t->getId() );
      if( (*tli)->getParent() == t )
	 addTask( *tli );
      //qDebug( "Calling subtask " + st->getId() + " from " + t->getId() + " <FIN>" );
   }
   qDebug( "END: Subpackages for "+ t->getId());
   qDebug( "Adding task: " + t->getId() + "<FIN>");

}

KTVCanvasItemBase*  KTVTaskCanvasView::taskItemAt( const QPoint& p )
{
   // qDebug( "On related widget: x=%d", p.x() );
   QPoint r = viewportToContents(p);
   // qDebug( "Converted: x=%d", r.x() );

   QCanvasItemList il = canvas()->collisions ( r );

   KTVCanvasItemBase *item = 0L;

   QCanvasItemList::iterator it;

   for ( it = il.begin(); !item && it != il.end(); ++it )
   {
      item = static_cast<KTVTaskCanvas*>(canvas())->qCanvasItemToItemBase( *it );
   }

   return item;
}

/*
 * returns the time that is currently centered in the viewport.
 */
time_t KTVTaskCanvasView::getCenterTime()
{
    int x = contentsX() + contentsWidth()/2;
    time_t t = m_canvas->timeFromX( x );

    qDebug("getCenterTime: %s", (const char*) time2ISO(t) );
    return( t );
}

/**
 * ensure that the given time is visible at a p percent of the viewport
 * width from the left side.
 *
 */
void KTVTaskCanvasView::xScrollToTime( int p, time_t ti )
{
    int timeX = m_canvas->timeToX( ti );

    if( p > 0 && p <= 100 )
    {
        timeX -= int(double(p/100.0)*contentsWidth());
    }
    qDebug( "Centering on %d %s", timeX, (const char*) time2ISO(timeX));
    setContentsPos( timeX, contentsY() );
}

void KTVTaskCanvasView::zoomIn()
{
    int w = m_canvas->getDayWidth();
    time_t centerTime = getCenterTime();
    qDebug("getCenterTime1: %s", (const char*) time2ISO(centerTime) );
    m_canvas->slSetDayWidth( int(1.2 * w));
    xScrollToTime( 50, centerTime );
    update();
}

void KTVTaskCanvasView::zoomOut()
{
    int w = m_canvas->getDayWidth();
    time_t centerTime = getCenterTime();
    m_canvas->slSetDayWidth( int(0.8 * w));
    xScrollToTime( 50, centerTime );
    update();

}

void KTVTaskCanvasView::zoomOriginal()
{
    time_t centerTime = getCenterTime();
    m_canvas->slSetDayWidthStandard();
    xScrollToTime( 50, centerTime );
    update();
}


/*
 * This slot is automating the scrolling synchronisation between the listview
 * and the canvas. To this slot the signal contentsMove of the list is connected,
 * which scrolls automagically.
 */
void KTVTaskCanvasView::slScrollTo( int, int y)
{
   // qDebug( "Scrolling!");
   emit scrolledBy( 0, y - contentsY() );
}


void KTVTaskCanvasView::contentsMousePressEvent ( QMouseEvent *e )
{
   qDebug( "Mouse-Move at %d %d", e->pos().x(), e->pos().y());
   emit canvasClicked( m_canvas->timeFromX( e->pos().x()));
}
