/*
 * QtResourceReportElement.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _QtResourceReportElement_h_
#define _QtResourceReportElement_h_

#include "QtReportElement.h"

class QtResourceReportElement : public QtReportElement
{
public:
    QtResourceReportElement(Report* r, const QString& df, int dl);
    ~QtResourceReportElement();

    bool generate() { return FALSE; }

private:
    QtResourceReportElement() { }
} ;

#endif

