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

class TjReportColumn
{
public:
    TjReportColumn()
    {
        leftX = width = xPage = 0;
    }
    ~TjReportColumn() { }

    void setLeftX(int x) { leftX = x; }
    int getLeftX() const { return leftX; }

    void setWidth(int w) { width = w; }
    int getWidth() const { return width; }

    void setXPage(int x) { xPage = x; }
    int getXPage() const { return xPage; }

private:
    int leftX;
    int width;
    // The horizontal page number of the page this column is on.
    int xPage;
} ;

#endif

