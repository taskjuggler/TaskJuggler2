/*
 * $RCSfile$ - TaskJuggler Viewer
 *
 * Copyright (c) 2001, 2002 by Klaas Freitag <freitag@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

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
#include "ktvheader.h"
#include "Project.h"
#include "Task.h"
#include "Utility.h"
#include "tasktip.h"
#include <qevent.h>


KTVTaskCanvasView::KTVTaskCanvasView( QWidget *parent, KTVTaskTable* tab, KTVHeader *h, const char *name )
   :QCanvasView( parent, name ),
    m_canvas(0),
    m_table(tab),
    m_header(h),
    m_scaleFactor(1.0),
    m_suppressScroll(false)
{
    m_canvas = new KTVTaskCanvas( parent, tab, h, name );
    setCanvas( m_canvas );
    (void) new TaskTip( this );

    setResizePolicy( QScrollView::Default);
    setVScrollBarMode( QScrollView::AlwaysOn );

}


void KTVTaskCanvasView::showProject( Project *p )
{
   qDebug( " ++++++ Starting to show +++++++" );
   m_pro = p;

   /* resize Contents */
   int w = m_header->overallWidth();
   int h = m_table->itemHeight() * (p->taskCount());
   qDebug("Resizing canvas to %dx%d", w, h );
   m_canvas->resize( w, h );
}

void KTVTaskCanvasView::syncInterval()
{
   int w = m_header->overallWidth();
   int h = m_canvas->height();
   qDebug("Resizing canvas to %dx%d", w, h );
   m_canvas->resize( w, h );
   updateContents();
    
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

/*
 * returns the time that is currently centered in the viewport.
 */
time_t KTVTaskCanvasView::getCenterTime()
{
    int x = contentsX() + visibleWidth()/2;
    time_t t = m_header->timeFromX( x );

    qDebug("getCenterTime: %s of x=%d", (const char*) time2ISO(t), x );
    return( t );
}

/**
 * ensure that the given time is visible at a p percent of the viewport
 * width from the left side.
 *
 */
void KTVTaskCanvasView::xScrollToTime( int p, time_t ti )
{
    int timeX = m_header->timeToX( ti );

    if( p > 0 && p <= 100 )
    {
	int delta = int(double(p)/100.0 * double(visibleWidth()));
	qDebug("Delta value ist %d at visible width %d", delta, visibleWidth() );
        timeX += delta;
    }
    qDebug( "Centering on %d of %d", timeX, contentsWidth() );
    setContentsPos( timeX, contentsY() );
}

void KTVTaskCanvasView::zoomIn()
{
    int w = m_header->dayWidth();
    time_t centerTime = getCenterTime();
    qDebug("getCenterTime1: %s", (const char*) time2ISO(centerTime) );

    if( w < 60 )
	w += 5;

    qDebug("Setting daywidth %d", w );
    m_header->slSetDayWidth( w );
    xScrollToTime( 50, centerTime );
    syncInterval();
    m_canvas->slUpdateTasks();
    updateContents();
}

void KTVTaskCanvasView::zoomOut()
{
    int w = m_header->dayWidth();
    time_t centerTime = getCenterTime();

    if( w > 14 )
	w -= 5;

    qDebug("Setting daywidth %d", w );
    m_header->slSetDayWidth( w );
    int perc = 50;
    xScrollToTime( perc, centerTime );
    syncInterval();
    m_canvas->slUpdateTasks();
    updateContents();

}

void KTVTaskCanvasView::zoomOriginal()
{
    time_t centerTime = getCenterTime();
    // m_canvas->slSetDayWidthStandard();
    xScrollToTime( 50, centerTime );
    syncInterval();
    m_canvas->slUpdateTasks();
    updateContents();
}

void KTVTaskCanvasView::clear()
{
    m_pro = 0;
    if( m_canvas )
    {
        m_canvas->clear();
        m_canvas->resize(0,0);
    }
}


void KTVTaskCanvasView::contentsMousePressEvent ( QMouseEvent *e )
{
    qDebug( "Mouse-Move at %d %d", e->pos().x(), e->pos().y());
    emit canvasClicked( m_header->timeFromX( e->pos().x()));
}

