/*
 * HTMLAccountReportElement.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */
#ifndef _HTMLAccountReportElement_h_
#define _HTMLAccountReportElement_h_

#include "HTMLReportElement.h"

class HTMLAccountReportElement : public HTMLReportElement
{
public:
    HTMLAccountReportElement(Report* r, const QString& df, int dl);
    ~HTMLAccountReportElement();

    bool generate();

private:
    HTMLAccountReportElement() { }
} ;

#endif

