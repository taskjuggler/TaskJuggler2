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
#include "TaskItem.h"

#include <kglobal.h>
#include <klocale.h>

int TaskItem::compare( QListViewItem * i, int col, bool ascending ) const
{
    if ( col == 2 )     // start date (QDateTime)
    {
        return static_cast<TaskItem *>( i )->startDate().secsTo( m_start );
    }
    else if ( col == 3 ) // end date (QDateTime)
    {
        return static_cast<TaskItem *>( i )->endDate().secsTo( m_end );
    }
    else if ( col == 4 || col == 5 ) // duration, priority (int)
    {
        int a = text( col ).toInt();
        int b = i->text( col ).toInt();
        if ( a < b )
            return -1;
        else if ( a > b )
            return 1;
        else
            return 0;
    }
    else if ( col == 6 )        // completion (double%)
    {
        QString x = text( col );
        x.truncate( x.length() - 1 );
        QString y = i->text( col );
        y.truncate( y.length() - 1 );
        double a = KGlobal::locale()->readNumber( x );
        double b = KGlobal::locale()->readNumber( y );

        if ( a < b )
            return -1;
        else if ( a > b )
            return 1;
        else
            return 0;
    }
    else if ( col == 9 )        // effort (double)
    {
        double a = KGlobal::locale()->readNumber( text( col ) );
        double b = KGlobal::locale()->readNumber( i->text( col ) );

        if ( a < b )
            return -1;
        else if ( a > b )
            return 1;
        else
            return 0;
    }
    else                // default
    {
        return QListViewItem::compare( i, col, ascending );
    }
}
