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
#include "ktvheader.h"
#include "Project.h"
#include "Task.h"
#include "Utility.h"
#include "tasktip.h"
#include <qevent.h>


KTVTaskCanvasView::KTVTaskCanvasView( QWidget *parent, KTVTaskTable* tab, KTVHeader *h, const char *name )
   :QCanvasView( parent, name ),
    m_canvas(0),
    m_table(tab),
    m_header(h),
    m_scaleFactor(1.0)
{
    m_canvas = new KTVTaskCanvas( parent, tab, h, name );
    setCanvas( m_canvas );
    (void) new TaskTip( this );

    connect( this, SIGNAL( contentsMoving( int, int )),
             this, SLOT( slScrollTo( int, int )));

    setResizePolicy( QScrollView::Default);
    setVScrollBarMode( QScrollView::AlwaysOn );

}


void KTVTaskCanvasView::showProject( Project *p )
{
   qDebug( " ++++++ Starting to show +++++++" );
   m_pro = p;

   /* resize Contents */
   int w = m_header->overallWidth();
   int h = m_table->itemHeight() * p->taskCount();
   qDebug("Resizing canvas to %dx%d", w, h );
   m_canvas->resize( w, h );
}


void KTVTaskCanvasView::addTask(Task *t )
{
   if( ! t ) return;

   QString idx = QString::number(t->getIndex());
   QString name = t->getName();
   // qDebug( "Adding task: " + t->getId());
   QDateTime dt;

   if( t->isContainer() )
   {

   }
   else if( t->isMilestone() )
   {

   }
   else
   {

   }

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
    time_t t = m_header->timeFromX( x );

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
    int timeX = m_header->timeToX( ti );

    if( p > 0 && p <= 100 )
    {
        timeX -= int(double(p/100.0)*contentsWidth());
    }
    qDebug( "Centering on %d %s", timeX, (const char*) time2ISO(timeX));
    setContentsPos( timeX, contentsY() );
}

void KTVTaskCanvasView::zoomIn()
{
    int w = m_header->dayWidth();
    time_t centerTime = getCenterTime();
    qDebug("getCenterTime1: %s", (const char*) time2ISO(centerTime) );

    w = ( (w+5 < 60) ? w+5 : 60);
    m_header->slSetDayWidth( w );
    xScrollToTime( 50, centerTime );
    update();
}

void KTVTaskCanvasView::zoomOut()
{
    int w = m_header->dayWidth();
    time_t centerTime = getCenterTime();
    w = ((w-5) > 10 ? w-5 : 10);
    m_header->slSetDayWidth( w );
    xScrollToTime( 50, centerTime );
    update();

}

void KTVTaskCanvasView::zoomOriginal()
{
    time_t centerTime = getCenterTime();
    // m_canvas->slSetDayWidthStandard();
    xScrollToTime( 50, centerTime );
    update();
}

void KTVTaskCanvasView::clear()
{
    if( m_canvas )
    {
        m_canvas->clear();
        m_canvas->resize(0,0);
    }
}

/*
 * This slot is automating the scrolling synchronisation between the listview
 * and the canvas. To this slot the signal contentsMove of the list is connected,
 * which scrolls automagically.
 */
void KTVTaskCanvasView::slScrollTo( int x, int y)
{
   // qDebug( "Scrolling!");
   emit scrolledBy( x-contentsX(), y - contentsY() );
}


void KTVTaskCanvasView::contentsMousePressEvent ( QMouseEvent *e )
{
   qDebug( "Mouse-Move at %d %d", e->pos().x(), e->pos().y());
   emit canvasClicked( m_header->timeFromX( e->pos().x()));
}
