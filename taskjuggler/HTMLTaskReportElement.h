/*
 * HTMLTaskReportElement.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _HTMLTaskReportElement_h_
#define _HTMLTaskReportElement_h_

#include "HTMLReportElement.h"

class HTMLTaskReportElement : public HTMLReportElement
{
public:
    HTMLTaskReportElement(Report* r, const QString& df, int dl);
    ~HTMLTaskReportElement();

    void generate();
private:
    HTMLTaskReportElement() { }
} ;

#endif
