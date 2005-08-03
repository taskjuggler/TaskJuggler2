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

#include "TjReportRow.h"

#include "CoreAttributes.h"

TjReportRow::TjReportRow(int columns)
{
    cells = new (TjReportCell*)[columns];

    topY = height = yPage = 0;
}

TjReportRow::~TjReportRow()
{
    delete [] cells;
}

void
TjReportRow::insertCell(TjReportCell* c, int pos)
{
    if (pos > ((int) ((sizeof(cells) / sizeof(TjReportCell*))) - 1))
        qFatal("TjReportRow::insert: pos out of range");
    cells[pos] = c;
}

void
TjReportRow::setCoreAttributes(const CoreAttributes* c)
{
    ca = c;
}

