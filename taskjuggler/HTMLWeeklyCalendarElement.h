/*
 * HTMLWeeklyCalendarElement.h - TaskJuggler
 *
 * Copyright (c) 2002, 2003, 2004 by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _HTMLWeeklyCalendarElement_h_
#define _HTMLWeeklyCalendarElement_h_

#include <qbitarray.h>

#include "HTMLReportElement.h"

class HTMLWeeklyCalendarElement : public HTMLReportElement
{
public:
    HTMLWeeklyCalendarElement(Report* r, const QString& df, int dl);
    ~HTMLWeeklyCalendarElement();

    void setDaysToShow(QBitArray& days);

    bool generate();

private:
    void generateTableHeader(bool weekStartsMonday);
    void generateWeekHeader(bool weekStartsMonday, time_t week);
    void generateTaksPerDay(time_t& week, TaskList& filteredTaskList,
                            bool weekStartsMonday);
    void generateResourcesPerDay(time_t& week,
                                 ResourceList& filteredResourceList,
                                 bool weekStartsMonday);

    bool showThisDay(int dayIndex, bool weekStartsMonday);

    HTMLWeeklyCalendarElement() { }

    QBitArray daysToShow;
    unsigned int numberOfDays;
} ;

#endif

