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

#ifndef _TjReportRow_h_
#define _TjReportRow_h_

#include "TjReportCell.h"

class CoreAttributes;

class TjReportRow
{
public:
    TjReportRow(int cols, int index);
    ~TjReportRow();

    int getIndex() const { return index; }

    void setTopY(int y) { topY = y; }
    int getTopY() const { return topY; }

    void setHeight(int h) { height = h; }
    int getHeight() const  { return height; }

    void setYPage(int y) { yPage = y; }
    int getYPage() const { return yPage; }

    void setLastOnPage(bool lop) { lastOnPage = lop; }
    bool getLastOnPage() const { return lastOnPage; }

    void setAlternate(bool alt) { alternate = alt; }
    bool isAlternate() const { return alternate; }

    void insertCell(TjReportCell* c, int pos);
    TjReportCell* getCell(int pos) const;

    void setCoreAttributes(CoreAttributes* c,
                           CoreAttributes* sc)
    {
        ca = c; subCA = sc;
    }
    CoreAttributes* getCoreAttributes() const { return ca; }
    CoreAttributes* getSubCoreAttributes() const { return subCA; }

private:
    int columns;
    int topY;
    int height;
    // The vertical page number of the page this column is on.
    int yPage;

    CoreAttributes* ca;
    CoreAttributes* subCA;
    bool hidden;
    // True if this row is the last row on the page.
    bool lastOnPage;
    bool alternate;
    TjReportCell** cells;

    int index;
} ;

#endif

