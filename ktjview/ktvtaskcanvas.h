/*
 * ktvtaskcanvas.h - TaskJuggler Viewer
 *
 * Copyright (c) 2001, 2002 by Klaas Freitag <freitag@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */
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
class KTVCanvasItemBase;
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
   int getDayWidth() { return m_dayWidth; }

    enum headerReso{ Day, Week, Month}; /* smallest reso of the header */
public slots:
   void slSetRowHeight(int);
   void slNewTask( Task *t, KTVTaskTableItem *it ){ m_tasks.insert( it, t ); }
   void slShowTask( KTVTaskTableItem* );
   void slHideTask( KTVTaskTableItem* );
   void slMoveItems( int, int );
   void slShowMarker( int );
   void slSetTopOffset( int );
   void slShowDebugMarker( int );

   /**
    * searches the according KTVCanvasItemBase object for a QCanvasItem.
    * QCanvasItems are deliverd by a lot of 'I was clicked'-functions of the
    * canvas and we mostly like to know what KTVCanvasItem we are talking about.
    */
   KTVCanvasItemBase* qCanvasItemToItemBase( QCanvasItem* );
   void slSetDayWidth( int );
   void slSetDayWidthStandard();

protected:
    enum topOffsetPart { Complete, Upper, Lower };
    time_t            timeFromX( int x );
    int                 timeToX( time_t );
    int             midnightToX( time_t );
    virtual int       topOffset( topOffsetPart part = Complete );

    virtual void    drawDayHead( QFont &f, time_t time, int x, QPainter &p );
    virtual void   drawWeekHead( const QFont &f, time_t time, int x, QPainter &p );
    virtual void  drawMonthHead( const QFont &f, time_t time, int x, QPainter &p );


   void connectTasks( Task *fromTask, Task* toTask,
		      KTVCanvasItemBase *fromItem =0L,
		      KTVCanvasItemBase *toItem =0L    );
protected slots:
   void   slHeightChanged( int );

private:
   KTVCanvasItemBase* tableItemToCanvasItem( const KTVTaskTableItem* ) const;
   KTVCanvasItemBase* taskToCanvasItem( const Task* ) const;
   void drawBackground( QPainter &painter, const QRect & clip );
   int m_days, m_dayWidth, m_dayHeight;
   int m_topOffset;
   KTVTaskTable *m_taskTable;
   time_t m_start, m_end;
   QPtrDict<Task> m_tasks;    			 // Stores the Tasks for the TableItems
   QPtrDict<KTVCanvasItemBase> m_canvasItems;    // Stores Canvas-Items for the Tasks


   CanvasItemList    m_canvasItemList;
   QCanvasLine      *m_dbgMark;
   QCanvasRectangle *m_canvasMarker;

   int    m_rowHeight;

    int   m_monthStartX;
    int   m_weekStartX;
};


#endif

