/*
 * CSVResourceReportElement.h - ResourceJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@kde.org>
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

    bool generate();
private:
    CSVResourceReportElement() { }
} ;

#endif


