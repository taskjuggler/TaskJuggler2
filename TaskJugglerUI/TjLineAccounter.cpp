/*
 * The TaskJuggler Project Management Software
 *
 * Copyright (c) 2001, 2002, 2003, 2004, 2005 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id: taskjuggler.cpp 1085 2005-06-23 20:34:54Z cs $
 */

#include <qstring.h>

#include "TjLineAccounter.h"

TjLineAccounter::~TjLineAccounter()
{
    for (std::list<TjLine*>::iterator it = vertLines.begin();
         it != vertLines.end(); ++it)
        delete *it;
    for (std::list<TjLine*>::iterator it = horizLines.begin();
         it != horizLines.end(); ++it)
        delete *it;
}

void
TjLineAccounter::insertLine(bool vertical, int coord, int start, int end)
{
    // Swap start and end in case they are the wrong way round.
    if (start > end)
    {
        int tmp = start;
        start = end;
        end = tmp;
    }

    std::list<TjLine*>& list = vertical ? vertLines : horizLines;

    TjLine* line = new TjLine(coord, start, end);
    std::list<TjLine*>::iterator it = list.begin();
    for ( ; it != list.end() &&
          ((*it)->coord < coord ||
           ((*it)->coord == coord && (*it)->start < start) ||
           ((*it)->coord == coord && (*it)->start == start &&
            (*it)->end < end)); ++it)
        ;
    list.insert(it, line);

    qDebug("Inserted %s Line at %d", vertical ? "vertical" : "horizontal", coord);
    for (std::list<TjLine*>::iterator it = list.begin(); it != list.end(); ++it)
        qDebug("  C: %d  S: %d  E: %d", (*it)->coord, (*it)->start, (*it)->end);
}

bool
TjLineAccounter::collision(bool vertical, int coord, int start, int end)
{
    // Swap start and end in case they are the wrong way round.
    if (start > end)
    {
        int tmp = start;
        start = end;
        end = tmp;
    }

    std::list<TjLine*>& list = vertical ? vertLines : horizLines;

    for (std::list<TjLine*>::iterator it = list.begin(); it != list.end(); ++it)
    {
        /* Do a fuzzy comparison of the coordinate to achive a minimum
         * distance between parallel lines. */
        if ((*it)->coord + minDist < coord)
            continue;
        if ((*it)->coord - minDist > coord)
            break;

        if ((start > (*it)->start && start < (*it)->end) ||
            (end > (*it)->start && end < (*it)->end) ||
            ((*it)->start > start && (*it)->start < end) ||
            ((*it)->end > start && (*it)->end < end))
        {
            return true;
        }
    }

    return false;
}

