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
#include "ResourceItem.h"

#include <kglobal.h>
#include <klocale.h>

int ResourceItem::compare( QListViewItem * i, int col, bool ascending ) const
{
    if ( col == 2 )        // rate (currency)
    {
        double a = KGlobal::locale()->readMoney( text( col ) );
        double b = KGlobal::locale()->readMoney( i->text( col ));

        if ( a < b )
            return -1;
        else if ( a > b )
            return 1;
        else
            return 0;
    }
    else if ( col == 3 || col == 4 )        // efficiency, min. effort (double)
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
    else if ( col == 5 || col == 6 || col == 7 ) // daily/weekly/monthly max (int)
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
    else                // default
    {
        return QListViewItem::compare( i, col, ascending );
    }
}
