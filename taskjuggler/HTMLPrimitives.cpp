/*
 * HTMLPrimitives.cpp - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

/* The following encoding table was copied from the Qt library sources since
 * this information is not available over the public API. */

#include "HTMLPrimitives.h"

#include <ctype.h>

#include <qcstring.h>
#include <qmap.h>

QString
HTMLPrimitives::htmlFilter(const QString& s) const
{
    QString out;
    bool parTags = false;
    for (uint i = 0; i < s.length(); i++)
    {
        QString repl;
        if (s[i] == '<')
        {
            /* Preserve HTML tags */
            uint j = i + 1;
            if (j < s.length() && s[j] == '/')
                j++;
            uint tagNameLen = 0;
            for ( ; j < s.length() && isalpha(s[j]); ++j)
                tagNameLen++;
            if (j < s.length() && s[j] == '/')
                j++;
            if (s[j] == '>' && tagNameLen > 0)
            {
                repl = s.mid(i, j - i + 1);
                i = j;
            }
            else
                repl = "&lt;";
        }
        else if (s[i] == '>')
            repl = "&gt;";
        else if (s[i] == '&')
            repl = "&amp;";
        else if (s[i] == '"')
            repl = "&quot;";
        else if (s.mid(i, 2) == "\n\n")
        {
            // Expand double line breaks to HTML paragraphs.
            repl = "</p><p>";
            parTags = true;
            i++;
        }
        else if(s[i].row() != 0 || s[i].cell() >= 128)
        {
            // Quote all non-ASCII characters as hex values
            repl.sprintf("&#x%02x%02x;", s[i].row(), s[i].cell());
        }

        if (repl.isEmpty())
            out += s[i];
        else
            out += repl;
    }

    return parTags ? QString("<p>") + out + "</p>" : out;
}


