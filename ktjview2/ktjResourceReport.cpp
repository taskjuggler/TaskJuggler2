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

#include "ktjResourceReport.h"
#include "ktjUtils.h"
#include "TaskItem.h"
#include "ResourceItem.h"

// TJ includes
#include "TaskList.h"

// Qt includes
#include <qdatetime.h>

// KDE includes
#include <kdebug.h>
#include <klocale.h>
#include <kglobal.h>
#include <kiconloader.h>

KtjResourceReport::KtjResourceReport( Project * proj, KtjReportView * view )
    : KtjReport( proj, view )
{
}

void KtjResourceReport::generate()
{
     // clear the view
    m_view->clear();

    // generate the columns
    int cols = setupColumns();

    // parse the resources
    ResourceListIterator it = m_proj->getResourceListIterator();

    Resource * res = 0;
    while ( ( res = static_cast<Resource *>( it.current() ) ) != 0 )
    {
        ++it;
        if ( res->isGroup() || !isResourceLoaded( res ) ) // skip groups and unallocated resources
            continue;
        generatePrimaryRow( m_view, res, cols );
    }
}

int KtjResourceReport::setupColumns()
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
        kdWarning() << "Invalid scale in KtjResourceReport::getColumnLabels" << endl;
        break;
    }

    time_t delta = intervalForCol( 1 ).getDuration();
    time_t tmp = m_start.toTime_t();
    time_t tmpEnd = m_end.toTime_t();

    //kdDebug() << "getColumnLabels: m_scale: " << m_scale << " , delta: " << delta <<
    //" , start: " << tmp << ", end: " << tmpEnd << endl;

    int result = 0;

    m_view->addColumn( i18n( "Name" ) );

    while ( tmp <= tmpEnd )
    {
        m_view->addColumn( formatDate( tmp, format ) );
        tmp += delta;
        result++;
    }

    return result;
}

void KtjResourceReport::generatePrimaryRow( KtjReportView * parent, Resource * res, int columns )
{
    ResourceItem * item = new ResourceItem( parent, res->getName() );
    item->setPixmap( 0, UserIcon( "resource" ) );

    double daily = m_proj->getDailyWorkingHours();

     // generate the columns
    for ( int i = 1; i <= columns; i++ )
    {
        double load = res->getLoad( 0, intervalForCol( i - 1 ) ) * daily; // FIXME scenario
        item->setText( i, KGlobal::locale()->formatNumber( load, 0 ) );
    }

    // generate the secondary rows (tasks)
    TaskListIterator it = m_proj->getTaskListIterator();

    Task * task = 0;
    while ( ( task = static_cast<Task *>( it.current() ) ) != 0 )
    {
        ++it;
        if ( isResourceLoaded( res, task ) && !task->isContainer() )
            generateSecondaryRow( item, res, task, columns );
    }
}

void KtjResourceReport::generateSecondaryRow( KListViewItem * parent, Resource * res, Task * task, int columns )
{
    TaskItem * item = new TaskItem( parent, task->getName(), // FIXME scenario
                                    KtjUtils::time_t2Q( task->getStart(0) ),
                                    KtjUtils::time_t2Q( task->getEnd(0) ) );
    item->setPixmap( 0, UserIcon( "task" ) );

    double daily = m_proj->getDailyWorkingHours();

    // generate the columns
    for ( int i = 1; i <= columns; i++ )
    {
        double load = task->getLoad( 0, intervalForCol( i - 1 ), res ) * daily; // FIXME scenario
        item->setText( i, KGlobal::locale()->formatNumber( load, 0 ) );
    }
}

bool KtjResourceReport::isResourceLoaded( Resource * res, Task * task ) const
{
    const Interval ival( startDate().toTime_t(), endDate().toTime_t() );
    return ( task->getLoad( 0, ival, res ) > 0 );
}

bool KtjResourceReport::isResourceLoaded( Resource * res ) const
{
    const Interval ival( startDate().toTime_t(), endDate().toTime_t() );
    return ( res->getLoad( 0, ival ) > 0 );
}
