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

#include "Report.h"

/**
 * @short Stores all information about an CSV task report.
 * @author Chris Schlaeger <cs@kde.org>
 */
class SVGTimeTimeReport : public Report
{
public:
    SVGTimeTimeReport(Project* p, const QString& f, const QString& df, int dl);

    virtual ~SVGTimeTimeReport()
    { }

    bool generate();

    virtual const char* getType() const { return "SVGTimeTimeReport"; }

    void setCaption(const QString& s) { caption = s; }
    const QString& getCaption() const { return caption; }

private:
    QString caption;
};

#endif
