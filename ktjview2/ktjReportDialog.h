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

#ifndef _KTJREPORTDIALOG_H_
#define _KTJREPORTDIALOG_H_

class ReportSelectionWidget;

class QDateTime;

#include "ktjReport.h"

#include <kdialogbase.h>

/**
 * @short Report options dialog
 *
 * Dialog for setting options and generating a report
 * @author Lukas Tinkl <lukas.tinkl@suse.cz>
 */
class KtjReportDialog: public KDialogBase
{
    Q_OBJECT
public:
    /**
     * Constructor
     * @param start start of the timeframe
     * @param end end of the timeframe
     */
    KtjReportDialog( const QDateTime & start, const QDateTime & end, QWidget * parent = 0, const char * name = 0 );

    /**
     * Destructor
     */
    ~KtjReportDialog();

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
     * @return the current scale
     */
    KtjReport::Scale scale() const;

private:
    /// base widget
    ReportSelectionWidget * m_base;
};

#endif
