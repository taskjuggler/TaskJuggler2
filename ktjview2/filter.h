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

#ifndef _FILTER_H_
#define _FILTER_H_

#include <qdict.h>
#include <qvaluevector.h>

class QString;
class QDomElement;

/**
 * A filter operation mode (AND/OR)
 */
enum FilterOp { FOP_AND = 0, FOP_OR };

/**
 * A filter expression
 */
enum FilterExpr { FEX_CONTAINS = 0, FEX_DOESNTCONTAIN, FEX_EQUALS, FEX_DOESNTEQUAL,
                  FEX_REGEXP, FEX_NOTREGEXP, FEX_GREATER, FEX_LESS_EQUAL,
                  FEX_LESS, FEX_GREATER_EQUAL };

/**
 * Container struct holding one row of filter conditions
 * @short A filter condition
 */
struct FilterCondition
{
    /// property
    QString prop;
    /// expression
    FilterExpr expr;
    /// value
    QString val;
};

/**
 * Identify the data to work on (task/resource)
 */
enum FilterType { FT_TASK = 0, FT_RESOURCE };


/**
 * Small class representing a task or resource filter based
 * on several conditions.
 *
 * @short Task/resource filter
 * @author Lukas Tinkl <lukas.tinkl@suse.cz>
 */
class Filter
{
public:
    /**
     * CTOR
     * @param name name of the filter
     */
    Filter( const QString & name, FilterType type, FilterOp fop = FOP_AND );

    /**
     * DTOR
     */
    ~Filter();

    /**
     * @return the name of the filter
     */
    QString name() const
        { return m_name; }

    /**
     * Set the @p name of the filter
     */
    void setName( const QString & name )
        { m_name = name; }

    /**
     * @return the operator mode; see FilterOp
     */
    FilterOp fop() const
        { return m_fop; }

    /**
     * Set the operator mode; see FilterOp
     */
    void setFop( FilterOp fop )
        { m_fop = fop; }

    /**
     * @return the type of the filter (task/resource)
     */
    FilterType type() const
        { return m_type; }

    /**
     * Set the @p type of the filter (task/resource)
     */
    void setType( FilterType type )
        { m_type = type; }

    /**
     * @return the filter conditions (criteria)
     */
    QValueVector<FilterCondition> conditions() const
        { return m_conditions; }

    /**
     * Add a condition
     * @param prop Property (column name)
     * @param expr Expression, see FilterExpr
     * @param val Value of the condition
     */
    void addCondition( const QString & prop, FilterExpr expr, const QString & val );

    /**
     * Directly assign the list of conditions
     */
    void setConditions( const QValueVector<FilterCondition> & conditions );

    /**
     * Clears the list of conditions
     */
    void clearConditions();

    /**
     * @return expression statement converted to a (translated)  QString
     */
    static QString exprToString( FilterExpr exp );

    /**
     * @return This filter as a DOM element
     */
    QDomElement save( QDomDocument doc );

private:
    QString m_name;
    FilterOp m_fop;
    FilterType m_type;
    QValueVector<FilterCondition> m_conditions;
};

typedef QDict<Filter> FilterList;
typedef QDictIterator<Filter> FilterListIterator;


/**
 * Container class holding individual filters.
 *
 * @short Filter container class
 * @author Lukas Tinkl <lukas.tinkl@suse.cz>
 */
class FilterManager
{
public:
    /**
     * CTOR
     * @param filename file where the filters are stored
     */
    FilterManager( const QString & filename );

    /**
     * DTOR
     */
    ~FilterManager() { };

    /**
     * @return the list of filters
     */
    FilterList filterList() const
        { return m_filterList; }

    /**
     * @return the filter by @p name, 0 if no such filter
     */
    Filter * filter( const QString & name ) const;

    /**
     * @return the filename where the filters are stored
     */
    QString filename() const
        { return m_filename; }

    /**
     * Set the @p filename where the filters are stored
     */
    void setFilename( const QString & filename )
        { m_filename = filename; }

    /**
     * Load the filter definition from an XML file
     */
    void load();

    /**
     * Save the filter definition into an XML file
     */
    void save();

    /**
     * Add a new filter (checks for an already existing filter with @p name)
     */
    void addFilter( const QString & name, FilterType type, FilterOp fop,
                    const QValueVector<FilterCondition> & conditions );

    /**
     * Remove filter with @p name
     */
    void removeFilter( const QString & name );

    /**
     * @return a string list containing all the filter names
     */
    QStringList filterStringList() const;

private:
    /// filename where the DOM is stored
    QString m_filename;
    /// QDict holding the filters
    FilterList m_filterList;
};

#endif
