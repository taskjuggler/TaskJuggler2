#ifndef _KTVTASKTABLE_H
#define _KTVTASKTABLE_H


#include <klistview.h>
#include <Project.h>
#include <Task.h>

#include "ktvtasktableitem.h"

#define COL_NAME 0
#define COL_PLAN_LEN 1
#define COL_PRIORITY 2
#define COL_COMPLETE 3
#define COL_PLAN_START_DATE 4
#define COL_PLAN_START_TIME 5

#define COL_PLAN_END_DATE 6
#define COL_PLAN_END_TIME 7


class KTVTaskTable: public KListView
{
    Q_OBJECT
public:
    KTVTaskTable( QWidget *parent, const char *name = 0 );
    virtual ~KTVTaskTable(){}

   void showProject( Project * );
private:
   void addTask( KTVTaskTableItem *parent, Task *t );
   QString beautyTimeSpan( time_t tStart, time_t tEnd ) const;
   
   KTVTaskTableItem *m_root;
};


#endif

