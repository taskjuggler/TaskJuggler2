#include "ktjTaskReport.h"

// TJ includes
#include "Interval.h"
#include "Utility.h"

KTJTaskReport::KTJTaskReport( Project * proj )
    : KTJReport( proj )
{

}

QString KTJTaskReport::name() const
{
    return i18n( "Task Report" );
}

QString KTJTaskReport::description() const
{
    return i18n( "Task report combined with allocated resources." );
}

QicsDataModel KTJTaskReport::generate()
{
    clear();

    int columns = generateHeader(); // generate the column header

    TaskListIterator tasks = m_proj->getTaskListIterator();
    Task * task = 0;

    while ( ( task = static_cast<Task *>( tasks.current() ) ) != 0 )
    {
        ++it;

        if ( task->isContainer() ) // skip groups
            continue;

        generateRow( task );    // generate main rows
    }

    return m_model;
}

int KTJTaskReport::generateHeader()
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
        kdWarning() << "Invalid scale in KTJTaskReport::getColumnLabels" << endl;
        break;
    }

    time_t delta = intervalForCol( 0 ).getDuration();
    QicsDataModelRow result;
    time_t tmp = m_start.toTime_t();
    time_t tmpEnd = m_end.toTime_t();

    //kdDebug() << "generateHeader: m_scale: " << m_scale << " , delta: " << delta <<
    //" , start: " << tmp << ", end: " << tmpEnd << endl;

    while ( tmp <= tmpEnd )
    {
        result.append( formatDate( tmp, format ) );
        tmp += delta;
    }

    m_model->addRows( 1 );
    m_model->setRowItems( 0, result );

    return result.count();
}

void KTJTaskReport::generateRow( const Task * task, int columns )
{
    // generate the task row
    QicsDataModelRow taskRow( columns );

    taskRow.append( task->getName() ); // row header

    double daily = m_proj->getDailyWorkingHours();
    for ( int i = 0; i < columns; i++ ) // data in columns
    {
        Interval ival = intervalForCol( i );
        double load = task->getLoad( 0, ival ) * daily;
        taskRow.append( load );
    }

    m_model->addRows( 1 );
    m_model->setRowItems( m_model->lastRow(), taskRow );

    // generate the resource subrows
    for ( ResourceListIterator tli( task->getBookedResourcesIterator( 0 ) ); *tli != 0; ++tli )
    {
        generateRow( task, static_cast<Resource *>( tli ) );
    }
}

void KTJTaskReport::generateRow( const Task * task, const Resource * res, int columns )
{
    QicsDataModelRow resRow( columns );

    for ( int i = 0; i < columns; i++ ) // data in columns
    {
        Interval ival = intervalForCol( i );
        double load = res->getLoad( 0, ival, res ) * m_proj->getDailyWorkingHours();
        resRow.append( load );
    }

    m_model->addRows( 1 );
    m_model->setRowItems( m_model->lastRow(), resRow );
}
