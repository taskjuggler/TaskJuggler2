/*
 * SVGReport.cpp - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id$
 */

#include <config.h>

#ifdef HAVE_KDE

#include "SVGReport.h"

// #include <qptrdict.h>
// #include <qfile.h>
// #include <klocale.h>

#include "tjlib-internal.h"
#include "Project.h"
#include "Scenario.h"

// Older versions of KDE do not have this macro
#ifndef KDE_IS_VERSION
#define KDE_IS_VERSION(a,b,c) 0
#endif

SVGReport::SVGReport(Project* p, const QString& file, const QString& defFile,
                       int dl) :
    Report(p, file, defFile, dl)
{
    // By default, use all scenarios.
    unsigned int i = 0;
    for (ScenarioListIterator sli(project->getScenarioIterator()); *sli ; ++sli, ++i)
    {
        if ((*sli)->getEnabled())
            scenarios.append(i);
    }

    taskSortCriteria[0] = CoreAttributesList::NameUp;
}

#endif