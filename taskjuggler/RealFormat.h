/*
 * RealFormat.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003 by Chris Schlaeger <cs@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _RealFormat_h_
#define _RealFormat_h_

#include <qstring.h>

class RealFormat
{
public:
    RealFormat(const QString& sp, const QString& ss, const QString& ts,
               const QString& fs, uint fd);
    RealFormat(const RealFormat& r);
    RealFormat()
    {
        signPrefix = "-";
        fractionSep = ",";
        fracDigits = 2;
    }
    ~RealFormat() { }

    void setSignPrefix(const QString& sp) { signPrefix = sp; }
    void setSignSuffix(const QString& ss) { signSuffix = ss; }
    void setThousandSep(const QString& ts) { thousandSep = ts; }
    void setFractionSep(const QString& fs) { fractionSep = fs; }
    void setFracDigits(uint fd) { fracDigits = fd; }
    
    QString format(double val, bool showZeroFract = TRUE) const;

private:
    QString signPrefix;
    QString signSuffix;
    QString thousandSep;
    QString fractionSep;
    uint fracDigits;
} ;

#endif

