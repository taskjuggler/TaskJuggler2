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
#include "ktvheader.h"
#include "Utility.h"

#include <qcolor.h>
#include <qscrollview.h>
#include <qlabel.h>
#include <qpainter.h>
#include <qsizepolicy.h>
#include <qfont.h>
#include <qpalette.h>
#include <klocale.h>
#include <kapplication.h>
#include <kstyle.h>


KTVHeader::KTVHeader( QWidget *parentWidget, const char *name)
    : QScrollView( parentWidget, name ),
      m_height(80),
      m_start(0L),
      m_end(0L),
      m_dayWidth(24),   // TODO: variable
      m_weekStartMon(false)
{
    setHScrollBarMode( AlwaysOff );
    setVScrollBarMode( AlwaysOff );

    m_headerFont.setFamily( "Helvetica [Cronyx]" );
    m_headerFont.setPointSize(8);

    // setSizePolicy( QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Expanding ));
}

KTVHeader::~KTVHeader()
{
}

void KTVHeader::slSetHeight( int h )
{
    m_height = h;
    setFixedHeight(h);
    int w = overallWidth();
    int sbw = kapp->style().pixelMetric( QStyle::PM_ScrollBarExtent );
    qDebug("Subtracting scrollbar width %d from overall %d", w, sbw );
    resizeContents( w+sbw, m_height );
}

int KTVHeader::overallWidth( )
{
    int days = 1 + daysBetween( m_start, m_end );
    return days*m_dayWidth;
}

void KTVHeader::setInterval( time_t start, time_t end )
{
    m_start = start;
    m_end = end;
    slSetDayWidth( m_dayWidth ); // to trigger resizing of contents

}

void KTVHeader::slSetDayWidth(int dayW)
{
    m_dayWidth = dayW;
    int w = overallWidth();
    int sbw = kapp->style().pixelMetric( QStyle::PM_ScrollBarExtent );
    resizeContents( w+sbw, m_height );
}

void KTVHeader::drawContents( QPainter *p, int clipx, int clipy, int clipw, int cliph )
{
    // Calculate the clipping coordinates...
    if( m_start == 0 || m_end == 0 ) return;
    int x1 = 0, y1 = 0;
    int x2 = overallWidth(), y2 = m_height;

    // Clip the coordinates so X/Windows will not have problems...
    if (x1 < clipx) x1=clipx;
    if (y1 < clipy) y1=clipy;
    if (x2 > clipx+clipw-1) x2=clipx+clipw-1;
    if (y2 > clipy+cliph-1) y2=clipy+cliph-1;

    QPen p1 ( QColor(222,222,222), 0 );
    QColorGroup cg( this->colorGroup());
    QPen blackPen( cg.color(QColorGroup::Foreground) );

    // Base filling
    p->setFont( m_headerFont );
    p->setPen( NoPen );
    // QBrush bgBrush( QColor(238,238,205)); // cg.brush( QColorGroup::Base);
    QBrush bgBrush( cg.brush( QColorGroup::Base));
    if ( x2 >= x1 && y2 >= y1 )
        p->fillRect(x1, y1, x2-x1+1, y2-y1+1, bgBrush );
    // Paint using the small coordinates...
    p->setPen(p1);

    // time_t trun = m_start;
    time_t trun = midnight(timeFromX( clipx ));
    QBrush weekEndBrush( QColor(246,242,222));
    //
    // show the weekends - draw sat and sun in light yellow
    //
    int x = -1;
    if( isWeekend(trun) ) // check if it is already weekend
    {
        // go to monday.
        while(isWeekend(trun)) trun = sameTimeNextDay(trun);
        trun = midnight(trun);  // monday morning.
        x = timeToX( trun-1);
        p->fillRect( 0,0,x,topOffset(All)-1, weekEndBrush );

        for( int i=0; i < 5; i++)  // go to saturday morning
            trun = sameTimeNextDay(trun);
    }
    else
    {
        // forward until weekend found.
        while( ! isWeekend( trun )) trun = sameTimeNextDay(trun);
        // ok, now trun is on a saturday at any time
        trun = midnight(trun); // go to start of saturday
    }
    x = timeToX(trun);
    while ( x < x2 ) {
        p->fillRect( x, 0, 2*m_dayWidth, topOffset(All), weekEndBrush);
        x += (7*m_dayWidth);
    }
    // weekend background finished

    //
    // horzontal lines
    //
    p->setPen(p1);
    p->drawLine(x1, topOffset(Day), x2, topOffset(Day));
    p->setPen(blackPen);
    p->drawLine(x1, topOffset(Week), x2, topOffset(Week));
    p->drawLine(x1, topOffset(All)-1, x2, topOffset(All)-1);

    //
    // Loops over days
    //
    // qDebug( "Y-Value for Days: %d", yDays );
    trun = m_start;  // m_start is on midnight
    int days = daysInInterval();
    for( int i = 0; i < days; i++ )
    {
        int dayX= timeToX(trun);
        int day = dayOfMonth(trun);
        p->setPen( p1 );
        // p->drawLine( dayX, topOffset(Day), dayX, topOffset(All)-1);
        p->drawLine( dayX, 1, dayX, topOffset(All)-1);

        p->setPen( blackPen );
        p->drawText( dayX+1, topOffset(Day)+1,   // x, y
                     m_dayWidth-2, topOffset(All)-topOffset(Day)-2, // w,h
                     Qt::AlignCenter, QString::number(day) );
        trun = sameTimeNextDay(trun);
    }

    trun = m_start;  // m_start is on midnight
    bool firstWeek = true;
    bool firstMon  = true;
    for( int i = 0; i < days; i++ )
    {
        int dayX= timeToX(trun);
        // check for week
        if( dayOfWeek(trun, m_weekStartMon ) == 0 )
        {
            // week begins here.
            // p->setPen( p1 );
            p->setPen( blackPen );
            p->drawLine( dayX, topOffset(Week), dayX, topOffset(All));

            QString weekStr = i18n("Week %1").arg(weekOfYear(trun-ONEDAY, m_weekStartMon));
            if( firstWeek )
            {
                firstWeek = false;
                QFontMetrics fm( m_headerFont );
                if( dayX-2 > fm.width(weekStr))
                    p->drawText( 1, topOffset(Week)+1,
                                 dayX-2, topOffset(Day)-topOffset(Week)-2,
                                 Qt::AlignCenter,
                                 weekStr );
            }
            else
            {
                p->drawText( dayX+1-m_dayWidth*7, topOffset(Week)+1,
                             m_dayWidth*7-2, topOffset(Day)-topOffset(Week)-2,
                             Qt::AlignCenter,
                             weekStr );
            }
        }

        // check for month
        if( dayOfMonth(trun) == 1 )
        {
            // a new month begins here, draw a vertical line
            p->setPen( blackPen );
            p->drawLine( dayX, topOffset(Month), dayX, topOffset(Week));

            QString mStr = monthAndYear( trun - ONEDAY /* need _last_ month */ );
            if( firstMon )
            {
                firstMon = false;
                QFontMetrics fm( m_headerFont );
                if( dayX-2 > fm.width(mStr))  // only if fitting in
                    p->drawText( 1, topOffset(Month)+1,
                                 dayX-2, topOffset(Week)-topOffset(Month)-2,
                                 Qt::AlignCenter,
                                 mStr );
            }
            else
            {
                p->drawText( dayX+1, topOffset(Month)+1,
                             m_dayWidth*daysLeftInMonth(trun)-2, topOffset(Week)-topOffset(Month)-2,
                             Qt::AlignCenter,
                             mStr );
            }
        }

        trun = sameTimeNextDay(trun);
    }
}

