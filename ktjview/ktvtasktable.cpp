#include <qdatetime.h>
#include <klocale.h>
#include <kglobal.h>

#include "ktvtasktable.h"
#include "Project.h"
#include "Task.h"

KTVTaskTable::KTVTaskTable( QWidget *parent, const char *name )
   :KListView( parent, name ),
    m_root(0)
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

   setShowSortIndicator( true );

}


void KTVTaskTable::showProject( Project *p )
{
   if( m_root )
      delete m_root;

   qDebug( " ++++++ Starting to show +++++++" );

   QString pName = p->getName();

   m_root = new KTVTaskTableItem( this );
   m_root->setExpandable( true );
   m_root->setOpen( true );
   
   m_root->setText( COL_NAME, i18n("Project: %1").arg(pName) );

   TaskList taskList = p->getTaskList();

   for (Task* t = taskList.first(); t != 0; t = taskList.next())
   {
      if( t->getParent() == 0 && t->isContainer() )
	 addTask( static_cast<KTVTaskTableItem*>(m_root), t );
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
   qDebug( "Adding task: " + t->getId());
   QDateTime dt;
   
   KTVTaskTableItem *ktv =  new KTVTaskTableItem( parent, t );
   ktv->setText( COL_NAME, name );
   ktv->setText( COL_ID, t->getId());
//    ktv->setText( 1, idx );

   ktv->setText( COL_PLAN_LEN, beautyTimeSpan( t->getPlanEnd(), t->getPlanStart() ));
   ktv->setText( COL_PRIORITY, QString::number( t->getPriority() ));
   
   int cmplt = t->getComplete();
   ktv->setText( COL_COMPLETE, cmplt == -1 ? i18n("iP"):  i18n("%1%").arg(cmplt));
   
   dt.setTime_t( t->getPlanStart() );
   ktv->setText( COL_PLAN_START_DATE, KGlobal::locale()->formatDate( dt.date(), true ));
   ktv->setText( COL_PLAN_START_TIME, KGlobal::locale()->formatTime( dt.time(), false ));
   
   dt.setTime_t( t->getPlanEnd() );
   ktv->setText( COL_PLAN_END_DATE, KGlobal::locale()->formatDate( dt.date(), true ));
   ktv->setText( COL_PLAN_END_TIME, KGlobal::locale()->formatTime( dt.time(), false ));
   
   TaskList subTasks;
   subTasks.clear();
   
   t->getSubTaskList(subTasks);
   int cnt = subTasks.count();
   qDebug( "Amount of subtasks: " + QString::number(cnt) );

   for (Task* st = subTasks.first(); st != 0; st = subTasks.next())
   {
      qDebug( "Calling subtask " + st->getId() + " from " + t->getId() );
      addTask( ktv, st );
      qDebug( "Calling subtask " + st->getId() + " from " + t->getId() + " <FIN>" );
   }
   
   
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
