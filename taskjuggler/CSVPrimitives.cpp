/*
 * HTMLPrimitives.cpp - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

/* The following encoding table was copied from the Qt library sources since
 * this information is not available over the public API. */

#include <qcstring.h>
#include <qmap.h>

#include "CSVPrimitives.h"

QString
CSVPrimitives::filter(const QString& s) const
{
    QString out;
    for (uint i = 0; i < s.length(); i++)
    {
        if (s[i] == '"')
            out += "\"";
        out += s[i];
    }

    return out;
}


