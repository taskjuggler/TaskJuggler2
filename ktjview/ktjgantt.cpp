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
    QVBox *vbox = new QVBox( this );
    vbox->setFrameStyle( QFrame::NoFrame );
    vbox->setMargin(0);
    vbox->setSpacing(0);

    vbox->setLineWidth(0);
    vbox->setBackgroundColor( QColor(red));
    m_header = new KTVHeader( vbox, "HEADER");
    m_header->setFrameStyle( QFrame::NoFrame );
    m_header->setMargin(0);
    // m_header->setSpacing(0);

    m_canvas = new KTVTaskCanvasView( vbox, m_table, m_header, "CANVAS");
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
    setResizeMode( vbox,  QSplitter::Stretch );
    // notify the part that this is our internal widget
    m_table->show();
    vbox->show();

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
    m_start = start;
    m_end = end;
    m_header->setInterval( start, end );
}

void KTJGantt::slZoomIn()
{
   m_canvas->zoomIn();
   update();
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
    qDebug("Changing Time Frame" );

    m_timeDialog = new TimeDialog( this, m_start, m_end );
    connect( m_timeDialog, SIGNAL( applyClicked()), this, SLOT( slTimeFromDialog() ) );
    m_timeDialog->exec();

}

void KTJGantt::slTimeFromDialog()
{
    if( m_timeDialog )
    {
        QDate dFrom = m_timeDialog->getStartDate();
        QDate dTo   = m_timeDialog->getEndDate();


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

#include "ktjgantt.moc"
