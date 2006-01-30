/*
 * TaskDependency.h - TaskJuggler
 *
 * Copyright (c) 2001, 2002, 2003, 2004 by Chris Schlaeger <cs@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * $Id: TaskDependency.h 1214 2005-11-17 20:05:36Z cs $
 */

#include "TaskDependency.h"

#include <assert.h>

#include "Project.h"
#include "Task.h"
#include "Scenario.h"

TaskDependency::TaskDependency(QString tri, int maxScenarios) : taskRefId(tri)
{
    gapDuration = new long[maxScenarios];
    gapLength = new long[maxScenarios];
    taskRef = 0;
    for (int sc = 0; sc < maxScenarios; ++sc)
        gapDuration[sc] = gapLength[sc] = (sc == 0 ? 0 : -1);
}

TaskDependency::~TaskDependency()
{
    delete [] gapDuration;
    delete [] gapLength;
}

long
TaskDependency::getGapDuration(int sc) const
{
    for ( ; ; )
    {
        if (gapDuration[sc] >= 0)
            return gapDuration[sc];
        Project* p = taskRef->getProject();
        Scenario* parent = p->getScenario(sc)->getParent();
        assert(parent);
        sc = p->getScenarioIndex(parent->getId()) - 1;
    }
}

long
TaskDependency::getGapLength(int sc) const
{
    for ( ; ; )
    {
        if (gapLength[sc] >= 0)
            return gapLength[sc];
        Project* p = taskRef->getProject();
        Scenario* parent = p->getScenario(sc)->getParent();
        assert(parent);
        sc = p->getScenarioIndex(parent->getId()) - 1;
    }
}
