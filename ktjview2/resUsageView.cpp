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
#include "settings.h"
#include "ktjUtils.h"

// TJ includes
#include "Utility.h"

// Qt includes
#include <qrect.h>
#include <qpalette.h>
#include <qstringlist.h>
#include <qpainter.h>
#include <qpopupmenu.h>
#include <qclipboard.h>

//KDE includes
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kapplication.h>
#include <kmessagebox.h>

#include <time.h>

ResUsageView::ResUsageView( QWidget * parent, const char * name )
    : QTable( parent, name ), m_proj( 0 ), m_display( DIS_LOAD_AND_TOTAL ), m_findDia( 0 )
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
    connect( this, SIGNAL( contextMenuRequested ( int, int, const QPoint & ) ),
             this, SLOT( slotPopupMenu( int, int, const QPoint & ) ) );

    setScale( SC_DAY );
}


ResUsageView::~ResUsageView()
{
    delete m_findDia;
}

void ResUsageView::resizeData( int len )
{
    Q_UNUSED( len );
}

void ResUsageView::paintCell( QPainter * p, int row, int col, const QRect & cr, bool selected, const QColorGroup & cg )
{
    if ( m_resList.isEmpty() || !m_proj )
        return;

    Resource * res = resourceForRow( row );
    if ( !res )
        return;

    //kdDebug() << "Painting cell, resource: " << res->getName() << endl;

    Interval ival = intervalForCol( col );
    if ( ival.isNull() )
        return;

    //kdDebug() << "Painting cell, interval: (" << ival.getStart()
    //          << ", " << ival.getEnd() << ")" << endl;

    double load = res->getLoad( 0, ival );
    //kdDebug() << "Painting cell, load: " << load << endl;

    //double aload = res->getAvailableWorkLoad( 0, ival );
    //kdDebug() << "Painting cell, available workload: " << aload << endl;

    QRect cRect = cellRect( row, col );
    p->eraseRect( cRect );
    cRect.setRect( cRect.left(), cRect.top(), cRect.width() - 1, cRect.height() - 1 );
    p->setClipRect( cRect, QPainter::CoordPainter );
    QTable::paintCell( p, row, col, cr, selected, cg );

    if ( load > 0 )             // allocated cell
        p->fillRect( cRect, Qt::lightGray ); // TODO make color configurable
    else
        p->fillRect( cRect, Settings::freeTimeColor() ); // free cell

    if ( KtjUtils::isVacationInterval( res, ival ) ) // vacations, sicktime
        p->fillRect( cRect, Settings::vacationTimeColor() );

    if ( m_proj && ( getWorkingDays( ival ) == 0 ) ) // holidays and weekends
        p->fillRect( cRect, Settings::holidayTimeColor() );


    // TODO perhaps would be better to draw a rectangle *around* for vacations and holidays

    p->drawText( cRect, Qt::AlignCenter, text( row, col ) );
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
        QString header = res->getName();
        uint level = res->treeLevel();
        if ( level > 0 )
        {
            QString pad;
            pad.fill( ' ', 2 * level );
            header.prepend( pad );
        }
        m_rowLabels.append( header );
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
    m_proj = 0;
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

Resource * ResUsageView::resourceForRow( int row ) const
{
    return static_cast<Resource *>( resList().at( row ) );
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
    QString format;             // corresponds with strftime(3)

    switch ( m_scale )
    {
    case SC_DAY:
        format = "%d/%m";
        break;
    case SC_WEEK:
        format = "%V/%y";
        break;
    case SC_MONTH:
        format = "%b/%y";
        break;
    case SC_QUARTER:
        format = "Q%q/%y";
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

void ResUsageView::slotPopupMenu( int, int, const QPoint & pos )
{
    if ( m_resList.isEmpty() || !m_proj )
        return;

    QPopupMenu * menu = new QPopupMenu( this, "cell_popup" );
    menu->insertItem( i18n( "&Copy" ), this, SLOT( slotCopy() ) );
    menu->exec( pos );
    delete menu;
}

void ResUsageView::slotCopy()
{
    QClipboard *cb = kapp->clipboard();
    cb->setText( text( currentRow(), currentColumn() ), QClipboard::Clipboard );
}

QString ResUsageView::text( int row, int col ) const
{
    if ( m_resList.isEmpty() || !m_proj )
        return QString::null;

    Resource * res = resourceForRow( row );
    if ( !res )
        return QString::null;

    //kdDebug() << "Getting text, resource: " << res->getName() << endl;

    Interval ival = intervalForCol( col );
    if ( ival.isNull() )
        return QString::null;

    //kdDebug() << "Getting text, interval: (" << ival.getStart()
    //          << ", " << ival.getEnd() << ")" << endl;

    double daily = res->getProject()->getDailyWorkingHours(); // TODO cache this value

    //kdDebug() << "Getting text, daily/days: " << daily << endl;

    double load = res->getLoad( 0, ival ) * daily;

    double aload = res->getAvailableWorkLoad( 0, ival ) * daily;

    double total = load + aload;

    //kdDebug() << "Getting text, load/aload/total: " << load << " / " << aload << " / " << total << endl;

    QString text;
    switch ( m_display )
    {
    case DIS_LOAD_AND_TOTAL:
        text = QString( "%1/%2" )
               .arg( KGlobal::locale()->formatNumber( load, 0 ) )
               .arg( KGlobal::locale()->formatNumber( total, 0 ) );
        break;
    case DIS_LOAD:
        if (load == 0)
        {
            text = QString(" ");
        }
        else
        {
            text = QString( "%1" ).arg( KGlobal::locale()->formatNumber( load, 0 ) );
        }
        break;
    case DIS_FREE:
        text = QString( "%1" ).arg( KGlobal::locale()->formatNumber( aload, 0 ) );
        break;
    default:
        kdWarning() << "Unsupported display type in ResUsageView::text" << endl;
        break;
    }

    return text;
}

void ResUsageView::setProject( Project * proj )
{
    m_proj = proj;
}

void ResUsageView::find()
{
    if ( !m_findDia )
    {
        m_findDia = new ruFindDlg( m_rowLabels, this, "ru_find_dialog" );
        connect( m_findDia, SIGNAL( signalMatch( int ) ), this, SLOT( slotFoundMatch( int ) ) );
        connect( m_findDia, SIGNAL( closeClicked() ), this, SLOT( slotCloseFindDialog() ) );
    }

    m_findDia->show();
    m_findDia->raise();
}


void ResUsageView::slotCloseFindDialog()
{
    m_findDia->delayedDestruct();
    m_findDia = 0;
}

void ResUsageView::slotFoundMatch( int match )
{
    if ( match == -1 )          // nothing found
    {
        slotCloseFindDialog();
        KMessageBox::sorry( this, i18n( "No matching resource found." ) );
    }
    else if ( match == -2 )
    {
        slotCloseFindDialog();
        KMessageBox::sorry( this, i18n( "No more matching resources found." ) );
    }
    else                        // found a matching resource, select the row
    {
        selectRow( match );
    }
}

void ResUsageView::setDisplayData( int data )
{
    m_display = static_cast<DisplayData>( data );
    repaintContents( true );
}

int ResUsageView::getWorkingDays( const Interval & ival ) const
{
    int workingDays = 0;

    for ( time_t s = midnight( ival.getStart() ); s < ival.getEnd(); s = sameTimeNextDay( s ) )
        if ( m_proj->isWorkingDay( s ) )
            workingDays++;

    return workingDays;
}

#include "resUsageView.moc"
