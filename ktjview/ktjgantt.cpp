/*
 * TaskJuggler Viewer
 *
 * Copyright (c) 2001, 2002 by Klaas Freitag <freitag@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */
#include "ktjgantt.h"

#include <kinstance.h>
#include <kaction.h>
#include <kstdaction.h>
#include <kfiledialog.h>
#include <kdebug.h>
#include <kaction.h>
#include <qsplitter.h>
#include <klocale.h>

#include <qvaluelist.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qmultilineedit.h>

#include "ktvtasktable.h"
#include "ktvtaskcanvasview.h"
#include "ktvheader.h"
#include "timedialog.h"
#include "Utility.h"
#include <qvbox.h>


KTJGantt::KTJGantt( QWidget *parentWidget, const char *)
    : QSplitter( parentWidget ),
      m_canvas(0L),
      m_header(0L),
      m_table(0L),
      m_weekStartMon( true ),
      m_doTableMove(true),
      m_timeDialog(0L)
{
    m_table  = new KTVTaskTable( this, "TABLE");
    m_ganttBox = new QVBox( this );
    m_ganttBox->setFrameStyle( QFrame::NoFrame );
    m_ganttBox->setMargin(0);
    m_ganttBox->setSpacing(0);

    m_ganttBox->setLineWidth(0);
    m_ganttBox->setBackgroundColor( QColor(red));
    m_header = new KTVHeader( m_ganttBox, "HEADER");
    m_header->setFrameStyle( QFrame::NoFrame );
    m_header->setMargin(0);
    // m_header->setSpacing(0);

    m_canvas = new KTVTaskCanvasView( m_ganttBox, m_table, m_header, "CANVAS");
    m_canvas->setFrameStyle( QFrame::NoFrame );
    m_canvas->setMargin(0);

    m_header->slSetHeight( m_table->headerHeight());

    m_table->setCanvasView( m_canvas );

    /* synchron y scroll */
    connect( m_canvas, SIGNAL( contentsMoving( int, int )),
	     this,     SLOT( slCanvasMoved( int, int )));
    connect( m_table,  SIGNAL( contentsMoving( int, int )),
	     this,     SLOT( slTableMoved(int, int)));

    connect( m_table, SIGNAL(newTaskAdded(Task *, KTVTaskTableItem *)),
	     m_canvas->canvas(), SLOT(slNewTask(Task *, KTVTaskTableItem *) ));

    connect( m_table, SIGNAL(moveMarker(int)),
    	     m_canvas->canvas(), SLOT(slShowMarker(int)) );

    connect( m_canvas, SIGNAL(canvasClicked(time_t)),
             this, SLOT(slCanvasClicked( time_t )));
    QValueList<int> sizes;
    sizes.append( 200 );
    setSizes( sizes );

    setResizeMode( m_table, QSplitter::KeepSize );
    setResizeMode( m_ganttBox,  QSplitter::Stretch );
    // notify the part that this is our internal widget
    m_table->show();
    m_ganttBox->show();

}

void KTJGantt::clear()
{
    // removes all entries from the listview and sets the root to 0
    m_table->clear();

    // 
    m_canvas->clear();
    // todo header
    // m_header
}

void KTJGantt::showProject( Project *p )
{
    /* Prepare the draw operation */
    clear();
    
    setInterval( p->getStart(), p->getEnd());
    m_canvas->showProject( p );
    /* the table creates all tasks in both the table and the canvas */
    m_table->showProject( p );
    /* finalise the canvas */
    update();
}

void KTJGantt::setInterval( time_t start, time_t end )
{
    time_t oldStart = m_header->startTime();
    time_t oldEnd   = m_header->endTime();

    bool needNewInterval = false;

    if( oldStart == 0 || oldEnd == 0 )
    {
	needNewInterval = true;
    }
    else
    {
	if( oldStart != start ) {
	    /* We need to resize the canvas and move the items of the canvas
	     * by the difference
	     */
	    long diff = (m_header->timeToX(oldStart)) - (m_header->timeToX(start));
	    m_canvas->getCanvas()->slMoveItemsX(diff);
	    qDebug("Diff to move horizontal: %ld", diff );
	    needNewInterval = true;
	}

	if( oldEnd != end )
	{
	    /* We need to resize the canvas but do not have to move items */
	    needNewInterval = true;
	}
    }

    if( needNewInterval )
    {
	m_header->setInterval( start, end );
	m_canvas->syncInterval();
	update();
    }
}

#if 0 
void KTJGantt::update()
{
    QSplitter::update();
    m_header->repaint();
    m_canvas->repaint();
}
#endif

void KTJGantt::slZoomIn()
{
   m_canvas->zoomIn();
}

void KTJGantt::slZoomOut()
{
   m_canvas->zoomOut();
}

void KTJGantt::slZoomOriginal()
{
   m_canvas->zoomOriginal();
}

void KTJGantt::slTimeFrame()
{
    m_timeDialog = new TimeDialog( this,
				   m_header->startTime(),
				   m_header->endTime() );
    connect( m_timeDialog, SIGNAL( applyClicked()), this, SLOT( slTimeFromDialog() ) );
    connect( m_timeDialog, SIGNAL( okClicked()), this, SLOT( slTimeFromDialog()));
    m_timeDialog->exec();

}

void KTJGantt::slTimeFromDialog()
{
    if( m_timeDialog )
    {
        QDate dFrom = m_timeDialog->getStartDate();
        QDate dTo   = m_timeDialog->getEndDate();

	qDebug("New time interval: "+dFrom.toString() );

	setInterval( QDateTime(dFrom).toTime_t(), QDateTime(dTo).toTime_t() );

    }
}

void KTJGantt::slCanvasClicked( time_t t)
{
    emit statusBarChange( time2ISO( t ));
}

KTJGantt::~KTJGantt()
{
}

void KTJGantt::slSetWeekStartsMonday(bool t)
{
    m_weekStartMon = t;
}

void KTJGantt::slCanvasMoved( int x, int y )
{
    /* if the canvas moved, set the table accordingly. */
    if( m_doTableMove )
    {
	m_table->setContentsPos( m_table->contentsX(), y );
    }
    else
    {
	m_doTableMove = true;
	m_canvas->updateContents();
    }

    /* Header to move? */
    if( x != m_header->contentsX() )
    {
	m_header->setContentsPos( x, 0 );
    }
}

void KTJGantt::slTableMoved( int, int y )
{
    m_doTableMove = false;
    m_canvas->setContentsPos( m_canvas->contentsX(), y );
    
}

void KTJGantt::slToggleGanttVisible( void )
{
    bool isVisible = m_canvas->isVisible();
    if( isVisible )
	m_ganttBox->hide();
    else
	m_ganttBox->show();
    
}


#include "ktjgantt.moc"
