// local includes
#include "resUsageView.h"

// TJ includes
#include "Utility.h"

// Qt includes
#include <qpainter.h>
#include <qrect.h>
#include <qpalette.h>
#include <qstringlist.h>

//KDE includes
#include <kdebug.h>

#include <time.h>

ResUsageView::ResUsageView( const QDateTime & start, const QDateTime & end, QWidget * parent, const char * name )
    : QTable( parent, name ), m_start( start ), m_end( end )
{
    setSelectionMode( QTable::SingleRow );
    setReadOnly( true );
    setFocusStyle( QTable::SpreadSheet );
    setSorting( false );
    setDragEnabled ( false );
    setRowMovingEnabled( false );
    setColumnMovingEnabled( false );

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
    if ( m_resList.isEmpty() )
        return;

    p->setClipRect( cellRect( row, col ), QPainter::CoordPainter );
    //... your drawing code
    p->setClipping( false );
}

void ResUsageView::assignResources( ResourceList reslist )
{
    m_resList = reslist;

    // get row labels
    Resource *res;
    for ( res = static_cast<Resource *>( reslist.first() ); res; res = static_cast<Resource *>( reslist.next() ) )
    {
        m_rowLabels.append( res->getName() );
    }
    m_rowLabels.sort();

    // insert rows
    insertRows( 0, reslist.count() );
    //set row labels
    setRowLabels( m_rowLabels );
}

void ResUsageView::clear()
{
    m_rowLabels.clear();
    setNumCols( 0 );
    setNumRows( 0 );
}

Scale ResUsageView::scale() const
{
    return m_scale;
}

void ResUsageView::setScale( Scale sc )
{
    m_scale = sc;
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
    QDateTime intervalStart = m_start;
    time_t intervalEnd = intervalStart.toTime_t();
    switch ( m_scale )
    {
    case SC_DAY:
        intervalStart.addDays( col );
        intervalEnd = sameTimeNextDay( intervalStart.toTime_t() );
        break;
    case SC_WEEK:
        intervalStart.addDays( col * 7 );
        intervalEnd = sameTimeNextWeek( intervalStart.toTime_t() );
        break;
    case SC_MONTH:
        intervalStart.addMonths( col );
        intervalEnd = sameTimeNextMonth( intervalStart.toTime_t() );
        break;
    case SC_QUARTER:
        intervalStart.addMonths( col * 3 );
        intervalEnd = sameTimeNextQuarter( intervalStart.toTime_t() );
        break;
    default:
        kdWarning() << "Invalid scale in ResUsageView::getInterval" << endl;
        break;
    }

    return Interval( intervalStart.toTime_t(), intervalEnd );
}

void ResUsageView::updateColumns()
{
    // clear the columns
    setNumCols( 0 );

    QStringList labels = getColumnLabels();

    // set the number of columns
    insertColumns( labels.count() );

    // set the column labels
    setColumnLabels( labels );
}

QStringList ResUsageView::getColumnLabels() const
{
    const char * format;

    switch ( m_scale )
    {
    case SC_DAY:
        format = "%d/%m/%y";
        break;
    case SC_WEEK:
        format = "%V/%y";
        break;
    case SC_MONTH:
        format = "%m/%y";
        break;
    case SC_QUARTER:
        format = "%q/%y";
        break;
    default:
        kdWarning() << "Invalid scale in ResUsageView::getColumnLabels" << endl;
        break;
    }

    time_t delta = intervalForCol( 1 ).getDuration();
    QStringList result;
    QDateTime tmp;
    for ( tmp = m_start; tmp <= m_end; tmp.addSecs( delta ) )
    {
        result.append( formatDate( tmp, format ) );
    }

    return result;
}

QString ResUsageView::formatDate( const QDateTime & date, const char * format ) const
{
    const time_t tdate = date.toTime_t();
    const struct tm* tms = localtime( &tdate );
    static char s[32];
    strftime(s, sizeof(s), format, tms);
    return QString::fromLocal8Bit(s);
}

#include "resUsageView.moc"
