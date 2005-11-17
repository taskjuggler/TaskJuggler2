/*
 * CSVPrimitives.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */
#ifndef _CSVPrimitives_h_
#define _CSVPrimitives_h_

#include <qstring.h>

/**
 * @short Utility functions to generate CSV files.
 * @author Chris Schlaeger <cs@kde.org>
 */
class CSVPrimitives
{
public:
    CSVPrimitives() { }
    ~CSVPrimitives() { }
    QString filter(const QString& s) const;
} ;

#endif

