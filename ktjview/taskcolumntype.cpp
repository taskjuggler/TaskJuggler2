
#include "Task.h"
#include "taskcolumntype.h"

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
    } else if( m_name == COST ) {
	
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
    } else if( m_name == END ) {
	return( format( t->getEnd(scen)));
    } else if( m_name == START ) {
	return( format( t->getStart(scen)));
    }
    
    return ret;
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
