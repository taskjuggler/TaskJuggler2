#include <klocale.h>

#include "ktvtasktable.h"


KTVTaskTableItem::KTVTaskTableItem( KTVTaskTable *parent )
   :KListViewItem( parent ),
    m_task(0)
{
   
}

KTVTaskTableItem::KTVTaskTableItem( KTVTaskTableItem *parent, Task *t )
   :KListViewItem( parent ),
    m_task(t)
{
   // qDebug("Creating item for task " + t->getId() );
}

#define  COMPARE(_me,_oth) ( _me==_oth? 0 : (_me<_oth?-1:1))

int KTVTaskTableItem::compare( QListViewItem *i, int col, bool ascending ) const
{
   Task *iTask = static_cast<KTVTaskTableItem*>(i)->getTask();
   Task *mTask = m_task;
   int r = 1;

   // qDebug( "SORTING" );
   
   if( mTask && iTask )
   {

      time_t itStart = iTask->getPlanStart();
      time_t itEnd = iTask->getPlanEnd();
      time_t mtStart = mTask->getPlanStart();
      time_t mtEnd = mTask->getPlanEnd();
      time_t me,other;
      me = other = 0;
      
   
      switch( col )
      {
	 case COL_PLAN_LEN:
	    other  = itEnd-itStart;
	    me     = mtEnd-mtStart;
	    r = COMPARE(me, other );
	    break;
	 case COL_PLAN_START_DATE:
	 case COL_PLAN_START_TIME:
	    me    = mtStart;
	    other = itStart;
	    r = COMPARE( me, other );
	    break;
	 case COL_PLAN_END_DATE:
	 case COL_PLAN_END_TIME:
	    me    = mtEnd;
	    other = itEnd;
	    r = COMPARE( me, other );
	    break;
	 case COL_PRIORITY:
	    me = mTask->getPriority();
	    other = iTask->getPriority();

	    r = COMPARE( me, other );
	    break;
	 case COL_COMPLETE:
	    me = mTask->getComplete();
	    other = iTask->getComplete();

	    r = COMPARE( me, other );
	    break;
	 case COL_NAME:
	 case COL_ID:
	 default:
	    return KListViewItem::compare(i, col, ascending );
	    break;
      }
   }
   return( r );
}

int KTVTaskTableItem::compareNumeric( long other, long me )
{
   if( me < other ) return( -1 );
   if( me > other ) return( 1 );
   return( 0 );
}
