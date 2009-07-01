/*
 * UsageLimits.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004, 2005 by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _UsageLimits_h_
#define _UsageLimits_h_

/**
 * This class stores usage limits of resources or task allocations. The values
 * are stored as number of scoreboard slots.
 *
 * @short The class stores usage limits
 * @see Resource
 * @see Allocation
 * @author Chris Schlaeger <cs@kde.org>
 */
class UsageLimits
{
    public:
        UsageLimits() :
            dailyMax(0),
            weeklyMax(0),
            monthlyMax(0),
            yearlyMax(0),
            projectMax(0)
        { }

        ~UsageLimits() { }

        void setDailyMax(uint m) { dailyMax = m; }
        uint getDailyMax() const { return dailyMax; }

        void setWeeklyMax(uint m) { weeklyMax = m; }
        uint getWeeklyMax() const { return weeklyMax; }

        void setMonthlyMax(uint m) { monthlyMax = m; }
        uint getMonthlyMax() const { return monthlyMax; }

        void setYearlyMax(uint m) { yearlyMax = m; }
        uint getYearlyMax() const { return yearlyMax; }

        void setProjectMax(uint m) { projectMax = m; }
        uint getProjectMax() const { return projectMax; }

    private:
        uint dailyMax;
        uint weeklyMax;
        uint monthlyMax;
        uint yearlyMax;
        uint projectMax;
} ;

#endif

