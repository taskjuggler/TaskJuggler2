/*
 * RealFormat.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@kde.org>
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

    RealFormat() :
        signPrefix("-"),
        signSuffix(),
        thousandSep(),
        fractionSep(","),
        fracDigits(2)
    { }

    ~RealFormat() { }

    void setSignPrefix(const QString& sp) { signPrefix = sp; }
    const QString& getSignPrefix() const { return signPrefix; }
    
    void setSignSuffix(const QString& ss) { signSuffix = ss; }
    const QString& getSignSuffix() const { return signSuffix; }
    
    void setThousandSep(const QString& ts) { thousandSep = ts; }
    const QString& getThousandSep() const { return thousandSep; }
    
    void setFractionSep(const QString& fs) { fractionSep = fs; }
    const QString& getFractionSep() const { return fractionSep; }
    
    void setFracDigits(uint fd) { fracDigits = fd; }
    uint getFracDigits() const { return fracDigits; }
    
    QString format(double val, bool showZeroFract = true) const;

private:
    QString signPrefix;
    QString signSuffix;
    QString thousandSep;
    QString fractionSep;
    uint fracDigits;
} ;

#endif

