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

#include "ktjReport.h"
#include "ktjUtils.h"

// TJ includes
#include "Utility.h"

// KDE includes
#include <kdebug.h>

KtjReport::KtjReport( Project * proj, KtjReportView * view )
    : m_proj( proj ), m_view( view )
{
    m_start = KtjUtils::time_t2Q( m_proj->getStart() );
    m_end = KtjUtils::time_t2Q( m_proj->getEnd() );
    setScale( SC_DAY );
    setDisplayData( DIS_LOAD );
}

KtjReport::Scale KtjReport::scale() const
{
    return m_scale;
}

void KtjReport::setScale( int sc )
{
    m_scale = static_cast<Scale>( sc );
}

QDateTime KtjReport::startDate() const
{
    return m_start;
}

void KtjReport::setStartDate( const QDateTime & date )
{
    m_start = date;
}

QDateTime KtjReport::endDate() const
{
    return m_end;
}

void KtjReport::setEndDate( const QDateTime & date )
{
    m_end = date;
}

KtjReport::DisplayData KtjReport::displayData() const
{
    return m_display;
}

void KtjReport::setDisplayData( int data )
{
    m_display = static_cast<DisplayData>( data );
}

Interval KtjReport::intervalForCol( int col ) const
{
    // get the start point, get the start of the next point (Utility.h) and calc the delta
    time_t intervalStart = m_start.toTime_t();
    time_t intervalEnd = intervalStart;
    QDateTime tmp;
    switch ( m_scale )
    {
    case SC_DAY:
        tmp = m_start.addDays( col );
        intervalEnd = sameTimeNextDay( tmp.toTime_t() );
        break;
    case SC_WEEK:
        intervalStart = beginOfWeek( intervalStart, true );
        tmp.setTime_t( intervalStart );
        tmp = tmp.addDays( col * 7 );
        intervalEnd = sameTimeNextWeek( tmp.toTime_t() );
        break;
    case SC_MONTH:
        intervalStart = beginOfMonth( intervalStart );
        tmp.setTime_t( intervalStart );
        tmp = tmp.addMonths( col );
        intervalEnd = sameTimeNextMonth( tmp.toTime_t() );
        break;
    case SC_QUARTER:
        intervalStart = beginOfQuarter( intervalStart );
        tmp.setTime_t( intervalStart );
        tmp = tmp.addMonths( col * 3 );
        intervalEnd = sameTimeNextQuarter( tmp.toTime_t() );
        break;
    default:
        kdWarning() << "Invalid scale in ResUsageView::intervalForCol" << endl;
        break;
    }

    return Interval( tmp.toTime_t(), intervalEnd );
}

QString KtjReport::formatDate( time_t date, QString format ) const
{
    if ( m_scale == SC_QUARTER )
        format.replace( "%q", QString::number( quarterOfYear( date ) ) ); // workaround against missing %q in strftime
    const struct tm* tms = localtime( &date );
    static char s[32];
    strftime(s, sizeof(s), format.latin1(), tms);
    return QString::fromLocal8Bit(s);
}
