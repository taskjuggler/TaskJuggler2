/*
 * RealFormat.cpp - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#include <stdlib.h>
#include <math.h>

#include "RealFormat.h"

RealFormat::RealFormat(const QString& sp, const QString& ss,
                       const QString& ts, const QString& fs, uint fd)
    : signPrefix(sp), signSuffix(ss), thousandSep(ts), fractionSep(fs),
    fracDigits(fd)
{
}

RealFormat::RealFormat(const RealFormat& r)
{
    signPrefix = r.signPrefix;
    signSuffix = r.signSuffix;
    thousandSep = r.thousandSep;
    fractionSep = r.fractionSep;
    fracDigits = r.fracDigits;
}

QString
RealFormat::format(double val, bool showZeroFract) const
{
    /* The algorithm further down does only truncation after fracDigits. So we
     * have to do real rounding first. */
    val = qRound(val * pow(10, fracDigits)) / pow(10, fracDigits);
    
    QString text;
    for (double v = fabs(val); v >= 1.0; v /= 1000)
    {
        if (!text.isEmpty())
            text = thousandSep + text;
        if (v > 1000.0)
            text = QString("%1").arg((int) v % 1000, 3) + text;
        else
            text = QString("%1").arg((int) v % 1000) + text;
    }
    if (text.isEmpty())
        text = "0";
    text.replace(QChar(' '), QChar('0'));
    if (!fractionSep.isEmpty() && fracDigits > 0)
    {
        double v = fabs(val) - abs((int) val);
        int fract = (int) (v * pow(10, fracDigits));
        text += fractionSep + QString("%1").arg(fract);
        /* If showZeroFract is false, we remove all zeros from the right end
         * of the text string. */
        if (!showZeroFract)
            while (text[text.length() - 1] == '0')
                text = text.left(text.length() - 1);
        /* If we have removed the whole fractional part, we remove the
         * fraction seperator as well. */
        if (text.right(fractionSep.length()) == fractionSep)
            text = text.left(text.length() - fractionSep.length());
    }
    if (val < 0.0)
        text = signPrefix + text + signSuffix;
    return text;
}

