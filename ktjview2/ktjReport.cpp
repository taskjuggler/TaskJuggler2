#include "ktjReport.h"

// TJ includes
#include "Utility.h"

KTJReport::KTJReport( Project * proj )
    : m_proj( proj )
{
    m_model = new QicsDataModelDefault();
    m_start = m_proj->getStartDate();
    m_end = m_proj->getEndDate();

    setScale( SC_DAY );
}

KTJReport::~KTJReport()
{
    delete m_model;
}

Scale KTJReport::scale() const
{
    return m_scale;
}

void KTJReport::setScale( int sc )
{
    m_scale = static_cast<Scale>( sc );
}

DisplayData KTJReport::displayData() const
{
    return m_display;
}

void KTJReport::setDisplayData( int data )
{
    m_display = static_cast<DisplayData>( data );
}

QDateTime KTJReport::startDate() const
{
    return m_start;
}

void KTJReport::setStartDate( const QDateTime & date )
{
    m_start = date;
}

QDateTime KTJReport::endDate() const
{
     return m_end;
}

void KTJReport::setEndDate( const QDateTime & date )
{
    m_end = date;
}

QString KTJReport::formatDate( time_t date, QString format ) const
{
    if ( m_scale == SC_QUARTER )
        format.replace( "%q", QString::number( quarterOfYear( date ) ) ); // workaround against missing %q in strftime
    const struct tm* tms = localtime( &date );
    static char s[32];
    strftime(s, sizeof(s), format.latin1(), tms);
    return QString::fromLocal8Bit(s);
}

Interval KTJReport::intervalForCol( int col ) const
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

void KTJReport::clear()
{
    m_model->clearModel();
}
