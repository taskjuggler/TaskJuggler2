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

#include <klocale.h>

#include "filter.h"

Filter::Filter( const QString & name, FilterType type )
{
    m_name = name;
    m_fop = FOP_AND;
    m_type = type;
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

//------------------------------------------------------


FilterManager::FilterManager( const QString & filename )
{
    m_filename = filename;
    m_filterList.setAutoDelete( true );
}

void FilterManager::load()
{
    // TODO
}

void FilterManager::save()
{
    // TODO
}

Filter * FilterManager::filter( const QString & name ) const
{
    return m_filterList.find( name );
}

