/*
 * SVGTimeTimeReport.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _SVGTimeTimeReport_h_
#define _SVGTimeTimeReport_h_

#include "SVGReport.h"

/**
 * @short Stores all information about an SVG time/time report.
 */
class SVGTimeTimeReport : public SVGReport
{
public:
    SVGTimeTimeReport(Project* p, const QString& f, const QString& df, int dl);

    virtual ~SVGTimeTimeReport()
    { }

    bool generate();

    virtual const char* getType() const { return "SVGTimeTimeReport"; }

    virtual void inheritValues();

private:
};

#endif
