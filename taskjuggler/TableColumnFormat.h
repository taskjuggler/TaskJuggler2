/*
 * TableColumnFormat.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _TableColumnFormat_h_
#define _TableColumnFormat_h_

#include <qstring.h>

#include "RealFormat.h"

class ReportElement;
class TableCellInfo;

typedef void (ReportElement::*GenCellPtr) (TableCellInfo*);

/**
 * @short Stores the format information of a table column.
 * @author Chris Schlaeger <cs@suse.de>
 */
class TableColumnFormat
{
public:
    enum HorizAlign { center = 0, left, right };

    TableColumnFormat(const QString& i, ReportElement* e, const QString& t);
    ~TableColumnFormat() { }

    const QString& getTitle() const { return title; }
    HorizAlign getHAlign() const { return hAlign; }
    int getFontFactor() const { return fontFactor; }
    bool getNoWrap() const { return noWrap; }
    RealFormat getRealFormat() const { return realFormat; }

    GenCellPtr genHeadLine1;
    GenCellPtr genHeadLine2;

    GenCellPtr genTaskLine1;
    GenCellPtr genTaskLine2;
    GenCellPtr genResourceLine1;
    GenCellPtr genResourceLine2;
    GenCellPtr genAccountLine1;
    GenCellPtr genAccountLine2;

    GenCellPtr genSummaryLine1;
    GenCellPtr genSummaryLine2;

    HorizAlign hAlign;
    int fontFactor;
    bool noWrap;
    RealFormat realFormat;

    const QString& getId() const { return id; }

protected:
    TableColumnFormat() { }

    QString id;
    ReportElement* el;
    QString title;
} ;

#endif

