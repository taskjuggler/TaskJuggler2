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


KTVTaskCanvasView::KTVTaskCanvasView( QWidget *parent, KTVTaskTable* tab, const char *name )
   :QCanvasView( parent, name )
{
   m_canvas = new KTVTaskCanvas( parent, tab, name );
   setCanvas( m_canvas );
   (void) new TaskTip( this );
   
   setVScrollBarMode( QScrollView::AlwaysOff );

   m_canvas->resize( 400, 300);
}


void KTVTaskCanvasView::showProject( Project *p )
{
   qDebug( " ++++++ Starting to show +++++++" );
   m_pro = p;
   
   QString pName = p->getName();
   
   /* resize the canvas */
   m_canvas->setInterval( p->getStart(), p->getEnd() );

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
   
   dt.setTime_t( t->getPlanStart() );
   
   dt.setTime_t( t->getPlanEnd() );
   
   TaskList subTasks;
   subTasks.clear();
   
   t->getSubTaskList(subTasks);
   int cnt = subTasks.count();
   qDebug( "Amount of subtasks: " + QString::number(cnt) );

   qDebug( "START: Subpackages for "+ t->getId());
   for (Task* st = subTasks.first(); st != 0; st = subTasks.next())
   {
      // qDebug( "Calling subtask " + st->getId() + " from " + t->getId() );
      if( st->getParent() == t )
	 addTask( st );
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

void KTVTaskCanvasView::zoomIn()
{
   int w = m_canvas->getDayWidth();

   w += (w/5);
   qDebug( "setting day-Width = %d", w );

   m_canvas->slSetDayWidth( w );

   update();
}

void KTVTaskCanvasView::zoomOut()
{
   int w = m_canvas->getDayWidth();

   w -= (w/5);
   qDebug( "setting day-Width = %d", w );
   m_canvas->slSetDayWidth( w );

   update();
}

void KTVTaskCanvasView::zoomOriginal()
{
   m_canvas->slSetDayWidthStandard();
   update();
}


void KTVTaskCanvasView::contentsMousePressEvent( QMouseEvent* )
{

   /* Take only the first of the collision list, that is the front most item */

 
}

