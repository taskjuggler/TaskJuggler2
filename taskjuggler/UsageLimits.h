/*
 * UsageLimits.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

/**
 * This class stores usage limits of resources or task allocations. The values
 * are stored as number of scoreboard slots.
 *
 * @short The class stores usage limits
 * @see Resource
 * @see Allocation
 * @author Chris Schlaeger <cs@suse.de>
 */ 
class UsageLimits
{
    public:
        UsageLimits()
        {
            dailyMax = weeklyMax = monthlyMax = 0;
        }
        ~UsageLimits() { }

        void setDailyMax(uint m) { dailyMax = m; }
        uint getDailyMax() const { return dailyMax; }

        void setWeeklyMax(uint m) { weeklyMax = m; }
        uint getWeeklyMax() const { return weeklyMax; }

        void setMonthlyMax(uint m) { monthlyMax = m; }
        uint getMonthlyMax() const { return monthlyMax; }
        
    private:
        uint dailyMax;
        uint weeklyMax;
        uint monthlyMax;
} ;

