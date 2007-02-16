/*
 * Utility.cpp - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <qdict.h>

#include "TjMessageHandler.h"
#include "tjlib-internal.h"
#include "Utility.h"

static QDict<const char> TZDict;
static bool TZDictReady = FALSE;

static QString UtilityError;

/* localtime() calls are fairly expensive, so we implement a hashtable based
 * cache to avoid redundant calls for the same value. Changing the timezone
 * invalidates the cache though. */
struct LtHashTabEntry
{
    time_t t;
    struct tm tms;
    LtHashTabEntry* next;
} ;

static long LTHASHTABSIZE;
static LtHashTabEntry** LtHashTab = 0;

bool
isRichText(const QString& str)
{
    /* This function tries to guess whether a string is a rich-text string or
     * not. It looks for xml tags marks and does a simple validation on them.
     */

    bool hasTags = FALSE;
    bool inTag = FALSE;
    for (uint i = 0; i < str.length(); ++i)
    {
        if (str[i] == '<')
        {
            if (inTag)
                return FALSE;
            inTag = hasTags = TRUE;
        }
        else if (str[i] == '>')
        {
            if (!inTag)
                return FALSE;
            inTag = FALSE;
        }
    }

    return hasTags && !inTag;
}

const char*
timezone2tz(const char* tzone)
{
    if (!TZDictReady)
    {
        TZDict.setAutoDelete(FALSE);

        // Let's start with generic timezones
        TZDict.insert("+1300", "GMT-13:00");
        TZDict.insert("+1200", "GMT-12:00");
        TZDict.insert("+1100", "GMT-11:00");
        TZDict.insert("+1000", "GMT-10:00");
        TZDict.insert("+0900", "GMT-9:00");
        TZDict.insert("+0800", "GMT-8:00");
        TZDict.insert("+0700", "GMT-7:00");
        TZDict.insert("+0600", "GMT-6:00");
        TZDict.insert("+0500", "GMT-5:00");
        TZDict.insert("+0400", "GMT-4:00");
        TZDict.insert("+0300", "GMT-3:00");
        TZDict.insert("+0200", "GMT-2:00");
        TZDict.insert("+0100", "GMT-1:00");
        TZDict.insert("+0000", "GMT-0:00");
        TZDict.insert("-0100", "GMT+1:00");
        TZDict.insert("-0200", "GMT+2:00");
        TZDict.insert("-0300", "GMT+3:00");
        TZDict.insert("-0400", "GMT+4:00");
        TZDict.insert("-0500", "GMT+5:00");
        TZDict.insert("-0600", "GMT+6:00");
        TZDict.insert("-0700", "GMT+7:00");
        TZDict.insert("-0800", "GMT+8:00");
        TZDict.insert("-0900", "GMT+9:00");
        TZDict.insert("-1000", "GMT+10:00");
        TZDict.insert("-1100", "GMT+11:00");
        TZDict.insert("-1200", "GMT+12:00");
        // Now some conveniance timezones. There will be more in the future.
        TZDict.insert("PST", "GMT+8:00");
        TZDict.insert("PDT", "GMT+7:00");
        TZDict.insert("MST", "GMT+7:00");
        TZDict.insert("MDT", "GMT+6:00");
        TZDict.insert("CST", "GMT+6:00");
        TZDict.insert("CDT", "GMT+5:00");
        TZDict.insert("EST", "GMT+5:00");
        TZDict.insert("EDT", "GMT+4:00");
        TZDict.insert("GMT", "GMT");
        TZDict.insert("UTC", "GMT");
        TZDict.insert("CET", "GMT-1:00");
        TZDict.insert("CEDT", "GMT-2:00");

        TZDictReady = TRUE;
    }

    return TZDict[tzone];
}

void initUtility(long dictSize)
{
    if (LtHashTab)
        exitUtility();

    /* Find a prime number that is equal or bigger than dictSize. */
    for (long i = 2; i < (dictSize / 2); i++)
        if (dictSize % i == 0)
        {
            dictSize++;
            i = 1;
        }

    LtHashTab = new LtHashTabEntry*[LTHASHTABSIZE = dictSize];
    for (long i = 0; i < LTHASHTABSIZE; ++i)
        LtHashTab[i] = 0;
}

