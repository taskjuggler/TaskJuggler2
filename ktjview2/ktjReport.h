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

#ifndef _KTJREPORT_H_
#define _KTJREPORT_H_

#include "ktjReportView.h"

#include "Project.h"
#include "Interval.h"

#include <qdatetime.h>

/**
 * @short Base class for reports
 *
 * This class serves as a base for all report related classes
 * @author Lukas Tinkl <lukas.tinkl@suse.cz>
 */
class KtjReport
{
public:
    /**
     * Time scale for the horizontal line
     */
    enum Scale { SC_DAY = 0, SC_WEEK, SC_MONTH, SC_QUARTER };

    /**
     * Specify which information to draw in a cell
     */
    enum DisplayData
    {
        /// resource load / total load (where total == load + free)
        DIS_LOAD_AND_TOTAL = 0,
        /// resource load
        DIS_LOAD,
        /// available resource workload
        DIS_FREE
    };

    /**
     * Constructor
     * @param proj The project to work on
     * @param view View (KListView based) where to display data
     */
    KtjReport( Project * proj, KtjReportView * view );

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

    /**
     * @return the type of data to display
     * @see DisplayData
     */
    DisplayData displayData() const;

    /**
     * Set the type of data to display
     * @see DisplayData
     */
    void setDisplayData( int data );

    /**
     * Generate the report
     */
    virtual void generate() = 0;

protected:
    /**
     * Setup column headers, return their count
     */
    virtual int setupColumns() = 0;

    /**
     * @return the Interval which corresponds to column @p col according to the current Scale
     */
    Interval intervalForCol( int col ) const;

    /**
     * @return formatted @p date according to @p format and the current scale
     * (uses strftime)
     */
    QString formatDate( time_t date, QString format ) const;

    /// our project
    Project * m_proj;
    /// our view
    KtjReportView * m_view;
    /// current time scale
    Scale m_scale;
    /// display params
    DisplayData m_display;
    /// project start, project end
    QDateTime m_start, m_end;
};

#endif
