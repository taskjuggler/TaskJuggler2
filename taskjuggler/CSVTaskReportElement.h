/*
 * CSVTaskReportElement.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _CSVTaskReportElement_h_
#define _CSVTaskReportElement_h_

#include "CSVReportElement.h"

class CSVTaskReportElement : public CSVReportElement
{
public:
    CSVTaskReportElement(Report* r, const QString& df, int dl);
    ~CSVTaskReportElement();

    bool generate();
private:
    CSVTaskReportElement() { }
} ;

#endif

