/*
 * TableColumnFormat.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003 by Chris Schlaeger <cs@suse.de>
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
    TableColumnFormat(ReportElement* e, const QString& t);
    ~TableColumnFormat() { }

    const QString& getTitle() const { return title; }

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

    QString hAlign;
    int fontFactor;

protected:
    TableColumnFormat() { }
    
    ReportElement* el;
    QString title;
} ;

#endif