void exitUtility()
{
    if (!LtHashTab)
        return;

    for (long i = 0; i < LTHASHTABSIZE; ++i)
        for (LtHashTabEntry* htep = LtHashTab[i]; htep; )
        {
            LtHashTabEntry* tmp = htep->next;
            delete htep;
            htep = tmp;
        }

    delete [] LtHashTab;
    LtHashTab = 0;
}

bool
setTimezone(const char* tZone)
{
    UtilityError = "";

    if (setenv("TZ", tZone, 1) < 0)
        qFatal("Ran out of space in environment section while "
               "setting timezone.");

    /* To valide the tZone value we call tzset(). It will convert the zone
     * into a three-letter acronym in case the tZone value is good. If not, it
     * will just copy the wrong value to tzname[0]. So, we need to validate
     * tZone names that are already 3 letter acronyms first. timezone2tz can
     * do this for us. */
    tzset();
    if (timezone2tz(tZone) == 0 && strcmp(tZone, tzname[0]) == 0)
    {
        UtilityError = QString(i18n("Illegal timezone '%1'")).arg(tZone);
        return false;
    }

    if (!LtHashTab)
        return true;
    for (long i = 0; i < LTHASHTABSIZE; ++i)
    {
        for (LtHashTabEntry* htep = LtHashTab[i]; htep; )
        {
            LtHashTabEntry* tmp = htep->next;
            delete htep;
            htep = tmp;
        }
        if (LtHashTab[i])
            LtHashTab[i] = 0;
    }
    return true;
}

static const struct tm * const
clocaltime(const time_t* t)
{
    static struct tm tm;

    /* In some cases we haven't initialized the module yet. So we do not use
     * the cache. */
    if (!LtHashTab)
        return localtime_r(t, &tm);

    long index = *t % LTHASHTABSIZE;
    if (LtHashTab[index])
        for (LtHashTabEntry* htep = LtHashTab[index]; htep;
             htep = htep->next)
            if (htep->t == *t)
                return &htep->tms;

    LtHashTabEntry* htep = new LtHashTabEntry;
    htep->next = LtHashTab[index];
    htep->t = *t;
    localtime_r(t, &htep->tms);
    LtHashTab[index] = htep;
    return &htep->tms;
}

const QString&
getUtilityError()
{
    return UtilityError;
}

QString
monthAndYear(time_t t)
{
    const struct tm* tms = clocaltime(&t);
    static char s[32];
    strftime(s, sizeof(s), "%b %Y", tms);
    return QString::fromLocal8Bit(s);
}

QString
shortMonthName(int mon)
{
    struct tm tms;
    tms.tm_mday = 1;
    tms.tm_mon = mon;
    tms.tm_year = 2000;
    static char s[32];
    strftime(s, sizeof(s), "%b", &tms);
    return QString::fromLocal8Bit(s);
}

bool
isWeekend(time_t t)
{
    const struct tm* tms = clocaltime(&t);
    return (tms->tm_wday < 1 || tms->tm_wday > 5);
}

int
daysLeftInMonth(time_t t)
{
    int left = 0;
    const struct tm* tms = clocaltime(&t);
    for (int m = tms->tm_mon; tms->tm_mon == m; )
    {
        left++;
        t = sameTimeNextDay(t);
        tms = clocaltime(&t);
    }
    return left;
}

int
weeksLeftInMonth(time_t t)
{
    int left = 0;
    const struct tm* tms = clocaltime(&t);
    for (int m = tms->tm_mon; tms->tm_mon == m; )
    {
        left++;
        t = sameTimeNextWeek(t);
        tms = clocaltime(&t);
    }
    return left;
}

int
monthLeftInYear(time_t t)
{
    int left = 0;
    const struct tm* tms = clocaltime(&t);
    for (int m = tms->tm_year; tms->tm_year == m; )
    {
        left++;
        t = sameTimeNextMonth(t);
        tms = clocaltime(&t);
    }
    return left;
}

int
quartersLeftInYear(time_t t)
{
    int left = 0;
    const struct tm* tms = clocaltime(&t);
    for (int m = tms->tm_year; tms->tm_year == m; )
    {
        left++;
        t = sameTimeNextQuarter(t);
        tms = clocaltime(&t);
    }
    return left;
}

