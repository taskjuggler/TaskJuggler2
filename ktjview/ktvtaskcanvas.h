#ifndef _KTVTASKCANVAS_H
#define _KTVTASKCANVAS_H


#include <klistview.h>
#include <Project.h>
#include <Task.h>
#include <qcanvas.h>
#include <qptrdict.h>
#include <qpixmap.h>

#include "ktvtaskcanvas.h"
#include "ktvcanvasitem.h"

class KTVTaskCanvasItem;
class QCanvasRectangle;
class KTVTaskTableItem;
class KTVTaskTable;

class KTVTaskCanvas: public QCanvas
{
    Q_OBJECT
public:
    KTVTaskCanvas( QWidget *parent, KTVTaskTable *tab=0, const char *name = 0 );
    virtual ~KTVTaskCanvas(){}

   void setDays( int d ) { m_days = d; }

   void setInterval( time_t start, time_t end );
   void setTable( KTVTaskTable *tab );
   const CanvasItemList& getCanvasItemsList() const { return m_canvasItemList; }
   
public slots:
   void slSetRowHeight(int);
   void slNewTask( Task *t, KTVTaskTableItem *it ){ m_tasks.insert( it, t ); }
   void slShowTask( KTVTaskTableItem* );
   void slHideTask( KTVTaskTableItem* );
   void slMoveItems( int, int );
   void slShowMarker( int );
   void slSetTopOffset( int );
   void slShowDebugMarker( int );
   
protected:
   time_t timeFromX( int x );
   int    timeToX( time_t );
   int    midnightToX( time_t );
   virtual int    topOffset() { return m_topOffset; }
   virtual void   drawDayHead( int x, QPainter &p );

protected slots:
   void   slHeightChanged( int );
   
private:

   void drawBackground( QPainter &painter, const QRect & clip );
   int m_days, m_dayWidth, m_dayHeight;
   int m_topOffset;
   KTVTaskTable *m_taskTable;
   time_t m_start, m_end;
   QPtrDict<Task> m_tasks;    // Stores the Tasks for the TableItems
   QPtrDict<KTVCanvasItemBase> m_canvasItems;    // Stores Canvas-Items for the Tasks

   CanvasItemList    m_canvasItemList;
   QCanvasLine      *m_dbgMark;
   QCanvasRectangle *m_canvasMarker;
   
   int    m_rowHeight;
};


#endif

