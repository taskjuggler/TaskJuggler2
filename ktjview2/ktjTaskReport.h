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

#ifndef _KTJTASKREPORT_H_
#define _KTJTASKREPORT_H_

#include "ktjReport.h"
#include "ktjReportView.h"

// TJ includes
#include "Task.h"
#include "Resource.h"

/**
 * @short Task Coverage report
 *
 * This report show tasks and their allocated resources in a hierarchical view
 * together with load
 * @author Lukas Tinkl <lukas.tinkl@suse.cz>
 */
class KtjTaskReport: public KtjReport
{
public:
    /**
     * Constructor
     * @param proj The project to work on
     * @param view View (KListView based) where to display data
     */
    KtjTaskReport( Project * proj, KtjReportView * view );
    virtual void generate();

protected:
    virtual int setupColumns();

private:
    /**
     * Generate primary row with tasks
     * @param parent view where to attach this item
     * @param task Task to work on
     * @param columns number of columns
     */
    void generatePrimaryRow( KtjReportView * parent, Task * task, int columns );

    /**
     * Generate primary row with tasks
     * @param parent parent item where to attach this item
     * @param task Task to work on
     * @param columns number of columns
     */
    void generateSecondaryRow( KListViewItem * parent, Task * task, Resource * res, int columns );
};

#endif
