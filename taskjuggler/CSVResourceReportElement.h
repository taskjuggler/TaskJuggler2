/*
 * CSVResourceReportElement.h - ResourceJuggler
 *
 * Copyright (c) 2001, 2002, 2003 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _CSVResourceReportElement_h_
#define _CSVResourceReportElement_h_

#include "CSVReportElement.h"

class CSVResourceReportElement : public CSVReportElement
{
public:
    CSVResourceReportElement(Report* r, const QString& df, int dl);
    ~CSVResourceReportElement();

    void generate();
private:
    CSVResourceReportElement() { }
} ;

#endif

