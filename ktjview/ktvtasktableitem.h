#ifndef _KTVTASKTABLEITEM_H
#define _KTVTASKTABLEITEM_H


#include <klistview.h>
#include <Project.h>
#include <Task.h>

class KTVTaskTable;

class KTVTaskTableItem: public KListViewItem
{
public:
    KTVTaskTableItem( KTVTaskTable *parent );
    KTVTaskTableItem( KTVTaskTableItem *parent, Task* );
   int compare( QListViewItem *i, int col, bool ascending ) const;

   Task *getTask() { return m_task; }


private:
   int compareNumeric( long other, long me );
   int compareTimes( time_t other, time_t me );
   Task *m_task;
};


#endif

