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

#ifndef _RESUSAGE_VIEW_H_
#define _RESUSAGE_VIEW_H_

#include <qtable.h>
#include <qdatetime.h>
#include <qstringlist.h>

#include "Resource.h"
#include "Project.h"
#include "ResourceList.h"
#include "Interval.h"

/**
 * Time scale for the horizontal line
 */
enum Scale { SC_DAY = 0, SC_WEEK, SC_MONTH, SC_QUARTER };

/**
 * This widget based on QTable displays resource loads in time.
 * Rows represent resources, columns time.
 *
 * @short Resource Usage view
 * @author Lukas Tinkl <lukas.tinkl@suse.cz>
 */
class ResUsageView: public QTable
{
    Q_OBJECT
public:
    ResUsageView( QWidget * parent = 0, const char * name = 0 );
    virtual ~ResUsageView();

    /**
     * Set the list (@p reslist) of resources to work on
     */
    void assignResources( ResourceList reslist );

    /**
     * Set the project @p proj to work on
     */
    void setProject( Project * proj );

    /**
     * Clear the view, unset the resources and project
     */
    void clear();

    /**
     * @return the current scale
     */
    Scale scale() const;

    /**
     * Set the scale
     */
    void setScale( int sc );

    /**
     * @return the start date of the view
     */
    QDateTime startDate() const;

    /**
     * Set the start date of the view
     */
    void setStartDate( const QDateTime & date );

    /**
     * @return the end date of the view
     */
    QDateTime endDate() const;

    /**
     * Set the end date of the view
     */
    void setEndDate( const QDateTime & date );

protected:
    /**
     * @override
     */
    virtual void paintCell( QPainter * p, int row, int col, const QRect & cr, bool selected, const QColorGroup & cg );

    /**
     * @override noop
     */
    virtual void resizeData( int len );

    /**
     * @override
     */
    virtual QString text ( int row, int col ) const;

signals:
    /**
     * Emitted when the horizontal (time) scale changes
     * @param sc the new scale
     */
    void scaleChanged( Scale sc );

private slots:
    /**
     * Update the column headers and labels with respect to the current scale
     */
    void updateColumns();

    /**
     * Popup up a context menu over cell at (@p row, @p col), at position @p pos
     */
    void slotPopupMenu( int row, int col, const QPoint & pos );

    /**
     * Copy the current's cell text to the clipboard
     */
    void slotCopy();

private:
    /**
     * @return the Interval which corresponds to column @p col according to the current Scale
     */
    Interval intervalForCol( int col ) const;

    /**
     * @return Resource which corresponds to @p row
     */
    Resource * resourceForRow( int row ) const;

    /**
     * @return list of column labels, depending on the current scale
     */
    QStringList getColumnLabels() const;

    /**
     * @return formatted @p date according to @p format and the current scale
     * (uses strftime)
     */
    QString formatDate( time_t date, QString format ) const;

    /**
     * @return true if Resource @p res has some vacation during Interval @p ival
     */
    bool isVacationInterval( const Resource * res, const Interval & ival ) const;

    /**
     * @return the number of working days falling within the interval @p ival
     */
    int getWorkingDays( const Interval & ival ) const;

    ResourceList resList() const
        { return m_resList; }

    /// our project
    Project * m_proj;
    /// list of resources
    ResourceList m_resList;
    /// list of row labels <res ID, res name>
    QStringList m_rowLabels;
    /// current time scale
    Scale m_scale;
    /// project start, project end
    QDateTime m_start, m_end;
};

#endif
