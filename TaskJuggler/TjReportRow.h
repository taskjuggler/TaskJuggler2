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

#ifndef _TjReportRow_h_
#define _TjReportRow_h_

#include "TjReportCell.h"

class CoreAttributes;

class TjReportRow
{
public:
    TjReportRow(int cols);
    ~TjReportRow();

    void setTopY(int y) { topY = y; }
    int getTopY() const { return topY; }

    void setHeight(int h) { height = h; }
    int getHeight() const  { return height; }

    void setYPage(int y) { yPage = y; }
    int getYPage() const { return yPage; }

    void insertCell(TjReportCell* c, int pos);
    TjReportCell* getCell(int pos) { return cells[pos]; }

    void setCoreAttributes(const CoreAttributes* c);

private:
    TjReportRow() { }

    int columns;
    int topY;
    int height;
    // The vertical page number of the page this column is on.
    int yPage;

    const CoreAttributes* ca;
    bool hidden;
    TjReportCell** cells;
} ;

#endif

