/*
 * HTMLWeeklyCalendarElement.h - TaskJuggler
 *
 * Copyright (c) 2002, 2003 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _HTMLWeeklyCalendarElement_h_
#define _HTMLWeeklyCalendarElement_h_

#include "HTMLReportElement.h"

class HTMLWeeklyCalendarElement : public HTMLReportElement
{
public:
    HTMLWeeklyCalendarElement(Report* r, const QString& df, int dl);
    ~HTMLWeeklyCalendarElement();

    void generate();
    
private:
    HTMLWeeklyCalendarElement() { }
} ;

#endif

