/*
 * HTMLPrimitives.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */
#ifndef _HTMLPrimitives_h_
#define _HTMLPrimitives_h_

#include <qstring.h>

/**
 * @short Utility functions to generate HTML files.
 * @author Chris Schlaeger <cs@kde.org>
 */
class HTMLPrimitives
{
public:
    HTMLPrimitives() { }
    ~HTMLPrimitives() { }
    QString htmlFilter(const QString& s) const;
} ;

#endif

