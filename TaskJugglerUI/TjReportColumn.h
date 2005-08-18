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

#ifndef _TjReportColumn_h_
#define _TjReportColumn_h_

#include "TjReportCell.h"

class TableColumnFormat;

class TjReportColumn
{
public:
    TjReportColumn()
    {
        leftX = width = xPage = 0;
        maxIndentLevel = 1;
        isGantt = lastOnPage = FALSE;
    }
    ~TjReportColumn() { }

    void setLeftX(int x) { leftX = x; }
    int getLeftX() const { return leftX; }

    void setWidth(int w) { width = w; }
    int getWidth() const { return width; }

    void setXPage(int x) { xPage = x; }
    int getXPage() const { return xPage; }

    void setTableColumnFormat(const TableColumnFormat* t) { tcf = t; }
    const TableColumnFormat* getTableColumnFormat() const { return tcf; }

    void setMaxIndentLevel(int level) { maxIndentLevel = level; }
    int getMaxIndentLevel() const { return maxIndentLevel; }

    void setLastOnPage(bool lop) { lastOnPage = lop; }
    bool getLastOnPage() const { return lastOnPage; }

    void setIsGantt(bool ig) { isGantt = ig; }
    bool getIsGantt() const { return isGantt; }

private:
    // The leftmost pixel of the column
    int leftX;
    // The column width in pixels
    int width;
    // The horizontal page number of the page this column is on.
    int xPage;

    int maxIndentLevel;

    const TableColumnFormat* tcf;

    // True if this column is the last column on the page.
    bool lastOnPage;

    // This flag is set if the column contains the GANTT chart.
    bool isGantt;
} ;

#endif

