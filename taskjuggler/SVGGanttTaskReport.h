/*
 * SVGGanttTaskReport.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#ifndef _SVGGanttTaskReport_h_
#define _SVGGanttTaskReport_h_

#include "SVGReport.h"

/**
 * @short Stores all information about an SVG task report.
 */
class SVGGanttTaskReport : public SVGReport
{
public:
    SVGGanttTaskReport(Project* p, const QString& f, const QString& df, int dl);

    virtual ~SVGGanttTaskReport()
    { }

    bool generate();

    virtual const char* getType() const { return "SVGGanttTaskReport"; }

    void setHideLinks(int i) { hideLinks = i; }
    int getHideLinks() { return hideLinks; }

    void setTaskBarPrefix(const QString& t) { taskBarPrefix = t; }
    const QString getTaskBarPrefix() const { return taskBarPrefix; }

    void setTaskBarPostfix(const QString& t) { taskBarPostfix = t; }
    const QString getTaskBarPostfix() const { return taskBarPostfix; }

    virtual void inheritValues();

private:
    int hideLinks;

    QString taskBarPrefix;
    QString taskBarPostfix;
};

#endif
