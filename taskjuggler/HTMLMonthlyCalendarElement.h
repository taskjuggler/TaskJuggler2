/*
 * HTMLMonthlyCalendarElement.h - TaskJuggler
 *
 * Copyright (c) 2006 by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id: HTMLMonthlyCalendarElement.h 1313 2006-07-27 10:50:04Z cs $
 */

#ifndef _HTMLMonthlyCalendarElement_h_
#define _HTMLMonthlyCalendarElement_h_

#include <qbitarray.h>

#include "HTMLReportElement.h"

class HTMLMonthlyCalendarElement : public HTMLReportElement
{
public:
    HTMLMonthlyCalendarElement(Report* r, const QString& df, int dl);
    ~HTMLMonthlyCalendarElement();

    bool generate();

private:
    void generateTableHeader();
    void generateTaksPerMonth(TaskList& filteredTaskList);

    HTMLMonthlyCalendarElement(); // leave unimplemented
} ;

#endif

