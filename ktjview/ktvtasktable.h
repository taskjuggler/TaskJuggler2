#ifndef _KTVTASKTABLE_H
#define _KTVTASKTABLE_H


#include <klistview.h>
#include <Project.h>
#include <Task.h>

#include <qpixmap.h>

#include "ktvtasktableitem.h"

#define COL_NAME 0
#define COL_ID 1
#define COL_PLAN_LEN 2
#define COL_PRIORITY 3
#define COL_COMPLETE 4
#define COL_PLAN_START_DATE 5
#define COL_PLAN_START_TIME 6

#define COL_PLAN_END_DATE 7
#define COL_PLAN_END_TIME 8

class QPixmap;

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

   QPixmap m_milestonePix;
   QPixmap m_taskPix;
   QPixmap m_containerPix;
   QPixmap m_menPix;
};


#endif

