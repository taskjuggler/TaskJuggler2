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

#ifndef _TjReportCell_h_
#define _TjReportCell_h_

#include <qstring.h>
#include <qnamespace.h>

class TjReportRow;
class TjReportColumn;

class TjReportCell
{
public:
    TjReportCell(TjReportRow* r, TjReportColumn* c);

    ~TjReportCell() { }

    void setText(const QString& t) { text = t; }
    const QString& getText() const { return text; }

private:
    TjReportCell() { }

    TjReportRow* row;
    TjReportColumn* column;

    QString text;
    Qt::AlignmentFlags alignment;
    int indentLevel;
} ;

#endif

