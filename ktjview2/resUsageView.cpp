// -*- Mode: C++; c-basic-offset: 4; indent-tabs-mode: nil; tab-width: 4; -*-
/***************************************************************************
 *   Copyright (C) 2004 by Lukas Tinkl                                     *
 *   lukas.tinkl@suse.cz                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

// local includes
#include "resUsageView.h"

// TJ includes
#include "Utility.h"

// Qt includes
#include <qpainter.h>
#include <qrect.h>
#include <qpalette.h>
#include <qstringlist.h>
#include <qpainter.h>

//KDE includes
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>

#include <time.h>

ResUsageView::ResUsageView( QWidget * parent, const char * name )
    : QTable( parent, name )
{
    clear();
    setReadOnly( true );
    setSelectionMode( QTable::SingleRow );
    setSorting( false );
    setDragEnabled ( false );
    setRowMovingEnabled( false );
    setColumnMovingEnabled( false );
    setShowGrid( true );

    connect( this, SIGNAL( scaleChanged( Scale ) ), this, SLOT( updateColumns() ) );

    setScale( SC_DAY );
}


ResUsageView::~ResUsageView()
{
}

void ResUsageView::resizeData( int len )
{
    Q_UNUSED( len );
}

void ResUsageView::paintCell( QPainter * p, int row, int col, const QRect & cr, bool selected, const QColorGroup & cg )
{
    //kdDebug() << "Painting cell, row: " << row << " , col: " << col << endl;

    if ( m_resList.isEmpty() )
        return;

    Resource * res = resourceForRow( row );
    if ( !res )
        return;

    //kdDebug() << "Painting cell, resource: " << res << endl;

    Interval ival = intervalForCol( col );
    if ( ival.isNull() )
        return;

    //kdDebug() << "Painting cell, interval: " << ival << endl;

    double aload = res->getAvailableWorkLoad( 0, ival );

    //kdDebug() << "Painting cell, available workload: " << aload << endl;

    double load = res->getLoad( 0, ival ); // FIXME use getLoad() or getCurrentLoad() ???

    //kdDebug() << "Painting cell, load: " << load << endl;

    const QString text = QString( "%1 / %2" )
                         .arg( KGlobal::locale()->formatNumber( aload, 2 ) )
                         .arg( KGlobal::locale()->formatNumber( load, 2 ) );

    //kdDebug() << "Painting cell, text: " << text << endl;

    //kdDebug() << "===========================" << endl;

    QRect cRect = cellRect( row, col );
    p->setClipRect( cRect, QPainter::CoordPainter );
    // TODO set colors
    QTable::paintCell( p, row, col, cr, selected, cg );
    p->drawText( cRect, Qt::AlignCenter, text );
    p->setClipping( false );
}

void ResUsageView::assignResources( ResourceList reslist )
{
    m_resList = reslist;
    m_resList.sort();

    // get row labels
    Resource *res;
    for ( res = static_cast<Resource *>( reslist.first() ); res; res = static_cast<Resource *>( reslist.next() ) )
    {
        m_rowLabels.append( res->getName() );
    }

    // insert rows
    insertRows( 0, reslist.count() );
    //set row labels
    setRowLabels( m_rowLabels );

    updateColumns();
}

void ResUsageView::clear()
{
    m_start = m_end = QDateTime();
    setNumCols( 0 );
    setNumRows( 0 );
    m_resList.clear();
    m_rowLabels.clear();
}

Scale ResUsageView::scale() const
{
    return m_scale;
}

void ResUsageView::setScale( int sc )
{
    m_scale = static_cast<Scale>( sc );
    emit scaleChanged( m_scale );
}

QDateTime ResUsageView::startDate() const
{
    return m_start;
}

void ResUsageView::setStartDate( const QDateTime & date )
{
    m_start = date;
}

QDateTime ResUsageView::endDate() const
{
    return m_end;
}

void ResUsageView::setEndDate( const QDateTime & date )
{
    m_end = date;
}

Interval ResUsageView::intervalForCol( int col ) const
{
    // get the start point, get the start of the next point (Utility.h) and calc the delta

    // FIXME this is in the relative coords, we may need to rework it to real weeks/months/quarters, days should be fine

    QDateTime intervalStart = m_start;
    time_t intervalEnd = intervalStart.toTime_t();
    switch ( m_scale )
    {
    case SC_DAY:
        intervalStart = intervalStart.addDays( col );
        intervalEnd = sameTimeNextDay( intervalStart.toTime_t() );
        break;
    case SC_WEEK:
        intervalStart = intervalStart.addDays( col * 7 );
        intervalEnd = sameTimeNextWeek( intervalStart.toTime_t() );
        break;
    case SC_MONTH:
        intervalStart = intervalStart.addMonths( col );
        intervalEnd = sameTimeNextMonth( intervalStart.toTime_t() );
        break;
    case SC_QUARTER:
        intervalStart = intervalStart.addMonths( col * 3 );
        intervalEnd = sameTimeNextQuarter( intervalStart.toTime_t() );
        break;
    default:
        kdWarning() << "Invalid scale in ResUsageView::intervalForCol" << endl;
        break;
    }

    return Interval( intervalStart.toTime_t(), intervalEnd );
}

Resource * ResUsageView::resourceForRow( int row )
{
    return static_cast<Resource *>( m_resList.at( row ) );
}

void ResUsageView::updateColumns()
{
    if ( !m_start.isValid() || !m_end.isValid() )
        return;

    // clear the columns
    setNumCols( 0 );

    QStringList labels = getColumnLabels();

    // set the number of columns
    insertColumns( 0, labels.count() );

    // set the column labels
    setColumnLabels( labels );

    //kdDebug() << "Settings columns: " << labels << " , count: " << labels.count() << endl;
}

QStringList ResUsageView::getColumnLabels() const
{
    QString format;

    switch ( m_scale )
    {
    case SC_DAY:
        format = "%x";
        break;
    case SC_WEEK:
        format = "%V/%Y";
        break;
    case SC_MONTH:
        format = "%b/%Y";
        break;
    case SC_QUARTER:
        format = "Q%q/%Y";
        break;
    default:
        kdWarning() << "Invalid scale in ResUsageView::getColumnLabels" << endl;
        break;
    }

    time_t delta = intervalForCol( 0 ).getDuration();
    QStringList result;
    time_t tmp = m_start.toTime_t();
    time_t tmpEnd = m_end.toTime_t();

    //kdDebug() << "getColumnLabels: m_scale: " << m_scale << " , delta: " << delta <<
    //" , start: " << tmp << ", end: " << tmpEnd << endl;

    while ( tmp <= tmpEnd )
    {
        result.append( formatDate( tmp, format ) );
        tmp += delta;
    }

    return result;
}

QString ResUsageView::formatDate( time_t date, QString format ) const
{
    if ( m_scale == SC_QUARTER )
        format.replace( "%q", QString::number( quarterOfYear( date ) ) ); // workaround against missing %q in strftime
    const struct tm* tms = localtime( &date );
    static char s[32];
    strftime(s, sizeof(s), format.latin1(), tms);
    return QString::fromLocal8Bit(s);
}

#include "resUsageView.moc"
