#include <klocale.h>

#include "ktvtasktable.h"


KTVTaskTableItem::KTVTaskTableItem( KTVTaskTable *parent, int height )
   :KListViewItem( parent ),
    m_task(0)
{
   setHeight(height);
}

KTVTaskTableItem::KTVTaskTableItem( KTVTaskTableItem *parent, Task *t, int height )
   :KListViewItem( parent ),
    m_task(t)
{
   // qDebug("Creating item for task " + t->getId() );
   setHeight(height);
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

       time_t itStart = iTask->getStart(0); // Task::Plan);
       time_t itEnd = iTask->getEnd(0); // Task::Plan);
       time_t mtStart = mTask->getStart(0);  //Task::Plan);
       time_t mtEnd = mTask->getEnd(0); // Task::Plan);
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
	     me = mTask->getComplete(0); // Task::Plan);
	     other = iTask->getComplete(0); // Task::Plan);

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
