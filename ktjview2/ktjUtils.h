#ifndef _KTJUTILS_H_
#define _KTJUTILS_H_

#include <qdatetime.h>
#include <time.h>

// TJ includes
#include "Interval.h"
#include "Resource.h"

namespace KtjUtils {

    /**
     * Converts a time_t representation into QDateTime
     *
     * @param secs number of seconds since 1970-01-01, UTC
     */
    QDateTime time_t2Q( time_t secs );

    /**
     * Converts a time_t representation to human readable, locale aware string
     *
     * @param secs number of seconds since 1970-01-01, UTC
     */
    QString time_t2QS( time_t secs );

    /**
     * @return true if Resource @p res has some vacation during Interval @p ival
     */
    bool isVacationInterval( const Resource * res, const Interval & ival );
}

#endif
