/*
 * The TaskJuggler Project Management Software
 *
 * Copyright (c) 2001, 2002, 2003, 2004, 2005 by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _TjGanttZoomStep_h_
#define _TjGanttZoomStep_h_

#include <time.h>

#include <qfont.h>
#include <qstring.h>

class TjGanttZoomStep {
public:
    enum StepUnits { hour = 0, day, week, month, quarter, year };

    TjGanttZoomStep(const QString& l,
                    StepUnits su1, const QString& pat1, const QString& f1,
                    StepUnits su2, const QString& pat2, const QString& f2,
                    int r, bool wsm,
                    const QFont& f) :
        label(l), font(f), stepUnit1(su1), pattern1(pat1), format1(f1),
        stepUnit2(su2), pattern2(pat2), format2(f2),
        ratio(r), weekStartsMonday(wsm) { }
    ~TjGanttZoomStep() { }

    const QString& getLabel() const { return label; }

    int calcStepSize(int stepSizeHint);
    int getStepSize() const { return stepSize; }
    int getPixelsPerYear() const;
    bool getWeekStartsMonday() const { return weekStartsMonday; }

    StepUnits getStepUnit(bool firstRow) const
    {
        return firstRow ? stepUnit1 : stepUnit2;
    }

    const char* getFormat(bool firstRow) const
    {
        return firstRow ? format1 : format2;
    }

    time_t intervalStart(bool firstRow, time_t t);
    time_t nextIntervalStart(bool firstRow, time_t t);

private:
    TjGanttZoomStep() : font(QFont()) { }

    QString label;

    const QFont& font;

    StepUnits stepUnit1;
    QString pattern1;
    QString format1;

    StepUnits stepUnit2;
    QString pattern2;
    QString format2;

    int ratio;

    int stepSize;

    bool weekStartsMonday;
} ;

#endif

