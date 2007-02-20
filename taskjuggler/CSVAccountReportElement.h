/*
 * CSVAccountReportElement.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */
#ifndef _CSVAccountReportElement_h_
#define _CSVAccountReportElement_h_

#include "CSVReportElement.h"

class CSVAccountReportElement : public CSVReportElement
{
public:
    CSVAccountReportElement(Report* r, const QString& df, int dl);
    ~CSVAccountReportElement() { };

    bool generate();

private:
    CSVAccountReportElement(); // leave unimplemented
} ;

#endif