int
daysBetween(time_t t1, time_t t2)
{
    int days = 0;
    // TODO: Very slow!!!
    for (time_t t = t1; t < t2; t = sameTimeNextDay(t))
        days++;
    return days;
}

int
weeksBetween(time_t t1, time_t t2)
{
    int days = 0;
    // TODO: Very slow!!!
    for (time_t t = t1; t < t2; t = sameTimeNextWeek(t))
        days++;
    return days;
}

int
monthsBetween(time_t t1, time_t t2)
{
    int months = 0;
    // TODO: Very slow!!!
    for (time_t t = t1; t < t2; t = sameTimeNextMonth(t))
        months++;
    return months;
}

int
quartersBetween(time_t t1, time_t t2)
{
    int quarters = 0;
    // TODO: Very slow!!!
    for (time_t t = t1; t < t2; t = sameTimeNextQuarter(t))
        quarters++;
    return quarters;
}

int
secondsOfDay(time_t t)
{
    const struct tm* tms = clocaltime(&t);
    return tms->tm_sec + tms->tm_min * 60 + tms->tm_hour * 3600;
}

int
hourOfDay(time_t t)
{
    const struct tm* tms = clocaltime(&t);
    return tms->tm_hour;
}

int
dayOfMonth(time_t t)
{
    const struct tm* tms = clocaltime(&t);
    return tms->tm_mday;
}

int
weekOfYear(time_t t, bool beginOnMonday)
{
    /* The  ISO 8601:1988 week number of the current year as a decimal
     * number, range 1 to 53, where  week  1 is  the first week that has at
     * least 4 days in the current year, and with Monday as the first day
     * of the week. This is also compliant with DIN 1355. */
    uint week = 0;
    uint weekday1Jan = dayOfWeek(beginOfYear(t), beginOnMonday);
    const struct tm* tms = clocaltime(&t);
    int days = tms->tm_yday;

    if (weekday1Jan > 3)
        days = days - (7 - weekday1Jan);
    else
        days = days + weekday1Jan;

    if (days < 0)
        if ((weekday1Jan == 4) ||
            (dayOfWeek(beginOfYear(beginOfYear(t) - 1), beginOnMonday) == 3))
            week = 53;
        else
            week = 52;
    else
        week = days / 7 + 1;

    if ((days > 360) && (week > 52))
    {
        if (weekday1Jan == 3)
            week = 53;
        else if (dayOfWeek(sameTimeNextYear(beginOfYear(t)),
                           beginOnMonday) == 4)
            week = 53;
        else
            week = 1;
    }

    return week;
}

int
monthOfWeek(time_t t, bool beginOnMonday)
{
    const struct tm* tms = clocaltime(&t);
    int tm_mon = tms->tm_mon;
    int tm_mday = tms->tm_mday;
    int lastDayOfMonth = dayOfMonth(beginOfMonth(sameTimeNextMonth(t)) - 1);
    if (tm_mday < 4)
    {
        if (dayOfWeek(t, beginOnMonday) - tm_mday >= 3)
            if (tm_mon == 0)
                return 12;
            else
                return tm_mon;
    }
    else if (tm_mday > lastDayOfMonth - 4)
    {
        if (tm_mday - dayOfWeek(t, beginOnMonday) > lastDayOfMonth - 4)
            if (tm_mon == 11)
                return 1;
            else
                return tm_mon + 2;
    }
    return tm_mon + 1;
}

int
monthOfYear(time_t t)
{
    const struct tm* tms = clocaltime(&t);
    return tms->tm_mon + 1;
}

int
quarterOfYear(time_t t)
{
    const struct tm* tms = clocaltime(&t);
    return tms->tm_mon / 3 + 1;
}

int
dayOfWeek(time_t t, bool beginOnMonday)
{
    const struct tm* tms = clocaltime(&t);
    if (beginOnMonday)
        return tms->tm_wday ? tms->tm_wday - 1 : 6;
    else
        return tms->tm_wday;
}

