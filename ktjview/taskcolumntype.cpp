
#include "Task.h"
#include "taskcolumntype.h"
#include "Resource.h"

#include <qdatetime.h>
#include <kglobal.h>
#include <klocale.h>

TaskColumnType::TaskColumnType( QString n, QString d, ColumnType t )
    : m_type(t), m_name(n), m_desc(d), m_show(true)
{

}


QString TaskColumnType::toString(Task *t, int scen ) const
{
    QString ret;

    if( m_name == COMPLETE ) {
	return format( t->getComplete(scen));
    } else if( m_name == RESPONSIBLE ) {
        return format( t->getResponsible());
    } else if( m_name == COST ) {

    } else if( m_name == PRIORITY ) {
        return format( t->getPriority());
    } else if( m_name == DEPEND ) {
	return format( t->getDependsIterator());
    } else if( m_name == PRECEDES ) {
	return format( t->getPrecedesIterator());
    } else if( m_name == PREVIOUS ) {
	return format( t->getPreviousIterator());
    } else if( m_name == FOLLOWERS ) {
	return format( t->getFollowersIterator());
    } else if( m_name == DURATION ) {
	return( format( t->getDuration(scen)));
    } else if( m_name == EFFORT ) {
	return( format( t->getEffort(scen)));
    } else if( m_name == LENGTH ) {
	return( format( t->getLength(scen)));
    } else if( m_name == END ) {
	return( format( t->getEnd(scen)));
    } else if( m_name == START ) {
	return( format( t->getStart(scen)));
    } else if( m_name == START_BUFFER_END ) {
        return( format( t->getStartBufferEnd(scen)));
    } else if( m_name == END_BUFFER_START ) {
        return( format( t->getEndBufferStart(scen)));
    } else if( m_name == MIN_START ) {
        return( format( t->getMinStart(scen)));
    } else if( m_name == MIN_END ) {
        return( format( t->getMinEnd(scen)));
    } else if( m_name == MAX_START ) {
        return( format( t->getMaxStart(scen)));
    } else if( m_name == MAX_END ) {
        return( format( t->getMaxEnd(scen)));
    } else if( m_name == STATUS  ) {
        QString statString;
        switch( t->getStatus(scen) )
        {
        case NotStarted:
            statString = i18n("Not yet started");
            break;
        case InProgressLate:
            statString = i18n("In progress, but late");
            break;
        case InProgress:
            statString = i18n("In progress");
            break;
        case OnTime:
            statString = i18n("on time");
            break;
        case InProgressEarly:
            statString = i18n("early in progress");
            break;
        case Finished:
            statString = i18n("finished");
            break;
        case Undefined:
            statString = i18n("Undefined");
            break;
        default:
            statString = QString::number((int)t->getStatus(scen));
            break;
        }

        return( statString );
    } else if( m_name == STATUS_NOTE ) {
        return t->getStatusNote(scen);
    }

    return ret;
}

QString TaskColumnType::format( Resource *res ) const
{
    QString s = "";
    if( res )
        s = res->getName();
    return s;
}

QString TaskColumnType::format( TaskListIterator it ) const
{
    QString ret="";

    bool first=true;
    for( ; *it !=0; ++it )
    {
	if( ! first )
	{
	    ret += ", ";
	}
	ret += (*it)->getName();
	first = false;
    }
    return ret;
}

QString TaskColumnType::format( int val) const
{
    return QString::number( val );
}

QString TaskColumnType::format( double val) const
{
    QString ret = (KGlobal::locale())->formatNumber( val, 2 );

    if( type() == Timespan ) {
	// format to days
	ret = ret +" " +i18n("days");
    }
    return ret;
}

QString TaskColumnType::format( time_t t ) const
{
    QDateTime dt;
    dt.setTime_t(t);
    QString ret;
    ret = (KGlobal::locale())->formatDate(dt.date());
    ret += " ";
    ret += (KGlobal::locale())->formatTime( dt.time());
    return ret;
}
