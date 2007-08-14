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

#include <assert.h>
#include "TjReportRow.h"

#include "CoreAttributes.h"

TjReportRow::TjReportRow(int cols, int idx) : columns(cols), index(idx)
{
    cells = new TjReportCell*[columns];
    for (int i = 0; i < columns; ++i)
        cells[i] = 0;

    topY = height = yPage = 0;
    lastOnPage = false;
    alternate = false;

    ca = subCA = 0;
}

TjReportRow::~TjReportRow()
{
    for (int i = 0; i < columns; ++i)
        delete cells[i];
    delete [] cells;
}

void
TjReportRow::insertCell(TjReportCell* c, int pos)
{
    if (pos > columns - 1)
        qFatal("TjReportRow::insert: pos (%d) out of range (%d)",
               pos, columns);
    cells[pos] = c;
}

TjReportCell*
TjReportRow::getCell(int pos) const
{
    assert(pos >= 0);
    assert(pos < columns);
    return cells[pos];
}

