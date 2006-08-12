/*
 * ICalReport.h - TaskJuggler
 *
 * Copyright (c) 2002, 2003, 2004, 2005 by Chris Schlaeger <cs@kde.org>
 * Copyright (c) 2002, 2003, 2004, 2005 by Klaas Freitag <freitag@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */
#ifndef _ICalReport_h_
#define _ICalReport_h_

#include <time.h>

#include <qstring.h>
#include <libkcal/calendarlocal.h>
#include <libkcal/todo.h>

#include <Report.h>

class Project;

class ICalReport : public Report
{
public:
    ICalReport(Project* p, const QString& f, const QString& df, int dl);
    virtual ~ICalReport() { }

    virtual const char* getType() const { return "ICalReport"; }

    bool generate();

private:
   KCal::Todo* generateTODO(Task *task, ResourceList& resourceList);
    KCal::Event* generateEvent(Task* task, ResourceList& resourceList);
};

#endif

