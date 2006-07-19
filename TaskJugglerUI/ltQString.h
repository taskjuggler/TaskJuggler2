/*
 * The TaskJuggler Project Management Software
 *
 * Copyright (c) 2001, 2002, 2003, 2004, 2005 by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */
#ifndef _ltQString_h_
#define _ltQString_h_

#include <qstring.h>

struct ltQString
{
    bool operator()(const QString& s1, const QString& s2) const
    {
        return s1 < s2;
    }
} ;

#endif