QString
dayOfWeekName(time_t t)
{
    const struct tm* tms = clocaltime(&t);
    static char buf[64];

    strftime(buf, 63, "%A", tms);
    return QString::fromLocal8Bit(buf);
}

int
dayOfYear(time_t t)
{
    const struct tm* tms = clocaltime(&t);
    return tms->tm_yday + 1;
}


int
year(time_t t)
{
    const struct tm* tms = clocaltime(&t);
    return tms->tm_year + 1900;
}

int
yearOfWeek(time_t t, bool beginOnMonday)
{
    const struct tm* tms = clocaltime(&t);
    int tm_year = tms->tm_year;

    int lastDayOfYear = dayOfYear(beginOfYear(sameTimeNextYear(t)) - 1);
    if (dayOfYear(t) < 4)
    {
        if (dayOfWeek(t, beginOnMonday) - dayOfYear(t) >= 3)
            return 1900 + tm_year - 1;
    }
    else if (dayOfYear(t) > lastDayOfYear - 4)
    {
        if (dayOfYear(t) - dayOfWeek(t, beginOnMonday) > lastDayOfYear - 4)
            return 1900 + tm_year + 1;
    }
    return 1900 + tm_year;
}

time_t
beginOfHour(time_t t)
{
    const struct tm* tms = clocaltime(&t);
    struct tm tmc;
    memcpy(&tmc, tms, sizeof(struct tm));
    tmc.tm_sec = tmc.tm_min = 0;
    tmc.tm_isdst = -1;
    return mktime(&tmc);
}

time_t
midnight(time_t t)
{
    const struct tm* tms = clocaltime(&t);
    struct tm tmc;
    memcpy(&tmc, tms, sizeof(struct tm));
    tmc.tm_sec = tmc.tm_min = tmc.tm_hour = 0;
    tmc.tm_isdst = -1;
    return mktime(&tmc);
}

time_t
beginOfWeek(time_t t, bool beginOnMonday)
{
    const struct tm* tms;
    for (tms = clocaltime(&t) ; tms->tm_wday != (beginOnMonday ? 1 : 0); )
    {
        t = sameTimeYesterday(t);
        tms = clocaltime(&t);
    }
    struct tm tmc;
    memcpy(&tmc, tms, sizeof(struct tm));
    tmc.tm_sec = tmc.tm_min = tmc.tm_hour = 0;
    tmc.tm_isdst = -1;
    return mktime(&tmc);
}

time_t
beginOfMonth(time_t t)
{
    const struct tm* tms = clocaltime(&t);
    struct tm tmc;
    memcpy(&tmc, tms, sizeof(struct tm));
    tmc.tm_mday = 1;
    tmc.tm_sec = tmc.tm_min = tmc.tm_hour = 0;
    tmc.tm_isdst = -1;
    return mktime(&tmc);
}

time_t
beginOfQuarter(time_t t)
{
    const struct tm* tms = clocaltime(&t);
    struct tm tmc;
    memcpy(&tmc, tms, sizeof(struct tm));
    tmc.tm_mon = (tmc.tm_mon / 3) * 3;
    tmc.tm_mday = 1;
    tmc.tm_sec = tmc.tm_min = tmc.tm_hour = 0;
    tmc.tm_isdst = -1;
    return mktime(&tmc);
}

time_t
beginOfYear(time_t t)
{
    const struct tm* tms = clocaltime(&t);
    struct tm tmc;
    memcpy(&tmc, tms, sizeof(struct tm));
    tmc.tm_mon = 0;
    tmc.tm_mday = 1;
    tmc.tm_sec = tmc.tm_min = tmc.tm_hour = 0;
    tmc.tm_isdst = -1;
    return mktime(&tmc);
}

time_t
hoursLater(int h, time_t t)
{
    // I hope this is correct under all circumstances.
    return t + h * ONEHOUR;
}

time_t
sameTimeNextDay(time_t t)
{
    const struct tm* tms = clocaltime(&t);
    struct tm tmc;
    memcpy(&tmc, tms, sizeof(struct tm));
    tmc.tm_mday++;
    tmc.tm_isdst = -1;
    if (mktime(&tmc) == -1)
        qFatal("Error at %s", time2ISO(t).latin1());
    return mktime(&tmc);
}

