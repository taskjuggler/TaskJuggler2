/*
 * TableColorSet.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */
#ifndef _TableColorSet_h_
#define _TableColorSet_h_

#include <qcolor.h>
#include <qmap.h>

class TableColorSet
{
public:
    TableColorSet()
    {
        colors["header"] = QColor(0xa5c2ff);
        colors["default"] = QColor(0xf3ebae);
        colors["error"] = QColor(0xff0000);
        colors["today"] = QColor(0xa387ff);
        colors["vacation"] = QColor(0xfffc60);
        colors["available"] = QColor(0xa4ff8d);
        colors["booked"] = QColor(0xff5a5d);
        colors["completed"] = QColor(0x87ff75);
    }
    ~TableColorSet() { }

    void setColor(const QString& name, uint value)
    {
        colors[name] = QColor(value);
    }
    const QColor& getColor(const QString& name) const
    {
       return colors[name];
    }
    QString getColorName(const QString& name) const
    {
        return colors[name].name();
    }

private:
    QMap<QString, QColor> colors;
} ;

#endif

