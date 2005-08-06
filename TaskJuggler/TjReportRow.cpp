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

#include <assert.h>
#include "TjReportRow.h"

#include "CoreAttributes.h"

TjReportRow::TjReportRow(int cols) : columns(cols)
{
    cells = new TjReportCell*[columns];
    for (int i = 0; i < columns; ++i)
        cells[i] = 0;

    topY = height = yPage = 0;
}

TjReportRow::~TjReportRow()
{
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

void
TjReportRow::setCoreAttributes(const CoreAttributes* c)
{
    ca = c;
}