time_t
sameTimeYesterday(time_t t)
{
    const struct tm* tms = clocaltime(&t);
    struct tm tmc;
    memcpy(&tmc, tms, sizeof(struct tm));
    tmc.tm_mday--;
    tmc.tm_isdst = -1;
    return mktime(&tmc);
}

time_t
sameTimeNextWeek(time_t t)
{
    const struct tm* tms = clocaltime(&t);
    int weekday = tms->tm_wday;
    do
    {
        t = sameTimeNextDay(t);
        tms = clocaltime(&t);
    } while (tms->tm_wday != weekday);
    return t;
}

time_t
sameTimeLastWeek(time_t t)
{
    const struct tm* tms = clocaltime(&t);
    int weekday = tms->tm_wday;
    do
    {
        t = sameTimeYesterday(t);
        tms = clocaltime(&t);
    } while (tms->tm_wday != weekday);
    return t;
}

time_t
sameTimeNextMonth(time_t t)
{
    const struct tm* tms = clocaltime(&t);
    struct tm tmc;
    memcpy(&tmc, tms, sizeof(struct tm));
    tmc.tm_mon++;
    tmc.tm_isdst = -1;
    return mktime(&tmc);
}

time_t
sameTimeNextQuarter(time_t t)
{
    const struct tm* tms = clocaltime(&t);
    struct tm tmc;
    memcpy(&tmc, tms, sizeof(struct tm));
    tmc.tm_mon += 3;
    tmc.tm_isdst = -1;
    return mktime(&tmc);
}

time_t
sameTimeNextYear(time_t t)
{
    const struct tm* tms = clocaltime(&t);
    struct tm tmc;
    memcpy(&tmc, tms, sizeof(struct tm));
    tmc.tm_year++;
    tmc.tm_isdst = -1;
    return mktime(&tmc);
}

time_t
sameTimeLastYear(time_t t)
{
    const struct tm* tms = clocaltime(&t);
    struct tm tmc;
    memcpy(&tmc, tms, sizeof(struct tm));
    tmc.tm_year--;
    tmc.tm_isdst = -1;
    return mktime(&tmc);
}

QString
time2ISO(time_t t)
{
    const struct tm* tms = clocaltime(&t);
    static char buf[128];

    strftime(buf, 127, "%Y-%m-%d %H:%M:%S %Z", tms);
    return QString::fromLocal8Bit(buf);
}

QString
time2tjp(time_t t)
{
    const struct tm* tms = clocaltime(&t);
    static char buf[128];

    strftime(buf, 127, "%Y-%m-%d-%H:%M:%S-%z", tms);
    return QString::fromLocal8Bit(buf);
}

QString
time2rfc(time_t t)
{
    const struct tm* tms = clocaltime(&t);
    static char buf[128];

    strftime(buf, 127, "%Y-%m-%d-%H:%M:%S-%z", tms);
    return QString::fromLocal8Bit(buf);
}

QString
time2user(time_t t, const QString& timeFormat, bool localtime)
{
    if (t == 0)
        return i18n("undefined");

    const struct tm* tms;
    if (localtime)
        tms = clocaltime(&t);
    else
        tms = gmtime(&t);

    static char buf[128];

    strftime(buf, 127, timeFormat, tms);
    return QString::fromLocal8Bit(buf);
}

QString
time2time(time_t t)
{
    const struct tm* tms = clocaltime(&t);
    static char buf[128];

    strftime(buf, 127, "%H:%M %Z", tms);
    return QString::fromLocal8Bit(buf);
}

QString
time2date(time_t t)
{
    const struct tm* tms = clocaltime(&t);
    static char buf[128];

    strftime(buf, 127, "%Y-%m-%d", tms);
    return QString::fromLocal8Bit(buf);
}

QString
time2weekday(time_t t)
{
    const struct tm* tms = clocaltime(&t);
    static char buf[128];

    strftime(buf, 127, "%A", tms);
    return QString::fromLocal8Bit(buf);
}

