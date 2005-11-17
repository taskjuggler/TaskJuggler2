/*
 * TableColumn.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _TableColumn_h_
#define _TableColumn_h_

#include <qstring.h>


/**
 * @short A column of a report.
 * @author Chris Schlaeger <cs@kde.org>
 */
class TableColumn
{
public:
    TableColumn(const QString& n) : name(n) { }
    ~TableColumn() { }

    const QString& getName() const { return name; }

protected:
    QString name;   
} ;

#endif

