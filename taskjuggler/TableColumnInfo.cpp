/*
 * TableColumnInfo.cpp - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#include "TableColumnInfo.h"

void 
TableColumnInfo::clearSum()
{
    delete [] sum;
    sum = new QMap<QString, double>[maxScenarios];
}

void 
TableColumnInfo::clearMemory()
{
    delete [] memory;
    memory = new QMap<QString, double>[maxScenarios];
}

void 
TableColumnInfo::addToSum(uint sc, const QString& key, double val)
{
    sum[sc][key] += val;
}

void 
TableColumnInfo::addSumToMemory(bool subtract)
{
    QMap<QString, double>::Iterator sit;

    for (uint sc = 0; sc < maxScenarios; ++sc)
        for (sit = sum[sc].begin(); sit != sum[sc].end(); ++sit)
        {
            if (subtract)
                memory[sc][sit.key()] -= *sit;
            else
                memory[sc][sit.key()] += *sit; 
        }
}

void
TableColumnInfo::negateMemory()
{
    QMap<QString, double>::Iterator mit;

    for (uint sc = 0; sc < maxScenarios; ++sc)
        for (mit = memory[sc].begin(); mit != memory[sc].end(); ++mit)
            *mit = -*mit;
}

void
TableColumnInfo::recallMemory()
{
    QMap<QString, double>::ConstIterator mit;

    for (uint sc = 0; sc < maxScenarios; ++sc)
    {
        sum[sc].clear();
        for (mit = memory[sc].begin(); mit != memory[sc].end(); ++mit)
            sum[sc][mit.key()] = *mit;
    }
}

