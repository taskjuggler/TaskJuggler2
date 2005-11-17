/*
 * HTMLResourceReportElement.h - ResourceJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _HTMLResourceReportElement_h_
#define _HTMLResourceReportElement_h_

#include "HTMLReportElement.h"

class HTMLResourceReportElement : public HTMLReportElement
{
public:
    HTMLResourceReportElement(Report* r, const QString& df, int dl);
    ~HTMLResourceReportElement();

    bool generate();
private:
    HTMLResourceReportElement() { }
} ;

#endif

