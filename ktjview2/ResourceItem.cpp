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
