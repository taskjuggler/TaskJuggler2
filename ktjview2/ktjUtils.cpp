#include "ktjUtils.h"

// Qt includes
#include <qptrlist.h>

// KDE includes
#include <kglobal.h>
#include <klocale.h>

// TJ includes
#include "Utility.h"

namespace KtjUtils
{
    QDateTime time_t2Q( time_t secs )
    {
        QDateTime result;
        result.setTime_t( secs );
        return result;
    }

    QString time_t2QS( time_t secs )
    {
        return KGlobal::locale()->formatDateTime( time_t2Q( secs ) );
    }

    bool isVacationInterval( const Resource * res, const Interval & ival )
    {
        QPtrListIterator<Interval> vacIt = res->getVacationListIterator();

        //kdDebug() << "Checking vacations of resource: " << res->getName() << endl;

        Interval * vacation;
        while ( ( vacation = vacIt.current() ) != 0 )
        {
            ++vacIt;
            if ( ival.contains( *vacation ) )
            {
                //kdDebug() << "Found vacation from: " << vacation->getStart() << " to: " << vacation->getEnd() << endl;
                return true;
            }
        }

        return false;
    }
}
