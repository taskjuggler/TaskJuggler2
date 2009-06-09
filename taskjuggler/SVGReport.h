/*
 * SVGReport.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _SVGReport_h_
#define _SVGReport_h_

#include "Report.h"

/**
 * @short Stores all information about an SVG report.
 */
class SVGReport : public Report
{
public:
    SVGReport(Project* p, const QString& f, const QString& df, int dl);

    virtual ~SVGReport()
    { }

    void setCaption(const QString& s) { caption = s; }
    const QString& getCaption() const { return caption; }

protected:


private:
    QString caption;
};

#endif
