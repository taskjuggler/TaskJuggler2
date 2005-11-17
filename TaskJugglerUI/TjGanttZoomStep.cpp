/*
 * The TaskJuggler Project Management Software
 *
 * Copyright (c) 2001, 2002, 2003, 2004, 2005 by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id: taskjuggler.cpp 1085 2005-06-23 20:34:54Z cs $
 */

#include "TjGanttZoomStep.h"

#include <assert.h>

#include <qfontmetrics.h>

#include "Utility.h"

int
TjGanttZoomStep::getPixelsPerYear() const
{
    switch (stepUnit2)
    {
        case hour:
            return stepSize * 24 * 365;
        case day:
            return stepSize * 365;
        case week:
            return stepSize * 52;
        case month:
            return stepSize * 12;
        case quarter:
            return stepSize * 4;
        case year:
            return stepSize;
        default:
            assert(0);
    }

    return 0;
}
int
TjGanttZoomStep::calcStepSize(int ppyHint)
{
    QFontMetrics fm(font);
    int margin = (int) (0.2 * fm.height());
    int widthPattern1 = fm.width(pattern1) + 2 * margin + 1;
    int widthPattern2 = fm.width(pattern2) + 2 * margin + 1;

    // Now pick the wider one.
    int minWidth = widthPattern1 / ratio > widthPattern2 ?
        widthPattern1 / ratio : widthPattern2;

    int scaleFactor;
    switch (stepUnit2)
    {
        case hour:
            scaleFactor = 24 * 365;
            break;
        case day:
            scaleFactor = 365;
            break;
        case week:
            scaleFactor = 52;
            break;
        case month:
            scaleFactor = 12;
            break;
        case quarter:
            scaleFactor = 4;
            break;
        case year:
            scaleFactor = 1;
            break;
        default:
            assert(0);
    }
    stepSize = ppyHint / scaleFactor > minWidth ?
        ppyHint / scaleFactor : minWidth;

    return stepSize * scaleFactor;
}

time_t
TjGanttZoomStep::intervalStart(bool firstRow, time_t t)
{
    switch (firstRow ? stepUnit1 : stepUnit2)
    {
        case hour:
            return midnight(t);
        case day:
            return midnight(t);
        case week:
            return beginOfWeek(t, weekStartsMonday);
        case month:
            return beginOfMonth(t);
        case quarter:
            return beginOfQuarter(t);
        case year:
            return beginOfYear(t);
        default:
            assert(0);
    }

    return 0;
}

time_t
TjGanttZoomStep::nextIntervalStart(bool firstRow, time_t t)
{
    switch (firstRow ? stepUnit1 : stepUnit2)
    {
        case hour:
            return hoursLater(1, t);
        case day:
            return sameTimeNextDay(t);
        case week:
            return sameTimeNextWeek(t);
        case month:
            return sameTimeNextMonth(t);
        case quarter:
            return sameTimeNextQuarter(t);
        case year:
            return sameTimeNextYear(t);
        default:
            assert(0);
    }

    return 0;
}