time_t
addTimeToDate(time_t day, time_t hour)
{
    day = midnight(day);
    const struct tm* tms = clocaltime(&day);

    struct tm tmc;
    memcpy(&tmc, tms, sizeof(struct tm));

    tmc.tm_hour = hour / (60 * 60);
    tmc.tm_min = (hour / 60) % 60;
    tmc.tm_sec = hour % 60;

    tmc.tm_isdst = -1;
    return mktime(&tmc);
}

time_t
date2time(const QString& date)
{
    UtilityError = "";

    int y, m, d, hour, min, sec;
    char tZone[64] = "";
    char* savedTZ = 0;
    bool restoreTZ = FALSE;
    if (sscanf(date, "%d-%d-%d-%d:%d:%d-%s",
               &y, &m, &d, &hour, &min, &sec, tZone) == 7 ||
        (sec = 0) ||    // set sec to 0
        sscanf(date, "%d-%d-%d-%d:%d-%s",
               &y, &m, &d, &hour, &min, tZone) == 6)
    {
        const char* tz;
        if ((tz = getenv("TZ")) != 0)
        {
            savedTZ = new char[strlen(tz) + 1];
            strcpy(savedTZ, tz);
        }
        if ((tz = timezone2tz(tZone)) == 0)
        {
            UtilityError = QString(i18n("Illegal timezone %1")).arg(tZone);
            return 0;
        }
        else
        {
            if (setenv("TZ", tz, 1) < 0)
                qFatal("date2time: Ran out of space in environment section.");
            restoreTZ = TRUE;
        }
    }
    else if (sscanf(date, "%d-%d-%d-%d:%d:%d",
                    &y, &m, &d, &hour, &min, &sec) == 6)
        tZone[0] = '\0';
    else if (sscanf(date, "%d-%d-%d-%d:%d", &y, &m, &d, &hour, &min) == 5)
    {
        sec = 0;
        tZone[0] = '\0';
    }
    else if (sscanf(date, "%d-%d-%d", &y, &m, &d) == 3)
    {
        tZone[0] = '\0';
        hour = min = sec = 0;
    }
    else
    {
        qFatal("Illegal date: %s", date.latin1());
        return 0;
    }

    if (y < 1970)
    {
        UtilityError = i18n("Year must be larger than 1969");
        return 0;
    }
    if (m < 1 || m > 12)
    {
        UtilityError = i18n("Month must be between 1 and 12");
        return 0;
    }
    if (d < 1 || d > 31)
    {
        UtilityError = i18n("Day must be between 1 and 31");
        return 0;
    }
    if (hour < 0 || hour > 23)
    {
        UtilityError = i18n("Hour must be between 0 and 23");
        return 0;
    }
    if (min < 0 || min > 59)
    {
        UtilityError = i18n("Minutes must be between 0 and 59");
        return 0;
    }
    if (sec < 0 || sec > 59)
    {
        UtilityError = i18n("Seconds must be between 0 and 59");
        return 0;
    }

#if defined(__CYGWIN__) || (defined(__SVR4) && defined(__sun))
    struct tm t = { sec, min, hour, d, m - 1, y - 1900, 0, 0, -1 };
#else
    struct tm t = { sec, min, hour, d, m - 1, y - 1900, 0, 0, -1, 0, 0 };
#endif
    time_t localTime = mktime(&t);

    if (restoreTZ)
    {
        if (savedTZ)
        {
            if (setenv("TZ", savedTZ, 1) < 0)
                qFatal("date2time: Ran out of space in environment section.");
            delete [] savedTZ;
        }
        else
            unsetenv("TZ");
    }

    return localTime;
}

#if defined(__SVR4) && defined(__sun)
/*
 * Note: a proper implementation of a "setenv" function for Solaris
 *       would take a map where the "variable-name" is linked to a
 *       pointer. This would allow freeing the memory allocated here
 *       (a kind of garbage collection).
 */

int setenv(const char* var, const char* val, int ignore)
{
   int varLen = strlen(var);
   int valLen = strlen(val);
   char *buffer = NULL;

   if ((buffer = (char*) malloc (varLen + valLen + 2)) == NULL)
      return -1;

   sprintf (buffer, "%s=%s", var, val);

   return putenv(buffer) ? -1 : 0;
}

int unsetenv (const char *var)
{
   return 0;    /* SKIP */
}
#endif

