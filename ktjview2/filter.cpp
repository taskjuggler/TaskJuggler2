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

#include <qstring.h>
#include <qdom.h>
#include <qdict.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qstringlist.h>

#include <klocale.h>

#include "filter.h"


Filter::Filter( const QString & name, FilterType type, FilterOp fop )
{
    m_name = name;
    m_type = type;
    m_fop = fop;
}

Filter::~Filter()
{
}

void Filter::addCondition( const QString & prop, FilterExpr expr, const QString & val )
{
    FilterCondition cond;
    cond.prop = prop;
    cond.expr = expr;
    cond.val = val;
    m_conditions.append( cond );
}

void Filter::setConditions( const QValueVector<FilterCondition> & conditions )
{
    m_conditions = conditions;
}

void Filter::clearConditions()
{
    m_conditions.clear();
}

QString Filter::exprToString( FilterExpr exp )
{
    switch ( exp )
    {
    case FEX_CONTAINS:
        return i18n( "contains" );
        break;
    case FEX_DOESNTCONTAIN:
        return i18n( "doesn't contain" );
        break;
    case FEX_EQUALS:
        return i18n( "equals" );
        break;
    case FEX_DOESNTEQUAL:
        return i18n( "doesn't equal" );
        break;
    case FEX_REGEXP:
        return i18n( "matches regexp" );
        break;
    case FEX_NOTREGEXP:
        return i18n( "doesn't match regexp" );
        break;
    case FEX_GREATER:
        return i18n( "greater than" );
        break;
    case FEX_LESS_EQUAL:
        return i18n( "less than or equal" );
        break;
    case FEX_LESS:
        return i18n( "less than" );
        break;
    case FEX_GREATER_EQUAL:
        return i18n( "greater than or equal" );
        break;
    case FEX_LAST:
        return QString::null;
        break;
    }
    return QString::null;
}

QDomElement Filter::save( QDomDocument doc )
{
    QDomElement resultElem = doc.createElement( "filter" );
    resultElem.setAttribute( "name", m_name );
    resultElem.setAttribute( "type", static_cast<int>( m_type ) );
    resultElem.setAttribute( "fop", static_cast<int>( m_fop ) );

    QDomElement condsElem = doc.createElement( "conditions" );

    QValueVector<FilterCondition>::ConstIterator it;
    for ( it = m_conditions.constBegin(); it != m_conditions.constEnd(); ++it )
    {
        FilterCondition cond = ( *it );
        QDomElement condElem = doc.createElement( "condition" );
        condElem.setAttribute( "property", cond.prop );
        condElem.setAttribute( "expression", cond.expr );
        condElem.setAttribute( "value", cond.val );
        condsElem.appendChild( condElem );
    }

    resultElem.appendChild( condsElem );

    return resultElem;
}

//------------------------------------------------------


FilterManager::FilterManager( const QString & filename )
{
    m_filename = filename;
    m_filterList.setAutoDelete( true );
}

void FilterManager::load()
{
    // load all the filters from XML
    QFile file( m_filename );
    file.open( IO_ReadOnly );

    QDomDocument doc( "filters" );
    doc.setContent( &file );
    file.close();

    // insert them into m_filterList
    m_filterList.clear();

    QDomNodeList filters = doc.elementsByTagName( "filter" );

    for ( uint i = 0; i < filters.count(); i++ )
    {
        QDomElement filterElem = filters.item( i ).toElement();
        if ( !filterElem.isNull() )
        {
            const QString name = filterElem.attribute( "name" );
            Filter * filter = new Filter( name,
                                          static_cast<FilterType>( filterElem.attribute( "type" ).toInt() ),
                                          static_cast<FilterOp>( filterElem.attribute( "fop" ).toInt() ) );

            QDomNodeList conditions = filterElem.namedItem( "conditions" ).childNodes();
            for ( uint j = 0; i < conditions.count(); j++ )
            {
                QDomElement condition = conditions.item( i ).toElement();
                if ( !condition.isNull() )
                {
                    filter->addCondition( condition.attribute( "property" ),
                                          static_cast<FilterExpr>( condition.attribute( "expression" ).toInt() ),
                                          condition.attribute( "value" ) );
                }
            }

            m_filterList.insert( name, filter );
        }
    }
}

void FilterManager::save()
{
    // save all the filters to the XML
    QDomDocument doc( "filters" );

    FilterListIterator it( m_filterList );

    for ( ; it.current(); ++it )
    {
        Filter * filter = ( *it );
        QDomElement filterElem = filter->save( doc );
        doc.appendChild( filterElem );
    }

    QFile file( m_filename );
    file.open( IO_WriteOnly );
    QTextStream stream( &file );
    doc.save( stream, 2 );
    file.close();
}

Filter * FilterManager::filter( const QString & name ) const
{
    return m_filterList.find( name );
}

void FilterManager::removeFilter( const QString & name )
{
    m_filterList.remove( name );
}

void FilterManager::addFilter( const QString & name, FilterType type,
                               FilterOp fop, const QValueVector<FilterCondition> & conditions )
{
    if ( filter( name ) != 0 )  // filter already exists
        removeFilter( name );

    Filter * newFilter = new Filter( name, type, fop ); // create new filter
    newFilter->setConditions( conditions );
    m_filterList.insert( name, newFilter );
}

QStringList FilterManager::filterStringList() const
{
    QStringList result;

    FilterListIterator it( m_filterList );
    for ( ; it.current(); ++it )
    {
        result.append( it.currentKey() );
    }

    result.sort();

    return result;
}