int KTVHeader::getHeaderLineHeight( topOffsetPart p )
{
   QFontMetrics fm( m_headerFont );
   int lineHeight = fm.height()+2;
   int re=0;
   // qDebug( "TopOffset: %d and lineHeight: %d", m_topOffset, lineHeight );
   switch( p )
   {
      case Month:
	 re = m_height-(2*lineHeight);
	 break;
      case Week:
      case Day:
	 re = lineHeight;
	 break;
      case All:
	 re = m_height;
	 break;
      default: /* is Day */
	 re = lineHeight;
   }
   // qDebug( "returning value %d for p %d", re, int(p) );

   return re;
}

int  KTVHeader::topOffset( topOffsetPart part )
{
   int re = 0;

   switch( part )
   {
      case Month:
	 re = 0;
	 break;
      case All:
	 re = m_height;
	 break;
      case Week:
	 // re = 1+getHeaderLineHeight( Month );
	 re = getHeaderLineHeight(All)- getHeaderLineHeight( Day )
	    - getHeaderLineHeight( Week );
	 break;
      case Day:
	 re = getHeaderLineHeight(All)- getHeaderLineHeight( Day );
	 break;
   }
   return re;
}


int KTVHeader::daysInInterval()
{
    return daysBetween( m_start, m_end );
}

time_t KTVHeader::timeFromX( int x )
{
    if( !m_dayWidth ) return m_start;
    // qDebug("MStart ist %ld", m_start );
    return m_start + time_t(double(ONEDAY) * double(x)/double(m_dayWidth));
}

int KTVHeader::midnightToX( time_t t )
{
    if( t < m_start ) return 0;
    if( t > m_end ) return width();

    return m_dayWidth * daysBetween( m_start, midnight(t) );
}

int KTVHeader::timeToX
( time_t t )
{
    if( t < m_start ) return 0;
    if( t > m_end ) return width();

    double p = double(m_dayWidth)/double(ONEDAY);

    return int( p* double(t-m_start));
}




// #include "ktvheader.moc"
