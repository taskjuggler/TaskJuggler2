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
#include "ktvreport.h"

#define PIX_MILESTONE "flag"
#define PIX_TASK      "package_settings"
#define PIX_CONTAINER "attach"
#define PIX_PROJECT   "finish"

/* need to inherit Report here because the sort functions are protected */

KTVTaskTable::KTVTaskTable( QWidget *parent, const char *name )
   :KListView( parent, name ),
    m_itemHeight(16),
    m_root(0),
    m_canvasView(0)
{
   // addColumn( i18n( "No." ));
   addColumn( i18n("Task" ));
   addColumn( i18n("Task ID" ));
   addColumn( i18n("Plan Duration"));   //  COL_PLAN_LEN
   addColumn( i18n("Priority" ));
   addColumn( i18n("Complete" ));
   addColumn( i18n("Plan Start")); //  COL_PLAN_START_DATE );
   addColumn( i18n("Plan Start")); //  COL_PLAN_START_TIME );
   addColumn( i18n("Plan End"));   //  COL_PLAN_END_DATE );
   addColumn( i18n("Plan End"));   //  COL_PLAN_END_TIME );

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


}


void KTVTaskTable::setCanvasView( KTVTaskCanvasView *view )
{
   m_canvasView = view;
}


void KTVTaskTable::showProject( Project *p )
{
   if( m_root )
      delete m_root;


   QString pName = p->getName();

   m_root = new KTVTaskTableItem( this, m_itemHeight );

   setItemHeight( m_root->height() );
   emit topOffsetChanged( header()->height() + m_root->height() );

   m_root->setExpandable( true );

   KIconLoader *loader = KGlobal::iconLoader();
   m_root->setPixmap( COL_NAME, loader->loadIcon( PIX_MILESTONE, KIcon::Small ));

   qDebug( " ++++++ Starting to show +++++++, itemHeight: %d", m_itemHeight);

   m_root->setOpen( true );

   m_root->setText( COL_NAME, i18n("Project: %1").arg(pName) );

   qDebug( "Y-Pos of root: %d, height is %d", m_root->itemPos(), m_root->height() );

   KTVReport rep( p, QString("dummy"), p->getStart(), p->getEnd() );
   TaskList filteredList;
   // filteredList.setSorting( CoreAttributesList::TreeMode );
   rep.filterTaskList(filteredList, 0);
   rep.sortTaskList(filteredList);

   for (Task* t = filteredList.first(); t != 0; t = filteredList.next())
   {
      if( (t->getParent() == 0) && t->isContainer() )
      {
	 addTask( static_cast<KTVTaskTableItem*>(m_root), t );
      }
   }

   /* open the first child of the project */
   QListViewItem *fi = m_root->firstChild();
   if( fi )
      fi->setOpen(true);
}

void KTVTaskTable::addTask( KTVTaskTableItem *parent, Task *t )
{
   if( ! t ) return;

   QString idx = QString::number(t->getIndex());
   QString name = t->getName();
   // qDebug( "Adding task: " + t->getId());
   QDateTime dt;

   KTVTaskTableItem *ktv =  new KTVTaskTableItem( parent, t, m_itemHeight );

   ktv->setText( COL_NAME, name );
   if( t->isContainer() )
      ktv->setPixmap( COL_NAME, m_containerPix );
   else if( t->isMilestone() )
      ktv->setPixmap( COL_NAME, m_milestonePix );
   else
      ktv->setPixmap( COL_NAME, m_taskPix );

   ktv->setText( COL_ID, t->getId() );
   ktv->setText( COL_PLAN_LEN, beautyTimeSpan( t->getEnd(Task::Plan),
											   t->getStart(Task::Plan) ));
   ktv->setText( COL_PRIORITY, QString::number( t->getPriority() ));

   double cmplt = t->getComplete(Task::Plan);
   ktv->setText( COL_COMPLETE, cmplt == -1 ? i18n("iP"):  i18n("%1%").arg(cmplt));

   dt.setTime_t( t->getStart(Task::Plan) );
   ktv->setText( COL_PLAN_START_DATE, KGlobal::locale()->formatDate( dt.date(), true ));
   ktv->setText( COL_PLAN_START_TIME, KGlobal::locale()->formatTime( dt.time(), false ));

   dt.setTime_t( t->getEnd(Task::Plan) );
   ktv->setText( COL_PLAN_END_DATE, KGlobal::locale()->formatDate( dt.date(), true ));
   ktv->setText( COL_PLAN_END_TIME, KGlobal::locale()->formatTime( dt.time(), false ));

   /* signal to create a new task in the canvas */
   emit( newTaskAdded( t, ktv));

   TaskList subTasks;
   subTasks.clear();

   t->getSubTaskList(subTasks);
   // int cnt = subTasks.count();
   // qDebug( "Amount of subtasks: " + QString::number(cnt) );

   // qDebug( "START: Subpackages for "+ t->getId());
   for (Task* st = subTasks.last(); st != 0; st = subTasks.prev())
   {
      // qDebug( "Calling subtask " + st->getId() + " from " + t->getId() );
      if( st->getParent() == t )
	 addTask( ktv, st );
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
   emit( itemHeightChanged(h));
}


void KTVTaskTable::slCollapsed( QListViewItem *it )
{
   if( ! it ) return;

   int y = it->itemPos();
   int childCnt = it->childCount();

   QListViewItem* child = it->firstChild();

   qDebug("---- ### expanded, childs: %d !", childCnt );

   while( child )
   {
      // if( child->childCount() > 0 && isOpen(child) )
      // {
      //    setOpen( child, false );
      // }
      emit hideTaskByItem( static_cast<KTVTaskTableItem*>(child) );
      m_canvasView->getCanvas()->slHideTask(static_cast<KTVTaskTableItem*>(child));
      child = child->nextSibling();
   }

   m_canvasView->getCanvas()->slMoveItems(y + m_itemHeight, -1 * childCnt * m_itemHeight );
   emit moveItems( y + m_itemHeight, -1 * childCnt * m_itemHeight );
   slUpdateCanvas();
}



void KTVTaskTable::slExpanded( QListViewItem* it)
{
    int y = it->itemPos();
   int childCnt = it->childCount();

   qDebug("---- ### expanded, childs: %d !", childCnt );

   // emit moveItems( y, childCnt * m_itemHeight );
   /* move items of the whole canvas to gain space for the new subproject */
   m_canvasView->getCanvas()->slMoveItems( y, childCnt*m_itemHeight );

   QListViewItem* child = it->firstChild();
   while( child )
   {
      m_canvasView->getCanvas()->slShowTask(static_cast<KTVTaskTableItem*>(child));

      /* No longer connected to internal reasons, but maybe extern */
      emit showTaskByItem( static_cast<KTVTaskTableItem*>(child) );

      child = child->nextSibling();
   }
   slUpdateCanvas();
}


void KTVTaskTable::slSelectionChanged( QListViewItem *it )
{
   if( ! it ) return;

   int y = it->itemPos();

   emit moveMarker( y- it->height() );

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
   if( m_canvasView )
   {
      int width = m_canvasView->contentsWidth();
      if( m_canvasView->contentsHeight() != h )
	 m_canvasView->resizeContents(width, h);
   }

}


void KTVTaskTable::slScrollTo( int, int y )
{
   emit scrolledBy( 0, y - contentsY() );
}


