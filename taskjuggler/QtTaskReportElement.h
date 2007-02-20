/*
 * QtTaskReportElement.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _QtTaskReportElement_h_
#define _QtTaskReportElement_h_

#include "QtReportElement.h"

class QtTaskReportElement : public QtReportElement
{
public:
    QtTaskReportElement(Report* r, const QString& df, int dl);
    ~QtTaskReportElement();

    bool generate() { return FALSE; }

private:
    QtTaskReportElement(); // leave unimplemented
} ;

#endif

